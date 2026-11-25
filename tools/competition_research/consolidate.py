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
import closure_pass as mass
import deep_pass as deep

FIELDS = ('name','identity','participants','format','calendar','regulation','movement','qualification','july1','evolution')
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

def expected_regular_teams(row, world):
    edition = '1996/97' if world == '9697' else '2026/27'
    years = {edition, edition[:4], edition.replace('/', '-')}
    counts = set()
    for key, obj in row.items():
        if not isinstance(obj, dict) or str(obj.get('edition', '')) not in years:
            continue
        if key == 'historicalSportData':
            n = obj.get('teams')
        elif key.startswith(('block', 'sportRuleEvidence')):
            n = obj.get('rules', {}).get('teams')
        else:
            continue
        if type(n) is int and n > 1:
            counts.add(n)
    return counts

def phase_catalogue(root, index, matrix):
    if not root or not index.get('phaseModelCatalogue'):
        return {}, {}
    data = read(root/'docs/competition_research'/index['phaseModelCatalogue'])
    if set(data['phaseTypes']) != {'ROUND_ROBIN', 'KNOCKOUT'}:
        raise ValueError('Unsupported phase type')
    rows = {int(x['row']): x for x in data['rows']}
    if set(rows) != set(matrix) or len(rows) != len(data['rows']):
        raise ValueError('Phase catalogue does not match raw matrix')
    plans = data['plans']
    for rid, row in rows.items():
        if row.get('plan') is not None:
            if int(row['plan']) != rid or any(p['type'] not in data['phaseTypes'] for p in plans[str(rid)]['phases']):
                raise ValueError('Invalid phase plan for row '+str(rid))
    return rows, plans

def complete_regular(stats, expected, last_date, end):
    family = stats['family']
    regular = family in ('DOUBLE_ROUND_ROBIN','SINGLE_ROUND_ROBIN') or family.startswith('ORIENTED_PAIR_REPEAT_')
    # A complete pair set of a subset is not a full league; fail closed if expected size is unknown/conflicting.
    return (regular and expected == {stats['teams']} and stats['fixtures'] > 0
            and not stats['invalid'] and not stats['clubDayClashes']
            and not stats['inferredDates'] and last_date < end)

def aliases_from_repo(root, matrix, native):
    """Read only explicitly indexed review overlays; validate every UUID/file/ordinal against native."""
    aliases=C.defaultdict(set);states={};warnings=[];expected={};routes={};plans={}
    if not root:return aliases,states,warnings,expected,routes,plans
    docs=root/'docs/competition_research'
    try:index=read(docs/'INDEX.json')
    except (OSError,ValueError) as e:return aliases,states,[str(e)],expected,routes,plans
    routes,plans=phase_catalogue(root,index,matrix)
    cache={}
    slot_index=C.defaultdict(set)
    for k in native:slot_index[k[0],k[2],k[3],k[4]].add(k)
    def load(p):
        if p not in cache:cache[p]=read(docs/p)
        return cache[p]
    def add(rid,label,uid,file,ordinal):
        if rid not in matrix or not isinstance(label,str):return
        m=matrix[rid];k=(m['world'],m['country'],str(uid),int(file),int(ordinal))
        if k in native:aliases[rid,norm(label)].add(k)
        elif not m['country'] and routes.get(rid,{}).get('category') in ('CONTINENTAL_CLUB','SUPERCUP_OR_INTERCONTINENTAL'):
            hits=slot_index[m['world'],str(uid),int(file),int(ordinal)]
            if len(hits)==1:aliases[rid,norm(label)].update(hits)
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
                if re.fullmatch(r'div\d+',m['detail']):expected[rid]=expected_regular_teams(r,m['world'])
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
    if index.get('automaticInsertion'):
        current=load(index['automaticInsertion'])
        records=current['rows']+current.get('excludedRows',[])
        canonical={int(r['row']):r for r in records}
        if len(canonical)!=len(records) or set(canonical)-set(matrix) or (index.get('canonicalFieldMatrix') and set(canonical)!=set(matrix)):
            raise ValueError('Canonical matrix inventory mismatch')
        for rid,r in canonical.items():
            f=r['fields']
            if set(f)!=set(FIELDS) or not set(f.values())<={'V','P','I','NA','?'}:
                raise ValueError('Invalid canonical field status')
            states[rid]=dict(states.get(rid,{}),**r)
            states[rid]['closed']=r['status'] in ('V','P')
    return aliases,states,warnings,expected,routes,plans

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
            hits={i for i in declared if matrix[i]['world']==w and (not country or matrix[i]['country']==country or (country=='INTERNATIONAL' and not matrix[i]['country']))}
            if g['source']=='github/openfootball/football.json' and title in OPEN:
                c,t=OPEN[title]
                if not country or c==country:hits.update(tiers[w,c,t])
            if country:hits.update(names[w,country,norm(title)])
            extra=mass.route(g,w,matrix,tiers)
            if extra is not None:
                # Known codebook jurisdictions; any conflict stays unresolved.
                if hits and extra and hits!=extra:ambiguous+=1;continue
                if not extra:continue
                hits=extra
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
        archived_tier_sizes=C.Counter((k[0],k[1],v['league']) for k,v in native.items())
        native,stale_native=current_native_filter(root,native)
        native_tiers=C.defaultdict(set)
        for k,v in native.items():native_tiers[k[0],k[1],v['league']].add(k)
        world_names=C.defaultdict(set);world_slots=C.defaultdict(set)
        for k,v in native.items():
            native_names[k[0],k[1],norm(v['name'])].add(k)
            world_names[k[0],norm(v['name'])].add(k)
            world_slots[k[0],k[2],k[3],k[4]].add(k)
        reviewed,states,warnings,expected,routes,plans=aliases_from_repo(root,matrix,native)
        international={rid for rid,m in matrix.items() if not m['country'] and routes.get(rid,{}).get('category') in ('CONTINENTAL_CLUB','SUPERCUP_OR_INTERCONTINENTAL')}
        for j in records(source/FILES[3]):
            if j['status']!='UNIQUE_CANDIDATE' or not j['file'] or not j['ordinal']:continue
            rid=int(j['row']);m=matrix.get(rid)
            if not m or j['world']!=m['world']:continue
            k=(m['world'],m['country'],j['uuid'],int(j['file']),int(j['ordinal']))
            if k in native:archive_alias[rid,key(j),norm(j['label'])].add(k)
            elif rid in international:
                hits=world_slots[m['world'],j['uuid'],int(j['file']),int(j['ordinal'])]
                if len(hits)==1:archive_alias[rid,key(j),norm(j['label'])].update(hits)
        alias_count=mass.reviewed_aliases(root,matrix,native,reviewed)
        all_rules=deep.rule_index(db,byrow,matrix,tiers,names,root)
        packets=[];queue=[];collisions=C.defaultdict(list);all_count=0;families=C.defaultdict(list);merged_count=0
        for rid,m in sorted(matrix.items()):
            w=m['world'];year=1996 if w=='9697' else 2026;cut=f'{year}-07-01';end=f'{year+1}-07-01'
            history=byrow[rid];target=[g for g in history if initial(g,w)];all_count+=len(target)
            league=bool(re.fullmatch(r'div\d+',m['detail']));options=[];rosters=[]
            tierkey=(w,m['country'],int(m['detail'][3:])-1) if league else None
            native_tier=native_tiers.get(tierkey,set())
            roster_complete=bool(native_tier) and len(native_tier)==archived_tier_sizes.get(tierkey)
            for g in target:
                k=key(g)
                fs=db.execute('SELECT date,date_basis,home,away,hg,ag,locator,stage FROM fixture WHERE source=? AND competition=? AND country=? AND season=? AND season_basis=? ORDER BY date,home,away,locator',k).fetchall()
                labels,stats=audit_fixtures(fs,league);mapping={};missing=[]
                for label in sorted(labels):
                    choices=reviewed[rid,norm(label)]|archive_alias[rid,k,norm(label)]|native_names[w,m['country'],norm(label)]
                    if rid in international:choices=choices | world_names[w,norm(label)]
                    if w=='2627' and league:choices=choices & native_tier
                    if len(choices)==1:mapping[label]=next(iter(choices))
                    else:missing.append({'label':label,'candidateCount':len(choices)})
                distinct=len(set(mapping.values()))==len(mapping);full=bool(labels) and not missing and distinct
                stats['expectedRegularTeams']=sorted(expected.get(rid,set()))
                complete=complete_regular(stats,expected.get(rid,set()),g['max_date'],end)
                rank=(int(complete and full),int(complete),int(full),len(mapping),-stats['inferredDates'],len(fs))
                options.append((rank,g,fs,stats,mapping,missing,full,complete))
                if roster_complete and w=='2627' and league and g['source'].startswith('euro2627/') and mass.exact_edition(g,w) and full and set(mapping.values())==native_tier and not stats['invalid'] and not stats['clubDayClashes'] and not stats['inferredDates'] and stats['orientedPairs']==stats['fixtures']:
                    rosters.append({'sourceGroup':dict(zip(('source','competition','country','season','season_basis'),key(g))),'identityCandidates':{l:native[k] for l,k in mapping.items()}})
            # Candidate club spelling variants require two actual same-edition opponent/date witnesses.
            pool=native_tier if league and w=='2627' else {k for k in native if k[0]==w and (k[1]==m['country'] or rid in international)}
            options,alias_witnesses=deep.reconcile(options,native,pool)
            raw_options=list(options)
            # A union can only fill missing pairs from the same explicit edition; any date conflict blocks it.
            if league and options and not any(o[6] and o[7] for o in options):
                merged=mass.merge_regular(options,m,native,expected.get(rid,set()),cut,end,audit_fixtures,complete_regular)
                if merged:options.append(merged);merged_count+=1
            # Prefer coherent complete candidates.
            options.sort(key=lambda x:x[0],reverse=True)
            current=states.get(rid,{})
            def normalize_field(f):
                v=current.get('fields',{}).get(f,m.get('name_status' if f=='name' else f,'I'))
                if isinstance(v,dict):v=v.get('status','I')
                return {'COMPLETE':'P','PARTIAL':'I','NOT_AUDITED':'I'}.get(v,v) if v in ('V','P','I','NA','?','COMPLETE','PARTIAL','NOT_AUDITED') else 'I'
            status={f:normalize_field(f) for f in FIELDS}
            gaps=[f for f in FIELDS if status[f] not in ('V','P','NA')]
            closed=current.get('closed',m['previously_closed'].lower()=='true')
            packet={'row':rid,'world':w,'country':m['country'],'id':m['id'],'detail':m['detail'],'name':m['name'],'existingFields':status,'existingClosed':closed,'sourceGroupCount':len(history),'initialGroupCount':len(target),'historyObservations':[{x:g[x] for x in ('source','competition','season','season_basis','teams','rows','unique_oriented_pairs','double_round_robin_candidate','inferred_dates','duplicate_pair_occurrences','min_date','max_date')} for g in history],'reviewGaps':gaps,'status':'REVIEW_REQUIRED'}
            packet['fullNativeRosterCandidate']=bool(rosters)
            if rosters:packet['nativeRosterCandidate']=rosters[0]
            packet['phaseCategory']=routes.get(rid,{}).get('category')
            packet['phaseQueue']=routes.get(rid,{}).get('queue','UNCLASSIFIED')
            packet['phaseModel']=routes.get(rid,{}).get('model')
            packet['phasePlan']=plans.get(str(rid))
            evidence=deep.combine(raw_options,w,packet['phasePlan'],all_rules.get(rid,[]),native,expected.get(rid,set()))
            evidence['corroboratedAliasCandidates']=alias_witnesses
            if evidence['editions'] or evidence['unresolvedEditionGroups'] or evidence['ruleAndTableEvidence'] or alias_witnesses:packet['deepEvidence']=evidence
            # Full roster only from a complete reviewed RR, explicit exact start edition, and live native tier bijection.
            wanted=f'{year}/{year+1}'
            for e in evidence['editions']:
                if league and roster_complete and e['edition']==wanted and e['completeReviewedRegularComponent'] and not e['conflicts'] and set(e['nativeRoster'])=={k[2] for k in native_tier} and len({native[k]['name'] for k in native_tier})==len(native_tier):
                    packet['fullNativeRosterCandidate']=True
                    packet['deepRosterBasis']='COMPLETE_REVIEWED_EXACT_EDITION_NATIVE_TIER_BIJECTION'
                    packet['nativeRosterCandidate']={'edition':wanted,'identityCandidates':{native[k]['name']:native[k] for k in native_tier}}
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
    # Check exact-edition joined events across rows as candidates, including cups; exclude shared parent phases.
    deep_days=C.defaultdict(set)
    for p in packets:
        if p.get('phaseQueue') in ('PARENT_PHASE_LINK','OUT_OF_SCOPE'):continue
        for ed in p.get('deepEvidence',{}).get('editions',[]):
            for e in ed['events']:
                if 'EXPLICIT' not in e['dateBases'] or not e['homeUUID'] or not e['awayUUID']:continue
                for u in (e['homeUUID'],e['awayUUID']):deep_days[p['world'],e['date'],u].add(p['row'])
    deep_cross=[{'world':w,'date':d,'uuid':u,'rows':sorted(rs)} for (w,d,u),rs in sorted(deep_days.items()) if len(rs)>1]
    deep_blocked={r for c in deep_cross for r in c['rows']}
    for p in packets:
        p['crossRowConflict']=p['row'] in blocked
        if p['row'] in deep_blocked:p['deepCrossRowConflict']=True
        if p['row'] in blocked or p['row'] in deep_blocked:p['status']='REVIEW_CROSS_ROW_CONFLICT'
        available=set()
        if p.get('fullUniqueIdentityCandidate') and p.get('completeRegularStructureCandidate'):available.add('participants')
        if p.get('completeRegularStructureCandidate'):available.update(('format','calendar'))
        p['candidateReviewFields']=sorted(available & set(p['reviewGaps']))
        p['manualGaps']=sorted(set(p['reviewGaps'])-available)
    queue=[(x[0],x[1],x[2],x[3],p['status'],x[5],len(p['manualGaps']),';'.join(p['manualGaps']),x[8],x[9],x[10]) for x,p in zip(queue,packets)]
    # Preserve prior verified status. The separate mass closure pass applies documented gates only.
    queue.sort(key=lambda x:(routes.get(x[0],{}).get('queue')=='OUT_OF_SCOPE',x[5],x[4]!='REGULAR_PACKET_READY_FOR_REVIEW',x[6],routes.get(x[0],{}).get('queue','UNCLASSIFIED'),-x[8],x[0]))
    queue=[(*x,routes.get(x[0],{}).get('queue','UNCLASSIFIED'),routes.get(x[0],{}).get('model')) for x in queue]
    with (out/'consolidation_queue.csv').open('w',encoding='utf-8-sig',newline='') as f:
        wr=csv.writer(f);wr.writerow(('row','world','country','name','candidateStatus','previouslyClosed','manualGapCount','manualGaps','initialGroups','family','unresolvedLabels','phaseQueue','phaseModel'));wr.writerows(queue)
    write(out/'consolidation_packets.json',{'scope':'REVIEW_ONLY: candidate identities, schedules, history, rule excerpts; not OG/CE runtime data or verified closure','packets':packets,'crossRowCandidateCollisions':cross,'deepCrossRowCandidateCollisions':deep_cross,'formatFamilies':dict(families),'warnings':warnings})
    summary={'matrixRows':len(matrix),'rowsWithGroups':sum(bool(byrow[r]) for r in matrix),'recoveredGroupAssociations':recovered,'ambiguousGroupAssociationsSkipped':ambiguous,'initialGroupsReviewed':all_count,'rowsWithInitialGroups':sum(bool(p['initialGroupCount']) for p in packets),'completeRegularStructureCandidates':sum(bool(p.get('completeRegularStructureCandidate')) for p in packets),'fullUniqueIdentityCandidates':sum(bool(p.get('fullUniqueIdentityCandidate')) for p in packets),'regularPacketsReadyForReview':sum(p['status']=='REGULAR_PACKET_READY_FOR_REVIEW' for p in packets),'crossRowCandidateCollisions':len(cross),'candidateFieldReviewTasks':sum(len(p['candidateReviewFields']) for p in packets),'formatFamilies':{k:len(v) for k,v in families.items()},'newVerifiedClosures':0,'newProvisionalClosures':0,'repoWarnings':len(warnings),'nativeRecordsRejectedAgainstCurrentRepo':stale_native,'currentRepoChecked':bool(root)}
    summary['phaseRouting']=dict(C.Counter(p['phaseQueue'] for p in packets))
    summary['phasePlans']=sum(p['phasePlan'] is not None for p in packets)
    summary['regularSizeGate']='expected same-edition sport count required; unknown/conflict/inferred dates not ready'
    ds=[p['deepEvidence'] for p in packets if 'deepEvidence' in p and p.get('phaseQueue')!='OUT_OF_SCOPE' and p['row'] not in (100,101,102)]
    summary['deepExtraction']={'rowsWithEvidence':len(ds),'ruleAndTableObservations':sum(len(d['ruleAndTableEvidence']) for d in ds),'exactEditionEvents':sum(len(e['events']) for d in ds for e in d['editions']),'unresolvedEditionGroups':sum(len(d['unresolvedEditionGroups']) for d in ds),'corroboratedAliasCandidates':sum(len(d['corroboratedAliasCandidates']) for d in ds),'july1ArithmeticCandidates':sum(bool(e['july1Arithmetic']) for d in ds for e in d['editions']),'internalConflicts':sum(len(d['conflicts']) for d in ds),'crossRowCandidateCollisions':len(deep_cross)}
    result=mass.assess(packets,native,root,out)
    summary['massClosure']=result
    summary['newProvisionalClosures']=len(result['newProvisionalClosureRows'])
    summary['reviewedAliasMappingsAccepted']=alias_count
    summary['mergedRegularCandidates']=merged_count
    summary['closurePass']='Whole matrix; candidate evidence distinct from provisional and verified completion'
    write(out/'consolidation_summary.json',summary);return summary

def selftest():
    deep.selftest()
    mass.selftest()
    mass.gate_selftest(audit_fixtures,complete_regular)
    f=[('1996-08-01','EXPLICIT','A','B',1,0,'1',''),('1996-08-02','EXPLICIT','B','A',0,0,'2','')]
    assert audit_fixtures(f,True)[1]['family']=='DOUBLE_ROUND_ROBIN'
    assert audit_fixtures(f[:1],True)[1]['family']=='SINGLE_ROUND_ROBIN'
    assert audit_fixtures(f,False)[1]['family']=='OTHER_PHASES_OR_PARTIAL'
    assert audit_fixtures(f+[f[0]],True)[1]['clubDayClashes']==2
    shape=audit_fixtures(f,True)[1]
    assert complete_regular(shape,{2},'1996-08-02','1997-07-01')
    assert not complete_regular(shape,{16},'1996-08-02','1997-07-01')
    assert not complete_regular(shape,set(),'1996-08-02','1997-07-01')
    assert not complete_regular(shape,{2,16},'1996-08-02','1997-07-01')
    assert not complete_regular(dict(shape,inferredDates=1),{2},'1996-08-02','1997-07-01')
    assert expected_regular_teams({'block102Research':{'edition':'1996/97','rules':{'teams':16}},'block103Research':{'edition':'2026/27','rules':{'teams':20}}},'9697')=={16}
    assert initial({'season':'1996/97','season_basis':'SOURCE_TITLE','min_date':'1996-08-01','max_date':'1997-05-01'},'9697')
    assert not initial({'season':'1996/97','season_basis':'SOURCE_TITLE','min_date':'1996-08-01','max_date':'1997-05-01'},'2627')

if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('source',type=Path);ap.add_argument('out',type=Path);ap.add_argument('--repo-root',type=Path)
    a=ap.parse_args();selftest();print(json.dumps(consolidate(a.source,a.out,a.repo_root),indent=2))
