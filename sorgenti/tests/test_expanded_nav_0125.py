"""0.10.24: packed callback presence, focus-only STAFF, and native art integrity."""
from pathlib import Path
import zipfile
root=Path(__file__).resolve().parents[2]
source=(root/'sorgenti/src/expanded_economy.c').read_text()
packed=(root/'sorgenti/src/menu_data.c').read_text()
squad=(root/'sorgenti/src/menus.c').read_text()
# The added entry must be packed as a real focusable menu entry, with NO FIRE
# callback; the native menu processor still navigates entries without one.
start=packed.index('CB_MenuFocusOnly staffHover;')
assert start>packed.index('} CB_MenuFocusOnly;')
block=packed[packed.index('/* 11: STAFF tooltip focus target.'):packed.index('MENU_MARK_END_MENU',start)]
assert '{ MENU_CMD_NAV, 9, 9, 9, 10 }' in block
assert 'MENU_CMD_ONSELECT' not in block
assert 'MENU_CMD_ONSELECT_FIRE, Expanded_ToggleFinanceView' in packed
assert 'MENU_CMD_ONSELECT_FIRE, SetExitMenuFlag' in packed
assert 'menu->selectedEntry->ordinal==11' in source
assert '_menuEntries[11].onSelect=0;' in source
assert '_menuEntries[i].downEntry=9;' in source
assert '_menuEntries[9].downEntry=11;' in source
assert '_menuEntries[11].downEntry=10;' in source
assert '"FINANCES / %s %02u",_expandedFinanceSeasonView?"YEAR":"WEEK"' in source
assert 'rect(168,187,33,12,42)' in source
assert 'cb_finance_toggle_label(184,189,gold' in source
assert '"BUS STAFF"' in source and '"STADIUM"' in source
assert '"WAGES","TRAVEL","STAFF","TOTAL"' in source
# The goals-view EXIT is the original packed entry 10, with its original
# SetExitMenuFlag callback. BACK is the original entry 11.
squad_menu=packed[packed.index('Menu_Squad _squadMenu = {'):packed.index('MENU_MARK_END_MENU',packed.index('Menu_Squad _squadMenu = {'))]
assert squad_menu.count('MENU_CMD_ONSELECT_FIRE, SetExitMenuFlag')>=2
assert 'MENU_CMD_ONSELECT_FIRE, SquadOnSelect11' in squad_menu
assert '_menuEntries[11].rightEntry=10;' in source
assert '_menuEntries[10].leftEntry=11;' in source
assert '_menuEntries[6].rightEntry=9;' in source
assert 'ui_move(10,263,236,52,18)' in source
assert 'ui_arrow(19,53,0)' in source and 'ui_arrow(26,53,0)' not in source
assert 'ui_move(16,12,49,14,14)' in source
assert "widescreen='stretch'" in (root/'CONFIGURA_4K.ps1').read_text()
assert 'rect(5,49,84,19,40)' in source
# No change to the approved gameplay assets or original data dimensions.
assert (root/'data/expanded/expanded_squad_bg.raw').stat().st_size==320*256
for name in ('club_business_bg.raw','expanded_squad_bg.raw','expanded_career_bg.raw'):
    assert (root/'data/expanded'/name).stat().st_size==320*256
assert (root/'data/TEAM.008').is_file()
assert (root/'AVVIA_SWOS.bat').is_file()
print('PASS 0.10.25: packed STAFF hover has no callback, finance toggle and squad EXIT retain real callbacks')

# 4K: expanded 320x256 artwork is scaled over the whole 336x272 display;
# the source crop is applied at presentation only (not to the framebuffer).
platform=(root/'sorgenti/src/platform_sdl.c').read_text()
menu=(root/'sorgenti/src/menu_system.c').read_text()
match=(root/'sorgenti/src/game_loop.c').read_text()
assert 'const SDL_Rect menuSrc = { 0, 0, 320, 256 };' in platform
assert 'SDL_RenderCopy(s_renderer, tex, &menuSrc, NULL);' in platform
assert 'SDL_RenderCopyF(s_renderer, tex, &menuSrc, &menuDst);' in platform
assert 's_stretch && s_menu_art_fullscreen' in platform
assert 'menu_art_mouse_coords(&x, &y);' in platform
assert 'Platform_SetMenuArtFullscreen(Expanded_UI_Active() ||' in menu
assert 'Platform_SetMenuArtFullscreen(0)' in match
assert 'cb_native(centerX,y,color,s,0,29)' in source
assert 'V0.10.25' in source
print('PASS 0.10.25: 4K presentation crop, matching mouse hover, match reset, native WEEK/YEAR font')
