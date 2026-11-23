/* SWOS Expanded single-CAR Zstandard container (SDL/Windows).
 * Copyright/third party: docs/ZSTANDARD/LICENSE and README.md.
 * Old uncompressed JSON .CAR files remain readable. Two containers, one
 * reader: 'SWOSZC3A' (one frame) and 'SWOSZC4A' (indexed, cached blocks). */
#ifndef SWOS_SAVE_CAR_ZSTD_H
#define SWOS_SAVE_CAR_ZSTD_H
#include <stdint.h>
#if defined(SWOS_SDL_BUILD) && defined(SWOS_CAR_ZSTD)
/* Statistics of the LAST EncodeBlocks (writer thread). */
typedef struct SaveCarZstdStats {
    uint32_t hits,misses,blocks;
    uint64_t raw_reused,raw_total,cache_bytes;
} SaveCarZstdStats;
/* Independent of current OS path. Encoded buffers use malloc/free;
 * decoded buffers use swos_io_alloc/swos_io_free. */
int SaveCarZstd_IsFrame(const uint8_t *bytes,uint32_t size);   /* either magic */
int SaveCarZstd_IsBlocks(const uint8_t *bytes,uint32_t size);  /* v4 only */
int SaveCarZstd_Encode(const uint8_t *json,uint32_t jsonSize,
                       uint8_t **frame,uint32_t *frameSize);
/* Indexed blocks: `cuts` are strictly ascending offsets inside (0,jsonSize)
 * that split the image into ncuts+1 blocks. Unchanged blocks (same length and
 * FNV64) reuse the previous generation's compressed frame. Single writer. */
int SaveCarZstd_EncodeBlocks(const uint8_t *json,uint32_t jsonSize,
                             const uint32_t *cuts,uint32_t ncuts,
                             uint8_t **frame,uint32_t *frameSize);
int SaveCarZstd_Decode(const uint8_t *frame,uint32_t frameSize,
                       uint8_t **json,uint32_t *jsonSize);
/* Drops the block cache (world switch / new career / any doubt). */
void SaveCarZstd_CacheReset(void);
SaveCarZstdStats SaveCarZstd_Stats(void);
#endif
#endif
