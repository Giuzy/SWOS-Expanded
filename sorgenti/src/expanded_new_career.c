/* NEW CAREER HD. Presentation/setup state only; career/save engine unchanged.
 * 0.10.68: +0.5 logical px for NEW CAREER C only; audio stays disabled. */
#ifdef SWOS_SDL_BUILD
#include "expanded_new_career.h"
#include "expanded_png.h"
#include "expanded_club_font.h" /* fallback only: runtime uses existing bitmap atlas */
#include "mjson.h"
#include "menu_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern uintptr_t _currentMenuPtr;
extern uint8_t _currentMenu[],_careerMenu[],_expandedNationalityPickerMenu[],_expandedFamePickerMenu[];
extern MenuEntry _menuEntries[];
extern uint16_t _currentKey,_controlMask,_fireResetFlag,_deadVar02;
extern volatile uint16_t _fire,_shortFire;
extern int16_t _menuControlsDirection;
extern uint8_t _expandedManagerStartAge,_expandedManagerFame,_plNationality;
extern uint16_t _coachOrPlayer;
extern char _expandedAgeText[],_expandedManagerFameText[];
extern void Expanded_SetAgeText(void);
extern void ChooseTeamAndStartCareer(void);
extern void ChangeSex(MenuEntry*),SetName(MenuEntry*),SetSurname(MenuEntry*),ChangeNationality(MenuEntry*),CoachOrPlayerCoach(MenuEntry*);
extern void Expanded_EditManagerAge(MenuEntry*),Expanded_ChangeManagerFame(void*);
extern void CallMenuOnSelect(void *,void *);
extern int Platform_NewCareerMouseState(int*,int*,int*,int*);
static SDL_atomic_t s_back;
static int s_selected,s_unemployed=0,s_previous_fire=1,s_press,s_action=-1,s_action_dir;
/* 50 Hz ticks: separate directional-rate limiters; AGE stays responsive. */
static int s_nav_cooldown,s_arrow_cooldown;
/* Focus moves at 12 virtual pixels/frame between form fields (66 px apart). */
static float s_focus_field_y=202.f;
static const int ord[]={3,4,5,7,9,15,13,10,11};
typedef struct {SDL_Rect rect; int advance,bearingX,bearingY;} CareerGlyph;
typedef struct {SDL_Texture *texture;CareerGlyph glyph[128];} CareerFont;
static CareerFont s_fonts[3]; /* form, button, title */
static SDL_Texture *s_focus_toggle,*s_focus_start,*s_flags,*s_legend_ab;
static SDL_Texture *s_bg,*s_panel,*s_toggle[2],*s_start[2],*s_check[2];
static int s_loaded;
/* Exact nationality code order from the active s_natNames in menu_data.c.
 * Not TEAM.nnn indexes! Historical 96/97 names have their own atlas tiles. */
static const uint8_t s_flag_codes[]={
 8,31,36,24,46,0,1,2,38,3,4,5,7,40,9,10,11,12,13,14,
 23,15,16,17,18,19,20,21,22,25,26,27,28,29,32,30,45,39,33,6,
 34,35,56,52,68,69,70,71,72,73,74,79,75,76,77,80,88,81,120,118,
 119,148,149,153,154,155,37,255
};
int Expanded_NewCareer_Active(void){return _currentMenuPtr==(uintptr_t)_careerMenu;}
int Expanded_NewCareer_InputActive(void){
 return Expanded_NewCareer_Active() || _currentMenuPtr==(uintptr_t)_expandedNationalityPickerMenu || _currentMenuPtr==(uintptr_t)_expandedFamePickerMenu;
}
void Expanded_NewCareer_Back(void){SDL_AtomicSet(&s_back,1);}
int Expanded_NewCareer_TakeBack(void){return SDL_AtomicSet(&s_back,0);}
int Expanded_NewCareer_Unemployed(void){return s_unemployed;}
void Expanded_NewCareer_Reset(void){
 s_selected=0;s_unemployed=0;s_previous_fire=1;s_press=0;s_action=-1;
 s_nav_cooldown=0;s_arrow_cooldown=0;s_focus_field_y=202.f;SDL_AtomicSet(&s_back,0);
 fprintf(stderr,"[new-career] setup reset: unemployed=OFF; fame independent\n");
}
static int ready(void){return _expandedManagerStartAge>=18 && _expandedManagerStartAge<=70 && _expandedManagerFame<5 && !_deadVar02;}
static SDL_Texture *texture(SDL_Renderer*r,const char*path,int colour){
 unsigned w=0,h=0;unsigned char*p=Expanded_PNG_LoadRGBA(path,&w,&h);
 if(!p){fprintf(stderr,"[new-career] missing/invalid PNG: %s\n",path);return NULL;}
 if(colour)for(size_t i=0;i<(size_t)w*h;i++){
  unsigned char*c=p+4*i;unsigned b=c[2],g=c[1],red=c[0];
  if(c[3] && b>red+24 && b>g+12 && b>90){unsigned v=b>g?b:g;
   if(colour==1){c[0]=v*29/255;c[1]=v*214/255+22;c[2]=v*70/255;}
   else{c[0]=v;c[1]=v*38/255;c[2]=v*42/255;}
  }
 }
 SDL_Surface*s=SDL_CreateRGBSurfaceWithFormatFrom(p,w,h,32,w*4,SDL_PIXELFORMAT_ABGR8888);
 SDL_Texture*t=s?SDL_CreateTextureFromSurface(r,s):NULL;
 if(s)SDL_FreeSurface(s);free(p);
 if(t){SDL_SetTextureBlendMode(t,SDL_BLENDMODE_BLEND);SDL_SetTextureScaleMode(t,SDL_ScaleModeNearest);}
 return t;
}
/* Contour pixels are derived at runtime from the APPROVED button alpha.
 * The old oversized focus PNGs included detached yellow strokes beneath START.
 * Here only the primary opaque connected silhouette can receive a 2px rim. */
/* Draw a 2-pixel yellow contour INSIDE the original opaque button shape.
 * Erode along an ellipse (independent horizontal/vertical insets), not by
 * shrinking the finished rectangular focus texture: this preserves both
 * curved ends and the approved 3D depth without resizing the button art. */
static SDL_Texture *alpha_focus(SDL_Renderer*r,const char*path,int inset_x,int inset_y){
 unsigned w=0,h=0;
 unsigned char *pixels=Expanded_PNG_LoadRGBA(path,&w,&h);
 if(!pixels||w<12||h<12||w>2048||h>1024){free(pixels);return NULL;}
 const size_t count=(size_t)w*h;
 unsigned char *mask=(unsigned char*)calloc(count,1);
 unsigned char *seen=(unsigned char*)calloc(count,1);
 unsigned char *best=(unsigned char*)calloc(count,1);
 unsigned char *inner=(unsigned char*)calloc(count,1);
 int *queue=(int*)malloc(count*sizeof(int));
 if(!mask||!seen||!best||!inner||!queue){
  free(mask);free(seen);free(best);free(inner);free(queue);free(pixels);return NULL;
 }
 for(size_t i=0;i<count;i++)mask[i]=pixels[i*4+3]>=128;
 /* Select only the dominant solid component; detached strokes are ignored. */
 size_t largest=0;
 for(size_t i=0;i<count;i++)if(mask[i]&&!seen[i]){
  size_t head=0,tail=1;queue[0]=(int)i;seen[i]=1;
  while(head<tail){
   int q=queue[head++];int x=q%(int)w,y=q/(int)w;
   for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){
    int nx=x+dx,ny=y+dy;
    if(nx<0||ny<0||nx>=(int)w||ny>=(int)h)continue;
    size_t ni=(size_t)ny*w+(unsigned)nx;
    if(mask[ni]&&!seen[ni]){seen[ni]=1;queue[tail++]=(int)ni;}
   }
  }
  if(tail>largest){memset(best,0,count);for(size_t k=0;k<tail;k++)best[queue[k]]=1;largest=tail;}
 }
 if(inset_x<1)inset_x=1;if(inset_y<1)inset_y=1;
 const int xx=inset_x*inset_x,yy=inset_y*inset_y;
 const int limit=xx*yy;
 /* Elliptical inward offset: its edges follow the alpha's curved outline,
  * even near the right-hand extrusion.  Done once, never at 50 Hz. */
 for(int y=0;y<(int)h;y++)for(int x=0;x<(int)w;x++){
  size_t pos=(size_t)y*w+(unsigned)x;
  if(!best[pos])continue;
  int ok=1;
  for(int dy=-inset_y;dy<=inset_y&&ok;dy++){
   for(int dx=-inset_x;dx<=inset_x;dx++){
    if(dx*dx*yy+dy*dy*xx>limit)continue;
    int nx=x+dx,ny=y+dy;
    if(nx<0||ny<0||nx>=(int)w||ny>=(int)h || !best[(size_t)ny*w+(unsigned)nx]){
     ok=0;break;
    }
   }
  }
  if(ok)inner[pos]=1;
 }
 unsigned char *rgba=(unsigned char*)calloc(count,4);
 if(!rgba){free(mask);free(seen);free(best);free(inner);free(queue);free(pixels);return NULL;}
 /* Slightly reinforce the upper/lower corner arcs near both endcaps.
  * The central straight contour remains 2px: no global thickening and
  * no rectangular rescale that would flatten the approved rounded shape. */
 for(int y=0;y<(int)h;y++)for(int x=0;x<(int)w;x++){
  size_t pos=(size_t)y*w+(unsigned)x;
  if(!inner[pos])continue;
  int edge=0;
  const int at_end=(x<(int)h || x>=(int)w-(int)h);
  const int edge_radius=at_end?3:2,edge_limit=at_end?10:4;
  for(int dy=-edge_radius;dy<=edge_radius&&!edge;dy++)
   for(int dx=-edge_radius;dx<=edge_radius;dx++){
    if(dx*dx+dy*dy>edge_limit)continue;
    int nx=x+dx,ny=y+dy;
    if(nx<0||ny<0||nx>=(int)w||ny>=(int)h || !inner[(size_t)ny*w+(unsigned)nx]){
     edge=1;break;
    }
   }
  if(edge){size_t j=pos*4;
   rgba[j]=255;rgba[j+1]=215;rgba[j+2]=39;rgba[j+3]=255;
  }
 }
 SDL_Surface *surf=SDL_CreateRGBSurfaceWithFormatFrom(rgba,w,h,32,w*4,SDL_PIXELFORMAT_ABGR8888);
 SDL_Texture *tex=surf?SDL_CreateTextureFromSurface(r,surf):NULL;
 if(surf)SDL_FreeSurface(surf);
 if(tex){SDL_SetTextureBlendMode(tex,SDL_BLENDMODE_BLEND);SDL_SetTextureScaleMode(tex,SDL_ScaleModeNearest);}
 free(rgba);free(mask);free(seen);free(best);free(inner);free(queue);free(pixels);
 return tex;
}
static int glyph_metric(const char *json,int len,int code,const char *field){
 char path[96];double n=0.0;
 snprintf(path,sizeof path,"$.glyphs.%d.%s",code,field);
 return mjson_get_number(json,len,path,&n)?(int)n:0;
}
static void load_font(SDL_Renderer*r,int index,const char*name){
 char path[256];
 snprintf(path,sizeof path,"assets/fonts/new_career/ui_%s.png",name);
 s_fonts[index].texture=texture(r,path,0);
 if(s_fonts[index].texture)SDL_SetTextureScaleMode(s_fonts[index].texture,SDL_ScaleModeNearest);
 snprintf(path,sizeof path,"assets/fonts/new_career/ui_%s.json",name);
 FILE*f=fopen(path,"rb");
 if(!f){fprintf(stderr,"[new-career] missing atlas metrics %s\n",path);return;}
 if(fseek(f,0,SEEK_END)!=0){fclose(f);return;}
 long len=ftell(f);rewind(f);
 if(len<=0||len>1000000){fclose(f);return;}
 char*json=(char*)calloc((size_t)len+1,1);
 if(!json){fclose(f);return;}
 size_t got=fread(json,1,(size_t)len,f);fclose(f);
 for(int ch=32;ch<127;ch++){
  CareerGlyph*g=&s_fonts[index].glyph[ch];
  g->rect=(SDL_Rect){glyph_metric(json,(int)got,ch,"x"),
     glyph_metric(json,(int)got,ch,"y"),
     glyph_metric(json,(int)got,ch,"w"),
     glyph_metric(json,(int)got,ch,"h")};
  g->advance=glyph_metric(json,(int)got,ch,"advance");
  g->bearingX=glyph_metric(json,(int)got,ch,"bearingX");
  g->bearingY=glyph_metric(json,(int)got,ch,"bearingY");
 }
 free(json);
}
static void load(SDL_Renderer*r){
 if(s_loaded)return;s_loaded=1;
 const char *base="assets/career/new_career/";
 char path[256];
 #define TEX(dst,file,col) snprintf(path,sizeof path,"%s%s",base,file);dst=texture(r,path,col)
 TEX(s_bg,"background_runtime.png",0);
 TEX(s_panel,"panel_runtime.png",0);
 TEX(s_toggle[0],"toggle_runtime.png",0);TEX(s_toggle[1],"toggle_runtime.png",1);
 TEX(s_start[0],"start_runtime.png",0);TEX(s_start[1],"start_runtime.png",2);
 TEX(s_check[0],"checkbox_off_approved_rgba.png",0);TEX(s_check[1],"checkbox_on_approved_rgba.png",0);
 /* No separate focus PNG: follows each button's own original alpha. */
 snprintf(path,sizeof path,"%stoggle_runtime.png",base);s_focus_toggle=alpha_focus(r,path,7,5);
 snprintf(path,sizeof path,"%sstart_runtime.png",base);s_focus_start=alpha_focus(r,path,6,3);
 TEX(s_flags,"nationality_flags.png",0);
 s_legend_ab=texture(r,"assets/ui/legends/a_select_b_back.png",0);
 #undef TEX
 load_font(r,0,"form");load_font(r,1,"button");load_font(r,2,"title");
 fprintf(stderr,"[new-career] 0.10.66: C optical baseline +1px, approved rounded legend 110pct aligned to frame; audio disabled\n");
}
void Expanded_NewCareer_Destroy(void){
 SDL_DestroyTexture(s_bg);SDL_DestroyTexture(s_panel);s_bg=s_panel=NULL;
 for(int i=0;i<2;i++){
  SDL_DestroyTexture(s_toggle[i]);SDL_DestroyTexture(s_start[i]);SDL_DestroyTexture(s_check[i]);
  s_toggle[i]=s_start[i]=s_check[i]=NULL;
 }
 SDL_DestroyTexture(s_focus_toggle);SDL_DestroyTexture(s_focus_start);SDL_DestroyTexture(s_flags);
 SDL_DestroyTexture(s_legend_ab);
 s_focus_toggle=s_focus_start=s_flags=s_legend_ab=NULL;
 for(int i=0;i<3;i++){
  SDL_DestroyTexture(s_fonts[i].texture);
  memset(&s_fonts[i],0,sizeof s_fonts[i]);
 }
 s_loaded=0;
}
/* The approved UI stays at 88.5%; the separate background is never resized by this logic. */
static SDL_FRect rect(float x,float y,float w,float h,float sx,float sy){
 return (SDL_FRect){(93+x*.885f)*sx,(50+y*.885f)*sy,w*.885f*sx,h*.885f*sy};
}
static void copy(SDL_Renderer*r,SDL_Texture*t,float x,float y,float w,float h,float sx,float sy){
 if(t){SDL_FRect d=rect(x,y,w,h,sx,sy);SDL_RenderCopyF(r,t,NULL,&d);}
}
static void draw_flag(SDL_Renderer*r,float x,float y,float sx,float sy){
 if(!s_flags)return;
 int idx=-1;
 for(unsigned i=0;i<sizeof s_flag_codes/sizeof s_flag_codes[0];i++)
  if(s_flag_codes[i]==_plNationality){idx=(int)i;break;}
 if(idx<0)return;
 SDL_Rect src={(idx%8)*48,(idx/8)*32,48,32};
 SDL_FRect dst=rect(x,y,48,32,sx,sy);
 SDL_RenderCopyF(r,s_flags,&src,&dst);
}
/* Kerning is applied to BOTH measurement and placement, so center alignment remains stable.
 * New pixel-art PNG atlases and metric JSON provide outlined glyphs at native size. */
static int pair_kern(unsigned a,unsigned b){
 if((a=='A'&&(b=='V'||b=='W'||b=='Y'||b=='T')) ||
    ((a=='V'||a=='W'||a=='Y')&&(b=='A'||b=='O')) ||
    (a=='T'&&(b=='A'||b=='O'||b=='E')) ||
    (a=='L'&&(b=='T'||b=='V'||b=='Y')))return -1;
 if((a=='P'&&b=='A')||(a=='F'&&b=='A')||(a=='R'&&b=='T')||
    (a=='C'&&b=='O')||(a=='O'&&b=='C'))return -1;
 return 0;
}
/* The C/O pair in the solitary role COACH has optical contact in this atlas.
 * Apply a small gap in BOTH metrics and rendering; do not change other labels. */
static int pair_kern_for_text(const char *s,unsigned a,unsigned b){
 if(strcmp(s,"COACH")==0){
  if(a=='C' && b=='O')return 2;
  if(a=='O' && b=='A')return 1;
 }
 return pair_kern(a,b);
}
static void fallback_text(SDL_Renderer*r,const char*s,int type,float x,float y,int centered,float sx,float sy,int selected){
 int unit=(int)((type==2?5:type==1?4:3)*(sx+sy)*.5f+.5f);
 if(unit<1)unit=1;
 int adv=6*unit;int left=(int)((93+x*.885f)*sx+.5f);
 int top=(int)((50+y*.885f)*sy+.5f);
 if(centered)left-=(int)strlen(s)*adv/2;
 SDL_SetRenderDrawColor(r,selected==1?255:selected==0?146:255,
   selected==1?226:selected==0?190:255,selected==1?72:selected==0?249:255,255);
 for(size_t i=0;s[i];i++){
  unsigned char ch=(unsigned char)s[i];
  if(ch==255){SDL_FRect line={(float)(left+(int)i*adv+2),(float)top,2.f,(float)(8*unit)};SDL_RenderFillRectF(r,&line);continue;}
  if(ch>='a'&&ch<='z')ch=(unsigned char)(ch-'a'+'A');
  if(ch<32||ch>126)ch='?';
  const unsigned char *bits=cb_glyphs[ch-32];
  for(int py=0;py<8;py++)for(int px=0;px<5;px++)if(bits[py]&(1u<<(4-px))){
   SDL_FRect dot={(float)(left+(int)i*adv+px*unit),(float)(top+py*unit),(float)unit,(float)unit};
   SDL_RenderFillRectF(r,&dot);
  }
 }
}
/* selected: 1 yellow, 0 inactive blue, -1 white.
 * max_width is in the atlas reference pixels (0 = original exact font size).
 * Only overlong values/buttons are fitted. Glyph atlas remains untouched and
 * SDL_ScaleModeNearest preserves the pixel edges and outlined shadow. */
static void pixel_text_tint_fit(SDL_Renderer*r,const char*s,int type,float x,float y,int centered,float sx,float sy,int selected,float max_width){
 if(!s)return;
 CareerFont*f=&s_fonts[type];
 if(!f->texture||!f->glyph['A'].advance){fallback_text(r,s,type,x,y,centered,sx,sy,selected);return;}
 Uint8 rr=255,gg=255,bb=255;
 if(selected==1){rr=255;gg=226;bb=72;}
 else if(selected==0){rr=146;gg=190;bb=249;}
 SDL_SetTextureColorMod(f->texture,rr,gg,bb);
 float screenScale=(sx+sy)*.5f;
 int total=0;unsigned prev=0;
 for(const unsigned char*p=(const unsigned char*)s;*p;p++){
  unsigned ch=*p;
  if(ch>='a'&&ch<='z')ch=ch-'a'+'A';
  if(ch==255){total+=8;prev=0;continue;}
  if(ch<32||ch>126)ch='?';
  if(prev)total+=pair_kern_for_text(s,prev,ch);
  total+=f->glyph[ch].advance;prev=ch;
 }
 /* At centered positions fit the full advance width, never clip glyphs
  * to hide a bad measurement. Let a little safety space for 3D glyph edges. */
 float fit=1.f;
 if(max_width>0.f && total>0 && (float)total>max_width)
  fit=max_width/(float)total;
 float fontScale=screenScale*fit;
 float atX=(93+x*.885f)*sx-(centered?total*fontScale*.5f:0.f);
 const float atY=(50+y*.885f)*sy+(1.f-fit)*f->glyph['A'].rect.h*screenScale*.5f;
 prev=0;
 for(const unsigned char*p=(const unsigned char*)s;*p;p++){
  unsigned ch=*p;
  if(ch==255){
   SDL_FRect caret={atX+1.f,atY,2.f*fontScale,29.f*fontScale};
   SDL_SetRenderDrawColor(r,255,255,255,255);SDL_RenderFillRectF(r,&caret);
   atX+=8.f*fontScale;prev=0;continue;
  }
  if(ch>='a'&&ch<='z')ch=ch-'a'+'A';
  if(ch<32||ch>126)ch='?';
  if(prev)atX+=pair_kern_for_text(s,prev,ch)*fontScale;
  CareerGlyph*g=&f->glyph[ch];
  if(g->rect.w>0&&g->rect.h>0){
   SDL_FRect dst={atX+g->bearingX*fontScale,atY+g->bearingY*fontScale,
                  g->rect.w*fontScale,g->rect.h*fontScale};
   /* AGE (18-70) is ONE line again. Move ONLY its two parentheses,
    * not the digits or AGE, by 2 output-reference pixels up/left. */
   if(type==0 && strcmp(s,"AGE (18-70)")==0 && (ch=='(' || ch==')')){
    dst.x-=2.f*sx;dst.y-=2.f*sy;
   }
   /* Keep all glyphs pixel-snapped. Apply the requested tiny 0.5
    * logical-pixel optical correction ONLY to the C in NEW CAREER.
    * SDL_RenderCopyF accepts fractional positions, and Nearest mode stays
    * crisp. On low-resolution displays this shift can quantize to 0 px. */
   dst.x=(float)((int)(dst.x+.5f));dst.y=(float)((int)(dst.y+.5f));
   if(type==2 && ch=='C' && strcmp(s,"NEW CAREER")==0)
    dst.y+=0.5f*sy;
   SDL_RenderCopyF(r,f->texture,&g->rect,&dst);
  }
  atX+=g->advance*fontScale;prev=ch;
 }
 SDL_SetTextureColorMod(f->texture,255,255,255);
}
static void pixel_text_tint(SDL_Renderer*r,const char*s,int type,float x,float y,int centered,float sx,float sy,int selected){
 pixel_text_tint_fit(r,s,type,x,y,centered,sx,sy,selected,0.f);
}
static void pixel_text(SDL_Renderer*r,const char*s,int type,float x,float y,int centered,float sx,float sy){
 pixel_text_tint_fit(r,s,type,x,y,centered,sx,sy,-1,0.f);
}
/* Thin focus; the two buttons use their already approved alpha contour sprites. */
static void focus(SDL_Renderer*r,float x,float y,float w,float h,float sx,float sy){
 SDL_SetRenderDrawColor(r,255,220,52,255);
 SDL_FRect d=rect(x,y,w,h,sx,sy);
 float corner=11*.885f*sx,cy=11*.885f*sy;
 SDL_FPoint p[9]={{d.x+corner,d.y},{d.x+d.w-corner,d.y},
  {d.x+d.w,d.y+cy},{d.x+d.w,d.y+d.h-cy},
  {d.x+d.w-corner,d.y+d.h},{d.x+corner,d.y+d.h},
  {d.x,d.y+d.h-cy},{d.x,d.y+cy},{d.x+corner,d.y}};
 SDL_RenderDrawLinesF(r,p,9);
}
static const char* value(int row){
 static const char *fallback[]={"MR","","","---","COACH","",""};
 if(row==3 && _plNationality==255)return "OTH";
 if(row==4)return _coachOrPlayer?"COACH":"PLAYER - COACH";
 if(row==6)return _expandedManagerFameText;
 uintptr_t p=_menuEntries[ord[row]].fg;
 return p>4096 && p!=(uintptr_t)0xffffffffu?(const char*)p:fallback[row];
}
void Expanded_NewCareer_Draw(SDL_Renderer*r,float w,float h){
 load(r);float sx=w/1664,sy=h/936;
 SDL_SetRenderDrawColor(r,0,0,0,255);SDL_RenderClear(r);
 if(s_bg){SDL_FRect d={0,0,w,h};SDL_RenderCopyF(r,s_bg,NULL,&d);}
 copy(r,s_panel,0,0,1670,942,sx,sy);
 /* One narrow dark joint between approved left/right halves, no bright stripe.
  * Keep the cyan top/bottom rims intact (do not overpaint their end caps). */
 for(int row=0;row<7;row++){
  SDL_FRect seam=rect(776,213+66*row,2,36,sx,sy);
  SDL_SetRenderDrawColor(r,3,23,53,255);SDL_RenderFillRectF(r,&seam);
 }
 /* NATION is an interactive right-hand field. A translucent blue wash
  * lifts its value panel, while the approved panel PNG stays unchanged.
  * Reapply the previous blend state to avoid changing subsequent UI. */
 {
  SDL_BlendMode prev_blend=SDL_BLENDMODE_NONE;
  SDL_GetRenderDrawBlendMode(r,&prev_blend);
  SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(r,67,165,255,66);
  SDL_FRect nation_bg=rect(790.f,408.f,548.f,40.f,sx,sy);
  SDL_RenderFillRectF(r,&nation_bg);
  SDL_SetRenderDrawBlendMode(r,prev_blend);
 }
 int sink=s_press>0?3:0;
 /* Bring both lower controls slightly upwards, and center their captions. */
 copy(r,s_toggle[s_unemployed],390+(s_action==7?sink:0),676+(s_action==7?sink:0),895,107,sx,sy);
 copy(r,s_start[ready()],532+(s_action==8?sink:0),785+(s_action==8?sink:0),606,89,sx,sy);
 copy(r,s_check[s_unemployed],1179+(s_action==7?sink:0),700+(s_action==7?sink:0),62,58,sx,sy);
 pixel_text(r,"NEW CAREER",2,835,68.5f,1,sx,sy);
 const char*labels[]={"TITLE","NAME","SURNAME","NATION","ROLE","AGE (18-70)","FAME"};
 for(int i=0;i<7;i++){
  pixel_text(r,labels[i],0,337,217.5f+66*i,0,sx,sy);
  /* Nation icon and three-letter code are centered as ONE visual group. */
  if(i==3){
   draw_flag(r,992,214+66*i,sx,sy);
   pixel_text_tint_fit(r,value(i),0,1112,217.5f+66*i,1,sx,sy,i==s_selected?1:-1,280.f);
  }else pixel_text_tint_fit(r,value(i),0,i==4?1066.f:1070.f,217.5f+66*i,1,sx,sy,
    i==s_selected?1:-1,i==4?248.f:(i==6?265.f:0.f));
 }
 pixel_text_tint_fit(r,"START UNEMPLOYED",1,802+(s_action==7?sink:0),707+(s_action==7?sink:0),1,sx,sy,-1,455.f);
 pixel_text(r,"START",1,835+(s_action==8?sink:0),812.87f+(s_action==8?sink:0),1,sx,sy);
 if(s_selected<7){
  const float target=202.f+66.f*(float)s_selected;
  if(s_focus_field_y<target){s_focus_field_y+=12.f;if(s_focus_field_y>target)s_focus_field_y=target;}
  else if(s_focus_field_y>target){s_focus_field_y-=12.f;if(s_focus_field_y<target)s_focus_field_y=target;}
  focus(r,782,s_focus_field_y,571,55,sx,sy);
 }else if(s_selected==7){
  s_focus_field_y=202.f+66.f*6.f;
  if(s_focus_toggle)copy(r,s_focus_toggle,390+(s_action==7?sink:0),676+(s_action==7?sink:0),895,107,sx,sy);
  else focus(r,405,690,862,73,sx,sy);
 }else{
  s_focus_field_y=202.f+66.f*6.f;
  if(s_focus_start)copy(r,s_focus_start,532+(s_action==8?sink:0),785+(s_action==8?sink:0),606,89,sx,sy);
  else focus(r,556,797,558,63,sx,sy);
 }
 /* Stable right/bottom margin, with one fixed legend sprite. */
 if(s_legend_ab){
  /* 10% larger than 0.10.65, same 12px right margin.
   * Legend bottom aligns to the lower edge of the main Career frame. */
  SDL_FRect d={1476.f*sx,775.f*sy,176.f*sx,96.f*sy};
  SDL_RenderCopyF(r,s_legend_ab,NULL,&d);
 }
}
static void action(int row,int direction){
 MenuEntry*e=&_menuEntries[ord[row]];((Menu*)_currentMenu)->selectedEntry=e;
 _controlMask=direction<0?0x40:direction>0?0x80:0x21;
 fprintf(stderr,"[new-career] action=%d direction=%d fame=%u unemployed=%d\n",row,direction,(unsigned)_expandedManagerFame,s_unemployed);
 switch(row){
 case 0:if(direction)ChangeSex(e);break;
 case 1:if(!direction)SetName(e);break;
 case 2:if(!direction)SetSurname(e);break;
 case 3:ChangeNationality(e);break;
 case 4:if(direction)CoachOrPlayerCoach(e);break;
 case 5:
  if(direction){
   int age=(int)_expandedManagerStartAge+direction;
   if(age<18)age=18;
   if(age>70)age=70;
   _expandedManagerStartAge=(uint8_t)age;
   Expanded_SetAgeText();
  }
  break;
 case 6:
  if(direction){_controlMask=direction<0?0x40:0x80;Expanded_ChangeManagerFame(e);}
  break;
 case 7:if(!direction)s_unemployed=!s_unemployed;break;
 case 8:if(!direction&&ready())ChooseTeamAndStartCareer();break;
 }
 _fireResetFlag=1;s_previous_fire=1;
}
int Expanded_NewCareer_Proc(void){
 if(!Expanded_NewCareer_InputActive())return 0;
 int x=0,y=0,moved=0,click=0;Platform_NewCareerMouseState(&x,&y,&moved,&click);
 if(Expanded_NewCareer_TakeBack()||_currentKey==27){s_press=0;s_action=-1;SetExitMenuFlag();_fireResetFlag=1;return 1;}
 if(!Expanded_NewCareer_Active()){
  Menu*m=(Menu*)_currentMenu;
  if(moved||click)for(int i=0;i<m->numEntries;i++){
   MenuEntry*e=&_menuEntries[i];
   if(!e->isInvisible&&!e->isDisabled&&e->onSelect&&x>=e->x&&x<e->x+e->dwWidth&&y>=e->y&&y<e->y+e->dwHeight){
    m->selectedEntry=e;
    if(click){_controlMask=0x21;CallMenuOnSelect((void*)e->onSelect,e);_fireResetFlag=1;return 1;}
    break;
   }
  }
  return 0;
 }
 int fire=(_fire||_shortFire)!=0,edge=fire&&!s_previous_fire;s_previous_fire=fire;
 if(s_nav_cooldown>0)--s_nav_cooldown;
 if(s_arrow_cooldown>0)--s_arrow_cooldown;
 if(s_press){if(--s_press==0){int a=s_action,d=s_action_dir;s_action=-1;action(a,d);}return 1;}
 float px=(x-93)/.885f,py=(y-50)/.885f;
 if(moved||click){
  int hit=-1;
  if(px>=312&&px<1363&&py>=200&&py<662){
   int row=(int)(py-200)/66;
   if(row<7 && py<200+66*row+60)hit=row;
  }
  if(px>=397&&px<1360&&py>=681&&py<779)hit=7;
  if(px>=539&&px<1132&&py>=788&&py<873)hit=8;
  if(hit>=0){
   if(hit!=s_selected){s_selected=hit;s_nav_cooldown=0;s_arrow_cooldown=0;}
   if(click){
    /* Arrows are never independent focus entries. A mouse click can use them,
     * while joypad/keyboard LEFT/RIGHT operate the focused field. */
    int direction=0;
    if(hit<7 && hit!=1 && hit!=2){
     if(px>=805&&px<872)direction=-1;
     else if(px>=1260&&px<1337)direction=1;
    }
    /* Only NATION has its legacy confirmation popup. TITLE/ROLE/AGE/FAME
     * are direction-only; NAME/SURNAME have the original text editor. */
    if(direction || hit==1 || hit==2 || hit==3 || hit==7 || hit==8){
     s_action=hit;s_action_dir=direction;s_press=(hit==5?2:5);
    }
   }
  }
 }
 if(s_press)return 1;
 if(_menuControlsDirection>=0){
  int d=_menuControlsDirection&6;
  /* Clamp 0..8: never wrap from TITLE to START or vice versa. */
  if(d==0 || d==4){
   if(s_nav_cooldown==0){
    if(d==0 && s_selected>0)--s_selected;
    else if(d==4 && s_selected<8)++s_selected;
    s_nav_cooldown=6;  /* 120ms minimum between focus selections */
   }
  }else if(s_selected!=1&&s_selected!=2&&s_selected!=7&&s_selected!=8){
   /* AGE uses the original rapid left/right stepping (2 ticks).
    * Other left/right selectors take 5 ticks and are rate-limited. */
   if(s_selected==5||s_arrow_cooldown==0){
    s_action=s_selected;s_action_dir=d==6?-1:1;
    s_press=(s_selected==5?2:5);
    if(s_selected!=5)s_arrow_cooldown=7;
   }
  }
 }
 ((Menu*)_currentMenu)->selectedEntry=&_menuEntries[ord[s_selected]];
 if(edge && (s_selected==1||s_selected==2||s_selected==3||s_selected==7||s_selected==8)){
  s_action=s_selected;s_action_dir=0;s_press=5;
 }
 return 1;
}
#endif
