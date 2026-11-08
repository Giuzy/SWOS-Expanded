/* SWOS Expanded — HD title background, independent of the Amiga match engine.
 * The current menu's 8-bit artwork is redrawn on every menu update. Here we
 * remove ONLY pixels identical to that artwork, leaving current button text,
 * focus borders, entry labels and build number in the original renderer. */
#include "expanded_title_overlay.h"
#include "expanded_png.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { TITLE_W=320, TITLE_H=256, HD_W=1664, HD_H=936 };
static const char *const k_png = "assets/title/title_background_approved_2026-11-08.png";
static const char *const k_old = "data/expanded/expanded_title_bg.raw";
static SDL_Texture *s_background;
static uint8_t s_original[TITLE_W * TITLE_H];
static int s_original_ok;
static int s_show_new = 1;

int Expanded_TitleOverlay_Initialize(SDL_Renderer *renderer)
{
    if (!renderer) return 0;
    FILE *f=fopen(k_old,"rb");
    if (!f) {
        fprintf(stderr,"[title-hd] legacy RAW missing: %s (using old renderer)\n",k_old);
        return 0;
    }
    s_original_ok = fread(s_original,1,sizeof s_original,f)==sizeof s_original;
    fclose(f);
    if (!s_original_ok) {
        fprintf(stderr,"[title-hd] legacy RAW incomplete: %s\n",k_old);
        return 0;
    }
    unsigned w=0,h=0;
    unsigned char *rgba=Expanded_PNG_LoadRGBA(k_png,&w,&h);
    if (!rgba) {
        fprintf(stderr,"[title-hd] PNG missing/invalid: %s; original title retained\n",k_png);
        return 0;
    }
    if (w!=HD_W || h!=HD_H) {
        fprintf(stderr,"[title-hd] wrong PNG dimensions: %ux%u, expected %dx%d\n",
                w,h,HD_W,HD_H);
        free(rgba);
        return 0;
    }
    /* Decoder RGBA byte order is ABGR8888 on little-endian Windows x64. */
    SDL_Surface *surface=SDL_CreateRGBSurfaceWithFormatFrom(
        rgba,(int)w,(int)h,32,(int)w*4,SDL_PIXELFORMAT_ABGR8888);
    if (surface) {
        s_background=SDL_CreateTextureFromSurface(renderer,surface);
        SDL_FreeSurface(surface);
    }
    free(rgba);
    if (!s_background) {
        fprintf(stderr,"[title-hd] SDL texture unavailable: %s; original retained\n",SDL_GetError());
        return 0;
    }
    SDL_SetTextureBlendMode(s_background,SDL_BLENDMODE_NONE);
    SDL_SetTextureScaleMode(s_background,SDL_ScaleModeLinear);
    s_show_new=1;
    fprintf(stderr,"[title-hd] loaded %s (%dx%d, PNG RGB); F8: new/original\n",
            k_png,HD_W,HD_H);
    return 1;
}
void Expanded_TitleOverlay_Destroy(void)
{
    if (s_background) SDL_DestroyTexture(s_background);
    s_background=NULL;
    s_original_ok=0;
}
int Expanded_TitleOverlay_Ready(void) {return s_background!=NULL && s_original_ok;}
int Expanded_TitleOverlay_Enabled(void) {return Expanded_TitleOverlay_Ready() && s_show_new;}
void Expanded_TitleOverlay_Toggle(void)
{
    if (!Expanded_TitleOverlay_Ready()) return;
    s_show_new=!s_show_new;
    fprintf(stderr,"[title-hd] F8: %s background\n",s_show_new?"PNG":"original");
}
void Expanded_TitleOverlay_Mask(uint32_t *argb,int dst_pitch,
                                const uint8_t *indexed,int src_pitch,
                                int frame_w,int frame_h)
{
    if (!Expanded_TitleOverlay_Enabled() || !argb || !indexed ||
        dst_pitch<TITLE_W || src_pitch<TITLE_W) return;
    int w=frame_w<TITLE_W?frame_w:TITLE_W;
    int h=frame_h<TITLE_H?frame_h:TITLE_H;
    for (int y=0;y<h;y++) {
        const uint8_t *src=indexed+(size_t)y*src_pitch;
        const uint8_t *ref=s_original+(size_t)y*TITLE_W;
        uint32_t *dst=argb+(size_t)y*dst_pitch;
        for (int x=0;x<w;x++)
            if (src[x]==ref[x]) dst[x]&=0x00FFFFFFu;
    }
}
void Expanded_TitleOverlay_Draw(SDL_Renderer *renderer,SDL_Texture *frame,
                                const SDL_Rect *src,int whole_scale,
                                float target_w,float target_h)
{
    if (!Expanded_TitleOverlay_Enabled()) return;
    SDL_SetTextureBlendMode(frame,SDL_BLENDMODE_BLEND);
    if (whole_scale==1) {
        SDL_RenderCopy(renderer,s_background,NULL,NULL);
        SDL_RenderCopy(renderer,frame,src,NULL);
    } else {
        SDL_FRect dest={0.0f,0.0f,target_w,target_h};
        SDL_RenderCopyF(renderer,s_background,NULL,&dest);
        SDL_RenderCopyF(renderer,frame,src,&dest);
    }
    SDL_SetTextureBlendMode(frame,SDL_BLENDMODE_NONE);
}
