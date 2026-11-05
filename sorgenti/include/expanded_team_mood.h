#ifndef EXPANDED_TEAM_MOOD_H
#define EXPANDED_TEAM_MOOD_H
/* Shared team-wide form/morale. Original SWOS PlayerFile stats are untouched. */
static inline unsigned Expanded_FaceTier(unsigned value) {
    if (value > 100u) value=100u;
    return value >= 80u ? 4u : value >= 60u ? 3u :
           value >= 40u ? 2u : value >= 20u ? 1u : 0u;
}
/* Average 65 = neutral; 100 = +20%; <=20 = -20%.
 * Integer rounding is symmetric at the endpoints. */
static inline unsigned Expanded_TeamPerformancePct(unsigned form,unsigned morale) {
    if(form>100u) form=100u;
    if(morale>100u) morale=100u;
    unsigned avg=(form+morale)/2u;
    if(avg>=65u) return 100u+(20u*(avg-65u)+17u)/35u;
    if(avg<=20u) return 80u;
    return 100u-(20u*(65u-avg)+22u)/45u;
}
#endif
