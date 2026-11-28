"""Public FM documentation. Research leads only; no verified rules or club lists."""
import hashlib,re
from html.parser import HTMLParser
from urllib.parse import urlsplit
from alternative_sources import norm,BAD
from http_retry import time_budget
from research_status import report
MASTER="https://master1gor.github.io/FMSites/FM26/"
FALSO="https://www.fmscout.com/c-fm26-leagues.html?id=13088"
DAVE="https://fmdaveincid.com/"
REGIONS=("CONMEBOL","CONCACAF","UEFA","AFC","CAF","OFC")
CONF={80:"UEFA",81:"CAF",82:"CONMEBOL",83:"CONCACAF",84:"AFC",85:"OFC"}
ALIASES={"BAH":("The Bahamas",),"BHR":("Bahrein",),"BRU":("Brunei Darussalam",),
 "FRO":("Feroe Islands",),"PHI":("Phillipines",),"IRQ":("Irak",),"KEN":("Kenia",),
 "SUR":("Surinam",),"VIR":("USA Virgin Islands",),"CGO":("Congo",),"COD":("DR Congo",),
 "KOR":("South Korea",),"PRK":("North Korea",),"CZE":("Czech Republic",)}
RISK=re.compile(r"\b(fantasy|fictional|fictitious|approximat\w*|adjusted|simplified|similar to reality|old format|not accurately|best as I could|I have set|does not have|was changed)\b",re.I)
TOPICS={"format":r"\b(groups?|round robin|play[- ]?offs?|apertura|clausura|derby|derbies|format)\b",
 "movement":r"\b(promot\w*|relegat\w*)\b","qualification":r"\b(qualif\w*|continental)\b",
 "regulation":r"\b(registration|transfer windows?|substitutions?|foreign players?|match rules|points deduction)\b",
 "roundDates":r"\b(schedule|calendar|dates?)\b","evolution":r"\b(expand\w*|changes?|starting in 20\d\d)\b",
 "finance":r"\b(prize money|tv money|sponsor\w*|ticket prices?|salaries)\b",
 "clubMetadata":r"\b(club data fixes|nicknames|stadiums|foundation dates|club colours|rivalries)\b"}
def topics(s):return sorted(k for k,p in TOPICS.items() if re.search(p,s,re.I))

class Lines(HTMLParser):
    BLOCK={"br","p","div","li","ul","ol","h1","h2","h3","h4","h5","h6","tr","td","th","section","article","hr"}
    def __init__(self,raw):
        super().__init__(convert_charrefs=True);self.parts=[];self.hidden=0;self.removed=False
        self.feed(raw.decode("utf-8-sig",errors="replace"));self.close()
    def handle_starttag(self,tag,attrs):
        a=dict(attrs)
        if tag in ("script","style"):self.hidden+=1
        if self.hidden:return
        if tag=="span" and "removed" in a.get("class","").split():self.removed=True
        if tag in self.BLOCK:self.parts.append("\n")
        if tag in ("h1","h2","h3"):self.parts.append("@SECTION@ ")
        if tag=="img":
            host=urlsplit(a.get("src","")).hostname or ""
            classes=set(a.get("class","").split())
            if "flag" in classes and host=="flagpedia.net":self.parts.append("\n@FLAG@ ")
            elif classes & {"logo","flagnp"}:self.parts.append("\n@STOP@ ")
    def handle_endtag(self,tag):
        if tag in ("script","style"):self.hidden=max(0,self.hidden-1);return
        if tag=="span":self.removed=False
        if not self.hidden and tag in self.BLOCK:self.parts.append("\n")
    def handle_data(self,data):
        if not self.hidden and not self.removed:self.parts.append(data)
    def lines(self):return [" ".join(s.split()) for s in "".join(self.parts).splitlines() if s.strip()]

def lookup(national,aliases):
    result={}
    for r in national:
        c=r["fifa"];region=CONF.get(r["source"].get("country"))
        if not region:continue
        for name in (r["source"].get("name",""),*aliases.get(c,()),*ALIASES.get(c,())):
            if name:result.setdefault((region,norm(name)),set()).add(c)
    return {name:next(iter(codes)) for name,codes in result.items() if len(codes)==1}

def lead(source,url,raw,**kw):
    return dict(source=source,url=url,label=source+" FM26 research reference",
                scope="FM_MOD_LEAD_NOT_REAL_EDITION_OR_RULE",
                pageSha256=hashlib.sha256(raw).hexdigest(),**kw)

def master(raw,region,names):
    lines=Lines(raw).lines()
    if not any(re.fullmatch(r"@SECTION@ Football Manager rule files for "+region+r" v26\.\d+",s,re.I) for s in lines):raise ValueError("MasterIgor region/version heading mismatch")
    start=next((i for i,s in enumerate(lines) if s.rstrip(":").casefold()=="@section@ available rule files"),None)
    if start is None:raise ValueError("MasterIgor section missing")
    url=MASTER+region+"/readme.html";result={};pending=False;section=None
    for s in lines[start+1:]:
        if s.startswith("@SECTION@"):break
        if s.startswith("@STOP@"):section=None;pending=False;continue
        if s.startswith("@FLAG@"):
            section=None;pending=True;s=s[6:].strip()
            if not s:continue
        if pending:
            pending=False
            m=re.fullmatch(r"(.+?)\s+(v26\.\d+)",s,re.I)
            if not m:continue
            country=names.get((region,norm(m[1])))
            if country:
                section=lead("MasterIgor",url,raw,country=country,region=region,
                    sourceVersion=m[2],sourceSeasonClaim="2025/26",
                    competitions=[],reviewTopics=[],adaptationMentioned=False)
                if country in result:raise ValueError("Ambiguous country section")
                result[country]=section
            continue
        if section is None:continue
        m=re.fullmatch(r"\[(\d+(?:\.\d+)?|Cup)\]\s+(.+)",s,re.I)
        if m:
            name=re.sub(r"\s*\(\*?ADDED\)\s*$","",m[2],flags=re.I).strip()
            if name and len(name)<=120 and not BAD.search(name) and not re.search(r"\b(futsal|beach soccer)\b",name,re.I):
                kind="CUP_UNCLASSIFIED" if m[1].casefold()=="cup" else "LEAGUE_TIER"
                entry={"name":name,"sourceCategory":kind}
                if kind=="LEAGUE_TIER":entry["sourceTier"]=m[1]
                if entry not in section["competitions"]:section["competitions"].append(entry)
        elif not s.startswith("[") and not BAD.search(s):
            section["reviewTopics"]=sorted(set(section["reviewTopics"])|set(topics(s)))
            section["adaptationMentioned"] |= bool(RISK.search(s))
            # Numerical mentions, not certified rules. Require exact competition
            # name in the note, never assign general country notes to a cup.
            for item in section["competitions"]:
                if len(norm(item["name"]))<5 or norm(item["name"]) not in norm(s):continue
                facts={}
                for key,pattern in (("groupsMentioned",r"\b(\d{1,2})\s+groups\b"),
                    ("qualifierTeamsMentioned",r"\bqualifier tournament with (\d{1,2})\s+teams\b"),
                    ("additionalDerbiesMentioned",r"\b(\d{1,2})\s+additional derby matches\b")):
                    matches={int(x) for x in re.findall(pattern,s,re.I)}
                    if len(matches)==1:facts[key]=next(iter(matches))
                if facts:
                    values=item.setdefault("modStatementLeads",[])
                    v={"mentions":facts,"adaptationMentioned":bool(RISK.search(s))}
                    if v not in values:values.append(v)
    if not result:raise ValueError("MasterIgor country markup unsupported")
    return {c:[p] for c,p in result.items() if p["competitions"] or p["reviewTopics"]}

def falso(raw,names):
    lines=[s.removeprefix("@SECTION@ ") for s in Lines(raw).lines()]
    if not any("megapack 2026 by falsobohemio" in s.casefold() for s in lines):raise ValueError("Falsobohemio page identity mismatch")
    start=next((i for i,s in enumerate(lines) if s.upper()=="SOUTH AMERICA"),None)
    end=next((i for i,s in enumerate(lines) if start is not None and i>start and s.upper()=="FANTASY BUT REALISTIC 2D AND 3D KITS"),None)
    if start is None or end is None:raise ValueError("Falsobohemio men's section boundaries missing")
    region=None;out={}
    for s in lines[start:end]:
        if s.upper() in ("SOUTH AMERICA","AFRICA",*REGIONS):
            region={"SOUTH AMERICA":"CONMEBOL","AFRICA":"CAF"}.get(s.upper(),s.upper());continue
        if not region or BAD.search(s):continue
        m=re.fullmatch(r"(.+?)(?:\s*:\s*|\s+-\s+)(.+)",s)
        if not m:continue
        c=names.get((region,norm(m[1])))
        if not c:continue
        tiers=re.search(r"\b(\d{1,2})\s+tiers?\b",m[2],re.I)
        item=lead("Falsobohemio",FALSO,raw,country=c,region=region,
            claimedTierCount=int(tiers[1]) if tiers else None,
            reviewTopics=topics(m[2]),adaptationMentioned=bool(RISK.search(m[2])),
            sourceScope="MEN_DOMESTIC_MOD_STRUCTURE_ONLY")
        if item not in out.setdefault(c,[]):out[c].append(item)
    if not out:raise ValueError("Falsobohemio no scoped country rows")
    return out

def dave(raw):
    joined=" ".join(Lines(raw).lines())
    if not re.search(r"increase realism megapack",joined,re.I) or not re.search(r"football manager 2026",joined,re.I):raise ValueError("Daveincid FM26 page identity mismatch")
    return lead("Daveincid",DAVE,raw,reviewTopics=topics(joined),
        sourceScope="GLOBAL_MOD_DESCRIPTION_NOT_COUNTRY_OR_CLUB_EVIDENCE")

class Sources:
    def __init__(self,cache,national,aliases):
        self.cache=cache;self.names=lookup(national,aliases);self.catalogues={};self.errors={};self.global_lead=None;self.loaded=False
    def load(self):
        if self.loaded:return
        self.loaded=True
        routes=[("MasterIgor/"+r,MASTER+r+"/readme.html",lambda raw,r=r:master(raw,r,self.names)) for r in REGIONS]
        routes += [("Falsobohemio",FALSO,lambda raw:falso(raw,self.names)),("Daveincid",DAVE,dave)]
        for label,url,parse in routes:
            try:
                report(label,"SEARCHING","public documentation; not FMF")
                with time_budget(20):value=parse(self.cache.get(url))
                if label=="Daveincid":self.global_lead=value
                else:
                    for c,items in value.items():self.catalogues.setdefault(c,[]).extend(items)
                report(label,"FOUND","global club/mod reference only" if label=="Daveincid" else f"{len(value)} country leads; not certified")
            except Exception as exc:
                self.errors[label]=str(exc);report(label,"ERROR",str(exc))
    def country(self,country,year):
        if year!=2026:return []
        self.load()
        return list(self.catalogues.get(country,[]))
    def row(self,row):
        ident=row.get("technicalIdentity",{});country=ident.get("country") or row.get("country")
        cat=ident.get("category")
        if row.get("world")!="2627" or not country or cat not in ("LEAGUE_DIVISION","NATIONAL_CUP","SUPERCUP","LEAGUE_CUP"):return []
        tier=re.fullmatch(r"div([1-9]\d*)",ident.get("detail",""))
        out=[]
        for p in self.country(country,2026):
            if p["source"]=="MasterIgor":
                wanted="LEAGUE_TIER" if cat=="LEAGUE_DIVISION" else "CUP_UNCLASSIFIED"
                matches=[x for x in p["competitions"] if x["sourceCategory"]==wanted and
                    (norm(x["name"])==norm(row.get("name","")) or
                     (wanted=="LEAGUE_TIER" and tier and x.get("sourceTier")==tier[1]))]
                if matches:out.append(dict(p,competitions=matches))
            elif p["source"]=="Falsobohemio" and cat=="LEAGUE_DIVISION" and tier and int(tier[1])<=(p.get("claimedTierCount") or 0):out.append(p)
        return out

def selftest():
    national=[{"fifa":c,"source":{"name":n,"country":k}} for c,n,k in (("BOL","Bolivia",82),("BHU","Bhutan",84),("CGO","Congo",81),("COD","DR Congo",81))]
    names=lookup(national,{})
    m=("<h1>Football Manager rule files for CONMEBOL v26.2</h1><h3>Available rule files:</h3>"
       '<p><img class="flag" src="https://flagpedia.net/bo.png"> Bolivia v26.2<br>[1] Division Profesional<br>[Cup] Copa Bolivia<br>[2.1] Regional League<br>[Yth] Youth League</p>'
       "<ul><li>Copa Bolivia uses 4 groups and 2 additional derby matches paired as best as I could</li></ul>"
       '<p><img class="flag" src="https://flagpedia.net/xx.png"> Unknown v26.1<br>[1] Wrong</p>').encode()
    p=master(m,"CONMEBOL",names)["BOL"][0]
    assert len(p["competitions"])==3 and p["adaptationMentioned"] and "format" in p["reviewTopics"]
    assert p["competitions"][1]["modStatementLeads"][0]["mentions"]=={"groupsMentioned":4,"additionalDerbiesMentioned":2}
    try:master(m,"CAF",names)
    except ValueError:pass
    else:raise AssertionError("wrong region")
    f=("<h1>Megapack 2026 by falsobohemio</h1><p>SOUTH AMERICA</p><ul><li>Bolivia: real format (2 tiers)</li></ul>"
       "<p>AFRICA</p><ul><li>Congo - 2 tiers</li><li>DR Congo - 1 tier</li></ul>"
       "<p>FANTASY BUT REALISTIC 2D AND 3D KITS</p><li>Bolivia - 9 tiers</li>"
       "<p>WOMEN'S FORMATS</p><li>Bhutan - 8 tiers</li>").encode()
    q=falso(f,names)
    assert q["BOL"][0]["claimedTierCount"]==2 and q["CGO"][0]["claimedTierCount"]==2 and q["COD"][0]["claimedTierCount"]==1 and "BHU" not in q
    class Fake:
        def __init__(self):self.calls=[]
        def get(self,url):
            self.calls.append(url)
            if url==FALSO:return f
            if url==DAVE:return b"Increase Realism Megapack Football Manager 2026 CLUB DATA FIXES"
            if "CONMEBOL" in url:return m
            raise ValueError("test blocked source")
    fake=Fake();s=Sources(fake,national,{})
    assert not s.country("BOL",1996) and not fake.calls
    assert len(s.country("BOL",2026))==2 and len(fake.calls)==8 and "clubMetadata" in s.global_lead["reviewTopics"]
    assert len(s.country("BOL",2026))==2 and len(fake.calls)==8 and len(s.errors)==5
    row={"world":"2627","name":"Copa Bolivia","technicalIdentity":{"country":"BOL","category":"NATIONAL_CUP"}}
    assert len(s.row(row))==1 and not s.row(dict(row,name="Other cup")) and not s.row(dict(row,world="9697"))
