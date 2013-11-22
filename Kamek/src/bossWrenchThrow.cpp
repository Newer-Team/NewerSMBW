#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include "boss.h"

class daWrench : public dEn_c {
	int onCreate();
	int onExecute();
	int onDelete();
	int onDraw();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;

	int timer;
	char Kaboom;
	char direction;
	char front;
	float ymod;
	int lifespan;
	u32 cmgr_returnValue;

	static daWrench *build();

	void updateModelMatrices();
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);


	USING_STATES(daWrench);
	DECLARE_STATE(Straight);
	DECLARE_STATE(Kaboom);

};

CREATE_STATE(daWrench, Straight);
CREATE_STATE(daWrench, Kaboom);


extern "C" void *PlayWrenchSound(dEn_c *);


void daWrench::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) { DamagePlayer(this, apThis, apOther); }

bool daWrench::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daWrench::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
}
bool daWrench::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daWrench::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
	S16Vec nullRot = {0,0,0};
	Vec efScale = {0.5f, 0.5f, 0.5f};
	SpawnEffect("Wm_ob_cmnboxgrain", 0, &this->pos, &nullRot, &efScale);

	PlaySoundAsync(this, SE_BOSS_JR_FLOOR_BREAK);
	this->Delete(1);
	return true;
}
bool daWrench::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daWrench::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->_vf220(apOther->owner);

	S16Vec nullRot = {0,0,0};
	Vec efScale = {0.5f, 0.5f, 0.5f};
	SpawnEffect("Wm_ob_cmnboxgrain", 0, &this->pos, &nullRot, &efScale);

	PlaySoundAsync(this, SE_BOSS_JR_FLOOR_BREAK);
	this->Delete(1);
	return true;
}



daWrench *daWrench::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daWrench));
	return new(buffer) daWrench;
}


int daWrench::onCreate() {

	allocator.link(-1, GameHeaps[0], 0, 0x20);

	nw4r::g3d::ResFile rf(getResource("choropoo", "g3d/choropoo.brres"));
	bodyModel.setup(rf.GetResMdl("spanner"), &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);

	allocator.unlink();


	this->direction = this->settings & 0xF;
	this->Kaboom = (this->settings >> 4) & 0xF;
	this->front = (this->settings >> 8) & 0xF;

	ActivePhysics::Info HitMeBaby;

	if (this->Kaboom == 0) {
		HitMeBaby.xDistToCenter = 0.0;
		HitMeBaby.yDistToCenter = 0.0;
		HitMeBaby.xDistToEdge = 5.0;
		HitMeBaby.yDistToEdge = 5.0;

		this->scale.x = 1.25;
		this->scale.y = 1.25;
		this->scale.z = 1.25;
	}

	else {
		HitMeBaby.xDistToCenter = 0.0;
		HitMeBaby.yDistToCenter = 0.0;
		HitMeBaby.xDistToEdge = 8.0;
		HitMeBaby.yDistToEdge = 8.0;

		this->scale.x = 2.0;
		this->scale.y = 2.0;
		this->scale.z = 2.0;
	}

	HitMeBaby.category1 = 0x3;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x47;
	HitMeBaby.bitfield2 = 0xFFFFFFFF;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();


	spriteSomeRectX = 5.0f;
	spriteSomeRectY = 5.0f;
	_320 = 0.0f;
	_324 = 5.0f;

	// These structs tell stupid collider what to collide with - these are from koopa troopa
	static const lineSensor_s below(12<<12, 4<<12, 0<<12);
	static const pointSensor_s above(0<<12, 12<<12);
	static const lineSensor_s adjacent(6<<12, 9<<12, 6<<12);

	collMgr.init(this, &below, &above, &adjacent);
	collMgr.calculateBelowCollisionWithSmokeEffect();

	cmgr_returnValue = collMgr.isOnTopOfTile();


	if (this->direction == 0) 	   { // Ground Facing Left
		this->pos.x -= 0.0; // -32 to +32
		this->pos.y += 36.0;
		this->rot.z = 0x2000;
	}
	else if (this->direction == 1) { // Ground Facing Right
		this->pos.x += 0.0; // +32 to -32
		this->pos.y += 36.0;
		this->rot.z = 0xE000;
	}
	if (this->front == 1) { this->pos.z = -1804.0; }
	else 				  { this->pos.z =  3300.0; }


	if (this->Kaboom) {
		doStateChange(&StateID_Kaboom); }
	else {
		doStateChange(&StateID_Straight); }

	this->onExecute();
	return true;
}


int daWrench::onDelete() {
	return true;
}

int daWrench::onDraw() {
	bodyModel.scheduleForDrawing();
	return true;
}


void daWrench::updateModelMatrices() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


int daWrench::onExecute() {
	acState.execute();
	updateModelMatrices();

	float rect[] = {this->_320, this->_324, this->spriteSomeRectX, this->spriteSomeRectY};
	int ret = this->outOfZone(this->pos, (float*)&rect, this->currentZoneID);
	if(ret) {
		this->Delete(1);
	}

	return true;
}


void daWrench::beginState_Kaboom() {
	float rand = (float)GenerateRandomNumber(10) * 0.4;

	if (this->direction == 0) { // directions 1 spins clockwise, fly rightwards
		speed.x = 1.0 + rand;
	}
	else {						// directions 0 spins anti-clockwise, fly leftwards
		speed.x = -1.0 - rand;
	}

	speed.y = 6.0;
}
void daWrench::executeState_Kaboom() {

	speed.y = speed.y - 0.01875;

	HandleXSpeed();
	HandleYSpeed();
	doSpriteMovement();

	cmgr_returnValue = collMgr.isOnTopOfTile();
	collMgr.calculateBelowCollisionWithSmokeEffect();

	if (collMgr.isOnTopOfTile()) {
		// hit the ground
		PlaySoundAsync(this, SE_BOSS_JR_BOMB_BURST);

		S16Vec efRot = {0,0,0};
		Vec efScale1 = {0.75f, 0.75f, 0.75f};
		Vec efScale2 = {1.25f, 1.25f, 1.25f};

		SpawnEffect("Wm_en_burst_s", 0, &this->pos, &efRot, &efScale1);
		SpawnEffect("Wm_mr_wirehit", 0, &this->pos, &efRot, &efScale2);
		this->Delete(1);
	}
	if (collMgr.outputMaybe & (0x15 << direction)) {
		// hit the wall
		PlaySoundAsync(this, SE_BOSS_JR_BOMB_BURST);

		if (this->direction == 0) { // directions 1 spins clockwise, fly rightwards
			S16Vec efRot = {0,0x4000,0};
			Vec efScale1 = {0.75f, 0.75f, 0.75f};
			Vec efScale2 = {1.25f, 1.25f, 1.25f};
			SpawnEffect("Wm_en_burst_s", 0, &this->pos, &efRot, &efScale1);
			SpawnEffect("Wm_mr_wirehit", 0, &this->pos, &efRot, &efScale2);
		}
		else {						// directions 0 spins anti-clockwise, fly leftwards
			S16Vec efRot = {0,-0x2000,0};
			Vec efScale1 = {0.75f, 0.75f, 0.75f};
			Vec efScale2 = {1.25f, 1.25f, 1.25f};
			SpawnEffect("Wm_en_burst_s", 0, &this->pos, &efRot, &efScale1);
			SpawnEffect("Wm_mr_wirehit", 0, &this->pos, &efRot, &efScale2);
		}

		this->Delete(1);
	}

	if (this->direction == 1) { // directions 1 spins clockwise, fly rightwards
		this->rot.z -= 0x1000; }
	else {						// directions 0 spins anti-clockwise, fly leftwards
		this->rot.z += 0x1000; }

	PlayWrenchSound(this);

}
void daWrench::endState_Kaboom() { }



void daWrench::beginState_Straight() {
	float rand = (float)GenerateRandomNumber(10) * 0.4;

	if (this->direction == 0) { // directions 1 spins clockwise, fly rightwards
		speed.x = 1.0 + rand;
	}
	else {						// directions 0 spins anti-clockwise, fly leftwards
		speed.x = -1.0 - rand;
	}

	speed.y = 6.0;
}
void daWrench::executeState_Straight() {

	speed.y = speed.y - 0.01875;

	HandleXSpeed();
	HandleYSpeed();
	doSpriteMovement();

	cmgr_returnValue = collMgr.isOnTopOfTile();
	collMgr.calculateBelowCollisionWithSmokeEffect();

	if (collMgr.isOnTopOfTile()) {
		// hit the ground
		PlaySoundAsync(this, SE_BOSS_JR_FLOOR_BREAK);

		S16Vec nullRot = {0,0,0};
		Vec efScale = {0.75f, 0.75f, 0.75f};
		SpawnEffect("Wm_en_burst_s", 0, &this->pos, &nullRot, &efScale);
		this->Delete(1);
	}
	if (collMgr.outputMaybe & (0x15 << direction)) {
		// hit the wall
		PlaySoundAsync(this, SE_BOSS_JR_FLOOR_BREAK);

		S16Vec nullRot = {0,0,0};
		Vec efScale = {0.75f, 0.75f, 0.75f};
		SpawnEffect("Wm_en_burst_s", 0, &this->pos, &nullRot, &efScale);
		this->Delete(1);
	}

	if (this->direction == 1) { // directions 1 spins clockwise, fly rightwards
		this->rot.z -= 0x1000; }
	else {						// directions 0 spins anti-clockwise, fly leftwards
		this->rot.z += 0x1000; }

	PlayWrenchSound(this);

}
void daWrench::endState_Straight() { }









