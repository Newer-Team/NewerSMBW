#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>
#include <playerAnim.h>
#include "boss.h"

extern "C" void *StageScreen;

extern u32 GameTimer;

#define time_macro *(u32*)((GameTimer) + 0x4)


// const char* effects_name_list [] = {
// 	"Wm_jr_electricline", // cool
// 	"Wm_jr_fireattack", // cool
// 	"Wm_jr_firehit", // cool
// 	"Wm_jr_fireplace", // cool
// 	"Wm_jr_fireplace_ind", // cool
// 	"Wm_jr_shot",
// 	"Wm_jr_sweat",
// 	"Wm_ko_fireattack", // cool
// 	"Wm_ko_firehit", // cool
// 	"Wm_ko_firehitdie01", // cool
// 	"Wm_ko_firehitdie02", // cool
// 	"Wm_ko_firehitdie03", // cool
// 	"Wm_ko_magmapocha",
// 	"Wm_ko_magmapochabig",
// 	"Wm_ko_shout", // cool
// 	"Wm_ko_shout02", // cool
// 	"Wm_seacloudout", // cool
// };


const char* CBarcNameList [] = {
	"KoopaShip",
	"koopa",
	"choropoo",
	"koopa_clown_bomb",
	"dossun",
	NULL
};


class daCaptainBowser : public daBoss {
public:
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();
	int afterExecute(int param);

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	nw4r::g3d::ResFile shipFile;

	m3d::mdl_c bodyModel;
	m3d::mdl_c shipModel;

	m3d::anmChr_c chrAnimation;
	m3d::anmChr_c shipAnm;

	nw4r::snd::SoundHandle prplSound;

	static const int SHIP_SCOLL_COUNT = 7;
	Physics shipSColls[SHIP_SCOLL_COUNT];
	mEf::es2 effect;

	mEf::es2 shipDmgA;
	mEf::es2 shipDmgB;
	mEf::es2 shipDmgC;
	mEf::es2 shipDmgD;
	mEf::es2 shipDmgE;

	mEf::es2 flamethrowerEffect;
	mEf::es2 flamethrowerEffectInd;

	nw4r::g3d::ResFile flamethrowerRF;
	m3d::mdl_c flamethrowerModel;
	m3d::anmTexSrt_c flamethrowerAnim;
	bool renderFlamethrowerModel;
	ActivePhysics flameCollision;

#define flameOffsetX -160.0f
#define flameOffsetY 150.0f
#define flameZ 1900.0f
	Vec flameScale;

	char isAngry;
	char isInvulnerable;
	char isIntro;
	int maxDamage;
	int playerCount;
	float roarLen;

	float bowserX, bowserY;
	s16 bowserRotX, bowserRotY;
	float bowserXSpeed, bowserYSpeed, bowserMaxYSpeed, bowserYAccel;

	s16 shipRotY;
	float sinTimerX;
	float sinTimerY;
	bool sinTimerXRunning, sinTimerYRunning, stopMoving;

	bool shipAnmFinished;
	float explosionBottomBound;

	bool deathSequenceRunning;
	bool forceClownEnds;
	int timerWhenCrashHappens;
	int timerForVictoryDance;
	void initiateDeathSequence();

	int clownCount;
	dEn_c *clownPointers[4];
	VEC2 clownDests[4];

	int saveTimer;
	bool exitedFlag;

	static daCaptainBowser *build();

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);

	void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);

	void addScoreWhenHit(void *other);

	USING_STATES(daCaptainBowser);
	DECLARE_STATE(Wait);
	DECLARE_STATE(Throw);
	DECLARE_STATE(Fire);

	DECLARE_STATE(Roar);
	DECLARE_STATE(Damage);

	DECLARE_STATE(Intro);
	DECLARE_STATE(FinalAttack);
	DECLARE_STATE(Outro);
	DECLARE_STATE(PanToExit);

};

daCaptainBowser *daCaptainBowser::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daCaptainBowser));
	return new(buffer) daCaptainBowser;
}

///////////////////////
// Externs and States
///////////////////////


	CREATE_STATE(daCaptainBowser, Wait);
	CREATE_STATE(daCaptainBowser, Throw);
	CREATE_STATE(daCaptainBowser, Fire);

	CREATE_STATE(daCaptainBowser, Roar);
	CREATE_STATE(daCaptainBowser, Damage);

	CREATE_STATE(daCaptainBowser, Intro);
	CREATE_STATE(daCaptainBowser, FinalAttack);
	CREATE_STATE(daCaptainBowser, Outro);
	CREATE_STATE(daCaptainBowser, PanToExit);



////////////////////////
// Collision Functions
////////////////////////
void daCaptainBowser::addScoreWhenHit(void *other) { };

bool daCaptainBowser::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
	apOther->owner->kill();
	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};
	SpawnEffect("Wm_en_burst_m", 0, &apOther->owner->pos, &nullRot, &oneVec);
	apThis->someFlagByte |= 2;
	return true;
}

void daCaptainBowser::spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (apOther->owner->name == WM_PAKKUN) { //time to get hurt
		if (this->isInvulnerable) {
			return;
		}
		this->damage -= 1;

		spawnHitEffectAtPosition((Vec2){apOther->owner->pos.x, apOther->owner->pos.y});

		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_en_burst_m", 0, &apOther->owner->pos, &nullRot, &oneVec);
		PlaySound(apOther->owner, SE_BOSS_CMN_STOMPED);
		apOther->owner->Delete(1);

		if (this->damage == this->maxDamage/2) 	{ doStateChange(&StateID_Roar); }
		else if (this->damage < 0)  			{ initiateDeathSequence(); }
		else 									{ doStateChange(&StateID_Damage); }
	}
}

void daCaptainBowser::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	DamagePlayer(this, apThis, apOther);
}



void daCaptainBowser::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->chrAnimation.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->chrAnimation, unk2);
	this->chrAnimation.setUpdateRate(rate);
}

int daCaptainBowser::onCreate() {
	bowserX = -148.0f;
	bowserY = 122.0f;
	bowserRotY = 0xD800;

	shipRotY = 0x4000;
	sinTimerXRunning = true;
	sinTimerYRunning = true;

	// Model creation
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	// B-b-b-bad boy Bowsaa
	this->resFile.data = getResource("koopa", "g3d/koopa.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("koopa");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Boss(&bodyModel, 0);

	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("kp_wait");
	this->chrAnimation.setup(mdl, anmChr, &this->allocator, 0);


	// A ship off the ol' block
	this->shipFile.data = getResource("KoopaShip", "g3d/KoopaShip.brres");
	nw4r::g3d::ResMdl mdlShip = this->shipFile.GetResMdl("KoopaShip");
	shipModel.setup(mdlShip, &allocator, 0x224, 1, 0);
	SetupTextures_MapObj(&shipModel, 0);

	nw4r::g3d::ResAnmChr anmChrShip = this->shipFile.GetResAnmChr("KoopaShip");
	this->shipAnm.setup(mdlShip, anmChrShip, &this->allocator, 0);
	this->shipAnm.bind(&this->shipModel, anmChrShip, 1);
	this->shipModel.bindAnim(&this->shipAnm, 0.0);
	this->shipAnm.setUpdateRate(1.0);

	// Flamethrower
	flamethrowerRF.data = getResource("fire_cannon", "g3d/flame_koopa.brres");

	nw4r::g3d::ResMdl ftResMdl = flamethrowerRF.GetResMdl("fire_effect1x6_right");
	nw4r::g3d::ResAnmTexSrt ftResAnm = flamethrowerRF.GetResAnmTexSrt("fire_effect1x6_right");

	flamethrowerModel.setup(ftResMdl, &allocator, 0x224, 1, 0);
	SetupTextures_MapObj(&flamethrowerModel, 0);

	flamethrowerAnim.setup(ftResMdl, ftResAnm, &allocator, 0, 1);
	flamethrowerAnim.bindEntry(&flamethrowerModel, ftResAnm, 0, 0);
	flamethrowerModel.bindAnim(&flamethrowerAnim, 0.0f);
	flamethrowerAnim.setFrameForEntry(0.0f, 0);
	flamethrowerAnim.setUpdateRate(1.0f);

	allocator.unlink();

	// Prep the goods
	this->playerCount = GetActivePlayerCount();
	this->maxDamage = 24;

	pos.z = 8000.0;
	this->scale = (Vec){0.57, 0.57, 0.57};

	this->damage = this->maxDamage;
	static const float scX1[] = {-176.447600, 64.340078, -157.925471, -157.925471, -158.561530, 49.742932, 48.957043};
	static const float scY1[] = {128.217300, 86.427956, 54.136191, -36.090801, 22.765115, 162.568398, 66.849169};
	static const float scX2[] = {-96.022566, 150.126781, 173.523155, 81.887358, -50.343171, 84.419332, 117.889270};
	static const float scY2[] = {54.136191, 54.136191, -36.090801, -79.779661, -26.166291, 142.846802, -2.083058};
	static s16 sRots[] = {-8192, -8192, -8192};

	for (int i = 0; i < SHIP_SCOLL_COUNT; i++) {
		shipSColls[i].setup(this,
				scX1[i], scY1[i], scX2[i], scY2[i],
				0, 0, 0, 1, 0);
		if (i >= 4)
			shipSColls[i].setPtrToRotation(&sRots[i - 4]);
	}

	// Bowser Physics!
	ActivePhysics::Info BowserPhysics;
	BowserPhysics.xDistToCenter = -152.0;
	BowserPhysics.yDistToCenter = 152.0;

	BowserPhysics.xDistToEdge = 28.0;
	BowserPhysics.yDistToEdge = 26.0;

	BowserPhysics.category1 = 0x3;
	BowserPhysics.category2 = 0x0;
	BowserPhysics.bitfield1 = 0x4F;
	BowserPhysics.bitfield2 = 0x8028E;
	BowserPhysics.unkShort1C = 0;
	BowserPhysics.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &BowserPhysics);



	// State Changers
	this->isIntro = 3;
	doStateChange(&StateID_Intro);

	return true;
}

int daCaptainBowser::onDelete() {
	if (prplSound.Exists())
		prplSound.Stop(0);
	return true;
}

int daCaptainBowser::afterExecute(int param) {
	return dEn_c::afterExecute(param);
}

int daCaptainBowser::onExecute() {
	if (forceClownEnds) {
		dEn_c *clownIter = 0;
		while (clownIter = (dEn_c*)dEn_c::search(JR_CLOWN_FOR_PLAYER, clownIter)) {
			clownIter->counter_500 = 120; // what a terrible hack.
			float *pPropRotationIncrement = (float*)(((u32)clownIter) + 0x740);
			*pPropRotationIncrement = 30.0f;

			SmoothRotation((s16*)(((u32)clownIter) + 0xD4A), 0, 0x300);
		}
	}

	acState.execute();
	if (saveTimer > 0)
		time_macro = saveTimer;

	if (!prplSound.Exists() && acState.getCurrentState() != &StateID_Outro && acState.getCurrentState() != &StateID_PanToExit) {
		PlaySoundWithFunctionB4(SoundRelatedClass, &prplSound, SE_BOSS_SHIP_PRPL, 1);
	}

	aPhysics.info.xDistToCenter = bowserX;
	aPhysics.info.yDistToCenter = bowserY + 28.0f;

	bodyModel._vf1C();
	shipModel._vf1C();

	for (int i = 0; i < SHIP_SCOLL_COUNT; i++)
		shipSColls[i].update();

	float xmod = sinTimerXRunning ? (sin(this->sinTimerX * 3.14 / 180.0) * 60.0) : 0.0f;
	float ymod = sin(this->sinTimerY * 3.14 / 130.0) * (sinTimerYRunning ? 84.0 : 10.0);

	if(this->isIntro == 0) {
		pos.x = ClassWithCameraInfo::instance->screenCentreX + 200.0 + xmod;
		pos.y = ClassWithCameraInfo::instance->screenCentreY - 180.0 + ymod;

		sinTimerX++;
		if (sinTimerX >= 360) {
			sinTimerX = 0;
			if (stopMoving)
				sinTimerXRunning = false;
		}

		sinTimerY++;
		if (sinTimerY >= 260) {
			sinTimerY = 0;
			if (stopMoving)
				sinTimerYRunning = false;
		}
	}

	if(this->shipAnm.isAnimationDone() && acState.getCurrentState() != &StateID_Outro && acState.getCurrentState() != &StateID_PanToExit) {
		this->shipAnm.setCurrentFrame(0.0);
	}

	return true;
}

int daCaptainBowser::onDraw() {

	if(this->isIntro > 2) { return false; }

	matrix.translation(pos.x+bowserX, pos.y+bowserY, pos.z-200.0); // 136.0 is the bottom of the platform footing
	matrix.applyRotationYXZ(&bowserRotX, &bowserRotY, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	Vec oneVec = {1.0f, 1.0f, 1.0f};
	bodyModel.setScale(&oneVec);
	bodyModel.calcWorld(false);

	bodyModel.scheduleForDrawing();


	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &shipRotY, &rot.z);

	shipModel.setDrawMatrix(matrix);
	shipModel.setScale(&scale);
	shipModel.calcWorld(false);

	shipModel.scheduleForDrawing();

	if (renderFlamethrowerModel) {
		matrix.translation(pos.x + flameOffsetX, pos.y + flameOffsetY, flameZ);
		s16 thing = 0x8000;
		matrix.applyRotationZ(&thing);

		flamethrowerModel.setDrawMatrix(matrix);
		flamethrowerModel.setScale(&flameScale);
		flamethrowerModel.calcWorld(false);

		flamethrowerModel.scheduleForDrawing();
	}

	return true;
}


//////////////////
// State Intro
//////////////////
	void daCaptainBowser::beginState_Intro() {
		this->timer = 0;
		bindAnimChr_and_setUpdateRate("kp_wait", 1, 0.0, 1.5);
		this->isInvulnerable = 1;
		roarLen = 300;
	}
	void daCaptainBowser::executeState_Intro() {

		if (this->chrAnimation.isAnimationDone()) {
			// End the intro
			if (this->isIntro == 1) {
				OSReport("We're done: %d", this->timer);
				doStateChange(&StateID_Wait);
				return;
			}
			this->chrAnimation.setCurrentFrame(0.0); }

		// Screen Rumble
		if ((this->timer > 180) && (this->timer < 420)) {
		// if (this->timer == 180) {
			// Do Rumbly - 807CD3AC

			pos.x = ClassWithCameraInfo::instance->screenCentreX;
			pos.y = ClassWithCameraInfo::instance->screenCentreY;

			ShakeScreen(StageScreen, 2, 1, 0, 0);
			PlaySoundAsync(this, SE_BOSS_KOOPA_RUMBLE1); // 0x5D4
			// Stage80::instance->ShakeScreen(self, 5, 1, 0);
		}

		// Bowser Flies In
		if (this->timer == 422) {
			this->isIntro = 2;

			for (int i = 0; i < SHIP_SCOLL_COUNT; i++)
				shipSColls[i].addToList();
		}

		if (this->timer > 420) {
			int effectiveTimer = min(timer, 719U);
			pos.x = ClassWithCameraInfo::instance->screenCentreX + ((effectiveTimer - 420.0) * 1.5) - ((roarLen * 1.5) - 200.0);
			pos.y = ClassWithCameraInfo::instance->screenCentreY - 380.0 + ((effectiveTimer - 420.0) * 1.5) - ((roarLen * 1.5) - 200.0);
		}

		// Bowser does a shitty roar
		if (this->timer == (roarLen - 190 + 420)) {
			this->isIntro = 1;
			bindAnimChr_and_setUpdateRate("kp_roar3", 1, 0.0, 1.0);
		}
		if (this->timer > (roarLen - 190 + 420)) {

			if (this->chrAnimation.getCurrentFrame() == 53.0) {
				PlaySound(this, SE_VOC_KP_L_SHOUT);
			}

			if (this->chrAnimation.getCurrentFrame() > 53.0) {
				ShakeScreen(StageScreen, 2, 2, 0, 0);
				Vec efPos = {pos.x-182.0f, pos.y+132.0f, pos.z};
				S16Vec efRot = {0,0,0x7000};
				Vec oneVec = {1.0f, 1.0f, 1.0f};
				effect.spawn("Wm_ko_shout", 0, &efPos, &efRot, &oneVec);
			}
		}

		this->timer++;
	}
	void daCaptainBowser::endState_Intro() {

		this->aPhysics.addToList();

		this->isInvulnerable = 0;
		this->isIntro = 0;
	}


//////////////////
// State Wait
//////////////////
	void daCaptainBowser::beginState_Wait() {
		if (this->isAngry == 0) {
			bindAnimChr_and_setUpdateRate("kp_wait", 1, 0.0, 1.5);
		}
		else {
			bindAnimChr_and_setUpdateRate("kp_wait", 1, 0.0, 2.0);
		}
	}
	void daCaptainBowser::executeState_Wait() {

	if (this->chrAnimation.isAnimationDone()) {
		this->chrAnimation.setCurrentFrame(0.0);

		int num = GenerateRandomNumber(4);

		if (num == 0) {
			doStateChange(&StateID_Fire);
		}
		else{
			doStateChange(&StateID_Throw);
		}
	}

	}
	void daCaptainBowser::endState_Wait() {  }


//////////////////
// State Throw
//////////////////
	void daCaptainBowser::beginState_Throw() {
		bindAnimChr_and_setUpdateRate("break", 1, 0.0, 1.0);
		this->timer = 0;
	}
	void daCaptainBowser::executeState_Throw() {

		if (this->chrAnimation.getCurrentFrame() == 60.0) { // throw back
			int num = GenerateRandomNumber(4);
			CreateActor(0x29F, 0x101 + ((num + 1) * 0x10), (Vec){pos.x+bowserX, pos.y+bowserY, pos.z}, 0, 0);
		}

		if (this->chrAnimation.getCurrentFrame() == 126.0) { // throw front
			int num = GenerateRandomNumber(4);
			CreateActor(0x29F, ((num + 1) * 0x10) + 1, (Vec){pos.x+bowserX, pos.y+bowserY, pos.z}, 0, 0);
		}

		if (this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
			if (this->isAngry == 1) {
				if (this->timer == 1) {
					doStateChange(&StateID_Wait);
				}
			}
			else {
				doStateChange(&StateID_Wait);
			}

			this->timer++;
		}

	}
	void daCaptainBowser::endState_Throw() {  }


//////////////////
// State Fire
//////////////////
	void daCaptainBowser::beginState_Fire() {
		bindAnimChr_and_setUpdateRate("fire1", 1, 0.0, 1.5);
		this->timer = 0;
	}
	void daCaptainBowser::executeState_Fire() {

		if (this->chrAnimation.getCurrentFrame() == 70.5) { // spit fire
			PlaySound(this, SE_BOSS_KOOPA_L_FIRE_SHOT);
			CreateActor(WM_ANTLION, 0, (Vec){pos.x-172.0f, pos.y+152.0f, pos.z}, 0, 0);
		}

		if (this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
			if (this->isAngry == 1) {
				if (this->timer == 1) {
					doStateChange(&StateID_Wait);
				}
			}
			else {
				doStateChange(&StateID_Wait);
			}

			this->timer++;
		}

	}
	void daCaptainBowser::endState_Fire() {  }



//////////////////
// State Roar
//////////////////
	void daCaptainBowser::beginState_Roar() {
		bindAnimChr_and_setUpdateRate("kp_roar3", 1, 0.0, 1.0);
		this->isInvulnerable = 1;
		this->timer = 0;
	}
	void daCaptainBowser::executeState_Roar() {

		if (this->chrAnimation.getCurrentFrame() == 53.0) { // This is where the smackdown starts
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_VOC_KP_L_SHOUT, 1);
		}

		if (this->chrAnimation.getCurrentFrame() > 53.0) { // This is where the smackdown starts
			Vec efPos = {pos.x-174.0f, pos.y+140.0f, pos.z};
			S16Vec efRot = {0,0,0x7000};
			Vec oneVec = {1.0f, 1.0f, 1.0f};
			effect.spawn("Wm_ko_shout", 0, &efPos, &efRot, &oneVec);
		}

		if (this->chrAnimation.isAnimationDone()) {
			doStateChange(deathSequenceRunning ? &StateID_FinalAttack : &StateID_Wait);
		}

	}
	void daCaptainBowser::endState_Roar() {
		this->isInvulnerable = 0;
		this->isAngry = 1;
	}



//////////////////
// State Damage
//////////////////
	void daCaptainBowser::beginState_Damage() {
		bindAnimChr_and_setUpdateRate("grow_big", 1, 0.0, 1.0);
		this->isInvulnerable = 1;
		this->chrAnimation.setCurrentFrame(9.0);

		PlaySound(this, SE_VOC_KP_DAMAGE_HPDP);
	}
	void daCaptainBowser::executeState_Damage() {

		if (this->chrAnimation.getCurrentFrame() == 65.0) { // stop it here before it's too late
			doStateChange(&StateID_Wait);
		}

	}
	void daCaptainBowser::endState_Damage() { this->isInvulnerable = 0; }


//////////////////
// State Outro
//////////////////
void daCaptainBowser::initiateDeathSequence() {
	deathSequenceRunning = true;
	stopMoving = true;
	saveTimer = time_macro;

	StopBGMMusic();
	dFlagMgr_c::instance->set(31, 0, false, false, false);

	*((u32*)(((char*)dBgGm_c::instance) + 0x900F0)) = 999;

	doStateChange(&StateID_Roar);
}

void daCaptainBowser::beginState_FinalAttack() {
	isInvulnerable = true;

	bindAnimChr_and_setUpdateRate("mastfail", 1, 0.0, 1.0f);

	timer = 0;
	flameScale.x = flameScale.y = flameScale.z = 0.0f;
}

void daCaptainBowser::executeState_FinalAttack() {
	timer++;

	if (timer == 56) {
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_VOC_JR_CHARGE, 1);
		handle.SetPitch(0.25f);
	} else if (timer == 185) {
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_VOC_KP_SHOUT, 1);
	} else if (timer == 348) {
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_EMY_BIG_PAKKUN_DAMAGE_1, 1);
	}

	if (timer > 204 && timer < 315) {
		if (timer == 205) {
			// First thing
			renderFlamethrowerModel = true;

			static const ActivePhysics::Info fcInfo = {
				0.0f, 0.0f, 0.0f, 0.0f, // placeholder
				3, 0, 0x4F, 0x8028E, 0,
				&dEn_c::collisionCallback
			};
			flameCollision.initWithStruct(this, &fcInfo);
			flameCollision.addToList();
		}
		if (timer < 289) {
			if (flameScale.y < 2.0f) {
				flameScale.y = flameScale.y = flameScale.z = flameScale.y + 0.1f;
			} else {
				flameScale.y = flameScale.y = flameScale.z = 2.5f;
			}
		}
		flameScale.x = flameScale.y * 1.5f;

		flameCollision.info.xDistToCenter = flameOffsetX - (48.0f * flameScale.x);
		flameCollision.info.yDistToCenter = flameOffsetY;
		flameCollision.info.xDistToEdge = 48.0f * flameScale.x;
		flameCollision.info.yDistToEdge = 7.0f * flameScale.y;

		Vec efPos = {pos.x + flameOffsetX, pos.y + flameOffsetY, flameZ};
		S16Vec efRot = {-0x4000, 0, 0};
		//flamethrowerEffect.spawn("Wm_en_fireburner", 0, &efPos, &efRot, &flameScale);
		flamethrowerEffectInd.spawn("Wm_en_fireburner6ind", 0, &efPos, &efRot, &flameScale);
		flamethrowerModel._vf1C();
		flamethrowerAnim.process();
	} else if (timer == 315) {
		renderFlamethrowerModel = false;
		flameCollision.removeFromList();
		aPhysics.removeFromList();

		Vec efPos = {pos.x + flameOffsetX - 6.0f, pos.y + flameOffsetY + 1.0f, pos.z};
		float offsets[] = {0.0f, 3.0f, 0.0f, -3.0f};
		for (int i = 0; i < 20; i++) {
			efPos.y = pos.y + flameOffsetY + offsets[i & 3];
			S16Vec nullRot = {0,0,0};
			Vec efScale = {1.5f, 1.5f, 1.5f};
			SpawnEffect("Wm_mr_fireball_hit01", 0, &efPos, &nullRot, &efScale);
			efPos.x -= 12.0f;
		}

		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_OBJ_PAIPO, 1);
	} else if (timer == 348) {
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_DEMO_ED_BALLOON_LAND, 1);

		Vec efPos = {pos.x - 72.0f, pos.y + 170.0f, pos.z};
		S16Vec nullRot = {0,0,0};
		Vec twoVec = {2.0f, 2.0f, 2.0f};
		SpawnEffect("Wm_mr_wallkick_l", 0, &efPos, &nullRot, &twoVec);
	}

	if (chrAnimation.isAnimationDone()) {
		doStateChange(&StateID_Outro);
	}
}

void daCaptainBowser::endState_FinalAttack() {
}

void daCaptainBowser::beginState_Outro() {
	WLClass::instance->_4 = 5;
	WLClass::instance->_8 = 0;
	dStage32C_c::instance->freezeMarioBossFlag = 1;

	bowserX += 56.0f;
	bindAnimChr_and_setUpdateRate("kp_death1", 1, 0.0, 1.0f);

	//shipRotY = -0x4000;
	shipAnm.bind(&shipModel, shipFile.GetResAnmChr("mastfail_after"), 1);
	shipModel.bindAnim(&shipAnm, 0.0);
	shipAnm.setUpdateRate(0.5f);

	bowserXSpeed = -3.5f;
	bowserYSpeed = 8.9f;
	bowserMaxYSpeed = -4.0f;
	bowserYAccel = -0.24375f;

	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_VOC_KP_L_FALL, 1);

	explosionBottomBound = 0.0f;

	timer = 0;
	timerWhenCrashHappens = 1000000;
	timerForVictoryDance = 1000000;
}

extern void *_8042A788;
extern void playFanfare(void *, int type);
extern dStateBase_c JrClownEndDemoState;
void daCaptainBowser::executeState_Outro() {
	timer++;

	if (!shipAnmFinished) {
		float frame = shipAnm.getCurrentFrame();

		// nuke the things!
		if (frame == 35.0f) {
			dActor_c *iter = 0;

			while (iter = (dActor_c*)dActor_c::searchByBaseType(2, iter)) {
				dStageActor_c *sa = (dStageActor_c*)iter;

				if (sa->name == EN_BIRIKYU_MAKER || sa->name == KAZAN_MGR) {
					sa->Delete(1);
				}

				if (sa->name == EN_LINE_BIRIKYU ||
						sa->name == EN_STAR_COIN ||
						sa->name == EN_HATENA_BALLOON ||
						sa->name == EN_ITEM ||
						sa->name == EN_TARZANROPE || // Meteor
						sa->name == WM_ANCHOR) { // Koopa Throw
					sa->killedByLevelClear();
					sa->Delete(1);
				}
			}

			// freeze ye olde clowne
			dEn_c *clownIter = 0;
			while (clownIter = (dEn_c*)dEn_c::search(JR_CLOWN_FOR_PLAYER, clownIter)) {
				clownIter->doStateChange(&JrClownEndDemoState);
			}
			forceClownEnds = true;

			// remove all ship collisions
			for (int i = 0; i < SHIP_SCOLL_COUNT; i++)
				shipSColls[i].removeFromList();

		} else if (frame == 150.0f) {
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_BOSS_KOOPA_CRASH, 1);
			timerWhenCrashHappens = timer;
			prplSound.Stop(10);

			ClassWithCameraInfo *cwci = ClassWithCameraInfo::instance;
			Vec efPos = {cwci->screenCentreX+168.0f, cwci->screenTop-cwci->screenHeight, pos.z};
			S16Vec nullRot = {0,0,0};
			Vec efScale = {4.0f, 4.0f, 4.0f};
			SpawnEffect("Wm_bg_volcano", 0, &efPos, &nullRot, &efScale);

			ShakeScreen(StageScreen, 17, 7, 0, 0);

		} else if (shipAnm.isAnimationDone()) {
			shipAnmFinished = true;
		}

		if (frame > 30.0f) {
			if (timer & 4 && explosionBottomBound > -249.0f) {
				static const char *efs[] = {"Wm_en_explosion", "Wm_ob_cmnboxpiece"};

				int id = MakeRandomNumber(2);
				Vec efPos = {
					pos.x - 150.0f + MakeRandomNumber(300),
					pos.y + explosionBottomBound + MakeRandomNumber(250 + explosionBottomBound),
					pos.z + 200.0f
				};
				float efScale = (float(MakeRandomNumber(30)) / 20.0f);

				S16Vec nullRot = {0,0,0};
				Vec svec = {efScale, efScale, efScale};
				SpawnEffect(efs[id], 0, &efPos, &nullRot, &svec);
			}

			if ((timer % 12) == 0 && (frame < 150.0f)) {
				nw4r::snd::SoundHandle handle;
				PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_DEMO_OP_CAKE_CLASH_1210f, 1);
			}

			explosionBottomBound -= 0.85f;
		}

		// handleYSpeed():
		bowserYSpeed += bowserYAccel;
		if (bowserYSpeed < bowserMaxYSpeed)
			bowserYSpeed = bowserMaxYSpeed;

		// doMovement()
		bowserX += bowserXSpeed;
		bowserY += bowserYSpeed;

		bowserRotX -= 0xC00;
		bowserRotY -= 0x100;
	}


	// FUN SHITS.
	if (timer == (timerWhenCrashHappens + 90)) {
		playFanfare(_8042A788, 4);
		timerForVictoryDance = timer + (7*60);

		// Prepare for moving the clowns
		clownCount = 0;

		dEn_c *unsortedClownPointers[4];
		dAcPy_c *unsortedClownPlayers[4];

		dEn_c *clownIter = 0;
		while (clownIter = (dEn_c*)dEn_c::search(JR_CLOWN_FOR_PLAYER, clownIter)) {
			dAcPy_c *player = *((dAcPy_c**)(((u32)clownIter) + 0x738));
			if (player) {
				unsortedClownPointers[clownCount] = clownIter;
				unsortedClownPlayers[clownCount] = player;
				clownCount++;
			}
		}

		// Now sort them by ID
		int sortedClownID = 0;
		for (int playerID = 0; playerID < 4; playerID++) {
			dAcPy_c *player = (dAcPy_c*)GetSpecificPlayerActor(playerID);

			for (int searchClown = 0; searchClown < clownCount; searchClown++) {
				if (player == unsortedClownPlayers[searchClown]) {
					clownPointers[sortedClownID] = unsortedClownPointers[searchClown];
					sortedClownID++;
					break;
				}
			}
		}

		ClassWithCameraInfo *cwci = ClassWithCameraInfo::instance;

		// calculate their end positions
		// Assuming each clown takes up ~32 width, and we have 16 padding between them ...
		float clownSpan = (clownCount * 48.0f) - 16.0f;
		float clownX = cwci->screenCentreX - (clownSpan / 2.0f);

		for (int i = 0; i < clownCount; i++) {
			clownDests[i].x = clownX;
			clownDests[i].y = cwci->screenCentreY;
			clownX += 48.0f;
		}
	}

	if (timer > (timerWhenCrashHappens + 90)) {
		// Let's move the clowns
		int playersAtEnd = 0;

		for (int i = 0; i < clownCount; i++) {
			dEn_c *clown = clownPointers[i];

			float moveSpeed = 1.2f;

			// Are we there already?
			float xDiff = abs(clown->pos.x - clownDests[i].x);
			float yDiff = abs(clown->pos.y - clownDests[i].y);

			int check = 0;
			if (xDiff < moveSpeed) {
				clown->pos.x = clownDests[i].x;
				check |= 1;
			}
			if (yDiff < moveSpeed) {
				clown->pos.y = clownDests[i].y;
				check |= 2;
			}
			if (check == 3) {
				playersAtEnd++;
				continue;
			}

			// Otherwise, move them a bit further
			VEC3 direction = {clownDests[i].x - clown->pos.x, clownDests[i].y - clown->pos.y, 0.0f};
			VECNormalize(&direction, &direction);
			VECScale(&direction, &direction, moveSpeed);

			clown->pos.x += direction.x;
			clown->pos.y += direction.y;
		}

		if (playersAtEnd >= clownCount) {
			//if (timerForVictoryDance == 1000000)
			//	timerForVictoryDance = timer + 45;
		}

		if (timer == timerForVictoryDance) {
			static const int vocs[4] = {
				SE_VOC_MA_CLEAR_LAST_BOSS,
				SE_VOC_LU_CLEAR_LAST_BOSS,
				SE_VOC_KO_CLEAR_LAST_BOSS,
				SE_VOC_KO2_CLEAR_LAST_BOSS
			};
			for (int i = 0; i < clownCount; i++) {
				dAcPy_c *player = *((dAcPy_c**)(((u32)clownPointers[i]) + 0x738));
				// let's be hacky
				dPlayerModelHandler_c *pmh = (dPlayerModelHandler_c*)(((u32)player) + 0x2A60);
				int whatAnim = goal_puton_capB;
				if (pmh->mdlClass->powerup_id == 4)
					whatAnim = goal_puton_capB;
				else if (pmh->mdlClass->powerup_id == 5)
					whatAnim = goal_puton_capC;
				pmh->mdlClass->startAnimation(whatAnim, 1.0f, 0.0f, 0.0f);

				nw4r::snd::SoundHandle handle;
				PlaySoundWithFunctionB4(SoundRelatedClass, &handle, vocs[pmh->mdlClass->player_id_1], 1);
			}
		}

		if (timer == (timerForVictoryDance + 180)) {
			doStateChange(&StateID_PanToExit);
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_PLY_CROWN_ACC, 1);
		}
		//dAcPy_c *splayer = *((dAcPy_c**)(((u32)clownPointers[0]) + 0x738));
		//OSReport("Player has %s\n", splayer->states2.getCurrentState()->getName());
	}
}
void daCaptainBowser::endState_Outro() {  }


void daCaptainBowser::beginState_PanToExit() {
	timer = 0;
}
void daCaptainBowser::endState_PanToExit() {
}

void daCaptainBowser::executeState_PanToExit() {
	float targetClownY = ClassWithCameraInfo::instance->screenCentreY - 160.0f;

	for (int i = 0; i < clownCount; i++) {
		dEn_c *clown = clownPointers[i];
		clown->pos.y -= 1.5f;
		if (clown->pos.y < targetClownY && !exitedFlag) {
			RESTART_CRSIN_LevelStartStruct.purpose = 0;
			RESTART_CRSIN_LevelStartStruct.world1 = 7;
			RESTART_CRSIN_LevelStartStruct.world2 = 7;
			RESTART_CRSIN_LevelStartStruct.level1 = 40;
			RESTART_CRSIN_LevelStartStruct.level2 = 40;
			RESTART_CRSIN_LevelStartStruct.areaMaybe = 0;
			RESTART_CRSIN_LevelStartStruct.entrance = 0xFF;
			RESTART_CRSIN_LevelStartStruct.unk4 = 0; // load replay
			DontShowPreGame = true;
			ExitStage(RESTART_CRSIN, 0, BEAT_LEVEL, MARIO_WIPE);
			exitedFlag = true;

			for (int i = 0; i < 4; i++)
				if (Player_Lives[i] == 0)
					Player_Lives[i] = 5;
		}
	}
}

