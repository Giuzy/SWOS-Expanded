/* NEW CAREER HD. Presentation/setup state only; career/save engine unchanged. */
#ifdef SWOS_SDL_BUILD
#include "expanded_new_career.h"
#include "expanded_png.h"
#include "expanded_club_font.h" /* fallback only: runtime uses approved-style PNG atlas */
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
extern uint8_t _expandedManagerStartAge,_expandedManagerFame;
extern uint16_t _coachOrPlayer;
extern char _expandedAgeText[],_expandedManagerFameText[];
extern void Expanded_SetAgeText(void);
extern void ChooseTeamAndStartCareer(void);
extern void ChangeSex(MenuEntry*),SetName(MenuEntry*),SetSurname(MenuEntry*),ChangeNationality(MenuEntry*),CoachOrPlayerCoach(MenuEntry*);
extern void Expanded_EditManagerAge(MenuEntry*),Expanded_ChangeManagerFame(void*);
extern void CallMenuOnSelect(void *,void *);
extern int Platform_NewCareerMouseState(int*,int*,int*,int*);
static SDL_atomic_t s_back;
static int s_selected,s_unemployed=1,s_previous_fire=1,s_press,s_action=-1,s_action_dir;
static const int ord[]={3,4,5,7,9,15,13,10,11};
typedef struct {SDL_Rect rect; int advance,bearingX,bearingY;} CareerGlyph;
typedef struct {SDL_Texture *texture;CareerGlyph glyph[128];} CareerFont;
static CareerFont s_fonts[3]; /* form, button, title */
static SDL_Texture *s_focus_toggle,*s_focus_start;
static SDL_Texture *s_bg,*s_panel,*s_toggle[2],*s_start[2],*s_check[2];
static int s_loaded;
int Expanded_NewCareer_Active(void){return _currentMenuPtr==(uintptr_t)_careerMenu;}
int Expanded_NewCareer_InputActive(void){
 return Expanded_NewCareer_Active() || _currentMenuPtr==(uintptr_t)_expandedNationalityPickerMenu || _currentMenuPtr==(uintptr_t)_expandedFamePickerMenu;
}
void Expanded_NewCareer_Back(void){SDL_AtomicSet(&s_back,1);}
int Expanded_NewCareer_TakeBack(void){return SDL_AtomicSet(&s_back,0);}
int Expanded_NewCareer_Unemployed(void){return s_unemployed;}
void Expanded_NewCareer_Reset(void){
 s_selected=0;s_unemployed=1;s_previous_fire=1;s_press=0;s_action=-1;SDL_AtomicSet(&s_back,0);
 fprintf(stderr,"[new-career] setup reset: unemployed=ON; fame independent\n");
}
static int ready(void){return _expandedManagerStartAge>=18 && _expandedManagerStartAge<=70 && _expandedManagerFame<5 && !_deadVar02;}
static SDL_Texture *texture(SDL_Renderer*r,const char*path,int colour){
 unsigned w=0,h=0;unsigned char*p=Expanded_PNG_LoadRGBA(path,&w,&h);
 if(!p){fprintf(stderr,"[new-career] missing/invalid PNG: %s\n",path);return NULL;}
 if(colour)for(size_t i=0;i<(size_t)w*h;i++){unsigned char*c=p+4*i;unsigned b=c[2],g=c[1],red=c[0];
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
static int glyph_metric(const char *json,int len,int code,const char *field){
 char path[96];double n=0.0;
 snprintf(path,sizeof path,"$.glyphs.%d.%s",code,field);
 return mjson_get_number(json,len,path,&n)?(int)n:0;
}
static void load_font(SDL_Renderer*r,int index,const char*name){
 char path[256];
 snprintf(path,sizeof path,"assets/fonts/new_career/ui_%s.png",name);
 s_fonts[index].texture=texture(r,path,0);
 if(s_fonts[index].texture)SDL_SetTextureScaleMode(s_fonts[index].texture,SDL_ScaleModeLinear);
 snprintf(path,sizeof path,"assets/fonts/new_career/ui_%s.json",name);
 FILE*f=fopen(path,"rb");
 if(!f){fprintf(stderr,"[new-career] missing atlas metrics %s\\n",path);return;}
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
 TEX(s_focus_toggle,"focus_toggle.png",0);
 TEX(s_focus_start,"focus_start.png",0);
 #undef TEX
 load_font(r,0,"form");load_font(r,1,"button");load_font(r,2,"title");
 fprintf(stderr,"[new-career] 0.10.57 UI .885, upright bitmap atlases and alpha focus\\n");
}
void Expanded_NewCareer_Destroy(void){
 SDL_DestroyTexture(s_bg);SDL_DestroyTexture(s_panel);s_bg=s_panel=NULL;
 for(int i=0;i<2;i++){SDL_DestroyTexture(s_toggle[i]);SDL_DestroyTexture(s_start[i]);SDL_DestroyTexture(s_check[i]);s_toggle[i]=s_start[i]=s_check[i]=NULL;}
 SDL_DestroyTexture(s_focus_toggle);SDL_DestroyTexture(s_focus_start);
 s_focus_toggle=s_focus_start=NULL;
 for(int i=0;i<3;i++){
  SDL_DestroyTexture(s_fonts[i].texture);
  memset(&s_fonts[i],0,sizeof s_fonts[i]);
 }
 s_loaded=0;
}
/* Keep approved UI at 88.5 percent while drawing type at integer screen pixels.
 * This avoids the resampling artifacts in the draft atlas. */
static SDL_FRect rect(float x,float y,float w,float h,float sx,float sy){
 return (SDL_FRect){(93+x*.885f)*sx,(50+y*.885f)*sy,w*.885f*sx,h*.885f*sy};
}
static void copy(SDL_Renderer*r,SDL_Texture*t,float x,float y,float w,float h,float sx,float sy){
 if(t){SDL_FRect d=rect(x,y,w,h,sx,sy);SDL_RenderCopyF(r,t,NULL,&d);}
}
/* Draw at the final screen coordinates. Text does NOT inherit the UI's 88.5% shrink.
 * The atlas is a runtime PNG, not a TTF dependency or the coarse 5x8 fallback. */
static void fallback_text(SDL_Renderer*r,const char*s,int type,float x,float y,int centered,float sx,float sy){
 int unit=(int)((type==2?5:type==1?4:3)*(sx+sy)*.5f+.5f);
 if(unit<1)unit=1;
 int adv=6*unit;int left=(int)((93+x*.885f)*sx+.5f);
 int top=(int)((50+y*.885f)*sy+.5f);
 if(centered)left-=(int)strlen(s)*adv/2;
 for(size_t i=0;s[i];i++){
  unsigned char ch=(unsigned char)s[i];
  if(ch==255){
   SDL_FRect line={(float)(left+(int)i*adv+2), (float)top,2.f,(float)(8*unit)};
   SDL_SetRenderDrawColor(r,255,255,255,255);SDL_RenderFillRectF(r,&line);
   continue;
  }
  if(ch>='a'&&ch<='z')ch=(unsigned char)(ch-'a'+'A');
  if(ch<32||ch>126)ch='?';
  const unsigned char *bits=cb_glyphs[ch-32];
  for(int py=0;py<8;py++)for(int px=0;px<5;px++)if(bits[py]&(1u<<(4-px))){
   SDL_FRect dot={(float)(left+(int)i*adv+px*unit),(float)(top+py*unit),(float)unit,(float)unit};
   SDL_SetRenderDrawColor(r,255,255,255,255);SDL_RenderFillRectF(r,&dot);
  }
 }
}
static void pixel_text(SDL_Renderer*r,const char*s,int type,float x,float y,int centered,float sx,float sy){
 if(!s)return;
 CareerFont*f=&s_fonts[type];
 if(!f->texture||!f->glyph['A'].advance){
  fallback_text(r,s,type,x,y,centered,sx,sy);return;
 }
 float screenScale=(sx+sy)*.5f;
 int total=0;
 for(const unsigned char*p=(const unsigned char*)s;*p;p++){
  unsigned ch=*p;
  if(ch>='a'&&ch<='z')ch=ch-'a'+'A';
  if(ch==255){total+=8;continue;}
  if(ch<32||ch>126)ch='?';
  total+=f->glyph[ch].advance;
 }
 float atX=(93+x*.885f)*sx-(centered?total*screenScale*.5f:0.f);
 const float atY=(50+y*.885f)*sy;
 for(const unsigned char*p=(const unsigned char*)s;*p;p++){
  unsigned ch=*p;
  if(ch==255){
   /* The legacy text editor emits 0xff as its cursor. Draw a normal caret,
    * never the '?' that the old sprite font displayed for unknown glyphs. */
   SDL_FRect caret={atX+1.f,atY,2.f*screenScale,29.f*screenScale};
   SDL_SetRenderDrawColor(r,255,255,255,255);SDL_RenderFillRectF(r,&caret);
   atX+=8.f*screenScale;continue;
  }
  if(ch>='a'&&ch<='z')ch=ch-'a'+'A';
  if(ch<32||ch>126)ch='?';
  CareerGlyph*g=&f->glyph[ch];
  if(g->rect.w>0&&g->rect.h>0){
   SDL_FRect dst={atX+g->bearingX*screenScale,atY+g->bearingY*screenScale,
                   g->rect.w*screenScale,g->rect.h*screenScale};
   dst.x=(float)((int)(dst.x+.5f));dst.y=(float)((int)(dst.y+.5f));
   SDL_RenderCopyF(r,f->texture,&g->rect,&dst);
  }
  atX+=g->advance*screenScale;
 }
}
/* One-pixel chamfer for form fields; button focus uses alpha-contour sprite. */
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
 if(row==4)return _coachOrPlayer?"COACH":"PL/COACH";
 if(row==6)return _expandedManagerFameText;
 uintptr_t p=_menuEntries[ord[row]].fg;
 return p>4096 && p!=(uintptr_t)0xffffffffu?(const char*)p:fallback[row];
}
void Expanded_NewCareer_Draw(SDL_Renderer*r,float w,float h){
 load(r);float sx=w/1664,sy=h/936;
 SDL_SetRenderDrawColor(r,0,0,0,255);SDL_RenderClear(r);
 if(s_bg){SDL_FRect d={0,0,w,h};SDL_RenderCopyF(r,s_bg,NULL,&d);}
 copy(r,s_panel,0,0,1670,942,sx,sy);
 /* Separate touching halves of each pair. Cover only the narrow 3px seam;
  * background, approved artwork and hitboxes otherwise stay unchanged. */
 SDL_SetRenderDrawColor(r,3,28,70,255);
 for(int row=0;row<7;row++){
  SDL_FRect seam=rect(775,207+66*row,4,47,sx,sy);
  SDL_RenderFillRectF(r,&seam);
 }
 int sink=s_press>0?3:0;
 copy(r,s_toggle[s_unemployed],390+(s_action==7?sink:0),682+(s_action==7?sink:0),895,107,sx,sy);
 copy(r,s_start[ready()],532+(s_action==8?sink:0),788+(s_action==8?sink:0),606,89,sx,sy);
 /* Checkbox stays INSIDE the unemployment toggle and follows button bounce. */
 copy(r,s_check[s_unemployed],1179+(s_action==7?sink:0),706+(s_action==7?sink:0),62,58,sx,sy);
 pixel_text(r,"NEW CAREER",2,835,59,1,sx,sy);
 const char*labels[]={"TITLE","NAME","SURNAME","NATION","ROLE","AGE (18-70)","FAME"};
 for(int i=0;i<7;i++){
  pixel_text(r,labels[i],0,337,210+66*i,0,sx,sy);
  pixel_text(r,value(i),0,1070,210+66*i,1,sx,sy);
 }
 pixel_text(r,"START UNEMPLOYED",1,802+(s_action==7?sink:0),701+(s_action==7?sink:0),1,sx,sy);
 pixel_text(r,"START",1,835+(s_action==8?sink:0),801+(s_action==8?sink:0),1,sx,sy);
 if(s_selected<7)focus(r,782,202+66*s_selected,571,55,sx,sy);
 else if(s_selected==7){
  if(s_focus_toggle)copy(r,s_focus_toggle,385+(s_action==7?sink:0),675+(s_action==7?sink:0),910,119,sx,sy);
  else focus(r,405,693,864,79,sx,sy);
 }else{
  if(s_focus_start)copy(r,s_focus_start,526+(s_action==8?sink:0),779+(s_action==8?sink:0),619,103,sx,sy);
  else focus(r,550,797,571,69,sx,sy);
 }
}
static void action(int row,int direction){
 MenuEntry*e=&_menuEntries[ord[row]];((Menu*)_currentMenu)->selectedEntry=e;
 _controlMask=direction<0?0x40:direction>0?0x80:0x21;
 fprintf(stderr,"[new-career] action=%d direction=%d fame=%u unemployed=%d\n",row,direction,(unsigned)_expandedManagerFame,s_unemployed);
 switch(row){
 case 0:ChangeSex(e);break;
 case 1:if(!direction)SetName(e);break;
 case 2:if(!direction)SetSurname(e);break;
 case 3:ChangeNationality(e);break;
 case 4:CoachOrPlayerCoach(e);break;
 case 5:
  /* AGE is an inline left/right bounded selector, not an editable input. */
  if(direction){
   int age=(int)_expandedManagerStartAge+direction;
   if(age<18)age=18;
   if(age>70)age=70;
   _expandedManagerStartAge=(uint8_t)age;
   Expanded_SetAgeText();
  }
  break;
 case 6:
  /* Direct FAME cycle: do not open the nested selector. */
  if(!direction)direction=1;
  _controlMask=direction<0?0x40:0x80;
  Expanded_ChangeManagerFame(e);break;
 case 7:s_unemployed=!s_unemployed;break;
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
  if(moved||click)for(int i=0;i<m->numEntries;i++){MenuEntry*e=&_menuEntries[i];
   if(!e->isInvisible&&!e->isDisabled&&e->onSelect&&x>=e->x&&x<e->x+e->dwWidth&&y>=e->y&&y<e->y+e->dwHeight){
    m->selectedEntry=e;if(click){_controlMask=0x21;CallMenuOnSelect((void*)e->onSelect,e);_fireResetFlag=1;return 1;}break;
   }
  }
  return 0;
 }
 int fire=(_fire||_shortFire)!=0,edge=fire&&!s_previous_fire;s_previous_fire=fire;
 if(s_press){if(--s_press==0){int a=s_action,d=s_action_dir;s_action=-1;action(a,d);}return 1;}
 float px=(x-93)/.885f,py=(y-50)/.885f;
 if(moved||click){
  int hit=-1;
  if(px>=312&&px<1363&&py>=200&&py<662){
   int row=(int)(py-200)/66;
   if(row<7 && py<200+66*row+60)hit=row;
  }
  if(px>=397&&px<1360&&py>=687&&py<785)hit=7;
  if(px>=539&&px<1132&&py>=791&&py<876)hit=8;
  if(hit>=0){
   s_selected=hit;
   if(click){
    /* Painted arrows are decorative; only the center of each row is clickable. */
    int arrow=hit<7&&hit!=1&&hit!=2&&
              ((px>=805&&px<872)||(px>=1260&&px<1337));
    if(!arrow){s_action=hit;s_action_dir=0;s_press=4;}
   }
  }
 }
 if(s_press)return 1;
 if(_menuControlsDirection>=0){
  int d=_menuControlsDirection&6;
  if(d==0)s_selected=(s_selected+8)%9;
  else if(d==4)s_selected=(s_selected+1)%9;
  else if(s_selected!=1&&s_selected!=2&&s_selected!=7&&s_selected!=8){s_action=s_selected;s_action_dir=d==6?-1:1;s_press=2;}
 }
 ((Menu*)_currentMenu)->selectedEntry=&_menuEntries[ord[s_selected]];
 if(edge){s_action=s_selected;s_action_dir=0;s_press=4;}
 return 1;
}
#endif
