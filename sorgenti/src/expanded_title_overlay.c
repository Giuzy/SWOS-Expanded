/* SWOS Expanded — modern SDL2 Title Menu (0.10.53_01).
 * No conversion to the 320x256 SWOS pixel framebuffer and no baked-in
 * old menu art. All five user-selected buttons are independent RGBA sprites.
 * The original SWOS callbacks are preserved in menus.c, but their old
 * buttons and background are never presented in this menu. */
#include "expanded_title_overlay.h"
#include "expanded_title_layout.h"
#include "expanded_png.h"
#include "expanded_club_font.h"
#include "build_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const s_background_path =
    "assets/title/title_background_approved_2026-11-08.png";
static const char *const s_logo_path =
    "assets/title/swos_expanded_logo_approved_2026-11-08.png";
static const char *const s_button_paths[EXP_TITLE_MAIN_COUNT] = {
    "assets/title/buttons/career.png",
    "assets/title/buttons/hall_of_fame.png",
    "assets/title/buttons/classic_modes.png",
    "assets/title/buttons/options.png",
    "assets/title/buttons/exit_game.png"
};
static const char *const s_main_labels[EXP_TITLE_MAIN_COUNT] = {
    "CAREER","HALL OF FAME","CLASSIC MODES","OPTIONS","EXIT GAME"
};
static SDL_Texture *s_background,*s_logo,*s_buttons[EXP_TITLE_MAIN_COUNT];
static unsigned s_button_widths[EXP_TITLE_MAIN_COUNT],s_button_heights[EXP_TITLE_MAIN_COUNT];
enum { LOGO_W=1200, LOGO_H=440, LOGO_X=920, LOGO_Y=17, LOGO_DST_W=620, LOGO_DST_H=227 };

static SDL_Texture *texture_from_png(SDL_Renderer *r,const char *path,
                                     unsigned *w,unsigned *h)
{
    unsigned width=0,height=0;
    unsigned char *rgba=Expanded_PNG_LoadRGBA(path,&width,&height);
    if(!rgba) {
        fprintf(stderr,"[title-ui] absent or unsupported PNG: %s\n",path);
        return NULL;
    }
    SDL_Surface *surface=SDL_CreateRGBSurfaceWithFormatFrom(
        rgba,(int)width,(int)height,32,(int)width*4,SDL_PIXELFORMAT_ABGR8888);
    SDL_Texture *texture=NULL;
    if(surface) {
        texture=SDL_CreateTextureFromSurface(r,surface);
        SDL_FreeSurface(surface);
    }
    free(rgba);
    if(!texture) {
        fprintf(stderr,"[title-ui] unable to create %s: %s\n",path,SDL_GetError());
        return NULL;
    }
    SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(texture,SDL_ScaleModeNearest);
    if(w) *w=width;
    if(h) *h=height;
    return texture;
}
int Expanded_TitleOverlay_Initialize(SDL_Renderer *renderer)
{
    if(!renderer) return 0;
    unsigned w=0,h=0;
    SDL_Texture *image=texture_from_png(renderer,s_background_path,&w,&h);
    if(image && w==EXP_TITLE_W && h==EXP_TITLE_H) s_background=image;
    else {
        if(image) SDL_DestroyTexture(image);
        fprintf(stderr,"[title-ui] approved background not available (%ux%u): black fallback\n",w,h);
    }
    w=h=0;
    image=texture_from_png(renderer,s_logo_path,&w,&h);
    if(image && w==LOGO_W && h==LOGO_H) s_logo=image;
    else {
        if(image) SDL_DestroyTexture(image);
        fprintf(stderr,"[title-ui] logo not available (%ux%u), keeping title usable\n",w,h);
    }
    for(int i=0;i<EXP_TITLE_MAIN_COUNT;i++)
        s_buttons[i]=texture_from_png(renderer,s_button_paths[i],
                           &s_button_widths[i],&s_button_heights[i]);
    fprintf(stderr,"[title-ui] RGB/RGBA title initialized; legacy title background/buttons and F8 disabled\n");
    return 1;
}
void Expanded_TitleOverlay_Destroy(void)
{
    if(s_background) SDL_DestroyTexture(s_background);
    if(s_logo) SDL_DestroyTexture(s_logo);
    s_background=s_logo=NULL;
    for(int i=0;i<EXP_TITLE_MAIN_COUNT;i++) {
        if(s_buttons[i]) SDL_DestroyTexture(s_buttons[i]);
        s_buttons[i]=NULL;
        s_button_widths[i]=s_button_heights[i]=0;
    }
}
/* All on-screen coordinates use the same 1664x936 canvas as the approved
 * artwork. The host scales the canvas to its target, never a GIF/spritesheet.
 * Nearest-neighbour for button textures retains the source pixel edges. */
static SDL_FRect scale_rect(ExpandedTitleRect r,float sx,float sy)
{
    SDL_FRect out={(float)r.x*sx,(float)r.y*sy,(float)r.w*sx,(float)r.h*sy};
    return out;
}
static void pixel_text(SDL_Renderer *r,const char *msg,int cx,int y,int size,
                       SDL_Color color,float sx,float sy)
{
    const int w=(int)strlen(msg)*size*6;
    int x=cx-w/2;
    SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r,color.r,color.g,color.b,color.a);
    for(const unsigned char *p=(const unsigned char*)msg;*p;p++,x+=6*size) {
        unsigned ch=*p;
        if(ch<32 || ch>126) ch='?';
        const unsigned char *glyph=cb_glyphs[ch-32];
        for(int row=0;row<8;row++) {
            for(int col=0;col<5;col++) {
                if(glyph[row]&(1u<<(4-col))) {
                    SDL_FRect dot={(float)(x+col*size)*sx,
                                   (float)(y+row*size)*sy,
                                   (float)size*sx,(float)size*sy};
                    SDL_RenderFillRectF(r,&dot);
                }
            }
        }
    }
}
static void focus_frame(SDL_Renderer *r,ExpandedTitleRect box,
                        float sx,float sy)
{
    const SDL_Color focus={255,226,74,255};
    SDL_SetRenderDrawColor(r,focus.r,focus.g,focus.b,255);
    SDL_FRect d=scale_rect(box,sx,sy);
    SDL_RenderDrawRectF(r,&d);
}
static const char *classic_label(int i)
{
    static const char *const labels[10] = {
        "CREDITS", "LOAD OLD COMPETITION", "EDIT TACTICS",
        "EDIT CUSTOM TEAM", "HIGHLIGHTS", "FRIENDLY",
        "DIY COMPETITION", "PRESET COMPETITION", "SEASON", "BACK"
    };
    if(Expanded_TitleMenu_Level()==1 && i>=9 && i<=11) {
        extern int Expanded_TitleMenu_ClassicHasContest(void);
        extern int Expanded_TitleMenu_ClassicReplay(void);
        if(Expanded_TitleMenu_ClassicHasContest()) {
            if(i==9) return Expanded_TitleMenu_ClassicReplay() ?
                "REPLAY COMPETITION" : "CONTINUE COMPETITION";
            if(i==10) return "SAVE COMPETITION";
            return "BACK";
        }
    }
    return i>=0 && i<10?labels[i]:"";
}
void Expanded_TitleOverlay_Draw(SDL_Renderer *renderer,
                                float target_w,float target_h)
{
    if(!renderer || target_w<=0 || target_h<=0) return;
    const float sx=target_w/(float)EXP_TITLE_W,sy=target_h/(float)EXP_TITLE_H;
    const int level=Expanded_TitleMenu_Level();
    const int sel=Expanded_TitleMenu_Selected();
    const int press=Expanded_TitleMenu_Pressing();
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);
    if(level==0) {
        if(s_background) {
            SDL_FRect bg={0,0,target_w,target_h};
            SDL_RenderCopyF(renderer,s_background,NULL,&bg);
        }
        if(s_logo) {
            ExpandedTitleRect logo={LOGO_X,LOGO_Y,LOGO_DST_W,LOGO_DST_H};
            SDL_FRect d=scale_rect(logo,sx,sy);
            SDL_RenderCopyF(renderer,s_logo,NULL,&d);
        }
        for(int i=0;i<EXP_TITLE_MAIN_COUNT;i++) {
            ExpandedTitleRect box=expanded_title_main_rects[i];
            const int moving=(sel==i && press>0);
            /* Six game ticks: travel down to the dark lower/right extrusion
             * and come back. Each frame uses the ORIGINAL full-res PNG. */
            int sink=moving?(press>=5?3:press>=3?7:press>=2?10:6):0;
            SDL_FRect dest=scale_rect(box,sx,sy);
            dest.y+=(float)sink*sy;
            dest.h-=(float)sink*sy;
            if(s_buttons[i]) {
                SDL_RenderCopyF(renderer,s_buttons[i],NULL,&dest);
            } else {
                SDL_SetRenderDrawColor(renderer,13,63,145,255);
                SDL_RenderFillRectF(renderer,&dest);
                pixel_text(renderer,s_main_labels[i],box.x+box.w/2,
                           box.y+box.h/2-16,4,(SDL_Color){250,250,255,255},sx,sy);
            }
            if(sel==i && !moving) focus_frame(renderer,box,sx,sy);
        }
        if(Expanded_TitleMenu_CareerActive()) {
            /* CAREER remains the approved unmodified PNG. State-dependent
             * CONTINUE appears as independent dynamic text above its label. */
            ExpandedTitleRect box=expanded_title_main_rects[0];
            pixel_text(renderer,"CONTINUE",box.x+box.w/2,box.y+18,3,
                       (SDL_Color){252,250,207,255},sx,sy);
        }
    } else {
        /* Provisional classic submenu has no illustrated background.
         * No native SWOS blue panels, logos, map or buttons are composited. */
        extern int Expanded_TitleMenu_ClassicHasContest(void);
        int n=Expanded_TitleMenu_ClassicHasContest()?12:10;
        pixel_text(renderer,"CLASSIC MODES",EXP_TITLE_W/2,88,5,
                   (SDL_Color){133,228,255,255},sx,sy);
        for(int i=0;i<n;i++) {
            ExpandedTitleRect box=Expanded_TitleClassicRect(i);
            int sink=(i==sel && press>0)?6:0;
            if(i==sel) {
                SDL_SetRenderDrawColor(renderer,44,101,189,130);
                SDL_FRect d=scale_rect(box,sx,sy);
                d.y+=(float)sink*sy;
                SDL_RenderFillRectF(renderer,&d);
                if(!sink) focus_frame(renderer,box,sx,sy);
            }
            pixel_text(renderer,classic_label(i),box.x+box.w/2,
                       box.y+7+sink,4,
                       i==sel?(SDL_Color){255,242,174,255}:
                               (SDL_Color){238,247,255,255},sx,sy);
        }
    }
    {
        char label[80];
        snprintf(label,sizeof(label),"BUILD %s",Build_DisplayVersion());
        pixel_text(renderer,label,EXP_TITLE_W/2,902,2,
                   (SDL_Color){240,95,81,255},sx,sy);
    }
}
