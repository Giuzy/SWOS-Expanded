/* SWOS Expanded — modern SDL2 Title Menu (0.10.54).
 * No conversion to the 320x256 SWOS pixel framebuffer and no baked-in
 * old menu art. All five user-selected buttons are independent RGBA sprites.
 * The original SWOS callbacks are preserved in menus.c, but their old
 * buttons and background are never presented in this menu. */
#include "expanded_title_overlay.h"
#include "expanded_title_layout.h"
#include "expanded_png.h"
#include "expanded_club_font.h"
#include "build_info.h"
#include "platform_sdl.h" /* live fullscreen state, shared with Alt+Enter */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* SIZE_MAX for safe sprite-outline allocation */
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
/* Approved base sprite for an existing career. There must be NO runtime
 * CONTINUE inscription overlay: the entire caption is baked into a clean
 * RGBA PNG, and the yellow focus is still generated dynamically. */
static const char *const s_continue_career_path =
    "assets/title/buttons/continue_career.png";
/* Two separately approved, clean RGBA sprites. Show only one at a time:
 * while WINDOWED display outward arrows (enter fullscreen); while
 * FULLSCREEN display inward arrows (restore windowed). */
static const char *const s_video_paths[2] = {
    "assets/title/buttons/fullscreen_expand.png",
    "assets/title/buttons/windowed_collapse.png"
};
static const char *const s_main_labels[EXP_TITLE_MAIN_COUNT] = {
    "CAREER","HALL OF FAME","CLASSIC MODES","OPTIONS","EXIT GAME"
};
/* Each selected button gets a cached yellow alpha-outline matching the
 * *entire* visible silhouette, including the lower and right 3D extrusion.
 * Hitboxes remain rectangular only for input handling. */
static SDL_Texture *s_background,*s_logo,*s_buttons[EXP_TITLE_MAIN_COUNT];
static SDL_Texture *s_button_focus[EXP_TITLE_MAIN_COUNT];
static SDL_Texture *s_continue_career,*s_continue_focus;
static SDL_Texture *s_video[2],*s_video_focus[2];
static unsigned s_continue_width,s_continue_height;
static unsigned s_video_width[2],s_video_height[2];
static unsigned s_button_widths[EXP_TITLE_MAIN_COUNT],s_button_heights[EXP_TITLE_MAIN_COUNT];
enum { LOGO_W=1200, LOGO_H=440, LOGO_X=920, LOGO_Y=17, LOGO_DST_W=620, LOGO_DST_H=227 };

/* Generate a crisp focus ring around the actual alpha contour of the source
 * PNG, rather than SDL_RenderDrawRectF around its rectangular hitbox. The
 * RGBA art is untouched; the selection is an independent texture cached
 * once on loading. The alpha includes the dark extrusion on the bottom and
 * right, so the ring follows that too. */
/* Square alpha dilation using separable sliding windows O(width*height).
 * The old per-pixel radius² scan became expensive for high-resolution
 * corner icons. The outline remains a separate pixel-perfect texture;
 * no approved PNG is edited and no 50 Hz render tick performs dilation. */
static SDL_Texture *focus_from_rgba(SDL_Renderer *r,
                                    const unsigned char *rgba,
                                    unsigned w,unsigned h,int radius)
{
    if(!r || !rgba || !w || !h || radius<=0) return NULL;
    const size_t n=(size_t)w*(size_t)h;
    if(n>SIZE_MAX/4) return NULL;
    unsigned char *ring=(unsigned char*)calloc(n,4);
    unsigned char *horizontal=(unsigned char*)malloc(n);
    if(!ring || !horizontal) {
        free(ring);
        free(horizontal);
        return NULL;
    }
    /* Pass one: any opaque alpha in a horizontal radius around the pixel. */
    for(unsigned y=0;y<h;y++) {
        unsigned count=0;
        unsigned end=(unsigned)radius<w?(unsigned)radius:w-1;
        for(unsigned x=0;x<=end;x++)
            if(rgba[((size_t)y*w+x)*4+3]>127) count++;
        for(unsigned x=0;x<w;x++) {
            horizontal[(size_t)y*w+x]=(unsigned char)(count!=0);
            int remove=(int)x-radius;
            int add=(int)x+radius+1;
            if(remove>=0 && remove<(int)w &&
               rgba[((size_t)y*w+(unsigned)remove)*4+3]>127) count--;
            if(add>=0 && add<(int)w &&
               rgba[((size_t)y*w+(unsigned)add)*4+3]>127) count++;
        }
    }
    /* Pass two: vertical radius over the horizontal result. Pixels
     * inside the original silhouette stay fully transparent in the ring. */
    for(unsigned x=0;x<w;x++) {
        unsigned count=0;
        unsigned end=(unsigned)radius<h?(unsigned)radius:h-1;
        for(unsigned y=0;y<=end;y++)
            count+=horizontal[(size_t)y*w+x];
        for(unsigned y=0;y<h;y++) {
            size_t p=(size_t)y*w+x;
            if(rgba[p*4+3]<=127 && count) {
                ring[p*4+0]=255;
                ring[p*4+1]=220;
                ring[p*4+2]=52;
                ring[p*4+3]=255;
            }
            int remove=(int)y-radius;
            int add=(int)y+radius+1;
            if(remove>=0 && remove<(int)h)
                count-=horizontal[(size_t)(unsigned)remove*w+x];
            if(add>=0 && add<(int)h)
                count+=horizontal[(size_t)(unsigned)add*w+x];
        }
    }
    free(horizontal);
    SDL_Surface *surface=SDL_CreateRGBSurfaceWithFormatFrom(
        ring,(int)w,(int)h,32,(int)w*4,SDL_PIXELFORMAT_ABGR8888);
    SDL_Texture *focus=NULL;
    if(surface) {
        focus=SDL_CreateTextureFromSurface(r,surface);
        SDL_FreeSurface(surface);
    }
    free(ring);
    if(focus) {
        SDL_SetTextureBlendMode(focus,SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(focus,SDL_ScaleModeNearest);
    }
    return focus;
}
static SDL_Texture *texture_from_png(SDL_Renderer *r,const char *path,
                                     unsigned *w,unsigned *h,
                                     SDL_Texture **focus,int focus_radius)
{
    if(focus) *focus=NULL;
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
    if(texture && focus)
        *focus=focus_from_rgba(r,rgba,width,height,focus_radius);
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
    SDL_Texture *image=texture_from_png(renderer,s_background_path,&w,&h,NULL,0);
    if(image && w==EXP_TITLE_W && h==EXP_TITLE_H) s_background=image;
    else {
        if(image) SDL_DestroyTexture(image);
        fprintf(stderr,"[title-ui] approved background not available (%ux%u): black fallback\n",w,h);
    }
    w=h=0;
    image=texture_from_png(renderer,s_logo_path,&w,&h,NULL,0);
    if(image && w==LOGO_W && h==LOGO_H) s_logo=image;
    else {
        if(image) SDL_DestroyTexture(image);
        fprintf(stderr,"[title-ui] logo not available (%ux%u), keeping title usable\n",w,h);
    }
    for(int i=0;i<EXP_TITLE_MAIN_COUNT;i++)
        s_buttons[i]=texture_from_png(renderer,s_button_paths[i],
                           &s_button_widths[i],&s_button_heights[i],
                           &s_button_focus[i],6);
    s_continue_career=texture_from_png(renderer,s_continue_career_path,
                                      &s_continue_width,&s_continue_height,
                                      &s_continue_focus,12);
    for(int i=0;i<2;i++)
        s_video[i]=texture_from_png(renderer,s_video_paths[i],
                                   &s_video_width[i],&s_video_height[i],
                                   &s_video_focus[i],i==0?22:18);
    if(!s_continue_career)
        fprintf(stderr,"[title-ui] CONTINUE CAREER PNG missing; please extract the approved assets ZIP\n");
    fprintf(stderr,"[title-ui] RGB/RGBA title initialized; legacy title background/buttons and F8 disabled\n");
    return 1;
}
void Expanded_TitleOverlay_Destroy(void)
{
    if(s_background) SDL_DestroyTexture(s_background);
    if(s_logo) SDL_DestroyTexture(s_logo);
    if(s_continue_career) SDL_DestroyTexture(s_continue_career);
    if(s_continue_focus) SDL_DestroyTexture(s_continue_focus);
    s_background=s_logo=s_continue_career=s_continue_focus=NULL;
    s_continue_width=s_continue_height=0;
    for(int i=0;i<2;i++) {
        if(s_video[i]) SDL_DestroyTexture(s_video[i]);
        if(s_video_focus[i]) SDL_DestroyTexture(s_video_focus[i]);
        s_video[i]=s_video_focus[i]=NULL;
        s_video_width[i]=s_video_height[i]=0;
    }
    for(int i=0;i<EXP_TITLE_MAIN_COUNT;i++) {
        if(s_buttons[i]) SDL_DestroyTexture(s_buttons[i]);
        if(s_button_focus[i]) SDL_DestroyTexture(s_button_focus[i]);
        s_buttons[i]=NULL;
        s_button_focus[i]=NULL;
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
/* Provisional Classic Modes rows have no button PNG yet: their focus
 * outline follows their actual rectangular temporary highlight panel. */
static void focus_frame(SDL_Renderer *r,ExpandedTitleRect box,
                        float sx,float sy)
{
    const SDL_Color focus={255,226,74,255};
    SDL_SetRenderDrawColor(r,focus.r,focus.g,focus.b,255);
    SDL_FRect d=scale_rect(box,sx,sy);
    SDL_RenderDrawRectF(r,&d);
}
static int title_press_offset(int frames)
{
    /* A short, non-elastic push: 2 -> 4 -> 4 -> 2 -> rest.
     * Translation only, never squash/recompress the approved PNG. */
    if(frames<=0) return 0;
    return frames>=4?2:frames>=2?4:2;
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
            /* Original full-resolution PNG rendered unchanged: a gentle
             * downward press, no vertical resizing, zoom or bounce. */
            int sink=moving?title_press_offset(press):0;
            SDL_FRect dest=scale_rect(box,sx,sy);
            const int continue_active=(i==0 && Expanded_TitleMenu_CareerActive());
            SDL_Texture *sprite=continue_active && s_continue_career ?
                                s_continue_career:s_buttons[i];
            SDL_Texture *focus=continue_active && s_continue_career ?
                               s_continue_focus:s_button_focus[i];
            if(continue_active && s_continue_career &&
               s_continue_width>0 && s_continue_height>0) {
                /* The new approved CONTINUE CAREER sprite is a different
                 * aspect ratio from CAREER. Fit INSIDE the same logical
                 * hitbox without stretching or resampling its proportions. */
                const float srcRatio=(float)s_continue_width/(float)s_continue_height;
                const float boxRatio=(float)box.w/(float)box.h;
                if(srcRatio>boxRatio) {
                    const float fitH=(float)box.w/srcRatio;
                    dest.y+=((float)box.h-fitH)*0.5f*sy;
                    dest.h=fitH*sy;
                } else {
                    const float fitW=(float)box.h*srcRatio;
                    dest.x+=((float)box.w-fitW)*0.5f*sx;
                    dest.w=fitW*sx;
                }
            }
            dest.y+=(float)sink*sy;
            if(sprite) {
                /* The yellow focus tracks the complete alpha silhouette
                 * including the dark bottom-right depth, while pressed. */
                if(sel==i && focus)
                    SDL_RenderCopyF(renderer,focus,NULL,&dest);
                SDL_RenderCopyF(renderer,sprite,NULL,&dest);
            } else {
                SDL_SetRenderDrawColor(renderer,13,63,145,255);
                SDL_RenderFillRectF(renderer,&dest);
                pixel_text(renderer,s_main_labels[i],box.x+box.w/2,
                           box.y+box.h/2-16+sink,4,
                           (SDL_Color){250,250,255,255},sx,sy);
                if(sel==i) focus_frame(renderer,box,sx,sy);
            }
        }
        /* Sixth selectable item, visually separate in the upper-left.
         * The icon shows the ACTION available, not a stale mode hint.
         * Query SDL's actual flags each frame: mouse, gamepad and Alt+Enter
         * all update the symbol without a duplicate state variable. */
        {
            const int videoMode=Platform_GetFullscreenMode()!=PLATFORM_FULLSCREEN_NONE;
            SDL_Texture *icon=s_video[videoMode];
            SDL_Texture *focus=s_video_focus[videoMode];
            ExpandedTitleRect box=expanded_title_video_rect;
            SDL_FRect dest=scale_rect(box,sx,sy);
            int push=sel==EXP_TITLE_VIDEO_INDEX?title_press_offset(press):0;
            if(icon && s_video_width[videoMode] && s_video_height[videoMode]) {
                float sourceRatio=(float)s_video_width[videoMode]/(float)s_video_height[videoMode];
                float targetRatio=(float)box.w/(float)box.h;
                if(sourceRatio>targetRatio) {
                    float fitted=(float)box.w/sourceRatio;
                    dest.y+=((float)box.h-fitted)*0.5f*sy;
                    dest.h=fitted*sy;
                } else {
                    float fitted=(float)box.h*sourceRatio;
                    dest.x+=((float)box.w-fitted)*0.5f*sx;
                    dest.w=fitted*sx;
                }
                dest.y+=push*sy;
                if(sel==EXP_TITLE_VIDEO_INDEX && focus)
                    SDL_RenderCopyF(renderer,focus,NULL,&dest);
                SDL_RenderCopyF(renderer,icon,NULL,&dest);
            } else {
                /* Still visibly operable if optional sprites were not
                 * extracted yet; do not fall back to old SWOS title art. */
                dest.y+=push*sy;
                SDL_SetRenderDrawColor(renderer,16,61,143,255);
                SDL_RenderFillRectF(renderer,&dest);
                if(sel==EXP_TITLE_VIDEO_INDEX) focus_frame(renderer,box,sx,sy);
                /* Placeholder only when an approved PNG is missing:
                 * four simple arrows, never an unapproved text label. */
                SDL_SetRenderDrawColor(renderer,242,250,255,255);
                for(int yi=-1;yi<=1;yi+=2)
                    for(int xi=-1;xi<=1;xi+=2) {
                        float cx=(box.x+box.w/2)*sx;
                        float cy=(box.y+box.h/2+push)*sy;
                        int dir=videoMode?-1:1;
                        float ex=cx+(float)(xi*dir*27)*sx;
                        float ey=cy+(float)(yi*dir*25)*sy;
                        SDL_RenderDrawLineF(renderer,cx,cy,ex,ey);
                        SDL_RenderDrawLineF(renderer,ex,ey,ex-(float)(xi*dir*8)*sx,ey);
                        SDL_RenderDrawLineF(renderer,ex,ey,ex,ey-(float)(yi*dir*8)*sy);
                    }
            }
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
            int sink=(i==sel)?title_press_offset(press):0;
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
        snprintf(label,sizeof(label),"%s",Build_DisplayVersion());
        pixel_text(renderer,label,EXP_TITLE_W/2,902,2,
                   (SDL_Color){240,95,81,255},sx,sy);
    }
}
