#!/usr/bin/env bash
# SWOS Expanded 0.10.65 - deterministic, idempotent Title PNG/font integration.
# Runs on user's copy; only touches expanded_title_overlay.c after checking anchors.
set -Eeuo pipefail
src="${1:-.}"
f="$src/src/expanded_title_overlay.c"
[[ -f "$f" ]] || { echo "ERRORE: manca $f" >&2; exit 1; }
if grep -Fq 'Expanded_TitleUI65_DrawButtonText' "$f"; then
    echo '[title-ui] 0.10.65 gia applicata.'
    exit 0
fi
for anchor in '#include <string.h>' 'assets/title/buttons/career.png' 'assets/title/buttons/continue_career.png' 'SDL_RenderCopyF(renderer,sprite,NULL,&dest);' 's_background=s_logo=s_continue_career=s_continue_focus=NULL;' 'if(!renderer) return 0;'; do
  if ! grep -Fq "$anchor" "$f"; then
    echo "ERRORE: Title sorgente non riconosciuto, patch annullata: $anchor" >&2
    exit 1
  fi
done
[[ -s "$src/include/expanded_title_ui_01065.h" ]] || { echo 'ERRORE: helper Title PNG/font mancante' >&2; exit 1; }
tmp="$(mktemp "${f}.01065.XXXXXXXX")"
trap 'rm -f "$tmp"' EXIT
awk '
{
  if (index($0,"assets/title/buttons/career.png")>0) gsub(/assets\/title\/buttons\/career\.png/,"assets/title/buttons/title_primary_blank.png");
  if (index($0,"assets/title/buttons/hall_of_fame.png")>0) gsub(/assets\/title\/buttons\/hall_of_fame\.png/,"assets/title/buttons/title_secondary_blank.png");
  if (index($0,"assets/title/buttons/classic_modes.png")>0) gsub(/assets\/title\/buttons\/classic_modes\.png/,"assets/title/buttons/title_secondary_blank.png");
  if (index($0,"assets/title/buttons/options.png")>0) gsub(/assets\/title\/buttons\/options\.png/,"assets/title/buttons/title_secondary_blank.png");
  if (index($0,"assets/title/buttons/exit_game.png")>0) gsub(/assets\/title\/buttons\/exit_game\.png/,"assets/title/buttons/title_secondary_blank.png");
  if (index($0,"assets/title/buttons/continue_career.png")>0) gsub(/assets\/title\/buttons\/continue_career\.png/,"assets/title/buttons/title_primary_blank.png");
  if (index($0,"if(strcmp(path,s_continue_career_path)==0)")>0){
      print "    /* Same blank source as CAREER; recolour only the separate continuation copy. */";
      print "    if(strcmp(path,s_continue_career_path)==0 && focus_radius==5)";
      next;
  }
  if (index($0,"&s_button_focus[i],8);")>0) gsub(/,8\);/,",4);");
  if (index($0,"&s_continue_focus,16);")>0) gsub(/,16\);/,",5);");
  print;
  if (index($0,"#include <string.h>")>0) print "#include \"expanded_title_ui_01065.h\"";
  if (index($0,"if(!renderer) return 0;")>0) print "    Expanded_TitleUI65_Init(renderer);";
  if (index($0,"s_background=s_logo=s_continue_career=s_continue_focus=NULL;")>0) print "    Expanded_TitleUI65_Destroy();";
  if (index($0,"SDL_RenderCopyF(renderer,sprite,NULL,&dest);")>0) {
      print "                if(!Expanded_TitleUI65_DrawButtonText(renderer,";
      print "                    continue_active ? \"CONTINUE CAREER\" : s_main_labels[i],";
      print "                    dest,i==0,continue_active,i==sel,sx,sy))";
      print "                    pixel_text(renderer,continue_active ? \"CONTINUE CAREER\" : s_main_labels[i],";
      print "                        box.x+box.w/2,box.y+box.h/2-16+sink,4,";
      print "                        i==sel?(SDL_Color){255,226,72,255}:(SDL_Color){250,250,255,255},sx,sy);";
  }
  if (index($0,"char label[80];")>0) print "        if(level==0)Expanded_TitleUI65_DrawLegend(renderer,sx,sy);";
}
' "$f" > "$tmp"
for anchor in 'Expanded_TitleUI65_DrawButtonText(renderer' 'Expanded_TitleUI65_Init(renderer);' 'Expanded_TitleUI65_DrawLegend(renderer,sx,sy)' 'title_secondary_blank.png' 'title_primary_blank.png'; do
  if ! grep -Fq "$anchor" "$tmp"; then echo "ERRORE: Title patch incompleta: $anchor" >&2; exit 1; fi
done
chmod --reference="$f" "$tmp"
mv -f "$tmp" "$f"
trap - EXIT
echo '[title-ui] 0.10.65: nuovi pulsanti blank + font NEW CAREER + legenda A integrati.'
