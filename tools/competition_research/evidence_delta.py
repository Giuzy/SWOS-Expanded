"""Research-only semantic deltas. Compare within the same verified inventory row.

Never infer completeness or resolve conflicts by overwriting old evidence.
A delta packet is not a complete roster or round list.
"""
import json
import unicodedata
import re
import datetime as dt
from collections import Counter

def norm(s):
    return ''.join(c for c in unicodedata.normalize('NFKD',str(s)).casefold() if c.isalnum())

# Row-scoped equivalents from the two BRA 2026 source inventories. These
# are research labels, NOT native club IDs and never used in another row.
BRA217 = {
 "Botafogo FR":"Botafogo","CA Mineiro":"Atlético-MG","CA Paranaense":"Athletico-PR",
 "CR Flamengo":"Flamengo","CR Vasco da Gama":"Vasco","Chapecoense AF":"Chapecoense",
 "Clube do Remo":"Remo","Coritiba FBC":"Coritiba","Cruzeiro EC":"Cruzeiro",
 "EC Bahia":"Bahia","EC Vitória":"Vitória","Fluminense FC":"Fluminense",
 "Grêmio FBPA":"Grêmio","Mirassol FC":"Mirassol","RB Bragantino":"Bragantino",
 "SC Corinthians Paulista":"Corinthians","SC Internacional":"Internacional",
 "SE Palmeiras":"Palmeiras","Santos FC":"Santos","São Paulo FC":"São Paulo"
}
BRA217 = {norm(k):norm(v) for k,v in BRA217.items()}

def freeze(v):
    return json.dumps(v,ensure_ascii=False,sort_keys=True,separators=(",",":"))

def legacy_snapshots(rec,scope,excluded):
    """Only base fixtures, never deep rules/tables, history, inferred dates or quarantined versions."""
    if not scope:return
    year=1996 if rec.get("world")=="9697" else 2026 if rec.get("world")=="2627" else None
    if year is None or rec.get("detail") not in ("div1","div2","div3","div4"):return
    for key,obs in rec.get("automaticResearch",{}).get("observations",{}).items():
        if key in excluded or obs.get("crossRowConflict"):continue
        group=obs.get("sourceGroup",{})
        source=group.get("source","")
        season=str(group.get("season","")).replace("-","/")
        if group.get("country")!=rec.get("country") or group.get("season_basis")!="SOURCE_SEASON":continue
        if source.startswith("euro"+rec["world"]+"/"):
            if not scope.get("fd") or group.get("competition")!=scope["fd"]:continue
        elif source=="github/openfootball/football.json":
            if not scope.get("openfootball") or group.get("competition")!=scope["openfootball"]:continue
        else:continue
        if season in (str(year)+"/"+str(year+1)[-2:],str(year)+"/"+str(year+1)):
            lo,hi=dt.date(year,7,1),dt.date(year+1,7,1)
        elif year==2026 and season=="2026" and scope.get("calendar_year"):
            lo,hi=dt.date(year,1,1),dt.date(year+1,1,1)
        else:continue
        cols=obs.get("fixtureColumns",[])
        required=("date","dateBasis","homeLabel","awayLabel")
        if not all(k in cols for k in required):continue
        shape=obs.get("shape",{})
        if any(shape.get(k,0) for k in ("invalid","clubDayClashes","inferredDates")):continue
        fixtures=obs.get("fixtures",[])
        seen=set();clubdays=set();clubs=set();dates=[]
        try:
            for item in fixtures:
                d,basis,a,b=(item[cols.index(k)] for k in required)
                day=dt.date.fromisoformat(d)
                if basis!="EXPLICIT" or not lo<=day<hi or not a or not b or a==b or (day,a) in clubdays or (day,b) in clubdays:
                    raise ValueError("invalid scope")
                if re.search(r"\b(women|female|ladies|youth|u[- ]?(?:17|18|19|20|21|23))\b",a+" "+b,re.I):
                    raise ValueError("excluded team")
                clubdays.update(((day,a),(day,b)));seen.add((a,b));clubs.update((a,b));dates.append(d)
        except (ValueError,TypeError,IndexError):continue
        if not dates or len(dates)!=shape.get("fixtures") or len(clubs)!=shape.get("teams"):continue
        n=len(clubs)
        complete=(shape.get("family")=="DOUBLE_ROUND_ROBIN" and len(dates)==n*(n-1) and seen=={(a,b) for a in clubs for b in clubs if a!=b})
        yield clubs,dates,complete

def delta(rec,packet,scope=None,excluded=()):
    brazil = (rec.get("row"),rec.get("world"),rec.get("country"),rec.get("detail"),rec.get("id")) == (217,"2627","BRA","div1","_brasilianLeague")
    def club(s):
        k=norm(s)
        return BRA217.get(k,k) if brazil else k
    def atom(field,v):
        if field=="participants" and isinstance(v,str):return club(v)
        if field=="july1" and isinstance(v,list) and len(v)==5:
            return freeze([v[0],club(v[1]),club(v[2]),v[3],v[4]])
        return freeze(v)
    known={}
    def mark(k,v):known.setdefault(k,set()).add(atom(k,v))
    def learn(fields):
        for k,v in fields.items():
            if k in ("participants","roundDates","july1") and isinstance(v,list):
                for item in v:mark(k,item)
            elif k=="participants" and isinstance(v,dict) and isinstance(v.get("observed"),list):
                for item in v["observed"]:mark(k,item)
                mark("participantsMeta",{a:b for a,b in v.items() if a!="observed"})
            elif k=="calendar" and isinstance(v,dict) and isinstance(v.get("observedDates"),list):
                for item in v["observedDates"]:mark("observedDates",item)
                mark("calendarMeta",{a:b for a,b in v.items() if a!="observedDates"})
            else:mark(k,v)
    # Prior strict observations have the same row/edition identity.
    for old in rec.get("automaticResearch",{}).get("strictAcquisition",{}).values():
        learn(old.get("fields",{}))
    # Exact row/edition/division source observations, not certified facts.
    # Match individual labels and dates; aggregates only when a single
    # comparable base snapshot has the same count/shape, never union them.
    for clubs,dates,complete in legacy_snapshots(rec,scope,excluded):
        for name in clubs:mark("participants",name)
        for date in dates:mark("observedDates",date)
        p=packet.get("participants")
        if isinstance(p,dict) and p.get("fullDoubleRoundRobin")==complete and {club(x) for x in p.get("observed",[])}=={club(x) for x in clubs}:
            mark("participantsMeta",{a:b for a,b in p.items() if a!="observed"})
        c=packet.get("calendar")
        if isinstance(c,dict) and c.get("matches")==len(dates) and set(c.get("observedDates",[]))==set(dates):
            mark("calendarMeta",{a:b for a,b in c.items() if a!="observedDates"})
        w=packet.get("competitionDays")
        if isinstance(w,dict) and w.get("observedWeekdays")==dict(Counter(dt.date.fromisoformat(d).strftime("%a") for d in dates)):
            mark("competitionDays",w)
        f=packet.get("format")
        if complete and isinstance(f,dict):
            if f.get("observedTeams")==len(clubs) and f.get("observedCycles")==2:
                mark("format",f)
            elif f=={"teams":len(clubs),"cycles":2,"rounds":2*(len(clubs)-1)}:
                mark("format",f)
    # Explicit historical source section only for the known BRA row. Other
    # legacy blocks may span editions: do not recurse through unscoped data.
    if brazil:
        block=rec.get("block65Research",{})
        for m in block.get("matches",[]):
            if (m.get("phase")=="LEAGUE" and "2026-01-01"<=m.get("date","")<"2026-07-01"
                and isinstance(m.get("home"),str) and isinstance(m.get("away"),str)
                and type(m.get("homeGoals")) is int and type(m.get("awayGoals")) is int):
                mark("participants",m["home"]);mark("participants",m["away"])
                mark("july1",[m["date"],m["home"],m["away"],m["homeGoals"],m["awayGoals"]])
    out={}
    for k,v in packet.items():
        if k in ("participants","roundDates","july1") and isinstance(v,list):
            fresh=[]
            for item in v:
                if atom(k,item) not in known.get(k,set()):
                    fresh.append(item);mark(k,item)
            if fresh:out[k]=fresh
        elif k=="participants" and isinstance(v,dict) and isinstance(v.get("observed"),list):
            fresh=[]
            for item in v["observed"]:
                if atom(k,item) not in known.get(k,set()):
                    fresh.append(item);mark(k,item)
            meta={a:b for a,b in v.items() if a!="observed"}
            if fresh or atom("participantsMeta",meta) not in known.get("participantsMeta",set()):
                out[k]=dict(meta,observed=fresh)
        elif k=="calendar" and isinstance(v,dict) and isinstance(v.get("observedDates"),list):
            fresh=[]
            for item in v["observedDates"]:
                if atom("observedDates",item) not in known.get("observedDates",set()):
                    fresh.append(item);mark("observedDates",item)
            meta={a:b for a,b in v.items() if a!="observedDates"}
            if fresh or atom("calendarMeta",meta) not in known.get("calendarMeta",set()):
                out[k]=dict(meta,observedDates=fresh)
        elif atom(k,v) not in known.get(k,set()):
            out[k]=v
    return out

def selftest():
    r={"row":217,"world":"2627","country":"BRA","detail":"div1","id":"_brasilianLeague",
       "block65Research":{"matches":[{"phase":"LEAGUE","date":"2026-01-28","home":"Atlético-MG","away":"Palmeiras","homeGoals":2,"awayGoals":2}]}}
    p={"participants":["CA Mineiro","SE Palmeiras","EC Bahia"],"july1":[["2026-01-28","CA Mineiro","SE Palmeiras",2,2],["2026-01-28","CA Mineiro","SE Palmeiras",1,2]]}
    d=delta(r,p)
    assert d["participants"]==["EC Bahia"] and len(d["july1"])==1
    r["automaticResearch"]={"strictAcquisition":{"test":{"fields":d}}}
    assert delta(r,p)=={}
    assert delta(dict(r,row=999),p)["participants"]==["CA Mineiro","SE Palmeiras"]
    a={"automaticResearch":{"strictAcquisition":{"x":{"fields":{"participants":["A"],"roundDates":[{"round":1,"first":"2026-08-01"}]}}}}}
    assert delta(a,{"participants":["A","B"]})=={"participants":["B"]}
    assert delta(a,{"roundDates":[{"round":1,"first":"2026-08-01"},{"round":2,"first":"2026-08-08"}]})=={"roundDates":[{"round":2,"first":"2026-08-08"}]}
    a={"automaticResearch":{"strictAcquisition":{"x":{"fields":{"calendar":{"observedDates":["2026-08-01"],"matches":1}}}}}}
    assert delta(a,{"calendar":{"observedDates":["2026-08-01","2026-08-02"],"matches":2}})=={"calendar":{"observedDates":["2026-08-02"],"matches":2}}

    legacy={"world":"9697","country":"ITA","detail":"div1","automaticResearch":{"observations":{"x":{"sourceGroup":{"source":"euro9697/all.xls","competition":"I1","country":"ITA","season":"1996/97","season_basis":"SOURCE_SEASON"},"shape":{"family":"DOUBLE_ROUND_ROBIN","teams":2,"fixtures":2},"fixtureColumns":["date","dateBasis","homeLabel","awayLabel"],"fixtures":[["1996-08-10","EXPLICIT","A","B"],["1996-08-11","EXPLICIT","B","A"]]}}}}
    packet={"participants":{"observed":["A","B"],"fullDoubleRoundRobin":True},"calendar":{"observedDates":["1996-08-10","1996-08-11"],"matches":2,"scope":"OBSERVED_DATES_NOT_ROUND_TEMPLATE"},"format":{"observedTeams":2,"observedCycles":2},"competitionDays":{"observedWeekdays":{"Sat":1,"Sun":1}}}
    assert delta(legacy,packet,{"fd":"I1"})=={}
    assert delta(legacy,packet,{"fd":"I2"})==packet
    assert delta(legacy,packet,{"fd":"I1"},{"x"})==packet
    changed=dict(packet,calendar={"observedDates":["1996-08-10","1996-08-12"],"matches":2,"scope":"OBSERVED_DATES_NOT_ROUND_TEMPLATE"})
    assert delta(legacy,changed,{"fd":"I1"})["calendar"]["observedDates"]==["1996-08-12"]
