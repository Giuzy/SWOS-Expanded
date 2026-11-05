#!/usr/bin/env python3
from pathlib import Path
import numpy as np
root=Path(__file__).resolve().parents[2]
asset=root/'data/expanded'
for name in ('expanded_calendar_bg','expanded_unemployed_bg'):
    a=np.fromfile(asset/(name+'.raw'),dtype=np.uint8).reshape(256,320)
    # No pale-grey collage margin; the background extends across all 320px.
    assert (a[180:256,310:320]==1).mean()<0.05, name
# Panels centered on the 320px framebuffer, without changing button callbacks.
src=(root/'sorgenti/src/expanded_economy.c').read_text()
assert 'ui_move(fields[i],204,rows[i],114,12)' in src
assert 'ui_move(10,204,223,114,16)' in src
assert 'ui_move(11,204,241,114,15)' in src
assert 'cb_center_narrow(278,87,white,"COSTS");' in src
assert 'cb_center_narrow(278,114,white,level==5?"MAXIMUM":"UPGRADE");' in src
assert 'ui_move(18,9,47,305,133)' in src
print('PASS: 0.10.11 artwork gutters, centered panels, career fields, business label')
