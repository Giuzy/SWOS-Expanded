#!/usr/bin/env python3
"""Regenerate SWOS 0.10.12 indexed menu art; requires Pillow."""
from pathlib import Path
from PIL import Image, ImageDraw
import re
root=Path(__file__).resolve().parents[2]
# SWOS menu palette. Keep the original 256-color indexed framebuffer format.
text=(root/'sorgenti/src/swos_globals.c').read_text()
m=re.search(r'uint16_t _menuPalette\[256\] = \{(.*?)\};',text,re.S)
assert m
vals=[int(v,16) for v in re.findall(r'0x([0-9A-Fa-f]{4})',m.group(1))]
assert len(vals)==256
pal=[(((v>>8)&15)*17,((v>>4)&15)*17,(v&15)*17) for v in vals]
idx=Image.new('P',(1,1));idx.putpalette(sum((list(c) for c in pal),[]))
assets=root/'data/expanded';tools=root/'sorgenti/tools'

def save(im,name):
    assert im.size==(320,256)
    indexed=im.convert('RGB').quantize(palette=idx,dither=Image.Dither.NONE)
    b=indexed.tobytes();assert len(b)==81920
    (assets/(name+'.raw')).write_bytes(b)
    indexed.convert('RGB').resize((960,768),Image.Resampling.NEAREST).save(assets/(name+'_preview.png'))

def box(d,x0,y0,x1,y1,fill='#071e48',stroke='#85bafa',shadow=False):
    if shadow: d.rectangle((x0+1,y0+2,x1+1,y1+2),fill='#03102a')
    d.rectangle((x0,y0,x1,y1),fill=fill,outline=stroke)
    d.line((x0+2,y0+1,x1-2,y0+1),fill='#c3dfff')
    d.line((x0+2,y1-1,x1-2,y1-1),fill='#1c4d8d')

# CAREER: preserve the undistorted manager-arrival scene, shift the full
# form 3 pixels right, flush with screen bottom and right. No shadow/grey
# remnant at the lower-right corner. The heading is SOLID green.
ref=Image.open(tools/'ref_career_art_0107.png').convert('RGB')
career=ref.crop((840,0,1200,288)).resize((320,256),Image.Resampling.NEAREST)
d=ImageDraw.Draw(career)
box(d,201,103,319,255,'#071c43')
# solid, full green header, with a clean gold border
box(d,203,106,318,120,'#087e16','#e6d45c')
for y in (124,138,152,166,180,194,208):
    box(d,204,y,317,y+11,'#0b2c5d')
box(d,204,223,317,238,'#17791a','#c1e38c')
box(d,204,241,317,255,'#8f2226','#ffb5a4')
save(career,'expanded_career_bg')

# Stadium backdrop: earlier builds accidentally included the pale grey
# vertical edge of the reference collage. Crop *before* that edge, not
# stretch the grey into the final 320x256 canvas. Recenter panels exactly.
ref=Image.open(tools/'ref_calendar_0106.png').convert('RGB')
left=ref.crop((42,460,330,920)).resize((160,256),Image.Resampling.NEAREST)
right=ref.crop((1320,460,1608,920)).resize((160,256),Image.Resampling.NEAREST)
scene=Image.new('RGB',(320,256),'#051e4d')
scene.paste(left,(0,0));scene.paste(right,(160,0))
# The old source collage has a giant league-table scoreboard in its middle.
# It was hidden by the old 14-row fixture panel, but would be exposed when
# the new adaptive panel shrinks. Reconstruct that central strip using the
# neighboring stadium pixels, blending left/right mirrored edges to avoid
# a hard vertical seam. This preserves the existing pitch pixel art.
original=scene.copy()
src=original.load();dst=scene.load()
for yy in range(32,211):
    alpha=1.0 if yy<=195 else (211-yy)/16.0
    for xx in range(160,216):
        t=(xx-159)/57.0
        l=src[159-(xx-160),yy]
        r=src[216+(215-xx),yy]
        repair=tuple(round((1-t)*l[k]+t*r[k]) for k in range(3))
        old=src[xx,yy]
        dst[xx,yy]=tuple(round(alpha*repair[k]+(1-alpha)*old[k]) for k in range(3))
d=ImageDraw.Draw(scene,'RGBA')
d.rectangle((0,0,319,255),fill=(3,13,37,60))

def calendar():
    im=scene.copy();d=ImageDraw.Draw(im)
    for x0,x1 in ((5,110),(112,225),(227,315)):
        box(d,x0,3,x1,17,'#071b49')
    box(d,191,20,316,34,'#09254e')
    # Panel center is exactly (5+315)/2=160; no right-hand grey gutter.
    # The frame is painted at runtime to fit 1..14 actual fixtures.
    # Leaving the original pitch here avoids a blank strip after week 8.
    for y in (195,213,231):
        for x,w in ((11,90),(115,90),(219,97)):
            box(d,x,y,x+w-1,y+13,'#0b3b77')
    return im

def unemployed():
    im=scene.copy();d=ImageDraw.Draw(im)
    for x0,x1 in ((5,110),(112,225),(227,315)):
        box(d,x0,3,x1,17,'#071b49')
    box(d,191,20,316,34,'#09254e')
    # Panel center is (19+301)/2=160, matching the screen center.
    box(d,19,52,301,178,'#07183d')
    for y in (86,120,154):d.rectangle((22,y,298,y+15),fill='#0c2c57')
    box(d,115,195,205,208,'#176b17','#e4d56c')
    box(d,115,231,205,244,'#963520','#ffc08d')
    return im

save(calendar(),'expanded_calendar_bg')
save(unemployed(),'expanded_unemployed_bg')

