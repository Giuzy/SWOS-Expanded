#!/usr/bin/env python3
"""0.10.15: render the *first* approved office/contract illustration full-screen.

The source is the first generated club-offer concept (contract on table,
chairman at right, window and stadium at left). Its whole composition is
mapped into the 320x256 SWOS indexed framebuffer, without black/blue sidebars
or the previous large solid-blue lower half. Live SWOS code draws the dynamic
club/salary and functional menu entries on top of its illustrative panels.
"""
from collections import Counter, defaultdict
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / 'data' / 'expanded'
SRC = Path(__file__).with_name('ref_offer_first_0115.png')
assert SRC.exists(), SRC
src = Image.open(SRC).convert('RGB')
# Full bleed.  BOX antialiasing retains the first illustration's fine pixel
# details when shrinking from 1472x1056 to the actual game resolution.
im = src.resize((320,256), Image.Resampling.BOX)
# The concept's badge says MACCLESFIELD; offers are for arbitrary clubs.
# Keep the shield/lion, but make the tiny nameplate neutral, not misleading.
d = ImageDraw.Draw(im)
d.rectangle((278,20,309,32), fill=(10,18,79), outline=(188,194,246), width=1)
try:
    font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf',7)
except OSError:
    font=ImageFont.load_default()
d.text((286,22),'SWOS',font=font,fill=(242,241,250),stroke_width=0)
# Build the game's native indexed palette from known art/raw pairs. This
# preserves the SWOS palette and requires no new graphics engine or DLL.
colors=defaultdict(Counter)
for name in ('expanded_title_bg','expanded_career_bg','expanded_calendar_bg','expanded_unemployed_bg'):
    preview=Image.open(BASE/(name+'_preview.png')).convert('RGB')
    raw=(BASE/(name+'.raw')).read_bytes()
    assert len(raw)==320*256
    for i, index in enumerate(raw):
        colors[index][preview.getpixel(((i%320)*3,(i//320)*3))]+=1
ids=np.array(sorted(colors),dtype=np.uint8)
pal=np.array([colors[int(i)].most_common(1)[0][0] for i in ids],dtype=np.int16)
pixels=np.asarray(im,dtype=np.int16)
out=np.empty((256,320),dtype=np.uint8)
for y in range(256):
    delta=pixels[y,:,None,:]-pal[None,:,:]
    dist=np.sum(delta.astype(np.int32)**2,axis=2)
    out[y]=ids[np.argmin(dist,axis=1)]
(BASE/'expanded_offer_bg.raw').write_bytes(out.tobytes())
indexed_rgb=pal[np.searchsorted(ids,out)].astype(np.uint8)
Image.fromarray(indexed_rgb,'RGB').resize((960,768),Image.Resampling.NEAREST).save(
    BASE/'expanded_offer_bg_preview.png')
print('0.10.15: full bleed first office scene, 320x256 indexed pixels')
