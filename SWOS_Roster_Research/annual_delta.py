"""Read-only inter-edition comparisons by source IDs, never automatic removals."""
from season_config import output_dir
from roster_storage import load_snapshot, atomic, read
from import_candidates import build

def roster_index(target,year):
    clubs=build(target,year)["clubs"]
    return {(c["source"],c["sourceId"]):c for c in clubs}

def compare(before,after):
    def names(values):return [list(k) for k in sorted(values)]
    common=set(before)&set(after)
    changes=[]
    for k in sorted(common):
        a,b=before[k],after[k]
        pa={p["sourceId"] for p in a["players"]}
        pb={p["sourceId"] for p in b["players"]}
        la={m["competitionId"] for m in a["memberships"]}
        lb={m["competitionId"] for m in b["memberships"]}
        if a["name"]!=b["name"] or pa!=pb or la!=lb:
            changes.append({"source":k[0],"sourceId":k[1],"oldName":a["name"],"newName":b["name"],
                "newlyObservedPlayerIds":sorted(pb-pa),"notReobservedPlayerIds":sorted(pa-pb),
                "oldCompetitionIds":sorted(la),"newCompetitionIds":sorted(lb)})
    return {"newlyObservedClubIds":names(set(after)-set(before)),
            "notReobservedClubIds":names(set(before)-set(after)),"changedObservations":changes}

def write_delta(root,year,*,scope):
    if scope not in ('rosters','competitions'):raise ValueError('Invalid delta scope')
    a,b=output_dir(root,year-1),output_dir(root,year)
    before=a/"candidates.json";after=b/"candidates.json"
    if scope=='rosters' and before.exists() and after.exists():
        previous_data=load_snapshot(before);current_data=load_snapshot(after)
        old={t["fifa"]:t for t in previous_data["targets"]}
        new={t["fifa"]:t for t in current_data["targets"]}
        for country in sorted(set(new)-set(old)):
            atomic(b/"annual_delta"/"clubs"/(country+".json"),{"country":country,"previousYear":year-1,"researchYear":year,"status":"NO_PREVIOUS_TARGET_BASELINE","scope":"First observation is not proof of a new club"})
        for country in sorted(set(old)&set(new)):
            result=compare(roster_index(old[country],year-1),roster_index(new[country],year))
            atomic(b/"annual_delta"/"clubs"/(country+".json"),dict(result,country=country,previousYear=year-1,researchYear=year,
                scope="Partial source observations; absence is not departure, dissolution or transfer"))
        old_gaps=previous_data.get("existing_club_gaps",{})
        new_gaps=current_data.get("existing_club_gaps",{})
        grouped={}
        def observations(owner):
            ids=set()
            for field,source,key in (("players","WD","player"),("alternative_players","OFB","player"),("transfermarkt_players","TM","transfermarktPlayerId")):
                for row in owner.get(field,[]):
                    if row.get(key):ids.add((source,str(row[key]).rsplit("/",1)[-1]))
            return ids
        for uid,owner in sorted(new_gaps.items()):
            if not owner.get("association"):continue
            previous=old_gaps.get(uid)
            row={"clubKey":uid,"name":owner.get("name"),"baseline":"PREVIOUS_PARTIAL_RESEARCH" if previous is not None else "NO_PREVIOUS_RESEARCH"}
            if previous is not None:
                p,q=observations(previous),observations(owner)
                row.update(newlyObservedIds=[list(k) for k in sorted(q-p)],notReobservedIds=[list(k) for k in sorted(p-q)])
            grouped.setdefault(owner["association"],[]).append(row)
        for country,rows in sorted(grouped.items()):
            atomic(b/"annual_delta"/"existing_clubs"/(country+".json"),{"country":country,"researchYear":year,"previousYear":year-1,"scope":"Research observations only; 2026 extras baseline is not a complete roster","clubs":rows})
    previous=a/"competition_candidates";current=b/"competition_candidates"
    if scope=='competitions' and previous.exists() and current.exists():
        for path in sorted(current.glob("*.json")):
            other=previous/path.name
            if not other.exists():continue
            def index(file):
                groups={}
                for row in read(file).get("observations",[]):
                    k=(row.get("source"),row.get("sourceId"),row.get("kind"))
                    if not all(k):continue
                    g=groups.setdefault(k,{"names":set(),"participants":set()})
                    g["names"].add(row.get("name") or "")
                    g["participants"].update(str(p["sourceId"]) for p in row.get("participants",[]) if p.get("sourceId"))
                return groups
            old,new=index(other),index(path);changes=[]
            for k in sorted(set(old)&set(new)):
                x,y=old[k],new[k]
                if x!=y:changes.append({"identity":list(k),"oldNames":sorted(x["names"]),"newNames":sorted(y["names"]),
                    "newlyObservedEntrants":sorted(y["participants"]-x["participants"]),"notReobservedEntrants":sorted(x["participants"]-y["participants"])})
            atomic(b/"annual_delta"/"competitions"/path.name,{"researchYear":year,"previousYear":year-1,
                "scope":"Observations only; promotion/relegation or rules not inferred",
                "newlyObservedCompetitions":[list(k) for k in sorted(set(new)-set(old))],
                "notReobservedCompetitions":[list(k) for k in sorted(set(old)-set(new))],"changes":changes})
def selftest():
    a={("TM","1"):{"name":"Old","players":[{"sourceId":"7"}],"memberships":[{"competitionId":"L"}]}}
    b={("TM","1"):{"name":"New","players":[{"sourceId":"8"}],"memberships":[{"competitionId":"M"}]}}
    c=compare(a,b)["changedObservations"][0]
    assert c["newlyObservedPlayerIds"]==["8"] and c["notReobservedPlayerIds"]==["7"]
    assert c["oldCompetitionIds"]==["L"] and c["newCompetitionIds"]==["M"]
