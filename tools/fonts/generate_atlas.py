#!/usr/bin/env python3
"""Genera atlanti font bitmap RGBA + metriche JSON da un TTF/OTF.

Solo strumento di sviluppo: SWOS_Expanded.exe carica PNG/JSON generati,
NON esegue Python e NON richiede SDL_ttf. Le dimensioni sono pixel nativi
nel canvas dell'atlante; non riscalare i glifi con filtro lineare.

Dipendenza di sviluppo: python -m pip install Pillow
Esempio:
  python tools/fonts/generate_atlas.py --ttf "C:/Fonts/IlFontApprovato.ttf" \
      --output-dir assets/fonts/expanded --sizes 24,32,42

Nessun file font TTF/OTF è distribuito da questo strumento.
"""
import argparse
import json
import math
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

# Latin basic + Latin-1 accents commonly used in Italian/European football,
# typography and currency. Add specific codepoints when translations require them.
CHARSET = ''.join(chr(i) for i in range(32, 127)) + (
    'ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞß'
    'àáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ'
    'ĀāĂăĄąĆćČčĐđĎďĘęĚěĞğİıŁłŃńŇňŐőŔŕŘřŚśŞşŠšŤťŮůŰűŹźŻżŽž'
    '€£¥₺$¢©®°±×÷–—…•‘’“”«»№%+-*/=<>!?@#&:;.,()[]{}|\\_'
)
CHARS = sorted(set(CHARSET), key=ord)


def build(ttf_path, out, size, max_width=1024):
    font = ImageFont.truetype(str(ttf_path), size)
    if hasattr(font, 'getmetrics'):
        ascent, descent = font.getmetrics()
    else:
        ascent, descent = size, math.ceil(size / 4)
    pad = 2
    cells = []
    for ch in CHARS:
        bbox = font.getbbox(ch, anchor='ls')
        if bbox is None:
            bbox = (0, 0, 0, 0)
        left, top, right, bottom = bbox
        w, h = max(0, right-left), max(0, bottom-top)
        advance = float(font.getlength(ch))
        # Space and nonprinting characters need no pixels.
        if ch == ' ' or (w == 0 or h == 0):
            w = h = 0
        cells.append((ch, left, top, w, h, advance))
    x = pad; y = pad; line_h = 0; placements = []; used_w = 0
    for ch,left,top,w,h,advance in cells:
        cw = max(1,w)+pad*2; chh = max(1,h)+pad*2
        if x+cw>max_width and x>pad:
            y += line_h; x=pad; line_h=0
        placements.append((ch,x+pad,y+pad,left,top,w,h,advance))
        x+=cw; line_h=max(line_h,chh); used_w=max(used_w,x)
    atlas_h=y+line_h+pad
    atlas=Image.new('RGBA',(min(max_width,max(16,used_w)),max(16,atlas_h)),(0,0,0,0))
    draw=ImageDraw.Draw(atlas)
    glyphs={}
    for ch,gx,gy,left,top,w,h,advance in placements:
        if w and h:
            draw.text((gx-left,gy-top),ch,font=font,anchor='ls',fill=(255,255,255,255))
        glyphs[str(ord(ch))]={'x':gx,'y':gy,'w':w,'h':h,
                                'bearingX':left,'bearingY':top,'advance':round(advance,3)}
    prefix=f'expanded_ui_{size}'
    atlas.save(out/f'{prefix}.png', optimize=True)
    metrics={'schema':'swos-expanded-font-atlas-v1',
             'atlas':f'{prefix}.png','family':'PROVISIONAL: supply approved TTF before production',
             'size_px':size,'ascent':ascent,'descent':descent,'line_height':ascent+descent,
             'source_font_filename':ttf_path.name,
             'renderer_guidance':'SDL2 render glyph rects with pixel-perfect integer positioning; draw at (penX + bearingX, baselineY + bearingY); advance penX by advance; no kerning; use white glyphs with SDL color modulation for text color; nearest sampling.',
             'glyphs':glyphs}
    (out/f'{prefix}.json').write_text(json.dumps(metrics,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    return atlas, metrics


def preview(ttf, sizes, dest):
    font_sizes=[ImageFont.truetype(str(ttf),sz) for sz in sizes]
    width=1300; height=420
    image=Image.new('RGB',(width,height),(12,34,73))
    d=ImageDraw.Draw(image)
    examples=[('NEW CAREER / SWOS EXPANDED',0),
              ('TITLE   MR     NAME   JOHN     SURNAME   SMITH',1),
              ('NATION   ENGLAND      ROLE   PL/COACH',1),
              ('AGE 70   FAME ESTABLISHED   € 1.250.000',1),
              ('START UNEMPLOYED        CHOOSE TEAM        BACK',2)]
    y=25
    for line,idx in examples:
        f=font_sizes[idx]
        d.text((35,y),line,font=f,fill='white',stroke_width=1,stroke_fill=(0,0,0))
        y+=76
    image.save(dest, optimize=True)


def main():
    p=argparse.ArgumentParser()
    p.add_argument('--ttf',type=Path,required=True,help='Local font file, TTF/OTF (not copied)')
    p.add_argument('--output-dir',type=Path,required=True)
    p.add_argument('--sizes',default='24,32,42')
    p.add_argument('--preview',type=Path,default=None)
    a=p.parse_args()
    sizes=[int(v) for v in a.sizes.split(',')]
    if not a.ttf.is_file(): p.error('TTF/OTF not found')
    a.output_dir.mkdir(parents=True,exist_ok=True)
    for size in sizes:
        img,m=build(a.ttf,a.output_dir,size)
        print(size,img.size,len(m['glyphs']),'glyphs')
    if a.preview:
        a.preview.parent.mkdir(parents=True,exist_ok=True)
        preview(a.ttf,sizes,a.preview)

if __name__=='__main__': main()
