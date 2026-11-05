"""SWOS Expanded 0.10.5 - check pixel art resources and all menu hitboxes."""
from pathlib import Path
import re
r=Path(__file__).resolve().parents[1]
game=r.parent
s=(r/'src/menu_data.c').read_text()
block=s.split('} _expandedClubOverviewMenu = {',1)[1].split('MENU_MARK_END_MENU\n};',1)[0]
assert 'MENU_MARK_WIDE_COORDS' in block
boxes=[tuple(map(int,m)) for m in re.findall(r'\{\s*\{\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\}',block)]
assert len(boxes)==11,boxes
native=[(x+24,y+36,w,h) for x,y,w,h in boxes]
assert native[0]==(96,3,126,18),native[0]
expected={1:(243,108,70,18),2:(49,133,43,48),3:(137,133,43,48),
          4:(225,133,43,48),5:(5,133,43,48),6:(93,133,43,48),
          7:(181,133,43,48),8:(269,133,43,48),10:(5,3,47,17)}
for i,box in expected.items():assert native[i]==box,(i,native[i],box)
for x,y,w,h in native:
    assert 0<=x and 0<=y and x+w<=320 and y+h<=256,(x,y,w,h)
# All real actions have a live callback, all facility ordinals 1..8 open detail.
assert block.count('MENU_CMD_ONSELECT_FIRE, Expanded_ClubFacilitySelected')==8
assert 'MENU_CMD_ONSELECT_FIRE, SetExitMenuFlag' in block
assert 'MENU_CMD_ONSELECT_FIRE, Expanded_ShowInfrastructure' in block
assert len(re.findall(r'MENU_CMD_NAV, [^}]*\b10\b',block))>=8
assert (game/'data/expanded/club_business_bg.raw').stat().st_size==320*256
stadia=[]
for i in range(1,6):
    b=(game/f'data/expanded/club_stadium_{i}.raw').read_bytes()
    assert len(b)==154*88
    stadia.append(b)
assert len(set(stadia))==5, 'Each stadium tier must show a different picture'
economy=(r/'src/expanded_economy.c').read_text()
assert 'cb_draw_full()' in economy and 'cb_draw_selection()' in economy
assert 'Expanded_UpgradeFacility' in economy and 'facility_cost(idx)' in economy
print('PASS 0.10.5: 9 visible clickable actions, 5 distinct stadiums, all assets in bounds')
