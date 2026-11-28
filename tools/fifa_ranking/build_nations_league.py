#!/usr/bin/env python3
"""Build the UEFA Nations League 2026/27 CE definition into
data_json/worlds/2627/competitions/ce_competitions.json (replaces the entry
with the same id, keeps everything else byte-for-byte as JSON objects).

Inputs (read-only, verified research data):
  docs/competition_research/WORLD/INTERNATIONAL/2627/NATIONAL_TEAM_QUALIFIER_GROUP.json
     row 536 roundTemplateAudit.unlDependencyRevision (groups, calendar,
     block4MatchSheet, block5RoundProfile, block10RuleDecision)
  docs/competition_research/WORLD/FIFA_RANKING_NATIONAL_ID_MAP_2627.json
     FIFA code -> OG national team file 80..85 / ordinal (verified identities)
Step 1 (v00.13.355): league phase, 54 nations, 14 groups, 156 matches, real
groups and verified per-group round days; KO / play-offs / final ranking are
later phases. Usage: python tools/fifa_ranking/build_nations_league.py [--check]
"""
import json, os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
NL_ID = (0xC9 << 16) | (0x10 << 8) | 0x02     # 13176834: international (201), JSON comp (low byte 2)
NL28_ID = (0xC9 << 16) | (0x11 << 8) | 0x02   # 13177090: same competition, 2028/29+ format

def main():
    src = os.path.join(ROOT, 'docs', 'competition_research', 'WORLD', 'INTERNATIONAL', '2627',
                       'NATIONAL_TEAM_QUALIFIER_GROUP.json')
    idm = json.load(open(os.path.join(ROOT, 'docs', 'competition_research', 'WORLD',
                                      'FIFA_RANKING_NATIONAL_ID_MAP_2627.json'), encoding='utf-8'))
    ids = {r['code']: r for r in idm['records']}
    rows = json.load(open(src, encoding='utf-8'))
    u = [r for r in rows if r.get('row') == 536][0]['roundTemplateAudit']['unlDependencyRevision']
    groups = u['groups']
    order = sorted(groups.keys(), key=lambda g: ('ABCD'.index(g[0]), int(g[1:])))
    assert len(order) == 14
    # verified group day of each round (block5RoundProfile.groupRounds)
    gr = u['block5RoundProfile']['groupRounds']
    assert [r['round'] for r in gr] == [1, 2, 3, 4, 5, 6]
    gday = {g: [] for g in order}
    for r in gr:
        seen = set()
        for d in r['days']:
            for g in d['groups']:
                gday[g].append(d['date']); seen.add(g)
        assert seen == set(order), r['round']
    # round anchor = the EARLIEST group day of the round (offsets >= 0)
    anchors = [min(gday[g][i] for g in order) for i in range(6)]
    # entrants in ACCESS LIST order (block8VerifiedAccessSeed.ranked): the CE
    # seed = access-list rank, last tie-break of art. 15/19 (never the FIFA rank)
    ranked = u['block8VerifiedAccessSeed']['ranked']
    group_of = {code: gi for gi, g in enumerate(order, 1) for code in groups[g]}
    assert sorted(r['rank'] for r in ranked) == list(range(1, 55))
    entrants = []
    seen = set()
    for a in sorted(ranked, key=lambda r: r['rank']):
        code = a['code']
        r = ids[code]
        assert code not in seen and code in group_of; seen.add(code)
        assert a['stableClub'] == r['stableClub'], code
        entrants.append({"nation": "%d:%d" % (r['teamFile'], r['ordinal']), "code": code,
                         "group": group_of[code], "access_rank": a['rank'], "source": "VERIFIED"})
    assert len(entrants) == 54
    # official group tie-break order (block13CalendarAndComparators.art2627.groupTiebreak)
    tbmap = {"H2H_POINTS": "H2H_POINTS", "H2H_GOAL_DIFFERENCE": "H2H_GOAL_DIFF", "H2H_GOALS": "H2H_GOALS_FOR",
             "H2H_REAPPLY_REMAINING_SUBSET": "H2H_REAPPLY", "OVERALL_GOAL_DIFFERENCE": "GOAL_DIFF",
             "OVERALL_GOALS": "GOALS_FOR", "OVERALL_AWAY_GOALS": "AWAY_GOALS_FOR", "OVERALL_WINS": "WINS",
             "OVERALL_AWAY_WINS": "AWAY_WINS", "DISCIPLINE_ASC": "DISCIPLINE", "ACCESS_LIST_RANK_ASC": "ACCESS_LIST"}
    gtb = u['block13CalendarAndComparators']['art2627']['groupTiebreak']
    assert len(gtb) == 11
    tiebreak = ["POINTS"] + [tbmap[x] for x in gtb]
    xmap = {"GROUP_POSITION_ASC": "GROUP_POSITION", "POINTS": "POINTS", "GOAL_DIFFERENCE": "GOAL_DIFF",
            "GOALS": "GOALS_FOR", "AWAY_GOALS": "AWAY_GOALS_FOR", "WINS": "WINS", "AWAY_WINS": "AWAY_WINS",
            "DISCIPLINE_ASC": "DISCIPLINE", "ACCESS_LIST_RANK_ASC": "ACCESS_LIST"}
    cross = [xmap[x] for x in u['block13CalendarAndComparators']['art2627']['crossGroupRanking']]
    mv = u['revisedMovement']
    assert mv['CDPlayoff'] is False and mv['legs'] == 2
    # final ranking buckets (revisedMovement.finalRankBuckets; phases 1-based:
    # 1 league, 2 QF, 3 A/B, 4 B/C, 5 SF, 6 third place, 7 final)
    final_ranking = [
        {"order": "FIXED", "select": [{"winner": [7, 1]}, {"loser": [7, 1]}, {"winner": [6, 1]}, {"loser": [6, 1]}]},
        {"order": "INTERIM", "select": [{"losers": 2}]},
        {"order": "INTERIM", "select": [{"interim": [9, 10]}]},
        {"order": "INTERIM", "select": [{"interim": [17, 20]}, {"winners": 3}]},
        {"order": "INTERIM", "select": [{"interim": [15, 16]}, {"losers": 3}]},
        {"order": "INTERIM", "select": [{"interim": [25, 28]}]},
        {"order": "INTERIM", "select": [{"interim": [33, 36]}, {"winners": 4}]},
        {"order": "INTERIM", "select": [{"losers": 4}]},
        {"order": "INTERIM", "select": [{"interim": [41, 44]}]},
        {"order": "INTERIM", "select": [{"interim": [45, 48]}]},
        {"order": "INTERIM", "select": [{"interim": [49, 54]}]}]
    sizes = [b[1] for b in mv['finalRankBuckets']]
    assert sizes == [4, 4, 2, 8, 6, 4, 8, 4, 4, 4, 6] and sum(sizes) == 54
    ms = u['block4MatchSheet']['phaseOrdinary']
    comp = {
        "id": NL_ID, "name": "UEFA NATIONS LEAGUE", "country": 201, "type": 2, "cycle_years": 2,
        "viewer_button": True, "viewer_region": 80,
        "source": "NATIONAL_TEAM_QUALIFIER_GROUP.json row 536 unlDependencyRevision (groups, calendar, block4MatchSheet, block5RoundProfile); revision 2026-09-15 approved by the user (block10RuleDecision)",
        "missing": "",
        "participants": "NATIONAL",
        "match": {"bench": ms['bench'], "subs": 5, "windows": 3, "halftime_free": 1, "et_extra_subs": 1,
                  "et_extra_windows": 1, "discipline": 1, "provenance": "VERIFIED"},
        "table": {"points": [3, 1, 0], "tiebreak": tiebreak, "provenance": "VERIFIED",
                  "note": "art. 15 order from block13 groupTiebreak; DISCIPLINE = yellow 1, red 3, second-yellow dismissal 3 (team officials not produced by OG); ACCESS_LIST = block8VerifiedAccessSeed rank, no random component"},
        "expiry": {"date": "06-30", "year_offset": 1},
        "phases": [{
            "name": "LEAGUE PHASE", "group_labels": order, "kind": "TABLE", "groups": 14, "meetings": 2, "pairing": "LISTED",
            "window": {"first": "09-24", "last": "11-17", "weekdays": "MON,TUE,WED,THU,FRI,SAT,SUN",
                       "rest": 3, "international_breaks": ""},
            "fifa": {"importance": 15},
            "first_edition_days": anchors,
            "first_edition_group_days": [gday[g] for g in order],
            "qualify": 2, "qualify_groups": 4
        }, {
            "name": "QUARTER-FINALS", "kind": "KNOCKOUT", "pairing": "WINNERS_RUNNERS_DRAW", "higher_home": 2,
            "match": {"legs": 2, "extra_time": 1, "penalties": 1, "away_goals": 0},
            "window": {"first": "03-25", "last": "03-30", "weekdays": "MON,TUE,WED,THU,FRI,SAT,SUN", "rest": 3,
                       "international_breaks": ""},
            "fifa": {"importance": 25}
        }, {
            "name": "PLAY-OFF A/B", "kind": "KNOCKOUT", "pairing": "POT_PAIRS", "higher_home": 2, "parallel": True,
            "interim_pots": {"hi": mv['AB']['higher'], "lo": mv['AB']['lower']},
            "match": {"legs": 2, "extra_time": 1, "penalties": 1, "away_goals": 0},
            "window": {"first": "03-25", "last": "03-30", "weekdays": "MON,TUE,WED,THU,FRI,SAT,SUN", "rest": 3,
                       "international_breaks": ""},
            "fifa": {"importance": 25},
            "note": "A/B play-off (revision 15/09/2026): interim 11-14 v 21-24, lower league hosts the first leg"
        }, {
            "name": "PLAY-OFF B/C", "kind": "KNOCKOUT", "pairing": "POT_PAIRS", "higher_home": 2, "parallel": True,
            "interim_pots": {"hi": mv['BC']['higher'], "lo": mv['BC']['lower']},
            "match": {"legs": 2, "extra_time": 1, "penalties": 1, "away_goals": 0},
            "window": {"first": "03-25", "last": "03-30", "weekdays": "MON,TUE,WED,THU,FRI,SAT,SUN", "rest": 3,
                       "international_breaks": ""},
            "fifa": {"importance": 25},
            "note": "B/C play-off: interim 29-32 v 37-40; no C/D play-off"
        }, {
            "name": "SEMI-FINALS", "kind": "KNOCKOUT", "pairing": "DRAW", "higher_home": 1, "source_phase": 2,
            "match": {"legs": 1, "extra_time": 1, "penalties": 1},
            "window": {"first": "06-09", "last": "06-10", "weekdays": "MON,TUE,WED,THU,FRI,SAT,SUN", "rest": 3,
                       "international_breaks": ""},
            "fifa": {"importance": 25}
        }, {
            "name": "THIRD PLACE", "kind": "KNOCKOUT", "pairing": "SOURCES", "higher_home": 1,
            "ties": [{"home": {"loser": [4, 1]}, "away": {"loser": [4, 2]}}],
            "match": {"legs": 1, "extra_time": 0, "penalties": 1},
            "window": {"first": "06-12", "last": "06-12", "weekdays": "MON,TUE,WED,THU,FRI,SAT,SUN", "rest": 0,
                       "international_breaks": ""},
            "fifa": {"importance": 25},
            "note": "third-place match: penalties directly after 90 minutes; played 12 June (one day before the final: the CE needs the final as last phase)"
        }, {
            "name": "FINAL", "kind": "KNOCKOUT", "pairing": "SOURCES", "higher_home": 1,
            "ties": [{"home": {"winner": [4, 1]}, "away": {"winner": [4, 2]}}],
            "match": {"legs": 1, "extra_time": 1, "penalties": 1},
            "window": {"first": "06-13", "last": "06-13", "weekdays": "MON,TUE,WED,THU,FRI,SAT,SUN", "rest": 0,
                       "international_breaks": ""},
            "fifa": {"importance": 25}
        }],
        "interim": {"phase": 0, "tiers": [4, 4, 4, 2], "cross": cross, "exclude_last_unequal": True,
                    "note": "art. 19 crossGroupRanking (block13); unequal group sizes: results v the last-placed team left out for the other positions"},
        "final_ranking": final_ranking,
        "first_edition": {"year": 2026, "entrants": entrants},
        "last_year": 2026,
        "note_successor": "from 2028/29 the competition continues with the new format as definition %d" % NL28_ID
    }
    comp28 = build_2028(comp, ms)
    dst = os.path.join(ROOT, 'data_json', 'worlds', '2627', 'competitions', 'ce_competitions.json')
    text = open(dst, encoding='utf-8').read()
    data = json.loads(text)
    others = [c for c in data['competitions'] if c.get('id') != NL_ID]
    # textual edit: every other competition keeps its exact formatting
    block = '    ' + json.dumps(comp, ensure_ascii=False)
    out = text
    for cid, cobj in ((NL_ID, comp), (NL28_ID, comp28)):
        block = '    ' + json.dumps(cobj, ensure_ascii=False)
        marker = '{"id": %d,' % cid
        if marker in out:            # replace our own previous single line
            start = out.index(marker) - 4
            end = out.index('\n', start)
            if out[end - 1] == ',':
                block += ','
            out = out[:start] + block + out[end:]
        else:
            body = out.rstrip()
            assert body.endswith('}')
            body = body[:-1].rstrip()
            assert body.endswith(']')
            body = body[:-1].rstrip()
            out = body + ',\n' + block + '\n  ]\n}\n'
    others = [c for c in data['competitions'] if c.get('id') not in (NL_ID, NL28_ID)]
    assert len(json.loads(out)['competitions']) == len(others) + 2
    if '--check' in sys.argv:
        print('ce_competitions.json', 'UP TO DATE' if out == text else 'STALE'); return 0 if out == text else 1
    open(dst, 'w', encoding='utf-8', newline='\n').write(out)
    print('UEFA NATIONS LEAGUE id=%d entrants=%d groups=%d anchors=%s' % (NL_ID, len(entrants), len(order), anchors))
    return 0

def build_2028(c26, ms):
    """UEFA Nations League 2028/29+ (user decision 28/11/2026). OFFICIAL = UEFA
    concept (block12SuccessorConstraints / block15); SWOS_PROVISIONAL = 2026/27
    rules reused or SWOS adaptation, replaceable by data when UEFA publishes
    the regulations (no engine change)."""
    import copy
    ph26 = c26['phases']
    tb = c26['table']
    # groups of 6: slots 0-1 pot 1, 2-3 pot 2, 4-5 pot 3. Same-pot pair home and
    # away; one game v each team of the two other pots (pot1 hosts pot2, pot2
    # hosts pot3, pot3 hosts pot1): 6 games, 5 opponents, 3 home / 3 away,
    # never more than 2 home or 2 away games in a row (SWOS round order).
    template = [[[0, 1], [2, 3], [4, 5]], [[5, 0], [1, 2], [3, 4]], [[4, 0], [1, 3], [2, 5]],
                [[0, 3], [5, 1], [2, 4]], [[0, 2], [4, 1], [3, 5]], [[1, 0], [3, 2], [5, 4]]]
    for s_ in range(6):
        games = [m for r in template for m in r if s_ in m]
        assert len(games) == 6 and sum(1 for m in games if m[0] == s_) == 3
        assert len({m[0] if m[1] == s_ else m[1] for m in games}) == 5
    labels = ["%s%d" % (lg, g) for lg in "ABC" for g in (1, 2, 3)]
    ko = lambda i: copy.deepcopy(ph26[i])
    qf = ko(1)
    qf.update({"pairing": "POT_PAIRS", "interim_pots": {"hi": [1, 2, 3, 4], "lo": [5, 6, 7, 8]},
               "status": "OFFICIAL: League A group winners + runners-up + 2 best thirds; SWOS_PROVISIONAL: draw interim 1-4 v 5-8, higher hosts leg 2"})
    ab = ko(2)
    ab.update({"interim_pots": {"hi": list(range(10, 16)), "lo": list(range(22, 28))},
               "note": "A/B: League A 4th+5th (interim 10-15) v League B 2nd+3rd (22-27); 2026/27 draw procedure; higher league hosts leg 2",
               "status": "USER_DECISION 28/11/2026 / SWOS_PROVISIONAL"})
    bc = ko(3)
    bc.update({"interim_pots": {"hi": list(range(28, 34)), "lo": list(range(40, 46))},
               "note": "B/C: League B 4th+5th (interim 28-33) v League C 2nd+3rd (40-45)",
               "status": "USER_DECISION 28/11/2026 / SWOS_PROVISIONAL"})
    league = {
        "name": "LEAGUE PHASE", "group_labels": labels, "kind": "TABLE", "groups": 9, "meetings": 1,
        "pairing": "BANDED_POTS", "bands": {"size": 18, "groups": 3, "per_group": 2},
        "round_template": template, "qualify": 2, "qualify_groups": 3,
        "window": copy.deepcopy(ph26[0]['window']),
        "fifa": {"importance": 15},
        "first_edition_days": ["2028-09-21", "2028-09-24", "2028-09-28", "2028-10-01", "2028-11-16", "2028-11-19"],
        "status": "OFFICIAL: 3 leagues x 18 (access list 1-18 A, 19-36 B, 37-54 C), 3 groups of 6, 3 pots of 2 per group (local bands 1-6/7-12/13-18), same-pot opponent home and away, one game v each other team; SWOS_PROVISIONAL: CE draw, venue pattern and round order, 2028 days = GAME_TEMPLATE_2026 candidate lanes (block13), later editions from the window"
    }
    comp = {
        "id": NL28_ID, "name": "UEFA NATIONS LEAGUE", "country": 201, "type": 2, "cycle_years": 2,
        "viewer_button": True, "viewer_region": 80,
        "source": "NATIONAL_TEAM_QUALIFIER_GROUP.json row 536 block12SuccessorConstraints/block15ConditionalReuseDecision + user decision 28/11/2026",
        "missing": "", "participants": "NATIONAL",
        "match": copy.deepcopy(c26['match']),
        "table": {"points": tb['points'], "tiebreak": tb['tiebreak'], "provenance": "GAME_TEMPLATE",
                  "note": "SWOS_PROVISIONAL: 2026/27 art. 15 order reused (H2H on unequal meetings as played)"},
        "expiry": copy.deepcopy(c26['expiry']),
        "phases": [league, qf, ab, bc, ko(4), ko(5), ko(6)],
        "interim": {"phase": 0, "tiers": [3, 3, 3], "cross": c26['interim']['cross'], "exclude_last_unequal": False,
                    "note": "SWOS_PROVISIONAL (user 28/11/2026): best thirds and league ranking by group position, points, goal difference, goals, away goals, wins, away wins, discipline, access-list rank (2026/27 criteria, equal groups)"},
        "final_ranking": [
            {"order": "FIXED", "select": [{"winner": [7, 1]}, {"loser": [7, 1]}, {"winner": [6, 1]}, {"loser": [6, 1]}]},
            {"order": "INTERIM", "select": [{"losers": 2}]},
            {"order": "INTERIM", "select": [{"interim": [9, 9]}]},
            {"order": "INTERIM", "select": [{"winners": 3}]},
            {"order": "INTERIM", "select": [{"interim": [19, 21]}]},
            {"order": "INTERIM", "select": [{"losers": 3}]},
            {"order": "INTERIM", "select": [{"interim": [16, 18]}]},
            {"order": "INTERIM", "select": [{"winners": 4}]},
            {"order": "INTERIM", "select": [{"interim": [37, 39]}]},
            {"order": "INTERIM", "select": [{"losers": 4}]},
            {"order": "INTERIM", "select": [{"interim": [34, 36]}]},
            {"order": "INTERIM", "select": [{"interim": [46, 54]}]}],
        "final_ranking_note": "SWOS_PROVISIONAL: ranks 1-18 = next League A (final four, QF losers, best non-qualified third, A/B winners, League B group winners), 19-36 = League B, 37-54 = League C; OFFICIAL concept: top 18 A, next 18 B, rest C after knockout and movement",
        "first_edition": {"year": 2028, "from_sources": True},
        "next_entrants": [{"source": "LEAGUE_RANK", "competition": NL28_ID, "alt_competition": NL_ID,
                           "from": 1, "to": 54,
                           "note": "access list = final ranking of the previous edition (2026/27 for 2028/29)"}]
    }
    assert sum({"FIXED": 4}.get(b['order'], 0) for b in comp['final_ranking']) == 4
    return comp

if __name__ == '__main__':
    sys.exit(main())
