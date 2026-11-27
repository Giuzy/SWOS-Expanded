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
from research_status import report, outcome

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

def observation_key(field,row):
    # IDs are source-specific; seasonal candidates and July memberships
    # are not interchangeable. Ignore only presentation/provenance changes.
    if field in ("alternative_clubs","alternative_players"):
        identity=(row.get("clubQid"),row.get("country"),row.get("competition"),row.get("season"))
        if all(identity):
            if field=="alternative_clubs":return ("ofb",)+identity
            if row.get("player"):return ("ofb",)+identity+(row["player"],row.get("stint"),row.get("stint_date"))
    if field in ("transfermarkt_clubs","transfermarkt_players"):
        identity=(row.get("transfermarktClubId"),row.get("country"),row.get("competition"),row.get("season"),row.get("level"))
        if all(identity):
            if field=="transfermarkt_clubs":return ("tm",)+identity
            if row.get("transfermarktPlayerId"):return ("tm",)+identity+(row["transfermarktPlayerId"],)
    return None

def player_name_scope(row):
    # No fuzzy surname, initials, global or cross-edition name matching.
    label=norm(row.get("playerLabel") or "")
    club=norm(row.get("club") or "")
    country=row.get("country");season=row.get("season")
    return (country,season,club,label) if all((country,season,club,label)) else None

def add_unique(cp,owner,field,row):
    seq=owner.get(field,[])
    key=observation_key(field,row)
    if any(old==row or (key is not None and observation_key(field,old)==key) for old in seq):return False
    other={"alternative_players":"transfermarkt_players","transfermarkt_players":"alternative_players"}.get(field)
    scope=player_name_scope(row) if other else None
    if scope:
        matches=[old for old in owner.get(other,[]) if player_name_scope(old)==scope]
        # Same displayed name in the same club/season is one import
        # candidate until reviewed; preserve the other source ID as a
        # compact unresolved match, not as a second automatic EXTRA.
        # Distinct IDs from the same source may be real homonyms.
        if len(matches)==1 and not any(player_name_scope(old)==scope for old in seq):
            review={"status":"NAME_MATCH_REVIEW","field":field,"name":row["playerLabel"],
                    "country":row["country"],"season":row["season"],"club":row["club"],
                    "candidate":list(key) if key else None,
                    "existing":list(observation_key(other,matches[0]) or ()),
                    "source":row.get("source"),"source_title":row.get("source_title")}
            reviews=cp.ensure(owner,"name_matches",[])
            if review not in reviews:cp.append(reviews,review)
            return False
    cp.append(cp.ensure(owner,field,[]),row)
    return True

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

def acquire(results,cp,cache,root,known,phase):
    if phase not in ('clubs','players','extras'):raise ValueError('Unknown research phase')
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
            from collections import Counter
            found=Counter();new=Counter();review=Counter()
            countries=set(targets) if phase!="extras" else {c for c,_ in gaps}
            label="OpenFootballBase "+phase
            required=("club_season",) if phase=="clubs" else ("club_season","squad")
            available=[s for s in ("2026","2026/27") if all((s,k) in tables for k in required)]
            if not available:
                report(label,"NO ADAPTER","archive has no required 2026/27 edition tables");return
            report(label,"SEARCHING",f"{len(countries)} countries; editions {available}")
            for season in available:
                eligible={}
                last_country=None
                def club_country(row):
                    return code(competitions.get(row.get("competition"),{}).get("country_code"),allowed) or ""
                for row in sorted(load((season,"club_season")),key=club_country):
                    cl=clubs.get(row["club"],{});comp=competitions.get(row["competition"],{})
                    if row.get("season")!=season or not cl or not comp:continue
                    c=scoped(comp.get("country_code"),row["club"],cl.get("category"),comp.get("name_en",""),comp.get("level"),season,allowed,row["competition"])
                    if not c or code(cl.get("country_code"),allowed)!=c:continue
                    q=cl.get("wikidata_qid")
                    if not isinstance(q,str) or not re.fullmatch(r"Q[1-9][0-9]*",q):continue
                    entry={"club":row["club"],"clubQid":q,"country":c,"competition":row["competition"],"season":season,"source":"OpenFootballBase data-1.26.0","scope":"SEASON_CANDIDATE_NOT_JULY1"}
                    owners=[]
                    if phase != "extras" and c in targets and comp["level"]==1:owners.append(targets[c])
                    matches=gaps.get((c,norm(row["club"])),[])
                    if phase == "extras" and len(matches)==1:owners.append(matches[0][1])
                    if not owners:continue
                    if last_country is not None and c!=last_country:cp.save()
                    last_country=c
                    if phase in ("clubs","extras"):
                        for owner in owners:
                            added=add_unique(cp,owner,"alternative_clubs",entry)
                            if phase=="clubs":
                                count+=int(added);found[c]+=1;new[c]+=int(added)
                    eligible[(row["club"],row["competition"])]=(entry,owners,matches[0][0] if phase=="extras" and len(matches)==1 else None)
                cp.save()
                if phase=="clubs":continue
                if (season,"squad") not in tables:continue
                last_country=None
                def squad_country(row):
                    match=eligible.get((row.get("club"),row.get("competition")))
                    return match[0]["country"] if match else ""
                for row in sorted(load((season,"squad")),key=squad_country):
                    match=eligible.get((row.get("club"),row.get("competition")))
                    if not match or row.get("season")!=season:continue
                    entry,owners,uid=match
                    q=row.get("player_qid")
                    if not isinstance(q,str) or not re.fullmatch(r"Q[1-9][0-9]*",q) or not row.get("player") or not row.get("source_revid"):continue
                    if row.get("stint")=="loan_out":continue
                    if uid and norm(row["player"]) in known[uid]:continue
                    c=entry["country"]
                    if last_country is not None and c!=last_country:cp.save()
                    last_country=c
                    observation=dict(entry,player=q,playerLabel=row["player"],source_title=row.get("source_title"),source_revid=row["source_revid"],stint=row.get("stint"),stint_date=row.get("stint_date"))
                    for owner in owners:
                        c=entry["country"];found[c]+=1;before=len(owner.get("name_matches",[]))
                        added=int(add_unique(cp,owner,"alternative_players",observation))
                        count+=added;new[c]+=added;review[c]+=len(owner.get("name_matches",[]))-before
                cp.save()
            for c in sorted(countries):report("OpenFootballBase "+phase+" "+c,*outcome(found[c],new[c],review[c]))
            report(label,"ACQUIRED" if count else "NO NEW DATA" if sum(found.values()) else "NO DATA",f"{count} new observations; {sum(found.values())} eligible; {sum(review.values())} reviews; editions {available}")
        finally:db.close()

def selftest():
    a={'clubQid':'Q1','country':'BRA','competition':'BRA-SA','season':'2026','player':'Q2','playerLabel':'A','source_revid':1}
    assert observation_key('alternative_players',a)==observation_key('alternative_players',dict(a,playerLabel='Alias',source_revid=2))
    assert observation_key('alternative_players',a)!=observation_key('alternative_players',dict(a,season='2026/27'))
    assert observation_key('alternative_players',a)!=observation_key('alternative_players',dict(a,stint='loan_in'))
    assert player_name_scope(dict(a,club="São Paulo",playerLabel="João Silva"))==player_name_scope(dict(a,club="Sao Paulo",playerLabel="Joao Silva"))
    assert player_name_scope(dict(a,club="A",playerLabel="João Silva"))!=player_name_scope(dict(a,club="B",playerLabel="Joao Silva"))
    assert player_name_scope(dict(a,club="A",playerLabel="João Silva"))!=player_name_scope(dict(a,club="A",playerLabel="João Silva",season="2026/27"))
    allowed={"GER"}
    assert scoped("DEU","A FC","men","Bundesliga",1,"2026/27",allowed)=="GER"
    assert scoped("DEU","A Women","men","Bundesliga",1,"2026/27",allowed) is None
    assert scoped("DEU","A FC","","Bundesliga",1,"2026/27",allowed) is None
    assert scoped("DEU","A FC","","Bundesliga",1,"2026/27",allowed,"DEU-BL")=="GER"
    assert scoped("DEU","A FC","women","Bundesliga",1,"2026/27",allowed,"DEU-BL") is None
    assert scoped("DEU","A FC","men","Bundesliga",1,"2025/26",allowed) is None
