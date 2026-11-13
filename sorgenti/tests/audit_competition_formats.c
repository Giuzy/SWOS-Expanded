/* SWOS Expanded competition database audit.
 * Parses INSTALLED OG JSON with the same mjson parser used by the game.
 * This validates recorded formats/references, NOT gameplay or cup outcomes.
 * No Python, fabricated scores, fixed 1996/97 counts or static code greps.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mjson.h"

#define MAX_IDS 4096u
typedef struct {char id[112];unsigned category;} KnownId;
static KnownId known[MAX_IDS];
static unsigned nknown,errors,refs,leagues,cups,tournaments,playoffs,qualifiers;
static unsigned league_cycles[8],group_tournaments,knockout_legs;
static FILE *csv;
static const char *categories[]={"leagues","cups","tournaments","playoffs"};
static const char *arraynames[]={"leagues","competitions","competitions","playoffs"};
static int read_file(const char *dir,const char *base,char **out,int *size) {
    char path[512];
    snprintf(path,sizeof path,"%s/%s.json",dir,base);
    FILE *f=fopen(path,"rb");
    if(!f){fprintf(stderr,"[comp-formats] FAIL missing %s\n",path);return 0;}
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return 0;}
    long len=ftell(f);
    if(len<2 || len>16000000 || fseek(f,0,SEEK_SET)!=0) {
        fprintf(stderr,"[comp-formats] FAIL size %s\n",path);
        fclose(f);return 0;
    }
    char *s=malloc((size_t)len+1u);
    if(!s){fclose(f);return 0;}
    if(fread(s,1,(size_t)len,f)!=(size_t)len) {
        fprintf(stderr,"[comp-formats] FAIL read %s\n",path);
        free(s);fclose(f);return 0;
    }
    fclose(f);s[len]=0;*out=s;*size=(int)len;return 1;
}
static int number(const char *s,int n,const char *path,long *v) {
    return mjson_get_long(s,n,path,v)!=0;
}
static int string(const char *s,int n,const char *path,char *out,int cap) {
    return mjson_get_string(s,n,path,out,cap)>0;
}
static int reference(const char *id,int category,const char *where) {
    for(unsigned i=0;i<nknown;++i)
        if(strcmp(known[i].id,id)==0) {
            if(category>=0 && known[i].category!=(unsigned)category) {
                fprintf(stderr,"[comp-formats] FAIL wrong category id=%s at=%s\n",id,where);
                ++errors;return 0;
            }
            ++refs;return 1;
        }
    fprintf(stderr,"[comp-formats] FAIL unknown reference id=%s at=%s\n",id,where);
    ++errors;return 0;
}
static void install(const char *id,unsigned cat) {
    for(unsigned i=0;i<nknown;++i)
        if(strcmp(known[i].id,id)==0) {
            fprintf(stderr,"[comp-formats] FAIL duplicate id=%s\n",id);
            ++errors;return;
        }
    if(nknown==MAX_IDS) {++errors;return;}
    snprintf(known[nknown].id,sizeof known[nknown].id,"%s",id);
    known[nknown++].category=cat;
}
static void audit_competitions(const char *dir,unsigned cat) {
    char *s=NULL;int size=0;
    if(!read_file(dir,categories[cat],&s,&size)) {++errors;return;}
    long count=-1;
    if(!number(s,size,"$.count",&count) || count<0 || count>2000) {
        fprintf(stderr,"[comp-formats] FAIL count category=%s\n",categories[cat]);
        ++errors;free(s);return;
    }
    unsigned entered=0;
    for(unsigned i=0;i<(unsigned)count;++i) {
        char prefix[92],path[196],id[112],signature[160];
        long compid=0,type=0,begin=0,end=0,teams=0,cycles=0,divisions=0;
        snprintf(prefix,sizeof prefix,"$.%s[%u]",arraynames[cat],i);
        snprintf(path,sizeof path,"%s.id",prefix);
        if(!string(s,size,path,id,sizeof id)) {
            fprintf(stderr,"[comp-formats] FAIL missing id category=%s index=%u\n",
                    categories[cat],i);++errors;continue;
        }
        install(id,cat);
        snprintf(path,sizeof path,"%s.comp.comp_id",prefix);
        if(!number(s,size,path,&compid)) {++errors;continue;}
        snprintf(path,sizeof path,"%s.comp.type",prefix);
        if(!number(s,size,path,&type)) {++errors;continue;}
        snprintf(path,sizeof path,"%s.comp.begin_month",prefix);
        if(!number(s,size,path,&begin)) {++errors;continue;}
        snprintf(path,sizeof path,"%s.comp.end_month",prefix);
        if(!number(s,size,path,&end)) {++errors;continue;}
        if(begin< -128 || begin>127 || end< -128 || end>127) {
            fprintf(stderr,"[comp-formats] FAIL invalid OG phase id=%s begin=%ld end=%ld\n",
                    id,begin,end);++errors;continue;
        }
        if(cat==0) {
            snprintf(path,sizeof path,"%s.rounds",prefix);
            if(!number(s,size,path,&cycles) || cycles<1 || cycles>4) {
                fprintf(stderr,"[comp-formats] FAIL unsupported cycles id=%s\n",id);
                ++errors;continue;
            }
            snprintf(path,sizeof path,"%s.divisions_count",prefix);
            if(!number(s,size,path,&divisions) || divisions<1 || divisions>32) {
                fprintf(stderr,"[comp-formats] FAIL divisions id=%s\n",id);
                ++errors;continue;
            }
            for(unsigned d=0;d<(unsigned)divisions;++d) {
                long clubs=0;
                snprintf(path,sizeof path,"%s.divisions[%u].teams",prefix,d);
                if(!number(s,size,path,&clubs) || clubs<2 || clubs>32) {
                    fprintf(stderr,"[comp-formats] FAIL division id=%s index=%u\n",id,d);
                    ++errors;continue;
                }
                snprintf(signature,sizeof signature,"ROUND_ROBIN_%ld_CYCLES",cycles);
                fprintf(csv,"league,%s,%s,%ld,%ld,%ld,%ld,%ld,VALIDATED_DATA,NOT_TESTED_IN_WINDOWS\n",
                        id,signature,clubs,cycles,begin,end,compid);
            }
            snprintf(path,sizeof path,"%s.divisions[%ld].teams",prefix,divisions);
            if(number(s,size,path,&teams)) {
                fprintf(stderr,"[comp-formats] FAIL extra division id=%s\n",id);
                ++errors;
            }
            ++league_cycles[cycles];++leagues;
        } else {
            unsigned rules=0,twoLeg=0,groupCount=0;
            if(cat==1) {
                snprintf(path,sizeof path,"%s.teams",prefix);
                if(!number(s,size,path,&teams) ||
                   teams>128 || teams< -1 ||
                   (teams== -1 && type!=0)) {
                    fprintf(stderr,"[comp-formats] FAIL invalid cup team count "
                                   "id=%s teams=%ld native-type=%ld\n",
                            id,teams,type);
                    ++errors;continue;
                }
                if(teams== -1)
                    fprintf(stderr,"[comp-formats] NATIVE_UNSIZED "
                                   "id=%s teams=-1 type=0 (not a league)\n",id);
            }
            for(unsigned r=0;r<16u;++r) {
                long legs=0;
                snprintf(path,sizeof path,"%s.round_rule_flags[%u].legs",prefix,r);
                if(!number(s,size,path,&legs))break;
                ++rules;
                if(legs==2)++twoLeg;
                if(legs<0 || legs>3) {
                    fprintf(stderr,"[comp-formats] FAIL invalid leg flag id=%s round=%u\n",id,r);
                    ++errors;
                }
            }
            if(!rules){fprintf(stderr,"[comp-formats] FAIL no rules id=%s\n",id);++errors;}
            if(cat==2) {
                for(unsigned r=0;r<16u;++r) {
                    long groups=0;
                    snprintf(path,sizeof path,"%s.rounds[%u].group_count",prefix,r);
                    if(!number(s,size,path,&groups))break;
                    if(groups>0)++groupCount;
                }
            }
            if(twoLeg)++knockout_legs;
            if(cat==1)++cups;
            if(cat==2) {++tournaments;if(groupCount)++group_tournaments;}
            if(cat==3)++playoffs;
            snprintf(signature,sizeof signature,"%s_%uRULES_%uTWOLEGS_%uGROUPSTAGES",
                     cat==1?(teams== -1?"OG_UNSIZED_FRIENDLY_OR_TOUR":
                              "KNOCKOUT_OR_QUALIFIER"):
                              cat==2?"TOURNAMENT":"PLAYOFF",
                     rules,twoLeg,groupCount);
            fprintf(csv,"%s,%s,%s,%ld,%ld,%ld,%ld,%ld,VALIDATED_DATA,NOT_TESTED_IN_WINDOWS\n",
                    categories[cat],id,signature,teams,cycles,begin,end,compid);
        }
        if(type<0 || type>2) {
            fprintf(stderr,"[comp-formats] FAIL native type id=%s type=%ld\n",id,type);
            ++errors;
        }
        ++entered;
    }
    fprintf(stderr,"[comp-formats] INVENTORY category=%s declared=%ld parsed=%u errors=%u\n",
            categories[cat],count,entered,errors);
    free(s);
}
static void audit_links(const char *dir,const char *file,const char *array,
                        const char *fieldA,int catA,const char *fieldB,int catB) {
    char *s=NULL;int size=0;
    if(!read_file(dir,file,&s,&size)){++errors;return;}
    long count=0;
    if(!number(s,size,"$.count",&count) || count<0 || count>2000) {
        ++errors;free(s);return;
    }
    for(unsigned i=0;i<(unsigned)count;++i) {
        char prefix[112],path[220],id[112];
        snprintf(prefix,sizeof prefix,"$.%s[%u]",array,i);
        if(fieldA)for(unsigned n=0;n<128u;++n) {
            snprintf(path,sizeof path,"%s.%s[%u]",prefix,fieldA,n);
            if(!string(s,size,path,id,sizeof id))break;
            reference(id,catA,file);
        }
        if(fieldB)for(unsigned n=0;n<128u;++n) {
            snprintf(path,sizeof path,"%s.%s[%u]",prefix,fieldB,n);
            if(!string(s,size,path,id,sizeof id))break;
            reference(id,catB,file);
        }
    }
    free(s);
}
static void audit_qualifying(const char *dir) {
    char *s=NULL;int size=0;
    if(!read_file(dir,"qualifying",&s,&size)){++errors;return;}
    long count=0;
    if(!number(s,size,"$.count",&count) || count<0 || count>2000) {
        ++errors;free(s);return;
    }
    for(unsigned i=0;i<(unsigned)count;++i) {
        char path[220],id[112];
        for(unsigned k=0;k<128u;++k) {
            snprintf(path,sizeof path,"$.groups[%u].comp_cup[%u]",i,k);
            if(string(s,size,path,id,sizeof id))reference(id,-1,"qualifying");
            else {
                long sentinel=0;
                if(!number(s,size,path,&sentinel))break;
                if(sentinel!= -1 && sentinel!= -2 && sentinel!=252 && sentinel!=255) {
                    fprintf(stderr,"[comp-formats] QUALIFIER_NUMERIC_REF group=%u value=%ld (native sentinel)\n",i,sentinel);
                }
            }
        }
        ++qualifiers;
    }
    free(s);
}
static void audit_blocks(const char *dir) {
    char *s=NULL;int size=0;
    if(!read_file(dir,"league_blocks",&s,&size)){++errors;return;}
    long count=0;
    if(!number(s,size,"$.count",&count) || count<0 || count>2000) {
        ++errors;free(s);return;
    }
    for(unsigned i=0;i<(unsigned)count;++i)for(unsigned k=0;k<128u;++k){
        char path[180],id[112];
        snprintf(path,sizeof path,"$.groups[%u].leagues[%u].league",i,k);
        if(!string(s,size,path,id,sizeof id))break;
        reference(id,0,"league_blocks");
    }
    free(s);
}
static void audit_career(const char *dir) {
    char *s=NULL;int size=0;
    if(!read_file(dir,"career",&s,&size)){++errors;return;}
    long count=0;unsigned seen_null=0;
    if(!number(s,size,"$.count",&count) || count<0 || count>2000) {
        ++errors;free(s);return;
    }
    for(unsigned i=0;i<(unsigned)count;++i){
        char path[180],id[112];
        snprintf(path,sizeof path,"$.contests[%u].competition",i);
        if(string(s,size,path,id,sizeof id))reference(id,-1,"career");
        else {
            if(i+1u!=(unsigned)count) {
                fprintf(stderr,"[comp-formats] FAIL premature career terminator=%u\n",i);
                ++errors;
            }
            ++seen_null;
        }
    }
    fprintf(stderr,"[comp-formats] career-terminators=%u (last entry may be native null sentinel)\n",seen_null);
    free(s);
}
int main(int argc,char **argv) {
    if(argc<2){fprintf(stderr,"Usage: audit_competition_formats <directory>\n");return 2;}
    csv=fopen("COMPETITION_FORMAT_AUDIT.csv","wb");
    if(!csv){fprintf(stderr,"[comp-formats] FAIL opening CSV\n");return 2;}
    fputs("category,id,format,teams,cycles,begin_phase,end_phase,native_id,data_check,windows_runtime\n",csv);
    for(unsigned cat=0;cat<4u;++cat)audit_competitions(argv[1],cat);
    audit_links(argv[1],"country_competitions","countries","leagues",0,"cups",1);
    audit_qualifying(argv[1]);
    audit_blocks(argv[1]);
    audit_career(argv[1]);
    fclose(csv);
    printf("COMP_FORMAT_DEFINITIONS=%u LEAGUES=%u CUPS=%u TOURNAMENTS=%u PLAYOFFS=%u QUALIFIER_GROUPS=%u\n",
           nknown,leagues,cups,tournaments,playoffs,qualifiers);
    printf("COMP_FORMAT_LEAGUE_CYCLES_1=%u _2=%u _3=%u _4=%u\n",
           league_cycles[1],league_cycles[2],league_cycles[3],league_cycles[4]);
    printf("COMP_FORMAT_TOURNAMENTS_WITH_GROUPS=%u COMP_FORMAT_TWO_LEGGED=%u COMP_FORMAT_CROSS_REFERENCES=%u\n",
           group_tournaments,knockout_legs,refs);
    printf("COMP_FORMAT_DATA_ERRORS=%u\n",errors);
    printf("COMP_FORMAT_DATA_VALIDATION=%s\n",errors?"FAIL":"PASS");
    puts("COMP_FORMAT_OG_LEAGUE_HANDOFF=NOT_TESTED");
    puts("COMP_FORMAT_DOMESTIC_CUPS_RUNTIME=NOT_TESTED");
    puts("COMP_FORMAT_CONTINENTAL_CUPS_RUNTIME=NOT_TESTED");
    puts("COMP_FORMAT_NATIONAL_TEAMS_RUNTIME=NOT_TESTED");
    puts("COMP_FORMAT_QUALIFIERS_RUNTIME=NOT_TESTED");
    puts("COMP_FORMAT_SAVE_LOAD_RUNTIME=NOT_TESTED");
    puts("COMP_FORMAT_REPORT=COMPETITION_FORMAT_AUDIT.csv");
    return errors?1:0;
}
