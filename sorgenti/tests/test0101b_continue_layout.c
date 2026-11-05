/* Regression: the continue button has no MENU_CMD_NAV command. */
#include <stddef.h>
#include <stdint.h>
#include "menu_data.h"
_Static_assert(offsetof(Menu_Continue, markEndMenu) ==
               offsetof(Menu_Continue, entry1) +
               sizeof(MenuEntry_Byte_Colors_TextPtr_OnSelect),
               "continueMenu entry layout must be contiguous");
#if UINTPTR_MAX == 0xffffffffu
_Static_assert(sizeof(Menu_Continue) == 66, "original 32-bit menu is 66 bytes");
#elif UINTPTR_MAX == 0xffffffffffffffffull
_Static_assert(sizeof(Menu_Continue) == 90, "64-bit menu is 90 bytes");
#endif
int main(void) { return 0; }
