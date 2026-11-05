#include <assert.h>
#include <stdint.h>
#include <stdio.h>
extern uint8_t _expandedFanMood,_expandedBoardMood;
void Expanded_TriggerFirstWelcomeSackingDemo(void);
int Expanded_DismissalPending(void);
int main(void) {
    _expandedFanMood=80;_expandedBoardMood=80;
    Expanded_TriggerFirstWelcomeSackingDemo();
    assert(_expandedFanMood==19 && _expandedBoardMood==20);
    assert((unsigned)_expandedFanMood+(unsigned)_expandedBoardMood==39);
    assert(Expanded_DismissalPending());
    _expandedFanMood=80;_expandedBoardMood=80;
    Expanded_TriggerFirstWelcomeSackingDemo();
    assert(_expandedFanMood==80 && _expandedBoardMood==80);
    puts("PASS: one-shot demo dismissal at 39, no repeat on rehire");
    return 0;
}
