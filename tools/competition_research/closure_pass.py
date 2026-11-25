"""Research-only whole-matrix consolidation. No game/CE writes."""
import collections as C
import csv
import json
import re
import tempfile
from pathlib import Path
import unicodedata

FIELDS=('name','identity','participants','format','calendar','regulation','movement','qualification','july1','evolution')
STATES=('V','P','I','NA','?')
COUNTRY=dict(x.split(':') for x in 'albania:ALB algeria:ALG andorra:AND argentina:ARG armenia:ARM austria:AUT azerbaijan:AZE belarus:BLR belgium:BEL bolivia:BOL bosnia-herzegovina:BIH brazil:BRA bulgaria:BUL chile:CHI china-pr:CHN colombia:COL costa-rica:CRC croatia:CRO cyprus:CYP czech-republic:CZE denmark:DEN ecuador:ECU egypt:EGY el-salvador:SLV england:ENG estonia:EST faroe-islands:FRO finland:FIN france:FRA georgia:GEO germany:GER ghana:GHA gibraltar:GIB greece:GRE guatemala:GUA honduras:HON hungary:HUN iceland:ISL iran:IRN ireland-republic:IRL israel:ISR italy:ITA japan:JPN kazakhstan:KAZ korea-republic:KOR kosovo:KOS latvia:LVA lithuania:LTU luxembourg:LUX macedonia-fyr:MKD malta:MLT mexico:MEX moldova:MDA montenegro:MNE morocco:MAR netherlands:NED northern-ireland:NIR norway:NOR panama:PAN paraguay:PAR peru:PER poland:POL portugal:POR qatar:QAT romania:ROU russia:RUS san-marino:SMR saudi-arabia:KSA scotland:SCO serbia:SRB slovakia:SVK slovenia:SVN south-africa:RSA spain:ESP surinam:SUR sweden:SWE switzerland:SUI tunisia:TUN turkey:TUR ukraine:UKR united-arab-emirates:UAE united-states:USA uruguay:URU venezuela:VEN wales:WAL'.split())
MATCH=dict(x.split(':') for x in 'B1:BEL1 D1:GER1 D2:GER2 E0:ENG1 E1:ENG2 E2:ENG3 E3:ENG4 F1:FRA1 F2:FRA2 I1:ITA1 I2:ITA2 N1:NED1 P1:POR1 SC0:SCO1 SC1:SCO2 SC2:SCO3 SC3:SCO4 SP1:ESP1 SP2:ESP2 T1:TUR1 G1:GRE1 ARG:ARG1 AUT:AUT1 BRA:BRA1 CHN:CHN1 DEN:DEN1 FIN:FIN1 IRL:IRL1 JAP:JPN1 MEX:MEX1 NOR:NOR1 POL:POL1 ROM:ROU1 RUS:RUS1 SUI:SUI1 SWE:SWE1 USA:USA1'.split())
INT={'UEFA CL':(165,525),'UEFA EL':(90,526),'UEFA CONF L':(527,),'Copa Lib':(178,321,423),'Copa Sud':(424,),'Recopa':(425,),'CONCACAF CC':(426,),'AFC CL':(427,),'AFC Pokal':(428,),'AFC Pres. Cup':(429,),'CAF CL':(430,),'CAF CC':(431,),'CAF SC':(432,),'OFC CC':(433,)}

def norm(s):
    return re.sub('[^a-z0-9]','',unicodedata.normalize('NFKD',str(s)).encode('ascii','ignore').decode().lower())

# Exact source titles already reviewed in block165, never generic stage headings.
OPEN={}
for c,d,ts in [('ALG',1,'Algeria | Ligue 1'),('ARG',1,'Argentina Primera Division'),('AUT',1,'Austria | Bundesliga;Österr. Bundesliga'),('AUT',2,'Österr. 2. Liga;Österr. Erste Liga'),('BEL',1,'Belgium First Division A'),('BRA',1,'Brasileiro Série A'),('BRA',2,'Brasileiro Série B'),('CHN',1,'China | Super League'),('COL',1,'Colombia Primera A'),('CZE',1,'Czech Republic First League'),('EGY',1,'Egypt | Premiership'),('ENG',1,'England | Premier League;English Premier League'),('ENG',2,'English Championship'),('ENG',3,'English League One'),('ENG',4,'English League Two'),('FRA',1,'French Ligue 1'),('FRA',2,'French Ligue 2'),('GER',1,'Deutsche Bundesliga;Germany | Bundesliga'),('GER',2,'Deutsche 2. Bundesliga'),('GER',3,'Deutsche 3. Liga'),('GRE',1,'Greek Super League'),('HUN',1,'Hungarian Nemzeti Bajnokság I'),('ITA',1,'Italian Serie A'),('ITA',2,'Italian Serie B'),('JPN',1,'Japan | J1 League'),('MEX',1,'Liga MX'),('USA',1,'Major League Soccer'),('MAR',1,'Morocco | Botola Pro 1'),('NED',1,'Netherlands Eredivisie'),('POR',1,'Portuguese Primeira Liga'),('ESP',1,'Primera División de España;Spain Primera División'),('ESP',2,'Segunda División de España;Spain Segunda División'),('RUS',1,'Russia Premier League'),('SCO',1,'Scottish Premiership'),('SUI',1,'Swiss Super League'),('TUR',1,'Turkish Süper Lig')]:
    for t in ts.split(';'):OPEN[norm(t)]=(c,d)
CUPS={}
for ids,ts in [((112,324),'Albanian Cup;Kupa e Shqipërisë'),((113,330),'ÖFB Cup;Austrian Cup'),((114,334),'Belarusian Cup'),((115,332),'Belgian Cup'),((116,337),'Bulgarian Cup'),((117,344),'Croatian Cup'),((118,345),'Cypriot Cup'),((119,346),'Czech Cup;MOL Cup;Raab Karcher Cup'),((120,347),'Danish Cup'),((121,354),'Estonian Cup'),((122,358),'Faroe Islands Cup;Løgmanssteypið'),((123,355),'Finnish Cup;Suomen Cup'),((124,363),'Greek Cup;Kypello Elladas'),((125,364),'Hungarian Cup;Magyar Kupa'),((126,368),'Mjólkurbikarinn;Mjólkurbikar karla'),((127,366),'FAI Cup;Ireland Cup'),((128,370),'Israel State Cup'),((129,380),'Latvian Cup;Latvijas kauss'),((130,378),'Lithuanian Cup;LFF Taurė'),((131,379),'Coupe de Luxembourg'),((132,384),'Maltese FA Trophy'),((133,387),'Irish Cup'),((134,389),'Norwegian Cup;Norgesmesterskapet'),((135,393),'Polish Cup;Puchar Polski'),((136,394),'Taça de Portugal'),((137,398),'Romanian Cup;Cupa României'),((138,401),'Russian Cup;Kubok Rossii'),((139,405),'Coppa Titano'),((140,409),'Slovak Cup'),((141,410),'Slovenian Cup'),((142,411),'Svenska Cupen'),((143,407),'Swiss Cup'),((144,414),'Türkiye Kupası;Turkish Cup'),((145,417),'Ukrainian Cup'),((146,421),'Welsh Cup;FAW Welsh Cup'),((148,325),"Coupe d'Algérie"),((155,357),'Coupe de France;Coupe de la France'),((156,360),'DFB Pokal;DFB-Pokal'),((157,322),'Coppa Italia'),((158,402),'Scottish Cup'),((159,403),'Scottish League Cup'),((160,353),'Copa del Rey'),((161,386),'KNVB Beker;Dutch Cup'),((163,371),'J.League Cup;J.League Yamazaki Nabisco Cup'),((164,372),"Emperor's Cup"),((327,),'Copa Argentina'),((336,),'Copa do Brasil'),((338,),'Canadian Championship'),((339,),'Copa Chile'),((341,),'Chinese FA Cup'),((342,),'Copa Colombia'),((348,),'Copa Ecuador'),((349,),'Egypt Cup'),((351,),'FA Cup'),((352,),'EFL Cup'),((356,),'Liigacup'),((359,),'David Kipiani Cup;Davit Kipiani Cup'),((367,),'Hazfi Cup;Jaam Hazfi'),((373,),'Kazakhstan Cup'),((377,),'Liechtenstein Cup'),((381,),'Coupe du Trône'),((383,),'Macedonian Cup;Kup na Makedonija'),((385,),'Montenegrin Cup'),((388,),'Northern Ireland League Cup'),((390,),'Chatham Cup'),((391,),'Copa Paraguay'),((395,),'Taça da Liga'),((396,),'Amir Cup;Prince Cup'),((413,),'Coupe de Tunisie'),((418,),'Copa AUF Uruguay'),((419,),'U.S. Open Cup;US Open Cup'),((420,),'Copa Venezuela'),((422,),'Welsh League Cup'),((89,),"UEFA Cup Winners' Cup;Cup Winners' Cup"),((90,526),'UEFA Cup;UEFA Europa League'),((165,525),'UEFA Champions League;Champions Cup'),((527,),'UEFA Conference League'),((178,321,423),'Copa Libertadores'),((424,),'Copa Sudamericana'),((425,),'Recopa Sudamericana'),((430,),'CAF Champions League'),((431,),'CAF Confederation Cup'),((432,),'CAF Super Cup'),((433,),'OFC Champions League')]:
    for t in ts.split(';'):CUPS[norm(t)]=ids

def route(g,w,matrix,tiers):
    src,t=g['source'],g['competition'];c=None;d=1
    if src.startswith('parquet/'):
        if t in INT:return {r for r in INT[t] if r in matrix and matrix[r]['world']==w}
        c=COUNTRY.get(t)
    elif src.startswith(('matches/','github/xgabora/')) and t in MATCH:
        code=MATCH[t];c,d=code[:3],int(code[3:])
    else:
        country=g.get('country','')
        title=re.sub(r'\s+(?:19|20)\d{2}(?:[-/](?:\d{2}|\d{4}))?$', '',t)
        label=norm(title)
        if src=='github/openfootball/football.json' and label in OPEN:
            c,d=OPEN[label]
            if country and country!=c:return set()
        elif label in CUPS:
            if label==norm('FA Cup') and country!='ENG':return None
            if label in {norm('Prince Cup'),norm("Cup Winners' Cup"),norm('Champions Cup'),norm('Irish Cup')} and not country:return None
            return {r for r in CUPS[label] if r in matrix and matrix[r]['world']==w and (not country or country=='INTERNATIONAL' or matrix[r]['country']==country)}
    return None if c is None else set(tiers.get((w,c,d),()))

def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,x):p.write_text(json.dumps(x,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def reviewed_aliases(root,matrix,native,aliases):
    if not root:return 0
    p=root/'docs/competition_research/closure/ALIASES_168.json'
    if not p.exists():return 0
    index=C.defaultdict(set)
    for k,v in native.items():index[k[0],k[1],v['league'],v['name']].add(k)
    n=0
    for rid,items in read(p)['aliases'].items():
        rid=int(rid);m=matrix.get(rid)
        if not m or m['world']!='2627' or not re.fullmatch(r'div\d+',m['detail']):continue
        for label,name in items.items():
            hits=index[m['world'],m['country'],int(m['detail'][3:])-1,name]
            if len(hits)==1:aliases[rid,norm(label)].update(hits);n+=1
    return n

def exact_edition(g,w):
    year=1996 if w=='9697' else 2026
    s=g['season'].replace('-','/').replace('–','/')
    if g['season_basis']=='SOURCE_SEASON_END_YEAR':return s==str(year+1)
    return g['season_basis'] in ('SOURCE_TITLE','SOURCE_SEASON') and s in (f'{year}/{(year+1)%100:02d}',f'{year}/{year+1}')

def roster(p,native):
    g=p.get('sourceGroup',{});m=p.get('identityCandidates',{});d=p['detail'];s=p.get('shape',{})
    if p['world']!='2627' or not re.fullmatch(r'div\d+',d) or not g.get('source','').startswith('euro2627/') or g.get('season')!='2026/27' or not p.get('fullUniqueIdentityCandidate'):return False
    if s.get('invalid') or s.get('clubDayClashes') or s.get('inferredDates'):return False
    want={k[2:] for k,v in native.items() if k[:2]==(p['world'],p['country']) and v['league']==int(d[3:])-1}
    got={(v['uuid'],v['file'],v['ordinal']) for v in m.values()}
    return bool(got) and got==want and len(got)==len(m)==s.get('teams') and s.get('orientedPairs')==s.get('fixtures')

def merge_regular(options,m,native,expected,cut,end,audit,complete):
    """Merge exact-edition explicit dates only. A conflicting oriented pair blocks the union."""
    if len(expected)!=1 or not re.fullmatch(r'div\d+',m['detail']):return None
    n=next(iter(expected));pairs={};labels={};origins=[];sources=set()
    for _,g,fs,stats,mapping,missing,full,ready in options:
        if not full or not exact_edition(g,m['world']) or stats['invalid'] or stats['inferredDates'] or stats['clubDayClashes'] or stats['orientedPairs']!=stats['fixtures']:continue
        if any(not cut<=d<end for d,*_ in fs):continue
        if m['world']=='2627' and any(native[k]['league']!=int(m['detail'][3:])-1 for k in mapping.values()):continue
        origins.append([g['source'],g['competition'],g['season']]);sources.add(g['source'])
        for d,b,h,a,hg,ag,loc,stage in fs:
            hk,ak=mapping[h],mapping[a];pair=(hk,ak)
            if hk==ak:return None
            if pair in pairs and pairs[pair][0]!=d:return None
            if pair not in pairs:pairs[pair]=(d,b,hk,ak,None,None,g['source']+':'+loc,stage)
            labels[hk]=native[hk]['name'];labels[ak]=native[ak]['name']
    if len(origins)<2 or len(labels)!=n or len(pairs)!=n*(n-1) or len(set(labels.values()))!=n:return None
    if m['world']=='2627':
        want={k for k,v in native.items() if k[:2]==(m['world'],m['country']) and v['league']==int(m['detail'][3:])-1}
        if set(labels)!=want:return None
    fs=sorted((d,b,labels[h],labels[a],hg,ag,loc,s) for d,b,h,a,hg,ag,loc,s in pairs.values())
    _,stats=audit(fs,True)
    if not complete(stats,expected,fs[-1][0],end):return None
    stats['mergedSourceGroups']=origins;stats['expectedRegularTeams']=sorted(expected)
    g={'source':'consolidated/exact-initial','competition':m['name'],'country':m['country'],'season':'1996/97' if m['world']=='9697' else '2026/27','season_basis':'SOURCE_SEASON','min_date':fs[0][0],'max_date':fs[-1][0]}
    return ((1,1,1,n,0,len(fs)),g,fs,stats,{v:k for k,v in labels.items()},[],True,True)

def history(p):
    """Observed complete double RR only; compare consecutive editions within a source/season basis."""
    groups=C.defaultdict(set)
    for g in p.get('historyObservations',[]):
        if str(g.get('double_round_robin_candidate'))!='1' or int(g.get('inferred_dates',1)) or int(g.get('duplicate_pair_occurrences',1)):continue
        n=int(g['teams']);s=g['season'];b=g['season_basis']
        if n<2 or int(g['rows'])!=n*(n-1) or int(g['unique_oriented_pairs'])!=n*(n-1):continue
        if not re.fullmatch(r'(?:19|20)\d{2}(?:[-/](?:\d{2}|\d{4}))?',s):continue
        y=int(s[:4]);kind='SPLIT' if len(s)>4 else b
        if b not in ('SOURCE_SEASON','SOURCE_TITLE','SOURCE_SEASON_END_YEAR'):continue
        groups[g['source'],kind,y].add(n)
    changes=[]
    for (src,kind,y),sizes in sorted(groups.items()):
        old=groups.get((src,kind,y-1),set())
        if len(old)==len(sizes)==1 and old!=sizes:changes.append({'source':src,'seasonBasis':kind,'year':y,'previousTeams':next(iter(old)),'teams':next(iter(sizes))})
    return {'completeObservedEditions':sum(len(v)==1 for v in groups.values()),'conflictingEditionCounts':sum(len(v)>1 for v in groups.values()),'observedChanges':changes}

def counts(rows):return {k:{s:sum(r['fields'][k]==s for r in rows) for s in STATES} for k in FIELDS}

def assess(packets,native,root,out):
    baseline={};meta={};coverage={};ix={};family={};indexed=False
    if root:
        docs=root/'docs/competition_research';ix=read(docs/'INDEX.json');coverage=read(docs/'COVERAGE.json')
        keys=[k for k in ix if re.fullmatch(r'fieldStatusMatrix\d+',k)]
        if keys:
            m=read(docs/ix[max(keys,key=lambda k:int(k[len('fieldStatusMatrix'):]))]);baseline={r['row']:{'status':r['status'],'fields':dict(zip(m['columns'],r['fields']))} for r in m['rows']};indexed=True
        if ix.get('automaticInsertion'):
            current=read(docs/ix['automaticInsertion'])
            records=current['rows'];current_baseline={r['row']:{'status':r['status'],'fields':dict(r['fields']),'conflicts':list(r.get('conflicts',[]))} for r in records}
            if len(records)!=len(current_baseline) or set(current_baseline)!=set(baseline):raise ValueError('Current insertion matrix mismatch')
            for rid,old in baseline.items():
                for field,value in old['fields'].items():
                    if value=='V' and current_baseline[rid]['fields'].get(field)!='V':raise ValueError('Verified field regressed')
            baseline=current_baseline
        cache={}
        for p in packets:
            path=ix.get('rowPath',{}).get(str(p['row']))
            if path:
                if path not in cache:cache[path]={x['row']:x for x in read(docs/path)}
                meta[p['row']]=cache[path][p['row']]
        for r in ix.get('approvedFamilyFieldRules',[]):
            for rid in r['rows']:family.setdefault(rid,[]).append(r)
    scope=[p for p in packets if p.get('phaseQueue')!='OUT_OF_SCOPE' and p['row'] not in (100,101,102)]
    if indexed and (set(baseline)!={p['row'] for p in scope} or len(baseline)!=len(scope) or any(set(r['fields'])!=set(FIELDS) or not set(r['fields'].values())<=set(STATES) for r in baseline.values())):raise ValueError('Field matrix scope mismatch')
    key_counts=C.Counter((p['world'],p['country'],p['id'],p['detail']) for p in scope)
    name_counts=C.Counter((p['world'],p['country'],norm(meta.get(p['row'],{}).get('competitionNaming',{}).get('officialName') or p['name'])) for p in scope)
    before=[];after=[];changes=[];new_closed=[]
    for p in scope:
        rid=p['row'];old=baseline.get(rid,{'status':'I','fields':dict.fromkeys(FIELDS,'I')});f=dict(old['fields']);before.append({'row':rid,'fields':dict(f),'status':old['status']})
        def promote(k,basis):
            if f[k]=='I':f[k]='P';changes.append({'row':rid,'field':k,'basis':basis})
        row=meta.get(rid,{});nm=row.get('competitionNaming',{});edition='1996/97' if p['world']=='9697' else '2026/27'
        # Current-edition name is separate from historical naming and from competition identity.
        if nm.get('officialName') and nm.get('startEdition')==edition and nm.get('status')=='SOURCE_SUPPORTED_NAME_PARTIAL' and (not p['name'] or norm(p['name'])==norm(nm['officialName'])):
            promote('name','EXPLICIT_SOURCE_SUPPORTED_INITIAL_EDITION_NAME')
            category=p.get('phaseCategory') or row.get('researchCategory') or row.get('technicalCategory')
            primary=(category=='LEAGUE_DIVISION' and re.fullmatch(r'div\d+',p['detail'])) or (category in ('NATIONAL_CUP','LEAGUE_CUP','CONTINENTAL_CLUB','SUPERCUP_OR_INTERCONTINENTAL','REGIONAL_STATE_LEAGUE') and p.get('phaseQueue')!='PARENT_PHASE_LINK')
            if primary and p['id'] and key_counts[p['world'],p['country'],p['id'],p['detail']]==1 and name_counts[p['world'],p['country'],norm(nm['officialName'])]==1:
                promote('identity','UNIQUE_SCOPED_PRIMARY_INVENTORY_AND_CURRENT_NAME')
        deep_conflict=bool(p.get('deepEvidence',{}).get('conflicts') or p.get('deepCrossRowConflict') or row.get('automaticResearch',{}).get('conflicts') or old.get('conflicts'))
        if p.get('fullNativeRosterCandidate') and (not p.get('deepRosterBasis') or not deep_conflict):promote('participants','SAME_EDITION_FULL_NATIVE_ROSTER_BIJECTION')
        if f['july1']=='?' and not deep_conflict:
            y=1996 if p['world']=='9697' else 2026
            active=[e for e in p.get('deepEvidence',{}).get('editions',[]) if e['edition'] in (str(y),f'{y}/{y+1}') and e.get('activeAtStartCandidate')]
            if len(active)==1:f['july1']='I';changes.append({'row':rid,'field':'july1','basis':'EXPLICIT_MATCHES_STRADDLE_START_APPLICABLE_NOT_COMPLETE'})
        if p.get('completeRegularStructureCandidate') and p.get('fullUniqueIdentityCandidate') and not p.get('crossRowConflict') and not deep_conflict and exact_edition(p.get('sourceGroup',{}),p['world']) and f['july1']=='NA':
            # Whole-edition promotion requires a reviewed full phase plan or already complete format.
            plan=p.get('phasePlan') or {};ph=plan.get('phases',[])
            if len(ph)==1 and ph[0].get('type')=='ROUND_ROBIN' and type(ph[0].get('teams')) is int and type(ph[0].get('meetingsPerPair')) is int and p['shape']['teams']==ph[0]['teams'] and p['shape']['fixtures']==ph[0]['teams']*(ph[0]['teams']-1)*ph[0]['meetingsPerPair']//2:
                if plan.get('completionScope')=='FULL_EDITION':promote('format','REVIEWED_FULL_PHASE_AND_EXACT_PAIR_SET')
                if f['format'] in ('V','P'):promote('calendar','REVIEWED_FULL_PHASE_AND_EXACT_PAIR_SET')
        for r in family.get(rid,[]):
            if r.get('world')==p['world'] and r.get('edition')==edition and r.get('id')==p['id'] and r.get('country')==p['country'] and p['detail'] in r.get('details',[]) and r.get('field') in ('regulation','movement','qualification','format') and r.get('completeForField') is True and r.get('reference') and (docs/r['reference']).is_file():
                promote(r['field'],'REVIEWED_FAMILY:'+r['reference'])
        gaps=[k for k,v in f.items() if v not in ('V','P','NA')]
        status='I' if not indexed or gaps or p.get('crossRowConflict') or deep_conflict else ('P' if 'P' in f.values() else 'V')
        if old['status']=='I' and status!='I':new_closed.append(rid)
        after.append({'row':rid,'world':p['world'],'name':nm.get('officialName') or nm.get('nonCommercialDisplayName') or nm.get('candidateName') or p['name'],'status':status,'fields':f,'gaps':gaps,'manualGaps':sorted(set(gaps)-set(p.get('candidateReviewFields',[]))),'history':history(p),'groupCount':p['sourceGroupCount'],'regularComponentCompleteCandidate':bool(p.get('completeRegularStructureCandidate')),'fullIdentityCandidate':bool(p.get('fullUniqueIdentityCandidate'))})
    b,a=counts(before),counts(after);closed=lambda rs:sum(r['status'] in ('P','V') for r in rs)
    stats={'denominator':len(scope),'indexedBaseline':indexed,'fieldsBefore':b,'fieldsAfter':a,'deltaComplete':{k:a[k]['V']+a[k]['P']-b[k]['V']-b[k]['P'] for k in FIELDS},'closedBefore':closed(before),'closedAfter':closed(after),'newProvisionalClosureRows':new_closed,'upgrades':changes,'existingPartialEvidence':{'names':coverage.get('namesSourceSupportedPartial'),'regulation':coverage.get('rulesPartial'),'evolution':coverage.get('lifecyclePartial'),'actualCalendar':coverage.get('nineCategoryDashboard',{}).get('initialCalendar',{}).get('reconciledRows'),'previousCumulativeLinkedRows':coverage.get('closureConsolidation',{}).get('block165',{}).get('linkedRows')},'observedHistoricalChanges':sum(len(r['history']['observedChanges']) for r in after),'rowsWithObservedHistoricalChanges':sum(bool(r['history']['observedChanges']) for r in after),'scope':'V/P/I, NA and unknown separate. Observed changes are not permanent rule certification. No source-completeness field.'}
    write(out/'mass_closure.json',{'summary':stats,'rows':after})
    with (out/'mass_closure_queue.csv').open('w',encoding='utf-8-sig',newline='') as f:
        wr=csv.writer(f);wr.writerow(('row','world','name','status','gapCount','gaps','manualGapCount','manualGaps'))
        for r in sorted(after,key=lambda r:(r['status']!='I',len(r['manualGaps']),len(r['gaps']),-r['groupCount'],r['row'])):wr.writerow((r['row'],r['world'],r['name'],r['status'],len(r['gaps']),';'.join(r['gaps']),len(r['manualGaps']),';'.join(r['manualGaps'])))
    return stats

def selftest():
    assert route({'source':'parquet/games.parquet','competition':'portugal'},'2627',{}, {('2627','POR',1):{286}})=={286}
    assert route({'source':'parquet/games.parquet','competition':'india'},'2627',{}, {}) is None
    assert route({'source':'matches/Matches.txt','competition':'E2'},'2627',{}, {('2627','ENG',3):{233}})=={233}
    assert route({'source':'parquet/games.parquet','competition':'Copa Lib'},'2627',{321:{'world':'2627'},423:{'world':'2627'}},{})=={321,423}
    g={'source':'a','season':'1996/97','season_basis':'SOURCE_TITLE','teams':4,'rows':12,'unique_oriented_pairs':12,'double_round_robin_candidate':1,'inferred_dates':0,'duplicate_pair_occurrences':0}
    h=dict(g,season='1997/98',teams=6,rows=30,unique_oriented_pairs=30)
    assert len(history({'historyObservations':[g,h]})['observedChanges'])==1
    assert not history({'historyObservations':[g,dict(h,inferred_dates=1)]})['observedChanges']
    assert not history({'historyObservations':[g,dict(h,season='1998/99')]})['observedChanges']
    assert exact_edition(g,'9697') and not exact_edition(dict(g,season='1997',season_basis='CALENDAR_YEAR_BUCKET'),'9697')


def gate_selftest(audit,complete):
    """Positive and negative merge, family, status and closure tests; no real data modified."""
    k1=('2627','ENG','a',1,1);k2=('2627','ENG','b',1,2)
    native={k1:{'name':'A','league':0},k2:{'name':'B','league':0}}
    row={'world':'2627','country':'ENG','id':'L','detail':'div1','name':'Test League'}
    mapping={'A':k1,'B':k2}
    g={'source':'first','competition':'Test League','season':'2026/27','season_basis':'SOURCE_SEASON'}
    def option(g,fs):return ((0,),g,fs,audit(fs,True)[1],mapping,[],True,False)
    f1=('2026-08-01','EXPLICIT','A','B',1,0,'f1','')
    f2=('2026-08-08','EXPLICIT','B','A',1,0,'f2','')
    o1=option(g,[f1]);o2=option(dict(g,source='second'),[f2])
    def go(opts):return merge_regular(opts,row,native,{2},'2026-07-01','2027-07-01',audit,complete)
    v=go([o1,o2]);assert v and len(v[2])==2 and all(f[4:6]==(None,None) for f in v[2])
    assert not go([o1,option(dict(g,source='second'),[f2,('2026-08-02',*f1[1:])])])
    assert not go([o1,option(dict(g,source='second',season='2025/26'),[f2])])
    assert not go([o1,option(dict(g,source='second'),[(f2[0],'INFERRED',*f2[2:])])])
    assert not go([o1,option(dict(g,source='second'),[(f1[0],*f2[1:])])])
    assert route({'source':'corpus/corpus.zip','competition':'FA Cup','country':'FRA'},'2627',{}, {}) is None
    assert route({'source':'corpus/corpus.zip','competition':'Cup','country':'ENG'},'2627',{}, {}) is None
    with tempfile.TemporaryDirectory() as d:
        root=Path(d);docs=root/'docs/competition_research';docs.mkdir(parents=True);out=root/'out';out.mkdir()
        rows=[];pack=[];meta=[]
        for rid in range(1,5):
            w='9697' if rid==2 else '2627';name=f'League {rid}';f=dict.fromkeys(FIELDS,'P')
            f.update(name='I',identity='I',regulation='I',july1='NA')
            if rid==3:f['july1']='?'
            if rid==4:f['name']='V'
            rows.append({'row':rid,'status':'I','fields':[f[k] for k in FIELDS]})
            pack.append({'row':rid,'world':w,'country':'ENG','id':f'L{rid}','detail':'div1','name':name,'sourceGroupCount':0,'crossRowConflict':rid==4})
            meta.append({'row':rid,'technicalCategory':'LEAGUE_DIVISION','competitionNaming':{'officialName':name,'startEdition':'1996/97' if rid==2 else '2026/27','status':'SOURCE_SUPPORTED_NAME_PARTIAL'}})
        ix={'fieldStatusMatrix168':'base.json','rowPath':{str(i):'meta.json' for i in range(1,5)},'approvedFamilyFieldRules':[{'rows':[i],'world':'2627','edition':'2026/27','country':'ENG','id':f'L{i}','details':['div1'],'field':'regulation','completeForField':True,'reference':'rule.json'} for i in range(1,5)]}
        for n,x in [('INDEX.json',ix),('COVERAGE.json',{'namesSourceSupportedPartial':412,'rulesPartial':499,'lifecyclePartial':210}),('base.json',{'columns':FIELDS,'rows':rows}),('meta.json',meta),('rule.json',{})]:write(docs/n,x)
        a=assess(pack,{},root,out)
        assert a['newProvisionalClosureRows']==[1] and a['closedAfter']==1 and a['fieldsAfter']['name']['V']==1 and a['existingPartialEvidence']['names']==412
