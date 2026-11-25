"""Read-only candidate consolidation. Never writes game data or certifies sporting rules."""
import argparse
import collections as C
from contextlib import closing
import csv
import datetime as dt
import json
from pathlib import Path
import re
import sqlite3
import tempfile
import unicodedata
import zipfile

FIELDS = ('identity','participants','format','calendar','regulation','movement','qualification','july1','evolution')
FILES = ('evidence.sqlite','matrix_candidates.csv','source_groups.csv','native_join_candidates.csv')
# Exact titles in the supplied OpenFootball JSON archive; no fuzzy competition joins.
OPEN = {'English Premier League':('ENG',1),'English Championship':('ENG',2),
        'Deutsche Bundesliga':('GER',1),'French Ligue 1':('FRA',1),
        'Italian Serie A':('ITA',1),'Netherlands Eredivisie':('NED',1),
        'Portuguese Primeira Liga':('POR',1),'Spain Primera División':('ESP',1),
        'Brasileiro Série A':('BRA',1)}
YEAR = re.compile(r'\s+((?:19|20)\d{2})(?:[-/](?:\d{2}|\d{4}))?$')

def norm(s):
    return re.sub('[^a-z0-9]','',unicodedata.normalize('NFKD',str(s)).encode('ascii','ignore').decode().lower())

def records(p):
    with p.open(encoding='utf-8-sig',newline='') as f:
        yield from csv.DictReader(f)

def read(p):
    return json.loads(p.read_text(encoding='utf-8-sig'))

def write(p,x):
    p.write_text(json.dumps(x,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def key(g):
    return tuple(g[x] for x in ('source','competition','country','season','season_basis'))

def initial(g,w):
    year=1996 if w=='9697' else 2026
    start=f'{year}-07-01';end=f'{year+1}-07-01'
    s=g['season'].replace('-','/').replace('–','/')
    if not (g['max_date']>=start and g['min_date']<end):return False
    if g['season_basis']=='SOURCE_SEASON_END_YEAR':return s==str(year+1)
    return s in (str(year),f'{year}/{(year+1)%100:02d}',f'{year}/{year+1}',str(year+1))

def audit_fixtures(rows, league):
    labels=set();pairs=C.Counter();days=C.Counter();invalid=0;inferred=0
    for date,basis,h,a,hg,ag,loc,stage in rows:
        try:dt.date.fromisoformat(date)
        except (ValueError,TypeError):invalid+=1
        if not h or not a or h==a:invalid+=1
        labels.update((h,a));pairs[h,a]+=1;days[date,h]+=1;days[date,a]+=1
        inferred+=basis!='EXPLICIT'
    n=len(labels);clashes=sum(v>1 for v in days.values());family='OTHER_PHASES_OR_PARTIAL'
    if league and n>1 and all(h!=a for h,a in pairs):
        if len(pairs)==n*(n-1) and len(set(pairs.values()))==1:
            legs=next(iter(pairs.values()));family='DOUBLE_ROUND_ROBIN' if legs==1 else f'ORIENTED_PAIR_REPEAT_{legs}'
        elif len(pairs)==n*(n-1)//2 and all(v==1 for v in pairs.values()) and len({frozenset(p) for p in pairs})==len(pairs):
            family='SINGLE_ROUND_ROBIN'
    return labels,{'family':family,'teams':n,'fixtures':len(rows),'orientedPairs':len(pairs),'invalid':invalid,'clubDayClashes':clashes,'inferredDates':inferred}

def aliases_from_repo(root, matrix, native):
    """Read only explicitly indexed review overlays; validate every UUID/file/ordinal against native."""
    aliases=C.defaultdict(set);states={};warnings=[]
    if not root:return aliases,states,warnings
    docs=root/'docs/competition_research'
    try:index=read(docs/'INDEX.json')
    except (OSError,ValueError) as e:return aliases,states,[str(e)]
    cache={}
    def load(p):
        if p not in cache:cache[p]=read(docs/p)
        return cache[p]
    def add(rid,label,uid,file,ordinal):
        if rid not in matrix or not isinstance(label,str):return
        m=matrix[rid];k=(m['world'],m['country'],str(uid),int(file),int(ordinal))
        if k in native:aliases[rid,norm(label)].add(k)
    for rid,m in matrix.items():
        try:
            p=index.get('namedClosureRowPath',{}).get(str(rid))
            if p:
                r=next(x for x in load(p)['rows'] if int(x['row'])==rid)
                states[rid]=r
            p=index['rowPath'].get(str(rid))
            candidates=[]
            if p:
                r=next(x for x in load(p) if int(x['row'])==rid)
                if r.get('participantKeyAudit'):candidates.append(r['participantKeyAudit'])
            for k in ('closureRowPath','closureAdditionalRowPath'):
                p=index.get(k,{}).get(str(rid))
                if p:
                    candidates.extend(x for x in load(p).get('rows',[]) if int(x['row'])==rid)
            for obj in candidates:
                for x in obj.get('mapping',[]):
                    if isinstance(x,dict) and all(k in x for k in ('sourceLabel','uuid','file','ordinal')):
                        add(rid,x['sourceLabel'],x['uuid'],x['file'],x['ordinal'])
                    elif isinstance(x,list) and obj.get('columns',[])[:5]==['sportName','nativeName','uuid','fileCountry','ordinal']:
                        add(rid,x[0],x[2],x[3],x[4])
        except (OSError,ValueError,KeyError,TypeError,StopIteration,IndexError) as e:warnings.append(f'row {rid}: {e}')
    return aliases,states,warnings

def current_native_filter(root,native):
    if not root:return native,0
    cache={};valid={}
    for k,v in native.items():
        path=v['path'].replace('\\','/')
        if path not in cache:
            try:
                j=read(root/path);items={}
                for e in j['teams']:
                    t=e['team'];q=(str(e.get('uuid')),int(j['country']),int(t['team_ordinal']))
                    items[q]=(t.get('team_name'),t.get('league')) if q not in items else None
                cache[path]=items
            except (OSError,ValueError,KeyError,TypeError):cache[path]={}
        if cache[path].get(k[2:])==(v['name'],v['league']):valid[k]=v
    return valid,len(native)-len(valid)

def consolidate(source,out,root=None):
    if source.resolve()==out.resolve():raise ValueError('Output must be separate from input')
    out.mkdir(parents=True,exist_ok=True)
    if source.is_file():
        with tempfile.TemporaryDirectory() as td:
            d=Path(td)
            with zipfile.ZipFile(source) as z:
                for name in FILES:
                    if z.namelist().count(name)!=1:raise ValueError('Missing/duplicate '+name)
                    z.extract(name,d)
            return consolidate(d,out,root)
    matrix={int(r['row']):r for r in records(source/FILES[1])}
    groups=list(records(source/FILES[2]));byrow=C.defaultdict(list);tiers=C.defaultdict(set);names=C.defaultdict(set)
    for rid,m in matrix.items():
        if re.fullmatch(r'div\d+',m['detail']):tiers[m['world'],m['country'],int(m['detail'][3:])].add(rid)
        if m['name']:names[m['world'],m['country'],norm(m['name'])].add(rid)
    recovered=0;ambiguous=0
    for g in groups:
        title=YEAR.sub('',g['competition']);country=g['country']
        declared={int(x) for x in g['candidate_matrix_rows'].split(';') if x.isdigit() and int(x) in matrix}
        for w in ('9697','2627'):
            hits={i for i in declared if matrix[i]['world']==w and (not country or matrix[i]['country']==country)}
            if g['source']=='github/openfootball/football.json' and title in OPEN:
                c,t=OPEN[title]
                if not country or c==country:hits.update(tiers[w,c,t])
            if country:hits.update(names[w,country,norm(title)])
            if len(hits)>1:ambiguous+=1;continue
            if hits:
                rid=next(iter(hits));byrow[rid].append(g)
                if rid not in declared:recovered+=1
    native={};native_names=C.defaultdict(set);ambiguous_keys=set();archive_alias=C.defaultdict(set)
    with closing(sqlite3.connect((source/FILES[0]).resolve().as_uri()+'?mode=ro',uri=True)) as db:
        for w,c,n,u,f,o,l,p in db.execute('SELECT world,country,name,uuid,file,ordinal,league,path FROM native'):
            if Path(p.replace('\\','/')).stem.upper().endswith(('_POOL','_2')):continue
            k=(w,c,u,f,o);value={'uuid':u,'file':f,'ordinal':o,'league':l,'name':n,'path':p}
            if k in native and native[k]!=value:ambiguous_keys.add(k)
            native[k]=value
        for k in ambiguous_keys:native.pop(k,None)
        uuid_counts=C.Counter((k[0],k[2]) for k in native);slot_counts=C.Counter((k[0],k[3],k[4]) for k in native)
        native={k:v for k,v in native.items() if uuid_counts[k[0],k[2]]==slot_counts[k[0],k[3],k[4]]==1}
        native,stale_native=current_native_filter(root,native)
        for k,v in native.items():native_names[k[0],k[1],norm(v['name'])].add(k)
        for j in records(source/FILES[3]):
            if j['status']!='UNIQUE_CANDIDATE' or not j['file'] or not j['ordinal']:continue
            rid=int(j['row']);m=matrix.get(rid)
            if not m or j['world']!=m['world']:continue
            k=(m['world'],m['country'],j['uuid'],int(j['file']),int(j['ordinal']))
            if k in native:archive_alias[rid,key(j),norm(j['label'])].add(k)
        reviewed,states,warnings=aliases_from_repo(root,matrix,native)
        packets=[];queue=[];collisions=C.defaultdict(list);all_count=0;families=C.defaultdict(list)
        for rid,m in sorted(matrix.items()):
            w=m['world'];year=1996 if w=='9697' else 2026;cut=f'{year}-07-01';end=f'{year+1}-07-01'
            history=byrow[rid];target=[g for g in history if initial(g,w)];all_count+=len(target)
            league=bool(re.fullmatch(r'div\d+',m['detail']));options=[]
            for g in target:
                k=key(g)
                fs=db.execute('SELECT date,date_basis,home,away,hg,ag,locator,stage FROM fixture WHERE source=? AND competition=? AND country=? AND season=? AND season_basis=? ORDER BY date,home,away,locator',k).fetchall()
                labels,stats=audit_fixtures(fs,league);mapping={};missing=[]
                for label in sorted(labels):
                    choices=reviewed[rid,norm(label)]|archive_alias[rid,k,norm(label)]|native_names[w,m['country'],norm(label)]
                    if len(choices)==1:mapping[label]=next(iter(choices))
                    else:missing.append({'label':label,'candidateCount':len(choices)})
                distinct=len(set(mapping.values()))==len(mapping);full=bool(labels) and not missing and distinct
                regular=stats['family'] in ('DOUBLE_ROUND_ROBIN','SINGLE_ROUND_ROBIN') or stats['family'].startswith('ORIENTED_PAIR_REPEAT_')
                complete=regular and not stats['invalid'] and not stats['clubDayClashes'] and bool(fs) and g['max_date']<end
                rank=(int(complete and full),int(complete),int(full),len(mapping),-stats['inferredDates'],len(fs))
                options.append((rank,g,fs,stats,mapping,missing,full,complete))
            # Prefer coherent complete candidates, never union incompatible calendars.
            options.sort(key=lambda x:x[0],reverse=True)
            current=states.get(rid,{});status={f:current.get('fields',{}).get(f,{}).get('status',m[f]) for f in FIELDS}
            gaps=[f for f in FIELDS if status[f] not in ('COMPLETE','NA')]
            closed=current.get('closed',m['previously_closed'].lower()=='true')
            packet={'row':rid,'world':w,'country':m['country'],'id':m['id'],'detail':m['detail'],'name':m['name'],'existingFields':status,'existingClosed':closed,'sourceGroupCount':len(history),'initialGroupCount':len(target),'historyObservations':[{x:g[x] for x in ('source','competition','season','season_basis','teams','rows','unique_oriented_pairs','double_round_robin_candidate')} for g in history],'reviewGaps':gaps,'status':'REVIEW_REQUIRED'}
            if options:
                _,g,fs,stats,mapping,missing,full,complete=options[0];packet.update({'sourceGroup':dict(zip(('source','competition','country','season','season_basis'),key(g))),'shape':stats,'fullUniqueIdentityCandidate':full,'completeRegularStructureCandidate':complete,'unmappedOrAmbiguous':missing,'identityCandidates':{l:native[k] for l,k in mapping.items()},'alternatives':[{'sourceGroup':list(key(o[1])),'shape':o[3],'mapped':len(o[4])} for o in options[1:]],'fixtureColumns':['date','dateBasis','homeLabel','awayLabel','homeUUID','awayUUID','locator','stage'],'fixtures':[[d,b,h,a,mapping[h][2] if h in mapping else None,mapping[a][2] if a in mapping else None,loc,s] for d,b,h,a,hg,ag,loc,s in fs]})
                # Snapshot evidence only: no inferred ranking or post-cutoff score.
                prior=[(d,b,h,a,hg,ag,loc,s) for d,b,h,a,hg,ag,loc,s in fs if d<cut]
                packet['july1Candidate']={'preCutoffOccurrences':len(prior),'status':'REVIEW_REQUIRED' if prior else 'NO_PRIOR_MATCH_IN_SELECTED_GROUP_NOT_PROOF_OF_NA','scores':[[d,h,a,hg,ag,loc] for d,b,h,a,hg,ag,loc,s in prior if b=='EXPLICIT' and hg is not None and ag is not None and not g['source'].startswith('parquet/')]}
                if full and complete:
                    packet['status']='REGULAR_PACKET_READY_FOR_REVIEW'
                    families[stats['family']].append(rid)
                    for d,b,h,a,hg,ag,loc,s in fs:
                        if cut<=d<end:
                            for l in (h,a):collisions[w,d,mapping[l][2]].append(rid)
                packet['ruleExcerpts']=[dict(zip(('source','locator','excerpt'),x)) for x in db.execute('SELECT source,locator,excerpt FROM rule WHERE competition=? AND country=? AND season=? LIMIT 30',(g['competition'],g['country'],g['season']))]
            packets.append(packet)
            queue.append((rid,w,m['country'],m['name'],packet['status'],int(bool(closed)),len(gaps),';'.join(gaps),len(target),packet.get('shape',{}).get('family',''),len(packet.get('unmappedOrAmbiguous',[]))))
        cross=[{'world':w,'date':d,'uuid':u,'rows':sorted(set(rs))} for (w,d,u),rs in collisions.items() if len(set(rs))>1]
    blocked={r for c in cross for r in c['rows']}
    for p in packets:
        p['crossRowConflict']=p['row'] in blocked
        if p['row'] in blocked:p['status']='REVIEW_CROSS_ROW_CONFLICT'
        available=set()
        if p.get('fullUniqueIdentityCandidate'):available.add('participants')
        if p.get('completeRegularStructureCandidate'):available.update(('format','calendar'))
        p['candidateReviewFields']=sorted(available & set(p['reviewGaps']))
        p['manualGaps']=sorted(set(p['reviewGaps'])-available)
    queue=[(x[0],x[1],x[2],x[3],p['status'],x[5],len(p['manualGaps']),';'.join(p['manualGaps']),x[8],x[9],x[10]) for x,p in zip(queue,packets)]
    # Existing verified status is never upgraded automatically. Queue ready packets and smallest gaps first.
    queue.sort(key=lambda x:(x[5],x[4]!='REGULAR_PACKET_READY_FOR_REVIEW',x[6],-x[8],x[0]))
    with (out/'consolidation_queue.csv').open('w',encoding='utf-8-sig',newline='') as f:
        wr=csv.writer(f);wr.writerow(('row','world','country','name','candidateStatus','previouslyClosed','manualGapCount','manualGaps','initialGroups','family','unresolvedLabels'));wr.writerows(queue)
    write(out/'consolidation_packets.json',{'scope':'REVIEW_ONLY: candidate identities, schedules, history, rule excerpts; not OG/CE runtime data or verified closure','packets':packets,'crossRowCandidateCollisions':cross,'formatFamilies':dict(families),'warnings':warnings})
    summary={'matrixRows':len(matrix),'rowsWithGroups':sum(bool(byrow[r]) for r in matrix),'recoveredGroupAssociations':recovered,'ambiguousGroupAssociationsSkipped':ambiguous,'initialGroupsReviewed':all_count,'rowsWithInitialGroups':sum(bool(p['initialGroupCount']) for p in packets),'completeRegularStructureCandidates':sum(bool(p.get('completeRegularStructureCandidate')) for p in packets),'fullUniqueIdentityCandidates':sum(bool(p.get('fullUniqueIdentityCandidate')) for p in packets),'regularPacketsReadyForReview':sum(p['status']=='REGULAR_PACKET_READY_FOR_REVIEW' for p in packets),'crossRowCandidateCollisions':len(cross),'candidateFieldReviewTasks':sum(len(p['candidateReviewFields']) for p in packets),'formatFamilies':{k:len(v) for k,v in families.items()},'newVerifiedClosures':0,'newProvisionalClosures':0,'repoWarnings':len(warnings),'nativeRecordsRejectedAgainstCurrentRepo':stale_native,'currentRepoChecked':bool(root)}
    write(out/'consolidation_summary.json',summary);return summary

def selftest():
    f=[('1996-08-01','EXPLICIT','A','B',1,0,'1',''),('1996-08-02','EXPLICIT','B','A',0,0,'2','')]
    assert audit_fixtures(f,True)[1]['family']=='DOUBLE_ROUND_ROBIN'
    assert audit_fixtures(f[:1],True)[1]['family']=='SINGLE_ROUND_ROBIN'
    assert audit_fixtures(f,False)[1]['family']=='OTHER_PHASES_OR_PARTIAL'
    assert audit_fixtures(f+[f[0]],True)[1]['clubDayClashes']==2
    assert initial({'season':'1996/97','season_basis':'SOURCE_TITLE','min_date':'1996-08-01','max_date':'1997-05-01'},'9697')
    assert not initial({'season':'1996/97','season_basis':'SOURCE_TITLE','min_date':'1996-08-01','max_date':'1997-05-01'},'2627')

if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('source',type=Path);ap.add_argument('out',type=Path);ap.add_argument('--repo-root',type=Path)
    a=ap.parse_args();selftest();print(json.dumps(consolidate(a.source,a.out,a.repo_root),indent=2))
