/* NEW CAREER HD. Presentation/setup state only; career/save engine unchanged. */
#ifdef SWOS_SDL_BUILD
#include "expanded_new_career.h"
#include "expanded_png.h"
#include "menu_system.h"
#include "mjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
static const int heights[]={28,32,40,48};
typedef struct {SDL_Rect rect; int advance; } Glyph;
typedef struct {SDL_Texture *texture; Glyph glyph[128];} Font;
static Font s_fonts[4];
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
static int number(const char*json,int len,int code,const char*field){
 char key[80];double n=0;snprintf(key,sizeof key,"$.glyphs.%d.%s",code,field);
 return mjson_get_number(json,len,key,&n)?(int)n:0;
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
 #undef TEX
 for(int i=0;i<4;i++){
  snprintf(path,sizeof path,"assets/fonts/new_career/career_%d.png",heights[i]);s_fonts[i].texture=texture(r,path,0);
  snprintf(path,sizeof path,"assets/fonts/new_career/career_%d.json",heights[i]);FILE*f=fopen(path,"rb");if(!f)continue;
  fseek(f,0,SEEK_END);long len=ftell(f);rewind(f);if(len<=0||len>1000000){fclose(f);continue;}
  char*json=calloc(1,len+1);if(!json){fclose(f);continue;}size_t got=fread(json,1,len,f);fclose(f);
  for(int c=32;c<128;c++){Glyph*g=&s_fonts[i].glyph[c];g->rect=(SDL_Rect){number(json,got,c,"x"),number(json,got,c,"y"),number(json,got,c,"w"),number(json,got,c,"h")};g->advance=number(json,got,c,"advance");}
  free(json);
 }
 fprintf(stderr,"[new-career] HD layers/font loaded; canvas 1664x936, UI scale .885 origin 93,50\n");
}
void Expanded_NewCareer_Destroy(void){
 SDL_DestroyTexture(s_bg);SDL_DestroyTexture(s_panel);s_bg=s_panel=NULL;
 for(int i=0;i<2;i++){SDL_DestroyTexture(s_toggle[i]);SDL_DestroyTexture(s_start[i]);SDL_DestroyTexture(s_check[i]);s_toggle[i]=s_start[i]=s_check[i]=NULL;}
 for(int i=0;i<4;i++){SDL_DestroyTexture(s_fonts[i].texture);memset(&s_fonts[i],0,sizeof s_fonts[i]);}s_loaded=0;
}
static int code(unsigned char c){if(c==255)return '|';if(c>='a'&&c<='z')return c-'a'+'A';return c<128?c:'?';}
static Glyph *glyph(Font*f,unsigned char c){int v=code(c);return f->glyph[v].advance?&f->glyph[v]:&f->glyph['?'];}
static int width(const char*s,int font){int w=0;if(s)while(*s)w+=glyph(&s_fonts[font],(unsigned char)*s++)->advance;return w;}
/* All hitboxes and artwork share this transform, including the approved 3px crop. */
static SDL_FRect rect(float x,float y,float w,float h,float sx,float sy){
 return (SDL_FRect){(93+x*.885f)*sx,(50+y*.885f)*sy,w*.885f*sx,h*.885f*sy};
}
static void copy(SDL_Renderer*r,SDL_Texture*t,float x,float y,float w,float h,float sx,float sy){
 if(t){SDL_FRect d=rect(x,y,w,h,sx,sy);SDL_RenderCopyF(r,t,NULL,&d);}
}
static void text(SDL_Renderer*r,const char*s,int font,float x,float y,int centered,float sx,float sy){
 Font*f=&s_fonts[font];if(!s||!f->texture)return;if(centered)x-=width(s,font)/2.0f;
 while(*s){Glyph*g=glyph(f,(unsigned char)*s++);SDL_FRect d=rect(x-4,y-4,g->rect.w,g->rect.h,sx,sy);SDL_RenderCopyF(r,f->texture,&g->rect,&d);x+=g->advance;}
}
static void focus(SDL_Renderer*r,float x,float y,float w,float h,float sx,float sy){
 SDL_SetRenderDrawColor(r,255,220,52,255);
 /* Chamfered outline follows the rounded rectangular field/button perimeter. */
 for(int k=0;k<3;k++){SDL_FRect d=rect(x-k,y-k,w+2*k,h+2*k,sx,sy);float c=9*.885f*sx,cy=9*.885f*sy;
 SDL_FPoint p[9]={{d.x+c,d.y},{d.x+d.w-c,d.y},{d.x+d.w,d.y+cy},{d.x+d.w,d.y+d.h-cy},{d.x+d.w-c,d.y+d.h},{d.x+c,d.y+d.h},{d.x,d.y+d.h-cy},{d.x,d.y+cy},{d.x+c,d.y}};SDL_RenderDrawLinesF(r,p,9);}
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
 int sink=s_press>0?3:0;
 copy(r,s_toggle[s_unemployed],390+(s_action==7?sink:0),682+(s_action==7?sink:0),895,107,sx,sy);
 copy(r,s_start[ready()],532+(s_action==8?sink:0),788+(s_action==8?sink:0),606,89,sx,sy);
 copy(r,s_check[s_unemployed],1294,707,62,58,sx,sy);
 text(r,"NEW CAREER",3,835,66,1,sx,sy);
 const char*labels[]={"TITLE","NAME","SURNAME","NATION","ROLE","AGE (18-70)","FAME"};
 for(int i=0;i<7;i++){
  text(r,labels[i],1,337,214+66*i,0,sx,sy);
  int font=(i==1||i==2)?0:1;
  text(r,value(i),font,1070,214+66*i,1,sx,sy);
 }
 text(r,"START UNEMPLOYED",2,835+(s_action==7?sink:0),710+(s_action==7?sink:0),1,sx,sy);
 text(r,"START",2,835+(s_action==8?sink:0),810+(s_action==8?sink:0),1,sx,sy);
 if(s_selected<7)focus(r,780,200+66*s_selected,582,59,sx,sy);
 else if(s_selected==7)focus(r,398,690,875,86,sx,sy);
 else focus(r,542,797,583,69,sx,sy);
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
  if(direction){int age=_expandedManagerStartAge+direction;if(age>=18&&age<=70)_expandedManagerStartAge=age;Expanded_SetAgeText();}
  else Expanded_EditManagerAge(e);
  break;
 case 6:Expanded_ChangeManagerFame(e);break;
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
  int hit=-1,d=0;
  if(px>=312&&px<1363&&py>=200&&py<662){int row=(int)(py-200)/66;if(row<7 && py<200+66*row+60){hit=row;
   if(row!=1&&row!=2){if(px>=813&&px<867)d=-1;else if(px>=1276&&px<1330)d=1;}
  }}
  if(px>=397&&px<1360&&py>=687&&py<785)hit=7;
  if(px>=539&&px<1132&&py>=791&&py<876)hit=8;
  if(hit>=0){s_selected=hit;if(click){s_action=hit;s_action_dir=d;s_press=4;}}
 }
 if(s_press)return 1;
 if(_menuControlsDirection>=0){
  int d=_menuControlsDirection&6;
  if(d==0)s_selected=(s_selected+8)%9;
  else if(d==4)s_selected=(s_selected+1)%9;
  else if(s_selected!=1&&s_selected!=2&&s_selected!=8){s_action=s_selected;s_action_dir=d==6?-1:1;s_press=2;}
 }
 ((Menu*)_currentMenu)->selectedEntry=&_menuEntries[ord[s_selected]];
 if(edge){s_action=s_selected;s_action_dir=0;s_press=4;}
 return 1;
}
#endif
