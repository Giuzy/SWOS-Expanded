#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "game_types.h"
uint8_t _seasonInformationsArray[20*sizeof(SeasonInformations)];
uint16_t _currentSeasonNumber=0;
volatile uint16_t _youreSacked=0;
void Expanded_TriggerFirstWelcomeSackingDemo(void);
void Expanded_DismissalAcknowledged(void);
int Expanded_DismissalPending(void);
int main(void) {
    SeasonInformations *history=(SeasonInformations*)_seasonInformationsArray;
    history[0].managerStatus=0;
    Expanded_TriggerFirstWelcomeSackingDemo();
    assert(Expanded_DismissalPending());
    Expanded_DismissalAcknowledged();
    assert(!Expanded_DismissalPending());
    assert(_youreSacked==1);
    assert(history[0].managerStatus==-1);
    puts("PASS: sacking persisted immediately in management record");
    return 0;
}
