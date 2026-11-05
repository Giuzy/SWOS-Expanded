#!/usr/bin/env python3
"""SWOS 0.10.9: remove the residual dark translucent strip behind career panel."""
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
ref=Image.open(TOOLS/'ref_career_art_0107.png').convert('RGB')
career=ref.crop((829,0,1189,288)).resize((320,256),Image.Resampling.NEAREST)
d=ImageDraw.Draw(career)
def box(x0,y0,x1,y1,fill='#071e48',stroke='#85bafa'):
    # No drop shadow: the illustration meets the outer frame directly.
    d.rectangle((x0,y0,x1,y1),fill=fill,outline=stroke)
    d.line((x0+2,y0+1,x1-2,y0+1),fill='#c3dfff')
    d.line((x0+2,y1-1,x1-2,y1-1),fill='#1c4d8d')
box(198,96,318,255,'#071c43')
box(200,98,316,111,'#147b19','#e6d45c')
for y in (115,129,143,157,171,185,199): box(201,y,315,y+11,'#0b2c5d')
box(201,215,315,229,'#17791a','#c1e38c')
box(201,232,315,243,'#8f2226','#ffb5a4')
box(201,246,315,254,'#153b73')
im=career.quantize(palette=idx,dither=Image.Dither.NONE)
(OUT/'expanded_career_bg.raw').write_bytes(bytes(im.getdata()))
im.convert('RGB').resize((960,768),Image.Resampling.NEAREST).save(OUT/'expanded_career_bg_preview.png')
print('Career 0.10.9: seamless panel, 320x256 indexed')
