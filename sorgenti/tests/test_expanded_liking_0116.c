#include <assert.h>
#include <stdint.h>
#include <stdio.h>
unsigned Expanded_InitialFanLiking(unsigned fame);
unsigned Expanded_InitialBoardLiking(unsigned fame,unsigned premium,unsigned raises,int32_t balance);
unsigned Expanded_OfferAcceptanceChance(uint32_t base,uint32_t ask,unsigned fame,unsigned rejected);
int main(void) {
    for(unsigned i=0;i<5;i++) assert(Expanded_InitialFanLiking(i)==20+20*i);
    assert(Expanded_InitialFanLiking(8)==100);
    assert(Expanded_InitialBoardLiking(0,0,0,500000)>=70);
    assert(Expanded_InitialBoardLiking(0,30,1,500000)<Expanded_InitialBoardLiking(0,0,0,500000));
    assert(Expanded_InitialBoardLiking(0,60,2,-1000)<Expanded_InitialBoardLiking(0,60,2,500000));
    assert(Expanded_OfferAcceptanceChance(1000,1100,0,0)>0); /* unknown can earn a raise */
    assert(Expanded_OfferAcceptanceChance(1000,1500,0,0)==0); /* but not 50% */
    assert(Expanded_OfferAcceptanceChance(1000,1100,4,0)>Expanded_OfferAcceptanceChance(1000,1100,0,0));
    puts("PASS: fame tiers, board terms, finances and attainable wage increase");
    return 0;
}
