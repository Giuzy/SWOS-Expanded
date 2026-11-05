#!/usr/bin/env python3
"""SWOS Expanded 0.10.6 - native indexed 320x256 title, career, calendar.

This generator intentionally does NOT touch club_business_bg.raw or stadium sprites:
that is the user-approved Club Business design with BACK in the top-left.
Only art (not menu callbacks) is generated. All selectable text is painted live.
"""
from pathlib import Path
from PIL import Image,ImageDraw,ImageEnhance
import re
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'data'/'expanded'; OUT.mkdir(parents=True,exist_ok=True)
PALETTE_SOURCE=(ROOT/'sorgenti'/'src'/'swos_globals.c').read_text()
m=re.search(r'uint16_t _menuPalette\[256\] = \{(.*?)\};',PALETTE_SOURCE,re.S)
assert m
v=[int(x,16) for x in re.findall(r'0x([0-9A-Fa-f]{4})',m.group(1))]
assert len(v)==256
pal=[(((c>>8)&15)*17,((c>>4)&15)*17,(c&15)*17) for c in v]
idxpal=Image.new('P',(1,1));idxpal.putpalette(sum([list(x) for x in pal],[]))

def save(im,name):
    im=im.convert('RGB');assert im.size==(320,256)
    indexed=im.quantize(palette=idxpal,dither=Image.Dither.NONE)
    b=bytes(indexed.getdata());assert len(b)==81920
    (OUT/(name+'.raw')).write_bytes(b)
    indexed.convert('RGB').resize((960,768),Image.Resampling.NEAREST).save(OUT/(name+'_preview.png'))
    return indexed

def box(d,xy,fill='#071c4c',stroke='#90c7ff'):
    x0,y0,x1,y1=xy
    d.rectangle((x0+1,y0+2,x1+1,y1+2),fill='#030b26')
    d.rectangle(xy,fill=fill,outline=stroke)
    d.line((x0+2,y0+1,x1-2,y0+1),fill='#c5e7ff')
    d.line((x0+1,y1-1,x1-1,y1-1),fill='#1b4e95')

def resize(src):return src.resize((320,256),Image.Resampling.NEAREST)

# Title art: approved yelling coach and the full, unframed logo, exactly once.
# The right-side source-image buttons are removed; all menu entries are live,
# so Continue/Save/Multiplayer visibility can change without baked text.
reference=ROOT/'sorgenti'/'tools'/'ref_title_0106.png'
assert reference.exists(), 'Missing the approved title illustration'
base=resize(Image.open(reference).convert('RGB'))
d=ImageDraw.Draw(base,'RGBA')
# Deep-blue right bank leaves the coach, players and logo exposed on the left.
d.rectangle((218,35,319,255),fill=(3,17,51,255))
d.rectangle((217,34,319,253),outline=(131,187,250,255),width=1)
d.line((219,35,319,35),fill=(205,235,255,255))
d.line((218,253,319,253),fill=(25,78,153,255))
# The background has no fixed buttons, preventing duplicated labels/artifacts.
save(base,'expanded_title_bg')

# Career: preserve the manager-arrival art from 0.10.5, but shift the right
# editable frame 11px LEFT so its outer border is fully visible on screen.
refcareer=ROOT/'sorgenti'/'tools'/'ref_career_calendar_0105.png'
assert refcareer.exists()
comp=Image.open(refcareer).convert('RGB')
# Same approved left scene (manager arriving at club), without a stretched face.
left=comp.crop((0,0,930,397)).resize((165,256),Image.Resampling.NEAREST)
career=Image.new('RGB',(320,256),'#03132e');career.paste(left,(0,0))
d=ImageDraw.Draw(career)
box(d,(163,3,307,29),'#197b15','#f3d03c')
box(d,(161,34,309,246),'#061b45')
for y in (39,59,79,99,119,139,159):
    box(d,(164,y,305,y+17),'#092c62')
box(d,(164,184,305,207),'#197f1a','#b8f18d')
box(d,(164,210,305,227),'#a61b20','#ffccae')
box(d,(164,230,305,243),'#153a77')
save(career,'expanded_career_bg')

# Calendar: symmetric fixture panel and three-column button grid, with the
# match scene visible as a backdrop. Native fixture list is still drawn on top
# using its unchanged SWOS coordinates (not a fake static fixture image).
# Use clean stadium/crowd edges from the approved calendar preview;
# no static fixture or button text is present in these crops.
ref=Image.open(ROOT/'sorgenti'/'tools'/'ref_calendar_0106.png').convert('RGB')
left=ref.crop((40,460,355,920)).resize((160,256),Image.Resampling.NEAREST)
right=ref.crop((1400,460,1645,920)).resize((160,256),Image.Resampling.NEAREST)
cal=Image.new('RGB',(320,256),'#05235b')
cal.paste(left,(0,0));cal.paste(right,(160,0))
d=ImageDraw.Draw(cal)
# Top manager/team/season bar and a separate date bar.
box(d,(5,2,315,17),'#071a49')
box(d,(211,18,315,30),'#071d4d')
# A translucent centered fixture panel keeps a faint stadium backdrop
# visible without losing readability of the REAL SWOS fixture rows.
d=ImageDraw.Draw(cal,'RGBA')
d.rectangle((5,31,315,164),fill=(3,13,43,227),outline=(135,195,255,255),width=1)
d.line((7,32,313,32),fill=(207,231,255,255))
for y in range(35,156,14):
    if (y//14)%2:d.rectangle((8,y,312,y+12),fill=(9,33,76,235))
d=ImageDraw.Draw(cal)
box(d,(4,166,316,185),'#071c49')
# Three original columns, all hitboxes and callbacks preserved.
for y in (191,210,228):
    for x in (6,110,214):box(d,(x,y,x+99,y+16),'#0a3978')
save(cal,'expanded_calendar_bg')
print('SWOS 0.10.6: three native 320x256 indexed artworks created; Club Business retained unchanged')
