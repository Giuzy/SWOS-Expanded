#ifndef SWOS_SAVE_POINTER_GUARD_H
#define SWOS_SAVE_POINTER_GUARD_H
#include <stdint.h>
/* In 64-bit SDL, an OG raw offset or 32-bit sentinel cannot be a live
 * host pointer. Amiga/other builds preserve their original pointer rules. */
static inline int SavePointer_IsInvalidHostString(const void *p)
{
#if defined(SWOS_SDL_BUILD) && UINTPTR_MAX > 0xFFFFFFFFu
    const uintptr_t addr=(uintptr_t)p;
    return p && (addr<0x10000u || addr==UINTPTR_MAX ||
                 addr==(uintptr_t)0xFFFFFFFFu);
#else
    (void)p;
    return 0;
#endif
}
#endif
