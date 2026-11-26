#ifndef SQX_MATCH_STORE_H
#define SQX_MATCH_STORE_H
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "game_types.h"

/* Sparse authoritative extra-player records, indexed by the existing raw
 * club and original extra ordinal. A promoted entry has stable address even
 * when the catalogue grows; the immutable JSON cache is no longer its writer.
 * The prefix is real storage for legacy GetPlayer's base + 76 convention. */
typedef struct SqxMatchRecord {
    uint16_t team, index;
    union {
        struct { uint8_t prefix[76]; PlayerFile player; };
        uint8_t bytes[76+sizeof(PlayerFile)];
    };
} SqxMatchRecord;
typedef struct SqxMatchStore {
    SqxMatchRecord **rows;
    size_t count,capacity;
} SqxMatchStore;
_Static_assert(sizeof(PlayerFile)==38,"PlayerFile ABI");
_Static_assert(offsetof(SqxMatchRecord,player)-offsetof(SqxMatchRecord,bytes)==76,"GetPlayer ABI");
static inline uint32_t SqxMatch_Key(uint16_t team,uint16_t index)
{ return ((uint32_t)team<<8)|index; }
static inline int SqxMatch_Valid(uint16_t index,const PlayerFile *p)
{ return index>=16 && index<30 && p && p->playerName[0] && memchr(p->playerName,0,sizeof p->playerName)!=NULL; }
static inline int SqxMatch_SameName(const PlayerFile *a,const PlayerFile *b)
{ return a && b && strncmp((const char *)a->playerName,(const char *)b->playerName,sizeof a->playerName)==0; }
static inline size_t SqxMatch_Lower(const SqxMatchStore *s,uint32_t key)
{
    size_t lo=0,hi=s->count;
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        SqxMatchRecord *r=s->rows[m];
        if(SqxMatch_Key(r->team,r->index)<key)lo=m+1;else hi=m;
    }
    return lo;
}
static inline SqxMatchRecord *SqxMatch_Find(const SqxMatchStore *s,uint16_t team,uint16_t index)
{
    uint32_t k=SqxMatch_Key(team,index);
    size_t at=SqxMatch_Lower(s,k);
    return at<s->count && SqxMatch_Key(s->rows[at]->team,s->rows[at]->index)==k?s->rows[at]:NULL;
}
/* No duplicate keys on restore, no replacement of an existing identity. */
static inline SqxMatchRecord *SqxMatch_Insert(SqxMatchStore *s,uint16_t team,uint16_t index,const PlayerFile *p)
{
    if(!s || !SqxMatch_Valid(index,p))return NULL;
    uint32_t key=SqxMatch_Key(team,index);
    size_t at=SqxMatch_Lower(s,key);
    if(at<s->count && SqxMatch_Key(s->rows[at]->team,s->rows[at]->index)==key)return NULL;
    if(s->count==s->capacity) {
        if(s->capacity>SIZE_MAX/2/sizeof *s->rows)return NULL;
        size_t next=s->capacity?s->capacity*2:16;
        SqxMatchRecord **rows=realloc(s->rows,next*sizeof *rows);
        if(!rows)return NULL;
        s->rows=rows;s->capacity=next;
    }
    SqxMatchRecord *r=calloc(1,sizeof *r);
    if(!r)return NULL;
    r->team=team;r->index=index;r->player=*p;
    memmove(s->rows+at+1,s->rows+at,(s->count-at)*sizeof *s->rows);
    s->rows[at]=r;++s->count;
    return r;
}
static inline SqxMatchRecord *SqxMatch_Ensure(SqxMatchStore *s,uint16_t team,uint16_t index,const PlayerFile *p)
{
    if(!s || !SqxMatch_Valid(index,p))return NULL;
    SqxMatchRecord *r=SqxMatch_Find(s,team,index);
    return r?(SqxMatch_SameName(&r->player,p)?r:NULL):SqxMatch_Insert(s,team,index,p);
}
static inline void SqxMatch_Clear(SqxMatchStore *s)
{
    for(size_t i=0;i<s->count;i++)free(s->rows[i]);
    free(s->rows);*s=(SqxMatchStore){0};
}
/* Exact 38 byte PlayerFile, all byte-sized fields, no pointers/endian data. */
#define SQX_MATCH_HEX_LEN (2*sizeof(PlayerFile))
static inline void SqxMatch_Encode(const PlayerFile *p,char out[SQX_MATCH_HEX_LEN+1])
{
    static const char digits[]="0123456789abcdef";
    const uint8_t *b=(const uint8_t *)p;
    for(size_t i=0;i<sizeof *p;i++) { out[2*i]=digits[b[i]>>4];out[2*i+1]=digits[b[i]&15]; }
    out[SQX_MATCH_HEX_LEN]=0;
}
static inline int SqxMatch_HexDigit(char c)
{ return c>='0' && c<='9'?c-'0':c>='a' && c<='f'?c-'a'+10:-1; }
static inline int SqxMatch_Decode(const char *hex,size_t len,PlayerFile *out)
{
    if(!hex || !out || len!=SQX_MATCH_HEX_LEN)return 0;
    PlayerFile p;uint8_t *b=(uint8_t *)&p;
    for(size_t i=0;i<sizeof p;i++) {
        int hi=SqxMatch_HexDigit(hex[2*i]),lo=SqxMatch_HexDigit(hex[2*i+1]);
        if(hi<0 || lo<0)return 0;
        b[i]=(uint8_t)((hi<<4)|lo);
    }
    if(!SqxMatch_Valid(16,&p))return 0;
    *out=p;return 1;
}
#endif
