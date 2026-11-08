#ifndef EXPANDED_TITLE_OVERLAY_H
#define EXPANDED_TITLE_OVERLAY_H
#include <SDL.h>
#include <stdint.h>
/* Approved PNG below the genuine SWOS title menu buttons and live texts.
 * F8 switches PNG/original for visual QA; all callbacks stay intact. */
int Expanded_TitleOverlay_Initialize(SDL_Renderer *renderer);
void Expanded_TitleOverlay_Destroy(void);
int Expanded_TitleOverlay_Ready(void);
int Expanded_TitleOverlay_Enabled(void);
void Expanded_TitleOverlay_Toggle(void);
void Expanded_TitleOverlay_Mask(uint32_t *argb, int dst_pitch,
                                const uint8_t *indexed, int src_pitch,
                                int frame_w, int frame_h);
void Expanded_TitleOverlay_Draw(SDL_Renderer *renderer, SDL_Texture *frame,
                                const SDL_Rect *src, int whole_scale,
                                float target_w, float target_h);
#endif
