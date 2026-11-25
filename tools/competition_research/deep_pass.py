"""Additional conservative archive extraction; research only, no game writes."""
import collections as C
import datetime as dt
import json
import re
import unicodedata
import zipfile

def norm(s):
    return re.sub(r'[^a-z0-9]','',unicodedata.normalize('NFKD',str(s or '')).encode('ascii','ignore').decode().lower())

YEAR=re.compile(r'\b((?:19|20)\d{2})(?:\s*[/–-]\s*(\d{2}|\d{4}))?\b')
LEVEL=re.compile(r'(?:^|\s)(first|second|third|fourth|fifth|sixth|1st|2nd|3rd|4th|5th|6th)\s+(?:level|division|tier)\b',re.I)
LEVELS={x:i+1 for i,x in enumerate(('first','second','third','fourth','fifth','sixth'))}
LEVELS.update({x:i+1 for i,x in enumerate(('1st','2nd','3rd','4th','5th','6th'))})
PHASE=re.compile(r'^(?:round\s+\d+|\d+(?:st|nd|rd|th)\s+round|matchday\s+\d+|group\s+[a-z0-9]+|first phase|second phase|final phase|(?:torneo\s+)?(?:apertura|clausura)|preliminary round|quarter.?finals?|semi.?finals?|finals?|first legs?|second legs?|play.?offs?)\b',re.I)
OUTSIDE=re.compile(r'\b(?:women|women.s|futsal|youth|under[- ]?\d+|u[- ]?1[789]|reserve|regional|amateur|district|friendly|friendlies|pre.season)\b',re.I)
CUP=re.compile(r'\b(?:cup|copa|coupe|pokal|supercup|super cup|taça|taca)\b',re.I)
TABLE=re.compile(r'^\s*(\d{1,3})[.)]?\s*([^\d].*?)\s+(\d{1,3})\s+(\d{1,3})\s+(\d{1,3})\s+(\d{1,3})\s+(\d{1,3})\s*[-:]\s*(\d{1,3})\s+(-?\d{1,3})(?:\s+(.*))?$')
RULES={
 'regulation':r'\b(?:points?\s+(?:for|per)|points?\s+system|points?\s+deduct|tie.?break|goal difference|goal average|head.to.head|away goals?|extra time|penalt(?:y|ies)|replay|regulations?|rules?)\b',
 'movement':r'\b(?:promot\w*|relegat\w*|play.?out|promotion play.?off)\b',
 'qualification':r'\b(?:qualif\w*|berths?|slots?|places? in|eligible|eligibility)\b',
 'format':r'\b(?:round.robin|single leg|two legs|home and away|format|groups? of|play.?offs?|teams? (?:play|participat)|rounds?)\b',
 'evolution':r'\b(?:renamed|replaced|abolished|discontinued|reintroduced|reinstated|expanded|reduced|first edition|last edition|formerly|changed from|suspended|cancelled)\b'
}
RULE_RE={k:re.compile(v,re.I) for k,v in RULES.items()}

def season_key(season,basis):
    s=str(season).replace('–','/').replace('-','/').strip()
    if basis=='SOURCE_SEASON_END_YEAR' and re.fullmatch(r'(?:19|20)\d{2}',s):
        y=int(s);return f'{y-1}/{y}'
    if basis not in ('SOURCE_TITLE','SOURCE_SEASON','EXPLICIT_EDITION'):
        return None
    m=YEAR.fullmatch(s)
    if not m:return None
    y=int(m[1])
    if not m[2]:return str(y)
    end=int(m[2]);end=(y//100)*100+end if len(m[2])==2 else end
    if end<y:end+=100
    return f'{y}/{end}' if end==y+1 else None

def section_aliases(root):
    docs=root/'docs/competition_research'
    read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
    index=read(docs/'INDEX.json');aliases=C.defaultdict(set)
    for path in sorted(set(index['rowPath'].values())):
        for row in read(docs/path):
            c=row.get('country') or 'INTERNATIONAL'
            naming=row.get('competitionNaming') or {}
            detail=row.get('detail') or ''
            tier=re.fullmatch(r'div(\d+)',detail)
            if tier:
                label=f'{c} DIV{tier[1]}'
            elif row.get('researchCategory') in ('NATIONAL_CUP','LEAGUE_CUP','CONTINENTAL_CLUB','SUPERCUP_OR_INTERCONTINENTAL','NATIONAL_TEAM','NATIONAL_TEAM_TOURNAMENT'):
                label=naming.get('officialName') or naming.get('nonCommercialDisplayName')
            else:continue
            if not label:continue
            names=[naming.get(k) for k in ('officialName','nonCommercialDisplayName','candidateName')]
            for h in naming.get('nameHistory',[]):
                if isinstance(h,dict):names.extend(h.get(k) for k in ('name','previousName'))
            for n in names:
                if n:aliases[c,norm(n)].add(label)
    # These titles are explicitly scoped, not universal synonyms.
    for c,n,t in [('ENG','Premiership',1),('ENG','Premier League',1),('ITA','Serie A',1),('ITA','Serie B',2),('JPN','J-League',1)]:
        aliases[c,norm(n)].add(f'{c} DIV{t}')
    return aliases

def section(line,c,aliases):
    clean=line.strip('= \t:').split('‹',1)[0].strip()
    if not clean or len(clean)>100 or OUTSIDE.search(clean):return None
    without_year=YEAR.sub('',clean).strip(' -,:')
    without_year=re.sub(r'\s+(?:results|fixtures|standings)$','',without_year,flags=re.I).strip(' -,:')
    choices=aliases.get((c,norm(without_year)),set())
    if len(choices)==1:return next(iter(choices))
    if len(choices)>1:return None
    # Do not infer a top tier from a country/year title or a cup/futsal title.
    m=LEVEL.search(without_year)
    if m and c!='INTERNATIONAL' and not CUP.search(without_year) and re.fullmatch(r'[\w .-]*?(?:level|division|tier)(?:\s*\([^)]*\))?',without_year,re.I):
        return f'{c} DIV{LEVELS[m[1].lower()]}'
    return None

def date_token(s,season):
    s=re.sub(r'\s+',' ',s.strip('[] ,'))
    s=re.sub(r'\bSept\b','Sep',s,flags=re.I)
    if not re.fullmatch(r'(?:[A-Za-z]{3,9} \d{1,2}(?:,? \d{4})?|\d{1,2} [A-Za-z]{3,9}(?: \d{4})?|\d{4}-\d{2}-\d{2}|\d{1,2}[/.]\d{1,2}[/.]\d{4})',s):return None
    for fmt in ('%d %B %Y','%d %b %Y','%B %d, %Y','%b %d, %Y','%B %d %Y','%b %d %Y','%Y-%m-%d','%d/%m/%Y','%d.%m.%Y'):
        try:return dt.datetime.strptime(s,fmt).date().isoformat(),'EXPLICIT'
        except ValueError:pass
    key=season_key(season,'SOURCE_TITLE')
    if not key:return None
    for fmt in ('%b %d','%B %d','%d %b','%d %B'):
        try:
            d=dt.datetime.strptime(s+' 2000',fmt+' %Y')
            y=int(key[:4])+(int('/' in key and d.month<7))
            return dt.date(y,d.month,d.day).isoformat(),'INFERRED_FROM_EDITION'
        except ValueError:pass
    return None

TEAM=r'[^\d\s<>\[\]\n][^<>\[\]\n]{0,100}?'
MATCHES=[
 re.compile(r'^('+TEAM+r')\s+(\d{1,2})\s*[-:]\s*(\d{1,2})\s+('+TEAM+r')$'),
 re.compile(r'^('+TEAM+r')\s{2,}(\d{1,2})\s+('+TEAM+r')\s{2,}(\d{1,2})(?:\s{2,}.*)?$'),
 re.compile(r'^('+TEAM+r')\s+-\s+('+TEAM+r')\s{2,}(\d{1,2})\s*[-:]\s*(\d{1,2})(?:\s+\d[\d,.]*)?$')
]

def match_line(s):
    annotations=re.findall(r'\[([^\]]+)\]',s)
    base=re.sub(r'\s*\[[^\]]+\]\s*$','',s).strip()
    # Do not read a ranked standings row or narrative as a match.
    if not base or re.match(r'^\d+[.)]?\s',base) or re.search(r'\b(?:aggregate|awarded|abandoned|walkover|w/o|annulled|penalties|pens|aet)\b',base,re.I):
        return None
    for i,pat in enumerate(MATCHES):
        m=pat.fullmatch(base)
        if not m:continue
        h,g,a,v=(m[1],m[2],m[4],m[3]) if i==0 else ((m[1],m[2],m[3],m[4]) if i==1 else (m[1],m[3],m[2],m[4]))
        h,a=h.strip(),a.strip()
        if not norm(h) or norm(h)==norm(a) or len(h)<2 or len(a)<2 or not any(ch.isalpha() for ch in h) or not any(ch.isalpha() for ch in a) or re.search(r'\d\s*[-:]\s*\d|\s+-\s+',h+' '+a):continue
        if any(re.search(r'\b(?:pens|penalties|aet|awarded|abandoned|walkover|annulled)\b',x,re.I) for x in annotations):
            g=v=None
        return h,a,int(g) if g is not None else None,int(v) if v is not None else None,annotations
    return None

def table_row(s,layout=None):
    if layout=='HOME_AWAY':
        m=re.fullmatch(r'\s*(\d{1,3})[.)]?\s*([^\d].*?)\s+((?:-?\d{1,3}\s+){13}-?\d{1,3})(?:\s+(.*))?',s)
        if not m:return None
        rank,team,numbers,tail=m.groups();p,hw,hd,hl,hf,ha,aw,ad,al,af,aa,gd,pd,pts=map(int,numbers.split())
        if p!=hw+hd+hl+aw+ad+al or gd!=hf+af-ha-aa or min(p,hw,hd,hl,hf,ha,aw,ad,al,af,aa)<0:return None
        return dict(rank=int(rank),team=team.strip(),played=p,won=hw+aw,drawn=hd+ad,lost=hl+al,gf=hf+af,ga=ha+aa,points=pts,note=tail or '',publishedPointAdjustment=pd)
    if layout=='NO_DRAW':
        m=re.fullmatch(r'\s*(\d{1,3})[.)]?\s*\(\s*\d+\)\s*([^\d].*?)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s*[-:]\s*(\d+)\s+(-?\d+)(?:\s+(.*))?',s)
        if not m:return None
        rank,team,p,w,l,gf,ga,pts,tail=m.groups();rank,p,w,l,gf,ga,pts=map(int,(rank,p,w,l,gf,ga,pts))
        if p!=w+l:return None
        return dict(rank=rank,team=team.strip(),played=p,won=w,drawn=0,lost=l,gf=gf,ga=ga,points=pts,note=tail or '',publishedNoDrawClassification=True)
    m=TABLE.fullmatch(s)
    if not m:return None
    rank,team,p,w,d,l,gf,ga,pts,tail=m.groups()
    rank,p,w,d,l,gf,ga,pts=map(int,(rank,p,w,d,l,gf,ga,pts))
    if rank<1 or p!=w+d+l:return None
    return dict(rank=rank,team=team.strip(),played=p,won=w,drawn=d,lost=l,gf=gf,ga=ga,points=pts,note=tail or '')

def scan_document(lines,countries,aliases):
    """Yield explicit section observations; never assign an unknown parent to a tier."""
    comp=None;c='';season='';date=None;phase='';round_name='';table=None;parent_phase=''
    for i,raw in enumerate(lines,1):
        s=raw.strip()
        if not s or s.startswith(('<!--','‹')):continue
        if s.startswith('==') and not re.fullmatch(r'=+',s):
            title=s.strip('= ').split('‹',1)[0].strip()
            em=YEAR.search(title)
            if em:season=em[0]
            for label,code in sorted(countries.items(),key=lambda x:-len(x[0])):
                if title.upper()==label or title.upper().startswith(label+' '):
                    c=code;break
            if not c:c='INTERNATIONAL'
            comp=section(title,c,aliases);date=None;phase='';round_name='';table=None;parent_phase=''
            continue
        if not c or not season or not (1995<=int(season[:4])<=2026):continue
        hit=section(s,c,aliases)
        if hit:
            comp=hit;date=None;phase='';round_name='';table=None;parent_phase=''
            continue
        if table and re.fullmatch(r'P\s+W\s+D\s+L\s+F\s+A\s+W\s+D\s+L\s+F\s+A\s+GD\s+PD\s+PTS',s,re.I):
            table['layout']='HOME_AWAY';continue
        if table and re.fullmatch(r'Pos\s+Prev\s+Team\s+Games\s+Won\s+Lst\s+GF-GA\s+Pnt',s,re.I):
            table['layout']='NO_DRAW';continue
        parsed=match_line(s) if comp else None
        tr=table_row(s,table.get('layout') if table else None) if comp else None
        if len(s)<110 and not parsed and not tr and not s.startswith('[') and (OUTSIDE.search(s) or CUP.search(s) or LEVEL.search(s) or re.search(r'\b(?:division|league|liga|championship|serie [a-z0-9]+|level|tier)\b',s,re.I)) and not any(rx.search(s) for rx in RULE_RE.values()) and not PHASE.match(s):
            comp=None;date=None;table=None
        if not comp:continue
        if re.match(r'^(?:final\s+)?(?:table|standings|classification)\b',s,re.I):
            table={'heading':s,'line':i,'date':None}
            for token in re.findall(r'\(([^)]+)\)|\[([^\]]+)\]',s):
                t=date_token(next(x for x in token if x),season)
                if t:table['date']=t
            continue
        if PHASE.match(s) and not parsed:
            label=s.split('[',1)[0].strip(' :')
            if re.match(r'^(?:round\s+\d+|\d+(?:st|nd|rd|th)\s+round|matchday\s+\d+)',label,re.I):
                round_name=label
            elif re.match(r'^(?:torneo\s+)?(?:apertura|clausura)\b',label,re.I):parent_phase=label;phase='';round_name=''
            else:phase=label;round_name=''
            date=next((t for x in re.findall(r'\[([^\]]+)\]',s) if (t:=date_token(x,season))),None);table=None
            continue
        if tr:
            if table is None:table={'heading':'RANKED_ROWS_NO_EXPLICIT_TABLE_HEADING','line':i,'date':None}
            yield ('rule',i,comp,c,season,'TABLE_ROW_JSON '+json.dumps(dict(tr,table=table,phase=' / '.join(x for x in (parent_phase,phase) if x)),ensure_ascii=False,sort_keys=True))
            continue
        match=parsed
        if match:
            h,a,g,v,annotations=match
            actual=next((t for x in annotations if (t:=date_token(x,season))),None) or date
            table=None
            if actual:
                d,b=actual
                if '1996-01-01'<=d<='2027-12-31':
                    if d>='2026-07-01':g=v=None
                    yield ('fixture',i,comp,c,season,(comp,c,season,'SOURCE_TITLE',d,b,h,a,g,v,' / '.join(x for x in (parent_phase,phase,round_name) if x)))
            continue
        tokens=[s]+re.findall(r'\[([^\]]+)\]',s)
        actual=next((t for x in tokens if (t:=date_token(x,season))),None)
        if actual:date=actual;table=None
        if len(s)<=600 and not table_row(s) and any(rx.search(s) for rx in RULE_RE.values()):
            yield ('rule',i,comp,c,season,s)

def collect_corpus(path,source,co,root,countries):
    aliases=section_aliases(root)
    with zipfile.ZipFile(path) as z:
        for m in z.infolist():
            if m.is_dir() or not m.filename.lower().endswith('.txt') or m.file_size>8_000_000 or not re.search(r'/(?:tables[^/]*|ec|ac|sac|oc|cc|interclub|misc)/',m.filename,re.I):continue
            try:
                before=co.counts[source];rules=0
                for typ,line,comp,c,season,payload in scan_document(z.read(m).decode('utf-8',errors='replace').splitlines(),countries,aliases):
                    loc=m.filename+':'+str(line)
                    if typ=='fixture':co.add(source,loc,payload)
                    else:
                        co.rulebuf.append((source,loc,comp,c,season,payload));rules+=1
                        if len(co.rulebuf)>=20000:co.flush()
                if co.counts[source]>before or rules:co.note(source,m.filename,'PARSED_SCOPED_SECTIONS',f'{co.counts[source]-before} fixtures; {rules} rule/table observations')
            except Exception as exc:co.note(source,m.filename,'ERROR_SECTION_PARSE',exc)
    co.flush()

def categorize(excerpt):
    if excerpt.startswith('TABLE_ROW_JSON '):
        return ['table'],json.loads(excerpt[len('TABLE_ROW_JSON '):])
    cats=[k for k,rx in RULE_RE.items() if rx.search(excerpt)]
    values={}
    for field,pattern in [('pointsWin',r'\b([0-9]+)\s+points?\s+(?:for|per)\s+(?:a\s+)?(?:win|victory)\b'),('pointsDraw',r'\b([0-9]+)\s+points?\s+(?:for|per)\s+(?:a\s+)?draw\b'),('pointsLoss',r'\b([0-9]+)\s+points?\s+(?:for|per)\s+(?:a\s+)?(?:loss|defeat)\b'),('promotedCount',r'\b(?:top|first)\s+(\d+)\s+(?:teams|clubs)\s+(?:are\s+|were\s+|will be\s+)?promoted\b'),('relegatedCount',r'\b(?:bottom|last)\s+(\d+)\s+(?:teams|clubs)\s+(?:are\s+|were\s+|will be\s+)?relegated\b')]:
        m=re.search(pattern,excerpt,re.I)
        if m:values[field]=int(m[1])
    # Explicit numerical clauses are candidates, never a complete regulation.
    for field,pattern in [
        ('groupSize',r'\bgroups? of (\d+) (?:teams|clubs)\b'),
        ('groupCount',r'\b(\d+) groups? of \d+ (?:teams|clubs)\b'),
        ('advancePerGroup',r'\b(?:top|first) (\d+) (?:teams|clubs) (?:in|from) each group (?:advance|qualif\w*)\b'),
        ('qualifyingCount',r'\b(?:top|first) (\d+) (?:teams|clubs) qualif\w*\b'),
        ('extraTimeMinutes',r'\b(\d+) minutes? of extra time\b')]:
        m=re.search(pattern,excerpt,re.I)
        if m:values[field]=int(m[1])
    if re.search(r'\bdouble round.robin\b',excerpt,re.I):values['observedFormatMention']='DOUBLE_ROUND_ROBIN'
    elif re.search(r'\bsingle round.robin\b',excerpt,re.I):values['observedFormatMention']='SINGLE_ROUND_ROBIN'
    return cats,values

def repository_rules(root):
    """Reuse exact-edition structured research; original payloads remain in their sheets."""
    if not root:return {}
    docs=root/'docs/competition_research'
    read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
    ix=read(docs/'INDEX.json');out=C.defaultdict(list)
    for path in sorted(set(ix['rowPath'].values())):
        for row in read(docs/path):
            for key,obj in row.items():
                if not isinstance(obj,dict) or not (key=='historicalSportData' or key.startswith(('sportRuleEvidence','block'))):continue
                ed=season_key(obj.get('edition',''),'SOURCE_TITLE')
                if not ed:continue
                fields=obj.get('rules',{}) if key!='historicalSportData' else {k:v for k,v in obj.items() if k in ('teams','regularOrInitialGamesPerClub','pointsWin','pointsDraw','pointsLoss','formatAndMovement')}
                if not isinstance(fields,dict) or not fields:continue
                values={k:v for k,v in fields.items() if v is not None}
                points=values.get('points')
                if isinstance(points,dict) and set(points)=={'win','draw','loss'} and all(type(v) is int for v in points.values()):
                    values.update(pointsWin=points['win'],pointsDraw=points['draw'],pointsLoss=points['loss'])
                # Do not guess shootout systems or the meaning/order of an unlabeled list.
                cats=set()
                for k in values:
                    n=k.lower()
                    if re.search(r'point|tie|penalt|extra.?time|away.?goal|replay',n):cats.add('regulation')
                    if re.search(r'promot|relegat|playout|movement',n):cats.add('movement')
                    if re.search(r'access|qualif|advance|berth|slot',n):cats.add('qualification')
                    if re.search(r'team|round|game|phase|group|leg|format|split|entrants',n):cats.add('format')
                out[int(row['row'])].append({'season':ed,'categories':sorted(cats),'values':values,
                    'source':'repository/research','locator':path+'#'+key,'status':obj.get('status','PARTIAL_RESEARCH_NOT_RUNTIME')})
    return out

def rule_index(db,byrow,matrix,tiers,names,root=None):
    links=C.defaultdict(set)
    for rid,groups in byrow.items():
        for g in groups:
            links[g['source'],g['competition'],g['country'],g['season']].add(rid)
    result=C.defaultdict(dict)
    for source,locator,comp,c,season,excerpt in db.execute('SELECT source,locator,competition,country,season,excerpt FROM rule'):
        hits=set(links.get((source,comp,c,season),()))
        if c:
            for w in ('9697','2627'):
                hits.update(names.get((w,'' if c=='INTERNATIONAL' else c,norm(comp)),()))
                m=re.fullmatch(r'([A-Z]{3}) DIV(\d+)',comp)
                if m and m[1]==c:hits.update(tiers.get((w,c,int(m[2])),()))
        cats,values=categorize(excerpt)
        if not cats:continue
        for w in ('9697','2627'):
            rows={rid for rid in hits if matrix[rid]['world']==w}
            if len(rows)!=1:continue
            rid=next(iter(rows))
            if matrix[rid].get('country') and c!=matrix[rid]['country']:continue
            # Preserve raw wording; same exact excerpt and edition is one observation.
            key=(season,excerpt)
            if key not in result[rid]:
                result[rid][key]={'season':season,'categories':cats,'values':values,'source':source,'locator':locator}
                if cats!=['table']:result[rid][key]['excerpt']=excerpt
    for rid,items in repository_rules(root).items():
        if rid in matrix:
            for item in items:result[rid][('repository',item['locator'])]=item
    return {rid:sorted(v.values(),key=lambda x:(x['season'],x['source'],x['locator'],x.get('excerpt',''))) for rid,v in result.items()}

def club_variant(s):
    words=re.findall(r'[a-z0-9]+',unicodedata.normalize('NFKD',s).encode('ascii','ignore').decode().lower())
    # Only explicit club designators; no edit-distance or city-only fallback.
    while words and words[0] in ('fc','cf','sc','ac','afc','fk','sk','sv','cd','ud','as','ks','nk'):words.pop(0)
    while words and words[-1] in ('fc','cf','sc','ac','afc','fk','sk','sv','cd','ud','as','ks','nk'):words.pop()
    return ''.join(words)

def reconcile(options,native,pool):
    variants=C.defaultdict(set);witness=C.defaultdict(set)
    for k in pool:
        v=club_variant(native[k]['name'])
        if len(v)>=4:variants[v].add(k)
    for _,g,fs,stats,mapping,missing,full,ready in options:
        ed=season_key(g['season'],g['season_basis'])
        if not ed:continue
        for d,b,h,a,*_ in fs:
            if b=='EXPLICIT' and h in mapping and a in mapping:
                witness[ed,d,mapping[a],0].add(mapping[h])
                witness[ed,d,mapping[h],1].add(mapping[a])
    out=[];accepted=[]
    for rank,g,fs,stats,mapping,missing,full,ready in options:
        ed=season_key(g['season'],g['season_basis']);mapping=dict(mapping)
        proposals={}
        for item in missing:
            label=item['label'];hits=variants.get(club_variant(label),set())
            if not ed or item['candidateCount'] or len(hits)!=1:continue
            k=next(iter(hits))
            if k in mapping.values():continue
            opponents=set();contradiction=False
            for d,b,h,a,*_ in fs:
                if b!='EXPLICIT':continue
                other,side=(a,0) if h==label else ((h,1) if a==label else (None,None))
                if other not in mapping:continue
                evidence=witness.get((ed,d,mapping[other],side),set())
                if evidence=={k}:opponents.add(mapping[other])
                elif evidence and k not in evidence:contradiction=True
            if len(opponents)>=2 and not contradiction:proposals[label]=k
        counts=C.Counter(proposals.values())
        for label,k in proposals.items():
            if counts[k]==1:
                mapping[label]=k;accepted.append({'label':label,'uuid':k[2],'edition':ed,'basis':'UNIQUE_SCOPED_VARIANT_PLUS_TWO_DISTINCT_OPPONENT_WITNESSES'})
        missing=[x for x in missing if x['label'] not in mapping]
        full=bool(mapping) and not missing and len(set(mapping.values()))==len(mapping)
        rank=(int(ready and full),int(ready),int(full),len(mapping),-stats['inferredDates'],len(fs))
        out.append((rank,g,fs,stats,mapping,missing,full,ready))
    return out,accepted

def table_summaries(rules):
    tables=C.defaultdict(list)
    for r in rules:
        if r['categories']!=['table']:continue
        v=r['values'];t=v.get('table',{})
        key=(r['source'],r['locator'].rsplit(':',1)[0],r['season'],t.get('line'),v.get('phase',''))
        tables[key].append(v)
    result=[]
    for key,rows in sorted(tables.items(),key=lambda x:str(x[0])):
        ranks=sorted(r['rank'] for r in rows);labels=[norm(r['team']) for r in rows]
        balanced=(len(rows)>1 and ranks==list(range(1,len(rows)+1)) and len(set(labels))==len(labels)
                  and sum(r['won'] for r in rows)==sum(r['lost'] for r in rows)
                  and sum(r['gf'] for r in rows)==sum(r['ga'] for r in rows)
                  and sum(r['drawn'] for r in rows)%2==0)
        result.append({'source':key[0],'document':key[1],'edition':season_key(key[2],'SOURCE_TITLE'),
                       'tableLine':key[3],'phase':key[4],'heading':rows[0]['table']['heading'],'date':rows[0]['table'].get('date'),
                       'rows':len(rows),'balancedRankedTableCandidate':balanced,'teams':sorted(r['team'] for r in rows)})
    changes=[];series=C.defaultdict(list)
    for t in result:
        if t['edition'] and t['balancedRankedTableCandidate'] and re.match(r'^final (?:table|standings|classification)\b',t['heading'],re.I):
            series[t['source'],t['phase']].append(t)
    for key,ts in series.items():
        per=C.defaultdict(list)
        for t in ts:per[t['edition']].append(t)
        for ed,items in sorted(per.items()):
            y=int(ed[:4]);prev=f'{y-1}/{y}' if '/' in ed else str(y-1)
            before=per.get(prev,[])
            if not before or len({tuple(sorted(norm(x) for x in t['teams'])) for t in items})!=1 or len({tuple(sorted(norm(x) for x in t['teams'])) for t in before})!=1:continue
            a,b=before[0],items[0];old={norm(x):x for x in a['teams']};new={norm(x):x for x in b['teams']}
            if old.keys()!=new.keys():
                changes.append({'from':prev,'to':ed,'source':key[0],'phase':key[1],
                                'oldTeamCount':len(old),'newTeamCount':len(new),'newLabels':sorted(new[k] for k in new.keys()-old.keys()),
                                'missingLabels':sorted(old[k] for k in old.keys()-new.keys()),'status':'OBSERVED_MEMBERSHIP_NOT_PROOF_OF_PROMOTION_RELEGATION_OR_RENAME'})
    return result,changes

def phase_summaries(events):
    stages=C.defaultdict(list)
    for e in events:
        for name in e['stages']:
            stages[name].append(e)
    return [{'stage':s,'events':len(es),'explicitEvents':sum('EXPLICIT' in e['dateBases'] for e in es),
             'firstDate':min(e['date'] for e in es),'lastDate':max(e['date'] for e in es),
             'teamLabels':len({u for e in es for u in (e['homeUUID'] or norm(e['home']),e['awayUUID'] or norm(e['away']))})}
            for s,es in sorted(stages.items())]

def combine(options,world,plan,rules,native,expected):
    """All explicit editions stay separate; one event with variants, not invented dates/results."""
    cut='1996-07-01' if world=='9697' else '2026-07-01'
    editions=C.defaultdict(dict);groups=C.defaultdict(list);label_ids=C.defaultdict(set);uncertain=[]
    for _,g,fs,stats,mapping,*_ in options:
        ed=season_key(g['season'],g['season_basis'])
        if ed:
            for label,k in mapping.items():label_ids[ed,norm(label)].add(k[2])
    for _,g,fs,stats,mapping,*_ in options:
        ed=season_key(g['season'],g['season_basis'])
        group=[g['source'],g['competition'],g['country'],g['season'],g['season_basis']]
        if not ed:
            uncertain.append({'group':group,'fixtureColumns':['date','dateBasis','home','away','homeUUID','awayUUID','locator','stage'],'fixtures':[[d,b,h,a,mapping[h][2] if h in mapping else None,mapping[a][2] if a in mapping else None,loc,stage] for d,b,h,a,hg,ag,loc,stage in fs]})
            continue
        gi=len(groups[ed]);groups[ed].append(group)
        for d,b,h,a,hg,ag,loc,stage in fs:
            def uid(label):
                choices=label_ids[ed,norm(label)]
                return next(iter(choices)) if len(choices)==1 else None
            hu,au=uid(h),uid(a)
            hk,ak=hu or 'label:'+norm(h),au or 'label:'+norm(a)
            key=(d,hk,ak)
            event=editions[ed].setdefault(key,{'date':d,'home':h,'away':a,'homeUUID':hu,'awayUUID':au,'dateBases':set(),'stages':set(),'groups':set(),'locations':set(),'preStartScores':set()})
            event['dateBases'].add(b)
            if stage:event['stages'].add(stage)
            event['groups'].add(gi);event['locations'].add((gi,loc))
            if d<cut and b=='EXPLICIT' and hg is not None and ag is not None and not g['source'].startswith('parquet/'):
                event['preStartScores'].add((hg,ag))
    result=[];all_conflicts=[]
    phases=(plan or {}).get('phases',[])
    simple=len(phases)==1 and phases[0].get('type')=='ROUND_ROBIN' and phases[0].get('meetingsPerPair') in (1,2) and not phases[0].get('groups')
    for ed,events in sorted(editions.items()):
        conflicts=[];pairs=C.defaultdict(set);days=C.defaultdict(set)
        for key,e in sorted(events.items()):
            if len(e['preStartScores'])>1:conflicts.append({'kind':'SCORE_CONFLICT','key':key})
            if key[1]==key[2]:conflicts.append({'kind':'SELF_PAIR','key':key})
            if 'EXPLICIT' in e['dateBases'] and e['homeUUID'] and e['awayUUID']:
                pairs[key[1:]].add(e['date'])
                for k in key[1:]:days[e['date'],k].add(key)
            for field in ('dateBases','stages','groups','locations','preStartScores'):e[field]=sorted(e[field])
        for key,ds in days.items():
            if len(ds)>1:conflicts.append({'kind':'SAME_DAY_CLUB_CONFLICT','key':key})
        if simple:
            for key,dates in pairs.items():
                if len(dates)>1:conflicts.append({'kind':'EXPLICIT_PAIR_DATE_CONFLICT','key':key,'dates':sorted(dates)})
        for (edition,label),ids in label_ids.items():
            if edition==ed and len(ids)>1:conflicts.append({'kind':'LABEL_ID_CONFLICT','label':label,'ids':sorted(ids)})
        ev=[e for _,e in sorted(events.items())]
        roster={u for e in ev for u in (e['homeUUID'],e['awayUUID']) if u}
        pre=[e for e in ev if e['date']<cut]
        post=[e for e in ev if e['date']>=cut]
        snapshot=None
        if pre and post and any('EXPLICIT' in e['dateBases'] for e in pre) and any('EXPLICIT' in e['dateBases'] for e in post):
            # Arithmetic on known matches only; no claim of full historical classification.
            standings={u:[0,0,0,0,0,0] for u in roster};excluded=0
            for e in pre:
                h,a=e['homeUUID'],e['awayUUID'];scores=e['preStartScores']
                if not h or not a or h==a or len(scores)!=1 or 'EXPLICIT' not in e['dateBases']:
                    excluded+=1;continue
                g,v=scores[0]
                for k,x,y in ((h,g,v),(a,v,g)):
                    row=standings[k];row[0]+=1;row[1]+=x>y;row[2]+=x==y;row[3]+=x<y;row[4]+=x;row[5]+=y
            values=C.defaultdict(set)
            for rule in rules:
                if season_key(rule['season'],'SOURCE_TITLE')==ed and 'regulation' in rule['categories']:
                    for k,v in rule['values'].items():
                        if k in ('pointsWin','pointsDraw','pointsLoss') and type(v) is int:values[k].add(v)
            points=all(len(values[k])==1 for k in ('pointsWin','pointsDraw','pointsLoss'))
            rows=[]
            for u,x in standings.items():
                row=dict(zip(('played','won','drawn','lost','gf','ga'),x),uuid=u)
                if points:row['calculatedPoints']=sum(x[i+1]*next(iter(values[k])) for i,k in enumerate(('pointsWin','pointsDraw','pointsLoss')))
                rows.append(row)
            rows.sort(key=lambda x:(-x.get('calculatedPoints',0),-(x['gf']-x['ga']),-x['gf'],x['uuid']))
            snapshot={'cutoff':cut,'status':'PARTIAL_ARITHMETIC_NOT_OFFICIAL_RANK','preStartEvents':len(pre),'excludedEvents':excluded,'rows':rows,'pointsRuleCandidate':{k:sorted(v) for k,v in values.items()},'limitations':'Completeness, sanctions, phase/carry-over and tie rules not certified; not a runtime table.'}
        explicit=sum('EXPLICIT' in e['dateBases'] for e in ev)
        full_rr=False
        n=len(roster)
        if simple and expected=={n} and n>1 and not conflicts and explicit==len(ev) and all(e['homeUUID'] and e['awayUUID'] for e in ev):
            pairset=set(pairs);legs=phases[0]['meetingsPerPair']
            full_rr=(len(ev)==n*(n-1)*legs//2 and len(pairset)==len(ev) and (len(pairset)==n*(n-1) if legs==2 else len({frozenset(k) for k in pairset})==len(pairset)))
        result.append({'edition':ed,'activeAtStartCandidate':bool(snapshot),'groups':groups[ed],'events':ev,'explicitEvents':explicit,'conflicts':conflicts,'nativeRoster':sorted(roster),'completeReviewedRegularComponent':full_rr,'observedStageCounts':phase_summaries(ev),'july1Arithmetic':snapshot})
        all_conflicts.extend({'edition':ed,**x} for x in conflicts)
    tables,changes=table_summaries(rules)
    return {'editions':result,'unresolvedEditionGroups':uncertain,'conflicts':all_conflicts,'publishedTableCandidates':tables,'observedTableMembershipChanges':changes,'ruleAndTableEvidence':rules,'scope':'Exact edition candidates only; historical results before START; not verified regulations or rankings.'}

def selftest():
    aliases={('ENG','premiership'):{'ENG DIV1'}}
    lines=['== England 1996/97','Premiership','Round 1 [Aug 17, 1996]','Alpha 2 - 0 Beta','Beta  1    Alpha  2','Alpha - Beta  3-1  10,234','Final Table','1. Alpha  3 3 0 0 7-2 9','Second Level (Division One)','Round 1 [Aug 18, 1996]','Gamma 1 - 0 Delta','Regional leagues','Gamma 2 - 0 Delta']
    rows=list(scan_document(lines,{'ENGLAND':'ENG'},aliases))
    fx=[x for x in rows if x[0]=='fixture']
    assert len(fx)==4 and [x[2] for x in fx]==['ENG DIV1']*3+['ENG DIV2']
    assert fx[2][-1][6:10]==('Alpha','Beta',3,1)
    assert match_line('Arsenal         2 West Ham U       0    Hartson 27, Bergkamp 40p 38,056')[:4]==('Arsenal','West Ham U',2,0)
    assert match_line('Verdy Kawasaki          - Kyoto Purple Sanga       1-0          12,835')[:4]==('Verdy Kawasaki','Kyoto Purple Sanga',1,0)
    assert date_token('Sept 02','1996/97')==('1996-09-02','INFERRED_FROM_EDITION')
    assert any(x[0]=='rule' and x[-1].startswith('TABLE_ROW_JSON ') for x in rows)
    assert match_line('1. Alpha  3 3 0 0 7-2 9') is None
    assert table_row('7.Sheffield Wednesday 38  14 15  9  50-51  57')['team']=='Sheffield Wednesday'
    assert table_row('1 Manchester United 38 15 4 0 36 9 10 3 6 37 26 38 0 82 C','HOME_AWAY')['won']==25
    assert table_row('1. ( 1) Kashima Antlers 30 21 9 61-34 66','NO_DRAW')['points']==66
    assert season_key('1997','SOURCE_SEASON_END_YEAR')=='1996/1997'
    assert season_key('1996','CALENDAR_YEAR_BUCKET') is None
    assert date_token('Sep 7','1996/97')[1]=='INFERRED_FROM_EDITION'
    assert categorize('3 points for a win; 1 point for a draw; 0 points for a loss')[1]=={'pointsWin':3,'pointsDraw':1,'pointsLoss':0}
    k=('9697','ENG','a',1,1);l=('9697','ENG','b',1,2)
    g={'source':'test','competition':'ENG DIV1','country':'ENG','season':'1996','season_basis':'SOURCE_TITLE'}
    fs=[('1996-06-01','EXPLICIT','Alpha','Beta',2,0,'x',''),('1996-08-01','EXPLICIT','Beta','Alpha',9,9,'y','')]
    option=((),g,fs,{'inferredDates':0},{'Alpha':k,'Beta':l},[],True,True)
    z=combine([option,option],'9697',{},[],{},set())
    e=z['editions'][0]
    assert len(e['events'])==2 and e['events'][1]['preStartScores']==[] and e['july1Arithmetic']['rows'][0]['played']==1
    bad=((),g,[('1996-06-01','EXPLICIT','Alpha','Beta',1,0,'x2','')],{},option[4],[],True,False)
    assert combine([option,bad],'9697',{},[],{},set())['conflicts']
    uncertain=((),dict(g,season_basis='CALENDAR_YEAR_BUCKET'),fs,{},option[4],[],True,False)
    assert len(combine([uncertain],'9697',{},[],{},set())['unresolvedEditionGroups'])==1
    assert categorize('4 groups of 6 teams; top 2 teams from each group qualify')[1]=={'groupSize':6,'groupCount':4,'advancePerGroup':2}
    k2=('9697','ENG','c',1,3);native={k:{'name':'Alpha FC'},l:{'name':'Beta'},k2:{'name':'Gamma'}}
    f=[('1996-06-01','EXPLICIT','Alpha FC','Beta',None,None,'a',''),('1996-06-02','EXPLICIT','Alpha FC','Gamma',None,None,'b','')]
    good=((),g,f,{'inferredDates':0},{'Alpha FC':k,'Beta':l,'Gamma':k2},[],True,False)
    other=((),dict(g,source='other'),[(x[0],x[1],'Alpha',*x[3:]) for x in f],{'inferredDates':0},{'Beta':l,'Gamma':k2},[{'label':'Alpha','candidateCount':0}],False,False)
    fixed,aliases=reconcile([good,other],native,set(native));assert len(aliases)==1 and fixed[1][6]
    assert not reconcile([good,tuple(other[:2])+([other[2][0]],)+tuple(other[3:])],native,set(native))[1]
