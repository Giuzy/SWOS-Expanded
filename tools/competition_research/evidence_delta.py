"""Research-only semantic deltas. Compare within the same verified inventory row.

Never infer completeness or resolve conflicts by overwriting old evidence.
A delta packet is not a complete roster or round list.
"""
import json
import unicodedata

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

def delta(rec,packet):
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
