"""Fill missing role evidence on existing research rows only; no new players."""
import json,re
from collections import defaultdict
from player_roles import position_evidence,projection,enrich,merge_positions
from alternative_sources import norm
from research_status import report, progress
from season_config import seasons

def qid(value):
    s=str(value or "").rsplit("/",1)[-1]
    return s if re.fullmatch(r"Q[1-9][0-9]*",s) else None

def tm_id(value):
    s=str(value or "")
    return s if re.fullmatch(r"[1-9][0-9]*",s) else None

def needs(row):
    return projection(row.get("positionObservations",[]))["status"] in ("UNKNOWN","GROUP_ONLY")

def owners_rows(owners):
    for owner in owners:
        country=owner.get("fifa") or owner.get("association")
        for field in ("transfermarkt_players","alternative_players","players","wikidata_undated_players"):
            for row in owner.get(field,[]):
                if row.get("country",country)==country:
                    yield owner,field,row
        for league in owner.get("leagues",[]):
            for edition in league.get("seasons",[]):
                for club in edition.get("clubs",[]):
                    for field in ("players","wikidata_undated_players"):
                        for row in club.get(field,[]):
                            if qid(row.get("club"))==qid(club.get("club")) and qid(club.get("club")):
                                yield owner,field,row

def tm_key(row):
    return tuple(row.get(k) for k in ("country","transfermarktClubId","competition","season","level"))

def tm_jobs(owners,year,attempted):
    jobs=defaultdict(list)
    for owner,field,row in owners_rows(owners):
        if field!="transfermarkt_players" or not needs(row) or not tm_id(row.get("transfermarktPlayerId")):continue
        country,cid,competition,season,level=tm_key(row)
        if not country or not tm_id(cid) or not competition or season not in seasons(year) or type(level)!=int or not 1<=level<=4:continue
        matching=[c for c in owner.get("transfermarkt_clubs",[]) if tm_key(c)==tm_key(row) and norm(c.get("club",""))==norm(row.get("club",""))]
        if not matching or not norm(row.get("club","")):continue
        # Same source club and season. Never query again if normal acquisition tried it.
        key=(str(cid),season)
        if key not in attempted:jobs[key].append(row)
    return jobs

def wd_query(ids,tm=False):
    if tm:
        tokens=" ".join(json.dumps(x) for x in ids)
        binding="VALUES ?input { "+tokens+" } ?player wdt:P2446 ?input."
    else:
        tokens=" ".join("wd:"+x for x in ids)
        binding="VALUES ?player { "+tokens+" } BIND(STR(?player) AS ?input)"
    return """SELECT DISTINCT ?input ?player ?position ?positionLabel WHERE {
      """+binding+"""
      OPTIONAL { ?player wdt:P413 ?position. }
      SERVICE wikibase:label { bd:serviceParam wikibase:language "en,mul". }
    }"""

def apply_wd(cp,targets,rows,tm=False):
    matched=defaultdict(lambda:defaultdict(list))
    for row in rows:
        key=tm_id(row.get("input")) if tm else qid(row.get("input"))
        player=qid(row.get("player"))
        if key not in targets or not player or (not tm and player!=key):continue
        matched[key][player].extend(position_evidence(row.get("positionLabel"),"Wikidata","CAREER_POSITION_NOT_EDITION",row.get("player"),row.get("position")))
    changed=0
    for key,players in matched.items():
        # A duplicate P2446 mapping is ambiguous even if one player has no role.
        if len(players)!=1:continue
        observations=merge_positions(*players.values())
        for old in targets[key]:
            if needs(old):changed+=int(enrich(cp,old,{"positionObservations":observations}))
    return changed

def backfill(owners,cp,tm,run,year,allow_tm=True,allow_wd=True):
    from transfermarkt_source import table_links,require_squad_season,BASE
    owners=list(owners)
    if allow_tm and not tm.blocked:
        for (cid,season),rows in progress("Transfermarkt missing-role squads",tm_jobs(owners,year,tm.role_squad_attempts).items()):
            if tm.blocked:break
            tm.role_squad_attempts.add((cid,season))
            path="/-/kader/verein/"+cid+"/saison_id/"+str(year)
            label="Missing roles TM "+rows[0]["country"]+"/"+rows[0]["club"]
            try:
                page=tm.page(path)
                require_squad_season(page,rows[0]["club"],cid,season,year)
                players=table_links(page,r"/profil/spieler/([1-9][0-9]*)(?:/|$)",with_positions=True)
                n=0
                for row in rows:
                    if not needs(row) or norm(row.get("club",""))!=norm(rows[0]["club"]):continue
                    match=players.get(tm_id(row.get("transfermarktPlayerId")))
                    if not match or norm(match[0])!=norm(row.get("playerLabel","")):continue
                    n+=int(enrich(cp,row,{"positionObservations":[dict(p,evidence=BASE+path) for p in match[1]]}))
                cp.save();report(label,"FOUND" if n else "NO NEW DATA",f"{n} existing observations enriched; no new player")
            except Exception as exc:
                tm.error(label,exc);report(label,"ERROR",str(exc))
    if not allow_wd:return
    targets={"WD":defaultdict(list),"TM":defaultdict(list)}
    for _,field,row in owners_rows(owners):
        if not needs(row):continue
        source="TM" if field=="transfermarkt_players" else "WD"
        key=tm_id(row.get("transfermarktPlayerId")) if source=="TM" else qid(row.get("player"))
        if key:targets[source][key].append(row)
    for source,entries in targets.items():
        keys=sorted(entries)
        for i in progress("Wikidata missing-role batches "+source,range(0,len(keys),25)):
            group=keys[i:i+25];batch={k:entries[k] for k in group}
            label="Missing roles "+source+" "+str(i+1)+"-"+str(i+len(group))+"/"+str(len(keys))
            rows=run(wd_query(group,source=="TM"),label)
            if rows is None:continue  # source error already logged; not a negative result
            n=apply_wd(cp,batch,rows,source=="TM")
            cp.save();report(label,"FOUND" if n else "NO NEW DATA",f"{n} existing observations enriched; no new player")

def selftest():
    class CP:
        def update(self,old,**kw):old.update(kw)
    cp=CP()
    row={"transfermarktPlayerId":"123","playerLabel":"Real Name"}
    sample={"input":"123","player":"http://www.wikidata.org/entity/Q1","position":"http://www.wikidata.org/entity/Q2","positionLabel":"goalkeeper"}
    assert apply_wd(cp,{"123":[row]},[sample,dict(sample,player="Q9",positionLabel="")],True)==0 and needs(row)
    assert apply_wd(cp,{"123":[row]},[sample],True)==1
    assert projection(row["positionObservations"])["swosCode"]==0
    assert apply_wd(cp,{"123":[row]},[dict(sample,positionLabel="Forward")],True)==0
    assert row["playerLabel"]=="Real Name" and row["transfermarktPlayerId"]=="123"
    x={"player":"Q1","playerLabel":"Real Name"}
    assert apply_wd(cp,{"Q1":[x]},[dict(sample,input="Q1",player="Q9")])==0
    assert apply_wd(cp,{"Q1":[x]},[dict(sample,input="Q1")])==1
    assert "P2446" in wd_query(["123"],True)
    club={"country":"BLZ","transfermarktClubId":"12","competition":"BZ1A","season":"2026/27","level":1,"club":"Club A"}
    player=dict(club,transfermarktPlayerId="13",playerLabel="New Name")
    owner={"fifa":"BLZ","transfermarkt_clubs":[club],"transfermarkt_players":[player]}
    assert list(tm_jobs([owner],2026,set()))==[("12","2026/27")]
    assert not tm_jobs([owner],2026,{("12","2026/27")})
    assert not tm_jobs([owner],2027,set())
    owner["transfermarkt_clubs"]=[]
    assert not tm_jobs([owner],2026,set())
