"""Discover domestic league/cup identities and current entrants for missing countries.
Derived research only; no native IDs, no CE writes, no future results.
"""
import json,re,urllib.error
from roster_storage import atomic,read
from research_status import report
from transfermarkt_source import (Transfermarkt,BASE,COUNTRY,TIER,NON_LEAGUE,BAD,
    country_page_matches,links,season_label,table_links,parse)
FIELDS=("name","identity","participants","format","regulation","movement","qualification",
        "july1","roundDates","competitionDays","evolution","seasonChange")
def routes(page,country_id):
    if not country_page_matches(page,country_id):raise ValueError("Country identity mismatch")
    out={}
    for table in page.all("table"):
        kind=None
        for tr in table.all("tr"):
            txt=tr.text()
            if txt=="First Tier":kind="LEAGUE_DIVISION";continue
            if txt=="Domestic Cup":kind="NATIONAL_CUP";continue
            if txt in TIER or txt in ("Super Cup","League Cup","Other competitions") or re.search(r"\b(women|youth|reserve|super cup|international)\b",txt,re.I):
                kind=None;continue
            if not kind:continue
            for code,name in links(tr,r"/startseite/wettbewerb/([A-Z0-9]+)(?:/|$)").items():
                if kind=="LEAGUE_DIVISION" and NON_LEAGUE.search(name):continue
                if kind=="NATIONAL_CUP" and re.search(r"\b(super\s*cup|league cup|play[- ]?off|qualification)\b",name,re.I):continue
                if code in out and out[code]!=(kind,name):raise ValueError("Ambiguous competition route "+code)
                out[code]=(kind,name)
    return out
def blocks_source(exc):
    return isinstance(exc,PermissionError) or (isinstance(exc,urllib.error.HTTPError) and exc.code in (401,403,429))

def result_status(candidates,seen,errors,added):
    if not seen:
        if errors:return "ERROR"
        return "NO DATA" if candidates else "NO ADAPTER"
    if errors or len(seen)<2:return "PARTIAL"
    return "ACQUIRED" if added else "NO NEW DATA"

def run(root,only=None,tm_factory=Transfermarkt):
    national=json.loads((root/"docs/NATIONAL_ROSTER_SOURCE_MAP_2627.json").read_text(encoding="utf-8-sig"))["records"]
    targets=[r for r in national if r["source"]["team_file"]==255 and (not only or r["fifa"]==only)]
    targets.sort(key=lambda r:(r["expanded"]["globalNumber"]<2560,r["fifa"]))
    tm=tm_factory(root/"tools/competition_research/cache",None,{"errors":[]},national,{})
    output=root/"SWOS_Roster_Research/results_2627/competition_candidates"
    for r in targets:
        country=r["fifa"];label="Domestic competitions "+country
        if tm.blocked:
            report(label,"NOT SEARCHED","source access blocked");continue
        path=output/(country+".json")
        old=read(path) if path.exists() else {"schema":"SWOS_DOMESTIC_COMPETITION_CANDIDATES_1","country":country,"observations":[]}
        if old.get("country")!=country or old.get("schema")!="SWOS_DOMESTIC_COMPETITION_CANDIDATES_1":raise ValueError("Competition candidate file identity mismatch")
        obs=old["observations"];before=len(obs);errors=[];seen=set()
        report(label,"SEARCHING","first tier and domestic cup")
        try:
            cid=tm.country_id(country)
            country_path=COUNTRY+cid+"/saison_id/2026"
            candidates=routes(tm.page(country_path),cid)
            for code,(kind,name) in sorted(candidates.items()):
                if tm.blocked:break
                item=label+"/"+code
                report(item,"SEARCHING",name)
                try:
                    route="/-/startseite/wettbewerb/"+code+"/saison_id/2026"
                    page=tm.page(route)
                    season=season_label(page,"clubs",name,code)
                    seen.add(kind)
                    packet={"source":"Transfermarkt","country":country,"kind":kind,"sourceId":code,"name":name,
                            "season":season,"sourceUrl":BASE+route,"countryEvidence":BASE+country_path,
                            "fields":{f:"I" for f in FIELDS},"scope":"SEASON_CANDIDATE_NOT_JULY1"}
                    packet["fields"].update(name="P",identity="P")
                    try:
                        clubs=table_links(page,r"/startseite/verein/([1-9][0-9]*)(?:/|$)")
                        packet["participants"]=[{"sourceId":k,"name":v} for k,v in sorted(clubs.items())]
                        packet["fields"]["participants"]="P"
                    except ValueError as e:
                        errors.append([code,"participants",str(e)])
                    if packet not in obs:obs.append(packet)
                    atomic(path,old)
                    report(item,"FOUND",f"{season}; {len(packet.get('participants',[]))} observed participants; remaining fields unresolved")
                except Exception as e:
                    errors.append([code,str(e)]);report(item,"ERROR",str(e))
                    if blocks_source(e):tm.blocked=True
            status={k:"FOUND" if k in seen else "UNKNOWN" for k in ("LEAGUE_DIVISION","NATIONAL_CUP")}
            old["lastSearch"]={"status":status,"errors":errors,"complete":not tm.blocked}
            atomic(path,old)
            report(label,result_status(candidates,seen,errors,len(obs)-before),f"{len(obs)-before} new observations; {len(candidates)} source routes; league {status['LEAGUE_DIVISION']}; cup {status['NATIONAL_CUP']}")
        except Exception as e:
            if blocks_source(e):tm.blocked=True
            errors.append(["country",str(e)])
            old["lastSearch"]={"status":{k:"FOUND" if k in seen else "UNKNOWN" for k in ("LEAGUE_DIVISION","NATIONAL_CUP")},"errors":errors,"complete":False}
            atomic(path,old)
            report(label,"ERROR",str(e))
    return len(targets)
def selftest():
    p=parse('<select><option value="22" selected>Bhutan</option></select><table><tr><td>First Tier</td></tr><tr><td><a href="/x/startseite/wettbewerb/BH1L">Premier League</a></td></tr><tr><td>Domestic Cup</td></tr><tr><td><a href="/x/startseite/wettbewerb/BHC">National Cup</a></td></tr><tr><td>Super Cup</td></tr><tr><td><a href="/x/startseite/wettbewerb/BHS">Super Cup</a></td></tr></table>')
    assert routes(p,"22")=={"BH1L":("LEAGUE_DIVISION","Premier League"),"BHC":("NATIONAL_CUP","National Cup")}
    try:routes(p,"99")
    except ValueError:pass
    else:raise AssertionError("Wrong country accepted")

    assert not blocks_source(ValueError("Requested season not evidenced or ambiguous: ['25/26']"))
    assert blocks_source(PermissionError("blocked"))
    assert blocks_source(urllib.error.HTTPError("https://example.org",429,"rate limit",{},None))
    assert not blocks_source(urllib.error.HTTPError("https://example.org",404,"not found",{},None))
    assert result_status({},set(),[],0)=="NO ADAPTER"
    assert result_status({"x":1},set(),[["x","wrong season"]],0)=="ERROR"
    assert result_status({"x":1},{"LEAGUE_DIVISION"},[],1)=="PARTIAL"
    assert result_status({"x":1},{"LEAGUE_DIVISION","NATIONAL_CUP"},[],0)=="NO NEW DATA"
    # Regression: a valid edition, then a wrong season, then another country.
    # Real parser, real atomic publication, no requests.
    from tempfile import TemporaryDirectory
    from pathlib import Path
    def country_html(cid,codes):
        return parse('<link rel="canonical" href="'+COUNTRY+cid+'"><table><tr><td>First Tier</td></tr>'+''.join('<tr><td><a href="/x/startseite/wettbewerb/'+code+'">'+name+'</a></td></tr>' for code,name in codes)+'</table>')
    def comp_html(code,name,year):
        return parse('<link rel="canonical" href="/x/startseite/wettbewerb/'+code+'"><h2>Clubs - '+name+' '+year+'</h2><table class="items"><tr><th>Club</th><th>Squad</th></tr><tr><td><a href="/x/startseite/verein/1">Club One</a></td><td>16</td></tr></table>')
    class FakeTM:
        def __init__(self,*args):self.blocked=False
        def country_id(self,c):return {"BLZ":"11","BRB":"12"}[c]
        def page(self,path):
            if path==COUNTRY+"11/saison_id/2026":return country_html("11",[("BZ1A","Apertura"),("BZ1C","Clausura")])
            if path==COUNTRY+"12/saison_id/2026":return country_html("12",[("BB1","Premier League")])
            for code,name,year in (("BZ1A","Apertura","2026/27"),("BZ1C","Clausura","2025/26"),("BB1","Premier League","2026")):
                if path=="/-/startseite/wettbewerb/"+code+"/saison_id/2026":return comp_html(code,name,year)
            raise AssertionError("Unexpected test URL "+path)
    with TemporaryDirectory() as tmp:
        root=Path(tmp);docs=root/"docs";docs.mkdir()
        atomic(docs/"NATIONAL_ROSTER_SOURCE_MAP_2627.json",{"records":[{"fifa":c,"source":{"team_file":255},"expanded":{"globalNumber":2560}} for c in ("BLZ","BRB")]})
        assert run(root,tm_factory=FakeTM)==2
        out=root/"SWOS_Roster_Research/results_2627/competition_candidates"
        bz=read(out/"BLZ.json");bb=read(out/"BRB.json")
        assert [x["sourceId"] for x in bz["observations"]]==["BZ1A"]
        assert bz["observations"][0]["participants"]==[{"sourceId":"1","name":"Club One"}]
        assert bz["lastSearch"]["errors"][0][0]=="BZ1C"
        assert bz["lastSearch"]["complete"]
        assert [x["sourceId"] for x in bb["observations"]]==["BB1"]
        assert run(root,tm_factory=FakeTM)==2
        assert read(out/"BLZ.json")==bz and read(out/"BRB.json")==bb
