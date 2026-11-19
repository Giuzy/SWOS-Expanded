/* Read-only OG qualification -> next-edition identity audit.
 * No draws, callbacks, RNG, saves or mutable competition state here.
 * A count match is insufficient: the exact ordered team identity is checked
 * as a SET, and duplicates are explicitly rejected.
 */
#ifndef EXPANDED_QUALIFICATION_TRANSITION_H
#define EXPANDED_QUALIFICATION_TRANSITION_H
#include <stdint.h>
#define EXP_QUAL_MAX_ENTRANTS 64u
/* Historic OG routes have separate verified native lists:
 * 0=Champions (type 2, 16), 1=CWC (type 1, 32), 2=UEFA Cup (type 1, 32).
 * The caller must first obtain an ID-validated current-edition field with
 * QualReadNext; this count gate is never a first-season seed fallback. */
static inline int ExpandedQual_ClassicEditionReady(unsigned route,
                                                  unsigned verified_count)
{
    return route<3u && verified_count==(route==0u?16u:32u);
}
typedef struct {
    unsigned matched,missing,extra,duplicate_source,duplicate_resident;
    unsigned invalid_source,invalid_resident;
    uint16_t first_missing,first_extra;
} ExpandedQualDiff;
static inline int ExpandedQual_SameEdition(unsigned source_world,
                                           uint16_t source_year,
                                           unsigned resident_world,
                                           uint16_t resident_year)
{
    return source_world==resident_world && source_year==resident_year;
}
static inline ExpandedQualDiff ExpandedQual_Compare(const uint16_t *source,
                                                     unsigned source_count,
                                                     const uint16_t *resident,
                                                     unsigned resident_count)
{
    ExpandedQualDiff d={0};
    d.first_missing=d.first_extra=0xffffu;
    if(source_count>EXP_QUAL_MAX_ENTRANTS ||
       resident_count>EXP_QUAL_MAX_ENTRANTS ||
       (source_count && !source) || (resident_count && !resident)) {
        ++d.invalid_source;
        return d;
    }
    for(unsigned i=0;i<source_count;++i) {
        const uint16_t club=source[i];
        if(club==0xffffu){++d.invalid_source;continue;}
        for(unsigned k=0;k<i;++k)
            if(source[k]==club){++d.duplicate_source;break;}
        unsigned found=0;
        for(unsigned k=0;k<resident_count;++k)
            if(resident[k]==club)++found;
        if(found)++d.matched;
        else {++d.missing;if(d.first_missing==0xffffu)d.first_missing=club;}
    }
    for(unsigned i=0;i<resident_count;++i) {
        const uint16_t club=resident[i];
        if(club==0xffffu){++d.invalid_resident;continue;}
        for(unsigned k=0;k<i;++k)
            if(resident[k]==club){++d.duplicate_resident;break;}
        unsigned found=0;
        for(unsigned k=0;k<source_count;++k)
            if(source[k]==club)++found;
        if(!found){++d.extra;if(d.first_extra==0xffffu)d.first_extra=club;}
    }
    return d;
}
static inline int ExpandedQual_IsExact(const ExpandedQualDiff *d)
{
    return d && !d->missing && !d->extra &&
           !d->duplicate_source && !d->duplicate_resident &&
           !d->invalid_source && !d->invalid_resident;
}
/* First edition is a SEED field, not a fictional earned qualification.
 * Manual entrants require an explicitly approved source; invented or
 * unverified lists cannot be promoted to validated first-season data. */
enum {EXP_QUAL_FIRST_NATIVE=1u,EXP_QUAL_FIRST_MANUAL_APPROVED=2u,
      EXP_QUAL_FIRST_MANUAL_UNVERIFIED=3u};
static inline int ExpandedQual_FirstEditionValid(unsigned source,
                                                 const ExpandedQualDiff *d,
                                                 unsigned entries)
{
    return entries>=2u && ExpandedQual_IsExact(d) &&
           (source==EXP_QUAL_FIRST_NATIVE ||
            source==EXP_QUAL_FIRST_MANUAL_APPROVED);
}
#ifdef SWOS_SDL_BUILD
/* Diagnostic only: native source identity vs next year's resident OG IDs. */
void Expanded_OGWorld_QualCaptureNative(uint16_t year,
 const uint8_t *a,unsigned na,const uint8_t *b,unsigned nb,
 const uint8_t *c,unsigned nc,const uint8_t *d,unsigned nd);
void Expanded_OGWorld_QualAuditNextYear(uint16_t year);
/* Validated OG career field for a single new edition. A missing source is
 * never substituted with the original first-year seed list. */
unsigned Expanded_OGWorld_QualReadNext(uint16_t year,unsigned cup,
                                      uint16_t *ids,unsigned capacity);
#endif
#endif
