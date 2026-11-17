#ifndef SWOS_SAVE_SLOT_ARCHIVE_H
#define SWOS_SAVE_SLOT_ARCHIVE_H
/* Move an incompatible per-world automatic save aside without overwriting
 * either that file or a previous archive. The caller must have completed
 * serialization of the replacement before invoking this helper. */
#include <errno.h>
#include <stdio.h>
#include <string.h>

static int SaveSlot_ArchiveIncompatible(const char *folder, const char *slot,
                                        char *archived, size_t archivedSize)
{
    char original[1280], candidate[1340];
    int n;
    if (!folder || !slot || !archived || !archivedSize) return 0;
    n=snprintf(original,sizeof original,"%s/%s",folder,slot);
    if (n<0 || (size_t)n>=sizeof original) return 0;
    for (unsigned index=1;index<=9999u;++index) {
        n=snprintf(candidate,sizeof candidate,"%s/%s.legacy-%04u",
                   folder,slot,index);
        if (n<0 || (size_t)n>=sizeof candidate ||
            (size_t)n>=archivedSize) return 0;
        FILE *previous=fopen(candidate,"rb");
        if (previous) { fclose(previous); continue; }
        if (errno!=ENOENT) return 0;
        /* On Windows rename fails if a destination already exists. */
        if (rename(original,candidate)==0) {
            memcpy(archived,candidate,(size_t)n+1u);
            return 1;
        }
        if (errno!=EEXIST) return 0;
    }
    return 0;
}
#endif
