"""Materialize research-only evidence, never game data. Explicit apply, fail closed."""
import csv
import hashlib
import json
import os
from pathlib import Path
import tempfile
import zipfile
from matrix_storage import read_matrix, plan_matrix

FIELDS = ('name','identity','participants','format','calendar','regulation','movement','qualification','july1','evolution')
STATES = {'V','P','I','NA','?'}

def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))

def encode(obj):
    return (json.dumps(obj,ensure_ascii=False,indent=2)+'\n').encode('utf-8')

def digest(obj):
    return hashlib.sha256(json.dumps(obj,ensure_ascii=False,sort_keys=True,separators=(',',':')).encode()).hexdigest()

def unique(rows):
    result = {int(x['row']):x for x in rows}
    if len(result)!=len(rows):
        raise ValueError('Duplicate matrix row')
    return result

def inside(root, relative):
    path = (root / relative).resolve()
    if not path.is_relative_to(root.resolve()) or path.suffix != '.json':
        raise ValueError('Invalid research path: '+str(relative))
    return path

def observation(p):
    keys = ('sourceGroup','shape','fullUniqueIdentityCandidate','completeRegularStructureCandidate',
            'fullNativeRosterCandidate','nativeRosterCandidate','identityCandidates','unmappedOrAmbiguous',
            'fixtureColumns','fixtures','july1Candidate','ruleExcerpts','phasePlan','phaseCategory',
            'phaseQueue','alternatives','historyObservations','crossRowConflict','deepEvidence','deepCrossRowConflict','deepRosterBasis')
    evidence = {k:p[k] for k in keys if k in p}
    # No result columns in fixtures; historical scores only before the world's start.
    if 'fixtures' in evidence and evidence.get('fixtureColumns') != [
        'date','dateBasis','homeLabel','awayLabel','homeUUID','awayUUID','locator','stage']:
        raise ValueError('Unexpected fixture columns')
    cut = '1996-07-01' if p['world']=='9697' else '2026-07-01'
    if any(x[0]>=cut for x in evidence.get('july1Candidate',{}).get('scores',[])):
        raise ValueError('Post-start historical score')
    for edition in evidence.get('deepEvidence',{}).get('editions',[]):
        for e in edition['events']:
            if e['date']>=cut and e.get('preStartScores'):raise ValueError('Deep post-start historical score')
    return evidence

def complete_pair_dates(o):
    if not (o.get('completeRegularStructureCandidate') and o.get('fullUniqueIdentityCandidate')):
        return None
    if o.get('shape',{}).get('family') not in ('DOUBLE_ROUND_ROBIN','SINGLE_ROUND_ROBIN'):
        return None
    g=o.get('sourceGroup',{})
    if g.get('season_basis') not in ('SOURCE_SEASON','SOURCE_SEASON_END_YEAR','SOURCE_TITLE','EXPLICIT_EDITION'):
        return None
    fs=o.get('fixtures',[])
    if not fs or any(len(x)!=8 or x[1]!='EXPLICIT' or not x[4] or not x[5] for x in fs):
        return None
    pairs={(x[4],x[5]):x[0] for x in fs}
    if len(pairs)!=len(fs):
        return None
    return (g.get('season'),g.get('season_basis'),pairs)

def merge_observation(old,p,s,prior_conflicts=()):
    old=old or {}
    versions=dict(old.get('observations',{}))
    o=observation(p)
    if o.get('fixtures') or o.get('historyObservations') or o.get('ruleExcerpts') or o.get('phasePlan') or o.get('nativeRosterCandidate') or o.get('deepEvidence'):
        versions.setdefault(digest(o),o)
    conflicts=set(old.get('conflicts',[])) | set(prior_conflicts)
    if p.get('deepEvidence',{}).get('conflicts'):conflicts.add('DEEP_EDITION_OBSERVATION_CONFLICT')
    if p.get('deepCrossRowConflict'):conflicts.add('DEEP_CROSS_ROW_CANDIDATE_COLLISION')
    if p.get('crossRowConflict'):
        conflicts.add('CURRENT_CROSS_ROW_COLLISION')
    dates=[v for o in versions.values() if (v:=complete_pair_dates(o))]
    for i,(season,basis,pairs) in enumerate(dates):
        for other,obasis,opairs in dates[:i]:
            if (season,basis)==(other,obasis) and any(pairs[k]!=opairs[k] for k in pairs.keys() & opairs.keys()):
                conflicts.add('COMPETING_EXPLICIT_PAIR_DATES')
    fields=dict(s['fields'])
    gaps=[k for k in FIELDS if fields[k] not in ('V','P','NA')]
    # An incomplete candidate never becomes verified merely by being inserted.
    status='I' if gaps or conflicts else ('P' if 'P' in fields.values() else 'V')
    assessments=dict(old.get('assessments',{}))
    assessment={'status':status,'fields':fields}
    assessments.setdefault(digest(assessment),assessment)
    return {**old,'scope':'RESEARCH_ONLY; observations are not runtime definitions or verified results',
            'status':status,'fields':fields,'gaps':gaps,'conflicts':sorted(conflicts),
            'reviewRequired':bool(gaps or conflicts),'observations':versions,'assessments':assessments}

def materialize(root,out,apply=False,expected_count=533):
    docs=(root/'docs/competition_research').resolve()
    index=read(docs/'INDEX.json')
    coverage=read(docs/'COVERAGE.json')
    packets=unique(read(out/'consolidation_packets.json')['packets'])
    mass=read(out/'mass_closure.json')
    states=unique(mass['rows'])
    if not mass['summary'].get('indexedBaseline') or len(states)!=expected_count:
        raise ValueError('Current indexed matrix required')
    if set(states)!={rid for rid,p in packets.items() if p.get('phaseQueue')!='OUT_OF_SCOPE' and rid not in (100,101,102)}:
        raise ValueError('Packet/matrix scope mismatch')
    prior=read_matrix(inside(docs,index['automaticInsertion'])) if index.get('automaticInsertion') else {}
    prior_rows=unique(prior.get('rows',[]))
    if prior_rows and set(prior_rows)!=set(states):raise ValueError('Prior matrix scope mismatch')
    updates={}
    files={}
    seen=set()
    rows=[]
    for rid,s in sorted(states.items()):
        p=packets[rid]
        if set(s['fields'])!=set(FIELDS) or not set(s['fields'].values())<=STATES or s['status'] not in ('V','P','I'):
            raise ValueError('Invalid field status '+str(rid))
        path=inside(docs,index['rowPath'][str(rid)])
        if path not in files:
            files[path]=read(path)
        row=unique(files[path])[rid]
        if any(str(row.get(k) or '')!=str(p.get(k) or '') for k in ('world','id','detail','country')):
            raise ValueError('Current sheet identity mismatch '+str(rid))
        if s['world']!=p['world'] or rid in seen:
            raise ValueError('Matrix identity mismatch '+str(rid))
        seen.add(rid)
        previous=prior_rows.get(rid,{})
        if any(v in ('V','P') and s['fields'].get(k) not in (('V',) if v=='V' else ('V','P')) for k,v in previous.get('fields',{}).items()):
            raise ValueError('Prior reviewed field regression '+str(rid))
        auto=merge_observation(row.get('automaticResearch'),p,s,previous.get('conflicts',[]))
        prior_auto=row.get('automaticResearch')
        row['automaticResearch']=auto
        prior_audit=previous.get('automaticAudit')
        if prior_audit and prior_auto!=auto:
            previous=dict(previous,automaticAudit=dict(prior_audit,stale=True))
        rows.append(dict(previous,row=rid,world=p['world'],name=s['name'] or previous.get('name',''),status=auto['status'],
                         fields=auto['fields'],gaps=auto['gaps'],conflicts=auto['conflicts'],
                         observationVersions=len(auto['observations'])))
    if seen!=set(states):
        raise ValueError('Incomplete materialization')
    counts={k:{v:sum(r['fields'][k]==v for r in rows) for v in sorted(STATES)} for k in FIELDS}
    closure={v:sum(r['status']==v for r in rows) for v in ('V','P','I')}
    stats={'denominator':len(rows),'fields':counts,'closureStatuses':closure,
           'rowsWithEvidence':sum(r['observationVersions']>0 for r in rows),
           'rowsWithConflicts':sum(bool(r['conflicts']) for r in rows),
           'rowsRequiringReview':sum(bool(r['gaps'] or r['conflicts']) for r in rows),
           'closedWithoutFlaggedConflict':sum(r['status'] in ('V','P') and not r['conflicts'] for r in rows),
           'scope':'Research only; previous verified/named and partial counters remain unchanged'}
    rel='closure/AUTOMATIC_INSERTION.json'
    index['automaticInsertion']=rel
    stats=dict(prior.get('summary',{}),**stats)
    if 'automaticAudit' in stats:
        stale=any(r.get('automaticAudit',{}).get('stale') for r in rows)
        stats['automaticAudit']=dict(stats['automaticAudit'],stale=stale)
        if 'automaticPayloadAudit' in coverage:
            coverage['automaticPayloadAudit']=stats['automaticAudit']
    coverage['automaticInsertion']=stats
    for path,content in files.items():
        updates[path]=encode(content)
    updates.update(plan_matrix(inside(docs,rel),dict(prior,summary=stats,rows=rows)))
    updates[docs/'INDEX.json']=encode(index)
    updates[docs/'COVERAGE.json']=encode(coverage)
    originals={p:p.read_bytes() if p.exists() else None for p in updates}
    changed={p:v for p,v in updates.items() if v!=originals[p]}
    # Portable patch and rollback archive. No changes to runtime, input archives or prior files.
    with zipfile.ZipFile(out/'insertion_patch.zip','w',zipfile.ZIP_DEFLATED) as z:
        manifest=[]
        for p,data in sorted(changed.items()):
            name=p.relative_to(root.resolve()).as_posix()
            previous=originals[p]
            z.writestr('new/'+name,data)
            if previous is not None:
                z.writestr('old/'+name,previous)
            manifest.append({'path':name,'before':hashlib.sha256(previous).hexdigest() if previous is not None else None,
                             'after':hashlib.sha256(data).hexdigest()})
        z.writestr('manifest.json',encode(manifest))
    if apply:
        staged={}
        written=[]
        try:
            for p,data in changed.items():
                p.parent.mkdir(parents=True,exist_ok=True)
                with tempfile.NamedTemporaryFile(dir=p.parent,delete=False) as f:
                    staged[p]=Path(f.name)
                    f.write(data)
            for p in changed:
                if (p.read_bytes() if p.exists() else None)!=originals[p]:
                    raise ValueError('Concurrent change: '+str(p))
            for p,temp in staged.items():
                os.replace(temp,p)
                written.append(p)
        except BaseException:
            for p in reversed(written):
                previous=originals[p]
                if previous is None:
                    p.unlink()
                else:
                    with tempfile.NamedTemporaryFile(dir=p.parent,delete=False) as f:
                        f.write(previous)
                        rollback=Path(f.name)
                    os.replace(rollback,p)
            raise
        finally:
            for temp in staged.values():
                temp.unlink(missing_ok=True)
    with (out/'insertion_review_queue.csv').open('w',encoding='utf-8-sig',newline='') as f:
        writer=csv.writer(f)
        writer.writerow(('row','world','name','status','gaps','conflicts'))
        for row in rows:
            if row['gaps'] or row['conflicts']:
                writer.writerow((row['row'],row['world'],row['name'],row['status'],
                                 ';'.join(row['gaps']),';'.join(row['conflicts'])))
    result=dict(stats,filesChanged=len(changed),applied=apply)
    (out/'insertion_summary.json').write_bytes(encode(result))
    return result

def selftest():
    with tempfile.TemporaryDirectory() as td:
        root=Path(td)
        docs=root/'docs/competition_research'
        docs.mkdir(parents=True)
        out=root/'out'
        out.mkdir()
        row={'row':1,'world':'9697','id':'x','country':'ENG','detail':'div1','verified':{'keep':True}}
        (docs/'sheet.json').write_bytes(encode([row]))
        (docs/'INDEX.json').write_bytes(encode({'rowPath':{'1':'sheet.json'}}))
        (docs/'COVERAGE.json').write_bytes(encode({'prior':7}))
        p=dict(row,historyObservations=[{'season':'1996/97'}],july1Candidate={'scores':[['1996-06-01','A','B',1,0,'x']]})
        fields=dict.fromkeys(FIELDS,'I')
        fields['name']='V'
        s={'row':1,'world':'9697','name':'Test','status':'I','fields':fields}
        (out/'consolidation_packets.json').write_bytes(encode({'packets':[p]}))
        (out/'mass_closure.json').write_bytes(encode({'summary':{'indexedBaseline':True},'rows':[s]}))
        assert materialize(root,out,False,1)['filesChanged']==4
        assert read(docs/'sheet.json')[0]==row
        assert materialize(root,out,True,1)['applied']
        saved=read(docs/'sheet.json')[0]
        assert saved['verified']==row['verified'] and saved['automaticResearch']['fields']['name']=='V'
        assert len(saved['automaticResearch']['observations'])==1
        assert materialize(root,out,True,1)['filesChanged']==0
        assert read(docs/'COVERAGE.json')['prior']==7
        p['july1Candidate']['scores'][0][0]='1996-07-01'
        (out/'consolidation_packets.json').write_bytes(encode({'packets':[p]}))
        try:
            materialize(root,out,True,1)
        except ValueError:
            pass
        else:
            raise AssertionError('Accepted future score')
        assert read(docs/'sheet.json')[0]==saved
        p['july1Candidate']['scores'][0][0]='1996-06-01'
        p['historyObservations'].append({'season':'1997/98'})
        (out/'consolidation_packets.json').write_bytes(encode({'packets':[p]}))
        assert materialize(root,out,True,1)['applied']
        assert len(read(docs/'sheet.json')[0]['automaticResearch']['observations'])==2
        p['id']='wrong'
        (out/'consolidation_packets.json').write_bytes(encode({'packets':[p]}))
        try:
            materialize(root,out,True,1)
        except ValueError:
            pass
        else:
            raise AssertionError('Accepted wrong identity')
