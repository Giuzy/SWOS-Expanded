#!/usr/bin/env bash
# SWOS Expanded 0.10.71. Idempotent source overlay for approved HD loader.
# Does not touch gameplay, saves, user menus or historical RAW images.
set -Eeuo pipefail
src="${1:-.}"
f="$src/src/platform_sdl.c"
[[ -f "$f" && -s "$src/include/expanded_loading_01071.h" ]] || {
  echo '[loading71] missing platform SDL source or helper header' >&2;exit 1;
}
if grep -Fq 'ExpandedLoading71_Init(s_renderer)' "$f";then
 echo '[loading71] already integrated'; exit 0
fi
for anchor in '#include <string.h>' 'static void expanded_splash_draw(void)' 'static SDL_Texture *expanded_splash_layer(const char *path)' 'int Platform_ExpandedSplashSet(int percent)' '/* Temporary screenshot aid:' 'if (s_expandedSplashBg) {SDL_DestroyTexture(s_expandedSplashBg);s_expandedSplashBg=NULL;}';do
  grep -Fq "$anchor" "$f" || { echo "[loading71] cannot patch, missing: $anchor" >&2;exit 1; }
done
tmp="$(mktemp "${f}.01071.XXXXXXXX")"
trap 'rm -f "$tmp"' EXIT
awk '
  /#include <string.h>/ {
    print;
    print "#include \"expanded_loading_01071.h\"";
    next
  }
  /^static void expanded_splash_draw\(void\)/ {
    print "static void expanded_splash_draw(void)";
    print "{";
    print "    ExpandedLoading71_Draw(s_renderer,s_expandedSplashBg,s_expandedSplashPct);";
    print "}";
    mode=1; next
  }
  mode==1 {
    if(index($0,"static SDL_Texture *expanded_splash_layer(const char *path)")>0){ mode=0; print; }
    next
  }
  /^int Platform_ExpandedSplashSet\(int percent\)[[:space:]]*$/ {
    print "int Platform_ExpandedSplashSet(int percent)";
    print "{";
    print "    if(!s_renderer)return 0;";
    print "    if(!s_expandedSplashBg){";
    print "        s_expandedSplashBg=ExpandedLoading71_Init(s_renderer);";
    print "        if(!s_expandedSplashBg){";
    print "            fprintf(stderr,\"[loading71] approved PNG background not available\\n\");";
    print "#ifndef SWOS_WASM_BUILD";
    print "            s_startup_splash_unavailable=1;";
    print "#endif";
    print "            Platform_RenderFrame();";
    print "            return 0;";
    print "        }";
    print "    }";
    print "    if(percent<0)percent=0;";
    print "    if(percent>100)percent=100;";
    print "    if(percent==0)s_expandedSplashStartMs=SDL_GetTicks();";
    print "    s_expandedSplashPct=percent;";
    print "    s_expandedSplashActive=1;";
    print "    Platform_RenderFrame();";
    print "    return 1;";
    print "}";
    mode=2; next
  }
  mode==2 {
    if(index($0,"/* Temporary screenshot aid:")>0){ mode=0; print; }
    next
  }
  {
    print;
    if(index($0,"if (s_expandedSplashBg) {SDL_DestroyTexture(s_expandedSplashBg);s_expandedSplashBg=NULL;}")>0)
      print "    ExpandedLoading71_Destroy();";
  }
  END {if(mode!=0){print "[loading71] unterminated patch" > "/dev/stderr"; exit 4}}
' "$f" > "$tmp"
# Guard against losing source between the forward declaration and definition.
grep -Fq 'int Platform_ExpandedSplashSet(int percent);' "$tmp" || {
 echo '[loading71] forward declaration lost during patch' >&2;exit 1;
}
grep -Fq 's_expandedSplashStartMs' "$tmp" || {
 echo '[loading71] startup state lost during patch' >&2;exit 1;
}
for anchor in 'ExpandedLoading71_Draw(s_renderer' 'ExpandedLoading71_Init(s_renderer)' 'ExpandedLoading71_Destroy();' '#include "expanded_loading_01071.h"';do
 grep -Fq "$anchor" "$tmp" || { echo "[loading71] integration check failed: $anchor" >&2;exit 1; }
done
chmod --reference="$f" "$tmp"
mv -f "$tmp" "$f"
trap - EXIT
echo '[loading71] approved HD PNG and dynamic loading UI integrated.'
