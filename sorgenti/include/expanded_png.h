#ifndef EXPANDED_PNG_H
#define EXPANDED_PNG_H
/* Lossless, non-interlaced PNG reader for SDL2 front-end RGBA textures.
 * Supports 8-bit PNG RGB (color type 2) and RGBA (color type 6).
 * Returns tightly packed RGBA bytes owned by the caller (free with free()).
 * NULL on missing/corrupt/unsupported input; outputs remain 0 on failure.
 * Legacy BMP/RAW loaders and gameplay are unaffected. */
unsigned char *Expanded_PNG_LoadRGBA(const char *path,
                                    unsigned *width, unsigned *height);
#endif
