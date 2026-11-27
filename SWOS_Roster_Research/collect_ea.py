#!/usr/bin/env python3
"""Local EA statistics only. Original values, no SWOS conversion."""
import argparse
import csv
import hashlib
import io
import json
import re
import sqlite3
import tempfile
import unicodedata
import urllib.request
import zipfile
from pathlib import Path
from http_retry import fetch
from roster_storage import atomic, read
from research_status import report, outcome, progress, show_progress, WorkTotal

HERE=Path(__file__).resolve().parent
LOCAL=HERE/"ea_local"
FC27="https://raw.githubusercontent.com/LakshmiKanth11/EA_FC_ANALYSIS/main/EA_FC_PLAYERS_DATASET_AND_%20DASHBOARD/data/players.csv"
FC26="https://raw.githubusercontent.com/JoelBicheno/fc26_player_data_analysis_toolkit/main/EAFC26-Men.csv"
SOURCES={"fc27":(FC27,"75a86961556e1f6f8df9d881052a514a663dc484"),"fc26":(FC26,"253b752cce7adf7575107c462d5dd7cc9bcbf858")}
def norm(s):
    return "".join(c for c in unicodedata.normalize("NFKD",str(s)).casefold() if c.isalnum())
def header(s):return norm(s)
def read_csv(data):
    return csv.DictReader(io.StringIO(data.decode("utf-8-sig")))
def download(edition,cache):
    url,expected=SOURCES[edition];p=cache/(edition+"-"+expected+".csv")
    if p.exists():data=p.read_bytes()
    else:data=fetch(urllib.request.Request(url,headers={"User-Agent":"SWOS-Expanded/1.2"}),timeout=90,limit=32*1024*1024)
    actual=hashlib.sha1(("blob "+str(len(data))+"\0").encode()+data).hexdigest()
    if actual!=expected:raise ValueError("EA snapshot hash mismatch; source changed")
    if not p.exists():
        t=p.with_suffix(".part");t.write_bytes(data);t.replace(p)
    return data

ALIASES={"id":("playerid","sofifaid","id"),"name":("name","shortname","longname","playername","fullname","commonname"),"nationality":("nationality","nationalityname","nation","country"),"overall":("overallrating","overall","rating","ovr"),"club":("club","clubname","team","teamname"),"league":("league","leaguename"),"gender":("gender","sex"),"birthdate":("birthdate","dob","dateofbirth"),"first":("firstname",),"last":("lastname",)}
DROP={"edition","snapshotdate","source","provenance","license","url","playerurl","playerfaceurl","clublogourl","nationflagurl","image","imageurl","card"}
def parse(data,edition,male_file=False):
    rows=read_csv(data)
    if not rows.fieldnames:raise ValueError("No CSV header")
    keys={header(k):k for k in rows.fieldnames}
    if len(keys)!=len(rows.fieldnames):raise ValueError("Ambiguous CSV headers")
    def get(row,k):
        return next((row.get(keys[a],"").strip() for a in ALIASES[k] if a in keys and row.get(keys[a],"").strip()),"")
    for needed in ("id","nationality","overall"):
        if not any(a in keys for a in ALIASES[needed]):raise ValueError("Missing EA "+needed+" column")
    if not male_file and not any(a in keys for a in ALIASES["gender"]):raise ValueError("Missing gender column")
    if edition=="fc27" and ("edition" not in keys or "snapshotdate" not in keys):raise ValueError("Missing FC27 snapshot columns")
    seen={}
    for row in rows:
        if None in row:raise ValueError("Malformed CSV row")
        gender=get(row,"gender").casefold()
        if gender and gender not in ("male","men","men's football","mens football","m"):continue
        if not gender and not male_file:continue
        if edition=="fc27" and (row[keys["edition"]].casefold()!="fc27" or row[keys["snapshotdate"]] not in ("2026-09-12","12-09-2026")):raise ValueError("FC27 edition/snapshot mismatch")
        uid=get(row,"id");name=get(row,"name") or " ".join(filter(None,(get(row,"first"),get(row,"last"))))
        nation=get(row,"nationality");overall=get(row,"overall")
        if not uid.isdigit() or not name or not nation or not overall.isdigit() or not 1<=int(overall)<=99:raise ValueError("Invalid EA player identity/rating")
        payload={k:v for k,v in row.items() if header(k) not in DROP and not header(k).endswith("url") and v not in ("",None)}
        payload.update(player_id=uid,name=name,nationality=nation)
        if uid in seen and seen[uid]!=payload:raise ValueError("Conflicting EA ID in snapshot: "+uid)
        seen[uid]=payload
    if not seen:raise ValueError("No male player rows")
    return seen

def country_names(root):
    m=json.loads((root/"docs/NATIONAL_ROSTER_SOURCE_MAP_2627.json").read_text(encoding="utf-8-sig"))
    names={}
    for r in m["records"]:
        for n in (r["fifa"],r["source"]["name"]):names.setdefault(norm(n),set()).add(r["fifa"])
    aliases={"United States":"USA","United States of America":"USA","South Korea":"KOR","Korea Republic":"KOR","North Korea":"PRK","Czech Republic":"CZE","Czechia":"CZE","Türkiye":"TUR","Turkey":"TUR","Ivory Coast":"CIV","Côte d'Ivoire":"CIV","DR Congo":"COD","Congo DR":"COD","Republic of Ireland":"IRL","Ireland Republic":"IRL","Bosnia and Herzegovina":"BIH","FYR Macedonia":"MKD","North Macedonia":"MKD","China PR":"CHN","Chinese Taipei":"TPE","Saudi Arabia":"KSA","United Arab Emirates":"UAE"}
    allowed={r["fifa"] for r in m["records"]}
    for n,c in aliases.items():
        if c in allowed:names.setdefault(norm(n),set()).add(c)
    return {n:next(iter(v)) for n,v in names.items() if len(v)==1}

def publish(db,folder,names,only_edition=None):
    # FC27 and FC26 source IDs are kept in separate internal namespaces.
    # Do not assume cross-source IDs identify the same person. FC26 remains
    # a separate set; output records omit edition/source/provenance as requested.
    for edition in ("fc27","fc26"):
        if only_edition and edition!=only_edition:continue
        groups={}
        for uid,p in db.execute("SELECT id,payload FROM players WHERE edition=? ORDER BY id",(edition,)):
            row=json.loads(p);c=names.get(norm(row["nationality"]),"UNMAPPED")
            groups.setdefault(c,[]).append(row)
        if not groups:continue
        dest=folder/edition
        for c,rows in progress("EA publication "+edition.upper()+" countries",sorted(groups.items()),overall=True):
            path=dest/(c+".json")
            if path.exists():
                current={str(row["player_id"]):row for row in rows}
                previous=read(path)
                if not isinstance(previous,list) or any(current.get(str(row["player_id"]))!=row for row in previous):
                    raise RuntimeError("EA published country differs; preserve and reconcile: "+str(path))
            atomic(path,rows)
            report(edition.upper()+" "+c,"SAVED",f"{len(rows)} players")
        atomic(dest/"INDEX.json",{"countries":{c:len(r) for c,r in sorted(groups.items())},"players":sum(map(len,groups.values()))})
def recover_published(db,folder,names):
    # A fresh clone can have published JSON but no local SQLite. Never replace
    # a published country with a partial local database; validate and merge IDs.
    for edition in ("fc27","fc26"):
        recovered={}
        for p in progress("EA recovery "+edition.upper()+" country files",sorted((folder/edition).glob("*.json"))):
            if p.name=="INDEX.json":continue
            rows=read(p)
            if not isinstance(rows,list):raise ValueError("Invalid EA country shard: "+str(p))
            for row in rows:
                uid=str(row["player_id"])
                if not uid.isdigit() or not row.get("name") or not row.get("nationality") or names.get(norm(row["nationality"]),"UNMAPPED")!=p.stem or uid in recovered:
                    raise ValueError("Invalid/duplicate EA published identity: "+str(p))
                recovered[uid]=row
        missing={}
        for uid,row in recovered.items():
            old=db.execute("SELECT payload FROM players WHERE edition=? AND id=?",(edition,uid)).fetchone()
            if old and json.loads(old[0])!=row:raise ValueError("EA local/published ID conflict: "+uid)
            if old is None:missing[uid]=row
        if missing:ingest(db,edition,missing)

def snapshot_digest(db,edition):
    h=hashlib.sha256();count=0
    for uid,payload in db.execute("SELECT id,payload FROM players WHERE edition=? ORDER BY id",(edition,)):
        h.update((json.dumps([uid,json.loads(payload)],ensure_ascii=False,sort_keys=True)+"\n").encode())
        count+=1
    return count,h.hexdigest()

def ingest(db,edition,rows,source_hash=None):
    added=0
    with db:
        existing={uid for uid, in db.execute("SELECT id FROM players WHERE edition=?",(edition,))}
        counter=WorkTotal("EA "+edition.upper()+" player IDs",len(rows),"new IDs",sum(uid not in existing for uid in rows))
        for uid,row in counter.walk(rows.items(),lambda item:item[0] not in existing,every=100):
            payload=json.dumps(row,ensure_ascii=False,sort_keys=True)
            previous=db.execute("SELECT payload FROM players WHERE edition=? AND id=?",(edition,uid)).fetchone()
            if previous:
                if json.loads(previous[0])!=row:raise ValueError("Existing EA snapshot differs; no automatic overwrite")
                continue
            db.execute("INSERT INTO players VALUES(?,?,?)",(edition,uid,payload));added+=1
        count,digest=snapshot_digest(db,edition)
        # A manual CSV or mixed local snapshot must not mark the pinned
        # official-source archive as fully ingested.
        if source_hash and count==len(rows):
            db.execute("INSERT OR REPLACE INTO completed_snapshots VALUES(?,?,?,?)",(edition,source_hash,count,digest))
        else:
            db.execute("DELETE FROM completed_snapshots WHERE edition=?",(edition,))
    return added

def already_ingested(db,edition,source_hash):
    mark=db.execute("SELECT source_hash,row_count,digest FROM completed_snapshots WHERE edition=?",(edition,)).fetchone()
    return bool(mark and mark[0]==source_hash and (mark[1],mark[2])==snapshot_digest(db,edition))

def schema(db):
    db.execute("CREATE TABLE IF NOT EXISTS players(edition TEXT,id TEXT,payload TEXT,PRIMARY KEY(edition,id))")
    db.execute("CREATE TABLE IF NOT EXISTS completed_snapshots(edition TEXT PRIMARY KEY,source_hash TEXT,row_count INTEGER,digest TEXT)")

def native_slots(root):
    """Read the named 2627 club slots; TEST/RESERVE and national copies excluded."""
    slots={}
    for folder in ("teams","squad_extras"):
        for path in sorted((root/"data_json/worlds/2627"/folder).glob("*.json")):
            for team in read(path).get("teams",[]):
                meta=team.get("team",{})
                if meta.get("country_number") in range(80,86):continue
                club=norm(meta.get("team_name") or "")
                if not club:continue
                for field in ("players","extra_players"):
                    for i,p in enumerate(team.get(field,[])):
                        name=p.get("player_name") or ""
                        if not name.strip() or re.match(r"^(TEST|RESERVE)(?:[ _-]*[0-9]+|$)",name,re.I):continue
                        clubkey=team.get("uuid") or (meta.get("country_number"),meta.get("team_ordinal"),path.name)
                        slot=p.get("uuid") or (folder,field,i)
                        slots[(clubkey,slot)]=(norm(name),club)
    return slots

def coverage_counts(slots,ea_rows,visible=False):
    """Exact name+club candidate only, never an EA/native identity binding."""
    from collections import Counter,defaultdict
    multiplicity=Counter(slots.values())
    by_key=defaultdict(lambda:defaultdict(set))
    for edition,uid,row in ea_rows:
        club=next((v for k,v in row.items() if header(k) in ALIASES["club"] and v),"")
        key=(norm(row.get("name") or ""),norm(club))
        if all(key):by_key[key][edition].add(uid)
    counts=Counter()
    jobs=slots.values()
    if visible:
        missing=sum(not by_key.get(k) for k in jobs)
        jobs=WorkTotal("SWOS named player slots",len(slots),"without exact EA candidate",missing).walk(jobs,lambda k:not by_key.get(k),every=500)
    for key in jobs:
        matches=by_key.get(key,{})
        if not matches:counts["no_exact_candidate"]+=1
        elif multiplicity[key]!=1 or any(len(ids)!=1 for ids in matches.values()):
            counts["ambiguous"]+=1
        else:counts["exact_candidate"]+=1
    return {"named_club_slots":len(slots),"without_exact_candidate":counts["no_exact_candidate"],
            "ambiguous":counts["ambiguous"],"exact_candidate":counts["exact_candidate"],
            "without_unambiguous_candidate":counts["no_exact_candidate"]+counts["ambiguous"]}

def coverage(db,slots):
    return coverage_counts(slots,((e,uid,json.loads(p)) for e,uid,p in db.execute("SELECT edition,id,payload FROM players")),visible=True)

def coverage_line(label,c):
    print(label+" | "+" | ".join(f"{k}={v}" for k,v in c.items()),flush=True)

def main():
    ap=argparse.ArgumentParser();ap.add_argument("--self-test",action="store_true");ap.add_argument("--repo-root",type=Path,default=HERE.parent);ap.add_argument("--edition",choices=("fc27","fc26","both"),default="both");ap.add_argument("--fc26-csv",type=Path);ap.add_argument("--export-only",action="store_true")
    a=ap.parse_args()
    if a.self_test:
        from research_status import selftest as progress_selftest
        progress_selftest()
        from http_retry import selftest as http_selftest
        http_selftest()
        data=b"player_id,first_name,last_name,overall_rating,nationality,gender,edition,snapshot_date,pace\n1,A,B,80,Germany,Men's Football,fc27,12-09-2026,70\n2,C,D,80,Germany,Women's Football,fc27,2026-09-12,90\n"
        rows=parse(data,"fc27");assert set(rows)=={"1"} and "edition" not in rows["1"] and rows["1"]["pace"]=="70"
        fc26=b"ID,Name,GENDER,OVR,Nation,Team,PAC,card\n7,Same Name,M,80,Egypt,A,75,https://example.com/card\n8,Same Name,M,79,Egypt,B,73,\n"
        rows=parse(fc26,"fc26",male_file=True);assert len(rows)==2 and "card" not in rows["7"]
        db=sqlite3.connect(":memory:");schema(db)
        assert ingest(db,"fc26",rows,"hash")==2
        assert already_ingested(db,"fc26","hash") and not already_ingested(db,"fc27","hash")
        assert ingest(db,"fc26",rows,"hash")==0
        try:ingest(db,"fc26",{"7":dict(rows["7"],name="Different")},"hash")
        except ValueError:pass
        else:raise AssertionError("Conflicting snapshot accepted")
        assert already_ingested(db,"fc26","hash")
        db.execute("DELETE FROM players WHERE id='7'");assert not already_ingested(db,"fc26","hash")
        db.close()
        slots={1:("alice","club"),2:("bob","club"),3:("bob","club"),4:("carol","other")}
        data=[("fc26","1",{"name":"Alice","Team":"Club"}),("fc27","7",{"name":"Alice","club":"Club"}),
              ("fc26","2",{"name":"Bob","Team":"Club"}),("fc26","3",{"name":"Carol","Team":"Wrong"})]
        assert coverage_counts(slots,data)=={"named_club_slots":4,"without_exact_candidate":1,"ambiguous":2,"exact_candidate":1,"without_unambiguous_candidate":3}
        print("EA self-test PASS");return
    LOCAL.mkdir(exist_ok=True);cache=LOCAL/"sources";cache.mkdir(exist_ok=True)
    names=country_names(a.repo_root);db=sqlite3.connect(LOCAL/"stats.sqlite3")
    db.execute("PRAGMA journal_mode=WAL")
    schema(db)
    failures=[];folder=LOCAL/"countries";added_by_edition={};missing_by_edition={}
    try:
        phase_total=2 if a.export_only else 3
        show_progress("EA research phases",0,phase_total)
        recover_published(db,folder,names)
        show_progress("EA research phases",1,phase_total)
        publish(db,folder,names)
        show_progress("EA research phases",2,phase_total)
        slots=native_slots(a.repo_root)
        before=coverage(db,slots)
        coverage_line("WORKLOAD EA SWOS",before)
        if a.export_only:
            coverage_line("RESULT EA SWOS",before)
            return
        editions=("fc27","fc26") if a.edition=="both" else (a.edition,)
        completed={e for e in editions if not (e=="fc26" and a.fc26_csv is not None) and already_ingested(db,e,SOURCES[e][1])}
        overall=WorkTotal("EA source editions",len(editions),"snapshots not verified complete",len(editions)-len(completed))
        for edition in overall.walk(editions,lambda e:e not in completed):
            try:
                local=edition=="fc26" and a.fc26_csv is not None
                source_hash=None if local else SOURCES[edition][1]
                report(edition.upper(),"SEARCHING","local CSV" if local else "pinned snapshot")
                if edition in completed:
                    report(edition.upper(),"SKIPPED","verified complete snapshot; 0 new players");continue
                data=a.fc26_csv.read_bytes() if local else download(edition,cache)
                report(edition.upper(),"VALIDATING",f"{len(data)} bytes; snapshot parser")
                rows=parse(data,edition,male_file=edition=="fc26" and not local)
                prior={uid for uid, in db.execute("SELECT id FROM players WHERE edition=?",(edition,))}
                report(edition.upper(),"SAVING",f"{len(rows)} validated players; SQLite")
                missing_by_edition[edition]=sum(uid not in prior for uid in rows)
                print(f"WORKLOAD EA {edition.upper()} | {len(rows)} validated player IDs | {missing_by_edition[edition]} new IDs pending",flush=True)
                added=ingest(db,edition,rows,source_hash)
                added_by_edition[edition]=added
                from collections import Counter
                totals=Counter();new=Counter()
                for uid,row in rows.items():
                    c=names.get(norm(row["nationality"]),"UNMAPPED");totals[c]+=1;new[c]+=int(uid not in prior)
                for c in progress("EA "+edition.upper()+" country summaries",sorted(totals),overall=True):report(edition.upper()+" "+c,*outcome(totals[c],new[c]))
                publish(db,folder,names,edition)
                report(edition.upper(),*outcome(len(rows),added))
            except Exception as e:
                failures.append((edition,str(e)));report(edition.upper(),"ERROR",str(e))
        after=coverage(db,slots)
        coverage_line("RESULT EA SWOS",after)
        print(f"RESULT EA | new player statistic records committed={sum(added_by_edition.values())} | by edition={added_by_edition} | missing IDs before ingest={missing_by_edition} | verified snapshots skipped={len(completed)} | edition errors={len(failures)} | net new exact candidate slots={after['exact_candidate']-before['exact_candidate']}",flush=True)
    finally:db.close()
    show_progress("EA research phases",phase_total,phase_total)
    if failures:raise SystemExit(1)
if __name__=="__main__":main()
