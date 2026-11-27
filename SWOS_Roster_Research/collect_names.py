#!/usr/bin/env python3
"""Read-only Expanded catalogue survey; Wikidata candidates, not runtime data."""
import csv, hashlib, json, re, sys, time, unicodedata, urllib.parse, urllib.request, urllib.error, zipfile
from pathlib import Path

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

def query(sparql):
    q = PREFIX + sparql
    key = hashlib.sha256(q.encode()).hexdigest()
    dest = CACHE / (key + '.json')
    if dest.exists():
        data = json.loads(dest.read_text(encoding='utf-8'))
    else:
        time.sleep(3)
        req = urllib.request.Request(ENDPOINT + '?' + urllib.parse.urlencode({'query':q,'format':'json'}), headers={'User-Agent':'SWOS-Expanded-Roster-Research/1.0 (https://github.com/Giuzy/SWOS-Expanded/issues/2)','Accept':'application/sparql-results+json'})
        with urllib.request.urlopen(req, timeout=75) as response:
            data = json.load(response)
        if 'results' not in data or 'bindings' not in data['results']:
            raise ValueError('Unexpected query result')
        write_json(dest, data)
        dest.with_suffix('.rq').write_text(q, encoding='utf-8')
    return [{k:v['value'] for k,v in row.items()} for row in data['results']['bindings']]

def league_query(codes):
    quoted = ' '.join(json.dumps(c) for c in codes)
    return f'''SELECT DISTINCT ?fifa ?country ?countryLabel ?league ?leagueLabel ?class WHERE {{
      VALUES ?fifa {{ {quoted} }}
      ?entity wdt:P3441 ?fifa.
      {{ {{ BIND(?entity AS ?country) }} UNION {{ ?entity (wdt:P1532|wdt:P17) ?country. }} }}
      ?league wdt:P17 ?country; wdt:P3983 ?level.
      FILTER(?level = 1)
      {{ ?league wdt:P641 wd:Q2736 }} UNION {{ ?league wdt:P31/wdt:P279* wd:Q15991303 }}
      OPTIONAL {{ ?league wdt:P2094 ?class. }}
      FILTER(!BOUND(?class) || ?class IN (wd:Q31930761, wd:Q35554451))
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
                entry['players'].update(norm(x['player_name']) for x in t.get('players',[])+t.get('extra_players',[]) if x.get('player_name'))
    return index

def main():
    root = next((p for p in [Path(sys.argv[1]).resolve()] if (p/MAP).is_file()),None) if len(sys.argv)>1 else next((p for p in [HERE,*HERE.parents] if (p/MAP).is_file()),None)
    if root is None: raise SystemExit('Estrarre la cartella nella root di SWOS Expanded (docs e data_json).')
    raw = (root/MAP).read_bytes()
    targets = [dict(fifa=r['fifa'],name=r['source']['name'],new=r['expanded']['globalNumber']>=2560) for r in json.loads(raw)['records'] if r['source']['team_file']==255]
    targets.sort(key=lambda r:(not r['new'],r['fifa']))
    assert len({r['fifa'] for r in targets})==len(targets)
    index = existing_index(root)
    seed = json.loads((HERE/'openfootball_clubs.json').read_text(encoding='utf-8'))
    CACHE.mkdir(parents=True,exist_ok=True)
    results={'status':'CANDIDATES_ONLY_NOT_RUNTIME','source':'Wikidata CC0','cutoff':CUTOFF,'map_sha256':hashlib.sha256(raw).hexdigest(),'limitations':['No complete roster claim. Unknown league/season/gender/country requires review.','Only P54 memberships with an explicit start date; missing start dates excluded; no end date is not proof of current affiliation.','Season membership is not proof of participants known on 1 July. Name matches are candidates, not IDs.'],'targets':[dict(x,leagues=[]) for x in targets],'errors':[], 'openfootball_identity_candidates':seed}
    bycode={x['fifa']:x for x in results['targets']}
    def save(): write_json(OUT/'candidates.json',results)
    def run(q,phase):
        try: return query(q)
        except Exception as e:
            results['errors'].append({'phase':phase,'error':str(e)})
            save()
            print('Gruppo non acquisito:', phase, str(e), flush=True)
            return []
    try:
        for group in chunks([r['fifa'] for r in targets],12):
            print('Paesi:', ' '.join(group),flush=True)
            for r in run(league_query(group),'leagues '+','.join(group)):
                entry=dict(r,seasons=[])
                bycode[r['fifa']]['leagues'].append(entry)
            save()
        byleague={}
        for t in results['targets']:
            for l in t['leagues']: byleague.setdefault(l['league'],[]).append(l)
        for group in chunks(sorted(byleague),10):
            print('Edizioni:', len(group),flush=True)
            for r in run(clubs_query(group),'seasons'):
                for l in byleague[r['league']]:
                    season=next((s for s in l['seasons'] if s['season']==r['season']),None)
                    if season is None:
                        season={k:v for k,v in r.items() if k not in ('club','clubLabel','league')}; season['clubs']=[]; l['seasons'].append(season)
                    if 'club' in r and not any(c['club']==r['club'] for c in season['clubs']):
                        season['clubs'].append({'club':r['club'],'name':r.get('clubLabel',qid(r['club'])),'players':[]})
            save()
        byclub={}
        for t in results['targets']:
            for l in t['leagues']:
                for s in l['seasons']:
                    for c in s['clubs']: byclub.setdefault(c['club'],[]).append(c)
        for group in chunks(sorted(byclub),15):
            print('Rose:', len(group),flush=True)
            for r in run(players_query(group),'players'):
                for c in byclub[r['club']]:
                    if not any(x['membership']==r['membership'] for x in c['players']): c['players'].append(r)
            save()
    except (Exception, KeyboardInterrupt) as e:
        print('Raccolta interrotta; risultati parziali conservati. Ripetere il comando per riprendere:',e,flush=True)
    for t in results['targets']:
        for l in t['leagues']:
            for s in l['seasons']:
                for c in s['clubs']:
                    m=index.get(norm(c['name']),{})
                    c['expanded_name_candidates']=[{**v,'players':None,'matching_player_names':sorted({p.get('playerLabel','') for p in c['players'] if norm(p.get('playerLabel','')) in v['players']})} for v in m.values()]
                    c['unique_player_ids']=len({p['player'] for p in c['players']})
    save()
    with (OUT/'coverage.csv').open('w',encoding='utf-8-sig',newline='') as f:
        w=csv.writer(f);w.writerow(['FIFA','country','new','league_candidates','season_candidates','club_ids','player_ids','status'])
        for t in results['targets']:
            ls=t['leagues'];ss=[s for l in ls for s in l['seasons']];cs=[c for s in ss for c in s['clubs']]
            w.writerow([t['fifa'],t['name'],t['new'],len(ls),len(ss),len({c['club'] for c in cs}),len({p['player'] for c in cs for p in c['players']}),'CANDIDATE' if cs else 'NO_DATA'])
    with zipfile.ZipFile(HERE/'SWOS_ROSTER_RESULTS.zip','w',zipfile.ZIP_DEFLATED) as z:
        for p in sorted(OUT.rglob('*')):
            if p.is_file(): z.write(p,p.relative_to(OUT))
    print('File pronto: SWOS_ROSTER_RESULTS.zip. Nessuna modifica al gioco.')
    if results['errors']:
        print('ATTENZIONE: raccolta parziale,',len(results['errors']),'gruppi non acquisiti. Ripetere il BAT per ritentare.')

if __name__=='__main__': main()
