#ifndef __PLAYER_H
#define __PLAYER_H

#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <stage.h>
#include <sfx.h>


extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);

extern "C" u32 GenerateRandomNumber(int max);
extern "C" char usedForDeterminingStatePress_or_playerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther, int unk1);

extern "C" void *StopBGMMusic();
extern "C" void *StartBGMMusic();

extern "C" void *MakeMarioEnterDemoMode();
extern "C" void *MakeMarioExitDemoMode();
extern "C" void *UpdateGameMgr();


class daBoss : public dEn_c {
public:
	u32 timer;
	char dying;
	int damage;

	daKameckDemo *Kameck;
};


// Functions
void DamagePlayer(dEn_c* actor, ActivePhysics *apThis, ActivePhysics *apOther);
void SetupKameck(daBoss* actor, daKameckDemo *Kameck);
void CleanupKameck(daBoss* actor, daKameckDemo *Kameck);
bool GrowBoss(daBoss* actor, daKameckDemo *Kameck, float initialScale, float endScale, float yPosModifier, int timer);
void OutroSetup(daBoss* actor);
bool ShrinkBoss(daBoss *actor, Vec *pos, float scale, int timer);
void BossExplode(daBoss* actor, Vec *pos);
void PlayerVictoryCries(daBoss* actor);
void BossGoalForAllPlayers();

#endif

