#ifndef __EFFECTS_H
#define __EFFECTS_H

#include <common.h>
#include <game.h>

/*
S16Vec* tRot = &(S16Vec){ 0, 0, 0};
Vec* tScale = &(Vec){0.0, 0.0, 0.0};
*/

bool CreateEffect(dEn_c* enemy, int effect_num);
bool CreateEffect(Vec* pos, S16Vec* rot, Vec* scale, int effect_num);
bool CreateEffect(Vec* pos, S16Vec* rot, Vec* scale, int effect_num);
bool CreateEffect(int effect_num, Vec* pos, int leng);
bool CreateEffect(int effect_num, Vec* pos, S16Vec* rot = 0, Vec* scale = 0);

// Actually will return an EGG::Effect instance
//void* CustomEffectSpawn(const char* name);

#endif
