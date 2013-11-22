#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>


extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);

extern int GlobalStarsCollected;


class dChallengeStar : public dEn_c {
	int onCreate();
	int onExecute();
	int onDelete();
	int onDraw();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;

	mEf::es2 effect;

	u64 eventFlag;
	s32 timer;
	bool collected;

	static dChallengeStar *build();

	void updateModelMatrices();
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);

};


void dChallengeStar::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {

	if (collected) {
		this->Delete(1);
		return; }


	PlaySoundAsync(this, SE_OBJ_BROOM_KEY_SHOW);
	S16Vec nullRot = {0,0,0};
	Vec efScale = {0.8f, 0.8f, 0.8f};
	SpawnEffect("Wm_ob_greencoinkira_a", 0, &this->pos, &nullRot, &efScale);

	GlobalStarsCollected--;
	if (GlobalStarsCollected == 0) {
		dFlagMgr_c::instance->flags |= this->eventFlag;
	}

	collected = true;

	this->Delete(1);
}

void dChallengeStar::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); }
bool dChallengeStar::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->playerCollision(apThis, apOther);
	return true;
}
bool dChallengeStar::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->playerCollision(apThis, apOther);
	return true;
}
bool dChallengeStar::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->playerCollision(apThis, apOther);
	return true;
}
bool dChallengeStar::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->playerCollision(apThis, apOther);
	return true;
}
bool dChallengeStar::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->playerCollision(apThis, apOther);
	return true;
}


dChallengeStar *dChallengeStar::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dChallengeStar));
	return new(buffer) dChallengeStar;
}


int dChallengeStar::onCreate() {

	collected = false;
	char die = this->settings & 0xF;
	if (GetSpecificPlayerActor(die) == 0) { this->Delete(1); return 2; }

	GlobalStarsCollected++;

	allocator.link(-1, GameHeaps[0], 0, 0x20);

	nw4r::g3d::ResFile rf(getResource("I_star", "g3d/silver_star.brres"));
	bodyModel.setup(rf.GetResMdl("I_star"), &allocator, 0x224, 1, 0);
	SetupTextures_Map(&bodyModel, 0);

	allocator.unlink();

	ActivePhysics::Info HitMeBaby;
	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = 3.0;
	HitMeBaby.xDistToEdge = 6.0;
	HitMeBaby.yDistToEdge = 6.0;
	HitMeBaby.category1 = 0x5;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x4F;
	HitMeBaby.bitfield2 = 0x200;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();

	char eventNum	= (this->settings >> 24)	& 0xFF;
	this->eventFlag = (u64)1 << (eventNum - 1);

	this->scale.x = 0.70;
	this->scale.y = 0.70;
	this->scale.z = 0.70;

	this->pos.x += 8.0;
	this->pos.y -= 14.0;
	this->pos.z = 3300.0;

	this->onExecute();
	return true;
}


int dChallengeStar::onDelete() {
	return true;
}

int dChallengeStar::onDraw() {
	bodyModel.scheduleForDrawing();
	return true;
}


void dChallengeStar::updateModelMatrices() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}

int dChallengeStar::onExecute() {
	updateModelMatrices();

	S16Vec nullRot = {0,0,0};
	Vec efScale = {0.8f, 0.8f, 0.8f};
	effect.spawn("Wm_ob_keyget02_kira", 0, &this->pos, &nullRot, &efScale);
	this->rot.y += 0x200;
	return true;
}

