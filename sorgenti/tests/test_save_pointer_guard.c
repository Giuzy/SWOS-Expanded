/* Isolated predicate test, not an SWOS/Windows runtime test.
 * cc -std=c11 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -Isorgenti/include \
 *    sorgenti/tests/test_save_pointer_guard.c -o test_guard && ./test_guard
 */
#include "save_pointer_guard.h"
#include <stdio.h>
int main(void)
{
    static char valid[]="DIV 1";
    if(!SavePointer_IsInvalidHostString((void *)(uintptr_t)0xec0u))return 1;
    if(!SavePointer_IsInvalidHostString((void *)(uintptr_t)0xFFFFFFFFu))return 2;
    if(!SavePointer_IsInvalidHostString((void *)(uintptr_t)UINTPTR_MAX))return 3;
    if(SavePointer_IsInvalidHostString(valid))return 4;
    if(SavePointer_IsInvalidHostString(NULL))return 5;
    puts("SAVE_POINTER_GUARD_ISOLATED=PASS");
    return 0;
}
