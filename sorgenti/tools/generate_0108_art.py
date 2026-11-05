#!/usr/bin/env python3
"""0.10.8 - refreshed backgrounds. Retains all original game data and art palette.

- Title: remove old shaded bank; live, larger buttons are rendered in C.
- Career: preserve original proportions while filling the previously empty area.
- Unemployed: same calendar visual language, shorter fixture panel, only live actions.
"""
from pathlib import Path
from PIL import Image,ImageDraw
import re
ROOT=Path(__file__).resolve().parents[2]
TOOLS=ROOT/'sorgenti'/'tools';OUT=ROOT/'data'/'expanded'
src=(ROOT/'sorgenti'/'src'/'swos_globals.c').read_text()
m=re.search(r'uint16_t _menuPalette\[256\] = \{(.*?)\};',src,re.S)
vals=[int(v,16) for v in re.findall(r'0x([0-9A-Fa-f]{4})',m.group(1))]
assert len(vals)==256
pal=[(((v>>8)&15)*17,((v>>4)&15)*17,(v&15)*17) for v in vals]
idx=Image.new('P',(1,1));idx.putpalette(sum((list(c) for c in pal),[]))
def save(im,name):
    assert im.size==(320,256)
    im=im.convert('RGB').quantize(palette=idx,dither=Image.Dither.NONE)
    (OUT/(name+'.raw')).write_bytes(bytes(im.getdata()))
    im.convert('RGB').resize((960,768),Image.Resampling.NEAREST).save(OUT/(name+'_preview.png'))
def box(d,x0,y0,x1,y1,fill='#071e48',stroke='#85bafa'):
    d.rectangle((x0+1,y0+2,x1+1,y1+2),fill='#03102a')
    d.rectangle((x0,y0,x1,y1),fill=fill,outline=stroke)
    d.line((x0+2,y0+1,x1-2,y0+1),fill='#c3dfff')
    d.line((x0+2,y1-1,x1-2,y1-1),fill='#1c4d8d')
# Title no semi-transparent shade from the original button bank.
ref=Image.open(TOOLS/'ref_title_0106.png').convert('RGB')
title=ref.crop((0,0,1170,936)).resize((320,256),Image.Resampling.NEAREST)
save(title,'expanded_title_bg')
# Career illustration: full width from approved new-manager-arrival art.
# 1176x941 aspect 1.25 crop from the right/top quadrant of the collage.
ref=Image.open(TOOLS/'ref_career_art_0107.png').convert('RGB')
career=ref.crop((829,0,1189,288)).resize((320,256),Image.Resampling.NEAREST)
d=ImageDraw.Draw(career,'RGBA')
d.rectangle((196,91,319,255),fill=(2,12,37,192))
d=ImageDraw.Draw(career)
box(d,198,96,318,255,'#071c43')
box(d,200,98,316,111,'#147b19','#e6d45c')
for y in (115,129,143,157,171,185,199): box(d,201,y,315,y+11,'#0b2c5d')
box(d,201,215,315,229,'#17791a','#c1e38c')
box(d,201,232,315,243,'#8f2226','#ffb5a4')
box(d,201,246,315,254,'#153b73')
save(career,'expanded_career_bg')
# Unemployed: SAME soccer background as the calendar, without nine dead
# blue buttons. Only the real FORWARD TIME, BACK and offer slots have boxes.
ref=Image.open(TOOLS/'ref_calendar_0106.png').convert('RGB')
left=ref.crop((42,460,330,920)).resize((160,256),Image.Resampling.NEAREST)
right=ref.crop((1375,460,1663,920)).resize((160,256),Image.Resampling.NEAREST)
unemployed=Image.new('RGB',(320,256),'#051e4d')
unemployed.paste(left,(0,0));unemployed.paste(right,(160,0))
d=ImageDraw.Draw(unemployed,'RGBA')
d.rectangle((0,0,319,255),fill=(3,13,37,60))
d=ImageDraw.Draw(unemployed)
for x0,x1 in ((5,110),(112,225),(227,315)):box(d,x0,3,x1,17,'#071b49')
box(d,191,20,316,34,'#09254e')
# Less dominant than the employed 5..315 x 39..183 panel.
box(d,19,52,301,178,'#07183d')
for y in (86,120,154): d.rectangle((22,y,298,y+15),fill='#0c2c57')
# Only interactive action cells: FORWARD TIME and BACK always; JOB OFFER
# only if an actual offer is ready (the C overlay will skip it otherwise).
box(d,115,195,205,208,'#176b17','#e4d56c')
box(d,115,231,205,244,'#963520','#ffc08d')
# Offer box is only drawn on demand in C, not baked in the art.
save(unemployed,'expanded_unemployed_bg')
print('0.10.8 backgrounds created; business/stadium/calendar art preserved.')
