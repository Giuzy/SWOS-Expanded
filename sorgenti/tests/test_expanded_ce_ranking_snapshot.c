/* Isolated static reader test: supply runtime JSON file path as argv[1].
 * No WORLD globals, competition writer, UI or CAR exercised. */
#include "expanded_ce_ranking_snapshot.h"
#include <assert.h>
int main(int argc,char **argv)
{
    FILE *f;
    char *buf,*p;
    long len;
    ExpCeRankSnapshot s={0};
    uint32_t count[4]={0},i;
    assert(argc==2);
    f=fopen(argv[1],"rb");assert(f);
    assert(fseek(f,0,SEEK_END)==0);
    len=ftell(f);assert(len>0&&len<=INT_MAX);
    assert(fseek(f,0,SEEK_SET)==0);
    buf=(char *)malloc((size_t)len+1u);assert(buf);
    assert(fread(buf,1,(size_t)len,f)==(size_t)len);
    fclose(f);buf[len]=0;
    assert(!ExpCeRankSnapshot_Parse(WORLD_9697,buf,(int)len,&s));
    assert(!ExpCeRankSnapshot_Parse(WORLD_2627,buf,(int)len/2,&s));
    assert(!s.rows&&!s.n);
    assert(ExpCeRankSnapshot_Parse(WORLD_2627,buf,(int)len,&s));
    assert(s.n==415u);
    for(i=0;i<s.n;++i)++count[s.rows[i].status];
    assert(count[1]==387u&&count[2]==23u&&count[3]==5u);
    assert(s.rows[0].source_row==1u&&s.rows[0].seed_milli==147500u);
    assert(s.rows[128].status==EXP_CE_RANK_ONLY_ACTIVE);
    assert(s.rows[361].own_milli==2500u&&s.rows[361].seed_milli==2800u);
    ExpCeRankSnapshot_Free(&s);
    p=strstr(buf,"\"sourceRow\": 1,");assert(p);
    p[strlen("\"sourceRow\": ")]='2';
    assert(!ExpCeRankSnapshot_Parse(WORLD_2627,buf,(int)len,&s)&&!s.rows&&!s.n);
    free(buf);puts("Static UEFA ranking seed reader PASS");return 0;
}
