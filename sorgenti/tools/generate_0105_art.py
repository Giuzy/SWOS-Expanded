#!/usr/bin/env python3
from pathlib import Path
from PIL import Image,ImageDraw,ImageEnhance,ImageFilter
import importlib.util,random
ROOT=Path(__file__).resolve().parents[2]
TOOLS=ROOT/'sorgenti'/'tools'
# import helpers from previous generation script, which regenerates base and stadium files
spec=importlib.util.spec_from_file_location('clubart',TOOLS/'generate_club_business_art.py')
club=importlib.util.module_from_spec(spec);spec.loader.exec_module(club)
W,H=320,256
OUT=ROOT/'data'/'expanded';OUT.mkdir(exist_ok=True,parents=True)
P=club.PAL
# Existing club artwork has exact SWOS palette and 5-tier stadiums.
def convert(img,filename):
 rgb=img.convert('RGB')
 pal=Image.new('P',(1,1));pal.putpalette(sum((list(c) for c in P),[]))
 idx=rgb.quantize(palette=pal,dither=Image.Dither.NONE)
 raw=bytes(idx.getdata());assert len(raw)==81920
 (OUT/filename).write_bytes(raw)
 # show palette indexed preview at 3x, with no interpolation
 idx.convert('RGB').resize((960,768),Image.Resampling.NEAREST).save(OUT/(filename.replace('.raw','_preview.png')))

def box(d,xy,fill='#071a4c',line='#91c9ff'):
 club.box(d,xy,fill,stroke=line,bright='#d3eaff')
def btn(d,x,y,w,h,color='#0a3977'):
 box(d,(x,y,x+w,y+h),color)
 d.line((x+3,y+2,x+w-3,y+2),fill='#79c7ff')
def shade(img,factor):return ImageEnhance.Brightness(img).enhance(factor)

def cover(im,w,h):
 iw,ih=im.size; ratio=max(w/iw,h/ih)
 tmp=im.resize((round(iw*ratio),round(ih*ratio)),Image.Resampling.NEAREST)
 x=(tmp.width-w)//2;y=(tmp.height-h)//2
 return tmp.crop((x,y,x+w,y+h))

# Title: actual SWOS logo + coach/player illustration, no static baked buttons.
title=Image.open(TOOLS/'ref_title_0105.png').convert('RGB')
bg=cover(title.crop((0,80,1672,941)),320,256)
bg=shade(bg,.65)
# hero art from the left part, not the buttons
hero=title.crop((0,300,475,941)).resize((175,160),Image.Resampling.NEAREST)
bg.paste(hero,(0,96))
d=ImageDraw.Draw(bg)
# Logo on a navy plaque, upper title retains its entire top edge
box(d,(20,1,300,94),'#031239',line='#d2a23c')
logo=title.crop((430,0,1175,415)).resize((270,90),Image.Resampling.NEAREST)
bg.paste(logo,(25,3))
d=ImageDraw.Draw(bg)
# menu bank right, 2x8 clickable buttons (real SWOS menu functions)
box(d,(170,96,318,254),'#06183d')
for r in range(8):
 for c in range(2):
  x=173+c*72;y=99+r*19
  btn(d,x,y,69,18,'#125092' if (r+c)%2==0 else '#0a3978')
convert(bg,'expanded_title_bg.raw')

# New career: left half photo arrival, right half full interactive fields
comp=Image.open(TOOLS/'ref_career_calendar_0105.png').convert('RGB')
arr=cover(comp.crop((0,0,930,397)),171,256)
bg=Image.new('RGB',(W,H),'#03132f');bg.paste(arr,(0,0))
d=ImageDraw.Draw(bg)
box(d,(173,3,317,29),'#176d13',line='#f2c338')
box(d,(173,34,317,246),'#071e4c')
for y in [39,59,79,99,119,139,159]:
 btn(d,177,y,136,18,'#0b2c61')
btn(d,177,184,136,23,'#167916')
btn(d,177,210,136,17,'#8d1519')
btn(d,177,230,136,13,'#194a7b')
convert(bg,'expanded_career_bg.raw')

# Calendar: stadium action is a lively backdrop, while fixtures remain readable.
cal=Image.new('RGB',(W,H),'#061330')
left=cover(comp.crop((0,425,530,941)),62,256)
cal.paste(left,(0,0))
# top sky / fans behind the header
sky=cover(comp.crop((0,410,1672,510)),258,50)
cal.paste(shade(sky,.6),(62,0))
d=ImageDraw.Draw(cal)
box(d,(63,3,317,23),'#071d4a')
box(d,(62,27,316,43),'#0b2d59')
box(d,(62,48,316,184),'#031133')
# row stripes and fine pixel border for fixture panel
for yy in range(52,177,13):
 if (yy//13)%2==0: d.rectangle((65,yy,313,yy+11),fill='#071b48')
# bottom bank: 3 columns x 3 rows, matching native calendar positions
box(d,(3,188,317,245),'#06153d')
for y in [191,210,228]:
 for x in [6,110,214]: btn(d,x,y,99,16,'#0b427a')
convert(cal,'expanded_calendar_bg.raw')

# Club Business v2: no bottom stripe; BACK top-left; larger finance and cards.
bg=Image.new('RGB',(W,H),'#020a28');d=ImageDraw.Draw(bg)
rng=random.Random(1045)
for i in range(15):
 x=rng.randrange(-20,320);y=rng.randrange(-20,256)
 d.line([(x,y),(x+20,y+17),(x+2,y+39)],fill='#0a285a',width=1)
box(d,(5,3,52,20),'#b61616',line='#ffe7b2')
# green header
d.polygon([(96,2),(225,2),(235,21),(85,21)],fill='#d9a12d')
d.polygon([(100,4),(222,4),(231,19),(90,19)],fill='#277f0c')
d.line((100,5,221,5),fill='#a5e861',width=1)
box(d,(242,3,315,20),'#092858')
box(d,(4,24,79,80))
box(d,(240,24,316,80))
box(d,(4,83,79,130))
box(d,(240,83,316,130))
# board figures
for xx in (15,35,55):
 d.rectangle((xx,57,xx+10,66),fill='#264a87',outline='#050b26')
 d.polygon([(xx+3,57),(xx+7,57),(xx+5,62)],fill='#ffffff')
 d.line((xx+5,59,xx+5,64),fill='#e52c3a')
 d.rectangle((xx+2,48,xx+8,56),fill='#e9ae82',outline='#211b26')
 d.rectangle((xx+1,46,xx+9,48),fill='#342724')
club.crowd(d,244,49,67,18,1997)
# stadium canvas is overlaid in C based on level, preserving dynamic tier art
box(d,(82,24,238,130),'#0b2b49')
# stadium info progress
for i in range(5): d.rectangle((10+i*13,119,20+i*13,125),fill='#1c336b',outline='#79a4de')
btn(d,243,108,70,18,'#277b0c')
# seven large facility buttons
for i in range(7):
 x=5+i*44
 box(d,(x,133,x+42,181),'#0a275b')
 d.rectangle((x+2,135,x+40,156),fill='#247a14')
 d.line((x+3,136,x+39,136),fill='#64bb37')
 club.icon(d,i,x+5,136)
 for j in range(5):
  xx=x+4+j*7
  d.rectangle((xx,171,xx+5,177),fill='#223866',outline='#8da9d7')
# finances wider, balance
box(d,(4,185,206,253),'#081c4c')
box(d,(210,185,316,253),'#081c4c')
d.line((8,198,201,198),fill='#7ca6df')
d.line((214,198,312,198),fill='#7ca6df')
d.line((110,201,110,248),fill='#7ca6df')
d.rectangle((215,202,311,218),fill='#15550e',outline='#83da56')
convert(bg,'club_business_bg.raw')
print('Generated 4 native 320x256 indexed artworks, retained 5 stadium tiers.')
