/* Shared bound for the OG career competition table, including its sentinel.
 * Keep the definition out of competition_data.h clients which intentionally
 * declare legacy records as byte arrays (not the typed OG declarations). */
#ifndef SWOS_CAREER_CONTESTS_CAP_H
#define SWOS_CAREER_CONTESTS_CAP_H

/* 34 compiled records, sentinel and additional dynamic competitions.
 * Same storage bound used by competition_data.c and the JSON serializer. */
#define CAREER_CONTESTS_CAP 48

#endif /* SWOS_CAREER_CONTESTS_CAP_H */
