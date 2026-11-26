#include "input_switch_poweroff.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    const char *bt="\\\\?\\HID#{00001124-0000-1000-8000-00805F9B34FB}_VID&0002057E_PID&2009#9&1&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}";
    const char *usb="\\\\?\\hid#vid_057e&pid_2009#9&1";
    assert(swos_switch_sleep_eligible(0x057e,0x2009,bt));
    assert(!swos_switch_sleep_eligible(0x057e,0x2009,usb));
    assert(!swos_switch_sleep_eligible(0x045e,0x2009,bt));
    assert(!swos_switch_sleep_eligible(0x057e,0x2006,bt));
    assert(!swos_switch_sleep_eligible(0x057e,0x2009,NULL));
    assert(!swos_switch_sleep_eligible(0x057e,0x2009,""));
    assert(!swos_switch_sleep_eligible(0x057e,0x2009,"virtual"));
    for(size_t n=0;n<strlen(bt);n++) {char shortpath[256];memcpy(shortpath,bt,n);shortpath[n]=0;if(n<=strlen("\\\\?\\hid#{00001124-0000-1000-8000-00805f9b34fb}_vid&0002057e_pid&2009#"))assert(!swos_switch_sleep_eligible(0x057e,0x2009,shortpath));}
    uint8_t b[SWOS_SWITCH_SLEEP_REPORT_SIZE];memset(b,255,sizeof b);swos_switch_sleep_report(b);
    const uint8_t expected[]={1,0,0,1,64,64,0,1,64,64,6,0};assert(!memcmp(b,expected,sizeof expected));for(size_t i=sizeof expected;i<sizeof b;i++)assert(!b[i]);
    puts("PASS: Windows Bluetooth path/VID/PID gates, truncated paths, exact sleep report");
}
