"""Additional scoped sources and gap routing. No sporting-state writer."""
import collections,csv,datetime as dt,hashlib,io,json,re,time,urllib.request,unicodedata
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import urljoin,urlsplit,quote
from http_retry import fetch as request_bytes
FD="https://www.football-data.co.uk/mmz4281/"
RSSSF="https://www.rsssf.org/"
# Fixed country/tier bindings from Football-Data's documented division codes.
# Scotland split-season formats are not assumed double RR.
FD_CODES={"ENG":("E0","E1","E2","E3"),"ITA":("I1","I2"),"ESP":("SP1","SP2"),"GER":("D1","D2"),"FRA":("F1","F2"),"NED":("N1",),"POR":("P1",),"BEL":("B1",),"TUR":("T1",),"GRE":("G1",),"SCO":("SC0","SC1","SC2","SC3")}
BAD=re.compile(r"\b(women|woman|female|ladies|feminine|femenin[oa]|femminile|frauen|damen|feminin|u[- ]?(?:17|18|19|20|21|23)|youth|reserve[s]?)\b",re.I)
def norm(x):return re.sub(r"[^a-z0-9]","",unicodedata.normalize("NFKD",str(x)).encode("ascii","ignore").decode().casefold())
def states(row):
    out=dict(row.get("fields",{}));out.update(row.get("operationalFields",{}));return out
def wanted(row,fields):
    s=states(row)
    return [k for k in fields if s.get(k,"?") in ("I","P","?") and (k!="july1" or s.get(k) in ("I","P"))]
def fd_route(row):
    i=row.get("technicalIdentity",{})
    if i.get("category")!="LEAGUE_DIVISION" or row.get("world") not in ("9697","2627"):return None
    m=re.fullmatch(r"div([1-9][0-9]*)",i.get("detail",""))
    codes=FD_CODES.get(i.get("country"),())
    if not m or int(m[1])>len(codes):return None
    code=codes[int(m[1])-1]
    # Publisher historical index: 1996/97 Italy A only; Scotland Premier/Div1 only.
    if row["world"]=="9697" and code in ("I2","SC2","SC3"):return None
    return FD+row["world"]+"/"+code+".csv",code
def fd_packet(raw,row,code):
    try:text=raw.decode("utf-8-sig")
    except UnicodeDecodeError:text=raw.decode("cp1252")
    reader=csv.DictReader(io.StringIO(text))
    if not {"Div","Date","HomeTeam","AwayTeam"}<=set(reader.fieldnames or []):raise ValueError("Unexpected CSV schema")
    year=1996 if row["world"]=="9697" else 2026
    start=dt.date(year,7,1);end=dt.date(year+1,7,1)
    pairs=set();days=set();clubs=set();dates=[];pre=[]
    for g in reader:
        if not any(str(g.get(k) or "").strip() for k in ("Div","Date","HomeTeam","AwayTeam")):continue
        if g.get("Div")!=code:raise ValueError("Wrong division")
        a,b=(str(g.get(k) or "").strip() for k in ("HomeTeam","AwayTeam"))
        day=None
        for fmt in ("%d/%m/%Y","%d/%m/%y"):
            try:day=dt.datetime.strptime(g["Date"],fmt).date();break
            except ValueError:pass
        if day is None or not start<=day<end or not a or not b or a==b or BAD.search(a) or BAD.search(b) or (day,a) in days or (day,b) in days:raise ValueError("Wrong edition/team/date collision")
        days.update(((day,a),(day,b)));pairs.add((a,b));clubs.update((a,b));dates.append(day.isoformat())
    if len(clubs)<2 or not dates:raise ValueError("Empty league edition")
    # Partial schedules/results are observations, NOT full entrant lists.
    n=len(clubs);complete=len(dates)==n*(n-1) and pairs=={(a,b) for a in clubs for b in clubs if a!=b}
    packet={"participants":{"observed":sorted(clubs),"fullDoubleRoundRobin":complete},
            "calendar":{"observedDates":sorted(set(dates)),"matches":len(dates),"scope":"OBSERVED_DATES_NOT_ROUND_TEMPLATE"},
            "competitionDays":{"observedWeekdays":dict(collections.Counter(dt.date.fromisoformat(d).strftime("%a") for d in dates)),"scope":"OBSERVED_NOT_TEMPLATE_POSTPONEMENTS_NOT_RESOLVED"}}
    if complete:packet["format"]={"observedTeams":n,"observedCycles":2,"scope":"OBSERVED_DOUBLE_RR_NOT_OFFICIAL_RULE"}
    return packet

class Links(HTMLParser):
    def __init__(self,raw):
        super().__init__(convert_charrefs=True);self.items=[];self.current=None
        self.feed(raw.decode("utf-8",errors="replace"));self.close()
    def handle_starttag(self,tag,attrs):
        if tag=="a":self.current=[dict(attrs).get("href",""),[]]
    def handle_data(self,data):
        if self.current is not None:self.current[1].append(data)
    def handle_endtag(self,tag):
        if tag=="a" and self.current is not None:
            href,parts=self.current;self.items.append((href," ".join(" ".join(parts).split())));self.current=None
def safe_rsssf(base,href):
    u=urljoin(base,href);p=urlsplit(u)
    if p.scheme in ("http","https") and p.netloc in ("rsssf.org","www.rsssf.org") and p.path.endswith((".html",".htm")):
        return "https://www.rsssf.org"+p.path
    return None
class Cached:
    def __init__(self,path,refresh=False):self.path=path;self.refresh=refresh;self.memory={}
    def get(self,url):
        if url in self.memory:return self.memory[url]
        self.path.mkdir(parents=True,exist_ok=True)
        p=self.path/(hashlib.sha256(url.encode()).hexdigest()+".bin")
        if p.exists() and not self.refresh and time.time()-p.stat().st_mtime<86400:
            raw=p.read_bytes()
        else:
            req=urllib.request.Request(url,headers={"User-Agent":"SWOS-Expanded/1.0","Accept":"text/html,text/csv,application/json"})
            raw=request_bytes(req,timeout=35,attempts=2,interval=2,limit=8*1024*1024)
            tmp=p.with_suffix(".part");tmp.write_bytes(raw);tmp.replace(p)
        self.memory[url]=raw;return raw

class Discovery:
    """Country/season archive references and exact-title search leads only.
    Never copy unscoped RSSSF text, results or rules into canonical fields."""
    def __init__(self,cache,countries):
        self.cache=cache;self.countries=countries;self.indexes={};self.errors={}
    def index(self,path):
        if path not in self.indexes:
            url=RSSSF+path
            try:self.indexes[path]=[(safe_rsssf(url,h),t) for h,t in Links(self.cache.get(url)).items]
            except Exception as e:self.indexes[path]=[];self.errors[path]=str(e)
        return self.indexes[path]
    def country_leads(self,row):
        c=row.get("technicalIdentity",{}).get("country") or row.get("country")
        if not c:return []
        names={norm(k) for k,v in self.countries.items() if v==c}
        extra={"BIH":["Bosnia-Hercegovina"],"MKD":["Macedonia"],"USA":["United States of America"],"SUR":["Surinam"],"SRB":["Yugoslavia"],"CIV":["Ivory Coast"]}
        names.update(norm(x) for x in extra.get(c,[]))
        year="1996" if row["world"]=="9697" else "2026"
        short="97" if year=="1996" else "27"
        links=self.index("resultsp99.html" if year=="1996" else "curdom.html")
        found=[]
        for url,title in links:
            if not url or BAD.search(title):continue
            m=re.fullmatch(r"(.+?)\s+("+year+r"(?:[/–-](?:"+short+r"|"+str(int(year)+1)+r"))?)(?:\s+\(incomplete\))?",title,re.I)
            if m and norm(m[1]) in names:found.append({"url":url,"label":title,"scope":"COUNTRY_EDITION_LEAD_SECTION_NOT_VERIFIED"})
            elif year=="2026" and norm(title) in names:found.append({"url":url,"label":title,"scope":"COUNTRY_ARCHIVE_LEAD_EDITION_AND_SECTION_NOT_VERIFIED"})
        return found
    def international_leads(self,row):
        name=row.get("name","")
        if not name or BAD.search(name):return []
        found=[]
        for p in ("intclub.html","intland.html"):
            for url,title in self.index(p):
                if url and not BAD.search(title) and norm(name)==norm(title):
                    found.append({"url":url,"label":title,"scope":"COMPETITION_ARCHIVE_LEAD_EDITION_NOT_VERIFIED"})
        return found
    def leads(self,row):
        return self.country_leads(row) if (row.get("technicalIdentity",{}).get("country") or row.get("country")) else self.international_leads(row)

def selftest():
    row={"world":"9697","technicalIdentity":{"country":"ENG","category":"LEAGUE_DIVISION","detail":"div1"},"fields":{"participants":"P","format":"I","july1":"?"}}
    assert fd_route(row)==(FD+"9697/E0.csv","E0")
    for country,tier in (("ITA","div2"),("SCO","div3"),("SCO","div4")):
        assert fd_route(dict(row,technicalIdentity=dict(row["technicalIdentity"],country=country,detail=tier))) is None
    assert wanted(row,("participants","format","july1"))==["participants","format"]
    b=b"Div,Date,HomeTeam,AwayTeam,FTHG,FTAG\nE0,10/08/96,A,B,9,0\nE0,10/09/96,B,A,0,1\n"
    p=fd_packet(b,row,"E0")
    assert p["format"]["observedCycles"]==2 and "FTHG" not in json.dumps(p) and "july1" not in p
    try:fd_packet(b.replace(b"E0,",b"E1,"),row,"E0")
    except ValueError:pass
    else:raise AssertionError("Wrong tier")
    assert safe_rsssf(RSSSF,"https://example.org/x.html") is None
    assert Links(b'<a href="x.html">Italy 1996/97</a>').items==[("x.html","Italy 1996/97")]
    class Fake:
        def get(self,url):return b'<a href="tablesi/ital2026.html">Italy</a><a href="women.html">Italy (Women)</a>'
    d=Discovery(Fake(),{"ITALY":"ITA"})
    assert len(d.leads({"world":"2627","country":"ITA"}))==1
