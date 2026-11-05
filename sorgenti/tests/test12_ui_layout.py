"""Static regression for SWOS packed menu coordinate conversion (+24,+36)."""
from pathlib import Path
import re
src = (Path(__file__).resolve().parents[1]/'src/menu_data.c').read_text()
career = (Path(__file__).resolve().parents[1]/'src/career_init.c').read_text()

def menu_block(name):
    m=re.search(r'\}\s*'+re.escape(name)+r'\s*=\s*\{(.*?)\n\};',src,re.S)
    assert m, name
    return m.group(1)

def boxes(name):
    return [tuple(map(int,m)) for m in re.findall(r'\{\s*\{\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\}',menu_block(name))]

def native(box):
    x,y,w,h=box
    return (x%256+24,y%256+36,w%256,h%256)

club=boxes('_expandedClubOverviewMenu')
assert len(club)==11,club
# 0.10.5 uses wide coordinates so the left and right facility cards have
# complete mouse hitboxes. The packed menu adds +24/+36 in both modes.
def wide(box):
    x,y,w,h=box
    return x+24,y+36,w,h
expected={1:(243,108,70,18),2:(49,133,43,48),3:(137,133,43,48),
          4:(225,133,43,48),5:(5,133,43,48),6:(93,133,43,48),
          7:(181,133,43,48),8:(269,133,43,48)}
for i,rect in expected.items():
    assert wide(club[i])==rect,(i,wide(club[i]),rect)
back=wide(club[10]); assert back==(5,3,47,17),back
preview=boxes('_expandedOfferPreviewMenu')
assert native(preview[-1])==(134,224,100,16),preview
assert 'if (!Expanded_WaitForFirstOffer(&offerTeam)) {\n            SetExitMenuFlag();' not in career
assert '_fireResetFlag = 1;' in career
# Every facility can navigate directly to the visible BACK (ordinal 10),
# without traversing the hidden UPGRADE entry (ordinal 9).
club_src=menu_block('_expandedClubOverviewMenu')
assert len(re.findall(r'MENU_CMD_NAV, [^}]*\b10\b',club_src)) >= 8
system=(Path(__file__).resolve().parents[1]/'src/menu_system.c').read_text()
assert 'Expanded_UI_After();' in system
assert 'if (_fire || _shortFire || _pl1Fire) return;' not in system
print('PASS 0.10.5: pixel-art card hitboxes, keyboard BACK, date strip and one-level return')
