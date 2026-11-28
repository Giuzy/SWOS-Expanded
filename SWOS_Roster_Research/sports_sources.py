"""Additional public sports sources: strict edition routes and non-sporting leads."""
import re
from urllib.parse import urlsplit
from alternative_sources import BAD,norm
from transfermarkt_source import Page
from betexplorer_source import own,cells

GSA="https://globalsportsarchive.com/en/soccer/competition/"
# Route URL, country label, exact title, explicit season, kind, independently
# known entrant count (None = identity only). Never infer counts from a table.
ROUTES={
 "AFG":((GSA+"afghanistan-champions-league-2026/81615","Afghanistan","Afghanistan Champions League","2026","LEAGUE_DIVISION",10),),
 "BHU":((GSA+"bank-of-bhutan-premier-league-2026/79964","Bhutan","Bank of Bhutan Premier League","2026","LEAGUE_DIVISION",10),),
 "BLZ":((GSA+"premier-league-of-belize-2026-2027/81131","Belize","Premier League of Belize","2026/27","LEAGUE_DIVISION",None),),
 "CAM":((GSA+"cambodian-premier-league-2026-2027/81614","Cambodia","Cambodian Premier League","2026/27","LEAGUE_DIVISION",11),
        (GSA+"hun-sen-cup-2026-2027/81050","Cambodia","Hun Sen Cup","2026/27","NATIONAL_CUP",None)),
 "NGA":((GSA+"nigeria-premier-football-league-2026-2027/81511","Nigeria","Nigeria Premier Football League","2026/27","LEAGUE_DIVISION",None),)
}

def routes(country,year):return ROUTES.get(country,()) if year==2026 else ()

def source_leads(country,year):
    leads=[{"source":"GlobalSportsArchive","url":r[0],"scope":"INSPECTED_EDITION_ROUTE_NOT_FIELD_EVIDENCE"} for r in routes(country,year)]
    if country=="CAM":
        leads.extend([
          {"source":"League321","url":"https://www.league321.com/cambodia-football.html","scope":"COUNTRY_DIRECTORY_NOT_FIELD_EVIDENCE"},
          {"source":"Sofascore","url":"https://www.sofascore.com/football/tournament/cambodia/hun-sen-cup/18539","scope":"COMPETITION_ROUTE_NOT_EDITION_EVIDENCE"}
        ])
    return leads

def packet(raw,country,year,route):
    if route not in routes(country,year):raise ValueError("Unscoped sports route")
    url,label,name,season,kind,count=route
    page=Page(raw.decode("utf-8")).root
    expected=label+" - "+name+" "+(season if "/" not in season else season[:5]+str(int(season[:4])+1))
    titles=[n.text() for n in page.all("title")]
    if not any(t.startswith(expected+" - ") for t in titles):
        raise ValueError("GlobalSportsArchive exact country/title/edition missing")
    result={"source":"GlobalSportsArchive","sourceId":url.rsplit("/",1)[-1],"country":country,
            "kind":kind,"name":name,"season":season,"sourceUrl":url,"identityEvidence":True,
            "scope":"COMPETITION_IDENTITY_NOT_PARTICIPANTS"}
    if not count:return result
    tables=[]
    for t in page.all("table"):
        rows=[r for r in t.all("tr") if own(r,t)]
        for i,tr in enumerate(rows):
            h=[norm(c.text()) for c in cells(tr)]
            req={"team","g","w","d","l","pts"}
            if not req<=set(h) or any(h.count(k)!=1 for k in req):continue
            ci={k:h.index(k) for k in req}
            if ci["team"]==0:continue
            members={};invalid=False
            for row in rows[i+1:]:
                cs=cells(row)
                if not cs:continue
                if len(cs)<=max(ci.values()):invalid=True;break
                rank=cs[ci["team"]-1].text()
                if not rank.isdigit() or int(rank)!=len(members)+1:invalid=True;break
                nums=[cs[ci[k]].text() for k in ("g","w","d","l")]
                if not all(v.isdigit() for v in nums) or int(nums[0])!=sum(map(int,nums[1:])):invalid=True;break
                links={}
                for a in cs[ci["team"]].all("a"):
                    u=urlsplit(a.attrs.get("href",""))
                    m=re.fullmatch(r"/en/soccer/team/[^/]+/([1-9][0-9]*)(?:/(?:overview|squad))?/?",u.path)
                    n=a.text().strip()
                    if u.netloc in ("","globalsportsarchive.com","www.globalsportsarchive.com") and m and n and len(n)<=120 and not BAD.search(n):
                        links[m[1]]=n
                if len(links)!=1:invalid=True;break
                sid,club=next(iter(links.items()))
                if sid in members or norm(club) in {norm(v) for v in members.values()}:invalid=True;break
                members[sid]=club
            if not invalid and len(members)==count:tables.append(members)
    if tables and all(t==tables[0] for t in tables):
        result["participants"]=[{"sourceId":k,"name":v} for k,v in sorted(tables[0].items())]
        result["scope"]="SEASON_PARTICIPANT_LABELS_NOT_JULY1"
    return result

def selftest():
    route=routes("BHU",2026)[0]
    title="<title>Bhutan - Bank of Bhutan Premier League 2026 - Standings, Fixtures &amp; Stats - Soccer | Global Sports Archive</title>"
    table="<table><tr><th>#</th><th>Team</th><th>G</th><th>W</th><th>D</th><th>L</th><th>PTS</th></tr>"
    table+="".join(f'<tr><td>{i}</td><td><a href="/en/soccer/team/team-{i}/{i}">Club {i}</a></td><td>1</td><td>1</td><td>0</td><td>0</td><td>3</td></tr>' for i in range(1,11))+"</table>"
    assert len(packet((title+table).encode(),"BHU",2026,route)["participants"])==10
    assert "participants" not in packet(title.encode(),"BHU",2026,route)
    try:packet(title.replace("2026","2027").encode(),"BHU",2026,route)
    except ValueError:pass
    else:raise AssertionError("wrong GSA edition accepted")
