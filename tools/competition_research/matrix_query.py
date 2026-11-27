#!/usr/bin/env python3
"""Bounded, read-only research matrix queries."""
import argparse,json
from collections import Counter
from pathlib import Path
from matrix_storage import raw,read_index,read_row,read_matrix,selftest,key
MATRIX=Path(__file__).resolve().parents[2]/"docs/competition_research/closure/AUTOMATIC_INSERTION.json"
LEGACY=("name","identity","participants","format","calendar","regulation","movement","qualification","july1","evolution")
OPERATIONAL=("name","identity","participants","format","regulation","movement","qualification","july1","roundDates","competitionDays","evolution","seasonChange")
def emit(x):print(json.dumps(x,ensure_ascii=False,separators=(",",":")))
def states(r,mode):return r.get("operationalFields" if mode=="operational" else "fields",{})
def label(r,mode):
    return {"id":r.get("row",r.get("researchKey")),"world":r["world"],"name":r.get("name"),
            "status":r.get("operationalStatus" if mode=="operational" else "status","UNREVIEWED"),
            "fields":states(r,mode),"path":r.get("path",r.get("inventoryPath"))}
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("command",choices=("summary","progress","country","missing","next","show","validate","self-test","evidence"))
    p.add_argument("target",nargs="?")
    p.add_argument("--matrix",type=Path,default=MATRIX)
    p.add_argument("--world",choices=("9697","2627"))
    p.add_argument("--country")
    p.add_argument("--mode",choices=("legacy","operational"),default="operational")
    p.add_argument("--field")
    p.add_argument("--limit",type=int,default=20)
    p.add_argument("--offset",type=int,default=0)
    p.add_argument("--full",action="store_true")
    a=p.parse_args()
    if not 1<=a.limit<=100 or a.offset<0:p.error("limit 1..100; non-negative offset")
    if a.command=="self-test":selftest();print("PASS");return
    m=raw(a.matrix)
    if a.command=="validate":
        d=read_matrix(a.matrix);rows=d["rows"]
        if len({r["row"] for r in rows})!=len(rows):raise ValueError("Duplicate row")
        emit({"rows":len(rows),"ceOnly":len(d.get("ceOnlyEntries",[])),"status":"STRUCTURE_VALID_NOT_SPORT_CERTIFICATION"});return
    if a.command in ("show","evidence"):
        if not a.target:p.error("needs row ID or CE research key")
        if a.target.isdecimal():r=read_row(a.matrix,int(a.target))
        else:
            matches=[x for x in m.get("ceOnlyEntries",[]) if x.get("researchKey")==a.target]
            if len(matches)!=1:p.error("unknown competition")
            r=matches[0]
        if a.command=="show":
            if a.field:emit({"id":a.target,"field":a.field,"status":states(r,a.mode).get(a.field,"?"),"evidence":r.get("evidenceByField",{}).get(a.field),"inventoryPath":r.get("inventoryPath")})
            else:emit(r if a.full else label(r,a.mode))
            return
        if not a.field:p.error("evidence needs --field (dotted source path)")
        relative=r.get("inventoryPath")
        if not relative:p.error("no inventory path")
        docs=a.matrix.parent.parent.resolve();source=(docs/relative).resolve()
        if not source.is_relative_to(docs) or source.suffix!=".json":p.error("invalid source path")
        obj=raw(source)
        if isinstance(obj,list):
            matches=[x for x in obj if x.get("row")==r.get("row")]
            if len(matches)!=1:p.error("ambiguous source row")
            obj=matches[0]
        elif "row" in r and (not isinstance(obj,dict) or obj.get("row")!=r["row"]):p.error("source identity not established")
        for part in a.field.split("."):
            if not isinstance(obj,dict) or part not in obj:p.error("field not found")
            obj=obj[part]
        if isinstance(obj,list):emit({"total":len(obj),"offset":a.offset,"items":obj[a.offset:a.offset+a.limit]})
        elif isinstance(obj,dict):emit({"totalKeys":len(obj),"offset":a.offset,"keys":list(obj)[a.offset:a.offset+a.limit]})
        else:emit(obj)
        return
    if a.command=="country":
        if not a.target:p.error("country needs FIFA code")
        a.country=a.target.upper()
    rows=read_index(a.matrix,a.country)
    if a.mode=="operational":rows+=m.get("ceOnlyEntries",[])
    if a.world:rows=[r for r in rows if r["world"]==a.world]
    if a.country:rows=[r for r in rows if (r.get("country") or key(r))==a.country]
    names=OPERATIONAL if a.mode=="operational" else LEGACY
    if a.field and a.field not in names:p.error("unknown field")
    if a.command in ("summary","progress"):
        emit({"mode":a.mode,"count":len(rows),"catalogueRows":sum("row" in r for r in rows),"ceOnly":sum("row" not in r for r in rows),
              "fields":{k:dict(Counter(states(r,a.mode).get(k,"?") for r in rows)) for k in names},
              "statuses":dict(Counter(label(r,a.mode)["status"] for r in rows)),
              "note":"NA separate; ? includes unreviewed; not certification"});return
    if a.command in ("missing","next"):
        wanted=[a.field] if a.field else names
        rows=[r for r in rows if any(states(r,a.mode).get(k,"?") in ("I","P","?") for k in wanted)]
        if a.command=="next":a.limit=min(a.limit,5)
    emit({"total":len(rows),"offset":a.offset,"rows":[label(r,a.mode) for r in rows[a.offset:a.offset+a.limit]]})
if __name__=="__main__":main()
