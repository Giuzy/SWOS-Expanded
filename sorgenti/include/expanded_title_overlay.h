#ifndef EXPANDED_TITLE_OVERLAY_H
#define EXPANDED_TITLE_OVERLAY_H
#include <SDL.h>
/* SDL-only new Title Menu: RGB backdrop, RGBA logo and user-approved
 * independent button PNGs. No legacy background, button overlay or F8 path. */
int Expanded_TitleOverlay_Initialize(SDL_Renderer *renderer);
void Expanded_TitleOverlay_Destroy(void);
void Expanded_TitleOverlay_Draw(SDL_Renderer *renderer,float target_w,float target_h);
/* Logic lives in menus.c on the menu thread; renderer only observes state. */
int Expanded_TitleMenu_Level(void);
int Expanded_TitleMenu_Selected(void);
int Expanded_TitleMenu_Pressing(void);
int Expanded_TitleMenu_CareerActive(void);
void Expanded_TitleMenu_QueueBack(void);
#endif
