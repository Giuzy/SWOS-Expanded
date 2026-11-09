#ifndef EXPANDED_NEW_CAREER_H
#define EXPANDED_NEW_CAREER_H
#ifdef SWOS_SDL_BUILD
#include <SDL.h>
int Expanded_NewCareer_Active(void);
int Expanded_NewCareer_InputActive(void);
int Expanded_NewCareer_Proc(void);
void Expanded_NewCareer_Reset(void);
int Expanded_NewCareer_Unemployed(void);
void Expanded_NewCareer_Back(void);
int Expanded_NewCareer_TakeBack(void);
void Expanded_NewCareer_Draw(SDL_Renderer *,float,float);
void Expanded_NewCareer_Destroy(void);
#endif
#endif
