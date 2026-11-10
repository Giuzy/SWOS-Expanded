/* SWOS Expanded 0.10.66: Title labels use the APPROVED NEW CAREER bitmap atlas.
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
/* Allow real pixel-art glyph outlines to breathe. Raw atlas advances can be
 * smaller than the visible dark-outline width: this was causing overlapping
 * text, especially CLASSIC MODES. Shared in measurement AND placement. */
static unsigned Expanded_TitleUI65_Code(unsigned code)
{
    if(code>='a' && code<='z')code=code-'a'+'A';
    return code>=32 && code<=126 ? code : '?';
}
static float Expanded_TitleUI65_Step(const ExpandedTitleUIFont *f,
                                     unsigned a,unsigned b)
{
    const ExpandedTitleUIGlyph *g=&f->glyph[a];
    const ExpandedTitleUIGlyph *h=&f->glyph[b];
    float step=(float)g->advance;
    if(g->src.w>0 && h->src.w>0) {
        /* +2 atlas pixels between neighboring opaque glyph rectangles. */
        float noTouch=(float)(g->bx+g->src.w-h->bx+2);
        if(step<noTouch)step=noTouch;
    }
    return step;
}
static int Expanded_TitleUI65_DrawLine(SDL_Renderer *r,const char *text,
                                        ExpandedTitleUIFont *f,SDL_FRect box,
                                        float baseScale,int selected,
                                        float sx,float sy,int classic)
{
    if(!text || !f->texture || !f->glyph['A'].advance)return 0;
    float pen=0.f;
    int minY=INT_MAX,maxY=INT_MIN;
    for(const unsigned char *p=(const unsigned char *)text;*p;p++){
        unsigned ch=Expanded_TitleUI65_Code(*p);
        ExpandedTitleUIGlyph *g=&f->glyph[ch];
        if(g->src.w>0 && g->src.h>0){
            if(g->by<minY)minY=g->by;
            if(g->by+g->src.h>maxY)maxY=g->by+g->src.h;
        }
        pen+=p[1]?Expanded_TitleUI65_Step(f,ch,Expanded_TitleUI65_Code(p[1])):
                  (float)g->advance;
    }
    if(minY==INT_MAX || pen<=0.f || maxY<=minY)return 0;
    float scale=baseScale*(sx<sy?sx:sy);
    float maxWidth=box.w-24.f*sx;
    if(classic)maxWidth*=0.955f; /* requested slightly narrower CLASSIC MODES */
    const float maxHeight=box.h*0.90f;
    if(maxWidth<=0.f || maxHeight<=0.f)return 0;
    if(pen*scale>maxWidth)scale=maxWidth/pen;
    if((maxY-minY)*scale>maxHeight)scale=maxHeight/(float)(maxY-minY);
    const float beginX=box.x+(box.w-pen*scale)*0.5f;
    const float originY=box.y+(box.h-(maxY-minY)*scale)*0.5f-minY*scale;
    float atX=beginX;
    SDL_SetTextureColorMod(f->texture,255,selected?226:255,selected?72:255);
    for(const unsigned char *p=(const unsigned char *)text;*p;p++){
        unsigned ch=Expanded_TitleUI65_Code(*p);
        ExpandedTitleUIGlyph *g=&f->glyph[ch];
        if(g->src.w>0 && g->src.h>0){
            SDL_FRect dst={atX+g->bx*scale,originY+g->by*scale,
                           g->src.w*scale,g->src.h*scale};
            dst.x=(float)((int)(dst.x+0.5f));
            dst.y=(float)((int)(dst.y+0.5f));
            SDL_RenderCopyF(r,f->texture,&g->src,&dst);
        }
        atX+=(p[1]?Expanded_TitleUI65_Step(f,ch,Expanded_TitleUI65_Code(p[1])):
                    (float)g->advance)*scale;
    }
    SDL_SetTextureColorMod(f->texture,255,255,255);
    return 1;
}
static int Expanded_TitleUI65_DrawButtonText(SDL_Renderer *r,const char *text,
                                               SDL_FRect button,int primary,
                                               int continue_active,int selected,
                                               float sx,float sy)
{
    /* CONTINUE CAREER uses two independently measured and centered lines.
     * This preserves the approved Title button art and green recolouring. */
    if(continue_active){
        ExpandedTitleUIFont *f=&s_exp_title_fonts[0];
        SDL_FRect top={button.x,button.y+button.h*0.195f,
                       button.w,button.h*0.35f};
        SDL_FRect bottom={button.x,button.y+button.h*0.455f,
                          button.w,button.h*0.35f};
        if(!Expanded_TitleUI65_DrawLine(r,"CONTINUE",f,top,1.0f,
                                          selected,sx,sy,0))return 0;
        return Expanded_TitleUI65_DrawLine(r,"CAREER",f,bottom,1.0f,
                                            selected,sx,sy,0);
    }
    ExpandedTitleUIFont *f=&s_exp_title_fonts[primary?1:0];
    return Expanded_TitleUI65_DrawLine(r,text,f,button,primary?1.0f:0.86f,
                    selected,sx,sy,strcmp(text,"CLASSIC MODES")==0);
}
static void Expanded_TitleUI65_DrawLegend(SDL_Renderer *r,float sx,float sy)
{
    if(!s_exp_title_legend)return;
    /* 10% larger, keep the 12px right margin. The bottom is exactly at
     * y=871, the lower edge of the Title's EXIT GAME button hitbox. */
    const SDL_FRect dest={1518.f*sx,775.f*sy,134.f*sx,96.f*sy};
    SDL_RenderCopyF(r,s_exp_title_legend,NULL,&dest);
}
#endif
