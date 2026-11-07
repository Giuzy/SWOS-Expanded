/* SWOS Expanded: loader for approved native Windows V4 BMP layers.
 * Reads the BMP on disk, preserves per-pixel alpha, never converts assets.
 * Only for SWOS_SDL_BUILD, included by the Business renderer. */
#ifndef EXPANDED_BUSINESS_BMP_H
#define EXPANDED_BUSINESS_BMP_H
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
static uint32_t exbmp_u32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static uint16_t exbmp_u16(const unsigned char *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1]<<8));
}
static int exbmp_load(const char *path,int width,int height,unsigned char *rgba) {
    unsigned char h[122],row[320*4];
    if(!path||!rgba||width<1||width>320||height<1||height>256) return 0;
    FILE *f=fopen(path,"rb");
    if(!f) return 0;
    int ok=(fread(h,1,sizeof h,f)==sizeof h) &&
        h[0]=='B' && h[1]=='M' &&
        exbmp_u32(h+2)==(uint32_t)(122+width*height*4) &&
        exbmp_u32(h+10)==122u && exbmp_u32(h+14)==108u &&
        exbmp_u32(h+18)==(uint32_t)width &&
        exbmp_u32(h+22)==(uint32_t)height &&
        exbmp_u16(h+26)==1u && exbmp_u16(h+28)==32u &&
        exbmp_u32(h+30)==3u &&
        exbmp_u32(h+54)==0x00ff0000u &&
        exbmp_u32(h+58)==0x0000ff00u &&
        exbmp_u32(h+62)==0x000000ffu &&
        exbmp_u32(h+66)==0xff000000u;
    if(ok) {
        for(int y=height-1;y>=0;y--) {
            if(fread(row,1,(size_t)width*4u,f)!=(size_t)width*4u) {ok=0;break;}
            for(int x=0;x<width;x++) {
                size_t src=(size_t)x*4u;
                size_t dst=((size_t)y*width+x)*4u;
                rgba[dst+0]=row[src+2];
                rgba[dst+1]=row[src+1];
                rgba[dst+2]=row[src+0];
                rgba[dst+3]=row[src+3];
            }
        }
        if(fgetc(f)!=EOF) ok=0;
    }
    fclose(f);
    return ok;
}
#endif
