#!/usr/bin/env python3
"""0.10.8: regression checks for actual original-menu integration."""
from pathlib import Path
import hashlib,re
root=Path(__file__).resolve().parents[2]
s=(root/'sorgenti/src/expanded_economy.c').read_text()
engine=(root/'sorgenti/src/menu_system.c').read_text()
packed=(root/'sorgenti/src/menu_data.c').read_text()
assert 'ui_move(ordinal,219,top+(int)i*11,99,11);' in s
assert 'int top=255-(int)n*11;' in s  # no blank rows under buttons
assert 'if(!_menuEntries[i].isInvisible) order[n++]=i;' in s
assert '_menuEntries[16].isInvisible=1;' in s
assert '"HALL OF FAME"' in s
assert '"AGE (18-70)"' in s
assert 'ui_move(10,204,223,114,16);' in s
assert 'ui_move(11,204,241,114,15);' in s
assert 'ui_native_text' in s and 'GetTextWidth' in s
assert 'DrawMenuTextCentered' in s and 'DrawMenuTextRightAligned' in s
assert 'ui_native_text(252,24,118,15,date,0);' in s
assert '_menuEntries[18].background=NO_BACKGROUND;' in s
assert '_menuEntries[3].content=NO_FOREGROUND;' in s
assert 'Expanded_UI_Partial(entry);' in engine
assert 'ui_restore_rect(art,5,191,310,65);' in s
assert 'if(unemployed && i==7)' in s
# Native sprite font used by new menus; the 5x8 bitmap font remains confined
# to the previously approved Club Business.
a=s.index('static void ui_move(unsigned ordinal')
assert 'cb_text_step(' not in s[a:]
# No functional callbacks changed by the art-only patch.
assert 'MENU_CMD_ONSELECT_FIRE, Expanded_ShowHall' in packed
for f in ('club_business_bg.raw',*(f'club_stadium_{i}.raw' for i in range(1,6))):
    p=root/'data/expanded'/f
    assert p.exists() and p.stat().st_size in (320*256,154*88)
for f in ('expanded_title_bg.raw','expanded_career_bg.raw','expanded_calendar_bg.raw'):
    assert (root/'data/expanded'/f).stat().st_size==320*256
print('PASS 0.10.11: original SWOS font, career flush bottom, original callbacks, partial redraws')
