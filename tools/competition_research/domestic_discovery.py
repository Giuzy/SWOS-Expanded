"""Discover domestic league/cup identities and current entrants for missing countries.
Derived research only; no native IDs, no CE writes, no future results.
"""
import json,re,urllib.error
from concurrent.futures import ThreadPoolExecutor
from roster_storage import atomic,read
from research_status import report, progress
from http_retry import time_budget, check_budget, DeadlineExceeded, HostCooldown
from transfermarkt_source import (Transfermarkt,BASE,COUNTRY,TIER,NON_LEAGUE,BAD,
    country_page_matches,links,season_label,table_links,parse,edition_page)
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

def catalogue_rule_fields(obs,packet,refresh):
    related=[p for p in obs if p.get("kind")==packet["kind"] and (p.get("wikidataId")==packet.get("wikidataId") or (p.get("source")=="Wikidata" and p.get("sourceId")==packet.get("wikidataId")))]
    return {f for f in ("format","regulation","movement","qualification","roundDates","evolution") if not any(p.get("fields",{}).get(f) in ("V","NA") for p in related) and (refresh or not any(x.get("field")==f and x.get("facts") for p in related for x in p.get("researchSectionLeads",[])))}

def catalogue_jobs(targets,catalogue,output,year,refresh):
    """One country ahead, one catalogue worker, main thread owns all candidate writes."""
    if catalogue is None:
        for r in progress("Domestic competition countries",targets,overall=True):yield r,None
        return
    def acquire(r):
        country=r["fifa"];path=output/(country+".json")
        old=read(path) if path.exists() else {"observations":[]}
        with time_budget(40):
            return catalogue.country(country,year,rule_fields=lambda packet:catalogue_rule_fields(old["observations"],packet,refresh))
    with ThreadPoolExecutor(max_workers=1) as pool:
        job=pool.submit(acquire,targets[0]) if targets else None
        for i,r in enumerate(progress("Domestic competition countries",targets,overall=True)):
            current=job
            job=pool.submit(acquire,targets[i+1]) if i+1<len(targets) else None
            yield r,current

def run(root,only=None,tm_factory=Transfermarkt,wiki_query=None,year=2026,source_catalog=False,refresh=False):
    national=json.loads((root/"docs/NATIONAL_ROSTER_SOURCE_MAP_2627.json").read_text(encoding="utf-8-sig"))["records"]
    targets=[r for r in national if (year!=2026 or r["source"]["team_file"]==255) and (not only or r["fifa"]==only)]
    targets.sort(key=lambda r:(r["expanded"]["globalNumber"]<2560,r["fifa"]))
    from season_config import output_dir,edition_label
    tm=tm_factory(root/"tools/competition_research/cache",None,{"errors":[]},national,{}) if year==2026 else tm_factory(root/"tools/competition_research/cache",None,{"errors":[]},national,{},year=year)
    output=output_dir(root,year)/"competition_candidates"
    catalogue=None
    if source_catalog:
        from domestic_catalog import Catalogue, observation_key
        catalogue=Catalogue(output_dir(root,year)/"sources"/"competition_domestic_catalog",national,refresh=refresh)
    from competition_sources import Cached, Discovery
    country_names={r["source"].get("name",""):r["fifa"] for r in national if r["source"].get("name")}
    archive=Discovery(Cached(root/"tools/competition_research/cache/domestic_archive",refresh),country_names) if source_catalog and year==2026 else None
    def observation_count():
        return sum(len(read(p).get("observations",[])) for p in (output/(r["fifa"]+".json") for r in targets) if p.exists())
    initial_count=observation_count()
    counts={"handled":0,"successful":0,"failed":0,"deferred":0,"with_errors":0}
    for r,catalogue_job in catalogue_jobs(targets,catalogue,output,year,refresh):
        with time_budget(160):
            country=r["fifa"];label="Domestic competitions "+country
            if tm.blocked and wiki_query is None and catalogue is None:
                counts["handled"]+=1;counts["deferred"]+=1
                report(label,"NOT SEARCHED","source access blocked");continue
            path=output/(country+".json")
            old=read(path) if path.exists() else {"schema":"SWOS_DOMESTIC_COMPETITION_CANDIDATES_1","country":country,"observations":[]}
            if old.get("country")!=country or old.get("schema")!="SWOS_DOMESTIC_COMPETITION_CANDIDATES_1":raise ValueError("Competition candidate file identity mismatch")
            obs=old["observations"];before=len(obs);errors=[];seen=set();incomplete=False
            report(label,"SEARCHING","first tier and domestic cup")
            seeds=[];covered={p["kind"] for p in obs if p.get("kind") in ("LEAGUE_DIVISION","NATIONAL_CUP") and p.get("participants") and p.get("fields",{}).get("participants")=="V"};seen.update(p["kind"] for p in obs if p.get("kind") in ("LEAGUE_DIVISION","NATIONAL_CUP") and p.get("fields",{}).get("identity") in ("P","V"));review=False
            from sports_sources import source_leads
            existing_leads=old.setdefault("externalSportsSourceLeads",[])
            for lead in source_leads(country,year):
                if lead not in existing_leads:existing_leads.append(lead)
            if archive is not None:
                try:
                    with time_budget(10):
                        leads=archive.leads({"world":"2627","country":country})
                    merged={p["url"]:p for p in old.get("archiveSourceLeads",[])}
                    for lead in leads:merged.setdefault(lead["url"],lead)
                    if merged:old["archiveSourceLeads"]=list(merged.values());atomic(path,old)
                except Exception as e:errors.append(["RSSSF archive discovery",str(e)]);incomplete=True
            if catalogue is not None:
                try:
                    packets,seeds,source_errors=catalogue_job.result()
                    if source_errors:errors.extend(source_errors);incomplete=True
                    keys={observation_key(p) for p in obs if p.get("source") in ("WikipediaDirectory","RSSSF","BetExplorer","GlobalSportsArchive")}
                    for p in packets:
                        packet=dict(p,fields={f:"I" for f in FIELDS})
                        packet["fields"]["name"]="P"
                        if p.get("identityEvidence"):
                            packet["fields"]["identity"]="P";seen.add(p["kind"])
                            if p.get("participants"):
                                packet["fields"]["participants"]="P";covered.add(p["kind"])
                        else:review=True
                        key=observation_key(packet)
                        if key not in keys:obs.append(packet);keys.add(key);atomic(path,old)
                    # Directory identity alone does not close participant gaps.
                except Exception as e:
                    incomplete=True;errors.append(["Domestic catalogue",str(e)]);report(label+"/Catalogue","ERROR",str(e))
            if wiki_query is not None:
                from wiki_sources import domestic_query,qid,valid_club_name
                from collect_names import clubs_query
                try:
                    with time_budget(35):
                        report(label+"/Wikidata","SEARCHING","missing competition identities; maximum 35s for Wiki phase")
                        needed={"LEAGUE_DIVISION","NATIONAL_CUP"}-covered
                        # Save all identity leads before any expensive edition lookup.
                        discovered=list(seeds)
                        if needed:
                            try:discovered.extend(wiki_query(domestic_query(country,year,needed)))
                            except Exception as e:
                                incomplete=True;errors.append(["Wikidata",str(e)])
                                report(label+"/Wikidata","ERROR",str(e))
                        edition_jobs={}
                        for r in discovered:
                            lid=qid(r.get("league"));kind=r.get("kind");name=r.get("leagueLabel","")
                            if not lid or kind not in ("LEAGUE_DIVISION","NATIONAL_CUP") or not valid_club_name(name):continue
                            if kind=="NATIONAL_CUP" and re.search(r"\b(super\s*cup|league cup|play[- ]?off|qualification)\b",name,re.I):continue
                            packet={"source":"Wikidata","country":country,"kind":kind,"sourceId":lid,"name":name,
                                    "sourceUrl":"https://www.wikidata.org/wiki/"+lid,"scope":"COMPETITION_IDENTITY_NOT_EDITION",
                                    "fields":{f:"I" for f in FIELDS}}
                            packet["fields"].update(name="P",identity="P")
                            if r.get("source")=="WikipediaDirectory":packet["identityEvidenceSource"]=r["sourceUrl"]
                            if packet not in obs:obs.append(packet);atomic(path,old)
                            seen.add(kind)
                            if any(p.get("source")=="Wikidata" and p.get("sourceId")==lid and p.get("participants") for p in obs):continue
                            edition_jobs.setdefault(lid,(r,packet))
                        for lid,(r,packet) in edition_jobs.items():
                            try:
                                editions={}
                                report(label+"/Wikidata/"+lid,"SEARCHING","edition and participants")
                                for row in wiki_query(clubs_query([r["league"]])):
                                    check_budget()
                                    sid=qid(row.get("season"));club=qid(row.get("club"));sl=row.get("seasonLabel","")
                                    season=edition_label(sl,year)
                                    if qid(row.get("league"))!=lid or not sid or not season or not valid_club_name(sl):continue
                                    if club and valid_club_name(row.get("clubLabel","")):
                                        editions.setdefault((sid,season),{})[club]=row["clubLabel"]
                                for (sid,season),clubs in editions.items():
                                    edition=dict(packet,season=season,editionId=sid,scope="SEASON_CANDIDATE_NOT_JULY1",
                                                 sourceUrl="https://www.wikidata.org/wiki/"+sid,
                                                 participants=[{"sourceId":k,"name":v} for k,v in sorted(clubs.items())],
                                                 fields=dict(packet["fields"],participants="P"))
                                    if edition not in obs:obs.append(edition);atomic(path,old)
                            except (DeadlineExceeded,HostCooldown):raise
                            except Exception as e:incomplete=True;errors.append(["Wikidata",lid,str(e)]);report(label+"/Wikidata","ERROR",str(e))
                except Exception as e:incomplete=True;errors.append(["Wikidata",str(e)]);report(label+"/Wikidata","ERROR",str(e))
            if tm.blocked:
                old["lastSearch"]={"status":{k:"FOUND" if k in seen else "UNKNOWN" for k in ("LEAGUE_DIVISION","NATIONAL_CUP")},"errors":errors+[["Transfermarkt","blocked"]],"complete":False}
                atomic(path,old)
                counts["handled"]+=1;counts["deferred"]+=1;counts["with_errors"]+=1
                report(label,"PARTIAL" if seen else "NOT SEARCHED","Transfermarkt blocked; retry later")
                continue
            try:
                check_budget()
                report(label+"/Transfermarkt","SEARCHING","country and domestic competition routes")
                cid=tm.country_id(country)
                country_path=COUNTRY+cid+"/saison_id/"+str(year)
                candidates=routes(tm.page(country_path),cid)
                for code,(kind,name) in progress("Domestic competition routes "+country,sorted(candidates.items())):
                    check_budget()
                    if tm.blocked:break
                    item=label+"/"+code
                    if not refresh and any(p.get("source")=="Transfermarkt" and p.get("sourceId")==code and p.get("kind")==kind and p.get("participants") for p in obs):
                        seen.add(kind);report(item,"SKIPPED","existing season participant observation");continue
                    report(item,"SEARCHING",name)
                    try:
                        route="/-/startseite/wettbewerb/"+code+"/saison_id/"+str(year)
                        page,season,route=edition_page(tm,route,"clubs",name,code,year)
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
                        if isinstance(e,(DeadlineExceeded,HostCooldown,urllib.error.URLError,TimeoutError)):
                            incomplete=True
                            if isinstance(e,(DeadlineExceeded,HostCooldown)):break
                status={k:"FOUND" if k in seen else "UNKNOWN" for k in ("LEAGUE_DIVISION","NATIONAL_CUP")}
                old["lastSearch"]={"status":status,"errors":errors,"complete":not tm.blocked and not incomplete}
                atomic(path,old)
                print(f"DIAG domestic | country={country} previous={before} current={len(obs)} new={len(obs)-before} errors={len(errors)} incomplete={incomplete}",flush=True)
                counts["handled"]+=1
                counts["with_errors"]+=int(bool(errors))
                counts["deferred" if incomplete or tm.blocked else "failed" if errors else "successful"]+=1
                report(label,("PARTIAL" if seen else "ERROR") if incomplete else ("REVIEW" if review and not seen and not candidates else result_status(candidates,seen,errors,len(obs)-before)),f"{len(obs)-before} new observations; {len(candidates)} source routes; league {status['LEAGUE_DIVISION']}; cup {status['NATIONAL_CUP']}")
            except Exception as e:
                if blocks_source(e):tm.blocked=True
                errors.append(["country",str(e)])
                old["lastSearch"]={"status":{k:"FOUND" if k in seen else "UNKNOWN" for k in ("LEAGUE_DIVISION","NATIONAL_CUP")},"errors":errors,"complete":False}
                atomic(path,old)
                counts["handled"]+=1;counts["with_errors"]+=1
                counts["deferred" if isinstance(e,(DeadlineExceeded,HostCooldown,TimeoutError)) or tm.blocked else "failed"]+=1
                report(label,"ERROR",str(e))
    print("RESULT domestic source work | "+" | ".join(f"{k}={v}" for k,v in counts.items()),flush=True)
    print(f"RESULT domestic competitions | new candidate observations={observation_count()-initial_count} | countries handled={len(targets)} | not certified competitions",flush=True)
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
                if code=="BZ1C" and path=="/-/startseite/wettbewerb/"+code+"/saison_id/2025":return comp_html(code,name,year)
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
        assert "Requested season not evidenced" in bz["lastSearch"]["errors"][0][1]
        assert bz["lastSearch"]["complete"]
        assert [x["sourceId"] for x in bb["observations"]]==["BB1"]
        assert run(root,tm_factory=FakeTM)==2
        assert read(out/"BLZ.json")==bz and read(out/"BRB.json")==bb
        # Timeout in Wiki must not erase earlier evidence or stop TM/next country.
        wiki_calls=[]
        def failed_wiki(q):
            wiki_calls.append(q)
            raise DeadlineExceeded("synthetic Wiki timeout")
        assert run(root,tm_factory=FakeTM,wiki_query=failed_wiki)==2
        assert len(wiki_calls)==2
        for country,old_packet in (("BLZ",bz),("BRB",bb)):
            packet=read(out/(country+".json"))
            assert packet["observations"]==old_packet["observations"]
            assert not packet["lastSearch"]["complete"]
            assert packet["lastSearch"]["errors"][0]==["Wikidata","synthetic Wiki timeout"]
        class TimeoutTM(FakeTM):
            def page(self,path):
                if path==COUNTRY+"11/saison_id/2026":raise DeadlineExceeded("synthetic TM timeout")
                return super().page(path)
        assert run(root,tm_factory=TimeoutTM)==2
        assert read(out/"BLZ.json")["observations"]==bz["observations"]
        assert not read(out/"BLZ.json")["lastSearch"]["complete"]
        assert read(out/"BRB.json")==bb
