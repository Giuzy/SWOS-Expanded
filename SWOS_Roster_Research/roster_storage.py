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

class ReconciliationConflict(ValueError):
    def __init__(self,path):
        super().__init__("Incompatible roster values at " + "/".join(map(str,path)))

def merge_compatible(work,published,path=()):
    """Merge independent additions; never choose between differing scalar values."""
    if type(work) is not type(published):
        raise ReconciliationConflict(path)
    if work==published and encode(work)==encode(published):
        return work
    if isinstance(work,dict):
        merged=dict(work)
        for key,value in published.items():
            merged[key]=merge_compatible(work[key],value,path+(key,)) if key in work else value
        return merged
    if isinstance(work,list):
        field=path[-1] if path else None
        if field in ("errors","positionObservations","name_matches","wiki_squad_players",
                     "wiki_club_leads","wikidata_undated_players","domestic_source_observations","identity_candidates"):
            # Append-only observations, not authoritative player records.
            merged=list(work);seen={encode(row) for row in work}
            for row in published:
                key=encode(row)
                if key not in seen:
                    merged.append(row);seen.add(key)
            return merged
        if contains_additive(work,published):return work
        if contains_additive(published,work):return published
        if field in ("targets","leagues","seasons","clubs","players",
                     "alternative_clubs","alternative_players","transfermarkt_clubs","transfermarkt_players"):
            # Source-specific identity, always inside its containing owner/edition.
            from alternative_sources import observation_key
            def wd(value):
                match=re.fullmatch(r"(?:https?://www.wikidata.org/entity/)?(Q[1-9][0-9]*)",str(value or ""))
                return match[1] if match else None
            def key_for(row):
                if field=="targets":return row.get("fifa")
                if field in ("leagues","seasons","clubs"):
                    return wd(row.get({"leagues":"league","seasons":"season","clubs":"club"}[field]))
                if field=="players":
                    pid=wd(row.get("player"))
                    membership=row.get("membership")
                    return (pid,membership.rsplit("/",1)[-1] if membership else None,
                            row.get("start"),row.get("end")) if pid and row.get("start") else None
                return observation_key(field,row)
            def index(rows):
                result={}
                for row in rows:
                    if not isinstance(row,dict):
                        raise ReconciliationConflict(path)
                    key=key_for(row)
                    if not key or key in result:
                        raise ReconciliationConflict(path+("missing/duplicate source identity",))
                    result[key]=row
                return result
            a,b=index(work),index(published)
            return [merge_compatible(row,b[key],path+(str(key),)) if key in b else row
                    for key,row in a.items()]+[row for key,row in b.items() if key not in a]
        # Unknown lists: retain the existing strict prefix-only policy.
        if contains_additive(work,published):return work
        if contains_additive(published,work):return published
    raise ReconciliationConflict(path)

def reconcile_additive(work,published):
    try:return merge_compatible(work,published)
    except ReconciliationConflict:return None

class StorageFailure(RuntimeError):
    """Fatal checkpoint failure, never a source acquisition error."""

class PublicationConflict(StorageFailure):
    def __init__(self,message,dbpath,retryable=False):
        super().__init__(message)
        self.dbpath=Path(dbpath)
        self.retryable=retryable

class Store:
    """One row per target/gap, original nested observations preserved losslessly.
    SQLite is the work authority, JSON publication only; refuse divergent seeds.
    """
    def __init__(self,path,dbpath):
        self.path=path
        self.dbpath=Path(dbpath)
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
                    try:merged=merge_compatible(work,seed)
                    except ReconciliationConflict as exc:
                        raise PublicationConflict(str(exc),self.dbpath) from exc
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
                raise PublicationConflict("External edit during publication: "+str(p),self.dbpath)
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
                    raise PublicationConflict("Published roster changed; SQLite checkpoint retained; reconciliation required",self.dbpath,retryable=True)
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
        # Independent country and source-ID additions, not name matching.
        row={"country":"SEN","competition":"SEN1","season":"2026/27","level":1,
             "transfermarktClubId":"123","transfermarktPlayerId":"456","playerLabel":"A"}
        left={"targets":[{"fifa":"SEN","transfermarkt_players":[row]},
                         {"fifa":"CAM","left":1}],"errors":[{"phase":"left"}]}
        right={"targets":[{"fifa":"CAM","right":2},{"fifa":"SEN","transfermarkt_players":[
            dict(row,transfermarktPlayerId="789"),dict(row,positionObservations=[{"group":"DEF"}])]}],
            "errors":[{"phase":"right"}]}
        merged=merge_compatible(left,right)
        assert [t["fifa"] for t in merged["targets"]]==["SEN","CAM"]
        assert merged["targets"][1]=={"fifa":"CAM","left":1,"right":2}
        assert [p["transfermarktPlayerId"] for p in merged["targets"][0]["transfermarkt_players"]]==["456","789"]
        assert merged["targets"][0]["transfermarkt_players"][0]["positionObservations"]==[{"group":"DEF"}]
        assert len(merged["errors"])==2
        assert reconcile_additive({"targets":[{"fifa":"SEN","status":"P"}]},
                                  {"targets":[{"fifa":"SEN","status":"NO DATA"}]}) is None
        assert reconcile_additive({"transfermarkt_players":[row]},
                                  {"transfermarkt_players":[dict(row,playerLabel="B")]}) is None
        assert merge_compatible({"domestic_source_observations":[{"source":"WD"}]},
                                {"domestic_source_observations":[{"source":"RSSSF"}]})=={
                                    "domestic_source_observations":[{"source":"WD"},{"source":"RSSSF"}]}
        a_wd={"leagues":[{"league":"Q1","seasons":[{"season":"Q2","clubs":[
            {"club":"Q3","players":[{"player":"Q4","start":"2026-01-01"}]}]}]}]}
        b_wd={"leagues":[{"league":"Q1","seasons":[{"season":"Q2","clubs":[
            {"club":"Q3","players":[{"player":"Q5","start":"2026-01-01"}]}]}]}]}
        assert len(merge_compatible(a_wd,b_wd)["leagues"][0]["seasons"][0]["clubs"][0]["players"])==2
        with tempfile.TemporaryDirectory() as sub:
            q=Path(sub)/"candidates.json";db=Path(sub)/"resume.sqlite3"
            base={"targets":[{"fifa":"SEN"},{"fifa":"CAM"}],"existing_club_gaps":{},"errors":[]}
            atomic(q,base)
            w=Store(q,db);work=w.load();work["targets"][0]["transfermarkt_players"]=[row]
            w.save(work)
            external=load_snapshot(q);external["targets"][1]["wiki_squad_players"]=[{"playerLabel":"C"}]
            for dest,value in w._publication(external).items():atomic(dest,value)
            try:w.publish()
            except PublicationConflict as exc:assert exc.retryable
            else:raise AssertionError("Unexpected publication overwrite")
            w.close()
            w=Store(q,db);w.publish()
            assert load_snapshot(q)["targets"]==w.load()["targets"]==[
                {"fifa":"SEN","transfermarkt_players":[row]},
                {"fifa":"CAM","wiki_squad_players":[{"playerLabel":"C"}]}]
            w.close()
        s=Store(p,Path(d)/"test.sqlite3")
        published=load_snapshot(p)
        divergent=load_snapshot(p)
        divergent["existing_club_gaps"]["x"]["players"][0]["player"]="Q99"
        for k,v in s._publication(divergent).items():atomic(k,v)
        try:s.publish()
        except PublicationConflict as exc:
            assert exc.dbpath==Path(d)/"test.sqlite3"
        else:raise AssertionError("Divergent publication accepted")
        assert s.load()==published and load_snapshot(p)==divergent
        s.close()
        try:Store(p,Path(d)/"test.sqlite3")
        except PublicationConflict as exc:
            assert exc.dbpath==Path(d)/"test.sqlite3"
            dest=recover_snapshot(exc.dbpath,Path(d)/"local")
            assert load_snapshot(dest/"candidates.json")==published
            assert load_snapshot(p)==divergent
        else:raise AssertionError("Divergent startup accepted")
