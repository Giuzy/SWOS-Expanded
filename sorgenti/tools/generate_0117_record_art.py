#!/usr/bin/env python3
"""SWOS Expanded 0.10.17: approved record background and generic club artwork.
Existing artwork is preserved pixel-for-pixel outside sample-text areas.
All visible team names/salary values are painted live by the game engine.
"""
from collections import Counter,defaultdict
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[2]
out=root/'data'/'expanded'
source=Path(__file__).with_name('ref_management_record_0117.png')
assert source.is_file(),source
# Build the exact same SWOS palette index->RGB mapping used by earlier versions.
colors=defaultdict(Counter)
for name in ('expanded_title_bg','expanded_career_bg','expanded_calendar_bg','expanded_unemployed_bg'):
    preview=Image.open(out/(name+'_preview.png')).convert('RGB')
    raw=(out/(name+'.raw')).read_bytes()
    assert len(raw)==320*256
    for i,idx in enumerate(raw):
        colors[idx][preview.getpixel(((i%320)*3,(i//320)*3))]+=1
ids=np.array(sorted(colors),dtype=np.uint8)
pal=np.array([colors[int(i)].most_common(1)[0][0] for i in ids],dtype=np.int16)
def nearest(color):
    a=np.array(color,dtype=np.int16)
    return int(ids[np.argmin(np.sum((pal.astype(np.int32)-a.astype(np.int32))**2,axis=1))])
def indexed(src):
    a=np.asarray(src.convert('RGB'),dtype=np.int16)
    h,w=a.shape[:2]
    dest=np.empty((h,w),dtype=np.uint8)
    for y in range(h):
        diff=a[y,:,None,:]-pal[None,:,:]
        dest[y]=ids[np.argmin(np.sum(diff.astype(np.int32)**2,axis=2),axis=1)]
    return Image.fromarray(dest,'L')
def write(name,im):
    a=np.asarray(im,dtype=np.uint8)
    assert a.shape==(256,320) or (name=='expanded_sacked_popup' and a.shape==(136,292))
    (out/(name+'.raw')).write_bytes(a.tobytes())
    lut=np.zeros((256,3),dtype=np.uint8)
    for i,rgb in zip(ids,pal): lut[i]=rgb
    rgb=Image.fromarray(lut[a],'RGB')
    rgb.resize((im.width*3,im.height*3),Image.Resampling.NEAREST).save(out/(name+'_preview.png'))
    print(name,im.size)
fontfile='/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf'
try: font=ImageFont.truetype(fontfile,9)
except OSError: font=ImageFont.load_default()
# The approved stadium-side manager artwork is the FULL background, no club name.
src=Image.open(source).convert('RGB')
w,h=src.size
crop_w=round(h*320/256)
src=src.crop((0,0,min(crop_w,w),h))
record=src.resize((320,256),Image.Resampling.BOX)
write('expanded_record_bg',indexed(record))
# Remove baked-in example club from the FIRST approved welcome artwork.
# The C renderer already paints the entire dynamic card at x=21..299, y=154..224.
name='expanded_welcome_bg'
im=Image.frombytes('L',(320,256),(out/(name+'.raw')).read_bytes())
d=ImageDraw.Draw(im)
d.rectangle((23,156,296,222),fill=nearest((4,5,48)))
write(name,im)
# Remove baked-in Macclesfield and example salary from the first office art;
# its panel outlines remain in place and are redrawn with actual club data.
name='expanded_offer_bg'
im=Image.frombytes('L',(320,256),(out/(name+'.raw')).read_bytes())
d=ImageDraw.Draw(im)
navy=nearest((8,12,61)); border=nearest((186,192,240))
d.rectangle((82,32,238,82),fill=navy,outline=border,width=1)
for y in (44,56,68):d.line((83,y,237,y),fill=border,width=1)
d.rectangle((57,149,261,186),fill=navy,outline=border,width=1)
# The old tiny shield nameplate was 'SWOS': use the requested generic CLUB.
d.rectangle((278,20,309,32),fill=navy,outline=border,width=1)
d.text((282,22),'CLUB',font=font,fill=nearest((245,245,255)))
write(name,im)
# Remove baked-in 'NOTTS COUNTY' from the stadium sign on the sacking popup.
name='expanded_sacked_popup'
im=Image.frombytes('L',(292,136),(out/(name+'.raw')).read_bytes())
d=ImageDraw.Draw(im)
d.polygon([(234,35),(276,24),(276,63),(234,76)],fill=nearest((5,10,59)))
d.line([(234,35),(276,24),(276,63),(234,76)],fill=border,width=1)
d.text((241,42),'CLUB',font=font,fill=nearest((242,241,250)))
write(name,im)
