/* SWOS Expanded PNG texture decoder -- no SDL2_image DLL dependency.
 * The project's already-vendored miniz handles PNG's zlib IDAT stream.
 * Decode once on asset load, never on the 50Hz rendering path.
 * Deliberately rejects interlaced/16-bit/indexed PNG rather than
 * silently misrendering it. New art uses 8-bit RGB/RGBA PNG. */
#include "expanded_png.h"
#include "miniz_swos.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define PNG_LIMIT (128u * 1024u * 1024u)
static uint32_t png_u32(const unsigned char *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
static int png_paeth(int a,int b,int c)
{
    int p=a+b-c;
    int da=abs(p-a),db=abs(p-b),dc=abs(p-c);
    return da<=db && da<=dc ? a : db<=dc ? b : c;
}
static unsigned char *png_load_memory(const unsigned char *png,size_t n,
                                     unsigned *width,unsigned *height)
{
    static const unsigned char sig[8]={137,80,78,71,13,10,26,10};
    unsigned w=0,h=0,bpp=0;
    int header=0,ended=0;
    size_t at=8,clen=0;
    unsigned char *compressed=NULL,*inflated=NULL,*rgba=NULL;
    if (n<33 || memcmp(png,sig,8)!=0) goto fail;
    while (at+12<=n) {
        uint32_t chunk=png_u32(png+at);
        const unsigned char *type=png+at+4,*data=png+at+8;
        if (chunk>PNG_LIMIT || chunk > n-at-12) goto fail;
        if ((uint32_t)mz_crc32(0,type,(size_t)chunk+4)!=
            png_u32(data+chunk)) goto fail;
        if (!memcmp(type,"IHDR",4)) {
            if (header || at!=8 || chunk!=13) goto fail;
            w=png_u32(data);h=png_u32(data+4);
            if (!w || !h || w>8192 || h>8192 || data[8]!=8 ||
                (data[9]!=2 && data[9]!=6) || data[10] || data[11] || data[12])
                goto fail;
            bpp=data[9]==6 ? 4 : 3;
            header=1;
        } else if (!memcmp(type,"IDAT",4)) {
            if (!header || ended || chunk>PNG_LIMIT-clen) goto fail;
            unsigned char *next=(unsigned char *)realloc(compressed,clen+chunk);
            if (!next) goto fail;
            compressed=next;
            memcpy(compressed+clen,data,chunk);
            clen+=chunk;
        } else if (!memcmp(type,"IEND",4)) {
            if (!header || !clen || chunk) goto fail;
            ended=1;
            break;
        } else if ((type[0]&32)==0) {
            /* Unknown critical chunk: cannot be safely ignored. */
            goto fail;
        }
        at+=(size_t)chunk+12;
    }
    if (!ended || !w || !h) goto fail;
    size_t stride=(size_t)w*bpp;
    size_t rowsize=stride+1;
    if (rowsize>PNG_LIMIT/h || (size_t)w*4>PNG_LIMIT/h) goto fail;
    size_t rawsize=rowsize*h, outsize=(size_t)w*h*4;
    inflated=(unsigned char *)malloc(rawsize);
    rgba=(unsigned char *)malloc(outsize);
    if (!inflated || !rgba) goto fail;
    mz_ulong outlen=(mz_ulong)rawsize;
    if (mz_uncompress(inflated,&outlen,compressed,(mz_ulong)clen)!=MZ_OK ||
        (size_t)outlen!=rawsize) goto fail;
    for (unsigned y=0;y<h;y++) {
        unsigned char *row=inflated+(size_t)y*rowsize+1;
        const unsigned char *prev=y ? row-rowsize : NULL;
        unsigned filter=row[-1];
        if (filter>4) goto fail;
        for (size_t i=0;i<stride;i++) {
            unsigned a=i>=bpp ? row[i-bpp] : 0;
            unsigned b=prev ? prev[i] : 0;
            unsigned c=prev && i>=bpp ? prev[i-bpp] : 0;
            int predictor=filter==0?0:filter==1?(int)a:filter==2?(int)b:
                          filter==3?(int)((a+b)/2):png_paeth((int)a,(int)b,(int)c);
            row[i]=(unsigned char)(row[i]+predictor);
        }
        for (unsigned x=0;x<w;x++) {
            const unsigned char *in=row+(size_t)x*bpp;
            unsigned char *out=rgba+((size_t)y*w+x)*4;
            out[0]=in[0];out[1]=in[1];out[2]=in[2];
            out[3]=bpp==4?in[3]:255;
        }
    }
    free(compressed);free(inflated);
    *width=w;*height=h;
    return rgba;
fail:
    free(compressed);free(inflated);free(rgba);
    return NULL;
}
unsigned char *Expanded_PNG_LoadRGBA(const char *path,
                                    unsigned *width,unsigned *height)
{
    unsigned char *file=NULL,*out=NULL;
    FILE *f=NULL;
    long size=0;
    if (width) *width=0;
    if (height) *height=0;
    if (!path || !width || !height) return NULL;
    f=fopen(path,"rb");
    if (!f) return NULL;
    if (fseek(f,0,SEEK_END)!=0 || (size=ftell(f))<33 ||
        size>(long)PNG_LIMIT || fseek(f,0,SEEK_SET)!=0) goto done;
    file=(unsigned char *)malloc((size_t)size);
    if (!file || fread(file,1,(size_t)size,f)!=(size_t)size) goto done;
    out=png_load_memory(file,(size_t)size,width,height);
done:
    free(file);
    fclose(f);
    return out;
}
