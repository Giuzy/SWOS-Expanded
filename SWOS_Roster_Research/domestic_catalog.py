"""Shared domestic identity/entrant research. Never writes runtime or results."""
import hashlib,json,re,time,urllib.parse,urllib.request
from pathlib import Path
from alternative_sources import norm,BAD
from http_retry import fetch,time_budget
from research_status import report
from transfermarkt_source import Page
from betexplorer_source import route as bet_route,packet as bet_packet,selftest as bet_selftest
from sports_sources import routes as sports_routes, packet as sports_packet, selftest as sports_selftest

WP="https://en.wikipedia.org/w/api.php"
WD="https://www.wikidata.org/w/api.php"
DIRECTORY="List of association football competitions"
UA="SWOS-Expanded-Research/1.3 (https://github.com/Giuzy/SWOS-Expanded/issues/2)"
MEN={"Q31930761","Q35554451"}
EXCLUDED={"Q606060","Q44418853","Q44418672","Q44418451","Q44418152","Q44408003","Q44417973","Q44417630","Q45053375","Q44417376","Q14510042","Q21152744","Q21198418","Q21198428","Q21193209","Q21198474","Q23905145"}
NON_DOMESTIC=re.compile(r"\b(defunct|discontinued|former|international|regional|district|super\s*cup|league cup|super shield|play[- ]?off|qualification|futsal|beach soccer|junior|school)\b",re.I)
ALIASES={"ATG":("Antigua and Barbuda",),"BIH":("Bosnia and Herzegovina",),"BRU":("Brunei",),"CGO":("Congo","Congo-Brazzaville","Republic of the Congo"),"COD":("DR Congo","Democratic Republic of the Congo","Congo-Kinshasa"),"CIV":("Ivory Coast","Côte d'Ivoire"),"CPV":("Cape Verde","Cabo Verde"),"CTA":("Central African Republic",),"EQG":("Equatorial Guinea",),"GAM":("Gambia","The Gambia"),"KOR":("South Korea","Korea Republic"),"PRK":("North Korea","Korea DPR"),"CHN":("China PR","China"),"HKG":("Hong Kong",),"MAC":("Macau","Macao"),"MKD":("North Macedonia",),"MYA":("Myanmar",),"PLE":("Palestine",),"SKN":("Saint Kitts and Nevis","St. Kitts and Nevis"),"LCA":("Saint Lucia",),"VIN":("Saint Vincent and the Grenadines",),"SWZ":("Eswatini","Swaziland"),"TLS":("East Timor","Timor-Leste"),"TRI":("Trinidad and Tobago",),"TCA":("Turks and Caicos Islands",),"VGB":("British Virgin Islands",),"VIR":("U.S. Virgin Islands","US Virgin Islands","United States Virgin Islands"),"USA":("United States",),"TPE":("Chinese Taipei","Taiwan"),"TAH":("Tahiti",),"VAN":("Vanuatu",),"SOL":("Solomon Islands",),"KSA":("Saudi Arabia",),"UAE":("United Arab Emirates",),"SRI":("Sri Lanka",),"TAN":("Tanzania",),"STP":("São Tomé and Príncipe","Sao Tome and Principe")}
# Positively inspected archive sections. No name-based guess of new routes.
RSSSF={
 ("AFG",2026):("https://www.rsssf.org/tablesa/afgh2026b.html","Afghanistan 2026",(("Afghanistan Champions League 2026","Afghanistan Champions League","LEAGUE_DIVISION",(("Final Table:","Halfway Table:","Table:"),),10,18),)),
 ("AIA",2026):("https://www.rsssf.org/tablesa/anguilla2026.html","Anguilla 2026",(("AFA Football League 2026","AFA Football League","LEAGUE_DIVISION",(("Final Table:","Halfway Table:","Table:"),),10,18),("President's Cup 2026","President's Cup","NATIONAL_CUP",("Group 1","Group 2"),5,4))),
 ("BHU",2026):("https://www.rsssf.org/tablesb/bhutan2026.html","Bhutan 2026",(("Premier League 2026","Bhutan Premier League","LEAGUE_DIVISION",(("Final Table:","Halfway Table:","Table:"),),10,18),)),
 ("CAM",2026):("https://www.rsssf.org/tablesc/cambod2027.html","Cambodia 2026/27",(("Cambodia Premier League 2026/27","Cambodia Premier League","LEAGUE_DIVISION",(("Table:","Final Table:"),),11,40),)),
}

def walk(node):
    for child in node.children:
        yield child
        yield from walk(child)

def rawtext(node):
    return "".join(x if isinstance(x,str) else "\n" if x.tag=="br" else rawtext(x) for x in node.parts)

def heading(node):
    return re.sub(r"\s*\[\s*edit\s*\]\s*$","",node.text(),flags=re.I).strip()

def competition_link(node):
    for a in node.all("a"):
        path=urllib.parse.urlsplit(a.attrs.get("href",""))
        if path.netloc not in ("","en.wikipedia.org") or path.fragment:continue
        if path.path.startswith("/wiki/"):title=path.path[6:]
        elif path.path.startswith("./"):title=path.path[2:]
        else:continue
        title=urllib.parse.unquote(title).replace("_"," ")
        name=a.text().strip()
        if name and title and ":" not in title and not BAD.search(name+" "+title):return title,name
    return None

def competition_kind(text):
    text=" ".join(text.casefold().split())
    if BAD.search(text) or NON_DOMESTIC.search(text):return None
    if re.match(r"^1st[- ]tier\b",text):return "LEAGUE_DIVISION"
    if re.match(r"^national cup\b",text):return "NATIONAL_CUP"
    return None

def directory_rows(html,national):
    lookup={}
    for r in national:
        c=r["fifa"]
        for name in (r["source"]["name"],*ALIASES.get(c,())):lookup.setdefault(norm(name),set()).add(c)
    output={};domestic=False;country=None;country_title=None;sub=None
    for node in walk(Page(html).root):
        if node.tag in ("h1","h2","h3"):
            domestic=node.tag=="h3" and heading(node).casefold()=="domestic"
            country=None;sub=None
        elif node.tag=="h4":
            country_title=heading(node);codes=lookup.get(norm(country_title),set())
            country=next(iter(codes)) if domestic and len(codes)==1 else None;sub=None
        elif node.tag in ("h5","h6"):
            sub=heading(node).casefold() if node.tag=="h5" else None
        elif country and node.tag in ("tr","li"):
            # Never extract a linked name from a different row or nested list.
            if node.tag=="tr":
                cells=[c for c in node.children if c.tag in ("td","th")]
                if len(cells)<3:continue
                marker=cells[0].text();link=competition_link(cells[1]);kind=competition_kind(cells[2].text())
                male=bool(re.search(r"\[\s*Sen\.\s*Men\s*\]",marker,re.I))
                if BAD.search(marker) or re.search(r"\b(youth|sen\.women)\b",marker,re.I):continue
            else:
                if sub not in ("leagues","cups") or list(node.all("li")):continue
                link=competition_link(node)
                if not link or not node.text().startswith(link[1]):continue
                kind=competition_kind(node.text()[len(link[1]):].lstrip(" :–—-"))
                male=bool(re.search(r"\bsenior male\b",link[1],re.I))
                if kind!=("LEAGUE_DIVISION" if sub=="leagues" else "NATIONAL_CUP"):continue
            if not link or not kind:continue
            title,name=link
            if BAD.search(name+" "+title) or NON_DOMESTIC.search(name+" "+title):continue
            if re.search(r"\b(defunct|discontinued|held between)\b",node.text(),re.I):continue
            row={"country":country,"countryTitle":country_title,"title":title,"name":name,"kind":kind,"seniorMale":male}
            if row not in output.setdefault(country,[]):output[country].append(row)
    return output

def ids(entity,prop):
    out=set()
    for row in entity.get("claims",{}).get(prop,[]):
        if row.get("rank")=="deprecated":continue
        value=row.get("mainsnak",{}).get("datavalue",{}).get("value",{})
        if isinstance(value,dict) and value.get("id"):out.add(value["id"])
    return out

def eligible_league(row):
    return str(row.get("class","")).rsplit("/",1)[-1] in MEN or (row.get("source")=="WikipediaDirectory" and row.get("directorySeniorMale") is True and row.get("directoryTier")==1 and bool(row.get("sourceUrl")))

def directory_evidence(row,entities,revision,year):
    title=row["title"];kind=row["kind"]
    source="https://en.wikipedia.org/w/index.php?oldid="+str(revision)+"#"+urllib.parse.quote(row["countryTitle"].replace(" ","_"))
    packet={"source":"WikipediaDirectory","sourceId":title,"country":row["country"],"kind":kind,"name":row["name"],"sourceUrl":source,"pageUrl":"https://en.wikipedia.org/wiki/"+urllib.parse.quote(title.replace(" ","_")),"identityEvidence":row["seniorMale"],"scope":"COMPETITION_IDENTITY_NOT_EDITION" if row["seniorMale"] else "DIRECTORY_IDENTITY_REVIEW"}
    entity=entities.get(title,{});country=entities.get(row["countryTitle"],{}).get("id")
    q=entity.get("id");classes=ids(entity,"P2094");sports=ids(entity,"P641")
    wrong=bool((classes|sports)&EXCLUDED)
    for prop in ("P3983","P576"):
        for claim in entity.get("claims",{}).get(prop,[]):
            if claim.get("rank")=="deprecated":continue
            value=claim.get("mainsnak",{}).get("datavalue",{}).get("value",{})
            if not isinstance(value,dict):continue
            if prop=="P3983" and kind=="LEAGUE_DIVISION" and value.get("amount") not in (None,"+1","1"):wrong=True
            if prop=="P576" and str(value.get("time","")).startswith("+") and value["time"][1:11]<f"{year}-07-01":wrong=True
    countries=ids(entity,"P17")
    if country and countries and countries!={country}:wrong=True
    if wrong:
        packet.update(identityEvidence=False,scope="DIRECTORY_IDENTITY_REVIEW",reason="WIKIDATA_CONFLICT")
        return packet,None
    if not (re.fullmatch(r"Q[1-9][0-9]*",str(q or "")) and re.fullmatch(r"Q[1-9][0-9]*",str(country or "")) and countries=={country} and (classes&MEN or row["seniorMale"])):return packet,None
    # Exact directory row supplies the tier when the Wikidata property is absent.
    packet.update(identityEvidence=True,scope="COMPETITION_IDENTITY_NOT_EDITION",wikidataId=q)
    seed={"source":"WikipediaDirectory","sourceUrl":source,"fifa":row["country"],"country":"http://www.wikidata.org/entity/"+country,"league":"http://www.wikidata.org/entity/"+q,"leagueLabel":row["name"],"kind":kind,"directorySeniorMale":row["seniorMale"],"directoryTier":1 if kind=="LEAGUE_DIVISION" else None}
    if classes&MEN:seed["class"]="http://www.wikidata.org/entity/"+sorted(classes&MEN)[0]
    return packet,seed

RANK=re.compile(r"^(\d+)\.\s*(.+?)\s{2,}(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+\d+\s*-\s*\d+\s+")
def rank_table(lines,at,count,max_games,allow_names=()):
    names=[]
    for line in lines[at+1:]:
        if not line:
            if names and len(names)==count:break
            continue
        if re.fullmatch(r"[-= ]+",line):continue
        m=RANK.match(line)
        if not m:break
        rank,name,played,w,d,l=m.groups()
        if int(rank)!=len(names)+1 or not 0<=int(played)<=max_games or int(w)+int(d)+int(l)!=int(played) or (BAD.search(name) and name not in allow_names) or "\ufffd" in name:raise ValueError("RSSSF unexpected rank/team row")
        names.append(name)
        if len(names)>count:raise ValueError("RSSSF unexpected participant count")
    if len(names)!=count or len({norm(x) for x in names})!=count:raise ValueError("RSSSF incomplete/ambiguous participant table")
    return sorted(names)

def rsssf_packets(raw,country,year,errors=None):
    url,title,routes=RSSSF[country,year]
    try:html=raw.decode("utf-8")
    except UnicodeDecodeError:html=raw.decode("cp1252")
    nodes=list(walk(Page(html).root))
    if not any(n.tag in ("title","h1","h2") and heading(n)==title for n in nodes):raise ValueError("RSSSF edition heading missing")
    packets=[]
    for section,name,kind,markers,count,max_games in routes:
        try:
            positions=[i for i,n in enumerate(nodes) if n.tag in ("h3","h4","h5") and heading(n)==section]
            if len(positions)!=1:raise ValueError("RSSSF section missing/ambiguous: "+section)
            blocks=[]
            for n in nodes[positions[0]+1:]:
                if n.tag in ("h1","h2","h3","h4","h5"):break
                if n.tag=="pre":blocks.append(rawtext(n))
            lines=[x.strip() for x in "\n".join(blocks).splitlines()]
            participants=[]
            # This one verified senior entrant is called Academy. Never relax the global youth filter.
            allowed=("BFF Academy FC",) if country=="BHU" and section=="Premier League 2026" else ()
            for marker in markers:
                alternatives=(marker,) if isinstance(marker,str) else marker
                tables=set();invalid=[]
                for i,line in enumerate(lines):
                    if line not in alternatives:continue
                    try:tables.add(tuple(rank_table(lines,i,count,max_games,allowed)))
                    except ValueError as exc:invalid.append(str(exc))
                if len(tables)!=1:raise ValueError("RSSSF table missing/ambiguous: "+section+"/"+"/".join(alternatives)+"; "+"; ".join(invalid))
                if invalid and errors is not None:errors.append(["RSSSF "+section,"Some table copies invalid: "+"; ".join(invalid)])
                participants.extend(next(iter(tables)))
            if len({norm(x) for x in participants})!=len(participants):raise ValueError("RSSSF duplicate entrants")
            season=title.rsplit(" ",1)[-1]
            packets.append({"source":"RSSSF","sourceId":name,"country":country,"kind":kind,"name":name,"season":season,"sourceUrl":url,"sourceSection":section,"identityEvidence":True,"scope":"SEASON_PARTICIPANT_LABELS_NOT_JULY1","participants":[{"name":n} for n in sorted(participants)]})
        except ValueError as exc:
            if errors is None:raise
            errors.append(["RSSSF "+section,str(exc)])
    return packets

def observation_key(p):
    return (p.get("source"),p.get("country"),p.get("kind"),p.get("sourceId"),p.get("name"),p.get("season"),p.get("scope"),p.get("wikidataId"),p.get("identityEvidence"),tuple(sorted((x.get("name",""),str(x.get("sourceId",""))) for x in p.get("participants",[]))),tuple(sorted(p.get("officialSourceLeads",[]))),tuple(sorted((x["field"],x["heading"],x["revision"],json.dumps(x.get("facts",[]),sort_keys=True)) for x in p.get("researchSectionLeads",[]))))

RULE_SECTIONS={
    "format":("format","competition format","league format","tournament format","competition system","league system"),
    "regulation":("rules","regulations","competition rules","competition regulations","tie-breaking criteria","tiebreakers"),
    "movement":("promotion and relegation","promotion/relegation","relegation and promotion"),
    "qualification":("qualification","qualification criteria","continental qualification"),
    "roundDates":("schedule","calendar","competition schedule"),
    "evolution":("history","historical background","format history"),
}
# Only direct, bounded textual statements in named sections. No inference
# from results, no automatic sporting status, and no claim of edition adoption.
def rule_facts(body,field):
    body=re.sub(r"<!--.*?-->|<ref\b[^>]*>.*?</ref>|<ref\b[^>]*/>"," ",body,flags=re.I|re.S)
    body=re.sub(r"\{\{[^{}]*\}\}"," ",body)
    body=re.sub(r"\[\[(?:[^|\]]+\|)?([^\]]+)\]\]",r"\1",body)
    body=" ".join(body.replace("'''","").replace("''","").split())
    if re.search(r"\b(women|youth|reserve|under[- ](?:17|18|19|20|21|23))\b",body,re.I):return []
    numbers={"one":1,"two":2,"three":3,"four":4,"five":5,"six":6,"seven":7,"eight":8,"nine":9,"ten":10}
    number=r"(\d{1,2}|one|two|three|four|five|six|seven|eight|nine|ten)"
    def integer(v):return int(v) if v.isdigit() else numbers[v.casefold()]
    patterns={
        "format":[("teams",r"\b(?:league|competition|tournament)\s+(?:consists of|is contested by|comprises)\s+"+number+r"\s+(?:teams|clubs)\b"),
                  ("meetingsPerOpponent",r"\b(?:each team|each club)\s+plays\s+(?:every other|each other)\s+(?:team|club)\s+"+number+r"\s+times\b")],
        "regulation":[("pointsWin",number+r"\s+points?\s+(?:are awarded\s+)?for\s+(?:a\s+)?win\b"),
                      ("pointsDraw",number+r"\s+points?\s+(?:are awarded\s+)?for\s+(?:a\s+)?draw\b"),
                      ("pointsLoss",number+r"\s+points?\s+(?:are awarded\s+)?for\s+(?:a\s+)?(?:loss|defeat)\b"),
                      ("substitutions",r"\b(?:up to|maximum of)\s+"+number+r"\s+substitutions\b")],
        "movement":[("relegated",r"\b(?:the\s+)?(?:bottom|last)\s+"+number+r"\s+(?:teams|clubs)\s+(?:are|will be)\s+relegated\b"),
                    ("promoted",r"\b(?:the\s+)?(?:top|first)\s+"+number+r"\s+(?:teams|clubs)\s+(?:are|will be)\s+promoted\b")],
    }
    found={}
    for key,pattern in patterns.get(field,[]):
        values={integer(m[1]) for m in re.finditer(pattern,body,re.I)}
        if len(values)==1:found[key]=next(iter(values))
    if field=="format":
        if re.search(r"\b(?:double round[- ]robin|each (?:team|club) plays (?:every other|each other) (?:team|club) twice)\b",body,re.I):found["doubleRoundRobinStated"]=True
    if field=="regulation":
        if re.search(r"\b(?:goal difference|head[- ]to[- ]head)\b",body,re.I):
            # Presence of a criterion is not its precedence or applicability.
            for key,pattern in (("mentionsGoalDifference",r"\bgoal difference\b"),("mentionsHeadToHead",r"\bhead[- ]to[- ]head\b")):
                if re.search(pattern,body,re.I):found[key]=True
    return [{"parameter":key,"value":value,"scope":"ARTICLE_STATEMENT_EDITION_UNVERIFIED"} for key,value in sorted(found.items())]

def section_leads(content,missing):
    result=[];ancestors=[]
    sections=list(re.finditer(r"(?m)^(={2,6})\s*([^=\n]+?)\s*\1\s*$",content))
    for i,m in enumerate(sections):
        depth,title=m.groups();name=" ".join(title.split()).casefold()
        while ancestors and ancestors[-1][0]>=len(depth):ancestors.pop()
        blocked=any(BAD.search(x) or re.search(r"\b(women|youth|reserve|results|standings|statistics|honours)\b",x) for _,x in ancestors)
        ancestors.append((len(depth),name))
        if blocked or BAD.search(name):continue
        body=content[m.end():sections[i+1].start() if i+1<len(sections) else len(content)]
        for field,labels in RULE_SECTIONS.items():
            if field in missing and name in labels:
                lead={"field":field,"heading":title.strip(),"scope":"SECTION_LEAD_EDITION_UNVERIFIED"}
                facts=rule_facts(body,field)
                if facts:lead["facts"]=facts
                result.append(lead)
    return result


class Catalogue:
    def __init__(self,cache,national,refresh=False,wikipedia=True,rsssf=True):
        self.cache=Path(cache);self.national=national;self.refresh=refresh;self.wikipedia=wikipedia;self.rsssf=rsssf
        self.directory=None;self.directory_error=None;self.revision=None
    def _download(self,url,validate,ttl=7*86400):
        self.cache.mkdir(parents=True,exist_ok=True)
        p=self.cache/(hashlib.sha256(url.encode()).hexdigest()+".bin")
        if p.exists() and not self.refresh and time.time()-p.stat().st_mtime<ttl:
            try:return validate(p.read_bytes())
            except (ValueError,KeyError,TypeError,RuntimeError):pass
        raw=fetch(urllib.request.Request(url,headers={"User-Agent":UA,"Accept":"application/json,text/html"}),timeout=15,attempts=2,total=35,limit=16*1024*1024)
        try:value=validate(raw)
        except Exception as exc:
            route=urllib.parse.urlsplit(url)
            print(f"DIAG source rejected | host={route.netloc} path={route.path} bytes={len(raw)} reason={exc}",flush=True)
            raise
        tmp=p.with_suffix(".part");tmp.write_bytes(raw);tmp.replace(p);return value
    def _api(self,url,params):
        route=url+"?"+urllib.parse.urlencode(dict(params,format="json",formatversion=2,maxlag=5))
        def validate(raw):
            data=json.loads(raw)
            if "error" in data:raise RuntimeError("MediaWiki: "+str(data["error"].get("code")))
            if not isinstance(data,dict):raise ValueError("MediaWiki response schema")
            return data
        return self._download(route,validate)
    def _directory(self):
        if self.directory is not None:return self.directory
        if self.directory_error is not None:raise RuntimeError(self.directory_error)
        try:
            report("Wikipedia competition directory","SEARCHING","one catalogue for all countries")
            p=self._api(WP,{"action":"parse","page":DIRECTORY,"prop":"text|revid"})["parse"]
            if p.get("title")!=DIRECTORY or not p.get("revid"):raise ValueError("Wikipedia catalogue identity mismatch")
            html=p["text"]
            if isinstance(html,dict):html=html["*"]
            self.directory=directory_rows(html,self.national);self.revision=p["revid"]
            if not self.directory:raise ValueError("Wikipedia catalogue markup unsupported")
            report("Wikipedia competition directory","FOUND",f"{len(self.directory)} scoped countries; identity only")
            return self.directory
        except Exception as e:self.directory=None;self.directory_error=str(e);raise
    def _entities(self,rows):
        titles=sorted({r["title"] for r in rows}|{r["countryTitle"] for r in rows});out={}
        for i in range(0,len(titles),20):
            group=titles[i:i+20]
            data=self._api(WD,{"action":"wbgetentities","sites":"enwiki","titles":"|".join(group),"props":"claims|sitelinks","sitefilter":"enwiki"})
            for e in data.get("entities",{}).values():
                title=e.get("sitelinks",{}).get("enwiki",{}).get("title")
                if title in group:
                    if title in out and out[title].get("id")!=e.get("id"):raise ValueError("Ambiguous exact Wikipedia entity")
                    out[title]=e
        return out
    def article_leads(self,packet,missing):
        if not packet.get("wikidataId") or not missing:return []
        data=self._api(WP,{"action":"query","prop":"pageprops|revisions","titles":packet["sourceId"],
                          "rvprop":"ids|timestamp|content","rvslots":"main"})
        pages=data.get("query",{}).get("pages",[])
        if len(pages)!=1 or pages[0].get("pageprops",{}).get("wikibase_item")!=packet["wikidataId"]:return []
        revisions=pages[0].get("revisions",[])
        if len(revisions)!=1:return []
        rev=revisions[0];content=rev.get("slots",{}).get("main",{}).get("content")
        if not rev.get("revid") or not isinstance(content,str):return []
        url="https://en.wikipedia.org/w/index.php?oldid="+str(rev["revid"])
        return [dict(x,sourceUrl=url+"#"+urllib.parse.quote(x["heading"].replace(" ","_")),revision=rev["revid"]) for x in section_leads(content,missing)]
    def country(self,country,year,rule_fields=None):
        packets=[];seeds=[];errors=[]
        url=bet_route(country,year)
        if self.rsssf and url:
            try:
                report("BetExplorer "+country,"SEARCHING","scoped men's league entrant labels only")
                with time_budget(15):
                    p=self._download(url,lambda raw:bet_packet(raw,country,year),ttl=86400)
                packets.append(p)
                report("BetExplorer "+country,"FOUND",f"{len(p['participants'])} source club labels; not July 1")
            except Exception as e:
                errors.append(["BetExplorer",str(e)]);report("BetExplorer "+country,"ERROR",str(e))
        if self.rsssf and (country,year) in RSSSF:
            try:
                report("RSSSF domestic "+country,"SEARCHING","exact edition/section; entrant names only")
                def parse_archive(raw):
                    failures=[]
                    found=rsssf_packets(raw,country,year,failures)
                    if not found:raise ValueError("; ".join(str(x) for x in failures))
                    return found,failures
                pp,failures=self._download(RSSSF[country,year][0],parse_archive,ttl=86400)
                packets.extend(pp);errors.extend(failures)
                report("RSSSF domestic "+country,"FOUND",f"{len(pp)} competition observations; {sum(len(p['participants']) for p in pp)} entrant labels")
            except Exception as e:errors.append(["RSSSF",str(e)]);report("RSSSF domestic "+country,"ERROR",str(e))
        if self.rsssf:
            for route in sports_routes(country,year):
                try:
                    with time_budget(12):
                        item=self._download(route[0],lambda raw:sports_packet(raw,country,year,route),ttl=86400)
                    packets.append(item)
                    report("GlobalSportsArchive "+country,"FOUND",f"{item['name']}; {len(item.get('participants',[]))} scoped names")
                except Exception as e:
                    errors.append(["GlobalSportsArchive",str(e)]);report("GlobalSportsArchive "+country,"ERROR",str(e))
        if self.wikipedia:
            try:
                rows=self._directory().get(country,[])
                entities={}
                if rows:
                    try:entities=self._entities(rows)
                    except Exception as e:errors.append(["Wikidata entity API",str(e)]);report("Wikidata entity API "+country,"ERROR",str(e))
                for row in rows:
                    packet,seed=directory_evidence(row,entities,self.revision,year)
                    entity=entities.get(row["title"],{})
                    if seed:
                        seeds.append(seed)
                        # Exact entity already validated for country/competition.
                        # Official sites are leads, never proof of rules or dates.
                        sites=[]
                        for claim in entity.get("claims",{}).get("P856",[]):
                            if claim.get("rank")=="deprecated":continue
                            url=claim.get("mainsnak",{}).get("datavalue",{}).get("value")
                            if isinstance(url,str):
                                parts=urllib.parse.urlsplit(url)
                                if parts.scheme in ("http","https") and parts.hostname and not parts.username and not parts.password and url not in sites:sites.append(url)
                        if sites:packet["officialSourceLeads"]=sorted(sites)
                        if rule_fields:
                            try:
                                leads=self.article_leads(packet,rule_fields(packet))
                                if leads:packet["researchSectionLeads"]=leads
                            except Exception as e:
                                errors.append(["Wikipedia competition article",str(e)])
                                report("Wikipedia competition article "+country,"ERROR",str(e))
                    packets.append(packet)
                report("Wikipedia domestic "+country,"FOUND" if rows else "NO DATA",f"{len(rows)} identity leads; {len(seeds)} exact linked Wikidata entities")
            except Exception as e:errors.append(["WikipediaDirectory",str(e)]);report("Wikipedia domestic "+country,"ERROR",str(e))
        return packets,seeds,errors

def selftest():
    bet_selftest()
    sports_selftest()
    facts=section_leads("==Format==\nThe league consists of 12 teams. Each team plays every other team twice.\n==Regulations==\nThree points for a win and one point for a draw.\n==Women==\n===Format===\nThe league consists of 8 teams.",{"format","regulation"})
    assert len(facts)==2 and any(x["parameter"]=="teams" and x["value"]==12 for x in facts[0]["facts"])
    assert any(x["parameter"]=="pointsWin" and x["value"]==3 for x in facts[1]["facts"])
    assert not rule_facts("Three points for a win. Two points for a win.","regulation")
    sections=section_leads("==Format==\n===Rules===\n==Women==\n===Format===\n==Results==\n===Schedule===\n==History==",{"format","regulation","roundDates"})
    assert [(x["field"],x["heading"]) for x in sections]==[("format","Format"),("regulation","Rules")]
    national=[{"fifa":"BHU","source":{"name":"BHUTAN"}},{"fifa":"AIA","source":{"name":"ANGUILLA"}}]
    html='<h3>Domestic</h3><h4>Bhutan</h4><table><tr><td>[Sen.Men] [L]</td><td><a href="/wiki/Bhutan_Premier_League">Bhutan Premier League</a></td><td>1st-tier</td></tr><tr><td>[Sen.Women]</td><td><a href="/wiki/Women">Women League</a></td><td>1st-tier</td></tr><tr><td>[Sen.Men]</td><td><a href="/wiki/Gold_Cup">Gold Cup</a></td><td>National cup</td></tr><tr><td>[Sen.Men]</td><td><a href="/wiki/Old">Old League</a></td><td>1st-tier, defunct</td></tr></table><h4>Anguilla</h4><h5>Leagues</h5><ul><li><a href="/wiki/AFA_Senior_Male_League">AFA Senior Male League</a>: 1st-tier</li><li><a href="/wiki/Second">Second League</a>: 2nd-tier</li></ul><h5>Cups</h5><ul><li><a href="/wiki/Shield">Shield</a>: Super cup</li></ul><h3>International</h3><h4>Bhutan</h4><table><tr><td>[Sen.Men]</td><td><a href="/wiki/Wrong">Wrong</a></td><td>1st-tier</td></tr></table>'
    rows=directory_rows(html,national)
    assert len(rows["BHU"])==2 and len(rows["AIA"])==1 and rows["AIA"][0]["seniorMale"]
    def claim(q):return {"mainsnak":{"datavalue":{"value":{"id":q}}}}
    entities={"Bhutan":{"id":"Q1"},"Bhutan Premier League":{"id":"Q2","claims":{"P17":[claim("Q1")]}}}
    packet,seed=directory_evidence(rows["BHU"][0],entities,1,2026)
    assert packet["identityEvidence"] and eligible_league(seed) and "class" not in seed
    entities["Bhutan Premier League"]["claims"]["P2094"]=[claim("Q606060")]
    packet,seed=directory_evidence(rows["BHU"][0],entities,1,2026)
    assert not packet["identityEvidence"] and seed is None
    table="\n".join(f"{i}.Club {i}  18  6  6  6  10-10  24" for i in range(1,11))
    raw=("<h2>Afghanistan 2026</h2><h4>Afghanistan Champions League 2026</h4><pre>NB: sixth edition\nFinal Table:\n"+table+"\n</pre><h4>Other Cup</h4><pre>Wrong</pre>").encode()
    p=rsssf_packets(raw,"AFG",2026)[0]
    assert len(p["participants"])==10 and set(p["participants"][0])=={"name"} and p["season"]=="2026"
    assert observation_key(p)==observation_key(dict(p,sourceUrl="other"))
    # A broken cup must not erase a valid league. Exact alternative table heading.
    aia=("<h2>Anguilla 2026</h2><h4>AFA Football League 2026</h4><pre>Table:\n"+table+
         "\n</pre><h4>President's Cup 2026</h4><pre>Group 1\ninvalid</pre>").encode()
    errors=[];partial=rsssf_packets(aia,"AIA",2026,errors)
    assert len(partial)==1 and partial[0]["kind"]=="LEAGUE_DIVISION" and errors
    assert rsssf_packets(raw.replace(b"Final Table:",b"Halfway Table:"),"AFG",2026)
    bh=("<h2>Bhutan 2026</h2><h4>Premier League 2026</h4><pre>Table:\n"+table.replace("Club 1  ","BFF Academy FC  ",1)+"\n</pre>").encode()
    assert len(rsssf_packets(bh,"BHU",2026)[0]["participants"])==10
    assert not rank_table(["Table:","1.A  1  1  0  0  2-0  3"],0,1,1)==[]
    for bad in (raw.replace(b"Afghanistan 2026",b"Afghanistan 2025"),raw.replace(b"Champions League 2026",b"Second League 2026"),raw.replace(b"Club 10  ",b"Women Club 10  ")):
        try:rsssf_packets(bad,"AFG",2026)
        except ValueError:pass
        else:raise AssertionError("Wrong archive scope accepted")
