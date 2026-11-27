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
from roster_storage import atomic

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

def publish(db,folder,names):
    # FC27 and FC26 source IDs are kept in separate internal namespaces.
    # Do not assume cross-source IDs identify the same person. FC26 remains
    # a separate set; output records omit edition/source/provenance as requested.
    for edition in ("fc27","fc26"):
        groups={}
        for uid,p in db.execute("SELECT id,payload FROM players WHERE edition=? ORDER BY id",(edition,)):
            row=json.loads(p);c=names.get(norm(row["nationality"]),"UNMAPPED")
            groups.setdefault(c,[]).append(row)
        if not groups:continue
        dest=folder/edition
        for c,rows in sorted(groups.items()):atomic(dest/(c+".json"),rows)
        atomic(dest/"INDEX.json",{"countries":{c:len(r) for c,r in sorted(groups.items())},"players":sum(map(len,groups.values()))})
def main():
    ap=argparse.ArgumentParser();ap.add_argument("--self-test",action="store_true");ap.add_argument("--repo-root",type=Path,default=HERE.parent);ap.add_argument("--edition",choices=("fc27","fc26","both"),default="both");ap.add_argument("--fc26-csv",type=Path)
    a=ap.parse_args()
    if a.self_test:
        data=b"player_id,first_name,last_name,overall_rating,nationality,gender,edition,snapshot_date,pace\n1,A,B,80,Germany,Men's Football,fc27,12-09-2026,70\n2,C,D,80,Germany,Women's Football,fc27,2026-09-12,90\n"
        rows=parse(data,"fc27");assert set(rows)=={"1"} and "edition" not in rows["1"] and rows["1"]["pace"]=="70"
        fc26=b"ID,Name,GENDER,OVR,Nation,Team,PAC,card\n7,Same Name,M,80,Egypt,A,75,https://example.com/card\n8,Same Name,M,79,Egypt,B,73,\n"
        rows=parse(fc26,"fc26",male_file=True);assert len(rows)==2 and "card" not in rows["7"]
        print("EA self-test PASS");return
    LOCAL.mkdir(exist_ok=True);cache=LOCAL/"sources";cache.mkdir(exist_ok=True)
    names=country_names(a.repo_root);db=sqlite3.connect(LOCAL/"stats.sqlite3")
    db.execute("PRAGMA journal_mode=WAL")
    db.execute("CREATE TABLE IF NOT EXISTS players(edition TEXT,id TEXT,payload TEXT,PRIMARY KEY(edition,id))")
    failures=[]
    try:
        for edition in ("fc27","fc26") if a.edition=="both" else (a.edition,):
            try:
                data=a.fc26_csv.read_bytes() if edition=="fc26" and a.fc26_csv else download(edition,cache)
                # Local CSV override requires explicit gender; verified men's file does not.
                rows=parse(data,edition,male_file=edition=="fc26" and not a.fc26_csv)
                with db:
                    for uid,row in rows.items():
                        payload=json.dumps(row,ensure_ascii=False,sort_keys=True)
                        previous=db.execute("SELECT payload FROM players WHERE edition=? AND id=?",(edition,uid)).fetchone()
                        if previous and previous[0]!=payload:raise ValueError("Existing EA snapshot differs; no automatic overwrite")
                        db.execute("INSERT OR IGNORE INTO players VALUES(?,?,?)",(edition,uid,payload))
                print(edition,len(rows),"male player rows",flush=True)
            except Exception as e:
                failures.append((edition,str(e)));print(edition,"ERROR",e,flush=True)
        publish(db,LOCAL/"countries",names)
    finally:db.close()
    if failures:raise SystemExit(1)
if __name__=="__main__":main()
