"""Local transactional roster research; country JSONs are publication snapshots."""
import hashlib
import json
import re
import sqlite3
import tempfile
from pathlib import Path

SCHEMA = "COUNTRY_SHARDS_V1"
def encode(value):
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
def digest(value):
    return hashlib.sha256(encode(value).encode()).hexdigest()
def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))
def country(value):
    return value if isinstance(value,str) and re.fullmatch(r"[A-Z]{3}",value) else "UNMAPPED"
def atomic(path, value):
    path.parent.mkdir(parents=True,exist_ok=True)
    if path.exists() and read(path)==value:
        return
    with tempfile.NamedTemporaryFile(dir=path.parent,delete=False) as f:
        temp=Path(f.name)
        f.write((encode(value)+"\n").encode())
    try:
        temp.replace(path)
    finally:
        temp.unlink(missing_ok=True)
def load_snapshot(path):
    obj=read(path)
    if "storage" not in obj:
        return obj
    if obj.pop("storage")!=SCHEMA:
        raise ValueError("Unknown roster snapshot schema")
    order=obj.pop("targetOrder")
    targets={};gaps={}
    for c in obj.pop("countries"):
        if country(c)!=c:
            raise ValueError("Invalid roster country")
        shard=read(path.parent/"countries"/(c+".json"))
        for t in shard["targets"]:
            if country(t["fifa"])!=c or t["fifa"] in targets:
                raise ValueError("Wrong/duplicate target")
            targets[t["fifa"]]=t
        for k,v in shard["existing_club_gaps"].items():
            if k in gaps or country(v.get("association"))!=c:
                raise ValueError("Wrong/duplicate gap")
            gaps[k]=v
    if set(order)!=set(targets) or len(order)!=len(targets):
        raise ValueError("Target order mismatch")
    obj["targets"]=[targets[k] for k in order]
    obj["existing_club_gaps"]=gaps
    return obj

def contains_additive(new,old):
    if type(new)!=type(old):return False
    if isinstance(old,dict):return all(k in new and contains_additive(new[k],v) for k,v in old.items())
    if isinstance(old,list):return len(new)>=len(old) and all(contains_additive(a,b) for a,b in zip(new,old))
    return new==old

def reconcile_additive(work,published):
    a={k:v for k,v in work.items() if k!="errors"}
    b={k:v for k,v in published.items() if k!="errors"}
    if contains_additive(a,b):merged=dict(a)
    elif contains_additive(b,a):merged=dict(b)
    else:return None
    errors=list(work.get("errors",[]))
    for row in published.get("errors",[]):
        if row not in errors:errors.append(row)
    if "errors" in work or "errors" in published:merged["errors"]=errors
    return merged

class Store:
    """One row per target/gap, original nested observations preserved losslessly.
    SQLite is the work authority, JSON publication only; refuse divergent seeds.
    """
    def __init__(self,path,dbpath):
        self.path=path
        self.db=sqlite3.connect(dbpath,timeout=10)
        self.db.execute("PRAGMA journal_mode=WAL")
        self.db.execute("PRAGMA synchronous=FULL")
        self.db.executescript("""
            CREATE TABLE IF NOT EXISTS records(kind TEXT, id TEXT, country TEXT, payload TEXT,
                PRIMARY KEY(kind,id));
            CREATE INDEX IF NOT EXISTS records_country ON records(country,kind);
            CREATE TABLE IF NOT EXISTS state(key TEXT PRIMARY KEY,value TEXT);
        """)
        self.db.execute("BEGIN IMMEDIATE")
        try:
            self._recover_export()
            seed=load_snapshot(path)
            saved=self.db.execute("SELECT value FROM state WHERE key='snapshot'").fetchone()
            if saved is None:
                self._write(seed)
                self._state("snapshot",digest(seed))
                self._state("generation","0")
            elif saved[0]!=digest(seed):
                # A previously completed export may have preceded process termination.
                work=self.load()
                if digest(work)!=digest(seed):
                    merged=reconcile_additive(work,seed)
                    if merged is None:
                        raise RuntimeError("Published roster changed independently; preserve DB and use --recover-snapshot; conflicting data require reconciliation")
                    self._write(merged)
                    previous=int(self.db.execute("SELECT value FROM state WHERE key='generation'").fetchone()[0])
                    self._state("generation",str(previous+1))
                    print("Roster publication recovery: compatible additive snapshot",flush=True)
                self._state("snapshot",digest(seed))
            self.generation=int(self.db.execute("SELECT value FROM state WHERE key='generation'").fetchone()[0])
            self.db.commit()
        except BaseException:
            self.db.rollback()
            self.db.close()
            raise
    def _state(self,key,value):
        self.db.execute("INSERT INTO state VALUES (?,?) ON CONFLICT(key) DO UPDATE SET value=excluded.value",(key,value))
    def _rows(self,data):
        meta={k:v for k,v in data.items() if k not in ("targets","existing_club_gaps")}
        meta["targetOrder"]=[t["fifa"] for t in data["targets"]]
        yield ("meta","root","UNMAPPED",encode(meta))
        for t in data["targets"]:
            yield ("target",t["fifa"],country(t["fifa"]),encode(t))
        for k,v in data["existing_club_gaps"].items():
            yield ("gap",k,country(v.get("association")),encode(v))
    def _write(self,data):
        rows=list(self._rows(data))
        if len({r[:2] for r in rows})!=len(rows):
            raise ValueError("Duplicate roster key")
        previous={(k,i):(c,p) for k,i,c,p in self.db.execute("SELECT kind,id,country,payload FROM records")}
        current={(k,i):(c,p) for k,i,c,p in rows}
        if previous.keys()-current.keys():
            raise ValueError("Research record removal is not automatic")
        changed=[r for r in rows if previous.get(r[:2])!=r[2:]]
        self.db.executemany("""INSERT INTO records VALUES (?,?,?,?)
            ON CONFLICT(kind,id) DO UPDATE SET country=excluded.country,payload=excluded.payload""",changed)
        return bool(changed)
    def load(self):
        targets={};gaps={};meta=None
        for kind,k,_,p in self.db.execute("SELECT kind,id,country,payload FROM records"):
            v=json.loads(p)
            if kind=="meta":meta=v
            elif kind=="target":targets[k]=v
            elif kind=="gap":gaps[k]=v
            else:raise ValueError("Invalid record kind")
        order=meta.pop("targetOrder")
        if len(order)!=len(targets) or set(order)!=set(targets):
            raise ValueError("Database target inventory mismatch")
        meta["targets"]=[targets[k] for k in order]
        meta["existing_club_gaps"]=gaps
        return meta
    def _check(self):
        if int(self.db.execute("SELECT value FROM state WHERE key='generation'").fetchone()[0])!=self.generation:
            raise RuntimeError("Concurrent roster writer; restart without overwriting")
    def save(self,data):
        self.db.execute("BEGIN IMMEDIATE")
        try:
            self._check()
            if self._pending_export() is not None:
                raise RuntimeError("Pending publication; reopen store first")
            changed=self._write(data)
            if changed:
                self._state("generation",str(self.generation+1))
            self.db.commit()
            self.generation+=int(changed)
            return changed
        except BaseException:
            self.db.rollback()
            raise
    def _pending_export(self):
        row=self.db.execute("SELECT value FROM state WHERE key='export_pending'").fetchone()
        return json.loads(row[0]) if row else None
    def _publication(self,data):
        groups={}
        def group(c):
            return groups.setdefault(c,{"targets":[],"existing_club_gaps":{}})
        for t in data["targets"]:group(country(t["fifa"]))["targets"].append(t)
        for k,v in data["existing_club_gaps"].items():group(country(v.get("association")))["existing_club_gaps"][k]=v
        meta={k:v for k,v in data.items() if k not in ("targets","existing_club_gaps")}
        meta.update(storage=SCHEMA,countries=sorted(groups),targetOrder=[t["fifa"] for t in data["targets"]])
        paths={self.path.parent/"countries"/(c+".json"):g for c,g in sorted(groups.items())}
        paths[self.path]=meta
        return paths
    def _recover_export(self):
        pending=self._pending_export()
        if pending is None:return
        data=self.load()
        if pending["digest"]!=digest(data):
            raise RuntimeError("Pending publication does not match work database")
        paths=self._publication(data)
        expected=pending["old"]
        if set(expected)!={p.relative_to(self.path.parent).as_posix() for p in paths}:
            raise RuntimeError("Pending publication inventory mismatch")
        # Validate all files before writing any. Old or new content is safe;
        # any unrelated edit must be reconciled manually.
        for p,value in paths.items():
            relative=p.relative_to(self.path.parent).as_posix()
            current=digest(read(p)) if p.exists() else None
            if current not in (expected[relative],digest(value)):
                raise RuntimeError("External edit during publication: "+str(p))
        for p,value in paths.items():atomic(p,value)
        if digest(load_snapshot(self.path))!=digest(data):
            raise RuntimeError("Roster export verification failed")
        self._state("snapshot",digest(data))
        self.db.execute("DELETE FROM state WHERE key='export_pending'")
    def publish(self):
        self.db.execute("BEGIN IMMEDIATE")
        try:
            self._check()
            if self._pending_export() is None:
                data=self.load()
                published=self.db.execute("SELECT value FROM state WHERE key='snapshot'").fetchone()[0]
                if digest(load_snapshot(self.path))!=published:
                    raise RuntimeError("Published roster changed; refusing overwrite; SQLite checkpoint retained. Reopen after closing other writers or use --recover-snapshot")
                if digest(data)==published and read(self.path).get("storage")==SCHEMA:
                    self.db.commit();return
                paths=self._publication(data)
                self._state("export_pending",encode({"digest":digest(data),"old":{
                    p.relative_to(self.path.parent).as_posix():digest(read(p)) if p.exists() else None for p in paths}}))
            # Durable intent first; after a kill, SQLite remains authoritative.
            self.db.commit()
            self.db.execute("BEGIN IMMEDIATE")
            self._check()
            self._recover_export()
            self.db.commit()
        except BaseException:
            self.db.rollback()
            raise
    def close(self):
        self.db.close()

def recover_snapshot(dbpath,folder):
    """Read-only export to a unique ignored directory, including committed WAL."""
    dbpath=Path(dbpath)
    if not dbpath.is_file():raise FileNotFoundError(dbpath)
    folder=Path(folder);folder.mkdir(parents=True,exist_ok=True)
    store=Store.__new__(Store)
    store.db=sqlite3.connect(dbpath.resolve().as_uri()+"?mode=ro",uri=True,timeout=10)
    try:
        store.db.execute("PRAGMA query_only=ON")
        store.db.execute("BEGIN")
        data=store.load()
        dest=Path(tempfile.mkdtemp(prefix="recovery-",dir=folder))
        store.path=dest/"candidates.json"
        for path,value in store._publication(data).items():atomic(path,value)
        if digest(load_snapshot(store.path))!=digest(data):raise RuntimeError("Recovery verification failed")
        return dest
    finally:store.db.close()

def selftest():
    with tempfile.TemporaryDirectory() as d:
        p=Path(d)/"candidates.json"
        initial={"targets":[{"fifa":"ENG","leagues":[]}],"existing_club_gaps":{"x":{"association":"ENG","players":[]}},"errors":[{"phase":"old"}]}
        atomic(p,initial)
        s=Store(p,Path(d)/"test.sqlite3")
        assert s.load()==initial
        a=s.load();a["existing_club_gaps"]["x"]["players"].append({"player":"Q1"})
        assert s.save(a) and not s.save(a)
        s.close()
        s=Store(p,Path(d)/"test.sqlite3")
        assert s.load()==a
        s.publish();assert load_snapshot(p)==a
        before=p.read_bytes();s.publish();assert p.read_bytes()==before
        a["existing_club_gaps"]["x"]["players"].append({"player":"Q2"})
        assert s.save(a);s.publish();assert load_snapshot(p)==a
        assert read(p.parent/"countries"/"ENG.json")["existing_club_gaps"]["x"]["players"][-1]["player"]=="Q2"
        competing=Store(p,Path(d)/"test.sqlite3")
        a["errors"].append({"phase":"new"})
        s.save(a)
        try:competing.save(competing.load())
        except RuntimeError:pass
        else:raise AssertionError("Concurrent writer accepted")
        competing.close();s.close()
        original=load_snapshot(p)
        external=dict(original,errors=original["errors"]+[{"phase":"external"}])
        for k,v in Store._publication(type("View",(),{"path":p})(),external).items():atomic(k,v)
        s=Store(p,Path(d)/"test.sqlite3")
        assert {r["phase"] for r in s.load()["errors"]}=={"old","new","external"}
        s.publish();expected=s.load();s.close()
        recovered=recover_snapshot(Path(d)/"test.sqlite3",Path(d)/"local")
        assert load_snapshot(recovered/"candidates.json")==expected==load_snapshot(p)
        assert reconcile_additive({"x":[1,2]},{"x":[1]})=={"x":[1,2]}
        assert reconcile_additive({"x":[1]},{"x":[2]}) is None
