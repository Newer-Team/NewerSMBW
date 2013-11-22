#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include "boss.h"

class daKoopaBreath : public dEn_c {
	int onCreate();
	int onExecute();

	mEf::es2 effect;
	static daKoopaBreath *build();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);

};

void daKoopaBreath::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) { DamagePlayer(this, apThis, apOther); }
void daKoopaBreath::spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) {}


daKoopaBreath *daKoopaBreath::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daKoopaBreath));
	return new(buffer) daKoopaBreath;
}


int daKoopaBreath::onCreate() {
	
	ActivePhysics::Info GreatBalls;
	
	GreatBalls.xDistToCenter = 0.0;
	GreatBalls.yDistToCenter = 0.0;
	GreatBalls.xDistToEdge = 38.0;
	GreatBalls.yDistToEdge = 28.0;
	
	GreatBalls.category1 = 0x3;
	GreatBalls.category2 = 0x0;
	GreatBalls.bitfield1 = 0x47;
	GreatBalls.bitfield2 = 0xFFFFFFFF;
	GreatBalls.unkShort1C = 0;
	GreatBalls.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &GreatBalls);
	this->aPhysics.addToList();
	speed.x = -2.0; 
	
	this->onExecute();
	return true;
}


int daKoopaBreath::onExecute() {
	HandleXSpeed();
	doSpriteMovement();

	PlaySound(this, SE_BOSS_JR_FIRE_BURNING);

	S16Vec nullRot = {0,0,0};
	Vec threeVec = {3.0f, 3.0f, 3.0f};
	effect.spawn("Wm_ko_fireattack", 0, &pos, &nullRot, &threeVec);

	float rect[] = {0.0, 0.0, 38.0, 38.0};
	int ret = this->outOfZone(this->pos, (float*)&rect, this->currentZoneID);
	if(ret) {
		this->Delete(1);
	}

	return true;
}
