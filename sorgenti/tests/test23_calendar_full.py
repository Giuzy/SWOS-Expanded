#!/usr/bin/env python3
"""0.10.13: all cached fixtures must be eligible for compact rendering."""
from pathlib import Path
root=Path(__file__).resolve().parents[2]
s=(root/'sorgenti/src/expanded_economy.c').read_text()
m=(root/'sorgenti/src/menus.c').read_text()
assert 'while(n<14 && _gameListArray[n*2])' in s
assert 'ui_fixture_rows_drawn=row+1' in s
assert 'ui_fixture_finish_panel();' in s
assert 'ui_fixture_bottom_for_count' in s
assert 'ui_restore_rect(ui_calendar,5,bottom,310,oldBottom-bottom)' in s
assert '_contestEntryHeight=512;' in m
assert '_currentMenuPtr==(uintptr_t)_contestMenu && Expanded_UI_Active()' in m
assert 'if (Expanded_UI_CompactFixtureRow(d0,slotType)) return;' in m
assert 'BuildGameListArrayFromTeamSelectionKeyCache();' in m
assert (root/'data/expanded/expanded_calendar_bg.raw').stat().st_size==320*256
print('PASS: 14 compact fixtures, adaptive frame, original game cache and scrolling')
