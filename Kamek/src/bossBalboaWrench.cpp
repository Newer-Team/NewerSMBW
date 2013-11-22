#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>
#include "boss.h"
extern "C" void dAcPy_vf3F8(void* player, dEn_c* monster, int t);

class daBalboa_c : public daBoss {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;
	m3d::mdl_c spikesModel;

	nw4r::g3d::ResFile resFile;
	m3d::anmChr_c animationChr;

	int timer;
	int damage;
	float Baseline;
	float dying;
	Vec PopUp [3];
	char throwCount;
	char throwMax;
	float throwRate;
	char upsideDown;
	int isBigBoss;
	char isRevenging;
	int spinner;

	ActivePhysics spikeCollision;
	float spikeOffset;
	bool spikeGoingUp, spikeGoingDown;

	static daBalboa_c *build();

	void setupModels();
	void updateModelMatrices();
	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);

	bool prePlayerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther);

	void addScoreWhenHit(void *other);

	USING_STATES(daBalboa_c);
	DECLARE_STATE(Grow);
	DECLARE_STATE(ManholeUp);
	DECLARE_STATE(HeadPoke);
	DECLARE_STATE(AllOut);
	DECLARE_STATE(ThrowWrench);
	DECLARE_STATE(BackDown);
	DECLARE_STATE(Outro);
	DECLARE_STATE(Damage);
	DECLARE_STATE(RevengeUp);
	DECLARE_STATE(Revenge);
};

daBalboa_c *daBalboa_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daBalboa_c));
	return new(buffer) daBalboa_c;
}

// Externs

	CREATE_STATE(daBalboa_c, Grow);
	CREATE_STATE(daBalboa_c, ManholeUp);
	CREATE_STATE(daBalboa_c, HeadPoke);
	CREATE_STATE(daBalboa_c, AllOut);
	CREATE_STATE(daBalboa_c, ThrowWrench);
	CREATE_STATE(daBalboa_c, BackDown);
	CREATE_STATE(daBalboa_c, Outro);
	CREATE_STATE(daBalboa_c, Damage);
	CREATE_STATE(daBalboa_c, RevengeUp);
	CREATE_STATE(daBalboa_c, Revenge);

// Collisions
	void balbieCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther);

	void balbieCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (apOther->owner->name != 544) {
			dEn_c::collisionCallback(apThis, apOther);
		}
	}

	void daBalboa_c::addScoreWhenHit(void *other) {}

bool daBalboa_c::prePlayerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (apOther->owner->stageActorType == 1) {
		if (apOther->info.category2 == 7) {
			if (collisionCat7_GroundPound(apThis, apOther))
				return true;
		}
	}

	return dEn_c::prePlayerCollision(apThis, apOther);
}

	void daBalboa_c::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {

		char ret = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 0);

		if(ret == 0) {
			this->dEn_c::playerCollision(apThis, apOther);
			this->_vf220(apOther->owner);
		}

		//FIXME hack to make multiple playerCollisions work
		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
		this->counter_504[apOther->owner->which_player] = 0;
	}

	bool daBalboa_c::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {

		dActor_c *block = apOther->owner;
		dEn_c *mario = (dEn_c*)block;

		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_en_vshit", 0, &mario->pos, &nullRot, &oneVec);

		mario->speed.y = -mario->speed.y;
		mario->pos.y += mario->speed.y;

		if (mario->direction == 0) { mario->speed.x = 4.0; }
		else					  { mario->speed.x = -4.0; }

		mario->doSpriteMovement();
		mario->doSpriteMovement();

		if (isRevenging) {
			_vf220(apOther->owner);
		} else {
			this->damage -= 1;

			apOther->someFlagByte |= 2;

			PlaySoundAsync(this, SE_EMY_PENGUIN_DAMAGE);

			doStateChange(&StateID_Damage);
		}

		return true;
	}
	bool daBalboa_c::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) {
		return collisionCat7_GroundPound(apThis, apOther);
	}

	bool daBalboa_c::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
		return true;
	}
	bool daBalboa_c::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
		return false;
	}
	bool daBalboa_c::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
		return true;
	}
	bool daBalboa_c::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
		return true;
	}
	bool daBalboa_c::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther) {
		dAcPy_vf3F8(apOther->owner, this, 3);
		return true;
	}


void daBalboa_c::setupModels() {
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	nw4r::g3d::ResMdl mdl;
	nw4r::g3d::ResAnmChr anmChr;

	this->resFile.data = getResource("choropoo", "g3d/choropoo.brres");

	mdl = this->resFile.GetResMdl("choropoo");
	this->bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&this->bodyModel, 0);

	anmChr = this->resFile.GetResAnmChr("throw_1"); // 11
	this->animationChr.setup(mdl, anmChr, &this->allocator, 0);

	nw4r::g3d::ResFile togeRes;
	togeRes.data = getResource("lift_zen", "g3d/lift_zen.brres");
	mdl = togeRes.GetResMdl("lift_togeU");
	spikesModel.setup(mdl, &allocator, 0, 1, 0);

	// throw_1 // 11
	// throw_2 // 75
	// throw_3 // 33
	// throw_4_left_hand // 87
	// throw_4_right_hand // 87
	// throw_5 // 23

	allocator.unlink();
}

// Animation Order...
// AppearLittle - Throw One, sound 0x21F
// Search - Throw two
// AppearFull - Throw 3 and sound 0x220
// Attack - Throw 4
// Disappear - Throw 5


void daBalboa_c::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->animationChr.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->animationChr, unk2);
	this->animationChr.setUpdateRate(rate);
}

int daBalboa_c::onCreate() {

	setupModels();

	this->scale = (Vec){1.0, 1.0, 1.0};
	this->isBigBoss = (this->settings >> 28);

	ActivePhysics::Info HitMeBaby;
	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = 27.0;

	HitMeBaby.xDistToEdge = 18.0;
	HitMeBaby.yDistToEdge = 24.0;

	HitMeBaby.category1 = 0x3;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x4F;
	HitMeBaby.bitfield2 = 0xFFBAFFFE;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &balbieCollisionCallback;


	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();

	ActivePhysics::Info spikeInfo = {
		0.0f, 0.0f, 16.0f, 16.0f,
		3, 0, 0x4F, 0xFFBAFFFE, 0, &dEn_c::collisionCallback};
	spikeCollision.initWithStruct(this, &spikeInfo);
	spikeCollision.trpValue0 = 0.0f;
	spikeCollision.trpValue1 = 0.0f;
	spikeCollision.trpValue2 = -16.0f;
	spikeCollision.trpValue3 = 16.0f;
	spikeCollision.collisionCheckType = 3;

	this->rot.x = 0; // X is vertical axis
	this->rot.y = 0xE000; // Y is horizontal axis
	this->rot.z = 0; // Z is ... an axis >.>
	this->upsideDown = 0;
	this->direction = 0; // Heading left.
	this->pos.z = -800.0;
	this->pos.y -= 8.0;
	this->damage = 3;
	this->isRevenging = 0;

	this->PopUp[0] = (Vec){this->pos.x, this->pos.y - 54.0f, this->pos.z};
	this->PopUp[1] = (Vec){this->pos.x - 224.0f, this->pos.y - 54.0f, this->pos.z};
	this->PopUp[2] = (Vec){this->pos.x - 112.0f, this->pos.y - 22.0f, this->pos.z};
	this->PopUp[3] = (Vec){this->pos.x - 112.0f, this->pos.y - 22.0f, this->pos.z};


	doStateChange(&StateID_Grow);

	this->onExecute();
	return true;
}

int daBalboa_c::onDelete() {
	return true;
}

int daBalboa_c::onExecute() {
	acState.execute();
	updateModelMatrices();

	if (spikeGoingUp) {
		spikeOffset += 2.5f;
		if (spikeOffset >= 48.0f) {
			spikeOffset = 48.0f;
			spikeGoingUp = false;
		}
	} else if (spikeGoingDown) {
		spikeOffset -= 2.5f;
		if (spikeOffset <= 0.0f) {
			spikeOffset = 0.0f;
			spikeGoingDown = false;
		}
	}
	spikeCollision.info.yDistToCenter = 16.0f + spikeOffset;

	bodyModel._vf1C();

	return true;
}

int daBalboa_c::onDraw() {

	bodyModel.scheduleForDrawing();
	if (spikeOffset > 0.0f)
		spikesModel.scheduleForDrawing();

	return true;
}

void daBalboa_c::updateModelMatrices() {
	// This won't work with wrap because I'm lazy.
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);

	mMtx spikeMatrix;
	VEC3 spikeScale = {2.0f,1.8f,2.0f};
	spikeMatrix.translation(pos.x, pos.y + spikeOffset, pos.z);
	spikesModel.setDrawMatrix(spikeMatrix);
	spikesModel.setScale(&spikeScale);
	spikesModel.calcWorld(false);
}

// Grow State

	void daBalboa_c::beginState_Grow() {
		this->timer = 0;

		SetupKameck(this, Kameck);

		bindAnimChr_and_setUpdateRate("begin_boss", 1, 0.0, 0.6);
	}

	void daBalboa_c::executeState_Grow() {

		if(this->animationChr.isAnimationDone())
			this->animationChr.setCurrentFrame(0.0);

		this->timer += 1;

		bool ret;
		ret = GrowBoss(this, Kameck, 1.0, 2.25, 0, this->timer);

		if (ret) {
			PlaySound(this, SE_EMY_CHOROPU_BOUND);
			doStateChange(&StateID_BackDown);
		}
	}
	void daBalboa_c::endState_Grow() {
		CleanupKameck(this, Kameck);
	}


// ManholeUp State

	void daBalboa_c::beginState_ManholeUp() {

		bindAnimChr_and_setUpdateRate("throw_1", 1, 0.0, 1.0);

		this->timer = 0;

		int randChoice;
		randChoice = GenerateRandomNumber(3);

		this->pos = this->PopUp[randChoice];



		if 		(randChoice == 0) { // On the left side!
			this->rot.y = 0xE000;
			this->rot.z = 0;
			this->upsideDown = 0;
			this->direction = 0; }

		else if (randChoice == 1) {	// On the right side!
			this->rot.y = 0x2000;
			this->rot.z = 0;
			this->upsideDown = 0;
			this->direction = 1; }

		// else if (randChoice == 2) {	// On the right ceiling!
		// 	this->rot.y = 0xE000;
		// 	this->rot.z = 0x8000;
		// 	this->upsideDown = 1;
		// 	this->direction = 0; }

		// else if (randChoice == 3) {	// On the left ceiling!
		// 	this->rot.y = 0x2000;
		// 	this->rot.z = 0x8000;
		// 	this->upsideDown = 1;
		// 	this->direction = 1; }

		else if (randChoice == 2) {	// In the Center!
			char Pdir = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);

			if (Pdir == 1) {
				this->rot.y = 0xE000;
				this->direction = 0; }
			else {
				this->rot.y = 0x2000;
				this->direction = 1; }
		}

		PlaySound(this, 0x21F);
	}

	void daBalboa_c::executeState_ManholeUp() {

		if (this->timer > 51) {
			doStateChange(&StateID_HeadPoke);
		}
		if (this->timer > 11) { }
		else {
			this->pos.y += 0.8182; // Height is 54 pixels, move up 9 pixels.
		}

		this->timer += 1;
	}
	void daBalboa_c::endState_ManholeUp() { }


// HeadPoke State

	void daBalboa_c::beginState_HeadPoke() {

		bindAnimChr_and_setUpdateRate("throw_2", 1, 0.0, 1.0);

		this->timer = 0;
	}

	void daBalboa_c::executeState_HeadPoke() {

		this->pos.y += 0.24; // Height is 54 pixels, move up 18 pixels.

		if(this->animationChr.isAnimationDone()) {
			doStateChange(&StateID_AllOut); }

	}
	void daBalboa_c::endState_HeadPoke() { }


// AllOut State

	void daBalboa_c::beginState_AllOut() {

		bindAnimChr_and_setUpdateRate("throw_3", 1, 0.0, 1.0);

		this->timer = 0;

		PlaySound(this, 0x220);
	}

	void daBalboa_c::executeState_AllOut() {


		this->pos.y += 0.8182; // Height is 54 pixels, move up 27 pixels.

		if(this->animationChr.isAnimationDone()) {
			doStateChange(&StateID_ThrowWrench);
		}
	}
	void daBalboa_c::endState_AllOut() { }


// ThrowWrench State

	void daBalboa_c::beginState_ThrowWrench() {

		this->throwCount = 0;
		if (this->isBigBoss == 1) {
			throwMax = 6;
			throwRate = 3.0;
		}
		else {
			throwMax = 4;
			throwRate = 2.0;
		}
		bindAnimChr_and_setUpdateRate("throw_4_right_hand", 1, 0.0, throwRate);
	}

	void daBalboa_c::executeState_ThrowWrench() {

		float frame = this->animationChr.getCurrentFrame();
		if (frame == 54.0) {
			u32 settings;
			u8 up = this->upsideDown;
			u8 throwc = this->throwCount;
			u8 dir;

			if (this->direction) { dir = 0; }
			else 				 { dir = 1; }

			settings = (dir) | (up << 1);
			settings = settings | (throwc & 1 << 8);

			if (this->isBigBoss == 1) { settings = settings | 0x10; }

			CreateActor(544, settings, this->pos, 0, 0);
		}

		if(this->animationChr.isAnimationDone()) {
			this->throwCount += 1;

			if (this->throwCount & 1) {
				bindAnimChr_and_setUpdateRate("throw_4_left_hand", 1, 0.0, throwRate);
			}
			else {
				bindAnimChr_and_setUpdateRate("throw_4_right_hand", 1, 0.0, throwRate);
			}
		}

		if (this->throwCount > throwMax) {
			doStateChange(&StateID_BackDown); }
	}
	void daBalboa_c::endState_ThrowWrench() { }


// BackDown State

	void daBalboa_c::beginState_BackDown() {

		bindAnimChr_and_setUpdateRate("throw_5", 1, 0.0, 1.0);

		this->timer = 0;

		PlaySound(this, 0x221);

		S16Vec nullRot = {0,0,0};
		Vec efScale = {2.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_mr_sanddive_out", 0, &this->pos, &nullRot, &efScale);
		SpawnEffect("Wm_mr_sanddive_smk", 0, &this->pos, &nullRot, &efScale);
	}

	void daBalboa_c::executeState_BackDown() {

		if (this->timer < 60) {
			this->pos.y -= 2.0;  // Height is 54 pixels, move down
		}

		if (this->timer > 90) {
			doStateChange(&StateID_ManholeUp); }

		this->timer += 1;

	}
	void daBalboa_c::endState_BackDown() { }


// Outro

	void daBalboa_c::beginState_Outro() {

		bindAnimChr_and_setUpdateRate("dead", 1, 0.0, 1.0);

		OutroSetup(this);
		this->timer = 0;
		this->rot.x = 0x0; // X is vertical axis
		this->rot.z = 0x0; // Z is ... an axis >.>

	}
	void daBalboa_c::executeState_Outro() {

		if(this->animationChr.isAnimationDone())
			this->animationChr.setCurrentFrame(0.0);

		if (this->dying == 1) {
			if (this->timer > 180) { ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE); }
			if (this->timer == 60) { PlayerVictoryCries(this); }

			this->timer += 1;
			return;
		}

		bool ret;
		ret = ShrinkBoss(this, &this->pos, 2.25, this->timer);
		this->pos.y -= 0.02;

		if (ret == true) 	{
			BossExplode(this, &this->pos);
			this->dying = 1;
			this->timer = 0;
		}
		else 		{ PlaySound(this, SE_EMY_CHOROPU_SIGN); }

		this->timer += 1;

	}
	void daBalboa_c::endState_Outro() { }


// Damage

	void daBalboa_c::beginState_Damage() {

		bindAnimChr_and_setUpdateRate("dead", 1, 0.0, 0.5);

		this->timer = 0;
		this->removeMyActivePhysics();
	}
	void daBalboa_c::executeState_Damage() {

		if (this->timer > 6) { doStateChange(&StateID_RevengeUp); }

		if(this->animationChr.isAnimationDone()) {
			this->animationChr.setCurrentFrame(0.0);

			this->timer += 1;
		}

		if (this->timer > 3) {
			this->pos.y -= 5.0; // Height is 54 pixels, move down
		}
		else if (this->timer > 2) {
			if (this->damage == 0) {
				StopBGMMusic();
				doStateChange(&StateID_Outro);
			}
			this->pos.y -= 3.5; // Height is 54 pixels, move down
		}
		else if (this->timer > 1) {
			this->pos.y -= 1.0; // Height is 54 pixels, move down
		}
		else if (this->timer > 0) {
			this->pos.y += 1.0; // Height is 54 pixels, move down
		}
		else {
			this->pos.y += 3.5; // Height is 54 pixels, move down
		}

	}
	void daBalboa_c::endState_Damage() {
		this->addMyActivePhysics();
	}


// Revenge Up

	void daBalboa_c::beginState_RevengeUp() {

		this->pos = this->PopUp[2];
		this->rot.y = 0;

		isRevenging = 1;
		bindAnimChr_and_setUpdateRate("throw_3", 1, 0.0, 1.0);
		spikeGoingUp = true;
		spikeCollision.addToList();

		PlaySound(this, 0x220);
	}
	void daBalboa_c::executeState_RevengeUp() {

		this->pos.y += 1.6363; // Height is 54 pixels, move up 27 pixels.

		if(this->animationChr.isAnimationDone()) {
			doStateChange(&StateID_Revenge);
		}

	}
	void daBalboa_c::endState_RevengeUp() { }


// Revenge

	void daBalboa_c::beginState_Revenge() {
		spinner = 0;

		this->throwCount = 0;
		if (this->isBigBoss == 1) {
			throwMax = 16;
			throwRate = 5.0;
		}
		else {
			throwMax = 12;
			throwRate = 3.0;
		}
		bindAnimChr_and_setUpdateRate("throw_4_right_hand", 1, 0.0, throwRate);
	}
	void daBalboa_c::executeState_Revenge() {

		float frame = this->animationChr.getCurrentFrame();

		rot.y = 16384.0 * (frame / 87.0) + (16384.0 * spinner);

		if (frame == 60.0) {
			u32 settings;
			u8 up = this->upsideDown;
			u8 throwc = this->throwCount;
			u8 dir;

			if (spinner < 2)	 { dir = 0; }
			else 				 { dir = 1; }

			settings = (dir) | (up << 1);
			settings = settings | (throwc & 1 << 8);

			if (this->isBigBoss == 1) { settings = settings | 0x10; }

			CreateActor(544, settings, this->pos, 0, 0);
		}

		if(this->animationChr.isAnimationDone()) {
			this->throwCount += 1;
			spinner += 1;
			if (spinner == 4) { spinner = 0; }

			if (this->throwCount & 1) {
				bindAnimChr_and_setUpdateRate("throw_4_left_hand", 1, 0.0, throwRate);
			}
			else {
				bindAnimChr_and_setUpdateRate("throw_4_right_hand", 1, 0.0, throwRate);
			}
		}

		if (this->throwCount > throwMax) {
			doStateChange(&StateID_BackDown); }

	}
	void daBalboa_c::endState_Revenge() {
			isRevenging = 0;
			spikeGoingDown = true;
			spikeCollision.removeFromList();
	}
