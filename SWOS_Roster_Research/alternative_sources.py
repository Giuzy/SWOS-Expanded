"""Season-specific alternative club/squad candidates, never runtime data."""
import hashlib
import io
import json
import re
import tarfile
import tempfile
import unicodedata
import urllib.request
from pathlib import Path
from http_retry import fetch
from roster_storage import atomic, digest

URL="https://github.com/florentfougeres/OpenFootballBase-Data/releases/download/data-1.26.0/openfootballbase-data.tar.gz"
SHA="f5f9ce5d5efd3732a8e730c7354a5c4e94f19007e37130c2cb67739d0478aabe"
BAD=re.compile(r"\b(women|woman|female|ladies|feminine|feminin|femenin[oa]|femminile|frauen|damen|féminin|u[- ]?(?:17|18|19|20|21|23)|youth|reserves?|academy)\b",re.I)
CODES={"DEU":"GER","PRT":"POR","NLD":"NED","CHE":"SUI","GRC":"GRE","HRV":"CRO","DNK":"DEN","SVN":"SVN","SLV":"SLV","ZAF":"RSA","SAU":"KSA","KOR":"KOR","IRN":"IRN","TWN":"TPE","GBR":"UNMAPPED","ROU":"ROU","PHL":"PHI","IDN":"IDN","VNM":"VIE","MYS":"MAS","SGP":"SIN","MMR":"MYA","KHM":"CAM","BGD":"BAN","NPL":"NEP","BTN":"BHU","LKA":"SRI","MNG":"MNG","MDV":"MDV","ARE":"UAE","OMN":"OMA","BHR":"BHR","PSE":"PLE","DZA":"ALG","MAR":"MAR","TUN":"TUN","SDN":"SDN","TCD":"CHA","NER":"NIG","NGA":"NGA","GNB":"GNB","GNQ":"EQG","COG":"CGO","COD":"COD","ZWE":"ZIM","ZMB":"ZAM","TZA":"TAN","MWI":"MWI","MOZ":"MOZ","AGO":"ANG","BWA":"BOT","SWZ":"SWZ","LSO":"LES","MUS":"MRI","SYC":"SEY","CPV":"CPV","GMB":"GAM","GHA":"GHA","CIV":"CIV","BFA":"BFA","BEN":"BEN","TGO":"TOG","GIN":"GUI","SLE":"SLE","LBR":"LBR","SOM":"SOM","DJI":"DJI","ERI":"ERI","ETH":"ETH","KEN":"KEN","UGA":"UGA","RWA":"RWA","BDI":"BDI","CAF":"CTA","CMR":"CMR","GAB":"GAB","STP":"STP","MDG":"MAD","COM":"COM","AUS":"AUS","NZL":"NZL","FJI":"FIJ","SLB":"SOL","VUT":"VAN","NCL":"NCL","PYF":"TAH","COK":"COK","WSM":"SAM","ASM":"ASA","TON":"TGA","PNG":"PNG","CRI":"CRC","SLV":"SLV","GTM":"GUA","HND":"HON","NIC":"NCA","PAN":"PAN","HTI":"HAI","JAM":"JAM","TTO":"TRI","PRI":"PUR","DOM":"DOM","CUW":"CUW","ATG":"ATG","BRB":"BRB","DMA":"DMA","GRD":"GRN","KNA":"SKN","LCA":"LCA","VCT":"VIN","BHS":"BAH","BMU":"BER","CYM":"CAY","VGB":"VGB","VIR":"VIR","AIA":"AIA","ABW":"ARU","BLZ":"BLZ","GUY":"GUY","SUR":"SUR","MEX":"MEX","CAN":"CAN","USA":"USA","URY":"URU","PRY":"PAR","CHL":"CHI","COL":"COL","ECU":"ECU","BOL":"BOL","PER":"PER","VEN":"VEN","ARG":"ARG","BRA":"BRA"}
def norm(s):
    return "".join(c for c in unicodedata.normalize("NFKD",str(s)).casefold() if c.isalnum())
def code(value,allowed):
    # Exact known FIFA code wins. Explicit translations only.
    return value if value in allowed else CODES.get(value) if CODES.get(value) in allowed else None

def archive(cache):
    p=cache/"openfootballbase-data-1.26.0.tar.gz"
    if not p.exists():
        data=fetch(urllib.request.Request(URL,headers={"User-Agent":"SWOS-Expanded-Research/1.2"}),timeout=120,limit=80*1024*1024)
        if hashlib.sha256(data).hexdigest()!=SHA:raise ValueError("OpenFootballBase archive checksum mismatch")
        tmp=p.with_suffix(".part");tmp.write_bytes(data);tmp.replace(p)
    if hashlib.sha256(p.read_bytes()).hexdigest()!=SHA:raise ValueError("Cached OpenFootballBase checksum mismatch")
    return p

def datasets(tar,folder):
    # Read only explicitly manifest-listed files, no extractall and no path traversal.
    members={}
    for m in tar.getmembers():
        if m.isfile():
            name=m.name.removeprefix("./")
            if name in members:raise ValueError("Duplicate archive member")
            members[name]=m
    manifests=[k for k in members if k.rsplit("/",1)[-1]=="manifest.json"]
    if len(manifests)!=1:raise ValueError("Manifest identity mismatch")
    mp=manifests[0];prefix=mp[:-len("manifest.json")]
    manifest=json.load(tar.extractfile(members[mp]))
    if manifest.get("format")!=1:raise ValueError("Unsupported manifest format")
    def extract(spec):
        rel=spec["path"]
        if not re.fullmatch(r"[A-Za-z0-9_./-]+\.parquet",rel) or ".." in rel.split("/"):raise ValueError("Invalid manifest path")
        item=members[prefix+rel]
        if item.size>128*1024*1024:raise ValueError("Oversize parquet")
        data=tar.extractfile(item).read()
        if hashlib.sha256(data).hexdigest()!=spec["sha256"]:raise ValueError("Manifest file checksum mismatch")
        p=folder/(spec["sha256"]+".parquet");p.write_bytes(data)
        return p,spec["rows"]
    tables={k:extract(manifest["tables"][k]) for k in ("club","competition")}
    for season in ("2026","2026/27"):
        spec=manifest.get("seasons",{}).get(season,{}).get("files",{})
        for k in ("club_season","squad"):
            if k in spec:tables[season,k]=extract(spec[k])
    return tables

def add_unique(cp,owner,field,row):
    seq=cp.ensure(owner,field,[])
    if row not in seq:cp.append(seq,row)

MALE_LEAGUES={"ENG-PL","ENG-CH","ESP-LL","ESP-SD","ITA-SA","ITA-SB","DEU-BL","DEU-B2","FRA-L1","FRA-L2","BEL-PL","PRT-PL","NLD-ED","TUR-SL","USA-MLS","SAU-PL","BRA-SA","ARG-PD"}
MALE_LABELS={"men","male","men's association football","men's football","football masculin","football association masculin","association football masculin","masculin","hommes","männlicher fußball"}
def scoped(country,club,category,competition,level,season,allowed,competition_code=None):
    c=code(country,allowed)
    if not c or not club or not competition or BAD.search(str(club)) or BAD.search(str(competition)) or BAD.search(str(category or "")):return None
    # OFB men's league catalogue is explicit in its published coverage; require
    # positive male club category, domestic league level, exact target season.
    label=str(category or "").strip().casefold()
    if label not in MALE_LABELS and not (not label and competition_code in MALE_LEAGUES):return None
    if type(level)!=int or level not in range(1,5) or season not in ("2026","2026/27"):return None
    return c

def acquire(results,cp,cache,root,known):
    import duckdb
    allowed={x["fifa"] for x in json.loads((root/"docs/NATIONAL_ROSTER_SOURCE_MAP_2627.json").read_text(encoding="utf-8-sig"))["records"]}
    targets={t["fifa"]:t for t in results["targets"]}
    gaps={}
    for uid,e in results["existing_club_gaps"].items():
        if uid in known and e.get("association"):
            gaps.setdefault((e["association"],norm(e["name"])),[]).append((uid,e))
    with tarfile.open(archive(cache),"r:gz") as tar,tempfile.TemporaryDirectory() as td:
        tables=datasets(tar,Path(td))
        db=duckdb.connect()
        try:
            def load(k):
                p,n=tables[k]
                cursor=db.execute("SELECT * FROM read_parquet(?)",[str(p)])
                cols=[x[0] for x in cursor.description];rows=[dict(zip(cols,r)) for r in cursor.fetchall()]
                if len(rows)!=n:raise ValueError("Manifest row count mismatch")
                return rows
            clubs={x["wikipedia_title"]:x for x in load("club") if x.get("wikipedia_title")}
            competitions={x["code"]:x for x in load("competition")}
            count=0
            for season in ("2026","2026/27"):
                if (season,"club_season") not in tables:continue
                eligible={}
                for row in load((season,"club_season")):
                    cl=clubs.get(row["club"],{});comp=competitions.get(row["competition"],{})
                    if row.get("season")!=season or not cl or not comp:continue
                    c=scoped(comp.get("country_code"),row["club"],cl.get("category"),comp.get("name_en",""),comp.get("level"),season,allowed,row["competition"])
                    if not c or code(cl.get("country_code"),allowed)!=c:continue
                    q=cl.get("wikidata_qid")
                    if not isinstance(q,str) or not re.fullmatch(r"Q[1-9][0-9]*",q):continue
                    entry={"club":row["club"],"clubQid":q,"country":c,"competition":row["competition"],"season":season,"source":"OpenFootballBase data-1.26.0","scope":"SEASON_CANDIDATE_NOT_JULY1"}
                    owners=[]
                    if c in targets and comp["level"]==1:owners.append(targets[c])
                    matches=gaps.get((c,norm(row["club"])),[])
                    if len(matches)==1:owners.append(matches[0][1])
                    if not owners:continue
                    for owner in owners:add_unique(cp,owner,"alternative_clubs",entry)
                    eligible[(row["club"],row["competition"])]=(entry,owners,matches[0][0] if len(matches)==1 else None)
                if (season,"squad") not in tables:continue
                for row in load((season,"squad")):
                    match=eligible.get((row.get("club"),row.get("competition")))
                    if not match or row.get("season")!=season:continue
                    entry,owners,uid=match
                    q=row.get("player_qid")
                    if not isinstance(q,str) or not re.fullmatch(r"Q[1-9][0-9]*",q) or not row.get("player") or not row.get("source_revid"):continue
                    if row.get("stint")=="loan_out":continue
                    if uid and norm(row["player"]) in known[uid]:continue
                    observation=dict(entry,player=q,playerLabel=row["player"],source_title=row.get("source_title"),source_revid=row["source_revid"],stint=row.get("stint"),stint_date=row.get("stint_date"))
                    for owner in owners:add_unique(cp,owner,"alternative_players",observation)
                    count+=1
                cp.save()
            print("Alternative season player observations:",count,flush=True)
        finally:db.close()

def selftest():
    allowed={"GER"}
    assert scoped("DEU","A FC","men","Bundesliga",1,"2026/27",allowed)=="GER"
    assert scoped("DEU","A Women","men","Bundesliga",1,"2026/27",allowed) is None
    assert scoped("DEU","A FC","","Bundesliga",1,"2026/27",allowed) is None
    assert scoped("DEU","A FC","","Bundesliga",1,"2026/27",allowed,"DEU-BL")=="GER"
    assert scoped("DEU","A FC","women","Bundesliga",1,"2026/27",allowed,"DEU-BL") is None
    assert scoped("DEU","A FC","men","Bundesliga",1,"2025/26",allowed) is None
