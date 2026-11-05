#!/usr/bin/env python3
"""Convert the user's approved welcome artwork and second-lettered sacked art
into the existing SWOS 8-bit 320x256 palette. UI text remains live in C.
"""
from collections import Counter,defaultdict
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[2]
out=root/'data'/'expanded'
colors=defaultdict(Counter)
for name in ('expanded_title_bg','expanded_career_bg','expanded_calendar_bg','expanded_unemployed_bg','expanded_offer_bg'):
    preview=Image.open(out/(name+'_preview.png')).convert('RGB')
    raw=(out/(name+'.raw')).read_bytes()
    assert len(raw)==320*256
    for i,idx in enumerate(raw):
        colors[idx][preview.getpixel(((i%320)*3,(i//320)*3))]+=1
ids=np.array(sorted(colors),dtype=np.uint8)
pal=np.array([colors[int(i)].most_common(1)[0][0] for i in ids],dtype=np.int16)
def convert(im,name,zoom=3):
    a=np.asarray(im.convert('RGB'),dtype=np.int16)
    h,w=a.shape[:2]
    dst=np.empty((h,w),dtype=np.uint8)
    for y in range(h):
        delta=a[y,:,None,:]-pal[None,:,:]
        dist=np.sum(delta.astype(np.int32)**2,axis=2)
        dst[y]=ids[np.argmin(dist,axis=1)]
    (out/(name+'.raw')).write_bytes(dst.tobytes())
    rgb=pal[np.searchsorted(ids,dst)].astype(np.uint8)
    Image.fromarray(rgb,'RGB').resize((w*zoom,h*zoom),Image.Resampling.NEAREST).save(out/(name+'_preview.png'))
    print(name,w,h)
welcome=Image.open(Path(__file__).with_name('ref_welcome_first_0116.png')).convert('RGB')
welcome=welcome.resize((320,256),Image.Resampling.BOX)
convert(welcome,'expanded_welcome_bg')
sacked=Image.open(Path(__file__).with_name('ref_sacked_second_title_0116.png')).convert('RGB')
# The approved second lettering is the brush-stroke SACKED! caption.
# Crop the lower panel, omit its baked-in off-centre button; C renders the
# actual centred native menu button on a solid strip below the illustration.
sacked=sacked.crop((295,650,1170,957)).resize((292,111),Image.Resampling.BOX)
popup=Image.new('RGB',(292,136),(6,8,53))
popup.paste(sacked,(0,0))
d=ImageDraw.Draw(popup)
d.rectangle((0,111,291,135),fill=(4,6,47))
convert(popup,'expanded_sacked_popup')
