/* v00.13.290: compact, versioned, portable binary codec for CE fixture rows
 * ("UE02"). PURE module: no game globals, no registry types, usable from the
 * CarWriter thread. Little-endian, columnar, zig-zag LEB128 deltas, FNV-1a/64
 * trailer. One row = 23 unsigned fields with the SAME widths as the UE01 hex
 * rows, so no information is lost. Row identity/order is preserved. */
#ifndef EXPANDED_CAR_COMPACT_H
#define EXPANDED_CAR_COMPACT_H
#include <stddef.h>
#include <stdint.h>
#define EXP_CCOL 23
typedef struct ExpCarCompactReader {
    const uint8_t *col[EXP_CCOL],*end[EXP_CCOL];
    uint64_t prev[EXP_CCOL];
    uint32_t n,next;
    uint16_t world;
    int32_t day;
} ExpCarCompactReader;
/* Column widths in bits (exported for tests). */
extern const uint8_t ExpCarCompact_Bits[EXP_CCOL];
/* Encode n rows; row(i,vals,user) fills the 23 values (returns 0 = abort).
 * Returns a malloc'd buffer (free()) and its length, or NULL on failure. */
uint8_t *ExpCarCompact_Encode(uint16_t world,int32_t day,uint32_t n,
    int (*row)(uint32_t i,uint64_t *vals,void *user),void *user,size_t *len);
/* Validates magic, bounds, column lengths and trailer hash; no allocation. */
int ExpCarCompact_Open(ExpCarCompactReader *r,const uint8_t *buf,size_t len);
/* 1 = row produced; 0 = corrupt or exhausted (value out of width, bad
 * varint, truncated column). */
int ExpCarCompact_Next(ExpCarCompactReader *r,uint64_t *vals);
/* 1 only if every row was read and every column consumed exactly. */
int ExpCarCompact_Finish(const ExpCarCompactReader *r);
/* Standard base64 (JSON-safe string). Returns length written (no NUL) or
 * (size_t)-1. out==NULL: length only. */
size_t ExpCarCompact_B64Encode(const uint8_t *in,size_t n,char *out,size_t cap);
size_t ExpCarCompact_B64Decode(const char *in,size_t n,uint8_t *out,size_t cap);
#endif
