"""Federation/league document discovery. Candidates only, never sporting evidence."""
import hashlib, ipaddress, re, time, urllib.request
from html.parser import HTMLParser
from urllib.parse import urlsplit, urljoin
from alternative_sources import norm, BAD
from http_retry import fetch, time_budget
from research_status import report

FIFA="https://inside.fifa.com/en/about-fifa/associations/"
FIFA_FALLBACK="https://inside.fifa.com/about-fifa/associations/"
RSSSF="https://www.rsssf.org/miscellaneous/fas-urls.html"
REFERENCES={
 "FIFA Professional Football Landscape":"https://inside.fifa.com/advancing-football/professional-landscape",
 "UEFA Intelligence Centre":"https://intellcentrecatalogue.uefa.com/league-formats-and-structures/",
 "Footy Contacts":"https://www.footycontacts.com/football-league-systems"}
SEEDS={
 "CAM":("https://www.cpl-cambodia.com/download",),
 "BLZ":("https://plbd1.com/regulations-statutes-of-the-premier-league-of-belize",),
 "BHU":("https://bhutanfootball.org/competition-department/domestic-competitions/",)}
TOPICS={
 "name":r"competitions?|championship|liga|ligue|league|campeonato|championnat|meisterschaft|بطولة|リーグ|联赛",
 "identity":r"competitions?|championship|liga|ligue|league|campeonato|championnat|meisterschaft|بطولة|リーグ|联赛",
 "participants":r"clubs?|teams?|equip[ae]s?|vereine|mannschaften|squads?|équip[ea]s?|команд|球[队隊]|ក្លឹប|أندية",
 "players":r"players?|squads?|rosters?|jugadores|joueurs|jogadores|spieler|rose|calciatori|игрок|球员|لاعب|កីឡាករ",
 "format":r"format|structure|pyramid|regulations?|reglamento|règlement|regulamento|regolamento|spielordnung|wettbewerb|規則|规则|نظام",
 "regulation":r"rules?|regulations?|reglamento|règlement|regulamento|regolamento|statutes?|spielordnung|satzung|規則|规则|لوائح|ข้อบังคับ|បទបញ្ជា",
 "movement":r"promot|relegat|ascenso|descenso|subida|descida|montée|relégation|aufstieg|abstieg|promoz|retrocess",
 "qualification":r"qualif|clasifica|califica|classifica|qualifika|acesso|تأهل",
 "calendar":r"fixtures?|schedule|calendar|calendrier|calendario|spielplan|terminliste|расписание|جدول|赛程|日程",
 "evolution":r"history|historique|historia|archives?|geschichte|storia|歷史|历史",
 "seasonChange":r"season|temporada|saison|stagione|spielzeit",
}
TOPICS["roundDates"]=TOPICS["competitionDays"]=TOPICS["calendar"]
TOPICS["july1"]=r"standings?|table|clasificaci[oó]n|classement|classifica|tabelle|results?|resultados"
EXCLUDE=re.compile(r"\b(women|female|girls?|youth|junior|futsal|beach soccer|u[- ]?(?:1[5-9]|2[013])|feminin|féminin|femenin[oa]|feminino|frauen|licen[cs]ing|agents?|referees?|stadium|marketing|media)\b",re.I)

def public_url(url):
    try:
        p=urlsplit(url)
        if p.scheme not in ("http","https") or not p.hostname or p.username or p.password or p.port not in (None,80,443):return None
        host=p.hostname.lower()
        if "." not in host or host.endswith((".local",".internal",".localhost")) or host in ("localhost",):return None
        try:ipaddress.ip_address(host);return None
        except ValueError:pass
        if not re.fullmatch(r"[a-z0-9.-]+",host):return None
        return p._replace(fragment="").geturl()
    except ValueError:return None

def host(url):
    return (urlsplit(url).hostname or "").lower().removeprefix("www.")

class SameHost(urllib.request.HTTPRedirectHandler):
    def redirect_request(self,req,fp,code,msg,headers,newurl):
        if not public_url(newurl) or host(req.full_url)!=host(newurl):
            raise ValueError("Cross-domain redirect not accepted: "+newurl)
        return super().redirect_request(req,fp,code,msg,headers,newurl)

class Cache:
    """Separate per BAT, shared retry/budget policy; no cross-domain redirects."""
    def __init__(self,path,refresh=False):
        self.path=path;self.refresh=refresh;self.memo={}
        self.opener=urllib.request.build_opener(SameHost()).open
    def get(self,url):
        if not public_url(url):raise ValueError("Invalid public source URL")
        if url in self.memo:return self.memo[url]
        self.path.mkdir(parents=True,exist_ok=True)
        p=self.path/(hashlib.sha256(url.encode()).hexdigest()+".bin")
        if p.exists() and not self.refresh and time.time()-p.stat().st_mtime<86400:raw=p.read_bytes()
        else:
            req=urllib.request.Request(url,headers={"User-Agent":"SWOS-Expanded/1.0","Accept":"text/html,application/pdf"})
            raw=fetch(req,timeout=12,attempts=1,limit=8*1024*1024,opener=self.opener)
            tmp=p.with_suffix(".part");tmp.write_bytes(raw);tmp.replace(p)
        self.memo[url]=raw
        return raw

class Page(HTMLParser):
    def __init__(self,raw):
        super().__init__(convert_charrefs=True);self.links=[];self.text=[];self.href=None;self.label=[];self.hidden=0
        self.feed(raw.decode("utf-8","replace"));self.close()
    def handle_starttag(self,tag,attrs):
        if tag in ("script","style"):self.hidden+=1
        if self.hidden:return
        if tag=="a":self.href=dict(attrs).get("href");self.label=[]
        if tag in ("br","p","div","li","h1","h2","h3","h4","tr"):self.text.append("\n")
    def handle_endtag(self,tag):
        if tag in ("script","style"):self.hidden=max(0,self.hidden-1)
        if self.hidden:return
        if tag=="a" and self.href:
            self.links.append((self.href," ".join(" ".join(self.label).split())));self.href=None
        if tag in ("p","div","li","h1","h2","h3","h4","tr"):self.text.append("\n")
    def handle_data(self,data):
        if self.hidden:return
        self.text.append(data)
        if self.href:self.label.append(data)
    def visible(self):return "".join(self.text)

def names_for(national,aliases):
    out={}
    for r in national:
        code=r["fifa"];out[code]={norm(n) for n in (r["source"]["name"],*aliases.get(code,())) if n}
    return out

def fifa_sites(raw,code,names):
    page=Page(raw);text=norm(page.visible())
    lines=[norm(x) for x in page.visible().splitlines() if x.strip()]
    if not any(n==line or (n in line and any(k in line for k in ("footballfederation","footballassociation","federationoffootball","associationfootball"))) for n in names for line in lines):raise ValueError("FIFA country name not found")
    if "memberassociation" not in text:raise ValueError("FIFA association page not identified")
    if code=="CGO" and any("democraticrepublicofthecongo" in x or "drcongo" in x for x in lines):raise ValueError("Ambiguous Congo association")
    sites=[]
    for href,label in page.links:
        u=public_url(urljoin(FIFA+code,href))
        if not u or host(u).endswith("fifa.com") or host(u) in ("facebook.com","instagram.com","twitter.com","x.com","youtube.com","linkedin.com"):continue
        # Explicit website label or visible domain, not arbitrary news links.
        if norm(label) in ("website","officialwebsite") or host(u) in label.lower():
            if u not in sites:sites.append(u)
    if len({host(u) for u in sites})!=1:raise ValueError("FIFA website absent or ambiguous")
    return sites[:1]

def rsssf_sites(raw,all_names):
    # RSSSF uses country blocks separated by blank lines; only exact, unique
    # first-line names; stale directory URLs remain leads, never auto-crawled.
    page=Page(raw);out={}
    for block in re.split(r"\n\s*\n",page.visible()):
        lines=[s.strip() for s in block.splitlines() if s.strip()]
        if not lines:continue
        matches=[c for c,n in all_names.items() if norm(lines[0]) in n]
        if len(matches)!=1:continue
        urls=[public_url(u.rstrip(".,;)")) for u in re.findall(r"https?://[^\s<>]+",block)]
        if any(urls):out.setdefault(matches[0],[]).extend(u for u in urls if u)
    return {k:list(dict.fromkeys(v)) for k,v in out.items()}

def match_topics(label,fields):
    if not label or BAD.search(label) or EXCLUDE.search(label):return []
    return sorted(f for f in fields if f in TOPICS and re.search(TOPICS[f],label,re.I))

def record(source,url,country,label,fields,**extra):
    return dict(source=source,url=url,country=country,label=label,reviewTopics=sorted(fields),
                scope="OFFICIAL_DOMAIN_CANDIDATE_NOT_FIELD_EVIDENCE",
                sourceScope="OFFICIAL_DOMAIN_CANDIDATE_NOT_FIELD_EVIDENCE",**extra)

class Sources:
    def __init__(self,cache,national,aliases):
        self.cache=cache;self.names=names_for(national,aliases);self.conf={r["fifa"]:r["source"].get("country") for r in national}
        self.errors={};self.directory=None;self.references=None;self.pages={}
    def attempt(self,label,url):
        try:
            report(label,"SEARCHING",url)
            return self.cache.get(url)
        except Exception as e:
            self.errors[label]=str(e);report(label,"ERROR",str(e));return None
    def global_sources(self):
        if self.directory is None:
            with time_budget(15):raw=self.attempt("RSSSF association directory",RSSSF)
            self.directory=rsssf_sites(raw,self.names) if raw else {}
        if self.references is None:
            self.references={}
            for label,url in REFERENCES.items():
                with time_budget(12):raw=self.attempt(label,url)
                if raw:self.references[label]=(url,hashlib.sha256(raw).hexdigest())
    def country(self,code,fields,year=2026):
        fields=set(fields)
        if year!=2026 or code not in self.names or not fields:return []
        self.global_sources()
        if code not in self.pages:
            result=[]
            with time_budget(30):
                sites=[]
                for prefix in (FIFA,FIFA_FALLBACK):
                    profile=prefix+code
                    raw=self.attempt("FIFA association "+code+" "+prefix,profile)
                    if raw:
                        try:sites=fifa_sites(raw,code,self.names[code])
                        except ValueError as e:
                            self.errors["FIFA association "+code+" "+prefix]=str(e)
                            report("FIFA association "+code,"ERROR",str(e))
                    if sites:break
                for u in self.directory.get(code,[]):
                    result.append(record("RSSSF association directory",u,code,"Historical federation address",[],directory=RSSSF,
                                         authority="CORROBORATED_BY_FIFA" if any(host(v)==host(u) for v in sites) else "UNVERIFIED_DIRECTORY_ADDRESS"))
                for u in sites:
                    result.append(record("FIFA Member Associations",u,code,"FIFA-listed federation address",[],directory=profile,authority="FIFA_LISTED_DOMAIN_NOT_CURRENT_CONTENT_VERIFIED"))
                seeds=list(dict.fromkeys(list(SEEDS.get(code,()))+sites))
                # Search the verified seed landing page or FIFA-listed domain;
                # maximum three HTML pages/country, no uncorroborated RSSSF crawl.
                queue=seeds[:3];visited=set()
                while queue and len(visited)<3:
                    u=queue.pop(0)
                    if u in visited:continue
                    visited.add(u);body=self.attempt("Federation "+code,u)
                    if not body:continue
                    if body.startswith(b"%PDF-"):continue
                    page=Page(body)
                    if not page.visible().strip():continue
                    result.append(record("Federation/league",u,code,"Inspected landing page",[],sha256=hashlib.sha256(body).hexdigest(),authority="CANDIDATE_CONTENT_NOT_IDENTITY_VERIFIED"))
                    links=[]
                    for href,label in page.links:
                        v=public_url(urljoin(u,href))
                        if not v or v==u:continue
                        if host(v)!=host(u) and not urlsplit(v).path.lower().endswith(".pdf"):continue
                        topics=match_topics(label+" "+urlsplit(v).path.replace("-"," "),TOPICS)
                        if not topics:continue
                        pdf=urlsplit(v).path.lower().endswith(".pdf")
                        # Cross-domain document links retained as review leads,
                        # never downloaded; only same-host HTML crawled.
                        result.append(record("Federation/league",v,code,label or urlsplit(v).path,topics,parent=u,
                                             documentKind="PDF_LINK" if pdf else "HTML_LINK",authority="LINK_ONLY_EDITION_AND_SECTION_UNVERIFIED"))
                        if not pdf and host(v)==host(u) and v not in visited:links.append((len(set(topics)&fields),v))
                    for score,v in sorted(set(links),key=lambda x:(-x[0],x[1])):
                        if score and v not in queue and len(queue)<3:queue.append(v)
            self.pages[code]=result
        out=[]
        for p in self.pages[code]:
            if not p["reviewTopics"] or fields.intersection(p["reviewTopics"]):
                out.append(dict(p,reviewTopics=sorted(fields.intersection(p["reviewTopics"]))))
        for label,(url,digest) in self.references.items():
            if label=="UEFA Intelligence Centre" and self.conf[code]!=80:continue
            topics=fields&{"name","identity","participants","format","movement","qualification","regulation","calendar","roundDates","competitionDays","evolution","seasonChange"}
            if topics:out.append(record(label,url,code,"Global methodology/reference; no country data extracted",topics,sha256=digest,authority="GLOBAL_REFERENCE_ONLY"))
        # Stable dedup by URL; maximum references per country/row, no raw results.
        unique={}
        for p in out:
            if p["url"] not in unique:unique[p["url"]]=p
        return list(unique.values())[:40]
    def row(self,row,fields):
        ident=row.get("technicalIdentity",{});code=ident.get("country") or row.get("country")
        if row.get("world")!="2627" or ident.get("category") not in ("LEAGUE_DIVISION","NATIONAL_CUP","SUPERCUP","LEAGUE_CUP"):return []
        return self.country(code,fields)

def selftest():
    national=[{"fifa":"CAM","source":{"name":"Cambodia","country":84}},{"fifa":"BLZ","source":{"name":"Belize","country":83}}]
    names=names_for(national,{})
    raw=b'<h1>Football Federation of Cambodia</h1><p>Member Associations</p><a href="https://the-ffc.org">Website</a>'
    assert fifa_sites(raw,"CAM",names["CAM"])==["https://the-ffc.org"]
    try:fifa_sites(raw,"BLZ",names["BLZ"])
    except ValueError:pass
    else:raise AssertionError("wrong country accepted")
    assert not public_url("http://127.0.0.1/x") and not public_url("https://user@example.com/x")
    assert match_topics("Reglamento campeonato",{"regulation"})==["regulation"]
    assert not match_topics("Women regulations",{"regulation"})
    directory=b"<pre>Cambodia\nFootball Federation\nhttps://the-ffc.org\n\nBelize\nFootball Federation\nhttps://example.com\n</pre>"
    assert rsssf_sites(directory,names)["BLZ"]==["https://example.com"]
    assert not Sources(None,national,{}).row({"world":"9697","technicalIdentity":{"country":"CAM","category":"LEAGUE_DIVISION"}},["format"])
    class Fake:
        def get(self,u):
            if u==FIFA+"CAM":return raw
            if u==RSSSF:return directory
            if u in REFERENCES.values():return b"<h1>Reference</h1>"
            if u==SEEDS["CAM"][0]:return b'<a href="https://cdn.example.com/rules.pdf">Competition Regulations 2026/27</a><a href="/women">Women regulations</a>'
            if u=="https://the-ffc.org":return b"<h1>Football Federation of Cambodia</h1>"
            raise AssertionError(u)
    s=Sources(Fake(),national,{})
    p=s.country("CAM",{"regulation"})
    assert any(x.get("documentKind")=="PDF_LINK" and x["reviewTopics"]==["regulation"] for x in p)
    assert not any(x["url"].endswith("/women") for x in p)
    assert not any(x["source"]=="UEFA Intelligence Centre" for x in p)
    assert not s.country("CAM",{"regulation"},1996)
