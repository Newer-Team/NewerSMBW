#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include "boss.h"


const char* BDarcNameList [] = {
	"koopa_clown_bomb",
	NULL
};


class dDroppedBomb : public dEn_c {
	int onCreate();
	int onExecute();
	int onDelete();
	int onDraw();

	void kill();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;

	u32 cmgr_returnValue;

	static dDroppedBomb *build();

	void updateModelMatrices();
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	// void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
};

void dDroppedBomb::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	DamagePlayer(this, apThis, apOther);
	this->kill();
}

dDroppedBomb *dDroppedBomb::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dDroppedBomb));
	return new(buffer) dDroppedBomb;
}

// void dDroppedBomb::spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) { this->kill(); }
bool dDroppedBomb::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {return true;}
bool dDroppedBomb::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) { return false; }
bool dDroppedBomb::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {return true;}
bool dDroppedBomb::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {return true;}
bool dDroppedBomb::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther) {return true;}
bool dDroppedBomb::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {return true;}

void dDroppedBomb::kill() {
	PlaySoundAsync(this, SE_BOSS_JR_BOMB_BURST);

	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};
	SpawnEffect("Wm_en_explosion", 0, &this->pos, &nullRot, &oneVec);
	Vec hitScale = {1.25f, 1.25f, 1.25f};
	SpawnEffect("Wm_mr_wirehit", 0, &this->pos, &nullRot, &hitScale);
	this->Delete(1);
}

int dDroppedBomb::onCreate() {

	allocator.link(-1, GameHeaps[0], 0, 0x20);

	nw4r::g3d::ResFile rf(getResource("koopa_clown_bomb", "g3d/koopa_clown_bomb.brres"));
	bodyModel.setup(rf.GetResMdl("koopa_clown_bomb"), &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);

	allocator.unlink();


	ActivePhysics::Info KoopaJunk;

	KoopaJunk.xDistToCenter = -20.0f;
	KoopaJunk.yDistToCenter = 0.0;
	KoopaJunk.xDistToEdge = 20.0f;
	KoopaJunk.yDistToEdge = 20.0f;

	this->scale.x = 1.0;
	this->scale.y = 1.0;
	this->scale.z = 1.0;

	KoopaJunk.category1 = 0x3;
	KoopaJunk.category2 = 0x0;
	KoopaJunk.bitfield1 = 0x4F;
	KoopaJunk.bitfield2 = 0xFFFFFFFF;
	KoopaJunk.unkShort1C = 0;
	KoopaJunk.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &KoopaJunk);
	this->aPhysics.addToList();


	spriteSomeRectX = 20.f;
	spriteSomeRectY = 20.f;
	_320 = 0.0f;
	_324 = 20.f;

	// These structs tell stupid collider what to collide with - these are from koopa troopa
	static const lineSensor_s below(12<<12, 4<<12, 0<<12);
	static const pointSensor_s above(0<<12, 12<<12);
	static const lineSensor_s adjacent(6<<12, 9<<12, 6<<12);

	collMgr.init(this, &below, &above, &adjacent);
	collMgr.calculateBelowCollisionWithSmokeEffect();

	cmgr_returnValue = collMgr.isOnTopOfTile();


	pos.z = 3300.0;
	speed.y = -1.5f;

	PlaySound(this, SE_EMY_ELCJ_THROW);
	return true;
}


int dDroppedBomb::onDelete() {
	return true;
}

int dDroppedBomb::onDraw() {
	bodyModel.scheduleForDrawing();
	return true;
}


void dDroppedBomb::updateModelMatrices() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


int dDroppedBomb::onExecute() {
	// acState.execute();
	updateModelMatrices();

	rot.x += 0x200;
	rot.y += 0x400;
	rot.z += 0x600;

	float rect[] = {this->_320, this->_324, this->spriteSomeRectX, this->spriteSomeRectY};
	int ret = this->outOfZone(this->pos, (float*)&rect, this->currentZoneID);
	if(ret) {
		this->Delete(1);
		dFlagMgr_c::instance->set(settings & 0xFF, 0, true, false, false);
	}

	speed.y = speed.y - 0.01875;

	HandleXSpeed();
	HandleYSpeed();
	doSpriteMovement();

	cmgr_returnValue = collMgr.isOnTopOfTile();
	collMgr.calculateBelowCollisionWithSmokeEffect();

	if (collMgr.isOnTopOfTile() || (collMgr.outputMaybe & (0x15 << direction))) {
		this->kill();
		dFlagMgr_c::instance->set(settings & 0xFF, 0, true, false, false);
	}

	return true;
}





// =========================================================================================================
// ================================================ CONTROLLER =============================================
// =========================================================================================================

class dBombDrop : public dStageActor_c {
	int onCreate();
	int onExecute();
	int onDelete();
	int onDraw();

	int timer;
	dStageActor_c * target;
	int eventA;
	int eventB;

	static dBombDrop *build();
};

dBombDrop *dBombDrop::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dBombDrop));
	return new(buffer) dBombDrop;
}


bool HackyBombDropVariable = false;
extern int BridgeBowserHP;

int dBombDrop::onCreate() {
	BridgeBowserHP = 2;

	int t = this->settings & 0xF;
	this->eventA = ((this->settings >> 24) & 0xFF) - 1;
	this->eventB = ((this->settings >> 16) & 0xFF) - 1;


	if (t == 0) {
		target = (dStageActor_c*)FindActorByType(EN_BOSS_KOOPA, 0);
	}
	else {
		target = GetSpecificPlayerActor(t - 1);
	}

	dFlagMgr_c::instance->set(eventA, 0, false, false, false);
	dFlagMgr_c::instance->set(eventB, 0, false, false, false);

	HackyBombDropVariable = false;

	return true;
}

int dBombDrop::onDelete() { return true; }
int dBombDrop::onDraw() { return true; }

int dBombDrop::onExecute() {
	pos.x = target->pos.x;

	bool active;
	active = dFlagMgr_c::instance->active(eventA);
	if (active) {
		create(WM_SMALLCLOUD, eventA+1, &pos , &rot, 0);
		HackyBombDropVariable = true;
		dFlagMgr_c::instance->set(eventA, 0, false, false, false);
	}

	active = dFlagMgr_c::instance->active(eventB);
	if (active) {
		create(WM_SMALLCLOUD, eventB+1, &pos, &rot, 0);
		HackyBombDropVariable = true;
		dFlagMgr_c::instance->set(eventB, 0, false, false, false);
	}

	return true;
}





