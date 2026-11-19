/* SWOS Expanded single-CAR Zstandard container (SDL/Windows).
 * Copyright/third party: docs/ZSTANDARD/LICENSE and README.md.
 * Old uncompressed JSON .CAR files remain readable. */
#ifndef SWOS_SAVE_CAR_ZSTD_H
#define SWOS_SAVE_CAR_ZSTD_H
#include <stdint.h>
#if defined(SWOS_SDL_BUILD) && defined(SWOS_CAR_ZSTD)
/* Independent of current OS path. Encoded buffers use malloc/free;
 * decoded buffers use swos_io_alloc/swos_io_free. */
int SaveCarZstd_IsFrame(const uint8_t *bytes,uint32_t size);
int SaveCarZstd_Encode(const uint8_t *json,uint32_t jsonSize,
                       uint8_t **frame,uint32_t *frameSize);
int SaveCarZstd_Decode(const uint8_t *frame,uint32_t frameSize,
                       uint8_t **json,uint32_t *jsonSize);
#endif
#endif
