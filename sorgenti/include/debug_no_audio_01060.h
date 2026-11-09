/* SWOS Expanded 0.10.60 ONLY: silent Windows debugging build.
 * Included BEFORE the normal Makefile's forced swos_platform.h;
 * the platform header sets audio to 0 (enabled) by default, so
 * override the switch AFTER importing it. Remove the -include
 * of this header from BUILD_SWOS.sh when audio is re-enabled. */
#ifndef SWOS_EXPANDED_DEBUG_NO_AUDIO_01060_H
#define SWOS_EXPANDED_DEBUG_NO_AUDIO_01060_H
#include "swos_platform.h"
#undef SWOS_AUDIO_DISABLED
#define SWOS_AUDIO_DISABLED 1
#endif
