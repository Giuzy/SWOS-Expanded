/* SWOS Expanded 0.10.65: Title labels use the APPROVED NEW CAREER bitmap atlas.
 * Included only by expanded_title_overlay.c. No separate build objects and no
 * modifications to the historical approved Title source PNGs. */
#ifndef EXPANDED_TITLE_UI_01065_H
#define EXPANDED_TITLE_UI_01065_H
#include "mjson.h"
#include <limits.h>

typedef struct {
    SDL_Rect src;
    int advance, bx, by;
} ExpandedTitleUIGlyph;
typedef struct {
    SDL_Texture *texture;
    ExpandedTitleUIGlyph glyph[128];
} ExpandedTitleUIFont;

static ExpandedTitleUIFont s_exp_title_fonts[2]; /* 0=form, 1=title */
static SDL_Texture *s_exp_title_legend;

static int Expanded_TitleUI65_Metric(const char *json,int len,int ch,const char *field)
{
    char key[92];double n=0.0;
    snprintf(key,sizeof key,"$.glyphs.%d.%s",ch,field);
    return mjson_get_number(json,len,key,&n)?(int)n:0;
}
static SDL_Texture *Expanded_TitleUI65_Png(SDL_Renderer *r,const char *path)
{
    unsigned w=0,h=0;
    unsigned char *pixels=Expanded_PNG_LoadRGBA(path,&w,&h);
    if(!pixels || w==0 || h==0){free(pixels);return NULL;}
    SDL_Surface *s=SDL_CreateRGBSurfaceWithFormatFrom(pixels,(int)w,(int)h,32,(int)w*4,SDL_PIXELFORMAT_ABGR8888);
    SDL_Texture *t=s?SDL_CreateTextureFromSurface(r,s):NULL;
    if(s)SDL_FreeSurface(s);
    free(pixels);
    if(t){SDL_SetTextureBlendMode(t,SDL_BLENDMODE_BLEND);SDL_SetTextureScaleMode(t,SDL_ScaleModeNearest);}
    return t;
}
static void Expanded_TitleUI65_Init(SDL_Renderer *r)
{
    const char *names[2]={"form","title"};
    char path[192];
    for(int i=0;i<2;i++){
        snprintf(path,sizeof path,"assets/fonts/new_career/ui_%s.png",names[i]);
        s_exp_title_fonts[i].texture=Expanded_TitleUI65_Png(r,path);
        snprintf(path,sizeof path,"assets/fonts/new_career/ui_%s.json",names[i]);
        FILE *f=fopen(path,"rb");
        if(!f){fprintf(stderr,"[title-ui] font metrics missing: %s\n",path);continue;}
        if(fseek(f,0,SEEK_END)!=0){fclose(f);continue;}
        long n=ftell(f);rewind(f);
        if(n<=0 || n>1000000){fclose(f);continue;}
        char *json=(char*)calloc((size_t)n+1,1);
        if(!json){fclose(f);continue;}
        size_t got=fread(json,1,(size_t)n,f);fclose(f);
        for(int ch=32;ch<127;ch++){
            ExpandedTitleUIGlyph *g=&s_exp_title_fonts[i].glyph[ch];
            g->src=(SDL_Rect){
                Expanded_TitleUI65_Metric(json,(int)got,ch,"x"),
                Expanded_TitleUI65_Metric(json,(int)got,ch,"y"),
                Expanded_TitleUI65_Metric(json,(int)got,ch,"w"),
                Expanded_TitleUI65_Metric(json,(int)got,ch,"h")};
            g->advance=Expanded_TitleUI65_Metric(json,(int)got,ch,"advance");
            g->bx=Expanded_TitleUI65_Metric(json,(int)got,ch,"bearingX");
            g->by=Expanded_TitleUI65_Metric(json,(int)got,ch,"bearingY");
        }
        free(json);
    }
    s_exp_title_legend=Expanded_TitleUI65_Png(r,"assets/ui/legends/a_select.png");
    if(!s_exp_title_legend)fprintf(stderr,"[title-ui] a_select legend is missing\n");
}
static void Expanded_TitleUI65_Destroy(void)
{
    for(int i=0;i<2;i++){
        if(s_exp_title_fonts[i].texture)SDL_DestroyTexture(s_exp_title_fonts[i].texture);
        memset(&s_exp_title_fonts[i],0,sizeof(s_exp_title_fonts[i]));
    }
    if(s_exp_title_legend)SDL_DestroyTexture(s_exp_title_legend);
    s_exp_title_legend=NULL;
}
static int Expanded_TitleUI65_DrawButtonText(SDL_Renderer *r,const char *text,
                                               SDL_FRect button,int primary,
                                               int continue_active,int selected,float sx,float sy)
{
    ExpandedTitleUIFont *f=&s_exp_title_fonts[primary && !continue_active?1:0];
    if(!text || !f->texture || !f->glyph['A'].advance)return 0;
    float pen=0.f;
    int minY=INT_MAX,maxY=INT_MIN;
    for(const unsigned char *p=(const unsigned char *)text;*p;p++){
        unsigned ch=*p;
        if(ch>='a'&&ch<='z')ch=ch-'a'+'A';
        if(ch<32||ch>126)ch='?';
        ExpandedTitleUIGlyph *g=&f->glyph[ch];
        if(g->src.w>0 && g->src.h>0){
            if(g->by<minY)minY=g->by;
            if(g->by+g->src.h>maxY)maxY=g->by+g->src.h;
        }
        pen+=(float)g->advance;
    }
    if(minY==INT_MAX || pen<=0.f)return 0;
    /* The focused Title label turns YELLOW, exactly as on the Career form.
     * Keep this a runtime tint of the approved pixel atlas: no baked label
     * or highlight in the PNG button. Reset modulation after every draw. */
    const float screen=(sx<sy?sx:sy);
    float scale=(primary && !continue_active?1.0f:continue_active?0.96f:0.86f)*screen;
    float maxWidth=button.w-(primary?36.f:24.f)*sx;
    float maxHeight=button.h*(primary?0.52f:0.61f);
    if(maxWidth<=0 || maxHeight<=0)return 0;
    SDL_SetTextureColorMod(f->texture,255,selected?226:255,selected?72:255);
    if(pen*scale>maxWidth)scale=maxWidth/pen;
    if((maxY-minY)*scale>maxHeight)scale=maxHeight/(float)(maxY-minY);
    float atX=button.x+(button.w-pen*scale)*0.5f;
    const float originY=button.y+(button.h-(maxY-minY)*scale)*0.5f-minY*scale;
    for(const unsigned char *p=(const unsigned char *)text;*p;p++){
        unsigned ch=*p;
        if(ch>='a'&&ch<='z')ch=ch-'a'+'A';
        if(ch<32||ch>126)ch='?';
        ExpandedTitleUIGlyph *g=&f->glyph[ch];
        if(g->src.w>0&&g->src.h>0){
            SDL_FRect dst={atX+g->bx*scale,originY+g->by*scale,
                           g->src.w*scale,g->src.h*scale};
            dst.x=(float)((int)(dst.x+0.5f));
            dst.y=(float)((int)(dst.y+0.5f));
            SDL_RenderCopyF(r,f->texture,&g->src,&dst);
        }
        atX+=g->advance*scale;
    }
    SDL_SetTextureColorMod(f->texture,255,255,255);
    return 1;
}
static void Expanded_TitleUI65_DrawLegend(SDL_Renderer *r,float sx,float sy)
{
    if(!s_exp_title_legend)return;
    /* Same bottom-right margin and height as NEW CAREER's A/B legend. */
    const SDL_FRect dest={1530.f*sx,837.f*sy,122.f*sx,87.f*sy};
    SDL_RenderCopyF(r,s_exp_title_legend,NULL,&dest);
}
#endif
