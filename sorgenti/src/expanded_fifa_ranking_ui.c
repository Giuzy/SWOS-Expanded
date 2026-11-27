/* NATION RANKING viewer (WORLD 2627). Readonly: it never applies a result,
 * rolls a week or writes the CAR; it only reads expanded_fifa_ranking.c at
 * the CE civil day. Native packed menu (_expandedNationRankingMenu) with the
 * original SWOS font and menu palette; rows painted after the entries. */
#include "expanded_fifa_ranking.h"
#include "expanded_universal_car.h"
#include "menu_system.h"
#include "text_render.h"
#include "world.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <SDL.h>

extern uint8_t *_charTables[];
extern uint16_t _menuPalette[256];
extern uint16_t _controlMask;
extern uintptr_t _currentMenuPtr;
extern uint8_t _expandedNationRankingMenu[];
extern uint16_t _gameType;
extern void FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color);
extern void DrawMenu(void);
extern void SetExitMenuFlag(void);
extern MenuEntry _menuEntries[];

#define NR_ROWS 15
#define NR_X0 8                /* left edge of the row boxes (wide -16 + 24) */
#define NR_Y0 65               /* first row box (29 + 36) */
#define NR_STEP 10
static int s_nr_top;           /* first visible rank (0-based), view state only */
static int s_nr_open;

int Expanded_NationRanking_Active(void)
{
    return s_nr_open && _currentMenuPtr==(uintptr_t)_expandedNationRankingMenu;
}

static uint8_t nr_nearest(unsigned r,unsigned g,unsigned b)
{
    uint8_t best=0;
    unsigned bestd=UINT_MAX,i;
    for(i=0;i<256u;++i) {
        const unsigned v=_menuPalette[i];
        const int dr=(int)r-(int)(((v>>8)&15u)*17u);
        const int dg=(int)g-(int)(((v>>4)&15u)*17u);
        const int db=(int)b-(int)((v&15u)*17u);
        const unsigned d=(unsigned)(2*dr*dr+3*dg*dg+2*db*db);
        if(d<bestd){bestd=d;best=(uint8_t)i;}
    }
    return best;
}
/* flag colour letters -> nearest menu palette index (same table as the
 * generator tools/fifa_ranking/build_fifa_nations.py) */
static int nr_flag_colour(char c)
{
    static const struct {char c;uint8_t r,g,b;} k[]={
        {'R',200,16,46},{'W',255,255,255},{'B',0,36,125},{'U',0,87,183},
        {'L',100,170,230},{'G',0,130,60},{'Y',250,205,0},{'K',0,0,0},
        {'O',240,120,0},{'M',130,20,45},{'N',140,90,40},{'S',160,160,160}};
    static int cache[16],ready;
    unsigned i;
    if(!ready){for(i=0;i<sizeof k/sizeof k[0];++i)cache[i]=nr_nearest(k[i].r,k[i].g,k[i].b);ready=1;}
    for(i=0;i<sizeof k/sizeof k[0];++i)if(k[i].c==c)return cache[i];
    return -1;                 /* 'T' transparent: the OG row box shows */
}
static void nr_flag(int x,int y,const char *px)
{
    int r,c;
    for(r=0;r<EXP_FIFA_FLAG_H;++r)
        for(c=0;c<EXP_FIFA_FLAG_W;++c) {
            const int col=nr_flag_colour(px[r*EXP_FIFA_FLAG_W+c]);
            if(col>=0)FillRectangle((uint16_t)(x+c),(uint16_t)(y+r),1,1,(uint16_t)col);
        }
}
/* small 5x3 triangle: up (gained) or down (lost) */
static void nr_arrow(int x,int y,int up,uint16_t col)
{
    int k;
    for(k=0;k<3;++k) {
        const int row=up?y+k:y+2-k;
        FillRectangle((uint16_t)(x+2-k),(uint16_t)row,(uint16_t)(1+2*k),1,col);
    }
}
static void nr_text(int x,int y,uint16_t col,const char *s)
{
    DrawMenuText((uint16_t)x,(uint16_t)y,col,(uint8_t *)s,_charTables[0]);
}
static void nr_right(int x,int y,uint16_t col,const char *s)
{
    DrawMenuTextRightAligned((uint16_t)x,(uint16_t)y,col,(uint8_t *)s,_charTables[0]);
}
static int32_t nr_today(void)
{
    const ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    return r?r->scheduler.clock.day_index:INT32_MIN;
}

void Expanded_NationRankingDraw(void)
{
    const uint16_t white=2;
    static int green=-1,red=-1,grey=-1;
    const int32_t today=nr_today();
    int i;
    if(!Expanded_NationRanking_Active())return;
    if(green<0){green=nr_nearest(40,200,60);red=nr_nearest(230,40,40);grey=nr_nearest(170,170,170);}
    /* column header (on the native header strip) */
    nr_text(NR_X0+4,NR_Y0-10,white,"MOV");
    nr_right(NR_X0+56,NR_Y0-10,white,"POS");
    nr_text(NR_X0+76,NR_Y0-10,white,"NATION");
    nr_right(NR_X0+240,NR_Y0-10,white,"+/-");
    nr_right(NR_X0+298,NR_Y0-10,white,"POINTS");
    for(i=0;i<NR_ROWS;++i) {
        ExpFifaViewRow v;
        char buf[32];
        const int y=NR_Y0+i*NR_STEP,ty=y+1;
        long long c;
        if(!ExpFifa_ViewRow((uint32_t)(s_nr_top+i),today,&v))break;
        /* 1. weekly position change */
        if(v.move>0) {
            nr_arrow(NR_X0+4,y+3,1,(uint16_t)green);
            snprintf(buf,sizeof buf,"%d",(int)v.move);nr_text(NR_X0+11,ty,(uint16_t)green,buf);
        } else if(v.move<0) {
            nr_arrow(NR_X0+4,y+3,0,(uint16_t)red);
            snprintf(buf,sizeof buf,"%d",-(int)v.move);nr_text(NR_X0+11,ty,(uint16_t)red,buf);
        } else FillRectangle(NR_X0+4,(uint16_t)(y+4),6,1,(uint16_t)grey);
        /* 2. position */
        snprintf(buf,sizeof buf,"%u",(unsigned)v.pos);nr_right(NR_X0+56,ty,white,buf);
        /* 3. flag, 4. name */
        nr_flag(NR_X0+61,y,v.nation->flag);
        nr_text(NR_X0+76,ty,white,v.nation->name);
        /* 5. weekly points change, 2 decimals (internal precision kept) */
        c=llround(v.week_delta*100.0);
        snprintf(buf,sizeof buf,"%c%lld.%02lld",c<0?'-':'+',(c<0?-c:c)/100,(c<0?-c:c)%100);
        nr_right(NR_X0+240,ty,(uint16_t)(c>0?green:c<0?red:white),buf);
        /* 6. total points */
        c=llround(v.nation->points*100.0);
        snprintf(buf,sizeof buf,"%lld.%02lld",c/100,c%100);
        nr_right(NR_X0+298,ty,white,buf);
    }
}

void Expanded_NationRankingInit(void)
{
    MenuEntry *back=&_menuEntries[17];
    /* BACK keeps the focus: FIRE closes, UP/DOWN scroll (bits 0,5 fire,
     * 3 up, 4 down). Mouse never drives this viewer. */
    /* bit 5 = FIRE edge (as OG "fire only" 0x20). Bit 0 is the HELD fire:
     * the press that opened this screen is still down on its first frame
     * and closed it immediately (Windows 27/11). */
    back->controlMask=(1u<<5)|(1u<<3)|(1u<<4);
}
void Expanded_NationRankingControl(void)
{
    const int n=(int)ExpFifa_Count();
    const int max_top=n>NR_ROWS?n-NR_ROWS:0;
    if(_controlMask&(1u<<5)) {SetExitMenuFlag();return;}
    if(_controlMask&((1u<<3)|(1u<<4))) {
        /* UP/DOWN are HELD states (every frame): one row on press, then
         * auto-repeat after 300 ms every 60 ms, like a held cursor key */
        static uint32_t last,next;
        const uint32_t now=SDL_GetTicks();
        if(now-last>120u)next=now;          /* new press */
        last=now;
        if((int32_t)(now-next)<0)return;
        next=now+(next==now?300u:60u);
        if((_controlMask&(1u<<3)) && s_nr_top>0){--s_nr_top;DrawMenu();}
        else if((_controlMask&(1u<<4)) && s_nr_top<max_top){++s_nr_top;DrawMenu();}
        return;
    }
}
/* Button action (WORLD -> regions screen). Opens the viewer at rank 1. */
void Expanded_NationRankingOpen(void)
{
    if(World_Current()!=WORLD_2627 || !ExpFifa_Active())return;
    s_nr_top=0;s_nr_open=1;
    ShowMenu(_expandedNationRankingMenu);
    s_nr_open=0;
}
/* The WORLD -> regions screen of a 2026/27 career (club or unemployed). */
int Expanded_NationRankingButtonWanted(uint16_t country,uint16_t depth)
{
    return World_Current()==WORLD_2627 && _gameType==4 && ExpFifa_Active() &&
           country==254u && depth==0u;
}
