#!/usr/bin/env python3
"""FASE 3 point 1: inventory of the competitions REPRESENTED in the data of
worlds 9697 and 2627, derived from the repository files only (no web data,
no invented rows).  Read-only; prints a Markdown report.

Sources (each row says which):
  9697  data_json/competitions/{leagues,cups,tournaments,playoffs}.json (OG)
  2627  data_json/worlds/2627/competitions/leagues.json  (CE type0 leagues)
        data_json/worlds/2627/competitions/ce_competitions.json (CE type1/2)
        sorgenti/src/world_data.c kWorld2627* tables (2627 mod, OG-native;
        NOT executed by the CE: ExpandedUniversalLive owns 2627 careers)

Status words:
  CE_RUNTIME       definition loaded and executed by the Competition Engine
  CE_BLOCKED       loaded by the CE, blocked locally by `missing`/chain rule
  OG_NATIVE_ONLY   present in OG/mod tables, not a CE definition (2627: not
                   run in a CE career; 9697: legacy OG career, unchanged)
"""
import json, os, re, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

def load(rel):
    with open(os.path.join(ROOT, rel), encoding='utf-8') as f:
        return json.load(f)

def og_9697():
    rows = []
    for l in load('data_json/competitions/leagues.json')['leagues']:
        if l['id'] == '_customsLeague':
            continue
        for d in range(l['divisions_count']):
            rows.append(('LEAGUE_DIVISION', l['id'], 'div%d' % (d + 1)))
    for c in load('data_json/competitions/cups.json')['competitions']:
        i = c['id']
        if i in ('_cupWinnersCup', '_uefaCup'):
            cat = 'CONTINENTAL_CLUB'
        elif 'WorldCupQualif' in i or 'ecQualifying' in i:
            cat = 'NATIONAL_TEAM_QUALIFIER_GROUP'
        elif i in ('_oceaniaWorldCupQualification', '_oceaniaCup'):
            cat = 'NATIONAL_TEAM_TOURNAMENT'
        elif i in ('_invitationTournament', '_tour', '_friendly'):
            cat = 'FRIENDLY_OR_CUSTOM'
        elif 'LeagueCup' in i:
            cat = 'LEAGUE_CUP'
        else:
            cat = 'NATIONAL_CUP'
        rows.append((cat, i, ''))
    for t in load('data_json/competitions/tournaments.json')['competitions']:
        i = t['id']
        cat = 'CONTINENTAL_CLUB' if i in ('_euroCup', '_copaLibertadores') \
            else 'NATIONAL_TEAM_TOURNAMENT'
        rows.append((cat, i, ''))
    for p in load('data_json/competitions/playoffs.json')['playoffs']:
        rows.append(('PROMOTION_PLAYOFF', p['id'], ''))
    return [(c, i, x, 'OG_NATIVE_ONLY') for c, i, x in rows]

def mod_2627_tables():
    src = open(os.path.join(ROOT, 'sorgenti/src/world_data.c'), encoding='latin-1').read()
    def table(name):
        m = re.search(r'static const \w+ %s\[\d*\] = \{\n(.*?)\n\};' % name, src, re.S)
        return m.group(1).splitlines() if m else []
    out = []
    for line in table('kWorld2627Cups'):
        m = re.search(r'/\* (_\w+) \*/', line)
        if m:
            cat = 'LEAGUE_CUP' if 'LeagueCup' in m.group(1) else 'NATIONAL_CUP'
            out.append((cat, m.group(1), ''))
    for line in table('kWorld2627ContiCups'):
        m = re.search(r'\{ "([^"]+)"', line)
        if m:
            n = m.group(1)
            cat = 'SUPERCUP_OR_INTERCONTINENTAL' if re.search(
                'RECOPA|SUPER CUP|INTERCONTINENTAL', n) else 'CONTINENTAL_CLUB'
            out.append((cat, n, 'kWorld2627ContiCups'))
    for line in table('kWorld2627StateContests46'):
        m = re.search(r'\{ "([^"]+)"', line)
        if m:
            out.append(('REGIONAL_STATE_LEAGUE', m.group(1), 'BRAZIL'))
    for line in table('kWorld2627Playoffs'):
        m = re.search(r'/\* (\w+) \*/', line)
        if m:
            out.append(('LEAGUE_PLAYOFF_PLAYOUT_SPEC', m.group(1), ''))
    for line in table('kWorld2627Splits'):
        m = re.search(r'\{ (\d+), (\d+), ', line)
        if m:
            out.append(('LEAGUE_SPLIT_SPEC', 'country%s' % m.group(1), 'div%d' % (int(m.group(2)) + 1)))
    for line in table('kWorld2627Groups'):
        m = re.search(r'/\* (\w+) \*/', line)
        if m:
            out.append(('LEAGUE_PARALLEL_GROUPS_SPEC', m.group(1), ''))
    for n in ('CHAMPIONS_LEAGUE', 'EUROPA_LEAGUE', 'CONFERENCE_LEAGUE'):
        out.append(('CONTINENTAL_CLUB', 'UEFA_' + n, 'OG compId 0..2'))
    # national-team competitions the 2627 runtime audit enumerates
    # (career_status.c og-external, OG tournament tables shared by both worlds)
    for n in ('WORLD_CUP', 'EUROPEAN_CHAMPIONSHIPS', 'COPA_AMERICA',
              'AFRICAN_NATIONS_CUP', 'NORTH_AMERICAN_CUP', 'ASIAN_CUP', 'OCEANIA_CUP'):
        out.append(('NATIONAL_TEAM_TOURNAMENT', n, 'OG tournament table'))
    for n in ('WORLD_CUP_QUALIFIERS', 'EURO_QUALIFIERS'):
        out.append(('NATIONAL_TEAM_QUALIFIER_GROUP', n, 'OG qualifying groups'))
    return [(c, i, x, 'OG_NATIVE_ONLY') for c, i, x in out]

def ce_2627():
    rows = []
    leagues = load('data_json/worlds/2627/competitions/leagues.json')
    leagues = leagues['leagues'] if isinstance(leagues, dict) else leagues
    for l in leagues:
        for d in range(l['divisions_count']):
            rows.append(('LEAGUE_DIVISION', l['id'], 'div%d' % (d + 1), 'CE_RUNTIME'))
    ce = load('data_json/worlds/2627/competitions/ce_competitions.json')
    for c in ce['competitions']:
        cat = 'CONTINENTAL_CLUB' if c['country'] >= 200 else 'NATIONAL_CUP'
        rows.append((cat, c['name'], 'id=%d' % c['id'],
                     'CE_BLOCKED' if c.get('missing') else 'CE_RUNTIME'))
    return rows

# Real competitions known to exist (research blobs in Issue #1, 24/11/2026)
# that NO repository table represents. Listed by name only: nothing about
# their format is asserted here.
NOT_REPRESENTED_2627 = (
    ('SUPERCUP_OR_INTERCONTINENTAL', 'UEFA SUPER CUP'),
    ('SUPERCUP_OR_INTERCONTINENTAL', 'FIFA CLUB WORLD CUP'),
    ('SUPERCUP_OR_INTERCONTINENTAL', 'NATIONAL SUPER CUPS (per country)'),
    ('CONTINENTAL_CLUB', 'OFC PRO LEAGUE (2026 edition ended before START)'),
    ('NATIONAL_TEAM_TOURNAMENT', 'UEFA / CONCACAF NATIONS LEAGUE'),
    ('LEAGUE_DIVISION', 'LIGUE 3 FRANCE (professional from 2026/27)'),
    ('LEAGUE_DIVISION', 'J3 LEAGUE JAPAN'),
)

def report():
    w97 = og_9697()
    w26 = ce_2627() + mod_2627_tables()
    print('# Inventario competizioni rappresentate nei dati (generato)\n')
    print('Generato da `sorgenti/tools/competition_inventory.py`; nessun dato web.\n')
    for name, rows in (('9697', w97), ('2627', w26)):
        cnt = {}
        for c, _, _, s in rows:
            cnt.setdefault(c, {}).setdefault(s, 0)
            cnt[c][s] += 1
        print('## Mondo %s — %d voci\n' % (name, len(rows)))
        print('| categoria | stato | voci |\n|---|---|---:|')
        for c in sorted(cnt):
            for s in sorted(cnt[c]):
                print('| %s | %s | %d |' % (c, s, cnt[c][s]))
        print()
    print('## 2627 — esistenti nella realtà ma NON rappresentati (solo nome)\n')
    for c, n in NOT_REPRESENTED_2627:
        print('- %s: %s — UNAVAILABLE' % (c, n))
    print()
    return w97, w26

if __name__ == '__main__':
    w97, w26 = report()
    if len(sys.argv) > 1 and sys.argv[1] == '--list':
        for name, rows in (('9697', w97), ('2627', w26)):
            print('## Elenco %s\n' % name)
            for r in rows:
                print('- %s | %s | %s | %s' % r)
            print()
