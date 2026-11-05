#!/usr/bin/env python3
"""0.10.10: compact fixture rows and visual regressions, no callback changes."""
from pathlib import Path
import hashlib, zipfile
root=Path(__file__).resolve().parents[2]
s=(root/'sorgenti/src/expanded_economy.c').read_text()
m=(root/'sorgenti/src/menu_system.c').read_text()
menus=(root/'sorgenti/src/menus.c').read_text()
assets=root/'data/expanded'
assert 'ui_move(ordinal,219,top+(int)i*11,99,11);' in s
assert 'static const int rows[]={124,138,152,166,180,194,208};' in s
assert 'ui_move(11,204,241,114,15);' in s
assert '"AGE (18-70)"' in s
assert 'ui_short_comp(' in s and '"D. %s"' in s
assert 'Expanded_UI_ClearFixtureArea' in s and 'Expanded_UI_CompactFixtureRow' in s
assert 'ui_native_ellipsis' in s
assert 'ui_move(16,12,49,12,13);' in s
assert 'ui_move(17,12,169,12,13);' in s
assert 'Expanded_UI_CompactFixtureRow(d0,slotType)' in menus
assert 'Expanded_UI_ClearFixtureArea();' in menus
assert 'entry->ordinal>=33 && entry->ordinal<=43' in m
assert 'expandedFixtureText' not in m
assert 'cb_center_narrow(278,87,white,"COSTS");' in s
assert '"UPGRADE COST"' not in s
assert 'ui_native_text(252,24,118,15,date,0);' in s
assert 'if(unemployed && i==7)' in s
assert 'e->isDisabled' in s
for name in ('expanded_title_bg','expanded_career_bg','expanded_calendar_bg','expanded_unemployed_bg','club_business_bg'):
 assert (assets/f'{name}.raw').stat().st_size==320*256
for i in range(1,6):
 assert (assets/f'club_stadium_{i}.raw').stat().st_size==154*88
# The approved title/stadium art is identical; the packed offer menu
# intentionally gained two new salary buttons, while other menus remain.
previous=Path('/mnt/data/SWOS_Expanded_0_10_9_MENU_FIX_BAT.zip')
if previous.exists():
 with zipfile.ZipFile(previous) as z:
  for name in ('data/expanded/expanded_title_bg.raw',
               *(f'data/expanded/club_stadium_{i}.raw' for i in range(1,6))):
   old=z.read('SWOS_Expanded_0_10_9/'+name)
   assert old==(root/name).read_bytes(),name
print('PASS 0.10.14: unchanged title/stadium, compact fixture layout, no old grey bar')
