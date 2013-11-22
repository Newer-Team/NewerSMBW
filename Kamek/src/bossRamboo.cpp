#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>
#include "boss.h"
#include "player.h"

class daRamboo_c : public daBoss {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;
	m3d::mdl_c hideModel;
	m3d::mdl_c fogModel;

	nw4r::g3d::ResFile resFile;
	m3d::anmChr_c anmFog;
	m3d::anmChr_c anmA;
	m3d::anmChr_c anmB;

	nw4r::g3d::ResAnmTexSrt resTexSrt;
	m3d::anmTexSrt_c fogSrt;

	int timer;
	int ytimer;
	char Hiding;
	char dying;
	float Baseline;
	bool fleeFast;

	u64 eventFlag;

	static daRamboo_c *build();

	void bindAnimChr_and_setUpdateRates(const char* name, m3d::anmChr_c &animationChr, m3d::mdl_c &model, float rate);
	void setupModels();
	void updateModelMatrices();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);


	USING_STATES(daRamboo_c);
	DECLARE_STATE(Grow);
	DECLARE_STATE(Advance);
	DECLARE_STATE(Wait);
	DECLARE_STATE(Flee);
	DECLARE_STATE(Outro);
};

daRamboo_c *daRamboo_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daRamboo_c));
	return new(buffer) daRamboo_c;
}


CREATE_STATE(daRamboo_c, Grow);
CREATE_STATE(daRamboo_c, Advance);
CREATE_STATE(daRamboo_c, Wait);
CREATE_STATE(daRamboo_c, Flee);
CREATE_STATE(daRamboo_c, Outro);

extern "C" void *EN_LandbarrelPlayerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther);

void daRamboo_c::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	EN_LandbarrelPlayerCollision(this, apThis, apOther);
	DamagePlayer(this, apThis, apOther);

	fleeFast = false;
	doStateChange(&StateID_Flee);
}
bool daRamboo_c::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
	S16Vec nullRot = {0,0,0};
	Vec efScale = {0.5f, 0.5f, 0.5f};
	SpawnEffect("Wm_en_obakedoor_sm", 0, &apOther->owner->pos, &nullRot, &efScale);
	return true;
}
bool daRamboo_c::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
}
bool daRamboo_c::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {

	if (apOther->owner->name == 412) { // Check if it's a glow block
		dEn_c *blah = (dEn_c*)apOther->owner;
		if (blah->_12C & 3 || strcmp(blah->acState.getCurrentState()->getName(), "daLightBlock_c::StateID_Throw")) {
			return true;
		}

		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_en_obakedoor_sm", 0, &apOther->owner->pos, &nullRot, &oneVec);
		SpawnEffect("Wm_mr_yoshistep", 0, &apOther->owner->pos, &nullRot, &oneVec);

		fleeFast = true;
		doStateChange(&StateID_Flee);
		//FIXME changed to dStageActor_c::Delete(u8) from fBase_c::Delete(void)
		apOther->owner->Delete(1);
		return true;
	}
	return false;
}
bool daRamboo_c::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
	return collisionCat1_Fireball_E_Explosion(apThis, apOther);
}
bool daRamboo_c::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
	DamagePlayer(this, apThis, apOther);
	return true;
}
bool daRamboo_c::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther) {
	DamagePlayer(this, apThis, apOther);
	return true;
}




void daRamboo_c::bindAnimChr_and_setUpdateRates(const char* name, m3d::anmChr_c &animationChr, m3d::mdl_c &model, float rate) {
	nw4r::g3d::ResAnmChr anmChr = resFile.GetResAnmChr(name);
	animationChr.bind(&model, anmChr, 1);
	model.bindAnim(&animationChr, 0.0);
	animationChr.setUpdateRate(rate);
}


void daRamboo_c::setupModels() {
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->resFile.data = getResource("teresa", "g3d/teresa.brres");
	bool ret;

	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("fog");
	this->fogModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&this->fogModel, 0);

	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("fog");
	ret = this->anmFog.setup(mdl, anmChr, &this->allocator, 0);


	nw4r::g3d::ResMdl mdlB = this->resFile.GetResMdl("teresaA");
	this->bodyModel.setup(mdlB, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&this->bodyModel, 0);

	nw4r::g3d::ResAnmChr anmChrC = this->resFile.GetResAnmChr("shay_teresaA");
	ret = this->anmA.setup(mdlB, anmChrC, &this->allocator, 0);


	nw4r::g3d::ResMdl mdlC = this->resFile.GetResMdl("teresaB");
	this->hideModel.setup(mdlC, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&this->hideModel, 0);

	nw4r::g3d::ResAnmChr anmChrE = this->resFile.GetResAnmChr("shay_teresaB");
	ret = this->anmB.setup(mdlC, anmChrE, &this->allocator, 0);

	nw4r::g3d::ResAnmTexSrt anmSrt = this->resFile.GetResAnmTexSrt("fog");
	this->resTexSrt = anmSrt;
	//setup(ResMdl mdl, ResAnmTexSrt anmSrt, mAllocator* allocator, void* NULL, int count);
	ret = this->fogSrt.setup(mdl, anmSrt, &this->allocator, 0, 1);
	//setEntryByte34(char toSet, int which);
	this->fogSrt.setEntryByte34(0, 0);

	allocator.unlink();
}



int daRamboo_c::onCreate() {

	setupModels();


	this->scale = (Vec){2.0, 2.0, 2.0};

	this->aPhysics.collisionCheckType = 1;

	ActivePhysics::Info HitMeBaby;
	HitMeBaby.xDistToCenter = 160.0;
	HitMeBaby.yDistToCenter = 80.0;

	HitMeBaby.xDistToEdge = 132.0;
	HitMeBaby.yDistToEdge = 132.0;

	HitMeBaby.category1 = 0x3;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x4F;
	HitMeBaby.bitfield2 = 0x80222;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &dEn_c::collisionCallback;


	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();

	this->Baseline = this->pos.y;
	this->rot.x = 0; // X is vertical axis
	this->rot.y = 0xE000; // Y is horizontal axis
	this->rot.z = 0; // Z is ... an axis >.>
	this->direction = 0; // Heading left.
	this->Hiding = 1;
	this->dying = 0;

	this->speed.x = 0.0;
	this->ytimer = 0;
	this->pos.z = 3300.0;

	char eventNum	= (this->settings >> 16) & 0xFF;

	this->eventFlag = (u64)1 << (eventNum - 1);

	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("fog");
	this->anmFog.bind(&this->fogModel, anmChr, 1);
	this->fogModel.bindAnim(&this->anmFog, 0.0);
	this->anmFog.setUpdateRate(1.0);

	nw4r::g3d::ResAnmTexSrt anmSrt = this->resFile.GetResAnmTexSrt("fog");
	//bindEntry(mdl_c* model, ResAnmTexSrt anmSrt, int which, int playState?);
	this->fogSrt.bindEntry(&this->fogModel, anmSrt, 0, 1);
	this->fogModel.bindAnim(&this->fogSrt, 1.0);
	//setFrameForEntry(float frame, int which);
	this->fogSrt.setFrameForEntry(1.0, 0);
	//setUpdateRateForEntry(float rate, int which);
	this->fogSrt.setUpdateRateForEntry(1.0, 0);

	doStateChange(&StateID_Grow);

	this->onExecute();
	return true;
}

int daRamboo_c::onDelete() {
	return true;
}

int daRamboo_c::onExecute() {
	acState.execute();
	updateModelMatrices();

	this->fogModel._vf1C();

	if(this->anmFog.isAnimationDone())
		this->anmFog.setCurrentFrame(0.0);

	this->fogSrt.process();
	if(this->fogSrt.isEntryAnimationDone(0))
		this->fogSrt.setFrameForEntry(1.0, 0);

	if (dFlagMgr_c::instance->flags & this->eventFlag) {
		dFlagMgr_c::instance->flags = dFlagMgr_c::instance->flags && this->eventFlag;
		doStateChange(&StateID_Outro);
	}

	return true;
}

int daRamboo_c::onDraw() {
	fogModel.scheduleForDrawing();

	if (this->Hiding == 0) {
		bodyModel.scheduleForDrawing(); }
	else {
		hideModel.scheduleForDrawing(); }
	return true;
}


void daRamboo_c::updateModelMatrices() {
	// This won't work with wrap because I'm lazy.
	matrix.translation(pos.x + 160.0, pos.y + (scale.x * 12.0) - 80.0, pos.z);

	fogModel.setDrawMatrix(matrix);
	fogModel.setScale(&scale);
	fogModel.calcWorld(false);

	matrix.translation(pos.x + 160.0, pos.y - 80.0, pos.z + 200.0);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	if (this->Hiding == 0) {
		bodyModel.setDrawMatrix(matrix);
		bodyModel.setScale(&scale);
		bodyModel.calcWorld(false); }
	else {
		hideModel.setDrawMatrix(matrix);
		hideModel.setScale(&scale);
		hideModel.calcWorld(false); }
}


// Grow State

void daRamboo_c::beginState_Grow() {
	this->timer = 0;
	bindAnimChr_and_setUpdateRates("begin_boss_b", anmB, hideModel, 1.0f);
	PlaySound(this, SE_EMY_TERESA);

	SetupKameck(this, Kameck);
}

void daRamboo_c::executeState_Grow() {
	this->timer += 1;

	bool ret;
	ret = GrowBoss(this, Kameck, 1.0, 15.0, 0, this->timer);

	if (timer == 450) {
		Hiding = false;
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_EMY_CS_TERESA_BRING_IT, 1);
		bindAnimChr_and_setUpdateRates("begin_boss", anmA, bodyModel, 1.0f);
	}

	if (Hiding)
		hideModel._vf1C();
	else
		bodyModel._vf1C();
	if (ret && anmA.isAnimationDone()) {
		Hiding = 0;
		doStateChange(&StateID_Advance);
	}
}
void daRamboo_c::endState_Grow() {
	this->Baseline = this->pos.y;

	CleanupKameck(this, Kameck);
}



float RightmostPlayerPos() {
	dStageActor_c* current;
	//The fuck you doing here? --Treeki
	//current is set to nothing, you can't store a thing there...
	//current->pos.x = 0.0;

	for(char ii = 0; ii < 4; ii++) {
		dStageActor_c* player = GetSpecificPlayerActor(ii);
		if(!player) {
			continue;
		}
				// actor->pos.x, actor->pos.y, actor->pos.z,
				// player->pos.x, player->pos.y, player->pos.z);
		if(player->pos.x > current->pos.x) {
			current = player;
		}
	}
	return current->pos.x;
}


// Advance State

void daRamboo_c::beginState_Advance() {
	this->speed.y = 0;
	this->speed.z = 0;
	this->timer = 0;

	bindAnimChr_and_setUpdateRates("wait", anmA, bodyModel, 1.0f);

}
void daRamboo_c::executeState_Advance() {

	this->bodyModel._vf1C();

	if (this->anmA.isAnimationDone()) {
		this->anmA.setCurrentFrame(0.0); }

	float px = RightmostPlayerPos();

	if ((px - 132.0) < this->pos.x) {
		this->pos.x -= this->timer / 28.0; }
	else {
		this->pos.x = (px - 132.0); }

	this->pos.y = this->Baseline + sin(this->ytimer * 3.14 / 192) * 36;


	if (this->timer >= 32) { this->timer = 31; }
	if (this->ytimer >= 384) { this->ytimer = 0; }

	PlaySound(this, SE_EMY_TERESA);

	this->timer += 1;
	this->ytimer += 1;
}

void daRamboo_c::endState_Advance() {  }





// Wait State

void daRamboo_c::beginState_Wait() {

	this->timer = 0;
	PlaySound(this, SE_EMY_TERESA_STOP);

}
void daRamboo_c::executeState_Wait() {

	if (this->timer < 90) {
		this->hideModel._vf1C(); }

	if (this->timer > 55) {
		this->bodyModel._vf1C(); }


	if (this->timer == 70)  {
		this->Hiding = 0;

		bindAnimChr_and_setUpdateRates("shay_teresaA", anmA, bodyModel, 1.0f);
	}


	if (this->timer == 55)  {
		bindAnimChr_and_setUpdateRates("shay_teresaB", anmB, hideModel, 1.0f);
	}



	if (this->anmB.isAnimationDone()) {
		PlaySound(this, SE_EMY_CS_TERESA_BEAT_YOU);
		doStateChange(&StateID_Advance);
	}

	this->timer += 1;

}

void daRamboo_c::endState_Wait() {

	this->Hiding = 0;

	bindAnimChr_and_setUpdateRates("shay_teresaA", anmA, bodyModel, 1.0f);
}





// Flee State

void daRamboo_c::beginState_Flee() {

	this->timer = 0;

}
void daRamboo_c::executeState_Flee() {

	this->hideModel._vf1C();

	if (this->timer == 10) {
		this->Hiding = 1;

		bindAnimChr_and_setUpdateRates("shay_teresaB_wait", anmB, hideModel, 1.0f);
	}

	this->pos.x += (60 - this->timer) / (fleeFast ? 8 : 25);


	if ((this->timer > 60) && (this->anmB.isAnimationDone())) {
		doStateChange(&StateID_Wait);
	}

	this->timer += 1;
}

void daRamboo_c::endState_Flee() {
}




void daRamboo_c::beginState_Outro() {

	bindAnimChr_and_setUpdateRates("DEATH", anmB, hideModel, 1.0f);
	Hiding = true;

	this->timer = 0;
	this->rot.x = 0x0; // X is vertical axis
	this->rot.y = 0xE000; // Y is horizontal axis
	this->rot.z = 0x0; // Z is ... an axis >.>

	OutroSetup(this);
}
void daRamboo_c::executeState_Outro() {
	hideModel._vf1C();
	if (this->anmB.isAnimationDone())
		this->anmB.setCurrentFrame(0.0);

	if (this->dying == 1) {
		if (this->timer > 180) {
			ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE);
		}

		if (this->timer == 60) {
			PlayerVictoryCries(this);
		}

		this->timer = timer + 1;
		return;
	}

	bool ret;
	Vec tempPos = (Vec){this->pos.x + 160.0f, this->pos.y + 80.0f, 5500.0f};
	ret = ShrinkBoss(this, &tempPos, 0.0, this->timer);

	if (timer >= 118) {
		BossExplode(this, &tempPos);
		this->dying = 1;
		this->timer = 0;
	}
	else if (timer == 20) {
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_EMY_BIG_TERESA_DEAD, 1);
	}

	timer = timer + 1;
}
void daRamboo_c::endState_Outro() { }


