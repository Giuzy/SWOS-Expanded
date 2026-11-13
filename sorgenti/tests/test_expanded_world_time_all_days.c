#include <assert.h>
#include <stdio.h>
#include "expanded_world_time.h"
int main(void) {
    ExpandedWorldDate d, back;
    int32_t n, max, previous;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2200,12,31}, &max));
    for (n = 0; n <= max; ++n) {
        assert(ExpandedWorld_IndexToDate(n, &d));
        assert(ExpandedWorld_DateToIndex(d, &previous));
        assert(previous == n && ExpandedWorld_Weekday(n) == n % 7);
    }
    /* Every canonical OG phase (8 per month, Jan to next years) must
     * survive EXACT phase->first-day->phase in both directions.
     * This remains true across 30+ years including leap boundaries. */
    unsigned checked_phases=0,checked_civil_days=0;
    for(unsigned year=1996u;year<=2026u;++year) {
        for(unsigned native_key=0u;native_key<255u;++native_key) {
            ExpandedWorldDate canonical;
            uint8_t floor_key=255u,next_key=255u;
            assert(ExpandedWorld_NativePhaseToDate(
                (uint16_t)year,(uint8_t)native_key,&canonical));
            assert(ExpandedWorld_DateToNativePhaseFloor(
                (uint16_t)year,canonical,&floor_key));
            assert(ExpandedWorld_DateToNativePhaseNext(
                (uint16_t)year,canonical,&next_key));
            if(floor_key!=native_key || next_key!=native_key) {
                fprintf(stderr,"OG_PHASE_ROUNDTRIP FAIL year=%u key=%u "
                               "canonical=%04u-%02u-%02u floor=%u next=%u\n",
                        year,native_key,(unsigned)canonical.year,
                        (unsigned)canonical.month,(unsigned)canonical.day,
                        (unsigned)floor_key,(unsigned)next_key);
                return 1;
            }
            ++checked_phases;
        }
        for(unsigned month=1u;month<=12u;++month) {
            ExpandedWorldDate first={(uint16_t)year,(uint8_t)month,1u};
            ExpandedWorldDate next_month={(uint16_t)(year+(month==12u)),
                         (uint8_t)(month==12u?1u:month+1u),1u};
            int32_t a,b;
            assert(ExpandedWorld_DateToIndex(first,&a));
            assert(ExpandedWorld_DateToIndex(next_month,&b));
            assert(b>a && b-a>=28 && b-a<=31);
            for(int32_t day_index=a;day_index<b;++day_index) {
                ExpandedWorldDate date,from_floor,from_next;
                uint8_t floor_key=255u,next_key=255u,back_key=255u;
                int32_t start_day=0,next_day=0;
                assert(ExpandedWorld_IndexToDate(day_index,&date));
                assert(ExpandedWorld_DateToNativePhaseFloor(
                    (uint16_t)year,date,&floor_key));
                assert(ExpandedWorld_DateToNativePhaseNext(
                    (uint16_t)year,date,&next_key));
                assert(ExpandedWorld_NativePhaseToDate(
                    (uint16_t)year,floor_key,&from_floor));
                assert(ExpandedWorld_NativePhaseToDate(
                    (uint16_t)year,next_key,&from_next));
                assert(ExpandedWorld_DateToIndex(from_floor,&start_day));
                assert(ExpandedWorld_DateToIndex(from_next,&next_day));
                if(!(start_day<=day_index && next_day>=day_index &&
                     next_day-start_day<=4 && from_floor.month==month)) {
                    fprintf(stderr,"OG_MONTH_PARTITION FAIL %04u-%02u-%02u "
                                   "floor=%u next=%u start=%ld nextStart=%ld\n",
                            year,month,(unsigned)date.day,
                            (unsigned)floor_key,(unsigned)next_key,
                            (long)start_day,(long)next_day);
                    return 1;
                }
                assert(ExpandedWorld_DateToNativePhaseFloor(
                    (uint16_t)year,from_floor,&back_key));
                assert(back_key==floor_key);
                assert(ExpandedWorld_DateToNativePhaseNext(
                    (uint16_t)year,from_next,&back_key));
                assert(back_key==next_key);
                if(day_index==start_day)assert(next_key==floor_key);
                else assert(next_key==floor_key+1u);
                /* The month ends at its last real day; the next
                 * first-of-month is always the next native phase. */
                if(day_index==b-1) {
                    assert(floor_key==(month-1u)*8u+7u);
                    assert(next_key==month*8u);
                }
                ++checked_civil_days;
            }
        }
    }
    assert(!ExpandedWorld_NativePhaseToDate(1996u,255u,&back));
    { uint8_t invalid=255u;
      assert(!ExpandedWorld_DateToNativePhaseFloor(
          1996u,(ExpandedWorldDate){1996,2,30},&invalid)); }
    {
        const ExpandedWorldDate origin={1996u,7u,1u};
        ExpandedWorldDate t5,t30,t;
        int32_t start,after5,after30;
        assert(ExpandedWorld_AddCalendarYears(origin,5u,&t5));
        assert(t5.year==2001u && t5.month==7u && t5.day==1u);
        assert(ExpandedWorld_AddCalendarYears(origin,30u,&t30));
        assert(t30.year==2026u && t30.month==7u && t30.day==1u);
        assert(ExpandedWorld_DateToIndex(origin,&start));
        assert(ExpandedWorld_DateToIndex(t5,&after5));
        assert(ExpandedWorld_DateToIndex(t30,&after30));
        assert(after5-start==1826 && after30-start==10957);
        assert(ExpandedWorld_AddCalendarYears((ExpandedWorldDate){2000,2,29},
                                               5u,&t));
        assert(t.year==2005u && t.month==2u && t.day==28u);
        t=origin;
        for(unsigned i=0;i<6u;++i)
            assert(ExpandedWorld_AddCalendarYears(t,5u,&t));
        assert(t.year==t30.year && t.month==t30.month && t.day==t30.day);
        for(unsigned years=0u;years<=30u;++years) {
            uint8_t phase=255u;
            assert(ExpandedWorld_AddCalendarYears(origin,(uint16_t)years,&t));
            assert(ExpandedWorld_DateToNativePhaseFloor(t.year,t,&phase));
            assert(phase==48u); /* July 1 stays July 1, every year. */
        }
    }
    puts("OG_CANONICAL_PHASE_ROUNDTRIP_30_YEARS=PASS");
    printf("OG_PHASE_COVERAGE_PHASES=%u CIVIL_DAYS=%u\n",
           checked_phases,checked_civil_days);
    puts("OG_MONTH_EIGHT_BUCKETS_NO_OVERLAP=PASS");
    puts("FIVE_YEAR_ANNIVERSARY_EXACT=PASS");
    assert(!ExpandedWorld_IndexToDate(-1, &back));
    assert(!ExpandedWorld_IndexToDate(max+1, &back));
    printf("OK: verified %d days (1900-01-01 through 2200-12-31)\n", (int)(max+1));
    return 0;
}
