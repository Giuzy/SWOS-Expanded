/* RG-010/014: preserve the ORIGINAL native menu blinking bit
 * in Expanded CAREER JOB OFFERS without a new frame clock. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_career_job_offer_flash.h"

int main(void)
{
    for(unsigned c=0;c<32u;++c) {
        assert(ExpandedCareer_JobOfferFlashOn(3u,5u,0x2000u,
                                              (uint16_t)c,0)==(c<16u));
        assert(!ExpandedCareer_JobOfferFlashOn(3u,5u,0u,
                                               (uint16_t)c,0));
        assert(!ExpandedCareer_JobOfferFlashOn(3u,5u,0x2000u,
                                               (uint16_t)c,1));
        assert(!ExpandedCareer_JobOfferFlashOn(3u,7u,0x2000u,
                                               (uint16_t)c,0));
        assert(!ExpandedCareer_JobOfferFlashOn(2u,5u,0x2000u,
                                               (uint16_t)c,0));
    }
    assert(ExpandedCareer_JobOfferFlashOn(3u,5u,0x2000u,32u,0));
    assert(!ExpandedCareer_JobOfferFlashOn(3u,5u,0x2000u,48u,0));
    puts("RG-010/014 JOB_OFFER_FLASH_ISO PASS: 162 pulse, flag, and mode cases");
    puts("WINDOWS_VISUAL_AND_CONTROLLER: NOT_TESTED");
    return 0;
}
