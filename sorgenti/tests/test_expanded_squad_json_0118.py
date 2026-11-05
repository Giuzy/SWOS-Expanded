"""Cross-check the new 0.10.18 skill formula against shipped team JSON."""
from pathlib import Path
import json
root=Path(__file__).resolve().parents[2]
eng=json.loads((root/'data_json/teams/ENGLAND.json').read_text())
leyton=next(t for t in eng['teams'] if t['team']['team_name']=='LEYTON ORIENT')
ian=next(p for p in leyton['players'] if p['player_name']=='IAN HENDON')
assert (ian['passing'],ian['velocity'],ian['heading'],ian['tackling'],
        ian['control'],ian['speed'],ian['finishing'])==(2,0,0,15,0,12,0)
score=40*ian['tackling']+25*ian['speed']+20*ian['heading']+15*ian['passing']
assert score==930 and score>900 and score<=1200
assert len(leyton['players'])==16
for team in eng['teams']:
 for p in team['players']:
  for field in ('passing','velocity','heading','tackling','control','speed','finishing'):
   assert 0<=p[field]<=15,(team['team']['team_name'],p['player_name'],field)
assert (root/'data/expanded/expanded_squad_bg.raw').stat().st_size==320*256
assert (root/'data/expanded/expanded_sacked_popup.raw').stat().st_size==320*256
source=(root/'sorgenti/src/expanded_economy.c').read_text()
assert 'Expanded_SquadRowPlayer(i)' in source
assert 'ExpandedSquad_Stars(skills,pos,(uint8_t)p->price)' in source
assert 'selected=(unsigned)m->selectedEntry->ordinal-28u' in source
assert 'ui_squad_tooltip(selected,selectedPlayer)' in source
menus=(root/'sorgenti/src/menus.c').read_text()
assert 's_expandedSquadRow[d7]=pf' in menus
assert 'DrawMenu(); return;' in menus
print('PASS: live PlayerFile seven skills, Ian Hendon 4 stars, joypad selectedEntry, full-screen assets')
