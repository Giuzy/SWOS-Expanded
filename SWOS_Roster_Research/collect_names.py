#!/usr/bin/env python3
"""Read-only Expanded catalogue survey; Wikidata candidates, not runtime data."""
import argparse, csv, hashlib, json, re, sys, time, unicodedata, urllib.parse, urllib.request, urllib.error
from pathlib import Path
from roster_storage import Store, selftest as storage_selftest
from http_retry import fetch

HERE = Path(__file__).resolve().parent
MAP = Path('docs/NATIONAL_ROSTER_SOURCE_MAP_2627.json')
OUT = HERE / 'results_2627'
CACHE = OUT / 'sources'
ENDPOINT = 'https://query.wikidata.org/sparql'
CUTOFF = '2026-07-01T00:00:00Z'
PREFIX = '''PREFIX wd: <http://www.wikidata.org/entity/>
PREFIX wdt: <http://www.wikidata.org/prop/direct/>
PREFIX p: <http://www.wikidata.org/prop/>
PREFIX ps: <http://www.wikidata.org/prop/statement/>
PREFIX pq: <http://www.wikidata.org/prop/qualifier/>
PREFIX wikibase: <http://wikiba.se/ontology#>
PREFIX bd: <http://www.bigdata.com/rdf#>
PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>
'''
LABEL = 'SERVICE wikibase:label { bd:serviceParam wikibase:language "en,it,mul". }'

def write_json(path, data):
    tmp = path.with_suffix(path.suffix + '.tmp')
    tmp.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding='utf-8')
    tmp.replace(path)

def update_changed(mapping, **fields):
    changed = False
    for key, value in fields.items():
        if key not in mapping or mapping[key] != value:
            mapping[key] = value
            changed = True
    return changed

class Checkpoint:
    """Avoid encoding, reading or replacing the full matrix on no-op steps."""
    def __init__(self, path, data, expected):
        self.path, self.data, self.expected = path, data, expected
        self.dirty = False

    def update(self, mapping, **fields):
        if update_changed(mapping, **fields):
            self.dirty = True

    def ensure(self, mapping, key, default):
        if key not in mapping:
            self.dirty = True
            mapping[key] = default
        return mapping[key]

    def append(self, seq, value):
        self.dirty = True
        seq.append(value)

    def save(self):
        if not self.dirty:
            return False
        if self.path.read_bytes() != self.expected:
            raise RuntimeError("Concurrent matrix change")
        write_json(self.path, self.data)
        self.expected = self.path.read_bytes()
        self.dirty = False
        return True

def chunks(items, size):
    for i in range(0, len(items), size):
        yield items[i:i+size]

def qid(url):
    item = url.rsplit('/', 1)[-1]
    if not re.fullmatch(r'Q[1-9][0-9]*', item):
        raise ValueError('Invalid Wikidata entity: ' + url)
    return item

def values(items):
    return ' '.join('wd:' + qid(x) for x in sorted(set(items)))

def norm(name):
    return ''.join(c for c in unicodedata.normalize('NFKD', name).casefold() if c.isalnum())

def placeholder(name):
    return not name or bool(re.match(r'^(TEST|RESERVE)(?:[ _-]*[0-9]+|$)',name,re.I))

def query(sparql):
    q = PREFIX + sparql
    key = hashlib.sha256(q.encode()).hexdigest()
    dest = CACHE / (key + '.json')
    data = None
    if dest.exists():
        cached = json.loads(dest.read_text(encoding='utf-8'))
        if cached.get('results',{}).get('bindings') or time.time()-dest.stat().st_mtime<86400:
            data=cached
    if data is None:
        req = urllib.request.Request(ENDPOINT, data=urllib.parse.urlencode({'query':q,'format':'json'}).encode(), headers={'User-Agent':'SWOS-Expanded-Roster-Research/1.1 (https://github.com/Giuzy/SWOS-Expanded/issues/2)','Accept':'application/sparql-results+json','Content-Type':'application/x-www-form-urlencoded'})
        data = json.loads(fetch(req, timeout=75, interval=3))
        if 'results' not in data or 'bindings' not in data['results']:
            raise ValueError('Unexpected query result')
        write_json(dest, data)
        dest.with_suffix('.rq').write_text(q, encoding='utf-8')
    return [{k:v['value'] for k,v in row.items()} for row in data['results']['bindings']]

def league_query(codes):
    quoted = ' '.join(json.dumps(c) for c in codes)
    return f'''SELECT DISTINCT ?fifa ?country ?countryLabel ?league ?leagueLabel ?class WHERE {{
      VALUES ?fifa {{ {quoted} }}
      {{ ?country wdt:P3441 ?fifa. }} UNION {{ ?entity wdt:P3441 ?fifa; wdt:P17 ?country. }}
      ?league wdt:P17 ?country; wdt:P3983 1; wdt:P2094 ?class.
      VALUES ?class {{ wd:Q31930761 wd:Q35554451 }}
      FILTER NOT EXISTS {{ ?league wdt:P2094 ?other. FILTER(?other NOT IN (wd:Q31930761,wd:Q35554451)) }}
      FILTER NOT EXISTS {{ ?league wdt:P576 ?end. FILTER(?end < "{CUTOFF}"^^xsd:dateTime) }}
      {LABEL}
    }}'''

def clubs_query(leagues):
    return f'''SELECT DISTINCT ?league ?season ?seasonLabel ?start ?end ?year ?expected ?club ?clubLabel WHERE {{
      VALUES ?league {{ {values(leagues)} }}
      ?season wdt:P3450 ?league.
      OPTIONAL {{ ?season wdt:P580 ?start. }} OPTIONAL {{ ?season wdt:P582 ?end. }}
      OPTIONAL {{ ?season wdt:P585 ?year. }} OPTIONAL {{ ?season wdt:P1132 ?expected. }}
      FILTER((BOUND(?start) && ?start >= "2025-07-01T00:00:00Z"^^xsd:dateTime && ?start < "2027-01-01T00:00:00Z"^^xsd:dateTime) || (BOUND(?year) && YEAR(?year)=2026))
      FILTER(!BOUND(?end) || ?end >= "{CUTOFF}"^^xsd:dateTime)
      FILTER NOT EXISTS {{ ?season wdt:P2094 ?other. FILTER(?other NOT IN (wd:Q31930761,wd:Q35554451)) }}
      OPTIONAL {{ ?season wdt:P1923 ?club. }}
      {LABEL}
    }}'''

def players_query(clubs):
    return f'''SELECT DISTINCT ?club ?player ?playerLabel ?membership ?start ?end WHERE {{
      VALUES ?club {{ {values(clubs)} }}
      ?player wdt:P106 wd:Q937857; wdt:P21 wd:Q6581097; p:P54 ?membership.
      ?membership ps:P54 ?club; pq:P580 ?start; wikibase:rank ?rank.
      FILTER(?rank != wikibase:DeprecatedRank)
      OPTIONAL {{ ?membership pq:P582 ?end. }}
      FILTER(?start <= "{CUTOFF}"^^xsd:dateTime)
      FILTER(!BOUND(?end) || ?end >= "{CUTOFF}"^^xsd:dateTime)
      FILTER NOT EXISTS {{ ?membership pq:P3831 ?role. FILTER(?role IN (wd:Q3246315,wd:Q18052877)) }}
      {LABEL}
    }}'''

def existing_index(root):
    index = {}
    for folder in ('teams','squad_extras'):
        for path in sorted((root/'data_json/worlds/2627'/folder).glob('*.json')):
            data = json.loads(path.read_text(encoding='utf-8-sig'))
            for t in data.get('teams', []):
                team = t.get('team', {})
                name = team.get('team_name')
                if not name or team.get('country_number') in range(80,86):
                    continue
                key = (team.get('country_number'),team.get('team_ordinal'),t.get('uuid'))
                matches = index.setdefault(norm(name), {})
                entry = matches.setdefault(key, {'name':name,'file':path.name,'uuid':t.get('uuid'),'country':key[0],'ordinal':key[1],'players':set()})
                ps=t.get('players',[])+t.get('extra_players',[])
                entry['players'].update(norm(x['player_name']) for x in ps if not placeholder(x.get('player_name')))
                entry['test_slots']=entry.get('test_slots',0)+sum(placeholder(x.get('player_name')) for x in ps)
                entry['extra_count']=entry.get('extra_count',0)+len(t.get('extra_players',[]))
    return index

def existing_query(fifa,name):
    labels=" ".join(json.dumps(x,ensure_ascii=False)+"@en" for x in sorted({name,name.title()}))
    return f"""PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    SELECT DISTINCT ?club ?clubLabel WHERE {{
       VALUES ?label {{ {labels} }}
       {{ ?country wdt:P3441 {json.dumps(fifa)}. }} UNION {{ ?entity wdt:P3441 {json.dumps(fifa)}; wdt:P17 ?country. }}
       ?club (rdfs:label|<http://www.w3.org/2004/02/skos/core#altLabel>) ?label; wdt:P17 ?country; wdt:P2094 ?class.
       VALUES ?class {{ wd:Q31930761 wd:Q35554451 }}
       FILTER NOT EXISTS {{ ?club wdt:P2094 ?other. FILTER(?other NOT IN (wd:Q31930761,wd:Q35554451)) }}
       {LABEL}
    }}"""
def merge_players(club,rows,known):
    saved={(p["player"],p.get("membership"),p.get("start"),p.get("end")):p for p in club.get("players",[])}
    for p in rows:
        if p.get("playerLabel") and p.get("start") and p["start"]<=CUTOFF and (not p.get("end") or p["end"]>CUTOFF):
            if norm(p["playerLabel"]) not in known:saved.setdefault((p["player"],p.get("membership"),p.get("start"),p.get("end")),p)
    return update_changed(club, players=list(saved.values()), status="P" if saved else "NO DATA")
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--self-test",action="store_true")
    ap.add_argument("--repo-root",type=Path,default=HERE.parent)
    ap.add_argument("--export-only",action="store_true")
    ap.add_argument("--skip-alternative",action="store_true")
    ap.add_argument("--alternative-only",action="store_true")
    args=ap.parse_args()
    assert placeholder("TEST 1") and placeholder("RESERVE 2") and not placeholder("A B")
    assert "P2094 ?class" in league_query(["BHU"])
    c={};sample={"player":"Q1","playerLabel":"Real Name","start":"2026-01-01"}
    assert merge_players(c,[sample,sample],set()) and len(c["players"])==1
    assert not merge_players(c,[sample,sample],set())
    assert not merge_players(c,[],set()) and c["status"]=="P"
    if args.self_test:
        from tempfile import TemporaryDirectory
        with TemporaryDirectory() as folder:
            testpath=Path(folder)/"candidates.json"
            data={"players":[sample],"status":"P"}
            write_json(testpath,data)
            before=testpath.read_bytes()
            cp=Checkpoint(testpath,data,before)
            cp.update(data,status="P")
            assert not cp.save() and testpath.read_bytes()==before
            cp.update(data,status="NEW")
            assert cp.save() and json.loads(testpath.read_bytes())["status"]=="NEW"
            assert not cp.save()
            cp.update(data,status="NEXT")
            testpath.write_bytes(before)
            try:
                cp.save()
            except RuntimeError:
                assert testpath.read_bytes()==before
            else:
                raise AssertionError("Concurrent modification not detected")
        storage_selftest()
        from alternative_sources import selftest as alt_selftest
        alt_selftest()
        print("Self-test PASS");return
    root=args.repo_root.resolve();raw=(root/MAP).read_bytes();national=json.loads(raw)["records"]
    targets=[dict(fifa=r["fifa"],name=r["source"]["name"],new=r["expanded"]["globalNumber"]>=2560) for r in national if r["source"]["team_file"]==255]
    targets.sort(key=lambda r:(not r["new"],r["fifa"]))
    index=existing_index(root);path=OUT/"candidates.json";CACHE.mkdir(parents=True,exist_ok=True)
    store=Store(path,OUT/'research.sqlite3')
    results=store.load()
    if args.export_only:
        store.publish();store.close();return
    cp=Checkpoint(path,results,None)
    def save_sqlite():
        if not cp.dirty:return False
        changed=store.save(results);cp.dirty=False
        return changed
    cp.save=save_sqlite
    cp.update(results,status="CANONICAL_NOMINAL_RESEARCH_NOT_RUNTIME",map_sha256=hashlib.sha256(raw).hexdigest())
    cp.ensure(results,"existing_club_gaps",{})
    cp.ensure(results,"errors",[])
    bycode={x["fifa"]:x for x in results["targets"]}
    for t in targets:cp.ensure(bycode,t["fifa"],dict(t,leagues=[],status="NO DATA"))
    cp.update(results,targets=list(bycode.values()))
    save=cp.save
    def run(q,phase):
        print(phase,flush=True)
        try:return query(q)
        except Exception as e:
            error={"phase":phase,"error":str(e)}
            if error not in results["errors"]:cp.append(results["errors"],error)
            save();return []
    rawmap={}
    for r in national:
        f=r["source"]["team_file"]
        if f!=255:rawmap.setdefault(f,set()).add(r["fifa"])
    gaps=results["existing_club_gaps"];known={}
    for matches in index.values():
        for v in matches.values():
            uid=v["uuid"]
            if not uid:continue
            known[uid]=v["players"]
            if len(v["players"])>=30 and not v["test_slots"]:continue
            cs=rawmap.get(v["country"],set()) if not v["file"].startswith("GUESTS_") else set()
            e=cp.ensure(gaps,uid,{"name":v["name"],"file":v["file"],"players":[],"status":"NO DATA","identity_candidates":[]})
            cp.update(e,real_names=len(v["players"]),test_slots=v["test_slots"],extra_count=v["extra_count"],capacity_gap=max(0,30-len(v["players"])),association=next(iter(cs)) if len(cs)==1 else None)
    pool=root/"data_json/worlds/2627/competitions/bottom_pools.json"
    if pool.exists():
        for c in json.loads(pool.read_text(encoding="utf-8-sig")).get("clubs",[]):
            if c.get("administrative_test") or c.get("roster",{}).get("template")!="TEST_ZERO":continue
            key="pool:"+str(c["key"]);cs=rawmap.get(c.get("country"),set());known[key]=set()
            cp.ensure(gaps,key,{"name":c["name"],"file":"bottom_pools.json","players":[],"status":"NO DATA","identity_candidates":[],"association":next(iter(cs)) if len(cs)==1 else None,"test_slots":c["roster"].get("base_slots",16),"real_names":0})
    save()
    try:
        if not args.skip_alternative:
            from alternative_sources import acquire
            try:
                acquire(results,cp,CACHE,root,known)
                save()
            except Exception as e:
                error={"phase":"OpenFootballBase","error":str(e)}
                if error not in results["errors"]:cp.append(results["errors"],error)
                save();print(error,flush=True)
        if args.alternative_only:return
        for t in targets:
            c=bycode[t["fifa"]]
            if c.get("leagues"):continue
            for r in run(league_query([t["fifa"]]),"League "+t["fifa"]):
                if r.get("class","").rsplit("/",1)[-1] not in ("Q31930761","Q35554451"):continue
                if not any(x["league"]==r["league"] for x in c["leagues"]):cp.append(c["leagues"],dict(r,seasons=[]))
            cp.update(c,status="P" if c["leagues"] else "NO DATA");save()
        byleague={}
        for t in results["targets"]:
            for l in t["leagues"]:
                if l.get("class","").rsplit("/",1)[-1] in ("Q31930761","Q35554451"):byleague.setdefault(l["league"],[]).append(l)
        for league,ls in byleague.items():
            for r in run(clubs_query([league]),"Edition "+qid(league)):
                for l in ls:
                    s=next((s for s in l["seasons"] if s["season"]==r["season"]),None)
                    if s is None:
                        s={k:v for k,v in r.items() if k not in ("club","clubLabel","league")};s["clubs"]=[];cp.append(l["seasons"],s)
                    if r.get("club") and not any(x["club"]==r["club"] for x in s["clubs"]):cp.append(s["clubs"],{"club":r["club"],"name":r.get("clubLabel",qid(r["club"])),"players":[],"status":"NO DATA"})
            save()
        byclub={}
        for t in results["targets"]:
            for l in t["leagues"]:
                if l.get("class","").rsplit("/",1)[-1] not in ("Q31930761","Q35554451"):continue
                for s in l["seasons"]:
                    for c in s["clubs"]:byclub.setdefault(c["club"],[]).append((c,set()))
        for uid,e in sorted(gaps.items(),key=lambda x:(not bool(x[1].get("test_slots")),x[1].get("real_names",0),x[0])):
            if uid not in known or not e.get("association"):continue
            if not e.get("identity_candidates"):
                cp.update(e,identity_candidates=sorted({r["club"] for r in run(existing_query(e["association"],e["name"]),"Club "+e["name"])}));save()
            qs=e["identity_candidates"]
            if len(qs)==1:byclub.setdefault(qs[0],[]).append((e,known[uid]))
        for group in chunks(sorted(byclub),5):
            rr=run(players_query(group),"Players "+",".join(qid(q) for q in group))
            for q in group:
                for c,k in byclub[q]:
                    if merge_players(c,[r for r in rr if qid(r["club"])==qid(q)],k):cp.dirty=True
            save()
    finally:
        try:
            save();store.publish()
        finally:store.close()
    print("Canonical nominal matrix updated. NO DATA unresolved; P candidates; runtime unchanged. Errors:",len(results["errors"]))
if __name__=="__main__":main()
