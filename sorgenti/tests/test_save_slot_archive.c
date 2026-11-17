/* Isolated regression for non-destructive replacement of legacy career autosaves.
 * Build: cc -std=c11 -Wall -Wextra -Werror -I../include test_save_slot_archive.c -o test_slot && ./test_slot
 */
#include "save_slot_archive.h"
#include <stdlib.h>
#include <time.h>
#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_MKDIR(dir) _mkdir(dir)
#define TEST_RMDIR(dir) _rmdir(dir)
#define TEST_PID() _getpid()
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(dir) mkdir((dir),0700)
#define TEST_RMDIR(dir) rmdir(dir)
#define TEST_PID() getpid()
#endif
static void must(int yes, const char *description) {
    if (!yes) {fprintf(stderr,"FAIL %s\n",description);exit(1);}
}
static void write_sample(const char *path, const char *value) {
    FILE *f=fopen(path,"wb");
    must(f!=NULL,"create sample file");
    must(fwrite(value,1,strlen(value),f)==strlen(value),"write sample file");
    must(fclose(f)==0,"close sample file");
}
static int equal_sample(const char *path, const char *expected) {
    char b[32]={0}; FILE *f=fopen(path,"rb");
    if(!f)return 0;
    size_t n=fread(b,1,sizeof b,f);
    fclose(f);
    return n==strlen(expected) && !memcmp(b,expected,n);
}
int main(void) {
    char dir[150],active[240],unrelated[240],legacy[240],arch1[400],arch2[400];
    snprintf(dir,sizeof dir,"test_slot_archive_%lu_%lu",
             (unsigned long)time(NULL),(unsigned long)TEST_PID());
    must(TEST_MKDIR(dir)==0,"create private test directory");
    snprintf(active,sizeof active,"%s/AUTO9697.CAR",dir);
    snprintf(unrelated,sizeof unrelated,"%s/AUTO2627.CAR",dir);
    snprintf(legacy,sizeof legacy,"%s/AUTOSAVE.CAR",dir);
    write_sample(active,"legacy-96");
    write_sample(unrelated,"other-world");
    write_sample(legacy,"global-legacy");
    must(SaveSlot_ArchiveIncompatible(dir,"AUTO9697.CAR",arch1,sizeof arch1),"archive first legacy");
    must(equal_sample(arch1,"legacy-96"),"archive retains exact old bytes");
    write_sample(active,"new-96");
    must(SaveSlot_ArchiveIncompatible(dir,"AUTO9697.CAR",arch2,sizeof arch2),"archive second legacy");
    must(strcmp(arch1,arch2)!=0,"archive name is unique");
    must(equal_sample(arch1,"legacy-96"),"previous archive never overwritten");
    must(equal_sample(arch2,"new-96"),"second archive retains bytes");
    must(equal_sample(unrelated,"other-world"),"other world unchanged");
    must(equal_sample(legacy,"global-legacy"),"AUTOSAVE.CAR unchanged");
    must(remove(arch1)==0 && remove(arch2)==0,"cleanup archives");
    must(remove(unrelated)==0 && remove(legacy)==0,"cleanup protected saves");
    must(TEST_RMDIR(dir)==0,"cleanup folder");
    puts("SAVE_SLOT_ARCHIVE_ISOLATED=PASS");
    return 0;
}
