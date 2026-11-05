from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[2]
sw=(root/'sorgenti/src/SWOS.c').read_text()
renderer=(root/'sorgenti/src/platform_sdl.c').read_text()
bmp=root/'data/expanded/expanded_loading_bg.bmp'
assert bmp.is_file() and bmp.stat().st_size>2_000_000
with Image.open(bmp) as img:
    assert img.size==(1672,941)
    assert img.format=='BMP'
for milestone in (0,12,38,56,63,78,88,100):
    assert f'Platform_ExpandedSplashSet({milestone})' in sw
assert 'if(expandedSplashAvailable)' in sw
assert 'SDL_RenderCopy(s_renderer,s_expandedSplashBg,NULL,NULL)' in renderer
assert 'SDL_RenderFillRect(s_renderer,&fill)' in renderer
assert 'splash_text(x+(w-len*7)/2,bar.y+4,pct)' in renderer
assert 'SDL_LoadBMP("data/expanded/expanded_loading_bg.bmp")' in renderer
assert 'Platform_ExpandedSplashClear()' in sw
print('PASS 0.10.25: separate BMP background, dynamic percent overlay, real phase milestones')
