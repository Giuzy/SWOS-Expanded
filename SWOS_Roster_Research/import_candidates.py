"""Read-only nominal import preparation. Not a TEAM/CE writer."""
import argparse, re, json, unicodedata
from pathlib import Path
from roster_storage import load_snapshot, atomic
from research_status import report
from alternative_sources import BAD, norm
from transfermarkt_source import NON_LEAGUE

from player_roles import merge_positions, projection
from season_config import season_map, edition_label, cutoff
def qid(v):
    m=re.fullmatch(r"(?:https?://www.wikidata.org/entity/)?(Q[1-9][0-9]*)",str(v or ""))
    return m[1] if m else None
def valid_name(v):
    return isinstance(v,str) and bool(v.strip()) and not BAD.search(v) and not re.fullmatch(r"(?:TEST|RESERVE)(?:\s+\d+)?",v.strip(),re.I)
def tm_id(v):
    return str(v) if re.fullmatch(r"[1-9][0-9]*",str(v)) else None
def archive_club_candidates(target,year=2026):
    country=target["fifa"];found={}
    for p in target.get("domestic_source_observations",[]):
        if p.get("country")!=country or p.get("kind")!="LEAGUE_DIVISION" or p.get("source") not in ("RSSSF","BetExplorer","GlobalSportsArchive") or p.get("scope")!="SEASON_PARTICIPANT_LABELS_NOT_JULY1" or p.get("identityEvidence") is not True or not season_map(year).get(p.get("season")):continue
        for x in p.get("participants",[]):
            name=x.get("name")
            if not valid_name(name) and not (country=="BHU" and name=="BFF Academy FC"):continue
            key=norm(name)
            if not key:continue
            row=found.setdefault(key,{"name":name,"scope":"ARCHIVE_NAME_REVIEW_NOT_NATIVE_ID","evidence":[]})
            ev={"source":p["source"],"url":p.get("sourceUrl"),"season":p["season"],"competition":p.get("name"),"sourceLabel":name}
            if x.get("sourceId"):ev["sourceId"]=x["sourceId"]
            if ev not in row["evidence"]:row["evidence"].append(ev)
    return [found[k] for k in sorted(found)]

def needs_club_sources(target,year=2026):
    # One source candidate is not proof the league is complete. Names are
    # comparison leads only; they are not merged with source-specific IDs.
    known=build(target,year)["clubs"]
    observed=archive_club_candidates(target,year)
    return not known or any(norm(x["name"]) not in {norm(c["name"]) for c in known} for x in observed)

def build(target,year=2026):
    SEASONS=season_map(year)
    country=target["fifa"]; clubs={}; warnings=[]; leagues={}
    def add(source,cid,name,season,competition,level,proof=None):
        season=SEASONS.get(season)
        if not cid or not valid_name(name) or not season or not competition:return None
        key=(source,cid)
        club=clubs.setdefault(key,{"source":source,"sourceId":cid,"name":name,"memberships":[],"players":{},"issues":[]})
        if norm(club["name"])!=norm(name):club["issues"].append("CLUB_NAME_CONFLICT")
        membership={"season":season,"competitionId":competition,"level":level}
        if proof:membership["evidence"]=proof
        if membership not in club["memberships"]:club["memberships"].append(membership)
        k=(source,competition,season)
        leagues.setdefault(k,{"source":source,"sourceId":competition,"season":season,"level":level,"clubs":set()})["clubs"].add(cid)
        return club
    def player(club,source,pid,name,season,competition,positions=None):
        if club is None or not pid or not valid_name(name) or season not in SEASONS:return
        rec=club["players"].setdefault(pid,{"source":source,"sourceId":pid,"name":name,"memberships":[]})
        if norm(rec["name"])!=norm(name):club["issues"].append("PLAYER_NAME_CONFLICT:"+pid)
        if positions:rec["positionObservations"]=merge_positions(rec.get("positionObservations"),[dict(p,observedMembershipSeason=SEASONS[season],observedCompetitionId=competition) for p in positions])
        m={"season":SEASONS[season],"competitionId":competition}
        if m not in rec["memberships"]:rec["memberships"].append(m)
    # Preserve competition/edition membership; identity is not a phase-specific club.
    for source,cf,pf,ck,pk,idfn,pfn in (
        ("TM","transfermarkt_clubs","transfermarkt_players","transfermarktClubId","transfermarktPlayerId",tm_id,tm_id),
        ("OFB","alternative_clubs","alternative_players","clubQid","player",qid,qid)):
        for row in target.get(cf,[]):
            if row.get("country")!=country or row.get("scope")!="SEASON_CANDIDATE_NOT_JULY1":continue
            if source=="TM" and (row.get("level")!=1 or NON_LEAGUE.search(str(row.get("competitionName","")))):continue
            add(source,idfn(row.get(ck)),row.get("club"),row.get("season"),row.get("competition"),row.get("level",1),row.get("source"))
        for row in target.get(pf,[]):
            if row.get("country")!=country or row.get("scope")!="SEASON_CANDIDATE_NOT_JULY1":continue
            club=clubs.get((source,idfn(row.get(ck))))
            if not club or not any(m["season"]==SEASONS.get(row.get("season")) and m["competitionId"]==row.get("competition") for m in club["memberships"]):
                warnings.append("UNBOUND_PLAYER:"+source+":"+str(row.get(pk)));continue
            player(club,source,pfn(row.get(pk)),row.get("playerLabel"),row.get("season"),row.get("competition"),row.get("positionObservations"))
    # Wikidata carries explicitly dated memberships, not the same roster authority.
    for league in target.get("leagues",[]):
        lid=qid(league.get("league"))
        if not lid or not valid_name(league.get("leagueLabel")) or NON_LEAGUE.search(league["leagueLabel"]):continue
        for edition in league.get("seasons",[]):
            label=str(edition.get("seasonLabel",""))
            season=edition_label(label,year)
            if not season:continue
            for entry in edition.get("clubs",[]):
                club=add("WD",qid(entry.get("club")),entry.get("name"),season,lid,1,qid(edition.get("season")))
                for p in entry.get("players",[]):player(club,"WD",qid(p.get("player")),p.get("playerLabel"),season,lid,p.get("positionObservations"))
    # Current Wikipedia rosters are surfaced separately, never counted as a
    # verified 2026/27 membership or used to promote roster readiness.
    wiki_leads=[]
    for row in target.get("wiki_club_leads",[]):
        if row.get("country")==country and qid(row.get("club")) and valid_name(row.get("clubLabel")):
            wiki_leads.append(row)
    wiki_rows=[r for r in target.get("wiki_squad_players",[]) if r.get("country")==country and r.get("scope")=="CURRENT_SQUAD_NOT_JULY1_OR_EDITION" and valid_name(r.get("playerLabel"))]
    for (source,cid),club in clubs.items():
        club["wikiCurrentRoster"]=[r for r in wiki_rows if r.get("clubSource")==source and r.get("clubSourceId")==cid]
        club["wikiCurrentCount"]=len({norm(r["playerLabel"]) for r in club["wikiCurrentRoster"]})
        club["wikidataUndatedRoster"]=[]
        if source=="WD":
            for league in target.get("leagues",[]):
                for edition in league.get("seasons",[]):
                    for entry in edition.get("clubs",[]):
                        if qid(entry.get("club"))==cid:
                            for row in entry.get("wikidata_undated_players",[]):
                                if row not in club["wikidataUndatedRoster"]:club["wikidataUndatedRoster"].append(row)
    groups={}
    for key,club in clubs.items():groups.setdefault(norm(club["name"]),[]).append(key)
    for keys in groups.values():
        if len(keys)>1:
            for k in keys:clubs[k]["issues"].append("CROSS_SOURCE_OR_HOMONYM_REVIEW")
    result=[]
    for k,c in sorted(clubs.items()):
        people=c.pop("players");names={}
        for pid,p in people.items():names.setdefault(norm(p["name"]),[]).append(pid)
        if any(len(ids)>1 for ids in names.values()):c["issues"].append("PLAYER_HOMONYM_REVIEW")
        for p in people.values():p["positionCandidate"]=projection(p.get("positionObservations",[]))
        c["players"]=sorted(people.values(),key=lambda p:p["sourceId"])
        c["issues"]=sorted(set(c["issues"]))
        c["nominalCount"]=len(people)
        c["nominalBySeason"]={season:sum(any(m["season"]==season for m in p["memberships"]) for p in people.values()) for season in sorted({m["season"] for m in c["memberships"]})}
        c["status"]="IDENTITY_REVIEW" if c["issues"] else "NOMINAL_ROSTER_16_PLUS" if max(c["nominalBySeason"].values(),default=0)>=16 else "ROSTER_INCOMPLETE"
        result.append(c)
    return {"schema":"SWOS_NOMINAL_IMPORT_CANDIDATES_1","country":country,"newCountry":bool(target.get("new")),
            "researchYear":year,"scope":"Season candidates only; not July 1 membership; no native IDs, skill values, official entrants or runtime approval",
            "clubs":result,"competitions":[dict(v,clubs=sorted(v["clubs"])) for k,v in sorted(leagues.items())],
            "warnings":sorted(set(warnings)),"nameMatches":target.get("name_matches",[]),
            "wikiClubIdentityLeads":wiki_leads,"wikiCurrentSquadObservations":wiki_rows,
            "domesticSourceObservations":target.get("domestic_source_observations",[]),
            "archiveClubNameCandidates":archive_club_candidates(target,year)}
def extras_for_club(uid,owner,native,year=2026):
    SEASONS=season_map(year)
    country=owner.get("association")
    if not country or not native or norm(owner.get("name",""))!=norm(native["name"]):
        return {"clubKey":uid,"name":owner.get("name"),"status":"IDENTITY_REVIEW","groups":[]}
    CUTOFF=cutoff(year)
    candidates={}
    for field,source,idfield,idfn in (("players","WD","player",qid),("alternative_players","OFB","player",qid),("transfermarkt_players","TM","transfermarktPlayerId",tm_id)):
        for row in owner.get(field,[]):
            label=row.get("playerLabel");pid=idfn(row.get(idfield))
            if not pid or not valid_name(label) or norm(label) in native["players"]:continue
            if source=="WD":
                ids={qid(x) for x in owner.get("identity_candidates",[])}-{None}
                if len(ids)!=1 or qid(row.get("club")) not in ids:continue
                if not row.get("start") or row["start"]>CUTOFF or (row.get("end") and row["end"]<=CUTOFF):continue
                season="JULY1_MEMBERSHIP_CANDIDATE"
            else:
                season=SEASONS.get(row.get("season"))
                if not season or row.get("country")!=country or row.get("scope")!="SEASON_CANDIDATE_NOT_JULY1":continue
                cf="transfermarkt_clubs" if source=="TM" else "alternative_clubs"
                ck="transfermarktClubId" if source=="TM" else "clubQid"
                matching=[v for v in owner.get(cf,[]) if v.get("country")==country and SEASONS.get(v.get("season"))==season and norm(v.get("club",""))==norm(native["name"])]
                ids={str(v.get(ck)) for v in matching if v.get(ck)}
                if len(ids)!=1 or str(row.get(ck)) not in ids:continue
                if not any(v.get("competition")==row.get("competition") for v in matching):continue
            obs={"source":source,"sourceId":pid,"season":season,"name":label,"evidence":row.get("source") or row.get("membership"),"competitionId":row.get("competition")}
            if row.get("positionObservations"):obs["positionObservations"]=row["positionObservations"]
            group=candidates.setdefault(norm(label),[])
            if obs not in group:group.append(obs)
    # Undated Wikidata and current Wikipedia rows are review leads only.
    ids={qid(x) for x in owner.get("identity_candidates",[])}-{None}
    for row in owner.get("wikidata_undated_players",[]):
        label=row.get("playerLabel");pid=qid(row.get("player"))
        if len(ids)==1 and qid(row.get("club")) in ids and pid and valid_name(label) and norm(label) not in native["players"]:
            obs={"source":"WD","sourceId":pid,"name":label,"season":"MEMBERSHIP_DATE_UNKNOWN_NOT_JULY1","evidence":row.get("membership")}
            if row.get("positionObservations"):obs["positionObservations"]=row["positionObservations"]
            group=candidates.setdefault(norm(label),[])
            if obs not in group:group.append(obs)
    # Current Wikipedia rows are extra leads, not July-dated evidence.
    for row in owner.get("wiki_squad_players",[]):
        label=row.get("playerLabel")
        if (row.get("country")==country and row.get("clubSource")=="EXTRA_WD" and
            row.get("scope")=="CURRENT_SQUAD_NOT_JULY1_OR_EDITION" and valid_name(label) and
            row.get("wikiPlayerPage") and norm(label) not in native["players"] and
            norm(row.get("club",""))==norm(native["name"])):
            obs={"source":"WP","sourceId":row["wikiPlayerPage"],"name":label,
                 "season":"CURRENT_SQUAD_NOT_JULY1_OR_EDITION","evidence":row.get("source")}
            if row.get("positionObservations"):obs["positionObservations"]=row["positionObservations"]
            group=candidates.setdefault(norm(label),[])
            if obs not in group:group.append(obs)
    groups=[]
    for name,obs in sorted(candidates.items()):
        ids={(item["source"],item["sourceId"]) for item in obs}
        positions=merge_positions(*(x.get("positionObservations") for x in obs))
        groups.append({"positionCandidate":projection(positions) if len(ids)==1 else {"group":None,"swosRole":None,"swosCode":None,"status":"IDENTITY_REVIEW"},"positionObservations":positions,"name":obs[0]["name"],"normalizedName":name,"status":"IDENTITY_REVIEW" if len(ids)>1 else "CURRENT_SQUAD_REVIEW" if obs[0]["season"] in ("CURRENT_SQUAD_NOT_JULY1_OR_EDITION","MEMBERSHIP_DATE_UNKNOWN_NOT_JULY1") else "NOMINAL_CANDIDATE","observations":obs})
    return {"clubKey":uid,"name":native["name"],"file":native.get("file"),"association":country,
            "status":"CANDIDATES" if groups else "NO DATA","existingRealNames":len(native["players"]),
            "testSlots":native.get("test_slots",0),"existingExtras":native.get("extra_count",0),
            "placement":"POOL_REVIEW" if uid.startswith("pool:") else "BASE_AND_EXTRA_REVIEW" if native.get("test_slots") else "EXTRA_CANDIDATE",
            "groups":groups,"nameMatches":owner.get("name_matches",[])}

def native_index(root):
    from collect_names import existing_index
    out={};ambiguous=set()
    national=json.loads((root/"docs/NATIONAL_ROSTER_SOURCE_MAP_2627.json").read_text(encoding="utf-8-sig"))["records"]
    rawmap={}
    for r in national:
        if r["source"]["team_file"]!=255:rawmap.setdefault(r["source"]["team_file"],set()).add(r["fifa"])
    for matches in existing_index(root).values():
        for v in matches.values():
            uid=v.get("uuid")
            if not uid:continue
            if uid in out:ambiguous.add(uid)
            else:
                codes=rawmap.get(v["country"],set()) if not v["file"].startswith("GUESTS_") else set()
                out[uid]=dict(v,associations=codes)
    for uid in ambiguous:out.pop(uid,None)
    pool=root/"data_json/worlds/2627/competitions/bottom_pools.json"
    if pool.exists():
        for c in json.loads(pool.read_text(encoding="utf-8-sig")).get("clubs",[]):
            if c.get("administrative_test") or c.get("roster",{}).get("template")!="TEST_ZERO":continue
            uid="pool:"+str(c["key"])
            if uid in out:raise ValueError("Duplicate pool key")
            out[uid]={"name":c["name"],"file":"bottom_pools.json","players":set(),"test_slots":c["roster"].get("base_slots",16),"associations":rawmap.get(c.get("country"),set())}
    return out

def prepare(snapshot,output,only=None,include_extras=True,data=None,year=2026):
    if only and not re.fullmatch(r"[A-Z]{3}",only):raise ValueError("Invalid FIFA country code")
    if data is None:data=load_snapshot(snapshot)
    if data.get("researchYear",2026)!=year:raise ValueError("Import research year mismatch")
    summary=[]
    for t in data["targets"]:
        if only and t["fifa"]!=only:continue
        v=build(t,year);atomic(output/"clubs"/(t["fifa"]+".json"),v)
        n=len(v["clubs"]);p=sum(c["nominalCount"] for c in v["clubs"])
        ready=sum(c["status"]=="NOMINAL_ROSTER_16_PLUS" for c in v["clubs"])
        review=sum(c["status"]=="IDENTITY_REVIEW" for c in v["clubs"])
        summary.append({"country":t["fifa"],"clubCandidates":n,"playerObservations":p,"nominal16Plus":ready,"identityReview":review,"competitions":len(v["competitions"]),
                        "wikiClubIdentityLeads":len(v["wikiClubIdentityLeads"]),"wikiCurrentSquadObservations":len(v["wikiCurrentSquadObservations"]),
                        "archiveClubNameCandidates":len(v["archiveClubNameCandidates"])})
        report("Import "+t["fifa"],"FOUND" if n else "NO DATA",f"{n} source-ID club candidates, {len(v['archiveClubNameCandidates'])} archive name leads (may overlap), {p} player observations, {ready} with 16+ names, {review} identity reviews")
    extra_summary=[]
    if include_extras:
        native=native_index(snapshot.parents[2]);groups={}
        for uid,e in sorted(data.get("existing_club_gaps",{}).items()):
            country=e.get("association") or "UNMAPPED"
            if not re.fullmatch(r"[A-Z]{3}",country):country="UNMAPPED"
            if only and country!=only:continue
            n=native.get(uid)
            if n and n["associations"]!={country}:n=None
            v=extras_for_club(uid,e,n,year)
            groups.setdefault(country,[]).append(v)
        for country,clubs in sorted(groups.items()):
            n=sum(len(c["groups"]) for c in clubs)
            atomic(output/"extras"/(country+".json"),{"schema":"SWOS_NOMINAL_EXTRA_CANDIDATES_1","country":country,"clubs":clubs})
            extra_summary.append({"country":country,"clubs":len(clubs),"nameGroups":n})
            report("Import EXTRA "+country,"FOUND" if n else "NO DATA",f"{n} distinct club/name groups; not native player slots")
    atomic(output/("INDEX_"+only+".json" if only else "INDEX.json"),{"schema":"SWOS_NOMINAL_IMPORT_INDEX_1","researchYear":year,"countries":summary,"extras":extra_summary,"extrasIncluded":include_extras})
    return summary
def selftest():
    lead={"source":"BetExplorer","country":"CAM","kind":"LEAGUE_DIVISION","identityEvidence":True,"scope":"SEASON_PARTICIPANT_LABELS_NOT_JULY1","season":"2026/27","participants":[{"name":"Club One"},{"name":"Club One"},{"name":"Women Club"}]}
    owner={"fifa":"CAM","domestic_source_observations":[lead,dict(lead,season="2025/26"),dict(lead,kind="NATIONAL_CUP")]}
    assert len(archive_club_candidates(owner))==1 and not build(owner)["clubs"] and needs_club_sources(owner)
    a={"fifa":"BHU","new":True,"transfermarkt_clubs":[],"transfermarkt_players":[]}
    c={"country":"BHU","club":"Paro FC","season":"2026","scope":"SEASON_CANDIDATE_NOT_JULY1","competition":"BH1L","level":1,"transfermarktClubId":"1"}
    a["transfermarkt_clubs"]=[c,dict(c,competition="BH2")]
    a["transfermarkt_players"]=[dict(c,transfermarktPlayerId=str(i+1),playerLabel="Player "+str(i)) for i in range(16)]
    a["transfermarkt_players"]+=[dict(a["transfermarkt_players"][0],competition="BH2"),dict(c,transfermarktClubId="99",transfermarktPlayerId="100",playerLabel="Other")]
    v=build(a)
    assert len(v["clubs"])==1 and len(v["competitions"])==2 and v["clubs"][0]["nominalCount"]==16
    assert v["clubs"][0]["status"]=="NOMINAL_ROSTER_16_PLUS" and len(v["warnings"])==1
    a["transfermarkt_clubs"].append(dict(c,club="Women FC",transfermarktClubId="3"))
    assert len(build(a)["clubs"])==1
    native={"name":"Paro FC","players":{norm("Player 0")},"test_slots":0,"extra_count":0}
    e={"name":"Paro FC","association":"BHU","transfermarkt_clubs":[c],"transfermarkt_players":a["transfermarkt_players"][:16]}
    x=extras_for_club("native-id",e,native)
    assert len(x["groups"])==15 and x["placement"]=="EXTRA_CANDIDATE"
    assert not extras_for_club("native-id",e,dict(native,name="Other"))["groups"]
    split=dict(c,season="2026/27")
    a["transfermarkt_clubs"]=[c,split]
    a["transfermarkt_players"]=[dict(c,transfermarktPlayerId=str(i+1),playerLabel="Player "+str(i)) for i in range(8)]+[dict(split,transfermarktPlayerId=str(i+1),playerLabel="Player "+str(i)) for i in range(8,16)]
    assert build(a)["clubs"][0]["status"]=="ROSTER_INCOMPLETE"
