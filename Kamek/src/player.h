#ifndef __PLAYER_H
#define __PLAYER_H

#include <common.h>
#include <game.h>

char NearestPlayer(dStageActor_c* actor);
void setNewActivePhysicsRect(dStageActor_c* actor, Vec* scale);
void changeActivePhysicsRect(dStageActor_c* actor, float xc, float yc, float xe, float ye);

#endif

