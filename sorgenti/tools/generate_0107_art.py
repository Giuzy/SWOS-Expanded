#!/usr/bin/env python3
"""0.10.7: background-only native indexed art, NO baked text/buttons.

The game's original sprite font renders every menu label and value. Club
Business and all five stadium levels are deliberately untouched.
"""
from pathlib import Path
from PIL import Image,ImageDraw
import re
ROOT=Path(__file__).resolve().parents[2]
TOOLS=ROOT/'sorgenti'/'tools'
OUT=ROOT/'data'/'expanded'
source=(ROOT/'sorgenti'/'src'/'swos_globals.c').read_text()
m=re.search(r'uint16_t _menuPalette\[256\] = \{(.*?)\};',source,re.S)
assert m
vals=[int(x,16) for x in re.findall(r'0x([0-9A-Fa-f]{4})',m.group(1))]
assert len(vals)==256
pal=[(((v>>8)&15)*17,((v>>4)&15)*17,(v&15)*17) for v in vals]
idxpal=Image.new('P',(1,1));idxpal.putpalette(sum((list(c) for c in pal),[]))
def save(im,name):
    assert im.size==(320,256)
    indexed=im.convert('RGB').quantize(palette=idxpal,dither=Image.Dither.NONE)
    data=bytes(indexed.getdata());assert len(data)==81920
    (OUT/(name+'.raw')).write_bytes(data)
    indexed.convert('RGB').resize((960,768),Image.Resampling.NEAREST).save(OUT/(name+'_preview.png'))
def box(d,x0,y0,x1,y1,fill='#071e48',stroke='#85bafa'):
    d.rectangle((x0+1,y0+2,x1+1,y1+2),fill='#03102a')
    d.rectangle((x0,y0,x1,y1),fill=fill,outline=stroke)
    d.line((x0+2,y0+1,x1-2,y0+1),fill='#c3dfff')
    d.line((x0+2,y1-1,x1-2,y1-1),fill='#1c4d8d')
# TITLE: undistorted full logo and shouting coach, NO duplicated head or
# baked menu labels. The rightmost original 502px (containing baked buttons)
# is excluded. The 15 real menu entries will sit flush with bottom edge.
ref=Image.open(TOOLS/'ref_title_0106.png').convert('RGB')
title=ref.crop((0,0,1170,936)).resize((320,256),Image.Resampling.NEAREST)
d=ImageDraw.Draw(title,'RGBA')
# Narrow darkened strip only where real buttons will appear; upper sky/logo
# remain untouched. No full-height empty dark-blue column.
d.rectangle((235,101,319,255),fill=(2,12,39,140))
save(title,'expanded_title_bg')
# CAREER: original illustration's LEFT section is cropped in its own 0.77
# aspect ratio to exactly 198x256. Faces and club badge are NOT stretched.
ref=Image.open(TOOLS/'ref_career_art_0107.png').convert('RGB')
scene=ref.crop((829,0,1186,462)).resize((198,256),Image.Resampling.NEAREST)
career=Image.new('RGB',(320,256),'#06152f')
career.paste(scene,(0,0))
d=ImageDraw.Draw(career)
# 116x158 compact panel, roughly half the 0.10.6 form area.
box(d,198,96,318,255,'#071c43')
box(d,200,98,316,111,'#147b19','#e6d45c')
for y in (115,129,143,157,171,185,199):
    box(d,201,y,315,y+11,'#0b2c5d')
box(d,201,215,315,229,'#17791a','#c1e38c')
box(d,201,232,315,243,'#8f2226','#ffb5a4')
box(d,201,246,315,254,'#153b73')
save(career,'expanded_career_bg')
# CALENDAR: original SWOS fixture rows and callbacks remain live. Unlike
# 0.10.6, the original huge header and panel backgrounds are disabled in C,
# so these compact centered panels are not obscured by native blue bands.
ref=Image.open(TOOLS/'ref_calendar_0106.png').convert('RGB')
left=ref.crop((42,460,330,920)).resize((160,256),Image.Resampling.NEAREST)
right=ref.crop((1375,460,1663,920)).resize((160,256),Image.Resampling.NEAREST)
cal=Image.new('RGB',(320,256),'#051e4d')
cal.paste(left,(0,0));cal.paste(right,(160,0))
d=ImageDraw.Draw(cal,'RGBA')
d.rectangle((0,0,319,255),fill=(3,13,37,60))
d=ImageDraw.Draw(cal)
for x0,x1 in ((5,110),(112,225),(227,315)):
    box(d,x0,3,x1,17,'#071b49')
box(d,191,20,316,34,'#09254e')
box(d,5,39,315,183,'#07183d')
# alternating stripes behind REAL fixtures; their text is drawn natively.
for y in range(46,172,14):
    if (y//14)%2: d.rectangle((8,y,312,y+12),fill='#0c2c57')
for y in (195,213,231):
    for x in (11,115,219): box(d,x,y,x+90,y+13,'#0b3b77')
save(cal,'expanded_calendar_bg')
print('0.10.7: title/career/calendar 320x256 native palette assets generated. Business unchanged.')
