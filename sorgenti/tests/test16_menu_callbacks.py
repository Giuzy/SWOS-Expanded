#!/usr/bin/env python3
"""SWOS Expanded 0.10.7: check original menu actions and new native art."""
from pathlib import Path
import re,hashlib
root=Path(__file__).resolve().parents[2]
code=(root/'sorgenti/src/menu_data.c').read_text()
ui=(root/'sorgenti/src/expanded_economy.c').read_text()
main=code.split('Menu_SWOSMainMenu _swosMainMenu = {',1)[1].split('Menu_Offers _offersMenu = {',1)[0]
callbacks=re.findall(r'MENU_CMD_ONSELECT_FIRE,\s*(\w+)',main)
expected=['EditTacticsMenu','EditCustomTeamMenu','HighlightsMenu','OptionsMenuSelected',
          'Expanded_ShowHall','FriendlyMenu','DIYCompetitionMenu','PresetCompetitionMenu',
          'SeasonMenu','CareerMenu','StartContest','SaveContest','LoadOldCompetitionMenu',
          'MultiplayerMenuSelected','CreditsMenuSelected','Expanded_ShowHall']
assert callbacks==expected,(callbacks,expected)
assert 'CommonMenuExit();' in (root/'sorgenti/src/menus.c').read_text()
assert 'if(e->isInvisible) continue;' in ui
assert '_menuEntries[16].isInvisible=1;' in ui
assert 'e->background=NO_BACKGROUND;' in ui
assert 'e->content=NO_FOREGROUND;' in ui
assert 'e->upEntry=(int8_t)up;' in ui and 'e->downEntry=(int8_t)down;' in ui
assert 'ui_native_text(252,24,118,15,date,0);' in ui
for name in ['expanded_title_bg.raw','expanded_career_bg.raw','expanded_calendar_bg.raw','club_business_bg.raw']:
    assert (root/'data/expanded'/name).stat().st_size==320*256
for i in range(1,6):assert (root/f'data/expanded/club_stadium_{i}.raw').stat().st_size==154*88
assert 'Expanded_UnemployedCalendarPointer()' in ui
assert 'Expanded_ClubOverviewDraw(void)' in ui
print('PASS 0.10.7: 16 original callbacks, dynamic visibility, native-font date and menu navigation')
