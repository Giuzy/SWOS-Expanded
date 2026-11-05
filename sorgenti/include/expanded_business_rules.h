#ifndef EXPANDED_BUSINESS_RULES_H
#define EXPANDED_BUSINESS_RULES_H
#include <stdint.h>
/* SWOS fixture sortKey is absolute, e.g. 56 at season start. */
static inline unsigned Expanded_FinanceWeek(unsigned index,unsigned start) {
    unsigned week=(uint8_t)(index-start)+1u;
    return week>52u?52u:week;
}
static inline uint64_t Expanded_AnnualExpenses(uint64_t weekly) {
    return weekly*52u;
}
#endif
