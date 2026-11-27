"""Scoped Wikidata/Wikipedia leads; no invented July or seasonal membership."""
import hashlib,json,re,time,urllib.parse,urllib.request
from pathlib import Path
from http_retry import fetch, HostCooldown
from roster_storage import atomic
from research_status import report, progress
from alternative_sources import norm,BAD

UA="SWOS-Expanded-Roster-Research/1.2 (https://github.com/Giuzy/SWOS-Expanded/issues/2)"
WD="https://www.wikidata.org/w/api.php"
WP="https://en.wikipedia.org/w/api.php"
MEN=("Q31930761","Q35554451")
def qid(v):
    s=str(v or "").rsplit("/",1)[-1]
    return s if re.fullmatch(r"Q[1-9][0-9]*",s) else None
def values(rows):
    return " ".join("wd:"+v for v in sorted({qid(x) for x in rows}-{None}))
def valid_club_name(s):
    return isinstance(s,str) and bool(s.strip()) and not BAD.search(s) and not re.search(r"\b(women|ladies|female|girls|youth|reserves?|under[- ]?\d+|u[- ]?\d{2}|futsal|academy|national team)\b",s,re.I)
def league_club_query(ids):
    return f"""SELECT DISTINCT ?league ?club ?clubLabel WHERE {{
      VALUES ?league {{ {values(ids)} }}
      ?club wdt:P118 ?league; wdt:P2094 ?class.
      VALUES ?class {{ wd:Q31930761 wd:Q35554451 }}
      FILTER NOT EXISTS {{ ?club (wdt:P2094|wdt:P641) ?excludedClass. VALUES ?excludedClass {{ wd:Q606060 wd:Q44418853 wd:Q44418672 wd:Q44418451 wd:Q44418152 wd:Q44408003 wd:Q44417973 wd:Q44417630 wd:Q45053375 wd:Q44417376 }} }}
      FILTER NOT EXISTS {{ ?club wdt:P576 ?end. }}
      SERVICE wikibase:label {{ bd:serviceParam wikibase:language "en,mul". }}
    }}"""
def tm_bridge_query(ids):
    tokens=" ".join(json.dumps(x) for x in sorted({str(i) for i in ids if re.fullmatch(r"[1-9][0-9]*",str(i))}))
    return f"""SELECT DISTINCT ?tm ?club ?clubLabel WHERE {{
      VALUES ?tm {{ {tokens} }}
      ?club wdt:P7223 ?tm.
      FILTER NOT EXISTS {{ ?club (wdt:P2094|wdt:P641) ?excludedClass. VALUES ?excludedClass {{ wd:Q606060 wd:Q44418853 wd:Q44418672 wd:Q44418451 wd:Q44418152 wd:Q44408003 wd:Q44417973 wd:Q44417630 wd:Q45053375 wd:Q44417376 }} }}
      SERVICE wikibase:label {{ bd:serviceParam wikibase:language "en,mul". }}
    }}"""
def domestic_query(code,year=2026,kinds=None):
    if not re.fullmatch(r"[A-Z]{3}",code):raise ValueError("Invalid FIFA code")
    branches={"LEAGUE_DIVISION":'{ ?league wdt:P3983 1. BIND("LEAGUE_DIVISION" AS ?kind) }',
              "NATIONAL_CUP":'{ ?league wdt:P31 wd:Q34262376. BIND("NATIONAL_CUP" AS ?kind) }'}
    selected=set(branches) if kinds is None else set(kinds)
    if not selected or selected-set(branches):raise ValueError("Invalid domestic kinds")
    selection=" UNION ".join(branches[k] for k in sorted(selected))
    return f"""SELECT DISTINCT ?country ?league ?leagueLabel ?kind WHERE {{
      {{ ?country wdt:P3441 {json.dumps(code)}. }}
      UNION {{ ?entity wdt:P3441 {json.dumps(code)}; wdt:P1532 ?country. }}
      UNION {{ ?entity wdt:P3441 {json.dumps(code)}; wdt:P17 ?country. }}
      ?league wdt:P17 ?country; wdt:P2094 ?class.
      VALUES ?class {{ wd:Q31930761 wd:Q35554451 }}
      {{ {selection} }}
      FILTER NOT EXISTS {{ ?league (wdt:P2094|wdt:P641) ?excludedClass. VALUES ?excludedClass {{ wd:Q606060 wd:Q44418853 wd:Q44418672 wd:Q44418451 wd:Q44418152 wd:Q44408003 wd:Q44417973 wd:Q44417630 wd:Q45053375 wd:Q44417376 }} }}
      FILTER NOT EXISTS {{ ?league wdt:P576 ?end. FILTER(?end < "{year}-07-01T00:00:00Z"^^xsd:dateTime) }}
      SERVICE wikibase:label {{ bd:serviceParam wikibase:language "en,mul". }}
    }}"""
def squad_templates(body):
    # Balance nested templates (e.g. flagicon), which otherwise truncate rows before name=.
    for m in re.finditer(r"\{\{\s*(?:fs[ _]player2?|football[ _]squad[ _]player)\s*\|",body,re.I):
        depth=1
        for token in re.finditer(r"\{\{|\}\}",body[m.end():]):
            depth+=1 if token.group()=="{{" else -1
            if depth==0:
                yield body[m.end():m.end()+token.start()]
                break

def squad_rows(wikitext):
    # Only explicit senior/current squad headings and football squad templates.
    # No arbitrary article links, past players, reserve/youth or loan-out lists.
    headings=list(re.finditer(r"(?m)^(={2,6})\s*([^=\n]+?)\s*\1\s*$",wikitext))
    rows=[];ancestors=[]
    for i,h in enumerate(headings):
        depth=len(h.group(1));title=" ".join(h.group(2).casefold().replace("-"," ").split())
        while ancestors and ancestors[-1][0]>=depth:ancestors.pop()
        excluded=any(re.search(r"\b(women|women\x27s|ladies|female|girls|youth|reserve|reserves|academy|former|loan|under\s*\d+|u\s*\d{2})\b",part) for _,part in ancestors)
        ancestors.append((depth,title))
        if excluded or title not in {"current squad","first team squad","first team","squad","current first team squad"}:continue
        # Restrict to the heading's direct content; any subheading ends it.
        body=wikitext[h.end():headings[i+1].start() if i+1<len(headings) else len(wikitext)]
        for template in squad_templates(body):
            t=re.search(r"(?:^|\|)\s*name\s*=\s*\[\[([^\]|#]+)(?:\|([^\]]+))?\]\]",template,re.I)
            if not t:continue
            link=t.group(1).strip();label=(t.group(2) or re.sub(r"\s*\([^()]*\)$","",link)).strip()
            if ":" in link or not valid_club_name(label) or "{{" in label:continue
            role=re.search(r"(?:^|\|)\s*pos\s*=\s*([^|{}\[\]]+)",template,re.I)
            pair=(link,label,role.group(1).strip() if role else None)
            if pair not in rows:rows.append(pair)
    return rows

def squad_names(wikitext):
    return list(dict.fromkeys((link,label) for link,label,_ in squad_rows(wikitext)))

class Wiki:
    def __init__(self,cache,cp,results,query,refresh=False):
        self.cache=Path(cache)/"wiki_api";self.cp=cp;self.results=results;self.query=query;self.refresh=refresh
        self.pages={};self.bridges={};self.bridge_checked=set()
    def add(self,owner,field,row):
        seq=self.cp.ensure(owner,field,[])
        if field=="wiki_squad_players":
            from player_roles import enrich
            for old in seq:
                if {k:v for k,v in old.items() if k!="positionObservations"}=={k:v for k,v in row.items() if k!="positionObservations"}:
                    enrich(self.cp,old,row);return False
        if row not in seq:self.cp.append(seq,row);return True
        return False
    def error(self,label,e):
        row={"phase":label,"error":str(e)}
        if row not in self.results["errors"]:self.cp.append(self.results["errors"],row)
        report(label,"ERROR",str(e))
    def api(self,url,params):
        route=url+"?"+urllib.parse.urlencode(dict(params,format="json",formatversion=2,maxlag=5))
        self.cache.mkdir(parents=True,exist_ok=True)
        p=self.cache/(hashlib.sha256(route.encode()).hexdigest()+".json")
        if p.exists() and not self.refresh and time.time()-p.stat().st_mtime<86400:
            return json.loads(p.read_text(encoding="utf-8"))
        value=json.loads(fetch(urllib.request.Request(route,headers={"User-Agent":UA,"Accept":"application/json"}),timeout=45,interval=1.5))
        if "error" in value:raise RuntimeError("MediaWiki: "+str(value["error"].get("code")))
        atomic(p,value);return value
    def bridge(self,rows):
        ids=sorted({str(x.get("transfermarktClubId")) for x in rows if re.fullmatch(r"[1-9][0-9]*",str(x.get("transfermarktClubId")))})
        ids=[cid for cid in ids if cid not in self.bridge_checked]
        found={}
        for i in progress("Wikipedia identity bridge batches",range(0,len(ids),30)):
            try:matches=self.query(tm_bridge_query(ids[i:i+30]))
            except HostCooldown as e:report("Wiki club bridge","DEFERRED",str(e));continue
            except Exception as e:self.error("Wiki club bridge",e);continue
            self.bridge_checked.update(ids[i:i+30])
            for r in matches:
                if r.get("tm") in ids[i:i+30] and qid(r.get("club")) and valid_club_name(r.get("clubLabel","")):
                    found.setdefault(r["tm"],set()).add(qid(r["club"]))
        self.bridges.update({k:next(iter(v)) for k,v in found.items() if len(v)==1})
        return {str(r.get('transfermarktClubId')):self.bridges[str(r.get('transfermarktClubId'))] for r in rows if str(r.get('transfermarktClubId')) in self.bridges}
    def squad(self,club):
        if club in self.pages:return self.pages[club]
        entity=self.api(WD,{"action":"wbgetentities","ids":club,"props":"sitelinks"}).get("entities",{}).get(club,{})
        title=entity.get("sitelinks",{}).get("enwiki",{}).get("title")
        if not title or not valid_club_name(title):self.pages[club]=[];return []
        p=self.api(WP,{"action":"query","prop":"pageprops|revisions","titles":title,
                       "rvprop":"ids|timestamp|content","rvslots":"main"}).get("query",{}).get("pages",[])
        if len(p)!=1 or p[0].get("pageprops",{}).get("wikibase_item")!=club:return []
        rev=p[0].get("revisions",[])
        if len(rev)!=1:return []
        r=rev[0];rev_id=r.get("revid");content=r.get("slots",{}).get("main",{}).get("content")
        if not rev_id or not isinstance(content,str):return []
        from player_roles import position_evidence
        source="https://en.wikipedia.org/w/index.php?oldid="+str(rev_id)
        data=[{"playerLabel":label,"wikiPlayerPage":link,"clubQid":club,
               "source":"https://en.wikipedia.org/w/index.php?oldid="+str(rev_id),
               "sourceRevision":rev_id,"revisionTime":r.get("timestamp"),
               "scope":"CURRENT_SQUAD_NOT_JULY1_OR_EDITION",
               **({"positionObservations":position_evidence(pos,"Wikipedia","CURRENT_SQUAD_NOT_JULY1_OR_EDITION",source)} if pos else {})} for link,label,pos in squad_rows(content)]
        self.pages[club]=data;return data
    def collect(self,owner,club,name,country,source,source_id,known):
        if not club or not valid_club_name(name):return
        label="Wikipedia squad "+country+"/"+name
        try:rows=self.squad(club)
        except Exception as e:self.error(label,e);return
        added=0
        for r in rows:
            if norm(r["playerLabel"]) in known:continue
            added+=int(self.add(owner,"wiki_squad_players",dict(r,country=country,club=name,clubSource=source,clubSourceId=source_id)))
        report(label,"FOUND" if rows else "NO DATA",f"{len(rows)} observed names; {added} new candidates; date/edition unverified")
    def target(self,owner):
        c=owner["fifa"];tm=owner.get("transfermarkt_clubs",[])
        bridged=self.bridge(tm)
        # Retain identity across phases; do not infer a competition from a name.
        jobs={}
        for r in tm:
            cid=str(r.get("transfermarktClubId"));q=bridged.get(cid)
            if r.get("country")==c and r.get("level")==1 and q:jobs[("TM",cid)]=(q,r.get("club"))
        for row in owner.get("alternative_clubs",[]):
            q=qid(row.get("clubQid"))
            if q and row.get("country")==c:jobs[("OFB",q)]=(q,row.get("club"))
        for l in owner.get("leagues",[]):
            for s in l.get("seasons",[]):
                for row in s.get("clubs",[]):
                    q=qid(row.get("club"))
                    if q:jobs[("WD",q)]=(q,row.get("name"))
        for row in owner.get("wiki_club_leads",[]):
            q=qid(row.get("club"))
            if q:jobs[("WD_LEAD",q)]=(q,row.get("clubLabel"))
        for (source,cid),(q,name) in progress("Wikipedia squads "+c,jobs.items()):
            self.collect(owner,q,name,c,source,cid,set())
            self.cp.save()
    def extra(self,owner,known):
        c=owner.get("association");name=owner.get("name")
        if not c or not valid_club_name(name):return
        ids={qid(x) for x in owner.get("identity_candidates",[])}-{None}
        tm=[r for r in owner.get("transfermarkt_clubs",[]) if r.get("country")==c and norm(r.get("club",""))==norm(name)]
        ids.update(self.bridge(tm).values())
        ids.update(qid(r.get("clubQid")) for r in owner.get("alternative_clubs",[]) if r.get("country")==c and norm(r.get("club",""))==norm(name) and qid(r.get("clubQid")))
        if len(ids)==1:
            q=next(iter(ids));self.collect(owner,q,name,c,"EXTRA_WD",q,known)
def selftest():
    text="""==Current squad==
{{Fs player|no=1|nat=COM|pos=GK|name=[[Player One]]}}
{{Fs player|no=2|pos=DF|name=[[Player Two (footballer)|Player Two]]}}
===Out on loan===
{{Fs player|name=[[Not Included]]}}
==Former players==
{{Fs player|name=[[Old Player]]}}
"""
    assert squad_names(text)==[("Player One","Player One"),("Player Two (footballer)","Player Two")]
    assert squad_rows("==Current squad==\n{{Fs player|nat={{flagicon|SEN}}|pos=GK|name=[[New Player]]}}")==[("New Player","New Player","GK")]
    assert not squad_rows("==Current squad==\n{{Fs player|nat={{flagicon|SEN}}|name=[[Incomplete Player]]")
    assert not squad_names("==Youth team==\n{{Fs player|name=[[Wrong]]}}")
    assert not squad_names("==Women\x27s team==\n===Current squad===\n{{Fs player|name=[[Wrong]]}}")
    assert not valid_club_name("Women FC") and valid_club_name("Paro FC")
    assert "P118" in league_club_query(["Q1"])
    assert "P7223" in tm_bridge_query(["12","invalid"])
    assert "Q34262376" in domestic_query("COM")
