#!/usr/bin/env python3
"""Rebuild 0.10.10 pixel-art screens. Only needed when changing artwork.
The Windows BAT uses the already-generated .raw files and needs no Pillow.
"""
from pathlib import Path
import re
import numpy as np
from PIL import Image, ImageDraw

root=Path(__file__).resolve().parents[2]
assets=root/"data"/"expanded"
tools=root/"sorgenti"/"tools"
text=(root/"sorgenti"/"src"/"swos_globals.c").read_text()
m=re.search(r"uint16_t _menuPalette\[256\] = \{(.*?)\};",text,re.S)
values=[int(v,16) for v in re.findall(r"0x([0-9A-Fa-f]{4})",m.group(1))]
assert len(values)==256
colors=[(((v>>8)&15)*17,((v>>4)&15)*17,(v&15)*17) for v in values]
palimg=Image.new("P",(1,1))
palimg.putpalette(sum((list(c) for c in colors),[]))

ref=Image.open(tools/"ref_career_art_0107.png").convert("RGB")
career=ref.crop((840,0,1200,288)).resize((320,256),Image.Resampling.NEAREST)
d=ImageDraw.Draw(career)
def box(x0,y0,x1,y1,fill="#071e48",stroke="#85bafa"):
    d.rectangle((x0,y0,x1,y1),fill=fill,outline=stroke)
    d.line((x0+2,y0+1,x1-2,y0+1),fill="#c3dfff")
    d.line((x0+2,y1-1,x1-2,y1-1),fill="#1c4d8d")
box(198,103,318,255,"#071c43")
box(200,106,316,120,"#147b19","#e6d45c")
for y in (124,138,152,166,180,194,208):
    box(201,y,315,y+11,"#0b2c5d")
box(201,223,315,238,"#17791a","#c1e38c")
box(201,241,315,255,"#8f2226","#ffb5a4")
career=career.quantize(palette=palimg,dither=Image.Dither.NONE)
(assets/"expanded_career_bg.raw").write_bytes(career.tobytes())
career.convert("RGB").resize((960,768),Image.Resampling.NEAREST).save(
    assets/"expanded_career_bg_preview.png")

for name,cut,end in (
    ("expanded_calendar_bg",270,316),
    ("expanded_unemployed_bg",270,303),
):
    data=np.fromfile(tools/("base_0109_"+name+".raw"),dtype=np.uint8).reshape(256,320)
    mapping=np.arange(320,dtype=int)
    mapping[cut:]=cut+np.rint(
        np.arange(320-cut)*(end-cut)/(319-cut)).astype(int)
    data=data[:,mapping]
    (assets/(name+".raw")).write_bytes(data.tobytes())
    image=Image.fromarray(data,"P")
    image.putpalette(sum((list(c) for c in colors),[]))
    image.convert("RGB").resize((960,768),Image.Resampling.NEAREST).save(
        assets/(name+"_preview.png"))
print("Generated 0.10.10 career, calendar and unemployed artwork")
