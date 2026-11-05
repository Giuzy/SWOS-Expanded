from pathlib import Path
import re
r=Path('/mnt/data/swos_exp06/OpenSWOS_Expanded_0_5')
s=(r/'src/menu_data.c').read_text(); ms=(r/'src/menu_system.c').read_text()
for name,rows in [('expandedNationalityPickerMenu',10),('expandedFamePickerMenu',5),('expandedInfrastructureMenu',8)]:
 a=s.index('} _'+name+' = {')
 b=s.index('MENU_MARK_END_MENU',a)
 x=s[a:b]
 assert x.count('MENU_CMD_ONSELECT_FIRE') >= rows, name
 assert 'MENU_MARK_BYTE_COORDS' in x
 print('PASS:',name,rows,'selectable rows')
a=s.index('Menu_ClubBusiness _clubBusinessMenu = {');b=s.index('Menu_ChooseTactics _chooseTacticsMenu = {',a)
assert '_expandedInfrastructureCaption' not in s[a:b]
assert 'PRESS I - INFRASTRUCTURE' in ms
assert 's_expandedModalReturnLatch' in ms and 'CommonMenuExit();' in ms
assert 'AGE (18-70)' in (r/'src/expanded_career.c').read_text()
assert 'expanded_facility_7' in (r/'src/save_format_json.c').read_text()
print('PASS: original club business restored; I shortcut; popup return redraw; age caption; save fields')
