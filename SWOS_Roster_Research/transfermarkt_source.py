"""Direct public Transfermarkt pages; bounded requests; research candidates only."""
import hashlib
import json
import re
import time
import urllib.error
import urllib.request
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import urlsplit
from alternative_sources import norm, BAD, add_unique
from http_retry import fetch
from research_status import report, outcome

BASE="https://www.transfermarkt.com"
COUNTRY="/wettbewerbe/national/wettbewerbe/"
SEED={"BHU":"22","KGZ":"90","SEN":"149"}
COUNTRY_DIRECTORY="https://transfermarkt-api.fly.dev/countries/"
ALIASES={"NED":"Netherlands","FRO":"Faroe Islands","IRL":"Ireland","KOR":"South Korea","PRK":"North Korea","TPE":"Chinese Taipei","USA":"United States","COD":"DR Congo","CGO":"Congo","CIV":"Ivory Coast","SWZ":"Eswatini","TUR":"Türkiye","VIE":"Vietnam"}
TIER={"First Tier":1,"Second Tier":2,"Third Tier":3,"Fourth Tier":4}
SCOPE="SEASON_CANDIDATE_NOT_JULY1"
VOID={"area","base","br","col","embed","hr","img","input","link","meta","param","source","track","wbr"}

class Node:
    def __init__(self,tag="",attrs=()):
        self.tag=tag;self.attrs=dict(attrs);self.parts=[];self.children=[];self.parent=None
    def text(self):
        return " ".join(" ".join(x.text() if isinstance(x,Node) else x for x in self.parts).split())
    def all(self,tag):
        for n in self.children:
            if n.tag==tag:yield n
            yield from n.all(tag)

class Page(HTMLParser):
    def __init__(self,html):
        super().__init__(convert_charrefs=True);self.root=Node();self.stack=[self.root];self.feed(html);self.close()
    def handle_starttag(self,tag,attrs):
        if tag in ("tr","td","th","option") and self.stack[-1].tag==tag:self.stack.pop()
        n=Node(tag,attrs);n.parent=self.stack[-1];self.stack[-1].children.append(n);self.stack[-1].parts.append(n)
        if tag not in VOID:self.stack.append(n)
    def handle_startendtag(self,tag,attrs):
        self.handle_starttag(tag,attrs)
        if tag not in VOID:self.handle_endtag(tag)
    def handle_endtag(self,tag):
        for i in range(len(self.stack)-1,0,-1):
            if self.stack[i].tag==tag:
                del self.stack[i:];break
    def handle_data(self,data):self.stack[-1].parts.append(data)

def parse(html):
    p=Page(html).root
    if re.search(r"verify that you're not a robot|enable javascript to continue|access denied|captcha", " ".join(x.text() for x in p.all("h1"))+" "+" ".join(x.text() for x in p.all("title")),re.I):
        raise PermissionError("Transfermarkt access challenge")
    if not list(p.all("table")):
        if re.search(r"not a robot|captcha|access denied|javascript is disabled",p.text(),re.I):raise PermissionError("Transfermarkt access challenge")
        raise ValueError("No Transfermarkt table")
    return p

def country_options(page):
    matches=[]
    for s in page.all("select"):
        rows=[(o.attrs.get("value",""),o.text(), "selected" in o.attrs) for o in s.all("option")]
        hint=" ".join(s.attrs.get(k,"") for k in ("id","name","class")).lower()
        if any(v=="22" and norm(n)=="bhutan" for v,n,_ in rows) or re.search(r"land[_-]?id|country",hint):
            matches.append([(v,n,selected) for v,n,selected in rows if re.fullmatch(r"[1-9][0-9]*",v) and n])
    unique={tuple(x) for x in matches if x}
    if len(unique)!=1:raise ValueError("Country selector unavailable or ambiguous")
    return list(next(iter(unique)))

def directory_ids(raw):
    data=json.loads(raw)
    out={}
    for row in data["countries"]:
        c=row.get("fifaCode",row.get("fifa_code"))
        i=row.get("id")
        if row.get("isHistorical",row.get("is_historical")) is not False:continue
        if not isinstance(c,str) or not re.fullmatch(r"[A-Z]{3}",c) or type(i)!=int or i<=0 or not row.get("name"):continue
        out.setdefault(c,set()).add(str(i))
    if len(out)<30:raise ValueError("Incomplete country directory")
    return {c:next(iter(ids)) for c,ids in out.items() if len(ids)==1}

def country_page_matches(page,country_id):
    try:
        if any(v==country_id and selected for v,n,selected in country_options(page)):return True
    except ValueError:pass
    # A canonical country route or OG URL is page-level evidence, unlike
    # arbitrary links to other countries in the page.
    urls=[n.attrs.get("href","") for n in page.all("link") if "canonical" in n.attrs.get("rel","").split()]
    urls += [n.attrs.get("content","") for n in page.all("meta") if n.attrs.get("property")=="og:url"]
    ids=set()
    for url in urls:
        u=urlsplit(url)
        if u.netloc not in ("","www.transfermarkt.com","transfermarkt.com"):continue
        m=re.search(r"/national/wettbewerbe/([1-9][0-9]*)(?:/|$)",u.path)
        if m:ids.add(m.group(1))
    return ids=={country_id}

def links(node,pattern):
    found={}
    for a in node.all("a"):
        href=a.attrs.get("href","");u=urlsplit(href)
        if u.netloc and u.netloc!="www.transfermarkt.com":continue
        m=re.search(pattern,u.path)
        name=a.text().strip()
        if m and name and not BAD.search(name):
            found.setdefault(m.group(1),name)
    return found

def leagues(page,country_id,maxlevel):
    if not country_page_matches(page,country_id):
        raise ValueError("Country page identity mismatch")
    out={}
    for table in page.all("table"):
        level=None
        for tr in table.all("tr"):
            txt=tr.text()
            if txt in TIER:level=TIER[txt];continue
            if txt in ("Domestic Cup","Super Cup") or "Youth" in txt or "Women" in txt:level=None
            if level is not None and level<=maxlevel:
                for code,name in links(tr,r"/startseite/wettbewerb/([A-Z0-9]+)(?:/|$)").items():
                    out[code]=(name,level)
    return out

def page_route_ids(page,kind):
    urls=[n.attrs.get("href","") for n in page.all("link") if "canonical" in n.attrs.get("rel","").split()]
    urls += [n.attrs.get("content","") for n in page.all("meta") if n.attrs.get("property")=="og:url"]
    pattern=r"/startseite/wettbewerb/([A-Z0-9]+)(?:/|$)" if kind=="clubs" else r"/kader/verein/([1-9][0-9]*)(?:/|$)"
    ids=set()
    for url in urls:
        u=urlsplit(url)
        if u.netloc not in ("","www.transfermarkt.com","transfermarkt.com"):continue
        m=re.search(pattern,u.path)
        if m:ids.add(m.group(1))
    return ids

def season_tokens(text):
    # Read whole season tokens, never the trailing year of 2025/2026.
    found=set()
    for m in re.finditer(r"(?<![\d/–-])(?:20)?\d{2}[/–-](?:20)?\d{2}(?![\d/–-])|(?<![\d/–-])20\d{2}(?![\d/–-])",text):
        t=m.group()
        if re.search(r"[/–-]",t):
            a,b=re.split(r"[/–-]",t)
            found.add(a[-2:]+"/"+b[-2:])
        else:found.add(t)
    return found

def season_label(page,kind,identity,expected_id=None):
    ids=page_route_ids(page,kind)
    if expected_id and ids and ids!={expected_id}:raise ValueError("Page ID mismatch")
    exact=bool(expected_id and ids=={expected_id})
    headings=[n.text() for tag in ("h1","h2","h3") for n in page.all(tag)]
    section=re.compile(r"^clubs\s*[-–:]\s*" if kind=="clubs" else r"^squad\b",re.I)
    relevant=[t for t in headings if section.search(t) and not BAD.search(t)]
    # A name change is accepted only for the same canonical competition/club ID.
    named=[t for t in relevant if norm(identity) in norm(t)]
    if not named and not (exact and len(relevant)==1):
        raise ValueError("Requested entity not evidenced in primary heading")
    relevant=named or relevant
    tokens=set().union(*(season_tokens(t) for t in relevant))
    if not tokens:
        titles=[n.text() for n in page.all("title")]
        titles += [n.attrs.get("content","") for n in page.all("meta") if n.attrs.get("property")=="og:title"]
        # Squad headings often omit the year; page title supplies it.
        matching=[t for t in titles if not BAD.search(t) and (norm(identity) in norm(t) or exact)]
        tokens=set().union(*(season_tokens(t) for t in matching))
    if tokens=={"26/27"}:return "2026/27"
    if tokens=={"2026"}:return "2026"
    raise ValueError("Requested season not evidenced or ambiguous: "+str(sorted(tokens)))

def table_links(page,pattern):
    # Use primary schema, not the count of tables with class items.
    # Header columns from nested tables are excluded.
    players="/profil/spieler/" in pattern
    candidates=[]
    for t in page.all("table"):
        if "items" not in t.attrs.get("class","").split():continue
        def own(n):
            p=n.parent
            while p is not None and p.tag!="table":p=p.parent
            return p is t
        headers={norm(n.text()) for n in t.all("th") if own(n)}
        if players:
            valid=("player" in headers and bool(headers & {"age","dateofbirthage","dateofbirth","position","marketvalue","nationality"}))
        else:
            valid=("club" in headers and bool(headers & {"squad","squadsize"}) and not bool(headers & {"pts","points","goals","w","d","l"}))
        if not valid:continue
        rows=links(t,pattern)
        if rows:candidates.append(rows)
    if len(candidates)!=1:raise ValueError("Primary data table missing or ambiguous: "+str(len(candidates)))
    return candidates[0]

class Transfermarkt:
    def __init__(self,cache,cp,results,national,known):
        self.cache=cache/"transfermarkt";self.cp=cp;self.results=results;self.known=known
        self.names={r["fifa"]:r["source"]["name"] for r in national}
        self.blocked=False;self.options=None;self.directory=None;self.discovered={};self.error_count=0
    def error(self,phase,exc):
        self.error_count+=1
        record={"phase":"Transfermarkt/"+phase,"error":str(exc)}
        if record not in self.results["errors"]:self.cp.append(self.results["errors"],record)
        self.cp.save()
        report("Transfermarkt "+phase,"ERROR",str(exc))
        if isinstance(exc,PermissionError) or isinstance(exc,urllib.error.HTTPError) and exc.code in (401,403,429):self.blocked=True
    def page(self,path):
        if self.blocked:raise PermissionError("Transfermarkt paused for this run")
        self.cache.mkdir(parents=True,exist_ok=True)
        p=self.cache/(hashlib.sha256(path.encode()).hexdigest()+".html")
        if p.exists() and time.time()-p.stat().st_mtime<86400:return parse(p.read_text(encoding="utf-8"))
        req=urllib.request.Request(BASE+path,headers={"User-Agent":"SWOS-Expanded-Roster-Research/1.0 (https://github.com/Giuzy/SWOS-Expanded/issues/2)","Accept":"text/html","Accept-Language":"en"})
        html=fetch(req,timeout=40,attempts=2,interval=5,limit=8*1024*1024).decode("utf-8",errors="replace")
        page=parse(html)
        tmp=p.with_suffix(".part");tmp.write_text(html,encoding="utf-8");tmp.replace(p)
        return page
    def country_id(self,c):
        if c in SEED:return SEED[c]
        if self.options is None:
            self.options=[]
            try:self.options=country_options(self.page(COUNTRY+"22"))
            except ValueError as exc:print("Transfermarkt country selector:",exc,flush=True)
        names={norm(self.names.get(c,"")),norm(ALIASES.get(c,""))}-{""}
        ids={v for v,n,_ in self.options if norm(n) in names}
        if len(ids)==1:return next(iter(ids))
        # Third-party public reference metadata only, never an access bypass
        # for blocked Transfermarkt HTML or evidence of league membership.
        if self.directory is None:
            self.directory={}
            try:
                p=self.cache/"country_directory.json"
                if p.exists() and time.time()-p.stat().st_mtime<86400:
                    raw=p.read_bytes();self.directory=directory_ids(raw)
                else:
                    req=urllib.request.Request(COUNTRY_DIRECTORY,headers={"User-Agent":"SWOS-Expanded/1.0","Accept":"application/json"})
                    raw=fetch(req,timeout=30,attempts=2,interval=3,limit=2*1024*1024)
                    self.directory=directory_ids(raw)
                    self.cache.mkdir(parents=True,exist_ok=True)
                    tmp=p.with_suffix(".part");tmp.write_bytes(raw);tmp.replace(p)
                print("Transfermarkt country IDs:",len(self.directory),flush=True)
            except Exception as exc:print("Transfermarkt reference directory unavailable:",exc,flush=True)
        if c in self.directory:return self.directory[c]
        raise ValueError("Country identity unavailable: "+c)
    def clubs(self,c,maxlevel):
        key=(c,maxlevel)
        if key in self.discovered:
            report("Transfermarkt clubs "+c,"SKIPPED","reusing in-run club discovery")
            return self.discovered[key]
        country_id=self.country_id(c)
        out=[]
        for code,(name,level) in leagues(self.page(COUNTRY+country_id+"/saison_id/2026"),country_id,maxlevel).items():
            path="/-/startseite/wettbewerb/"+code+"/saison_id/2026"
            try:
                report("Transfermarkt "+c+"/"+code,"SEARCHING",name)
                before=len(out)
                page=self.page(path);season=season_label(page,"clubs",name,code)
                # Positive domestic tier inherited from exact country selector.
                for cid,club in table_links(page,r"/startseite/verein/([1-9][0-9]*)(?:/|$)").items():
                    out.append({"club":club,"transfermarktClubId":cid,"country":c,"competition":code,"level":level,"season":season,"source":BASE+path,"scope":SCOPE})
                report("Transfermarkt "+c+"/"+code,"FOUND" if len(out)>before else "NO DATA",f"{len(out)-before} eligible club candidates")
            except Exception as e:
                self.error(c+"/"+code,e)
                if self.blocked:break
        self.discovered[key]=out
        return out
    def players(self,entry,owner,known):
        path="/-/kader/verein/"+entry["transfermarktClubId"]+"/saison_id/2026"
        label="Transfermarkt players "+entry["country"]+"/"+entry["club"]
        report(label,"SEARCHING")
        before=len(owner.get("transfermarkt_players",[]));reviews=len(owner.get("name_matches",[]));found=0
        page=self.page(path)
        if season_label(page,"players",entry["club"],entry["transfermarktClubId"])!=entry["season"]:raise ValueError("Squad season mismatch")
        for pid,name in table_links(page,r"/profil/spieler/([1-9][0-9]*)(?:/|$)").items():
            if norm(name) in known:continue
            found+=1
            row=dict(entry,transfermarktPlayerId=pid,playerLabel=name,source=BASE+path)
            add_unique(self.cp,owner,"transfermarkt_players",row)
        self.cp.save()
        report(label,*outcome(found,len(owner.get("transfermarkt_players",[]))-before,len(owner.get("name_matches",[]))-reviews))
    def acquire(self,phase,targets):
        if self.blocked:
            report("Transfermarkt "+phase,"SKIPPED","source paused after access error")
            return
        bycode={t["fifa"]:t for t in self.results["targets"]}
        if phase=="clubs":
            for t in targets:
                if self.blocked:break
                c=t["fifa"]
                label="Transfermarkt clubs "+c
                report(label,"SEARCHING")
                try:
                    failures=self.error_count;entries=self.clubs(c,1)
                    added=sum(int(add_unique(self.cp,bycode[c],"transfermarkt_clubs",row)) for row in entries)
                    self.cp.save()
                    status,detail=outcome(len(entries),added)
                    if self.error_count!=failures:report(label,"PARTIAL" if entries else "ERROR",status+"; "+detail+"; some competition requests failed")
                    else:report(label,status,detail)
                except Exception as e:self.error(c,e)
        elif phase=="players":
            for t in targets:
                if self.blocked:break
                owner=bycode[t["fifa"]]
                if not owner.get("transfermarkt_clubs"):report("Transfermarkt players "+t["fifa"],"SKIPPED","no discovered clubs")
                for row in owner.get("transfermarkt_clubs",[]):
                    if self.blocked:break
                    try:self.players(row,owner,set())
                    except Exception as e:self.error(t["fifa"]+"/"+row["transfermarktClubId"],e)
        elif phase=="extras":
            groups={}
            for uid,e in self.results["existing_club_gaps"].items():
                if uid in self.known and e.get("association"):
                    groups.setdefault(e["association"],[]).append((uid,e))
            for c,owners in sorted(groups.items()):
                if self.blocked:break
                try:
                    report("Transfermarkt extras "+c,"SEARCHING")
                    entries=self.clubs(c,4)
                    matches={}
                    for row in entries:matches.setdefault(norm(row["club"]),[]).append(row)
                    for uid,e in sorted(owners,key=lambda x:(not bool(x[1].get("test_slots")),x[1].get("real_names",0),x[0])):
                        if self.blocked:break
                        rows=matches.get(norm(e["name"]),[])
                        if len({x["transfermarktClubId"] for x in rows})!=1 or len(rows)!=1:
                            report("Transfermarkt extras "+c+"/"+e["name"],"SKIPPED","no unique club identity")
                            continue
                        row=rows[0];add_unique(self.cp,e,"transfermarkt_clubs",row);self.cp.save()
                        try:self.players(row,e,self.known[uid])
                        except Exception as exc:self.error(c+"/"+row["transfermarktClubId"],exc)
                except Exception as exc:self.error(c,exc)
        else:raise ValueError("Unknown research phase")

def selftest():
    country=parse('<select><option value="22" selected>Bhutan</option><option value="90">Kyrgyzstan</option></select><table><tr><td>First Tier</td></tr><tr><td><a href="/bhutan/startseite/wettbewerb/BH1L">Bhutan Premier League</a></td></tr><tr><td>Domestic Cup</td></tr><tr><td><a href="/cup/startseite/wettbewerb/BHPO">Cup</a></td></tr></table>')
    assert leagues(country,"22",1)=={"BH1L":("Bhutan Premier League",1)}
    no_selector=parse('<link rel="canonical" href="https://www.transfermarkt.com/senegal/national/wettbewerbe/149"><table><tr><td>First Tier</td></tr><tr><td><a href="/ligue-1/startseite/wettbewerb/SEN1">Ligue 1</a></td></tr></table>')
    assert leagues(no_selector,"149",1)=={"SEN1":("Ligue 1",1)}
    assert not country_page_matches(no_selector,"90")
    payload={"countries":[{"id":i+1,"fifaCode":"A"+chr(65+i//26)+chr(65+i%26),"name":"Country","isHistorical":False} for i in range(32)]}
    payload["countries"].append({"id":999,"fifaCode":"AAA","name":"Duplicate","isHistorical":False})
    assert "AAA" not in directory_ids(json.dumps(payload))
    try:leagues(country,"90",1)
    except ValueError:pass
    else:raise AssertionError("Wrong country accepted")
    club=parse('<h2>Clubs - Bhutan Premier League 2026</h2><table class="items"><tr><th>Club</th><th>Squad</th></tr><tr><td><a href="/paro/startseite/verein/123">Paro FC</a></td></tr></table>')
    assert season_label(club,"clubs","Bhutan Premier League")=="2026"
    assert table_links(club,r"/startseite/verein/([1-9][0-9]*)(?:/|$)")=={"123":"Paro FC"}
    squad=parse('<h2>Squad Paro FC 2026</h2><table class="items"><tr><th>Player</th><th>Age</th></tr><tr><td><a href="/a/profil/spieler/456">Real Name</a></td></tr></table>')
    assert season_label(squad,"players","Paro FC")=="2026"
    assert table_links(squad,r"/profil/spieler/([1-9][0-9]*)(?:/|$)")=={"456":"Real Name"}
    multi=parse('<link rel="canonical" href="/jogorku-liga/startseite/wettbewerb/KG1L"><h2>Clubs - Jogorku Liga 2026</h2><table class="items"><tr><th>Club</th><th>Pts</th></tr><tr><td><a href="/other/startseite/verein/999">Other</a></td></tr></table><table class="items"><tr><th>Club</th><th>Squad</th></tr><tr><td><a href="/real/startseite/verein/123">Real</a></td></tr></table>')
    assert season_label(multi,"clubs","Kyrgyz Premier League","KG1L")=="2026"
    assert table_links(multi,r"/startseite/verein/([1-9][0-9]*)(?:/|$)")=={"123":"Real"}
    assert season_tokens("2025/2026")=={"25/26"}
    assert season_tokens("2026-2027")=={"26/27"}
    title=parse('<title>Paro FC - Detailed squad 2026 | Transfermarkt</title><h2>Squad Paro FC</h2><table></table>')
    assert season_label(title,"players","Paro FC")=="2026"
    for bad in (lambda:season_label(multi,"clubs","Kyrgyz Premier League","OTHER"),lambda:season_label(parse('<h2>Clubs - Bhutan Premier League 2025/2026</h2><table></table>'),"clubs","Bhutan Premier League"),lambda:table_links(parse('<table class="items"><tr><th>Club</th><th>Pts</th></tr></table>'),r"/startseite/verein/([1-9][0-9]*)(?:/|$)")):
        try:bad()
        except ValueError:pass
        else:raise AssertionError("Wrong ID/season/table accepted")
    try:parse("<h1>JavaScript is disabled</h1><p>verify that you're not a robot</p>")
    except PermissionError:pass
    else:raise AssertionError("Challenge accepted")
