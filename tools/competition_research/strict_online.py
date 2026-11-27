#!/usr/bin/env python3
"""Acquire exact men's competition sources into the canonical research matrix, not runtime."""
import argparse, collections, datetime as dt, hashlib, json, os, re, tempfile, urllib.request, sys
from pathlib import Path
from matrix_storage import read_matrix, plan_matrix
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/"SWOS_Roster_Research"))
from http_retry import fetch as request_bytes
from competition_sources import Cached, Discovery, fd_route, fd_packet, wanted as gap_fields, selftest as sources_selftest
from research import COUNTRIES
FIELDS=("name","identity","participants","format","regulation","movement","qualification","july1","roundDates","competitionDays","evolution","seasonChange")
ROUTES=[
 (186,"ITA","div1","2026-27/it.1.json","Italian Serie A 2026/27",20),
 (217,"BRA","div1","2026/br.1.json","Brasileiro Série A 2026",20),
 (231,"ENG","div1","2026-27/en.1.json","English Premier League 2026/27",20),
 (232,"ENG","div2","2026-27/en.2.json","English Championship 2026/27",24),
 (235,"ESP","div1","2026-27/es.1.json","Spain Primera División 2026/27",20),
 (239,"FRA","div1","2026-27/fr.1.json","French Ligue 1 2026/27",18),
 (243,"GER","div1","2026-27/de.1.json","Deutsche Bundesliga 2026/27",18),
 (275,"NED","div1","2026-27/nl.1.json","Netherlands Eredivisie 2026/27",18),
 (286,"POR","div1","2026-27/pt.1.json","Portuguese Primeira Liga 2026/27",18)]
BASE="https://raw.githubusercontent.com/openfootball/football.json/master/"
BAD=re.compile(r"\b(women|woman|female|ladies|feminine|femenin[oa]|femminile|frauen|damen|feminin|u[- ]?(?:17|18|19|20|21|23)|youth|reserve[s]?)\b",re.I)
def read(p):return json.loads(p.read_text(encoding="utf-8-sig"))
def encode(x):return (json.dumps(x,ensure_ascii=False,indent=2)+"\n").encode()
def states(row):
    s=dict(row.get("fields",{}));s.update(row.get("operationalFields",{}));return s
def missing(row,k):return states(row).get(k,"I")=="I"
def fetch(path):
    req=urllib.request.Request(BASE+path,headers={"User-Agent":"SWOS-Expanded/1.0"})
    return request_bytes(req,timeout=45,limit=4194304)
def validate(raw,route):
    _,_,_,path,title,n=route
    if len(raw)>4194304:raise ValueError("Oversize")
    data=json.loads(raw)
    if data.get("name")!=title or BAD.search(title):raise ValueError("Wrong competition/edition/gender")
    games=data["matches"]
    if not isinstance(games,list) or len(games)!=n*(n-1):raise ValueError("Incomplete round-robin")
    pairs=set();clubs=set();days=set();rounds=collections.defaultdict(list);pre=[]
    cut=dt.date(2026,7,1)
    lo,hi=(dt.date(2026,1,1),dt.date(2027,1,1)) if path.startswith("2026/") else (cut,dt.date(2027,7,1))
    for g in games:
        a,b=g["team1"],g["team2"];day=dt.date.fromisoformat(g["date"])
        r=re.fullmatch(r"Matchday ([1-9][0-9]*)",g["round"])
        if not a or not b or a==b or BAD.search(a) or BAD.search(b) or not r or not lo<=day<hi or (a,b) in pairs or (day,a) in days or (day,b) in days:raise ValueError("Scope/date/pair collision")
        pairs.add((a,b));clubs.update((a,b));days.update(((day,a),(day,b)));rounds[int(r[1])].append((day,a,b))
        if day<cut:
            score=g.get("score")
            ft=score if isinstance(score,list) else score.get("ft") if isinstance(score,dict) else None
            if not isinstance(ft,list) or len(ft)!=2 or any(type(x)!=int or x<0 for x in ft):raise ValueError("Missing pre-cutoff score")
            pre.append([day.isoformat(),a,b,*ft])
    if len(clubs)!=n or pairs!={(a,b) for a in clubs for b in clubs if a!=b} or set(rounds)!=set(range(1,2*n-1)):raise ValueError("Incomplete participants/rounds")
    if any(len(g)!=n//2 or len({t for _,a,b in g for t in (a,b)})!=n for g in rounds.values()):raise ValueError("Incomplete round membership")
    return {"participants":sorted(clubs),"format":{"teams":n,"cycles":2,"rounds":len(rounds)},
       "roundDates":[{"round":r,"first":min(x[0] for x in g).isoformat(),"last":max(x[0] for x in g).isoformat()} for r,g in sorted(rounds.items())],"july1":sorted(pre)}
def apply(root,download=fetch,refresh=False,only_country=None,only_world=None):
    docs=root/"docs/competition_research";index=read(docs/"INDEX.json")
    mp=docs/index["canonicalFieldMatrix"];cp=docs/"COVERAGE.json";matrix=read_matrix(mp);coverage=read(cp)
    rows={r["row"]:r for r in matrix["rows"]}
    if len(rows)!=len(matrix["rows"]):raise ValueError("Duplicate row")
    changes={};errors=[];inserted=0
    cache=Cached(root/"tools/competition_research/cache/strict_online",refresh)
    discovery=Discovery(cache,COUNTRIES)
    allrows=matrix["rows"]+matrix.get("ceOnlyEntries",[])
    selected=[r for r in allrows if (not only_country or (r.get("technicalIdentity",{}).get("country") or r.get("country"))==only_country) and (not only_world or r["world"]==only_world)]
    selected_ids={r["row"] for r in selected if "row" in r}
    gaps={k:sum(k in gap_fields(r,FIELDS) for r in selected) for k in FIELDS}
    print("Matrix scan",len(selected),"competitions; gaps",gaps,flush=True)
    scanned=0;lead_count=0;fd_routes=0
    searched=collections.defaultdict(set)
    attempted=collections.defaultdict(set)
    no_source=0
    def stage(p,obj):
        if p not in changes:changes[p]=[p.read_bytes(),obj]
        else:changes[p][1]=obj
    for route in ROUTES:
        rid,country,tier,path,title,n=route
        row=rows.get(rid)
        if row is None or rid not in selected_ids:continue
        ident=row.get("technicalIdentity",{})
        if (row.get("world"),ident.get("country"),ident.get("detail"),ident.get("category"))!=("2627",country,tier,"LEAGUE_DIVISION"):
            errors.append([rid,"identity mismatch"]);continue
        wanted=[k for k in gap_fields(row,("participants","format","roundDates","july1")) if k not in row.get("strictAcquisitionFields",[])]
        if not wanted:continue
        attempted[rid].update(wanted)
        try:
            print("Download",rid,title,flush=True)
            raw=cache.get(BASE+path) if download is fetch else download(path);ev=validate(raw,route)
            searched[rid].update(wanted)
            rel=index["rowPath"][str(rid)];sheet=(docs/rel).resolve()
            if not sheet.is_relative_to(docs.resolve()) or sheet.suffix!=".json":raise ValueError("Bad path")
            records=changes[sheet][1] if sheet in changes else read(sheet)
            matches=[r for r in records if r.get("row")==rid]
            if len(matches)!=1:raise ValueError("Sheet row mismatch")
            rec=matches[0]
            if (rec.get("world"),rec.get("country"),rec.get("detail"),rec.get("id"))!=("2627",country,tier,ident.get("id")):raise ValueError("Sheet identity mismatch")
            packet={k:ev[k] for k in wanted if ev[k]}
            if not packet:continue
            key=hashlib.sha256(raw).hexdigest()
            target=rec.setdefault("automaticResearch",{}).setdefault("strictAcquisition",{})
            if key not in target:
                target[key]={"document":title,"fields":packet,"scope":"P evidence only: native IDs, nominal dates, complete cutoff and rules require review; no post-start scores"}
                stage(sheet,records);inserted+=1
            row["strictAcquisitionFields"]=sorted(set(row.get("strictAcquisitionFields",[]))|set(packet))
            row.setdefault("evidenceRefs",{})["strictAcquisition"]=rel+"#row"+str(rid)
        except Exception as e:errors.append([rid,str(e)])
    for row in selected:
        fields=gap_fields(row,FIELDS)
        # Legacy calendar field is also a gap, but never infer round numbers.
        if row.get("fields",{}).get("calendar") in ("P","I"):fields=list(dict.fromkeys(fields+["calendar"]))
        if not fields:continue
        scanned+=1
        ident=row.get("technicalIdentity",{})
        label=row.get("name") or str(row.get("researchKey") or row.get("row"))
        print("Check",row.get("row",row.get("researchKey")),row["world"],label,"missing",",".join(fields),flush=True)
        # Store source leads only in the canonical row, never promote V/P/I.
        leads=discovery.leads(row)
        if leads:
            old=row.get("strictSourceLeads",[])
            merged={x["url"]:x for x in old}
            for lead in leads:
                if lead["url"] not in merged:
                    merged[lead["url"]]=lead;lead_count+=1
            row["strictSourceLeads"]=list(merged.values())
        route=fd_route(row)
        if not route or "row" not in row:
            if not leads and row.get("row") not in attempted:no_source+=1
            continue
        target_fields=[k for k in fields if k in ("participants","format","calendar","competitionDays")]
        if not target_fields:continue
        attempted[row["row"]].update(target_fields)
        url,code=route;fd_routes+=1
        try:
            rid=row["row"]
            raw=cache.get(url);packet=fd_packet(raw,row,code)
            searched[rid].update(target_fields)
            wanted={k:v for k,v in packet.items() if k in target_fields}
            if not wanted:continue
            rel=index["rowPath"][str(rid)];sheet=(docs/rel).resolve()
            if not sheet.is_relative_to(docs.resolve()) or sheet.suffix!=".json":raise ValueError("Bad path")
            records=changes[sheet][1] if sheet in changes else read(sheet)
            matches=[r for r in records if r.get("row")==rid]
            if len(matches)!=1:raise ValueError("Sheet row mismatch")
            rec=matches[0]
            if (rec.get("world"),rec.get("country"),rec.get("detail"),rec.get("id"))!=(row["world"],ident.get("country"),ident.get("detail"),ident.get("id")):raise ValueError("Sheet identity mismatch")
            key="football-data:"+hashlib.sha256(encode(wanted)).hexdigest()
            target=rec.setdefault("automaticResearch",{}).setdefault("strictAcquisition",{})
            if key not in target:
                target[key]={"document":url,"fields":wanted,"scope":"P evidence only: observed dates and clubs, no scores, no certified rules"}
                stage(sheet,records);inserted+=1
            row["strictAcquisitionFields"]=sorted(set(row.get("strictAcquisitionFields",[]))|set(wanted))
            row.setdefault("evidenceRefs",{})["strictAcquisition"]=rel+"#row"+str(rid)
        except Exception as e:errors.append([row.get("row"),"Football-Data: "+str(e)])
    for row in selected:
        s=states(row);partial=set(row.get("strictAcquisitionFields",[]))
        rid=row.get("row",row.get("researchKey"))
        row["acquisitionAvailability"]={k:("AVAILABLE" if s.get(k)=="V" else "NA" if s.get(k)=="NA" else "PARTIAL" if s.get(k)=="P" or k in partial else "UNKNOWN" if s.get(k,"?")=="?" else "NO DATA" if k in searched[rid] else "SOURCE_ERROR" if k in attempted[rid] else "NOT_SEARCHED") for k in FIELDS}
    matrix["strictAcquisitionSummary"]={"routes":len(ROUTES),"footballDataRoutes":fd_routes,"rowsScanned":scanned,"inventorySelected":len(selected),"gapCounts":gaps,"newObservations":inserted,"newSourceLeads":lead_count,"rowsWithoutRouteOrLead":no_source,"july1ApplicabilityUnknown":sum(states(r).get("july1","?")=="?" for r in selected),"errors":errors,"archiveIndexErrors":discovery.errors,"scope":"OpenFootball exact routes, Football-Data observed league CSV, RSSSF archive leads only; NO DATA is not absence; UNKNOWN is unreviewed; SOURCE_ERROR and NOT_SEARCHED distinguish missing adapter from a successful search. RSSSF links are not field evidence. No automatic status promotion."}
    coverage["strictAcquisitionSummary"]=matrix["strictAcquisitionSummary"]
    stage(cp,coverage)
    pending={p:(old,encode(obj)) for p,(old,obj) in changes.items() if json.loads(old)!=obj}
    pending.update({p:(p.read_bytes(),data) for p,data in plan_matrix(mp,matrix).items()})
    staged={};written=[]
    try:
        for p,(old,new) in pending.items():
            with tempfile.NamedTemporaryFile(dir=p.parent,delete=False) as f:f.write(new);staged[p]=Path(f.name)
        for p,(old,_) in pending.items():
            if p.read_bytes()!=old:raise RuntimeError("Concurrent matrix change")
        for p,t in staged.items():os.replace(t,p);written.append(p)
    except BaseException:
        for p in reversed(written):
            with tempfile.NamedTemporaryFile(dir=p.parent,delete=False) as f:f.write(pending[p][0]);t=Path(f.name)
            os.replace(t,p)
        raise
    finally:
        for t in staged.values():t.unlink(missing_ok=True)
    print("Scanned",scanned,"/",len(selected),"rows; new source leads",lead_count,"rows without route/lead",no_source,flush=True)
    print("Observations",inserted,"errors",len(errors),"archive index errors",len(discovery.errors),"files",len(pending),flush=True)
def selftest():
    from matrix_storage import selftest as matrix_selftest
    matrix_selftest()
    sources_selftest()
    r=(1,"ENG","div1","2026-27/x.json","Men League 2026/27",2)
    a={"name":r[4],"matches":[{"round":"Matchday 1","date":"2026-08-01","team1":"A","team2":"B","score":{"ft":[9,0]}},{"round":"Matchday 2","date":"2026-08-08","team1":"B","team2":"A"}]}
    assert validate(encode(a),r)["july1"]==[]
    br=(1,"BRA","div1","2026/x.json","Men League 2026",2)
    b={"name":br[4],"matches":[dict(a["matches"][0],date="2026-03-01",score=[0,0]),dict(a["matches"][1],date="2026-08-08")]}
    assert validate(encode(b),br)["july1"]==[["2026-03-01","A","B",0,0]]
    for bad in (dict(a,name="Women League 2026/27"),dict(a,name="Men League 2025/26"),dict(a,matches=a["matches"][:1])):
        try:validate(encode(bad),r)
        except ValueError:pass
        else:raise AssertionError("Wrong scope accepted")
    assert not missing({"fields":{"format":"V"}},"format")
    assert not missing({"fields":{"july1":"?"}},"july1")
def main():
    a=argparse.ArgumentParser();a.add_argument("--self-test",action="store_true");a.add_argument("--repo-root",type=Path,default=ROOT);a.add_argument("--refresh",action="store_true");a.add_argument("--country");a.add_argument("--world",choices=("9697","2627"))
    x=a.parse_args();selftest()
    if x.country:x.country=x.country.upper()
    if x.self_test:print("Self-test PASS");return
    apply(x.repo_root.resolve(),refresh=x.refresh,only_country=x.country,only_world=x.world)
if __name__=="__main__":main()
