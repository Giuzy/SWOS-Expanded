#!/usr/bin/env python3
"""Offline-first, read-only SWOS competition evidence collector. NOT a runtime importer."""
import argparse
import collections
import csv
import datetime as dt
import hashlib
import io
import json
import os
from pathlib import Path
import re
import sqlite3
import sys
import tempfile
import traceback
import unicodedata
import urllib.request
import zipfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
CUTOFF = "2026-07-01"
START = "1996-01-01"
FIELDS = ("identity", "participants", "format", "calendar", "regulation", "movement", "qualification", "july1", "evolution")
# These are explicit division codes from the Matches.txt / football-data schema.
DIV = {"E0":("ENG",1),"E1":("ENG",2),"E2":("ENG",3),"E3":("ENG",4),"EC":("ENG",5),
       "SC0":("SCO",1),"SC1":("SCO",2),"SC2":("SCO",3),"SC3":("SCO",4),
       "D1":("GER",1),"D2":("GER",2),"F1":("FRA",1),"F2":("FRA",2),
       "I1":("ITA",1),"I2":("ITA",2),"SP1":("ESP",1),"SP2":("ESP",2),
       "P1":("POR",1),"N1":("NED",1),"B1":("BEL",1),"G1":("GRE",1),"T1":("TUR",1)}
COUNTRIES = {
 "ALBANIA":"ALB","ALGERIA":"ALG","ARGENTINA":"ARG","AUSTRALIA":"AUS","AUSTRIA":"AUT",
 "BELARUS":"BLR","BELGIUM":"BEL","BOLIVIA":"BOL","BRAZIL":"BRA","BULGARIA":"BUL",
 "CHILE":"CHI","CHINA":"CHN","COLOMBIA":"COL","CROATIA":"CRO","CYPRUS":"CYP",
 "CZECH REPUBLIC":"CZE","CZECHIA":"CZE","DENMARK":"DEN","ECUADOR":"ECU",
 "EL SALVADOR":"SLV","ENGLAND":"ENG","ESTONIA":"EST","FAROE ISLANDS":"FRO",
 "FINLAND":"FIN","FRANCE":"FRA","GERMANY":"GER","GHANA":"GHA","GREECE":"GRE",
 "HOLLAND":"NED","NETHERLANDS":"NED","HUNGARY":"HUN","ICELAND":"ISL","INDIA":"IND",
 "ISRAEL":"ISR","ITALY":"ITA","JAPAN":"JPN","LATVIA":"LVA","LITHUANIA":"LTU",
 "LUXEMBOURG":"LUX","MALTA":"MLT","MEXICO":"MEX","NEW ZEALAND":"NZL",
 "NORTHERN IRELAND":"NIR","NORWAY":"NOR","PARAGUAY":"PAR","PERU":"PER",
 "POLAND":"POL","PORTUGAL":"POR","REPUBLIC IRELAND":"IRL","REPUBLIC OF IRELAND":"IRL",
 "IRELAND":"IRL","ROMANIA":"ROU","RUSSIA":"RUS","SAN MARINO":"SMR","SCOTLAND":"SCO",
 "SERBIA":"SRB","SLOVAKIA":"SVK","SLOVENIA":"SVN","SOUTH AFRICA":"RSA","SPAIN":"ESP",
 "SURINAM":"SUR","SURINAME":"SUR","SWEDEN":"SWE","SWITZERLAND":"SUI","TAIWAN":"TPE",
 "TURKEY":"TUR","UKRAINE":"UKR","URUGUAY":"URU","USA":"USA","UNITED STATES":"USA",
 "VENEZUELA":"VEN","WALES":"WAL","YUGOSLAVIA":"YUG","SOUTH KOREA":"KOR",
 "KOREA REPUBLIC":"KOR","SAUDI ARABIA":"KSA","IRAN":"IRN","CANADA":"CAN",
 "ANDORRA":"AND","ARMENIA":"ARM","AZERBAIJAN":"AZE","BOSNIA HERZ":"BIH","BOSNIA AND HERZEGOVINA":"BIH",
 "COSTA RICA":"CRC","EGYPT":"EGY","GEORGIA":"GEO","GIBRALTAR":"GIB","GUATEMALA":"GUA",
 "HONDURAS":"HON","KAZAKHSTAN":"KAZ","KOSOVO":"KOS","LIECHTENSTEIN":"LIE","MOLDOVA":"MDA",
 "MONTENEGRO":"MNE","MOROCCO":"MAR","N MACEDONIA":"MKD","NORTH MACEDONIA":"MKD",
 "PANAMA":"PAN","QATAR":"QAT","TUNISIA":"TUN","U A E":"UAE","UNITED ARAB EMIRATES":"UAE"}
CODES = set(COUNTRIES.values())
CODE_ALIAS = {"DEU":"GER","CHE":"SUI","NLD":"NED","GRC":"GRE","HRV":"CRO","ROM":"ROU",
              "JAP":"JPN","JPN":"JPN","CHL":"CHI","PRY":"PAR","URY":"URU","ZAF":"RSA","DNK":"DEN"}
REPOS = [
 ("footballcsv/footballcsv.github.io","master"),
 ("footballcsv/england","master"),("footballcsv/deutschland","master"),
 ("schochastics/football-data","master"),("withqwerty/open-football","main"),
 ("openfootball/football.json","master"),
 ("xgabora/Club-Football-Match-Data","main"),("openfootball/world","master")]
RULE = re.compile(r"\b(?:tie.break|away goals?|extra time|penalt(?:y|ies)|promot(?:ed|ion)|relegat(?:ed|ion)|qualif(?:y|ied|ication)|play.off|points? deduction|head.to.head|goal difference|goal average|abolished|renamed|replaced|format|rules?|regulations?|first edition)\b",re.I)
RESULT = re.compile(r"^([^\d\[\]<>][^<>\[\]]{1,100}?)\s+(\d{1,2})\s*[-:]\s*(\d{1,2})\s+([^\d\[\]<>][^<>\[\]]{1,100}?)(?:\s*\[([^\]]+)\])?\s*$")
EDITION = re.compile(r"\b((?:19|20)\d{2})(?:\s*[/–-]\s*(\d{2}|\d{4}))?\b")
STAGE = re.compile(r"^(?:\d+(?:st|nd|rd|th)\s+round|round\s+\d+|group\s+[A-Z0-9]+|quarter.?finals?|semi.?finals?|finals?|first legs?|second legs?|preliminary round|matchday\s+\d+)", re.I)

def norm(value):
    return re.sub(r"[^a-z0-9]", "", unicodedata.normalize("NFKD", str(value or "")).encode("ascii","ignore").decode().lower())

def country(value):
    s = str(value or "").strip().upper().replace("_"," ")
    return COUNTRIES.get(s, CODE_ALIAS.get(s, s if s in CODES else ""))

def iso(value, dayfirst=True):
    s = str(value or "").strip()
    if not s or s.lower() in ("nan","nat","none","null","<na>"):
        return ""
    if re.match(r"^\d{4}-\d{2}-\d{2}",s):
        s = s[:10]
    for fmt in ("%Y-%m-%d","%Y/%m/%d","%d/%m/%Y","%d/%m/%y","%d.%m.%Y","%d-%m-%Y",
                "%b %d, %Y","%B %d, %Y","%d %b %Y","%d %B %Y","%a %b %d %Y","%a %b %d, %Y","%a %d %b %Y"):
        try:
            d = dt.datetime.strptime(s,fmt).date()
            if 1900 <= d.year <= 2100:
                return d.isoformat()
        except ValueError:
            pass
    return ""

def integer(value):
    try:
        v = float(str(value))
        return int(v) if v.is_integer() and 0 <= v <= 99 else None
    except (ValueError,TypeError,OverflowError):
        return None

def edition(value, date):
    s = str(value or "").strip()
    if re.fullmatch(r"(?:19|20)\d{2}(?:[-/]\d{2,4})?",s):
        return s, "SOURCE_SEASON"
    return (date[:4], "CALENDAR_YEAR_BUCKET") if date else ("UNKNOWN","UNKNOWN")

def hashfile(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda:f.read(1024*1024),b""):
            h.update(block)
    return h.hexdigest()

def dump(path, data):
    path.write_text(json.dumps(data,ensure_ascii=False,indent=2,default=str)+"\n",encoding="utf-8")

def csvout(path, header, rows):
    with path.open("w",encoding="utf-8-sig",newline="") as f:
        w=csv.writer(f); w.writerow(header); w.writerows(rows)

def get(row,*keys):
    for k in keys:
        v = row.get(norm(k))
        if v is not None and str(v).strip().lower() not in ("","nan","nat","none","null","<na>"):
            return v
    return ""

def structured(row, default_comp="", default_country="", default_season=""):
    r={norm(k):v for k,v in row.items()}
    date=iso(get(r,"date","matchdate","match_date","game_date"))
    home=str(get(r,"home","home_team","HomeTeam","home_name","team1")).strip()
    away=str(get(r,"away","away_team","AwayTeam","away_name","team2")).strip()
    comp=str(get(r,"competition","league","division","div","comp","tournament") or default_comp).strip()
    c=country(get(r,"country","country_name","nation") or default_country)
    if comp.upper() in DIV:
        c,_=DIV[comp.upper()]
    if str(get(r,"level")).lower()=="international":
        c="INTERNATIONAL"
    if not c:
        hc=country(get(r,"home_country","home_code"))
        ac=country(get(r,"away_country","away_code"))
        if hc and hc==ac and str(get(r,"level")).lower()!="international":
            c=hc
    if not date or not home or not away or norm(home)==norm(away) or not comp or date<START or date>"2027-12-31":
        return None
    hg=integer(get(r,"gh","FTHome","FTHG","home_goals","home_score","score1","HG"))
    ag=integer(get(r,"ga","FTAway","FTAG","away_goals","away_score","score2","AG"))
    if hg is None or ag is None:
        score=str(get(r,"score","full_time","result","FT"))
        m=re.fullmatch(r"\s*(\d{1,2})\s*[-–:]\s*(\d{1,2})\s*",score)
        if m: hg,ag=map(int,m.groups())
    season,scheme=edition(get(r,"season","season_end_year","season_end","year") or default_season,date)
    if scheme=="CALENDAR_YEAR_BUCKET" and comp.upper() in DIV:
        d=dt.date.fromisoformat(date); y=d.year if d.month>=7 else d.year-1
        season=f"{y}/{(y+1)%100:02d}";scheme="JULY_SPLIT_CANDIDATE"
    # No real-world result on/after the 2627 start is written to output.
    if date>=CUTOFF: hg=ag=None
    return (comp,c,season,scheme,date,"EXPLICIT",home,away,hg,ag,str(get(r,"round","stage","matchweek","wk"))[:100])

class Collector:
    def __init__(self, out):
        self.out=out
        self.db=sqlite3.connect(out/"evidence.sqlite")
        self.db.executescript("""
          PRAGMA journal_mode=DELETE; PRAGMA synchronous=NORMAL;
          CREATE TABLE fixture(source TEXT, locator TEXT, competition TEXT,country TEXT,season TEXT,
            season_basis TEXT,date TEXT,date_basis TEXT,home TEXT,away TEXT,hg INTEGER,ag INTEGER,stage TEXT);
          CREATE TABLE rule(source TEXT,locator TEXT,competition TEXT,country TEXT,season TEXT,excerpt TEXT);
          CREATE TABLE native(world TEXT,country TEXT,name TEXT,uuid TEXT,file INTEGER,ordinal INTEGER,league INTEGER,path TEXT);
        """)
        self.log=[]; self.pending=[]; self.rulebuf=[]; self.counts=collections.Counter()
    def add(self, source, locator, data):
        if data is not None:
            self.pending.append((source,locator,*data));self.counts[source]+=1
        if len(self.pending)>=20000: self.flush()
    def flush(self):
        self.db.executemany("INSERT INTO fixture VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)",self.pending)
        self.db.executemany("INSERT INTO rule VALUES (?,?,?,?,?,?)",self.rulebuf)
        self.pending.clear();self.rulebuf.clear();self.db.commit()
    def note(self, source, member, status, detail=""):
        self.log.append({"source":source,"member":member,"status":status,"detail":str(detail)[:1000]})
    def table(self, frame, source, locator, comp="", c="", season=""):
        cols={norm(k) for k in frame.columns}
        if not (cols & {"home","hometeam","homename","team1"}) or not (cols & {"away","awayteam","awayname","team2"}):
            self.note(source,locator,"UNSUPPORTED_SCHEMA",list(frame.columns));return
        before=self.counts[source]
        for i,row in enumerate(frame.itertuples(index=False,name=None),1):
            self.add(source,locator+":"+str(i),structured(dict(zip(frame.columns,row)),comp,c,season))
        self.note(source,locator,"PARSED",str(self.counts[source]-before)+" candidates; columns "+",".join(map(str,frame.columns)))
    def csvfile(self, stream, source, locator, comp="", c="", season=""):
        try:
            raw=io.TextIOWrapper(stream,encoding="utf-8-sig",errors="replace",newline="")
            sample=raw.read(8192);raw.seek(0)
            try: dialect=csv.Sniffer().sniff(sample,delimiters=",;\t")
            except csv.Error: dialect=csv.excel
            reader=csv.DictReader(raw,dialect=dialect)
            cols={norm(x) for x in (reader.fieldnames or [])}
            if not (cols & {"home","hometeam","homename","team1"}) or not (cols & {"away","awayteam","awayname","team2"}):
                self.note(source,locator,"UNSUPPORTED_SCHEMA",reader.fieldnames);return
            before=self.counts[source]
            for i,row in enumerate(reader,2):
                self.add(source,locator+":"+str(i),structured(row,comp,c,season))
            self.note(source,locator,"PARSED",self.counts[source]-before)
        except Exception as exc: self.note(source,locator,"ERROR",exc)
    def corpus(self,z,source):
        # RSSSF prose is heterogeneous. Extract only candidate lines with explicit score syntax
        # and date context; no proof of regulation or full edition from heuristics.
        for member in z.infolist():
            p=member.filename
            if member.is_dir() or not p.lower().endswith(".txt") or member.file_size>8_000_000: continue
            if not re.search(r"/(?:tables[^/]*|ec|ac|sac|oc|cc|interclub|misc)/",p,re.I): continue
            try: lines=z.read(member).decode("utf-8",errors="replace").splitlines()
            except Exception as exc: self.note(source,p,"ERROR",exc);continue
            title=next((x.strip("= ").strip() for x in lines[:25] if x.startswith("==")),"")
            if not title: continue
            match=EDITION.search(title)
            if not match: continue
            y=int(match.group(1))
            if not 1996<=y<=2026: continue
            season=match.group(0); comp=title; c=""
            for label,code in sorted(COUNTRIES.items(),key=lambda x:-len(x[0])):
                if title.upper().startswith(label+" "): c=code;break
            date=""; basis=""; stage=""; startyear=y
            author=next((x.strip() for x in lines[-80:] if x.lower().startswith("author:")),"")
            before=self.counts[source]
            for i,line in enumerate(lines,1):
                s=line.strip()
                if s.startswith("=="):
                    section=s.strip("= ").strip().split("‹",1)[0].strip()
                    em=EDITION.search(section)
                    if em:
                        match=em;startyear=int(em.group(1));season=em.group(0)
                    if section:
                        comp=section;date="";basis="";stage=""
                    continue
                if not 1996<=startyear<=2026:continue
                if not s or s.startswith(("<!--","‹","==","[", "<")) and not s.startswith("["): continue
                # Distinguish headers with an explicit division/cup from match records.
                if len(s)<120 and not RESULT.fullmatch(s) and not STAGE.match(s) and re.search(r"\b(?:division|liga|league|cup|championship)\b",s,re.I) and not RULE.search(s) and not re.search(r"\d\s*[-:]\s*\d",s):
                    if not s.startswith(("NB:","[")) and not re.search(r"[,;]",s):
                        comp=s;date="";basis="";stage=""
                if STAGE.match(s):
                    stage=s.split("[",1)[0][:100]
                    date="";basis=""
                brackets=re.findall(r"\[([^\]]+)\]",s)
                def dated(token):
                    d=iso(token)
                    if d: return d,"EXPLICIT"
                    for fmt in ("%b %d","%B %d","%d %b","%d %B"):
                        try:
                            md=dt.datetime.strptime(token+" 2000",fmt+" %Y")
                            # Split-season month heuristic is labelled, never certified.
                            yy=startyear+(1 if match.group(2) and md.month<7 else 0)
                            return dt.date(yy,md.month,md.day).isoformat(),"INFERRED_FROM_EDITION"
                        except ValueError: pass
                    return "",""
                # RSSSF also uses "Sep 11: Club 0 (0) Club 1 (1)".
                inline=re.fullmatch(r"([A-Za-z]{3,9} \d{1,2})(?:, (\d{4}))?:\s*(.*?)\s+(\d{1,2})\s*\(\d{1,2}\)\s+(.*?)\s+(\d{1,2})\s*\(\d{1,2}\)\s*",s)
                if inline:
                    token,yy,h,g,v,a=inline.groups();d,b=dated(token+(", "+yy if yy else ""))
                    if d and START<=d<="2027-12-31" and norm(h)!=norm(v):
                        hg,ag=(int(g),int(a)) if d<CUTOFF else (None,None)
                        self.add(source,p+":"+str(i),(comp,c,season,"SOURCE_TITLE",d,b,h.strip(),v.strip(),hg,ag,stage))
                    continue
                m=RESULT.fullmatch(s)
                if m:
                    h,g,a,v,annotation=m.groups()
                    dd,bb=dated(annotation or "")
                    d=dd or date; b=bb or basis
                    if d and START<=d<="2027-12-31" and norm(h)!=norm(v):
                        hg,ag=(int(g),int(a)) if d<CUTOFF else (None,None)
                        self.add(source,p+":"+str(i),(comp,c,season,"SOURCE_TITLE",d,b,h.strip(),v.strip(),hg,ag,stage))
                else:
                    for token in brackets:
                        dd,bb=dated(token)
                        if dd: date,basis=dd,bb
                if RULE.search(s) and len(s)<=600 and not m and (startyear<2026):
                    self.rulebuf.append((source,p+":"+str(i)+" "+author,comp,c,season,s))
                if len(self.rulebuf)>=20000: self.flush()
            if self.counts[source]>before: self.note(source,p,"CANDIDATE_TEXT",self.counts[source]-before)
    def archive(self,path,source,kind):
        import pandas as pd
        try:
            with zipfile.ZipFile(path) as z:
                if kind=="corpus": self.corpus(z,source);return
                for member in z.infolist():
                    p=member.filename;lo=p.lower()
                    if member.is_dir() or member.file_size>180_000_000: continue
                    if lo.endswith(".rds") and ("match_result" in lo or "matchresults" in lo):
                        try:
                            import rdata
                            payload=z.read(member)
                            try: frame=rdata.read_rds(io.BytesIO(payload))
                            except Exception:
                                import pyreadr
                                with tempfile.TemporaryDirectory() as td:
                                    tmp=Path(td)/"data.rds";tmp.write_bytes(payload)
                                    frame=next(iter(pyreadr.read_r(str(tmp)).values()))
                            if not isinstance(frame,pd.DataFrame): raise ValueError("RDS is not a dataframe")
                            default=re.sub(r"_?match_?results.*$","",Path(p).stem,flags=re.I) or Path(p).stem
                            self.table(frame,source,p,default,country(default))
                        except Exception as exc:self.note(source,p,"ERROR_RDS",exc)
                    elif lo.endswith((".csv",".tsv")):
                        with z.open(member) as f: self.csvfile(io.BytesIO(f.read()),source,p,*path_hints(p))
                    elif lo.endswith(".json") and "football" in source and member.file_size<10_000_000:
                        try:
                            j=json.loads(z.read(member))
                            if not isinstance(j,dict) or not isinstance(j.get("matches"),list):continue
                            comp,c,season=path_hints(p);comp=j.get("name") or comp
                            rows=[]
                            for m in j["matches"]:
                                if not isinstance(m,dict): continue
                                score=m.get("score") or {};ft=score.get("ft") if isinstance(score,dict) else None
                                rows.append({"date":m.get("date"),"home":m.get("team1"),"away":m.get("team2"),"home_goals":ft[0] if isinstance(ft,list) and len(ft)==2 else None,"away_goals":ft[1] if isinstance(ft,list) and len(ft)==2 else None,"round":m.get("round")})
                            self.table(pd.DataFrame(rows),source,p,comp,c,season)
                        except Exception as exc:self.note(source,p,"ERROR_JSON",exc)
                    elif lo.endswith((".txt",".md")) and (source.endswith("openfootball/world") or source.endswith("withqwerty/open-football") or "footballcsv.github.io" in source):
                        # Inventory links / document locations, not unverified fixture import.
                        self.note(source,p,"TEXT_INVENTORY",member.file_size)
        except Exception as exc:self.note(source,str(path),"ERROR_ARCHIVE",exc)

def path_hints(p):
    stem=Path(p).stem; c=""; season=""
    for part in re.split(r"[/\\]",p):
        cc=country(part)
        if cc: c=cc
        if re.fullmatch(r"(?:19|20)\d{2}[-/]\d{2,4}",part):season=part
    # FootballCSV filenames e.g. eng.1.csv; no guesses for other file names.
    if stem.upper() in DIV:
        return stem.upper(),DIV[stem.upper()][0],season
    m=re.fullmatch(r"([a-z]{2,3})\.(\d)",stem,re.I)
    short={"eng":"ENG","de":"GER","ger":"GER","it":"ITA","es":"ESP","fr":"FRA","pt":"POR","nl":"NED","be":"BEL","sco":"SCO"}
    if m and m.group(1).lower() in short:
        c=short[m.group(1).lower()];stem=c+" DIV"+m.group(2)
    return stem,c,season

def download(repo,branch,cache,limit):
    target=cache/(repo.replace("/","__")+"-"+branch+".zip")
    if target.exists() and zipfile.is_zipfile(target):return target,"CACHE"
    url="https://codeload.github.com/"+repo+"/zip/refs/heads/"+branch
    temp=target.with_suffix(".part")
    try:
        req=urllib.request.Request(url,headers={"User-Agent":"SWOS-Expanded-Research/1.0"})
        with urllib.request.urlopen(req,timeout=45) as r,temp.open("wb") as f:
            total=0
            while True:
                b=r.read(1024*1024)
                if not b:break
                total+=len(b)
                if total>limit:raise ValueError("download size limit reached")
                f.write(b)
        if not zipfile.is_zipfile(temp):raise ValueError("response not a ZIP")
        temp.replace(target);return target,"DOWNLOADED"
    finally:
        if temp.exists():temp.unlink()

def catalogue(root,co):
    docs=root/"docs/competition_research"
    index=json.loads((docs/"INDEX.json").read_text(encoding="utf-8"))
    rows={}
    for path in sorted(set(index["rowPath"].values())):
        try:
            j=json.loads((docs/path).read_text(encoding="utf-8"))
            for r in j:
                if isinstance(r,dict) and "row" in r: rows[int(r["row"])]=r
        except Exception as exc:co.note("SWOS",path,"ERROR_MATRIX",exc)
    named={}
    for p in set(index.get("namedClosureRowPath",{}).values()):
        try:
            for r in json.loads((docs/p).read_text(encoding="utf-8"))["rows"]: named[int(r["row"])]=r
        except Exception as exc:co.note("SWOS",p,"ERROR_NAMED",exc)
    native=collections.defaultdict(list);uuidmap={}
    for world,folder in [("9697",root/"data_json/teams"),("2627",root/"data_json/worlds/2627/teams")]:
        for p in sorted(folder.glob("*.json")):
            c=country(p.stem.removesuffix("_2").removesuffix("_POOL"))
            if not c:continue # no continental copies, pool, custom or national-team files
            try:
                j=json.loads(p.read_text(encoding="utf-8"));f=int(j["country"])
                for entry in j.get("teams",[]):
                    t=entry.get("team",{});uid=entry.get("uuid")
                    if not uid or not t.get("team_name") or t.get("team_ordinal") is None:continue
                    rec=(world,c,t["team_name"],uid,f,int(t["team_ordinal"]),t.get("league"),str(p.relative_to(root)))
                    native[(world,c,norm(rec[2]))].append(rec);uuidmap[(world,uid)]=rec
                    co.db.execute("INSERT INTO native VALUES (?,?,?,?,?,?,?,?)",rec)
            except Exception as exc:co.note("SWOS",str(p),"ERROR_NATIVE",exc)
    # Only aliases already associated with the same native UUID in the current world.
    def walk(obj):
        if isinstance(obj,dict):
            uid=obj.get("uuid")
            label=obj.get("sourceLabel") or obj.get("sport") or obj.get("label")
            if uid and isinstance(label,str):
                for world in ("9697","2627"):
                    rec=uuidmap.get((world,uid))
                    if rec:native[(world,rec[1],norm(label))].append(rec)
            for val in obj.values():walk(val)
        elif isinstance(obj,list):
            if len(obj)>=3 and all(isinstance(x,str) for x in obj[:3]):
                for world in ("9697","2627"):
                    rec=uuidmap.get((world,obj[2]))
                    if rec:native[(world,rec[1],norm(obj[0]))].append(rec)
            for val in obj:walk(val)
    for p in (docs/"closure").glob("*.json"):
        try:walk(json.loads(p.read_text(encoding="utf-8")))
        except Exception as exc:co.note("SWOS",p.name,"ERROR_ALIAS",exc)
    co.db.commit()
    return rows,named,native

def candidates(co,rows,named,native):
    db=co.db
    db.execute("CREATE INDEX ix_group ON fixture(source,competition,country,season,season_basis)")
    db.execute("CREATE INDEX ix_date ON fixture(date)")
    db.commit()
    lookup=collections.defaultdict(set);tiers=collections.defaultdict(set)
    for rid,r in rows.items():
        naming=r.get("competitionNaming") or {}; world=str(r.get("world",""));c=str(r.get("country") or "INTERNATIONAL")
        labels=[naming.get("officialName"),naming.get("nonCommercialDisplayName"),naming.get("candidateName"),named.get(rid,{}).get("name")]
        for h in naming.get("nameHistory") or []:
            if isinstance(h,dict):labels.extend((h.get("previousName"),h.get("name")))
        for lab in labels:
            if lab:lookup[(c,norm(lab))].add(rid)
        m=re.fullmatch(r"div(\d+)",str(r.get("detail","")),re.I)
        if m and c:tiers[(c,int(m.group(1)))].add(rid)
    groups=[];joins=[];trans=collections.defaultdict(list);rowhits=collections.defaultdict(list)
    mapped=[];membership={}
    # Read-only suggestions: observed source labels are never treated as native club IDs.
    for source,comp,c,season,scheme,n,lo,hi,old,modern,missing, inferred in db.execute("""SELECT source,competition,country,season,season_basis,count(*),min(date),max(date),
         sum(date<'1996-07-01'),sum(date>='2026-07-01'),sum(hg IS NULL OR ag IS NULL),sum(date_basis!='EXPLICIT')
         FROM fixture GROUP BY 1,2,3,4,5"""):
        key=(source,comp,c,season,scheme)
        teams=set();pairs=collections.Counter();score_conf=0
        for h,a in db.execute("SELECT home,away FROM fixture WHERE source=? AND competition=? AND country=? AND season=? AND season_basis=?",key):
            teams.update((h,a));pairs[(norm(h),norm(a))]+=1
        nt=len(teams);balanced=nt>1 and len(pairs)==nt*(nt-1) and all(v==1 for v in pairs.values())
        hits=set(lookup.get((c or "INTERNATIONAL",norm(comp)),set()))
        # Exact competition alias inside parentheses (RSSSF headers), never fuzzy certification.
        for label in re.findall(r"\(([^()]+)\)",comp):
            hits.update(lookup.get((c or "INTERNATIONAL",norm(label)),set()))
        if not c:
            stripped=EDITION.sub("",comp).split("‹",1)[0].strip()
            historical={"championscup":"UEFA Champions League","championsleague":"UEFA Champions League",
                        "cupwinnerscup":"UEFA Cup Winners' Cup","uefacup":"UEFA Cup"}
            if norm(stripped) in historical:
                hits.update(lookup.get(("INTERNATIONAL",norm(historical[norm(stripped)])),set()))
        div=DIV.get(comp.upper())
        dm=re.fullmatch(r"([A-Z]{3}) DIV(\d+)",comp)
        if dm:div=(dm.group(1),int(dm.group(2)))
        if div:hits.update(tiers.get(div,set()))
        elif c:
            m=re.search(r"\b(first|second|third|fourth|fifth) division\b",comp,re.I)
            if m:hits.update(tiers.get((c,("first","second","third","fourth","fifth").index(m.group(1).lower())+1),set()))
        # Source groups may point to both worlds, never merge their native identities.
        for rid in sorted(hits):
            r=rows[rid];world=str(r.get("world",""))
            resolved=0
            for t in sorted(teams):
                records={x[3]:x for x in native.get((world,c,norm(t)),[])}
                status="UNIQUE_CANDIDATE" if len(records)==1 else ("AMBIGUOUS" if records else "UNMAPPED")
                rec=next(iter(records.values())) if len(records)==1 else None
                resolved+=bool(rec)
                # Only keep join suggestions for initial-edition date windows, not every historic season.
                if (world=="9697" and lo<"1998-01-01" and hi>="1996-01-01") or (world=="2627" and hi>="2026-01-01"):
                    joins.append((rid,world,*key,t,status,rec[2] if rec else "",rec[3] if rec else "",rec[4] if rec else "",rec[5] if rec else ""))
            rowhits[rid].append((key,n,nt,resolved))
            if (world=="9697" and hi>="1996-07-01" and lo<"1997-07-01") or (world=="2627" and hi>="2026-01-01"):
                mapped.append((rid,world,*key))
        groups.append((*key,n,nt,len(pairs),n-len(pairs),int(balanced),lo,hi,old,modern,missing,inferred,";".join(map(str,sorted(hits)))))
        trans[(source,comp,c,scheme)].append((season,n,nt,int(balanced),lo,hi))
        if balanced:membership[(source,comp,c,scheme,season)]=teams
    changes=[];movement=[]
    cutoff=[]
    for g in groups:
        source,comp,c,season,scheme,n,nt,pairs,dups,balanced,lo,hi,old,modern,missing,inferred,hits=g
        for world,day in (("9697","1996-07-01"),("2627",CUTOFF)):
            if lo<day<=hi:
                count,scored=db.execute("SELECT count(*),sum(hg IS NOT NULL AND ag IS NOT NULL) FROM fixture WHERE source=? AND competition=? AND country=? AND season=? AND season_basis=? AND date<?",(source,comp,c,season,scheme,day)).fetchone()
                cutoff.append((world,day,source,comp,c,season,scheme,count,scored or 0,hits,"ACTIVE_CANDIDATE_NOT_CERTIFIED"))
    csvout(co.out/"july1_candidates.csv",["world","cutoff","source","competition","country","season","season_basis","prior_fixture_occurrences","prior_score_occurrences","candidate_matrix_rows","status"],cutoff)
    for key,history in trans.items():
        history.sort()
        for a,b in zip(history,history[1:]):
            if a[1:4]!=b[1:4]:changes.append((*key,a[0],b[0],*a[1:4],*b[1:4],"OBSERVED_CHANGE_NOT_PERMANENT_RULE"))
            if a[0][:4].isdigit() and b[0][:4].isdigit() and int(b[0][:4])==int(a[0][:4])+1:
                prev=membership.get((*key,a[0]));cur=membership.get((*key,b[0]))
                if prev is not None and cur is not None:
                    old={norm(t):t for t in prev};new={norm(t):t for t in cur}
                    for t in sorted(old.keys()-new.keys()):movement.append((*key,a[0],b[0],"ABSENT_NEXT",old[t],"NOT_PROVEN_RELEGATION"))
                    for t in sorted(new.keys()-old.keys()):movement.append((*key,a[0],b[0],"NEW_NEXT",new[t],"NOT_PROVEN_PROMOTION"))
    csvout(co.out/"source_groups.csv",["source","competition","country","season","season_basis","rows","teams","unique_oriented_pairs","duplicate_pair_occurrences","double_round_robin_candidate","min_date","max_date","before_1996_07_01","on_after_2026_07_01","missing_or_suppressed_score","inferred_dates","candidate_matrix_rows"],groups)
    csvout(co.out/"native_join_candidates.csv",["row","world","source","competition","country","season","season_basis","label","status","native_name","uuid","file","ordinal"],joins)
    csvout(co.out/"observed_changes.csv",["source","competition","country","season_basis","before","after","old_matches","old_teams","old_balanced","new_matches","new_teams","new_balanced","status"],changes)
    csvout(co.out/"observed_membership_changes.csv",["source","competition","country","season_basis","before","after","direction","label","status"],movement)
    db.execute("CREATE TABLE candidate_group(matrix_row INTEGER,world TEXT,source TEXT,competition TEXT,country TEXT,season TEXT,season_basis TEXT)")
    db.executemany("INSERT INTO candidate_group VALUES (?,?,?,?,?,?,?)",mapped)
    db.create_function("labelkey",1,norm,deterministic=True)
    # Compare exact oriented labels and dates, only for candidate initial-edition groups.
    # Source independence, actual competition membership and aliases require human review.
    db.execute("""CREATE TEMP TABLE compared AS SELECT g.matrix_row,g.world,f.date,labelkey(f.home) home,labelkey(f.away) away,f.source,f.hg,f.ag
      FROM candidate_group g JOIN fixture f ON g.source=f.source AND g.competition=f.competition AND g.country=f.country AND g.season=f.season AND g.season_basis=f.season_basis
      WHERE f.date_basis='EXPLICIT' AND ((g.world='9697' AND f.date>='1996-07-01' AND f.date<'1997-07-01') OR (g.world='2627' AND f.date>='2026-01-01'))""")
    overlaps=db.execute("""SELECT matrix_row,world,date,home,away,count(DISTINCT source),group_concat(DISTINCT source),count(DISTINCT CASE WHEN hg IS NOT NULL AND ag IS NOT NULL THEN printf('%d:%d',hg,ag) END)
      FROM compared GROUP BY 1,2,3,4,5 HAVING count(DISTINCT source)>1""")
    csvout(co.out/"cross_source_exact_pairs.csv",["matrix_row","world","date","normalized_home","normalized_away","source_count","sources","known_score_variants"],overlaps)
    cross_count=db.execute("SELECT count(*) FROM (SELECT 1 FROM compared GROUP BY matrix_row,world,date,home,away HAVING count(DISTINCT source)>1)").fetchone()[0]
    db.commit()
    proposals=[]
    for rid,r in sorted(rows.items()):
        rr=named.get(rid,{});fs=rr.get("fields",{});states=[fs.get(k,{}).get("status","NOT_AUDITED") for k in FIELDS]
        hh=rowhits[rid]
        naming=r.get("competitionNaming") or {}
        proposals.append((rid,r.get("world"),r.get("country"),r.get("id"),r.get("detail"),naming.get("officialName") or rr.get("name") or naming.get("candidateName"),*states,rr.get("complete",""),rr.get("applicable",""),rr.get("closed",False),len(hh),sum(x[1] for x in hh),sum(x[2] for x in hh),sum(x[3] for x in hh),"REVIEW_REQUIRED"))
    proposals.sort(key=lambda p:(bool(p[17]),-(p[15] if isinstance(p[15],int) else 0),-p[18],p[0]))
    csvout(co.out/"matrix_candidates.csv",["row","world","country","id","detail","name",*FIELDS,"verified_complete","verified_applicable","previously_closed","candidate_groups","candidate_fixture_occurrences","candidate_team_occurrences","unique_join_occurrences","status"],proposals)
    # Conflicts within exactly identified source groups, using labels without speculative aliases.
    # Same fixture date + normalized source labels is not assumed equivalent across competitions.
    csvout(co.out/"same_source_score_conflicts.csv",["source","competition","country","date","home","away","variants"],db.execute("""SELECT source,competition,country,date,home,away,count(DISTINCT printf('%d:%d',hg,ag))
     FROM fixture WHERE hg IS NOT NULL AND ag IS NOT NULL GROUP BY 1,2,3,4,5,6 HAVING count(DISTINCT printf('%d:%d',hg,ag))>1"""))
    csvout(co.out/"rule_candidates.csv",["source","locator","competition","country","season","excerpt"],db.execute("SELECT * FROM rule"))
    return {"matrix_rows":len(rows),"named_verified_rows":len(named),"source_groups":len(groups),"mapped_matrix_rows":len(rowhits),"initial_join_candidates":len(joins),"observed_changes":len(changes),"observed_membership_changes":len(movement),"cross_source_exact_pairs":cross_count,"july1_candidates":len(cutoff),"rule_candidates":db.execute("SELECT count(*) FROM rule").fetchone()[0],"fixture_candidates":db.execute("SELECT count(*) FROM fixture").fetchone()[0],"verified_upgrades":0,"note":"All generated evidence requires review; no existing closure or game file changed."}

def selftest():
    assert iso("1996-07-01")=="1996-07-01"
    assert iso("31/02/1996")==""
    assert norm("Atlético Madrid")=="atleticomadrid"
    x=structured({"Division":"I1","MatchDate":"2026-07-01","HomeTeam":"A","AwayTeam":"B","FTHome":"2.0","FTAway":"1.0"})
    assert x[1]=="ITA" and x[8:10]==(None,None)
    y=structured({"competition":"Serie A","date":"1996-09-07","home":"A","away":"B","gh":2,"ga":1})
    assert y[8:10]==(2,1)
    assert structured({"date":"1996-09-07","home":"A"}) is None
    assert RESULT.fullmatch("Bologna 1 - 0 Lazio [Sep 7]")
    assert path_hints("E0")[:2]==("E0","ENG")
    assert structured({"Div":"I1","Date":"06/09/96","HomeTeam":"A","AwayTeam":"B","FTHG":2,"FTAG":1}, "I1")[2]=="1996/97"
    # Synthetic integration: separate source occurrences, exact cross-source pair,
    # native UUID suggestion, and no promotion of matrix closure.
    with tempfile.TemporaryDirectory() as td:
        co=Collector(Path(td))
        raw={"Div":"I1","Date":"06/09/96","HomeTeam":"A","AwayTeam":"B","FTHG":2,"FTAG":1}
        for source in ("s1","s2"):co.add(source,"test",structured(raw))
        co.flush()
        rows={1:{"row":1,"world":"9697","country":"ITA","detail":"div1","id":"test"}}
        native={("9697","ITA","a"):[("9697","ITA","A","uuid-a",20,0,0,"test")]}
        result=candidates(co,rows,{},native)
        assert result["fixture_candidates"]==2 and result["cross_source_exact_pairs"]==1
        assert result["verified_upgrades"]==0 and result["matrix_rows"]==1
        co.db.close()
    print("SELFTEST PASS (normalization, cutoff, SQLite candidate integration; not historical certification)")

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--self-test",action="store_true")
    ap.add_argument("--no-download",action="store_true")
    ap.add_argument("--refresh",action="store_true",help="Fetch fresh remote source ZIPs instead of cached ones")
    ap.add_argument("--max-download-mb",type=int,default=150)
    args=ap.parse_args();selftest()
    if args.self_test:return
    import pandas as pd
    sources=ROOT/"sources";cache=HERE/"cache";outbase=HERE/"output"
    for p in (sources,cache,outbase):p.mkdir(parents=True,exist_ok=True)
    stamp=dt.datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    out=outbase/("RICERCA_"+stamp);out.mkdir()
    co=Collector(out);manifest=[]
    order=[("corpus",("corpus",),(".zip",)),("parquet",("games",),(".parquet",)),
           ("matches",("matches",),(".txt",".csv")),("worldfootballR",("worldfootballr",),(".zip",)),
           ("euro9697",("all-euro-data-1996-1997",),(".xls",".xlsx")),
           ("euro2627",("all-euro-data-2026-2027",),(".xls",".xlsx"))]
    paths=[p for p in sources.rglob("*") if p.is_file()]
    for kind,prefixes,exts in order:
        chosen=sorted(p for p in paths if p.suffix.lower() in exts and any(p.name.lower().startswith(x) for x in prefixes))
        if not chosen:co.note(kind,"","MISSING_LOCAL_SOURCE");continue
        for p in chosen:
            source=kind+"/"+str(p.relative_to(sources)).replace("\\","/")
            print("Reading",source,flush=True)
            manifest.append({"source":source,"bytes":p.stat().st_size,"sha256":hashfile(p)})
            try:
                if p.suffix.lower()==".zip":co.archive(p,source,kind)
                elif kind=="parquet":
                    import pyarrow.parquet as pq
                    pf=pq.ParquetFile(p)
                    for k,b in enumerate(pf.iter_batches(batch_size=50000)):
                        co.table(b.to_pandas(),source,p.name+":batch"+str(k))
                elif kind=="matches":
                    with p.open("rb") as f:co.csvfile(f,source,p.name)
                else:
                    with pd.ExcelFile(p) as x:
                        for sheet in x.sheet_names:
                            comp,c,season=path_hints(sheet)
                            season="1996/97" if kind=="euro9697" else "2026/27"
                            co.table(x.parse(sheet),source,p.name+":"+sheet,comp,c,season)
            except Exception as exc:co.note(source,p.name,"ERROR",exc)
            co.flush()
    if not args.no_download:
        for repo,branch in REPOS:
            source="github/"+repo
            # Already uploaded parquet is the preferred source; remote repo still inventoried/downloaded.
            print("GitHub",repo,flush=True)
            try:
                if args.refresh:
                    old=cache/(repo.replace("/","__")+"-"+branch+".zip")
                    if old.exists(): old.unlink()
                p,status=download(repo,branch,cache,args.max_download_mb*1024*1024)
                manifest.append({"source":source,"branch":branch,"status":status,"bytes":p.stat().st_size,"sha256":hashfile(p)})
                co.archive(p,source,"github");co.flush()
            except Exception as exc:co.note(source,branch,"DOWNLOAD_OR_PARSE_ERROR",exc)
    else:co.note("github","","DISABLED_BY_ARGUMENT")
    print("SWOS identity and matrix reconciliation",flush=True)
    rows,named,native=catalogue(ROOT,co)
    co.flush()
    summary=candidates(co,rows,named,native)
    # Test negative invariant on actual output as well as the synthetic self-test.
    if co.db.execute("SELECT count(*) FROM fixture WHERE date>=? AND (hg IS NOT NULL OR ag IS NOT NULL)",(CUTOFF,)).fetchone()[0]:
        raise RuntimeError("cutoff score leakage")
    summary["source_counts"]=dict(co.counts);summary["cutoff"]=CUTOFF
    summary["research_index_sha256"]=hashfile(ROOT/"docs/competition_research/INDEX.json")
    summary["collector_sha256"]=hashfile(Path(__file__))
    summary["source_errors"]=sum("ERROR" in x["status"] for x in co.log)
    summary["missing_local_sources"]=sum(x["status"]=="MISSING_LOCAL_SOURCE" for x in co.log)
    dump(out/"summary.json",summary);dump(out/"source_manifest.json",manifest);dump(out/"source_log.json",co.log)
    co.db.close()
    archive=outbase/(out.name+".zip")
    with zipfile.ZipFile(archive,"w",zipfile.ZIP_DEFLATED,compresslevel=3) as z:
        for p in sorted(out.iterdir()):
            if p.is_file():z.write(p,p.name)
    print(json.dumps(summary,ensure_ascii=False,indent=2),flush=True)
    print("RESULT ZIP:",archive,flush=True)

if __name__=="__main__":
    try:
        main()
    except Exception:
        message=traceback.format_exc()
        print(message,file=sys.stderr,flush=True)
        out=HERE/"output";out.mkdir(parents=True,exist_ok=True)
        stamp=dt.datetime.now().strftime("%Y%m%d-%H%M%S-%f")
        with zipfile.ZipFile(out/("ERRORE_RICERCA_"+stamp+".zip"),"w",zipfile.ZIP_DEFLATED) as z:
            z.writestr("error.txt",message)
        sys.exit(1)
