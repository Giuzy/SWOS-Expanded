/* SWOS Expanded 0.10.71 — approved pixel-art loading scene.
 * SDL2-only, no gameplay or save changes. The PNG layers are independent:
 * background / canonical logo / clean panel / live fill / real atlas text.
 * This header is included only from platform_sdl.c. */
#ifndef EXPANDED_LOADING_01071_H
#define EXPANDED_LOADING_01071_H
#include "expanded_png.h"
#include "mjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { SDL_Rect rect; int advance,bx,by; } ExpandedLoading71Glyph;
static SDL_Texture *s_el71_logo,*s_el71_panel,*s_el71_fill,*s_el71_font;
static ExpandedLoading71Glyph s_el71_glyphs[128];
static int s_el71_font_ok;

static SDL_Texture *ExpandedLoading71_Texture(SDL_Renderer *renderer,const char *path)
{
    unsigned w=0,h=0;
    unsigned char *pixels=Expanded_PNG_LoadRGBA(path,&w,&h);
    if(!pixels || !w || !h){free(pixels);return NULL;}
    SDL_Surface *surface=SDL_CreateRGBSurfaceWithFormatFrom(
        pixels,(int)w,(int)h,32,(int)w*4,SDL_PIXELFORMAT_ABGR8888);
    SDL_Texture *t=surface?SDL_CreateTextureFromSurface(renderer,surface):NULL;
    if(surface)SDL_FreeSurface(surface);
    free(pixels);
    if(t){
        SDL_SetTextureBlendMode(t,SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(t,SDL_ScaleModeNearest);
    }
    return t;
}
static int ExpandedLoading71_GetMetric(const char *js,int n,int ch,const char *key)
{
    char path[96];double d=0;
    snprintf(path,sizeof(path),"$.glyphs.%d.%s",ch,key);
    return mjson_get_number(js,n,path,&d)?(int)d:0;
}
static void ExpandedLoading71_LoadFont(SDL_Renderer *r)
{
    s_el71_font=ExpandedLoading71_Texture(r,"assets/fonts/new_career/ui_form.png");
    FILE *file=fopen("assets/fonts/new_career/ui_form.json","rb");
    if(!file)return;
    if(fseek(file,0,SEEK_END)!=0){fclose(file);return;}
    long n=ftell(file);
    if(n<=0 || n>1000000){fclose(file);return;}
    rewind(file);
    char *js=(char*)calloc((size_t)n+1,1);
    if(!js){fclose(file);return;}
    size_t count=fread(js,1,(size_t)n,file);
    fclose(file);
    for(int ch=32;ch<127;ch++){
        ExpandedLoading71Glyph *g=&s_el71_glyphs[ch];
        g->rect=(SDL_Rect){
            ExpandedLoading71_GetMetric(js,(int)count,ch,"x"),
            ExpandedLoading71_GetMetric(js,(int)count,ch,"y"),
            ExpandedLoading71_GetMetric(js,(int)count,ch,"w"),
            ExpandedLoading71_GetMetric(js,(int)count,ch,"h")};
        g->advance=ExpandedLoading71_GetMetric(js,(int)count,ch,"advance");
        g->bx=ExpandedLoading71_GetMetric(js,(int)count,ch,"bearingX");
        g->by=ExpandedLoading71_GetMetric(js,(int)count,ch,"bearingY");
    }
    s_el71_font_ok=s_el71_font && s_el71_glyphs['A'].advance>0;
    free(js);
}
static void ExpandedLoading71_Text(SDL_Renderer *r,const char *text,
                                   float centerX,float topY,float scale)
{
    if(!s_el71_font_ok || !text)return;
    float width=0;
    for(const unsigned char *p=(const unsigned char*)text;*p;p++){
        unsigned ch=*p;
        if(ch>='a' && ch<='z')ch=ch-'a'+'A';
        if(ch>126 || ch<32)ch='?';
        width+=s_el71_glyphs[ch].advance*scale;
    }
    float x=centerX-width*.5f;
    SDL_SetTextureColorMod(s_el71_font,255,255,255);
    for(const unsigned char *p=(const unsigned char*)text;*p;p++){
        unsigned ch=*p;
        if(ch>='a' && ch<='z')ch=ch-'a'+'A';
        if(ch>126 || ch<32)ch='?';
        ExpandedLoading71Glyph *g=&s_el71_glyphs[ch];
        if(g->rect.w && g->rect.h){
            SDL_FRect dst={x+g->bx*scale,topY+g->by*scale,
                g->rect.w*scale,g->rect.h*scale};
            SDL_RenderCopyF(r,s_el71_font,&g->rect,&dst);
        }
        x+=g->advance*scale;
    }
}
static SDL_Texture *ExpandedLoading71_Init(SDL_Renderer *r)
{
    SDL_Texture *bg=ExpandedLoading71_Texture(r,"data/expanded/expanded_loading_bg_approved_2026-11-10.png");
    if(!bg)return NULL;
    s_el71_logo=ExpandedLoading71_Texture(r,
        "assets/title/swos_expanded_logo_approved_2026-11-08.png");
    s_el71_panel=ExpandedLoading71_Texture(r,"assets/loading/loading_panel.png");
    s_el71_fill=ExpandedLoading71_Texture(r,"assets/loading/loading_fill.png");
    ExpandedLoading71_LoadFont(r);
    if(!s_el71_logo || !s_el71_panel || !s_el71_fill || !s_el71_font_ok)
        fprintf(stderr,"[loading71] asset overlay missing: logo=%d panel=%d fill=%d font=%d\n",
          !!s_el71_logo,!!s_el71_panel,!!s_el71_fill,s_el71_font_ok);
    return bg;
}
static void ExpandedLoading71_Destroy(void)
{
    SDL_Texture **textures[]={&s_el71_logo,&s_el71_panel,&s_el71_fill,&s_el71_font};
    for(unsigned i=0;i<sizeof(textures)/sizeof(textures[0]);i++){
        if(*textures[i])SDL_DestroyTexture(*textures[i]);
        *textures[i]=NULL;
    }
    memset(s_el71_glyphs,0,sizeof s_el71_glyphs);
    s_el71_font_ok=0;
}
static void ExpandedLoading71_Draw(SDL_Renderer *r,SDL_Texture *bg,int percent)
{
    if(!r || !bg)return;
    int oldW=0,oldH=0;
    SDL_RenderGetLogicalSize(r,&oldW,&oldH);
    /* Render the approved 1664x936 art at native pixels, restore menu
     * logical coordinates BEFORE returning to Platform_RenderFrame. */
    SDL_RenderSetLogicalSize(r,1664,936);
    SDL_RenderCopy(r,bg,NULL,NULL);
    if(s_el71_logo){
        /* Half the approved Title's displayed 620x227 logo. */
        SDL_Rect logo={24,18,310,114};
        SDL_RenderCopy(r,s_el71_logo,NULL,&logo);
    }
    const SDL_Rect panel={602,794,460,106};
    if(s_el71_panel)SDL_RenderCopy(r,s_el71_panel,NULL,&panel);
    if(s_el71_fill && percent>0){
        /* Source transparent fill is taken directly from approved green
         * artwork. Scale into the EMPTY track without baking in 72%. */
        /* Panel track is x=211..1777 in its 1960px source. At the
         * approved 460px display width its inner span is x=652..1019,
         * i.e. 367 logical pixels. A 360px fill leaves a dark gap even
         * at 100%. Keep real progress milestones, no faked completion. */
        SDL_Rect src={211,245,1222,80};
        SDL_Rect dst={652,851,((367*percent)+99)/100,19};
        if(dst.w>0)SDL_RenderCopy(r,s_el71_fill,&src,&dst);
    }
    /* Center LOADING, not the three animated points; atlas is canonical. */
    if(s_el71_font_ok){
        ExpandedLoading71_Text(r,"LOADING",831,818,0.60f);
        /* Three static dots follow the centered word without shifting it. */
        ExpandedLoading71_Text(r,"...",907,818,0.60f);
        char number[12];snprintf(number,sizeof number,"%d%%",percent);
        ExpandedLoading71_Text(r,number,832,851,0.52f);
    }
    SDL_RenderSetLogicalSize(r,oldW,oldH);
}
#endif
