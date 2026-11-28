#ifndef EXPANDED_CE_DISCIPLINE_H
#define EXPANDED_CE_DISCIPLINE_H
/* v00.13.355 team disciplinary points per played CE fixture (UEFA Nations
 * League art. 15/19 criterion DISCIPLINE; generic). Recorded ONLY by the CE
 * completion writers from the OG witness of a real 50 Hz match (bookings +
 * 3 x sendings-off, OG team statistics: a second-yellow dismissal counts 3,
 * the cancelled first yellow 0). OG QUICK results carry no cards: no entry
 * (criterion not decisive). Team officials' cards are not produced by OG
 * (documented gap). Persisted in the same atomic CAR ("ce_discipline"). */
#include <stdint.h>

int ExpCeDisc_Record(uint64_t event_id, uint32_t home_points, uint32_t away_points);
int ExpCeDisc_Get(uint64_t event_id, uint32_t *home_points, uint32_t *away_points);
void ExpCeDisc_Undo(uint64_t event_id);
void ExpCeDisc_Reset(void);
uint32_t ExpCeDisc_Count(void);
char *ExpCeDisc_Encode(void);            /* malloc'd "DS01;...", NULL when empty */
int ExpCeDisc_Validate(const char *text);
int ExpCeDisc_Decode(const char *text);  /* transactional */
#endif
