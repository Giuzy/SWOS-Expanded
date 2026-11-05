from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'src/menu_data.c').read_text();m=(r/'src/menus.c').read_text()
e=(r/'src/expanded_economy.c').read_text();c=(r/'src/chairman_memo.c').read_text()
assert 'ShowMenu(_expandedClubOverviewMenu)' in c
assert 'Expanded_NationalityRowFromEntry' in s and 'entry->fg == (uintptr_t)s_natRows[i]' in s
assert 'Expanded_NationalityChangePage(-1)' in s and 'Expanded_NationalityChangePage(1)' in s
assert '_expandedNationalityAction = 1' not in s and '_expandedNationalityAction = 2' not in s
assert 'Expanded_NationalityMenuInit();\n    ((Menu *)_currentMenu)->selectedEntry' in m
assert '{ { 34, 181, 105, 16 }' in s and '{ { -110, 181, 105, 16 }' in s
assert 'Expanded_ClubOverviewDraw' in e and 'MONTHLY INCOME (AVG)' in e
print('PASS: nationality mapping, in-place pagination, club routing, facility button separation')
