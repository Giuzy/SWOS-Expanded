/* SWOS Expanded v00.13.305 — universal Competition Engine viewer.
 *
 * ONE enumeration + ONE OG-style page (data_json/menus/openswos/
 * ceViewerMenu.json) for every CE competition of WORLD2627: leagues,
 * groups, knockouts, single finals, multi-phase competitions and the roll
 * of honour. The page is generated from the CE definition (phases), the CE
 * edition (participants, state, winners) and the CE ledger (dates, results).
 * Strictly readonly: it never writes a result, a fixture, the CAR, the RNG
 * or the clock, and uses no OG competition slot.
 */
#ifndef EXPANDED_CE_VIEWER_H
#define EXPANDED_CE_VIEWER_H
#ifdef SWOS_SDL_BUILD
#include <stdint.h>
#include "expanded_competition_identity.h"
/* 1 = the CE owns the competition viewers (WORLD2627 career, CE active). */
int ExpCeViewer_Active(void);
/* Competitions with the JSON flag "viewer_button" (no OG slot): a country's
 * (WORLD competition list) and the manager's club's (VIEW COMPETITIONS).
 * Fill ids/names, return the count. */
uint32_t ExpCeViewer_WorldButtons(uint8_t country,uint32_t *ids,
                                  char (*names)[40],uint32_t cap);
uint32_t ExpCeViewer_ClubButtons(uint32_t *ids,char (*names)[40],uint32_t cap);
/* OG list rows: record to store in the competition list for a button, and the
 * reverse (1 = it was a button record and the viewer has been shown). */
const void *ExpCeViewer_ButtonRecord(uint32_t def_id);
int ExpCeViewer_OpenRecord(const void *rec);
/* Does a visible original cup row identify this CE button? Readonly. */
int ExpCeViewer_MatchesNative(uint32_t def_id,uint8_t country,const void *rec);
/* Opens the viewer of a competition definition (edition chosen by the CE). */
int ExpCeViewer_OpenDefinition(uint32_t def_id);
/* Actual native OG cup renderer for a materialised, all-KO CE button or native alias. */
int ExpCeViewer_OpenNativeCup(ExpandedEditionKey key);
#endif
#endif
