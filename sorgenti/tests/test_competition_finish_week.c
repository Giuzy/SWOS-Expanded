/* RG-027: pure Sunday cadence. No OG score, RNG, keycache or save. */
#include <stdint.h>
#include <stdio.h>
#include "expanded_competition_finish_week.h"
static int leap(int y) { return y%4==0 && (y%100!=0 || y%400==0); }
static int dim(int y,int m) {
    static const int d[]={31,28,31,30,31,30,31,31,30,31,30,31};
    return d[m-1] + (m==2 && leap(y));
}
static int32_t index_of(int y,int m,int day) {
    int32_t n=0;
    for(int k=1900;k<y;k++)n+=leap(k)?366:365;
    for(int k=1;k<m;k++)n+=dim(y,k);
    return n+day-1;
}
static int die(const char *s) { fprintf(stderr,"FAIL %s\n",s);return 1; }
int main(void) {
    int32_t step=0;
    if(ExpandedFinish_DaysToNextSundayOrEnd(-1,7,&step) ||
       ExpandedFinish_DaysToNextSundayOrEnd(7,7,&step) ||
       ExpandedFinish_DaysToNextSundayOrEnd(0,0,&step) ||
       ExpandedFinish_DaysToNextSundayOrEnd(0,-3,&step) ||
       ExpandedFinish_DaysToNextSundayOrEnd(0,7,NULL))
        return die("invalid input must fail closed");
    for(int weekday=0;weekday<7;weekday++) {
        for(int32_t remaining=1;remaining<60;remaining++) {
            const int32_t sunday=(weekday==6?7:6-weekday);
            const int32_t expected=remaining<sunday?remaining:sunday;
            if(!ExpandedFinish_DaysToNextSundayOrEnd(
                weekday,remaining,&step) || step!=expected ||
               step<1 || step>7)
                return die("weekday or final cap");
        }
    }
    int32_t current=index_of(2027,5,1);
    const int32_t end=index_of(2027,6,30);
    const int32_t first=current;
    const int32_t may2=index_of(2027,5,2);
    const int32_t june27=index_of(2027,6,27);
    unsigned count=0;
    while(current<end) {
        const int weekday=(int)(current%7); /* 1900-01-01 Mon0 */
        const int32_t remaining=end-current;
        if(!ExpandedFinish_DaysToNextSundayOrEnd(
            weekday,remaining,&step))return die("May-June step");
        current+=step;
        if(++count>12)return die("infinite loop");
        if(current<end && current%7!=6)return die("intermediate not Sunday");
        if(count==1 && current!=may2)return die("first Sunday must be May 2");
        if(count==9 && current!=june27)return die("ninth Sunday June 27");
    }
    if(first>=end || current!=end || count!=10u ||
       (int)(end%7)!=2)return die("end exactly June 30 Wed");
    puts("PASS RG027 SUN 7*59 + 2027-05-01 -> 2027-06-30, 10 ticks");
    return 0;
}
