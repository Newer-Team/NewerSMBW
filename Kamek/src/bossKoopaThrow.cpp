#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include "boss.h"

struct TypeInfo {
	const char *arcName;
	const char *brresName;
	const char *modelName;
	const char *deathEffect;
	int launchSound;
	int breakSound;
	int flySound;
	float size;
	float scale;
	u16 xrot;
	u16 yrot;
	u16 zrot;
};

static const TypeInfo types[6] = {
	{"choropoo", 			"g3d/choropoo.brres", 			"spanner", 				"Wm_en_hit", 		0, 						SE_BOSS_JR_FLOOR_BREAK, 0, 							8.0f, 	2.0f, 0, 		0, 		0x1000},
	{"choropoo", 			"g3d/choropoo.brres", 			"spanner", 				"Wm_en_burst_s", 	0, 						SE_BOSS_JR_BOMB_BURST, 	0, 							12.0f, 	2.0f, 0, 		0, 		0x1000},
	{"koopa_clown_bomb", 	"g3d/koopa_clown_bomb.brres", 	"koopa_clown_bomb", 	"Wm_en_burst_s", 	SE_EMY_ELCJ_THROW, 		SE_BOSS_JR_BOMB_BURST, 	0, 							16.0f, 	0.8f, 0x200, 	0x800, 	0x1000},
	{"bros", 				"g3d/t00.brres", 				"bros_hammer", 			"Wm_en_hit", 		0, 						SE_OBJ_HAMMER_HIT_BOTH, 0,						 	16.0f, 	2.0f, 0, 		0, 		0x1000},
	{"dossun",				"g3d/t02.brres", 				"dossun", 				"Wm_en_hit", 		SE_EMY_DOSSUN, 			SE_EMY_DOSSUN_DEAD, 	0, 							14.0f, 	1.0f, 0,		0,		0},
	{"KoopaShip",			"g3d/present.brres", 			"PresentBox_penguin", 	"Wm_dm_presentopen",SE_DEMO_OP_PRESENT_THROW_2400f, SE_DEMO_OP_PRESENT_BOX_BURST, 	0, 			20.0f, 	1.0f, 0x20, 	0x40, 	0x200}
};


const char* KPTarcNameList [] = {
	"choropoo",
	"koopa_clown_bomb",
	"dossun",
	"KoopaShip",
	NULL
};


class daKoopaThrow : public dEn_c {
	int onCreate();
	int onExecute();
	int onDelete();
	int onDraw();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;

	int timer;
	char Type;
	char direction;
	char front;
	float ymod;
	int lifespan;
	u32 cmgr_returnValue;

	bool playsAnim;
	m3d::anmChr_c chrAnim;

	nw4r::snd::SoundHandle hammerSound;

	const TypeInfo *currentInfo;

	static daKoopaThrow *build();

	void updateModelMatrices();
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);


	USING_STATES(daKoopaThrow);
	DECLARE_STATE(Straight);

};

CREATE_STATE(daKoopaThrow, Straight);


// Types:
//
//	0 - Wrench
//  1 - Exploding Wrench
//	2 - Bomb
//  3 - Hammer
//  4 - Thwomp
//	5 - Present
//


extern "C" void *PlayWrenchSound(dEn_c *);
extern "C" void *dAcPy_c__ChangePowerupWithAnimation(void * Player, int powerup);
extern "C" int CheckExistingPowerup(void * Player);

void daKoopaThrow::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (Type == 5) {
		PlaySoundAsync(this, currentInfo->breakSound);
		S16Vec nullRot = {0,0,0};
		Vec efScale = {3.0f, 3.0f, 3.0f};
		SpawnEffect(currentInfo->deathEffect, 0, &this->pos, &nullRot, &efScale);
		// dStageActor_c *spawned = CreateActor(EN_ITEM, 0x20000063, this->pos, 0, 0);
		// spawned->pos.x = this->pos.x;
		// spawned->pos.y = this->pos.y;

		int p = CheckExistingPowerup(apOther->owner);
		if (p == 0 || p == 3) {	// Powerups - 0 = small; 1 = big; 2 = fire; 3 = mini; 4 = prop; 5 = peng; 6 = ice; 7 = hammer
			dAcPy_c__ChangePowerupWithAnimation(apOther->owner, 1);
		}

		this->Delete(1);

		return;
	}

	DamagePlayer(this, apThis, apOther);

	if (Type == 1 || Type == 2) {
		PlaySoundAsync(this, SE_BOSS_JR_BOMB_BURST);

		S16Vec nullRot = {0,0,0};
		Vec efScale = {0.75f, 0.75f, 0.75f};
		SpawnEffect("Wm_en_burst_s", 0, &this->pos, &nullRot, &efScale);
		Vec hitScale = {1.25f, 1.25f, 1.25f};
		SpawnEffect("Wm_mr_wirehit", 0, &this->pos, &nullRot, &hitScale);
		this->Delete(1);
	}
}

void daKoopaThrow::spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) {}

bool daKoopaThrow::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daKoopaThrow::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
}
bool daKoopaThrow::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daKoopaThrow::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (Type == 1 || Type == 2) {
		S16Vec nullRot = {0,0,0};
		Vec burstScale = {0.75f, 0.75f, 0.75f};
		SpawnEffect("Wm_en_burst_s", 0, &this->pos, &nullRot, &burstScale);
		Vec hitScale = {1.25f, 1.25f, 1.25f};
		SpawnEffect("Wm_mr_wirehit", 0, &this->pos, &nullRot, &hitScale);
	}
	else {
		S16Vec nullRot = {0,0,0};
		Vec efScale = {0.5f, 0.5f, 0.5f};
		SpawnEffect("Wm_ob_cmnboxgrain", 0, &this->pos, &nullRot, &efScale);
	}

	PlaySoundAsync(this, currentInfo->breakSound);
	this->Delete(1);
	return true;
}
bool daKoopaThrow::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daKoopaThrow::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
	DamagePlayer(this, apThis, apOther);

	if (Type == 1 || Type == 2) {
		PlaySoundAsync(this, SE_BOSS_JR_BOMB_BURST);

		S16Vec nullRot = {0,0,0};
		Vec burstScale = {0.75f, 0.75f, 0.75f};
		SpawnEffect("Wm_en_burst_s", 0, &this->pos, &nullRot, &burstScale);
		Vec hitScale = {1.25f, 1.25f, 1.25f};
		SpawnEffect("Wm_mr_wirehit", 0, &this->pos, &nullRot, &hitScale);
		this->Delete(1);
	}
	return true;
}


daKoopaThrow *daKoopaThrow::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daKoopaThrow));
	return new(buffer) daKoopaThrow;
}


int daKoopaThrow::onCreate() {

	this->direction = this->settings & 0xF;
	this->Type = (this->settings >> 4) & 0xF;
	this->front = (this->settings >> 8) & 0xF;

	currentInfo = &types[Type];

	allocator.link(-1, GameHeaps[0], 0, 0x20);

	nw4r::g3d::ResFile rf(getResource(currentInfo->arcName, currentInfo->brresName));
	nw4r::g3d::ResMdl resMdl = rf.GetResMdl(currentInfo->modelName);

	bodyModel.setup(resMdl, &allocator, (Type == 4 ? 0x224 : 0), 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);

	if (Type == 4) {
		// Thwomp
		playsAnim = true;

		nw4r::g3d::ResAnmChr anmChr = rf.GetResAnmChr("boss_throw");
		chrAnim.setup(resMdl, anmChr, &allocator, 0);
		chrAnim.bind(&bodyModel, anmChr, 1);
		bodyModel.bindAnim(&chrAnim, 0.0);
		chrAnim.setUpdateRate(1.0);
	}

	allocator.unlink();


	ActivePhysics::Info KoopaJunk;

	KoopaJunk.xDistToCenter = 0.0f;
	KoopaJunk.yDistToCenter = (Type == 4) ? currentInfo->size : 0.0;
	KoopaJunk.xDistToEdge = currentInfo->size;
	KoopaJunk.yDistToEdge = currentInfo->size;

	this->scale.x = currentInfo->scale;
	this->scale.y = currentInfo->scale;
	this->scale.z = currentInfo->scale;

	KoopaJunk.category1 = 0x3;
	KoopaJunk.category2 = 0x0;
	KoopaJunk.bitfield1 = 0x47;
	KoopaJunk.bitfield2 = 0xFFFFFFFF;
	KoopaJunk.unkShort1C = 0;
	KoopaJunk.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &KoopaJunk);
	this->aPhysics.addToList();


	spriteSomeRectX = currentInfo->size;
	spriteSomeRectY = currentInfo->size;
	_320 = 0.0f;
	_324 = currentInfo->size;

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
		// this->rot.z = 0x2000;
	}
	else if (this->direction == 1) { // Ground Facing Right
		this->pos.x += 0.0; // +32 to -32
		this->pos.y += 36.0;
		// this->rot.z = 0xE000;
	}
	if (this->front == 1) { this->pos.z = -1804.0; }
	else 				  { this->pos.z =  3300.0; }


	if (currentInfo->launchSound != 0) {
		PlaySound(this, currentInfo->launchSound);
	}

	if (Type == 3) {
		PlaySoundWithFunctionB4(SoundRelatedClass, &hammerSound, SE_EMY_MEGA_BROS_HAMMER, 1);
	}

	doStateChange(&StateID_Straight);

	this->onExecute();
	return true;
}


int daKoopaThrow::onDelete() {
	if (hammerSound.Exists())
		hammerSound.Stop(10);
	return true;
}

int daKoopaThrow::onDraw() {
	bodyModel.scheduleForDrawing();
	return true;
}


void daKoopaThrow::updateModelMatrices() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


int daKoopaThrow::onExecute() {
	acState.execute();
	updateModelMatrices();
	if (playsAnim) {
		if (chrAnim.isAnimationDone())
			chrAnim.setCurrentFrame(0.0f);
		bodyModel._vf1C();
	}

	float rect[] = {this->_320, this->_324, this->spriteSomeRectX, this->spriteSomeRectY};
	int ret = this->outOfZone(this->pos, (float*)&rect, this->currentZoneID);
	if(ret) {
		this->Delete(1);
	}

	return true;
}



void daKoopaThrow::beginState_Straight() {
	float rand = (float)GenerateRandomNumber(10) * 0.4;

	if (this->direction == 0) { // directions 1 spins clockwise, fly rightwards
		speed.x = 1.5 + rand;
	}
	else {						// directions 0 spins anti-clockwise, fly leftwards
		speed.x = -1.5 - rand;
	}

	speed.y = 9.0;
}
void daKoopaThrow::executeState_Straight() {

	speed.y = speed.y - 0.01875;

	HandleXSpeed();
	HandleYSpeed();
	doSpriteMovement();

	// cmgr_returnValue = collMgr.isOnTopOfTile();
	// collMgr.calculateBelowCollisionWithSmokeEffect();

	// if (collMgr.isOnTopOfTile() || (collMgr.outputMaybe & (0x15 << direction))) {
	// 	// hit the ground or wall
	// 	PlaySoundAsync(this, currentInfo->breakSound);

	// 	SpawnEffect(currentInfo->deathEffect, 0, &this->pos, &(S16Vec){0,0,0}, &(Vec){0.75, 0.75, 0.75});
	// 	this->Delete(1);
	// }

	if (this->direction == 1) { // directions 1 spins clockwise, fly rightwards
		this->rot.x -= currentInfo->xrot;
		this->rot.y -= currentInfo->yrot;
		this->rot.z -= currentInfo->zrot; }
	else {						// directions 0 spins anti-clockwise, fly leftwards
		this->rot.x -= currentInfo->xrot;
		this->rot.y -= currentInfo->yrot;
		this->rot.z += currentInfo->zrot; }


	if (Type < 2) {
		PlayWrenchSound(this);
	}
	else if (currentInfo->flySound == 0) { return; }
	else {
		PlaySound(this, currentInfo->flySound);
	}

}
void daKoopaThrow::endState_Straight() { }









