#!/usr/bin/env python3
"""Read-only Expanded catalogue survey; Wikidata candidates, not runtime data."""
import argparse, csv, hashlib, json, re, sys, time, unicodedata, urllib.parse, urllib.request, urllib.error
from pathlib import Path
from roster_storage import Store, PublicationConflict, recover_snapshot, selftest as storage_selftest
from http_retry import fetch, HostCooldown, DeadlineExceeded, BLOCKED, INFLIGHT
from research_status import report, outcome
from player_roles import position_evidence, merge_positions
from season_config import output_dir, cutoff, year_value

HERE = Path(__file__).resolve().parent
MAP = Path('docs/NATIONAL_ROSTER_SOURCE_MAP_2627.json')
OUT = HERE / 'results_2627'
CACHE = OUT / 'sources'
ENDPOINT = 'https://query.wikidata.org/sparql'
CUTOFF = '2026-07-01T00:00:00Z'
REFRESH_WIKI = False
YEAR = 2026
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

QUERY_MEMO={}

def query(sparql):
    q = PREFIX + sparql
    key = hashlib.sha256(q.encode()).hexdigest()
    dest = CACHE / (key + '.json')
    data = QUERY_MEMO.get(str(dest))
    if data is None and dest.exists():
        cached = json.loads(dest.read_text(encoding='utf-8'))
        if not REFRESH_WIKI and (cached.get('results',{}).get('bindings') or time.time()-dest.stat().st_mtime<7*86400):
            data=cached
    if data is None:
        req = urllib.request.Request(ENDPOINT, data=urllib.parse.urlencode({'query':q,'format':'json'}).encode(), headers={'User-Agent':'SWOS-Expanded-Roster-Research/1.1 (https://github.com/Giuzy/SWOS-Expanded/issues/2)','Accept':'application/sparql-results+json','Content-Type':'application/x-www-form-urlencoded'})
        data = json.loads(fetch(req, timeout=15, attempts=2, interval=3, total=35))
        if 'results' not in data or 'bindings' not in data['results']:
            raise ValueError('Unexpected query result')
        from roster_storage import atomic
        atomic(dest, data)
        dest.with_suffix('.rq').write_text(q, encoding='utf-8')
    QUERY_MEMO[str(dest)]=data
    return [{k:v['value'] for k,v in row.items()} for row in data['results']['bindings']]

def league_query(codes):
    quoted = ' '.join(json.dumps(c) for c in codes)
    return f'''SELECT DISTINCT ?fifa ?country ?countryLabel ?league ?leagueLabel ?class WHERE {{
      VALUES ?fifa {{ {quoted} }}
      {{ ?country wdt:P3441 ?fifa. }} UNION {{ ?entity wdt:P3441 ?fifa; wdt:P1532 ?country. }} UNION {{ ?entity wdt:P3441 ?fifa; wdt:P17 ?country. }}
      ?league wdt:P17 ?country; wdt:P3983 1; wdt:P2094 ?class.
      VALUES ?class {{ wd:Q31930761 wd:Q35554451 }}
      # Other P2094 facets may describe age/status, not a different sex.
      FILTER NOT EXISTS {{ ?league (wdt:P2094|wdt:P641) ?excludedClass. VALUES ?excludedClass {{ wd:Q606060 wd:Q44418853 wd:Q44418672 wd:Q44418451 wd:Q44418152 wd:Q44408003 wd:Q44417973 wd:Q44417630 wd:Q45053375 wd:Q44417376 }} }}
      FILTER NOT EXISTS {{ ?league wdt:P576 ?end. FILTER(?end < "{CUTOFF}"^^xsd:dateTime) }}
      {LABEL}
    }}'''

def clubs_query(leagues):
    return f'''SELECT DISTINCT ?league ?season ?seasonLabel ?start ?end ?year ?expected ?club ?clubLabel WHERE {{
      VALUES ?league {{ {values(leagues)} }}
      ?season wdt:P3450 ?league.
      OPTIONAL {{ ?season wdt:P580 ?start. }} OPTIONAL {{ ?season wdt:P582 ?end. }}
      OPTIONAL {{ ?season wdt:P585 ?year. }} OPTIONAL {{ ?season wdt:P1132 ?expected. }}
      FILTER((!BOUND(?start) && !BOUND(?year)) || (BOUND(?start) && ?start >= "{YEAR-1}-07-01T00:00:00Z"^^xsd:dateTime && ?start < "{YEAR+1}-01-01T00:00:00Z"^^xsd:dateTime) || (BOUND(?year) && YEAR(?year)={YEAR}))
      FILTER(!BOUND(?end) || ?end >= "{CUTOFF}"^^xsd:dateTime)
      FILTER NOT EXISTS {{ ?season (wdt:P2094|wdt:P641) ?excludedClass. VALUES ?excludedClass {{ wd:Q606060 wd:Q44418853 wd:Q44418672 wd:Q44418451 wd:Q44418152 wd:Q44408003 wd:Q44417973 wd:Q44417630 wd:Q45053375 wd:Q44417376 }} }}
      OPTIONAL {{ {{ ?season wdt:P1923 ?club. }} UNION {{ ?club wdt:P1344 ?season. }} }}
      {LABEL}
    }}'''

def players_query(clubs):
    return f'''SELECT DISTINCT ?club ?player ?playerLabel ?membership ?start ?end ?position ?positionLabel WHERE {{
      VALUES ?club {{ {values(clubs)} }}
      ?player wdt:P106 wd:Q937857; wdt:P21 wd:Q6581097; p:P54 ?membership.
      ?membership ps:P54 ?club; wikibase:rank ?rank.
      OPTIONAL {{ ?membership pq:P580 ?start. }}
      FILTER(?rank != wikibase:DeprecatedRank)
      OPTIONAL {{ ?player wdt:P413 ?position. }}
      OPTIONAL {{ ?membership pq:P582 ?end. }}
      FILTER(!BOUND(?start) || ?start <= "{CUTOFF}"^^xsd:dateTime)
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

def existing_queries(fifa,names):
    labels=" ".join("("+json.dumps(name,ensure_ascii=False)+" "+json.dumps(label,ensure_ascii=False)+"@en)" for name in sorted(set(names)) for label in sorted({name,name.title()}))
    return f"""PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    SELECT DISTINCT ?input ?club ?clubLabel WHERE {{
       VALUES (?input ?label) {{ {labels} }}
       {{ ?country wdt:P3441 {json.dumps(fifa)}. }} UNION {{ ?entity wdt:P3441 {json.dumps(fifa)}; (wdt:P1532|wdt:P17) ?country. }}
       ?club (rdfs:label|<http://www.w3.org/2004/02/skos/core#altLabel>) ?label; wdt:P17 ?country; wdt:P2094 ?class.
       VALUES ?class {{ wd:Q31930761 wd:Q35554451 }}
       FILTER NOT EXISTS {{ ?club (wdt:P2094|wdt:P641) ?excludedClass. VALUES ?excludedClass {{ wd:Q606060 wd:Q44418853 wd:Q44418672 wd:Q44418451 wd:Q44418152 wd:Q44408003 wd:Q44417973 wd:Q44417630 wd:Q45053375 wd:Q44417376 }} }}
       {LABEL}
    }}"""
def existing_query(fifa,name):
    return existing_queries(fifa,[name])

def identity_batches(gaps,known):
    groups={}
    for uid,e in sorted(gaps.items(),key=lambda x:(not bool(x[1].get("test_slots")),x[1].get("real_names",0),x[0])):
        if uid in known and e.get("association") and not e.get("identity_candidates"):
            groups.setdefault(e["association"],[]).append((uid,e))
    for code,entries in groups.items():
        for group in chunks(entries,12):
            yield code,group

def apply_identities(cp,entries,rows):
    for _,e in entries:
        ids=sorted({r["club"] for r in rows if r.get("input")==e["name"] and re.fullmatch(r"Q[1-9][0-9]*",str(r.get("club","")).rsplit("/",1)[-1])})
        if ids:cp.update(e,identity_candidates=ids)

def deferred_error(e):
    return isinstance(e,(HostCooldown,DeadlineExceeded,TimeoutError,urllib.error.URLError)) and (not isinstance(e,urllib.error.HTTPError) or e.code in (408,429,500,502,503,504))

def wiki_ready():
    host=urllib.parse.urlsplit(ENDPOINT).netloc
    worker=INFLIGHT.get(host)
    return time.monotonic()>=BLOCKED.get(host,0) and not (worker and worker.is_alive())

def merge_players(club,rows,known):
    # Preserve legacy evidence byte-for-byte; only prevent new duplicates.
    saved=list(club.get("players",[]))
    def key(p):
        membership=p.get("membership")
        try:player=qid(p["player"])
        except (ValueError,TypeError,AttributeError,KeyError):player=p.get("player")
        return (player,membership.rsplit("/",1)[-1] if membership else None,p.get("start"),p.get("end"))
    existing={key(p):p for p in saved}
    changed=False
    for raw in rows:
        p={k:v for k,v in raw.items() if k not in ("position","positionLabel")}
        pos=position_evidence(raw.get("positionLabel"),"Wikidata","CAREER_POSITION_NOT_EDITION",raw.get("player"),raw.get("position"))
        if pos:p["positionObservations"]=merge_positions(p.get("positionObservations"),pos)
        if p.get("playerLabel") and p.get("start") and p["start"]<=CUTOFF and (not p.get("end") or p["end"]>CUTOFF):
            k=key(p)
            if norm(p["playerLabel"]) not in known:
                if k not in existing:
                    saved.append(p);existing[k]=p
                else:
                    old=existing[k]
                    merged=merge_positions(old.get("positionObservations"),p.get("positionObservations"))
                    if merged!=old.get("positionObservations",[]):
                        replacement=dict(old,positionObservations=merged)
                        saved[saved.index(old)]=replacement;existing[k]=replacement;changed=True
    return update_changed(club, players=saved, status="P" if saved else "NO DATA") or changed
def extra_candidates_by_name(owner,native_names):
    """Compute import review groups; never create a second roster or write TEAM."""
    groups={}
    for field in ("players","alternative_players","transfermarkt_players","wikidata_undated_players","wiki_squad_players"):
        for row in owner.get(field,[]):
            label=row.get("playerLabel")
            if placeholder(label):continue
            k=norm(label)
            if not k or k in native_names:continue
            groups.setdefault(k,[]).append((field,row))
    # Cross-source matches remain evidence, not extra people. Distinct IDs
    # with identical names must be reviewed, not silently merged/imported.
    return groups

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--self-test",action="store_true")
    ap.add_argument("--repo-root",type=Path,default=HERE.parent)
    ap.add_argument("--year",type=year_value,default=2026,help="Starting calendar year; later years use separate all-country research output")
    ap.add_argument("--export-only",action="store_true")
    ap.add_argument("--recover-snapshot",action="store_true",help="Export existing SQLite data read-only to a new local recovery directory")
    ap.add_argument("--restart-from-json",action="store_true",help="Use separate resumable work DB seeded from published shards; preserve original SQLite and WAL")
    ap.add_argument("--refresh-wiki",action="store_true",help="Refresh cached Wikidata queries and Wikipedia pages")
    ap.add_argument("--skip-wikipedia",action="store_true")
    ap.add_argument("--prepare-import",action="store_true",help="Read published JSON only, generate country candidate lists; no downloads")
    ap.add_argument("--country",help="Limit import preview to one FIFA code (requires --prepare-import)")
    ap.add_argument("--skip-alternative",action="store_true")
    ap.add_argument("--alternative-only",action="store_true")
    ap.add_argument("--skip-transfermarkt",action="store_true")
    args=ap.parse_args()
    global REFRESH_WIKI
    REFRESH_WIKI=args.refresh_wiki
    assert placeholder("TEST 1") and placeholder("RESERVE 2") and not placeholder("A B")
    assert "P2094 ?class" in league_query(["BHU"])
    c={};sample={"player":"Q1","playerLabel":"Real Name","start":"2026-01-01"}
    assert merge_players(c,[sample,sample],set()) and len(c["players"])==1
    assert not merge_players(c,[sample,sample],set())
    assert not merge_players(c,[dict(sample,player="https://www.wikidata.org/entity/Q1",playerLabel="Alias")],set())
    native={"joaosilva"}
    d={}
    merge_players(d,[dict(sample,player="Q2",playerLabel="João Silva")],native)
    assert not d["players"]
    grouped=extra_candidates_by_name({"players":[dict(sample,playerLabel="João Silva"),dict(sample,playerLabel="John Doe")],
                                      "transfermarkt_players":[{"playerLabel":"Joao Silva","transfermarktPlayerId":"99"}]},
                                     {"johndoe"})
    assert list(grouped)==["joaosilva"] and len(grouped["joaosilva"])==2
    assert not merge_players(c,[],set()) and c["status"]=="P"
    if args.self_test:
        from http_retry import selftest as http_selftest
        http_selftest()
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
        sample_gaps={"a":{"association":"ITA","name":"Same"},"b":{"association":"ENG","name":"Same"},"c":{"association":"ITA","name":"Other","identity_candidates":["Q9"]}}
        assert sorted((code,len(group)) for code,group in identity_batches(sample_gaps,set(sample_gaps)))==[("ENG",1),("ITA",1)]
        cp=Checkpoint(None,{},None)
        apply_identities(cp,[("a",sample_gaps["a"])],[{"input":"Other","club":"Q2"},{"input":"Same","club":"Q1"},{"input":"Same","club":"invalid"}])
        assert sample_gaps["a"]["identity_candidates"]==["Q1"] and "identity_candidates" not in sample_gaps["b"]
        assert deferred_error(HostCooldown("test")) and not deferred_error(ValueError("bad"))
        from player_roles import selftest as roles_selftest
        roles_selftest()
        from role_backfill import selftest as backfill_selftest
        backfill_selftest()
        from domestic_catalog import selftest as catalogue_selftest
        catalogue_selftest()
        from wiki_sources import selftest as wiki_selftest
        wiki_selftest()
        from season_config import selftest as season_selftest
        season_selftest()
        from annual_delta import selftest as delta_selftest
        delta_selftest()
        from alternative_sources import selftest as alt_selftest
        alt_selftest()
        from transfermarkt_source import selftest as tm_selftest
        tm_selftest()
        from import_candidates import selftest as import_selftest
        import_selftest()
        print("Self-test PASS");return
    global YEAR, CUTOFF, OUT, CACHE
    YEAR=args.year;CUTOFF=cutoff(YEAR)
    root=args.repo_root.resolve();OUT=output_dir(root,YEAR);CACHE=OUT/"sources"
    raw=(root/MAP).read_bytes();national=json.loads(raw)["records"]
    if args.country:args.country=args.country.upper()
    from import_candidates import prepare as prepare_import
    out=OUT;preview=out/"import_candidates"
    if args.prepare_import:
        prepare_import(out/"candidates.json",preview,args.country,year=YEAR)
        if YEAR!=2026:
            from annual_delta import write_delta
            write_delta(root,YEAR,scope='rosters')
        return
    targets=[dict(fifa=r["fifa"],name=r["source"]["name"],new=r["expanded"]["globalNumber"]>=2560) for r in national if YEAR!=2026 or r["source"]["team_file"]==255]
    targets.sort(key=lambda r:(not r["new"],r["fifa"]))
    if args.country:ap.error("--country requires --prepare-import")
    index=existing_index(root);path=OUT/"candidates.json";CACHE.mkdir(parents=True,exist_ok=True)
    if not path.exists() and YEAR!=2026:
        from roster_storage import atomic
        atomic(path,{"targets":[],"existing_club_gaps":{},"errors":[],"cutoff":CUTOFF,"status":"CANONICAL_NOMINAL_RESEARCH_NOT_RUNTIME","researchYear":YEAR})
    dbpath=OUT/('research.sqlite3.restart' if args.restart_from_json else 'research.sqlite3')
    print('Roster work database:',dbpath,flush=True)
    if args.recover_snapshot:
        from roster_storage import recover_snapshot
        print("Recovered SQLite snapshot:",recover_snapshot(dbpath,OUT/"sources"),flush=True)
        return
    store=Store(path,dbpath)
    results=store.load()
    if args.export_only:
        try:
            store.publish();prepare_import(path,preview,data=results,year=YEAR)
            if YEAR!=2026:
                from annual_delta import write_delta
                write_delta(root,YEAR,scope='rosters')
        finally:store.close()
        return
    # Recover/publish work from earlier interrupted or pre-checkpoint runs first.
    try:store.publish()
    except BaseException:
        store.close();raise
    cp=Checkpoint(path,results,None)
    publication_failed=False
    def save_sqlite():
        nonlocal publication_failed
        if publication_failed:raise RuntimeError("Roster publication failed; close and reopen before continuing")
        if not cp.dirty:return False
        changed=store.save(results);cp.dirty=False
        if changed:
            try:store.publish()
            except BaseException:
                publication_failed=True
                raise
            report("Roster country JSON","SAVED","SQLite checkpoint published")
        return changed
    cp.save=save_sqlite
    if results.get("researchYear",2026)!=YEAR:
        store.close();raise RuntimeError("Research year mismatch")
    cp.update(results,status="CANONICAL_NOMINAL_RESEARCH_NOT_RUNTIME",map_sha256=hashlib.sha256(raw).hexdigest(),researchYear=YEAR)
    cp.ensure(results,"existing_club_gaps",{})
    cp.ensure(results,"errors",[])
    bycode={x["fifa"]:x for x in results["targets"]}
    for t in targets:cp.ensure(bycode,t["fifa"],dict(t,leagues=[],status="NO DATA"))
    cp.update(results,targets=list(bycode.values()))
    save=cp.save
    query_failed=False
    query_status=dict(results.get("wikidataQueryStatus",{}))
    query_errors=[]
    def flush_query_status():
        cp.update(results,wikidataQueryStatus=dict(query_status))
        for error in query_errors:
            if error not in results["errors"]:cp.append(results["errors"],error)
        query_errors.clear()
        save()
    def tracked_query(q,phase="Wiki club bridge"):
        key=hashlib.sha256((PREFIX+q).encode()).hexdigest()
        try:rows=query(q)
        except Exception as e:
            query_status[key]={"phase":phase,"status":"DEFERRED" if deferred_error(e) else "ERROR","reason":str(e)}
            if not isinstance(e,HostCooldown):
                error={"phase":phase,"error":str(e)}
                if error not in query_errors:query_errors.append(error)
            raise
        query_status[key]={"phase":phase,"status":"COMPLETED" if rows else "NO DATA","rows":len(rows)}
        return rows
    def run(q,phase):
        nonlocal query_failed
        query_failed=False
        report("Wikidata "+phase,"SEARCHING")
        try:
            rows=tracked_query(q,phase)
            report("Wikidata "+phase,"FOUND" if rows else "NO DATA",f"{len(rows)} source rows; validation follows")
            return rows
        except Exception as e:
            query_failed=True
            report("Wikidata "+phase,"DEFERRED" if deferred_error(e) else "ERROR",str(e));return []
    def query_batches(jobs):
        pending=[]
        for payload,q,label in jobs:
            rows=run(q,label)
            if query_failed:pending.append((payload,q,label))
            else:yield payload,rows
        for payload,q,label in pending:
            if not wiki_ready():break
            rows=run(q,label)
            if not query_failed:yield payload,rows
        flush_query_status()
    rawmap={}
    for r in national:
        f=r["source"]["team_file"]
        if f!=255:rawmap.setdefault(f,set()).add(r["fifa"])
    gaps=results["existing_club_gaps"];known={}
    for matches in index.values():
        for v in matches.values():
            uid=v["uuid"]
            if not uid:continue
            if YEAR==2026 and len(v["players"])>=30 and not v["test_slots"]:continue
            # Later years collect complete observed rosters, not only absent names.
            known[uid]=v["players"] if YEAR==2026 else set()
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
    from transfermarkt_source import Transfermarkt
    from wiki_sources import Wiki, league_club_query, valid_club_name, qid as safe_qid
    from season_config import edition_label
    from domestic_catalog import Catalogue, eligible_league, observation_key as domestic_key
    catalogue=Catalogue(CACHE/'domestic_catalog',national,refresh=args.refresh_wiki,wikipedia=not args.skip_wikipedia and not args.alternative_only,rsssf=not args.skip_alternative)
    wiki=Wiki(CACHE,cp,results,tracked_query,refresh=args.refresh_wiki)
    tm=Transfermarkt(CACHE,cp,results,national,known,year=YEAR)
    def alternative(phase):
        if not args.skip_alternative:
            try:
                from alternative_sources import acquire
                acquire(results,cp,CACHE,root,known,phase=phase,year=YEAR)
                save()
            except Exception as e:
                error={"phase":"OpenFootballBase/"+phase,"error":str(e)}
                if error not in results["errors"]:cp.append(results["errors"],error)
                save();report("OpenFootballBase "+phase,"ERROR",str(e))
        if not args.skip_transfermarkt:
            tm.acquire(phase,targets)
            save()
    def collect_players(byclub):
        jobs=[(group,players_query(group),"Players "+",".join(qid(q) for q in group)) for group in chunks(sorted(byclub),5)]
        for group,rr in query_batches(jobs):
            label="Players "+",".join(qid(q) for q in group)
            added=0
            for q in group:
                for c,k in byclub[q]:
                    before=len(c.get("players",[]))
                    candidates=[r for r in rr if qid(r["club"])==qid(q)]
                    if merge_players(c,candidates,k):cp.dirty=True
                    for r in candidates:
                        if not r.get('start') and r.get('playerLabel') and norm(r['playerLabel']) not in k:
                            leads=cp.ensure(c,'wikidata_undated_players',[])
                            row=dict({k:v for k,v in r.items() if k not in ("position","positionLabel")},scope='MEMBERSHIP_DATE_UNKNOWN_NOT_JULY1')
                            pos=position_evidence(r.get("positionLabel"),"Wikidata","CAREER_POSITION_NOT_EDITION",r.get("player"),r.get("position"))
                            if pos:row["positionObservations"]=pos
                            from player_roles import enrich
                            matches=[old for old in leads if {k:v for k,v in old.items() if k!="positionObservations"}=={k:v for k,v in row.items() if k!="positionObservations"}]
                            if matches:enrich(cp,matches[0],row)
                            else:cp.append(leads,row)
                    added+=len(c.get("players",[]))-before
            save()
            report("Wikidata "+label,*outcome(len(rr),added))
    try:
        print("PHASE 1/3: clubs in new countries, then other inactive countries",flush=True)
        alternative("clubs")
        for t in targets:
            code=t["fifa"];owner=bycode[code]
            packets,seeds,errors=catalogue.country(code,YEAR)
            observations=cp.ensure(owner,"domestic_source_observations",[])
            keys={domestic_key(p) for p in observations}
            for p in packets:
                key=domestic_key(p)
                if key not in keys:cp.append(observations,p);keys.add(key)
            for error in errors:
                row={"phase":"Domestic catalogue/"+code,"error":str(error)}
                if row not in results["errors"]:cp.append(results["errors"],row)
            if not args.alternative_only:
                for seed in seeds:
                    if seed.get("kind")=="LEAGUE_DIVISION" and eligible_league(seed) and not any(safe_qid(x["league"])==safe_qid(seed["league"]) for x in owner["leagues"]):
                        cp.append(owner["leagues"],dict(seed,seasons=[]))
                # Archive entrant names are identity review leads, never a seasonal club binding.
                labels={v["name"]:p for p in packets if p.get("source")=="RSSSF" and p.get("kind")=="LEAGUE_DIVISION" for v in p.get("participants",[]) if valid_club_name(v.get("name",""))}
                for group in chunks(sorted(labels),12):
                    rows=run(existing_queries(code,group),"Archive club identities "+code)
                    for name in group:
                        matches={safe_qid(row.get("club")):row for row in rows if row.get("input")==name and safe_qid(row.get("club")) and valid_club_name(row.get("clubLabel",""))}
                        if len(matches)==1:
                            row=next(iter(matches.values()))
                            wiki.add(owner,"wiki_club_leads",dict(row,country=code,sourceLabel=name,sourceUrl=labels[name]["sourceUrl"],scope="RSSSF_NAME_IDENTITY_REVIEW_NOT_EDITION"))
            save()
        if not args.alternative_only:
            pending=[]
            for t in targets:
                code=t["fifa"]
                if any(eligible_league(l) for l in bycode[code]["leagues"]) and not args.refresh_wiki:
                    report("Wikidata League "+code,"SKIPPED","existing league candidates")
                else:pending.append(code)
            for group in chunks(pending,5):
                rr=run(league_query(group),"Leagues "+",".join(group))
                for code in group:
                    owner=bycode[code];added=0;found=0
                    for row in rr:
                        if row.get("fifa")!=code or not safe_qid(row.get("league")) or not eligible_league(row) or not valid_club_name(row.get("leagueLabel","")):continue
                        found+=1
                        if not any(safe_qid(x["league"])==safe_qid(row["league"]) for x in owner["leagues"]):
                            cp.append(owner["leagues"],dict(row,seasons=[]));added+=1
                    if not query_failed:
                        cp.update(owner,status="P" if owner["leagues"] else "NO DATA")
                        report("Wikidata League "+code,*outcome(found,added))
                    save()
            byleague={}
            for t in (bycode[x["fifa"]] for x in targets):
                for l in t["leagues"]:
                    if eligible_league(l):byleague.setdefault(l["league"],[]).append(l)
            for league,ls in byleague.items():
                label="Edition "+qid(league);rr=run(clubs_query([league]),label);added=0
                for r in rr:
                    if safe_qid(r.get("league"))!=safe_qid(league) or not safe_qid(r.get("season")) or not edition_label(r.get("seasonLabel",""),YEAR) or not valid_club_name(r.get("seasonLabel","")):continue
                    for l in ls:
                        s=next((s for s in l["seasons"] if s["season"]==r["season"]),None)
                        if s is None:
                            s={k:v for k,v in r.items() if k not in ("club","clubLabel","league")};s["clubs"]=[];cp.append(l["seasons"],s)
                        if safe_qid(r.get("club")) and valid_club_name(r.get('clubLabel','')) and not any(x["club"]==r["club"] for x in s["clubs"]):
                            cp.append(s["clubs"],{"club":r["club"],"name":r.get("clubLabel",qid(r["club"])),"players":[],"status":"NO DATA"});added+=1
                save()
                if not query_failed:report("Wikidata "+label,*outcome(sum(bool(x.get("club")) for x in rr),added))
            # P118 is a league/club identity lead, not proof of the 2026 edition.
            for t in targets:
                owner=bycode[t['fifa']]
                for l in owner['leagues']:
                    if not eligible_league(l) or any(s.get('clubs') for s in l.get('seasons',[])):continue
                    for row in run(league_club_query([l['league']]),'League clubs '+t['fifa']+'/'+qid(l['league'])):
                        if valid_club_name(row.get('clubLabel','')):
                            wiki.add(owner,'wiki_club_leads',dict(row,country=t['fifa'],scope='LEAGUE_IDENTITY_NOT_EDITION'))
                    save()
        flush_query_status()
        prepare_import(path,preview,include_extras=False,data=results,year=YEAR)
        print("PHASE 2/3: players of discovered clubs",flush=True)
        alternative("players")
        if not args.alternative_only:
            byclub={}
            for t in (bycode[x["fifa"]] for x in targets):
                for l in t["leagues"]:
                    if not eligible_league(l):continue
                    for s in l["seasons"]:
                        for c in s["clubs"]:byclub.setdefault(c["club"],[]).append((c,set()))
            collect_players(byclub)
        if not args.skip_wikipedia and not args.alternative_only:
            for t in targets:
                wiki.target(bycode[t['fifa']]);save()
        from role_backfill import backfill
        def run_role(q,label):
            rows=run(q,label)
            return None if query_failed else rows
        print("PHASE 2/3: backfill missing roles on existing country observations",flush=True)
        backfill([bycode[t["fifa"]] for t in targets],cp,tm,run_role,YEAR,allow_tm=not args.skip_transfermarkt,allow_wd=not args.alternative_only)
        flush_query_status()
        prepare_import(path,preview,include_extras=False,data=results,year=YEAR)
        print("PHASE 3/3: existing club roster gaps / extra players",flush=True)
        alternative("extras")
        if not args.alternative_only:
            byclub={}
            jobs=[(entries,existing_queries(code,[e["name"] for _,e in entries]),"Club identities "+code+" ("+str(len(entries))+")") for code,entries in identity_batches(gaps,known)]
            for entries,rows in query_batches(jobs):
                apply_identities(cp,entries,rows);save()
            for uid,e in sorted(gaps.items(),key=lambda x:(not bool(x[1].get("test_slots")),x[1].get("real_names",0),x[0])):
                if uid not in known or not e.get("association"):
                    report("Wikidata extras "+e["name"],"SKIPPED","not eligible or association unresolved")
                    continue
                qs=e.get("identity_candidates",[])
                if len(qs)==1:
                    report("Wikidata extras "+e["association"]+"/"+e["name"],"FOUND","unique identity candidate; player search follows")
                    byclub.setdefault(qs[0],[]).append((e,known[uid]))
                elif qs:report("Wikidata extras "+e["association"]+"/"+e["name"],"REVIEW","ambiguous identity")
            collect_players(byclub)
        if not args.skip_wikipedia and not args.alternative_only:
            wiki.bridge([row for uid,e in gaps.items() if uid in known for row in e.get('transfermarkt_clubs',[])])
            save()
            for uid,e in sorted(gaps.items()):
                if uid in known and e.get('association'):
                    wiki.extra(e,known[uid]);save()
        print("PHASE 3/3: backfill missing roles on already collected EXTRA observations",flush=True)
        backfill(gaps.values(),cp,tm,run_role,YEAR,allow_tm=not args.skip_transfermarkt,allow_wd=not args.alternative_only)
    finally:
        try:
            if not publication_failed:
                flush_query_status();store.publish()
        finally:store.close()
    groups=[extra_candidates_by_name(e,known[uid]) for uid,e in gaps.items() if uid in known]
    prepare_import(path,preview,data=results,year=YEAR)
    if YEAR!=2026:
        from annual_delta import write_delta
        write_delta(root,YEAR,scope='rosters')
    print("EXTRA review: distinct missing club/name groups",sum(map(len,groups)),
          "multi-observation groups",sum(len(v)>1 for g in groups for v in g.values()),
          "(not certified players)",flush=True)
    print("Canonical nominal matrix updated. NO DATA unresolved; P candidates; runtime unchanged. Errors:",len(results["errors"]))
def launch():
    try:main()
    except PublicationConflict as exc:
        print("ROSTER RECOVERY — CONFLICT: "+str(exc),flush=True)
        try:
            dest=recover_snapshot(exc.dbpath,exc.dbpath.parent/"sources")
        except Exception as error:
            print("ROSTER RECOVERY — ERROR: "+str(error),flush=True)
            print("Original SQLite and JSON preserved. Send this window log.",flush=True)
        else:
            print("ROSTER RECOVERY — SAVED: "+str(dest),flush=True)
            print("Original SQLite and JSON preserved. Conflicting observations need review; send this window log.",flush=True)
        raise SystemExit(2)

if __name__=="__main__":launch()
