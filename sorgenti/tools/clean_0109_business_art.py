#!/usr/bin/env python3
"""0.10.9: remove board portraits and fan confetti under dynamic pixel faces.
The other club business artwork, buttons and five stadiums stay untouched.
"""
from pathlib import Path
from PIL import Image
import re
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'data/expanded'
p=OUT/'club_business_bg.raw'
raw=bytearray(p.read_bytes());assert len(raw)==320*256
# Panel interiors use original SWOS navy (palette index 40).
for x0,x1,y0,y1 in ((12,77,44,74),(244,314,44,75)):
    for y in range(y0,y1): raw[y*320+x0:y*320+x1]=bytes([40])*(x1-x0)
p.write_bytes(raw)
s=(ROOT/'sorgenti/src/swos_globals.c').read_text()
m=re.search(r'uint16_t _menuPalette\[256\] = \{(.*?)\};',s,re.S)
vals=[int(v,16) for v in re.findall(r'0x([0-9A-Fa-f]{4})',m.group(1))]
assert len(vals)==256
pal=sum((list((((v>>8)&15)*17,((v>>4)&15)*17,(v&15)*17)) for v in vals),[])
im=Image.frombytes('P',(320,256),bytes(raw));im.putpalette(pal)
im.convert('RGB').resize((960,768),Image.Resampling.NEAREST).save(OUT/'club_business_bg_preview.png')
print('0.10.9 club dashboard: portraits/confetti removed, face areas clean')
