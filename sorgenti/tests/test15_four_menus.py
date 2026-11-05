"""0.10.5: four pixel-art menus use actual SWOS hitboxes and original callbacks."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
src=root/'sorgenti'/'src'
s=(src/'menu_data.c').read_text()
c=(src/'expanded_economy.c').read_text()
m=(src/'menu_system.c').read_text()
for name in ['expanded_title_bg.raw','expanded_career_bg.raw','expanded_calendar_bg.raw','club_business_bg.raw']:
    path=root/'data'/'expanded'/name
    assert path.stat().st_size==320*256,(name,path.stat().st_size)
for i in range(1,6):
    assert (root/'data'/'expanded'/f'club_stadium_{i}.raw').stat().st_size==154*88
assert len({(root/'data'/'expanded'/f'club_stadium_{i}.raw').read_bytes() for i in range(1,6)})==5
block=s.split('} _expandedClubOverviewMenu = {',1)[1].split('MENU_MARK_END_MENU\n};',1)[0]
coords=[tuple(map(int,a)) for a in re.findall(r'\{\s*\{\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\}',block)]
assert len(coords)==11
native=[(x+24,y+36,w,h) for x,y,w,h in coords]
expected={1:(243,108,70,18),2:(49,133,43,48),3:(137,133,43,48),
          4:(225,133,43,48),5:(5,133,43,48),6:(93,133,43,48),
          7:(181,133,43,48),8:(269,133,43,48),10:(5,3,47,17)}
for i,v in expected.items():assert native[i]==v,(i,native[i],v)
for x,y,w,h in native: assert x>=0 and y>=0 and x+w<=320 and y+h<=256,(x,y,w,h)
assert block.count('MENU_CMD_ONSELECT_FIRE, Expanded_ClubFacilitySelected')==8
assert 'MENU_CMD_ONSELECT_FIRE, SetExitMenuFlag' in block
assert 'Expanded_UI_Prepare();' in m and 'Expanded_UI_Before();' in m and 'Expanded_UI_After();' in m
assert 'Expanded_UI_Prepare(void)' in c and 'Expanded_UI_After(void)' in c
assert 'Expanded_UnemployedCalendarPointer()' in c
assert 'cb_blit(83,33,CB_SW,CB_SH' in c
# Original callbacks in career and title retained, no hardcoded fake menu actions.
for fn in ['EditTacticsMenu','EditCustomTeamMenu','HighlightsMenu','OptionsMenuSelected',
           'Expanded_ShowHall','FriendlyMenu','DIYCompetitionMenu','PresetCompetitionMenu',
           'SeasonMenu','CareerMenu','MultiplayerMenuSelected','LoadOldCompetitionMenu',
           'SetName','SetSurname','ChangeNationality','Expanded_EditManagerAge',
           'Expanded_ChangeManagerFame','ChooseTeamAndStartCareer']:
    assert fn in s,fn
assert 'Expanded_ClubOverviewDraw();' in m
print('PASS 0.10.5: 4 indexed backgrounds, 5 stadium variants, 9 club buttons, callbacks and menu hooks')
