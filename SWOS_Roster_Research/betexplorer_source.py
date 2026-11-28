"""Scoped public BetExplorer standings, football research only; no odds or results."""
import re
from urllib.parse import urlsplit
from alternative_sources import BAD,norm
from transfermarkt_source import Page
from season_config import edition_label

BASE="https://www.betexplorer.com"
# Only directly identified men's senior first-tier routes, observed 27/11/2026.
# Explicit season and country must still be present in each fetched page.
ROUTES={
    "ANG":("angola","girabola","Girabola","Angola"),
    "BHR":("bahrain","premier-league","Premier League","Bahrain"),
    "BHU":("bhutan","premier-league","Premier League","Bhutan"),
    "CAM":("cambodia","cpl","CPL","Cambodia"),
    "COD":("dr-congo","ligue-1","Ligue 1","DR Congo"),
    "ETH":("ethiopia","premier-league","Premier League","Ethiopia"),
    "GUI":("guinea","ligue-1-2026-2027","Ligue 1","Guinea"),
    "IRQ":("iraq","stars-league","Stars League","Iraq"),
    "JOR":("jordan","premier-league","Premier League","Jordan"),
    "KUW":("kuwait","premier-league","Premier League","Kuwait"),
    "MLI":("mali","premiere-division","Premiere Division","Mali"),
    "NGA":("nigeria","npfl","NPFL","Nigeria"),
    "NIG":("niger","super-ligue","Super Ligue","Niger"),
    "TAN":("tanzania","ligi-kuu-bara","Ligi Kuu Bara","Tanzania"),
    "TJK":("tajikistan","vysshaya-liga","Vysshaya Liga","Tajikistan"),
    "UZB":("uzbekistan","super-league","Super League","Uzbekistan"),
}

def route(country,year):
    if year!=2026 or country not in ROUTES:return None
    slug,league,title,label=ROUTES[country]
    return BASE+"/football/"+slug+"/"+league+"/"

def own(node,table):
    p=node.parent
    while p is not None and p.tag!="table":p=p.parent
    return p is table

def cells(row):
    return [n for n in row.children if n.tag in ("td","th")]

def packet(raw,country,year):
    url=route(country,year)
    if url is None:raise ValueError("No inspected BetExplorer route")
    slug,league,title,label=ROUTES[country]
    page=Page(raw.decode("utf-8")).root
    titles=[n.text() for n in page.all("title")]
    country_path="/football/"+slug+"/"
    def same_host(href):
        u=urlsplit(href)
        return u.netloc in ("","www.betexplorer.com","betexplorer.com") and u.scheme in ("","http","https")
    breadcrumb=any(same_host(a.attrs.get("href","")) and urlsplit(a.attrs.get("href","")).path.rstrip("/")==country_path.rstrip("/") and norm(a.text())==norm(label) for a in page.all("a"))
    if not breadcrumb and not any(re.search(r"\bFootball\s+"+re.escape(label)+r"\s*(?:\||$)",t,re.I) for t in titles):
        raise ValueError("BetExplorer country not evidenced")
    headings=[]
    for h in page.all("h1"):
        m=re.fullmatch(re.escape(title)+r"\s+(\d{4}(?:[/–-]\d{2,4})?)\s+(?:Table\s*&\s*Stats|Table and Stats)",h.text(),re.I)
        if m and not BAD.search(h.text()):headings.append(edition_label(m[1],year))
    if len(headings)!=1 or not headings[0]:raise ValueError("BetExplorer exact competition/edition heading missing")
    season=headings[0];matches=[]
    for table in page.all("table"):
        rows=[r for r in table.all("tr") if own(r,table)]
        for idx,row in enumerate(rows):
            aliases={"played":"mp","matchesplayed":"mp","p":"mp","won":"w","drawn":"d","lost":"l","points":"pts"}
            cols=[aliases.get(norm(c.text()),norm(c.text())) for c in cells(row)]
            required={"team","mp","w","d","l","pts"}
            if not required<=set(cols) or any(cols.count(k)!=1 for k in required):continue
            ci={k:cols.index(k) for k in required}
            if ci["team"]<1:continue
            found={};bad=False
            for tr in rows[idx+1:]:
                cs=cells(tr)
                if not cs:continue
                if any(c.tag=="th" for c in cs):break
                if len(cs)<=max(ci.values()):bad=True;break
                rank=cs[ci["team"]-1].text().strip()
                team=cs[ci["team"]].text().strip()
                if not re.fullmatch(r"\d+\.?",rank) or int(rank.rstrip("."))!=len(found)+1 or not team or len(team)>120 or "\ufffd" in team or (BAD.search(team) and not (country=="BHU" and team=="BFF Academy FC")) or norm(team) in found:
                    bad=True;break
                nums=[cs[ci[k]].text() for k in ("mp","w","d","l")]
                if not all(re.fullmatch(r"\d+",v) for v in nums) or int(nums[0])!=sum(map(int,nums[1:])):bad=True;break
                ids=set()
                for a in cs[ci["team"]].all("a"):
                    href=a.attrs.get("href","")
                    if not same_host(href):continue
                    m=re.fullmatch(r"/football/team/[^/]+/([A-Za-z0-9]{6,16})/?",urlsplit(href).path)
                    if m:ids.add(m[1])
                if len(ids)>1:bad=True;break
                found[norm(team)]={"name":team,**({"sourceId":next(iter(ids))} if ids else {})}
            if not bad and len(found)>=2:matches.append(found)
    if not matches:
        shapes=[sorted({norm(n.text())[:80] for n in t.all("th")})[:16] for t in page.all("table")[:8]]
        print(f"DIAG BE schema | country={country} season={season} bytes={len(raw)} tables={len(page.all('table'))} headers={shapes}",flush=True)
        raise ValueError("BetExplorer verified edition; standings absent from delivered HTML or unsupported schema (not NO DATA)")
    # Home/away/overall copies may have the same entrants; never union distinct groups.
    if any(set(v)!=set(matches[0]) for v in matches):raise ValueError("BetExplorer conflicting standing groups")
    participants=[]
    for k in sorted(matches[0]):
        labels={v[k]["name"] for v in matches};ids={v[k]["sourceId"] for v in matches if v[k].get("sourceId")}
        if len(labels)!=1 or len(ids)>1:raise ValueError("BetExplorer conflicting club identity")
        participants.append({"name":next(iter(labels)),**({"sourceId":next(iter(ids))} if ids else {})})
    return {"source":"BetExplorer","sourceId":slug+"/"+league,"country":country,"kind":"LEAGUE_DIVISION","name":title,"season":season,"sourceUrl":url,
            "identityEvidence":True,"scope":"SEASON_PARTICIPANT_LABELS_NOT_JULY1","participants":participants}

def selftest():
    h='<title>CPL table, stats &amp; results, Football Cambodia | BetExplorer</title><h1>CPL 2026/2027 Table &amp; Stats</h1>'
    table='<table><tr><th>#</th><th>Team</th><th>MP</th><th>W</th><th>D</th><th>L</th><th>Pts</th></tr><tr><td>1.</td><td><a href="/football/team/one/abcd1234/">Club One</a></td><td>1</td><td>1</td><td>0</td><td>0</td><td>3</td></tr><tr><td>2.</td><td>Club Two</td><td>1</td><td>0</td><td>0</td><td>1</td><td>0</td></tr></table>'
    assert packet((h+table+table).encode(),"CAM",2026)["participants"]==[{"name":"Club One","sourceId":"abcd1234"},{"name":"Club Two"}]
    assert packet((h+table.replace(">MP<",">Played<").replace(">Pts<",">Points<")).encode(),"CAM",2026)["participants"]==packet((h+table).encode(),"CAM",2026)["participants"]
    for bad in (h.replace("Cambodia","Thailand")+table,h.replace("2026/2027","2025/2026")+table,h+table+table.replace("Club Two","Other"),h+table.replace("Club Two","Women Club")):
        try:packet(bad.encode(),"CAM",2026)
        except ValueError:pass
        else:raise AssertionError("Wrong BetExplorer scope accepted")
