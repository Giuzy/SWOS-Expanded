/*
 * Amiga SWOS RE Project
 * SWOS Amiga C/68000 Reversed by Ben Baker (https://github.com/benbaker76)
 * SWOS DOS Reversed by Zlatko Karakaš (https://github.com/zlatkok)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player_face3.h"
#include "player_sprites.h"
#include "game_types.h"
#include "string_utils.h"
#include "menu_system.h"
#include "display.h"
#include "blitter.h"

/* ---- Constants ---- */
#define SPR_TEAM1_PLAYER_NAMES_START    0x8D

/* ---- External globals (in SWOS2.s / data section) ---- */
extern uint8_t _surnameBuf[];
extern uint8_t _facesFrequency[];
extern void *_skinAndHairColorConversionTable[];
extern GoalieFaceState _goalieFaces;     /* GK face/skin state (was adjacency-aliased globals) */
extern uint16_t _useIndividualPlayerSkinColor;
extern TeamGame *_topTeamPtr;
extern TeamGame *_bottomTeamPtr;
extern Sprite **_team1SpritesTable;
extern Sprite **_team2SpritesTable;
extern uint8_t _plNamesLengthTable[];
extern uint8_t _numberBuffer[];
extern uint8_t _smallCharsTable[];

/* ================================================================
 * ExtractSurname  (SWOS2.s line 45863)
 *
 * Extracts surname (or abbreviated name) from a player name.
 * maxLenAndFlags bits 15:14 control mode:
 *   00 = try full name, abbreviate if too long
 *   10 = force "F. surname" mode
 *   01 = skip first name, copy surname only
 * Lower bits = max output length.
 * Returns pointer to surnameBuf or original name if it fits.
 * ================================================================ */
__attribute__((used, externally_visible))
uint8_t *ExtractSurname(uint8_t *name, uint16_t maxLenAndFlags)
{
    uint8_t *origName = name;
    uint16_t maxLen = maxLenAndFlags;
    int16_t mode = 0;       /* 0=default, 1=force initial, -1=skip firstname */
    uint16_t flags;
    uint8_t *out;
    uint16_t charCount;
    uint8_t *p;              /* first non-space char */
    uint8_t *firstWordEnd;   /* space after first word (or null) */
    uint16_t firstWordLen;
    uint8_t *strEnd;
    uint8_t *lastNonSpace;
    uint16_t restLen;

    flags = maxLenAndFlags & 0xC000;
    if (flags == 0) {
        /* No flags: check if name fits within maxLen */
        uint16_t len = GetStringLength(name);
        if (len <= maxLen)
            return origName;
        /* Doesn't fit, fall through to build abbreviated */
    } else if (maxLenAndFlags & 0x8000) {
        /* Bit 15 set: force initial mode */
        maxLen &= (uint16_t)~0x8000;
        mode = 1;
    } else {
        /* Bit 14 set: skip first name entirely */
        maxLen &= (uint16_t)~0x4000;
        mode = -1;
    }

    /* Build into surnameBuf */
    out = _surnameBuf;
    charCount = 0;
    p = origName;

    /* Skip leading spaces */
    while (*p == ' ')
        p++;

    if (*p == 0) {
        _surnameBuf[0] = 0;
        return _surnameBuf;
    }

    /* Find end of first word */
    firstWordEnd = p;
    while (*firstWordEnd != 0) {
        if (*firstWordEnd == ' ')
            break;
        firstWordEnd++;
    }
    firstWordLen = (uint16_t)(firstWordEnd - p);

    /* Find string end (null terminator) then trim trailing spaces */
    strEnd = p;
    while (*strEnd)
        strEnd++;
    strEnd--;
    while (*strEnd == ' ')
        strEnd--;
    /* strEnd now points to last non-space char */
    lastNonSpace = strEnd;

    /* Backtrack from last non-space to end of first word */
    while (strEnd != p) {
        strEnd--;
        if (strEnd == firstWordEnd)
            goto found_multiword;
    }

    /* Single word: just copy with limit */
    goto copy_remainder;

found_multiword:
    restLen = (uint16_t)(uint32_t)(lastNonSpace - strEnd);
    strEnd++;   /* point past first-word-end to start of rest */

    if (mode < 0)
        goto copy_remainder;    /* skip first name */

    if (mode > 0)
        goto emit_initial;      /* force initial mode */

    /* Default mode: check if "firstname rest" fits */
    {
        uint16_t totalLen = firstWordLen + restLen + 1;
        if (totalLen > maxLen)
            goto emit_initial;
    }

    /* Copy "firstname rest" in full */
    {
        int16_t cnt;
        uint8_t *src = p;
        cnt = (int16_t)firstWordLen - 1;
        while (cnt >= 0) {
            *out++ = *src++;
            cnt--;
        }
        *out++ = ' ';
        cnt = (int16_t)restLen - 1;
        while (cnt >= 0) {
            *out++ = *strEnd++;
            cnt--;
        }
    }
    goto terminate;

emit_initial:
    *out++ = *p;
    *out++ = '.';
    *out++ = ' ';
    charCount = 3;
    /* fall through to copy_remainder */

copy_remainder:
    for (;;) {
        *out++ = *strEnd++;
        if (*strEnd == 0)
            break;
        charCount++;
        if (charCount == maxLen) {
            *(out - 1) = '.';
            break;
        }
    }

terminate:
    *out = 0;
    return _surnameBuf;
}

/* ================================================================
 * CountFaces  (SWOS2.s line 12497)
 *
 * Counts frequency of each face type (0-2) across 16 players.
 * ================================================================ */
__attribute__((used, externally_visible))
void CountFaces(TeamGame *team)
{
    int16_t i;

    _facesFrequency[0] = 0;
    _facesFrequency[1] = 0;
    _facesFrequency[2] = 0;

    for (i = 0; i < 16; i++) {
        int16_t face = (int16_t)team->players[i].face;
        _facesFrequency[face]++;
    }
}

/* ================================================================
 * ConvertColorTableToSkinColor  (SWOS2.s line 12516)
 *
 * Applies skin/hair color conversion to a dest color table.
 * color is sign-extended from byte, used as index into
 * skinAndHairColorConversionTable (3 pointers to pair lists).
 * Each pair: (index, value) uint16_t, terminated by negative index.
 * ================================================================ */
__attribute__((used, externally_visible))
void ConvertColorTableToSkinColor(uint16_t color, uint16_t *destTable)
{
    int16_t colorIdx = (int16_t)(int8_t)(color & 0xFF);
    uint16_t *convTable;
    int16_t idx;

    convTable = (uint16_t *)_skinAndHairColorConversionTable[colorIdx];

    for (;;) {
        idx = (int16_t)*convTable++;
        if (idx < 0)
            break;
        destTable[idx] = *convTable++;
    }
}

/* ================================================================
 * DetermineGoalieFaces  (SWOS2.s line 12540)
 *
 * Scans 16 players to assign goalkeeper face colors.
 * gkFaceToIndex[0..2]: maps face type → slot index (0=main, 1=reserve)
 * gkFaceIndices[0..1]: records which face type is in each slot
 * Goalkeepers are prioritized; non-GK faces fill remaining slots.
 * ================================================================ */
__attribute__((used, externally_visible))
void DetermineGoalieFaces(TeamGame *team, uint8_t *gkFaceToIndex, uint8_t *gkFaceIndices)
{
    uint16_t slotIdx = 0;
    int16_t i;

    gkFaceToIndex[0] = 0;
    gkFaceToIndex[1] = 0;
    gkFaceToIndex[2] = 0;
    gkFaceIndices[0] = 0xFF;
    gkFaceIndices[1] = 0xFF;

    for (i = 0; i < 16; i++) {
        int8_t position = team->players[i].position;
        uint8_t face;
        int16_t faceExt;

        if (position < 0)
            continue;

        face = (uint8_t)team->players[i].face;

        /* Skip if face already assigned to slot 0 */
        if (face == gkFaceIndices[0])
            continue;

        /* Non-goalkeeper: also skip if face already assigned to slot 1 */
        if (position != 0) {
            if (face == gkFaceIndices[1])
                continue;
        }

        /* Record face mapping */
        faceExt = (int16_t)(int8_t)face;
        gkFaceToIndex[faceExt] = (uint8_t)slotIdx;
        gkFaceIndices[slotIdx] = (uint8_t)faceExt;

        /* Advance slot only for first player (i==0) or goalkeepers */
        if (i == 0 || position == 0) {
            slotIdx++;
            if (slotIdx == 2)
                return;
        }
    }
}

/* ================================================================
 * InitializePlayerSpriteFrameIndices  (SWOS2.s line 12629)
 *
 * Sets frameOffset for each of 11 players per team based
 * on skin color. Goalkeepers use gkFaceToPosition lookup with
 * multiplier 0x3A (58), outfield use multiplier 0x65 (101).
 * ================================================================ */
__attribute__((used, externally_visible))
void InitializePlayerSpriteFrameIndices(void)
{
    int16_t d1;
    uint16_t idx = 0;

    for (d1 = 10; d1 >= 0; d1--, idx++) {
        Sprite *spr1 = _team1SpritesTable[idx];
        Sprite *spr2 = _team2SpritesTable[idx];

        /* Team 1 */
        spr1->frameOffset = 0;
        if (_useIndividualPlayerSkinColor != 0) {
            int16_t face = (int16_t)_topTeamPtr->players[idx].face;
            if (d1 == 10) {
                /* Goalkeeper: use face-to-position lookup */
                face = (int16_t)(int8_t)_goalieFaces.team1FaceToPosition[face];
                if (face == -1)
                    face = 0;
                face = face * 0x3A;
            } else if (_topTeamPtr->players[idx].blond) {
                face = Face3_FrameOffset(0);    /* (face 3: player_face3.c) */
            } else {
                face = face * 0x65;
            }
            spr1->frameOffset = (int16_t)face;
        }

        /* Team 2 */
        spr2->frameOffset = 0;
        if (_useIndividualPlayerSkinColor != 0) {
            int16_t face = (int16_t)_bottomTeamPtr->players[idx].face;
            if (d1 == 10) {
                face = (int16_t)(int8_t)_goalieFaces.team2FaceToPosition[face];
                if (face == -1)
                    face = 0;
                face = face * 0x3A;
            } else if (_bottomTeamPtr->players[idx].blond) {
                face = Face3_FrameOffset(1);
            } else {
                face = face * 0x65;
            }
            spr2->frameOffset = (int16_t)face;
        }
    }
}

/* ================================================================
 * makePlayerNameSprite  (SWOS2.s line 11884)
 *
 * Builds a sprite from player shirt number + surname.
 * Writes "NN surname" into numberBuffer, renders via Text2Sprite.
 * Returns next sprite index.
 * ================================================================ */
static uint16_t makePlayerNameSprite(uint16_t playerNumber, uint8_t *shortName,
                                     uint16_t spriteIdx, uint8_t *nameLenOut)
{
    uint8_t *p;
    uint8_t *surname;
    uint16_t textWidth;

    /* Write player number to numberBuffer */
    Int2Ascii(playerNumber, _numberBuffer);

    /* Find end of number string and append space */
    p = _numberBuffer;
    while (*p)
        p++;
    *p++ = ' ';

    /* Clear the destination sprite */
    HWSprite_LoadFromSpriteGraphics(spriteIdx);

    /* Extract surname (bit14=skip firstname, max 11 chars) */
    surname = ExtractSurname(shortName, 0x400B);

    /* Copy surname after "NN " */
    StringCopy(surname, p);

    /* Render text into sprite */
    Text2Sprite(0, 0, spriteIdx, _numberBuffer, _smallCharsTable, 2, &textWidth);

    *nameLenOut = (uint8_t)textWidth;
    return spriteIdx + 1;
}


#ifdef SWOS_SDL_BUILD
/* v00.13.320 bench extension: name sprites of the 10 EXTRA bench players per team (match slots
 * 16..25). The game's 32 name sprites (0x8D..0xAC, 80x6) are all in use and every one of the
 * 1220 table slots is taken (SWOS.log [sprite-map] null=0), so these are NEW sprites in the
 * table's extension slots 1684..1703 (same pattern as shirt_numbers.c), cloned from the OG
 * name sprite header. Built once; rendered with the OG makePlayerNameSprite/Text2Sprite. */
#define EXP_NAMESPR_FIRST 1684
#define EXP_NAMESPR_COUNT 20
static SpriteGraphics s_extraNameHdr[EXP_NAMESPR_COUNT];
static uint8_t s_extraNamePix[EXP_NAMESPR_COUNT][80*8];
static uint8_t s_extraNameLen[EXP_NAMESPR_COUNT];
static int s_extraNameReady;
int ExpBenchNameSprites_Build(void)
{
    extern uint8_t *_spriteGraphicsPtr;
    SpriteGraphics **t=(SpriteGraphics **)_spriteGraphicsPtr;
    const SpriteGraphics *src;
    int k;
    if(s_extraNameReady)return 1;
    if(!t || !(src=t[0x8D]) || src->pixWidth!=80 || src->nlines!=6 || src->type!=0)return 0; /* unknown format: refuse */
    for(k=0;k<EXP_NAMESPR_COUNT;++k){
        s_extraNameHdr[k]=*src;
        s_extraNameHdr[k].sprData=s_extraNamePix[k];
        s_extraNameHdr[k].maskPtr=0;
        s_extraNameHdr[k].ordinal=(int16_t)(EXP_NAMESPR_FIRST+k);
        t[EXP_NAMESPR_FIRST+k]=&s_extraNameHdr[k];
    }
    s_extraNameReady=1;
    return 1;
}
/* v00.13.323 LONG BENCH ART (RG-MATCH-BENCH-EXT): the approved data/expanded/bench_long.png (21x112,
 * 15 seats + coach slot, 7 px per slot) turned into game sprites: for each bench of N=6..15 seats
 * two variants of height 7*(N+1): "upper" = the BOTTOM rows of the art (the player's bench grows
 * upwards and keeps its lower end), "lower" = the TOP rows (the CPU bench grows downwards and keeps
 * its upper end). Table slots 1704..1723. Colours are mapped to the palette of the match
 * (Platform_NearestColour), alpha<128 is transparent (index 0). Built lazily, once per match. */
#define EXP_BENCHART_FIRST 1704
#define EXP_BENCHART_W 21
#define EXP_BENCHART_H 112
static SpriteGraphics s_benchArtHdr[20];
/* v342 (user 26/11/2026): every bench carries the OG bench drop shadow below its end: 5 rows, columns
 * 3..20, colour of the original shell shadow (0,34,0 measured on the OG tile bench), so both benches
 * look the same; the old tile shadow of the upper bench is covered by the art. */
#define EXP_BENCHART_SHADOW_H 5u
static uint8_t s_benchArtPix[20][(EXP_BENCHART_H+EXP_BENCHART_SHADOW_H)*32];
static int s_benchArtState;   /* 0 not built, 1 built, -1 failed */
extern uint8_t Platform_NearestColour(uint8_t r,uint8_t g,uint8_t b);
extern unsigned char *Expanded_PNG_LoadRGBA(const char *path,unsigned *width,unsigned *height);
extern const char *swos_data_path(const char *rel,char *buf,uint32_t buf_sz);
void ExpBenchLong_Reset(void){ if(s_benchArtState<0)s_benchArtState=0; }
int ExpBenchLongSprite(unsigned count,int upper)
{
    extern uint8_t *_spriteGraphicsPtr;
    if(count<6u || count>15u || !_spriteGraphicsPtr)return -1;
    if(s_benchArtState==0) {
        char path[1024];
        unsigned w=0,h=0,n,y,x;
        unsigned char *px=Expanded_PNG_LoadRGBA(swos_data_path("data/expanded/bench_long.png",path,(uint32_t)sizeof path),&w,&h);
        SpriteGraphics **t=(SpriteGraphics **)_spriteGraphicsPtr;
        s_benchArtState=-1;
        if(px && w==EXP_BENCHART_W && h==EXP_BENCHART_H && t[0x8D]) {
            for(n=6;n<=15;++n) {
                const unsigned H=7u*(n+1u);
                int v;
                for(v=0;v<2;++v) {        /* v0 = upper (bottom rows), v1 = lower (top rows) */
                    const unsigned k=(n-6u)*2u+(unsigned)v, y0=v==0?EXP_BENCHART_H-H:0u;
                    SpriteGraphics *hd=&s_benchArtHdr[k];
                    memset(s_benchArtPix[k],0,sizeof s_benchArtPix[k]);
                    for(y=0;y<H;++y)for(x=0;x<w;++x) {
                        const unsigned char *p=px+((y0+y)*w+x)*4u;
                        if(p[3]>=128)s_benchArtPix[k][y*32u+x]=Platform_NearestColour(p[0],p[1],p[2]);
                    }
                    *hd=*t[0x8D];
                    hd->sprData=s_benchArtPix[k];hd->maskPtr=0;hd->type=0;
                    {   const uint8_t sh=Platform_NearestColour(0x00,0x22,0x00);
                        for(y=H;y<H+EXP_BENCHART_SHADOW_H;++y)for(x=3u;x<w;++x)s_benchArtPix[k][y*32u+x]=sh;
                    }
                    hd->pixWidth=EXP_BENCHART_W;hd->nlines=(int16_t)(H+EXP_BENCHART_SHADOW_H);hd->wquads=2;
                    hd->centerX=0;hd->centerY=0;hd->ordinal=(int16_t)(EXP_BENCHART_FIRST+k);
                    t[EXP_BENCHART_FIRST+k]=hd;
                }
            }
            s_benchArtState=1;
        }
        free(px);
        fprintf(stderr,"[bench-art] build=%d size=%ux%u\n",s_benchArtState,w,h);
    }
    if(s_benchArtState!=1)return -1;
    return EXP_BENCHART_FIRST+(int)((count-6u)*2u)+(upper?0:1);
}

/* OG name sprite re-render for a field slot taken by an extended-bench player (v322). */
void ExpNameSprite_RenderAt(uint16_t spriteIdx,uint16_t shirt,const uint8_t *shortName,uint8_t *lenOut)
{
    if(shortName && lenOut)(void)makePlayerNameSprite(shirt,(uint8_t *)shortName,spriteIdx,lenOut);
}
/* team 0/1, extra bench ordinal 0..9 (match slot 16+n). Returns the sprite index or -1. */
int ExpBenchNameSprite_Render(int team,int n,uint16_t shirt,const uint8_t *shortName)
{
    if(!s_extraNameReady || team<0 || team>1 || n<0 || n>9 || !shortName)return -1;
    {   int k=team*10+n;
        memset(s_extraNamePix[k],0,sizeof s_extraNamePix[k]);
        (void)makePlayerNameSprite(shirt,(uint8_t *)shortName,(uint16_t)(EXP_NAMESPR_FIRST+k),&s_extraNameLen[k]);
        return EXP_NAMESPR_FIRST+k;
    }
}
#endif

/* ================================================================
 * MakeSpritesFromPlayerNames  (SWOS2.s line 11853)
 *
 * Creates name sprites for all 16 players of both teams.
 * Starting at SPR_TEAM1_PLAYER_NAMES_START (0x8D).
 * ================================================================ */
__attribute__((used, externally_visible))
void MakeSpritesFromPlayerNames(TeamGame *leftTeam, TeamGame *rightTeam)
{
    uint8_t *nameLenPtr = _plNamesLengthTable;
    uint16_t sprIdx = SPR_TEAM1_PLAYER_NAMES_START;
    int16_t i;

    for (i = 0; i < 16; i++) {
        uint16_t shirtNum = (uint16_t)(uint8_t)leftTeam->players[i].shirtNumber;
        sprIdx = makePlayerNameSprite(shirtNum, (uint8_t *)leftTeam->players[i].shortName,
                                      sprIdx, nameLenPtr++);
    }

    for (i = 0; i < 16; i++) {
        uint16_t shirtNum = (uint16_t)(uint8_t)rightTeam->players[i].shirtNumber;
        sprIdx = makePlayerNameSprite(shirtNum, (uint8_t *)rightTeam->players[i].shortName,
                                      sprIdx, nameLenPtr++);
    }
#ifdef SWOS_SDL_BUILD
    ExpBenchLong_Reset();
    fprintf(stderr,"[sprite-map] extra name sprites build=%d\n",ExpBenchNameSprites_Build());
#endif
    {   /* v320 DIAGNOSTIC ONLY (bench extension, RG-MATCH-BENCH-EXT): free sprite slots, once */
        static int s_done;
        extern uint8_t *_spriteGraphicsPtr;
        if(!s_done && _spriteGraphicsPtr) {
            SpriteGraphics **t=(SpriteGraphics **)_spriteGraphicsPtr;
            int k,nul=0,first=-1,runs=0,prevnul=0;
            s_done=1;
            for(k=0;k<1220;++k){
                if(!t[k]){++nul;if(!prevnul){++runs;if(runs<=12)fprintf(stderr,"[sprite-map] NULL run starts at 0x%03X\n",k);}prevnul=1;
                          if(first<0)first=k;}
                else prevnul=0;
            }
            fprintf(stderr,"[sprite-map] slots=1220 null=%d firstNull=0x%03X nullRuns=%d\n",nul,first<0?0:first,runs);
            for(k=0x8D;k<=0xAE;++k)
                fprintf(stderr,"[sprite-map] 0x%03X %s w=%d h=%d type=%u wq=%d\n",k,t[k]?"SET":"NULL",
                        t[k]?t[k]->pixWidth:-1,t[k]?t[k]->nlines:-1,t[k]?(unsigned)t[k]->type:0u,t[k]?t[k]->wquads:-1);
        }
    }
}
