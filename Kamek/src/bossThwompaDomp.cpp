#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>
#include "boss.h"

// Externs
	extern "C" int posIsInZone(Vec,float*,float*,u8 zone);
	extern "C" void* ScreenPositionClass;
	extern "C" int SpawnThwompEffects(dEn_c *);

	extern "C" Vec ConvertStagePositionIntoScreenPosition__Maybe(Vec);
	extern "C" void AnotherSoundRelatedFunction(void*,SFX,Vec,int);


class daEnMegaDosun_c : public daBoss {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	static daEnMegaDosun_c *build();

	mHeapAllocator_c allocator;		// _524

	nw4r::g3d::ResFile resFile;		// _540
	m3d::mdl_c bodyModel;			// _544
	m3d::anmVis_c anmVis;			// _584

	lineSensor_s belowSensor;
	float shakePosXoffset;			// _5CC shakePosXoffset
	u16 puruMoveCounter;			// _5D8
	u16 shakeIndex;					// _5DA 0=shake,1=normal
	u32 countdownTimer;				// _5DC

	int timer;
	char notFalling;
	char dying;
	float leftBuffer;
	float rightBuffer;
	float topBuffer;
	bool isOutofScreen;
	Vec OutOfScreenPosition;

	void setupBodyModel();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);

	void powBlockActivated(bool isNotMPGP);

	USING_STATES(daEnMegaDosun_c);
	DECLARE_STATE(UpWait);
	DECLARE_STATE(DownMoveWait);
	DECLARE_STATE(PuruMove);
	DECLARE_STATE(DownMove);
	DECLARE_STATE(DownWait);
	DECLARE_STATE(UpMove);
	DECLARE_STATE(Grow);
	DECLARE_STATE(Outro);
};

// States
	CREATE_STATE(daEnMegaDosun_c, UpWait);
	CREATE_STATE(daEnMegaDosun_c, DownMoveWait);
	CREATE_STATE(daEnMegaDosun_c, PuruMove);
	CREATE_STATE(daEnMegaDosun_c, DownMove);
	CREATE_STATE(daEnMegaDosun_c, DownWait);
	CREATE_STATE(daEnMegaDosun_c, UpMove);
	CREATE_STATE(daEnMegaDosun_c, Grow);
	CREATE_STATE(daEnMegaDosun_c, Outro);

daEnMegaDosun_c *daEnMegaDosun_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daEnMegaDosun_c));
	return new(buffer) daEnMegaDosun_c;
}

void daEnMegaDosun_c::powBlockActivated(bool isNotMPGP) { }

// Collisions

	void daEnMegaDosun_c::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
		DamagePlayer(this, apThis, apOther);
	}
	bool daEnMegaDosun_c::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
		return true;
	}
	bool daEnMegaDosun_c::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
		return false;
	}
	bool daEnMegaDosun_c::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
		DamagePlayer(this, apThis, apOther);
		return true;
	}
	bool daEnMegaDosun_c::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
		return true;
	}
	bool daEnMegaDosun_c::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
		return true;
	}
	bool daEnMegaDosun_c::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther) {
		return true;
	}
	bool daEnMegaDosun_c::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther) {
		DamagePlayer(this, apThis, apOther);
		return true;
	}


void daEnMegaDosun_c::setupBodyModel() {
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->resFile.data = getResource("dossun", "g3d/t00.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("dossun_big");
	bodyModel.setup(mdl, &allocator, 0x60, 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);

	nw4r::g3d::ResAnmVis anmRes = this->resFile.GetResAnmVis("dossun_big");
	this->anmVis.setup(mdl, anmRes, &this->allocator, 0);
	this->anmVis.bind(&bodyModel, anmRes, 1);

	allocator.unlink();
}

int daEnMegaDosun_c::onCreate() {

	this->setupBodyModel();

	this->_36D = 0;			// byte

	this->frzMgr.setSomething(1,1,1);	//@0x809f5c5c

	this->pos.y -= 21.0;
	this->pos.z = -280.0;		// behind layer1 (hides spikes)

	belowSensor.flags =
		SENSOR_10000000 | SENSOR_1000000 | SENSOR_BREAK_BRICK |
		SENSOR_BREAK_BLOCK | SENSOR_100 | SENSOR_LINE;
	belowSensor.lineA = -31 << 12;
	belowSensor.lineB = 31 << 12;
	belowSensor.distanceFromCenter = 5 << 12;

	ActivePhysics::Info hm;
	hm.xDistToCenter = 0.0;
	hm.yDistToCenter = 41.0;
	hm.xDistToEdge = 36.0;
	hm.yDistToEdge = 38.0;
	hm.category1 = 0x03;
	hm.category2 = 0x00;
	hm.bitfield1 = 0x0000004F;
	hm.bitfield2 = 0x0008820E;
	hm.unkShort1C = 0x0100;
	hm.callback = &dEn_c::collisionCallback;
	this->aPhysics.initWithStruct(this, &hm);

	lineSensor_s aboveSensor(-31 << 12, 31 << 12, -31 << 12);
	this->collMgr.init(this, &belowSensor, &aboveSensor, 0);

	this->pos_delta2.x = 0.0;
	this->pos_delta2.y = 36.0;
	this->pos_delta2.z = 0.0;
	this->_320 = 0.0;
	this->_324 = 48.0;

	this->aPhysics.addToList();

	this->scale.x = 1.0;
	this->scale.y = 1.0;
	this->scale.z = 1.0;
	this->max_speed.x = 0.0;
	this->max_speed.y = -8.0;
	this->max_speed.z = 0.0;
	this->y_speed_inc = -0.25;
	this->rot.x = 0;
	this->rot.y = 0;
	this->rot.z = 0;
	this->notFalling = 0;
	this->direction = 0;
	this->countdownTimer = 0;
	this->isOutofScreen = false;

	// Measured in half tiles
	this->leftBuffer	= this->pos.x - (((float)((this->settings >> 24) & 0xFF) - 5.0) * 8.0);  //nyb 5-6 LEFT
	this->rightBuffer	= this->pos.x + (((float)((this->settings >> 16) & 0xFF) - 3.0) * 8.0);  //nyb 7-8 RIGHT
	this->topBuffer		= this->pos.y + (((float)((this->settings >> 8) & 0xFF) - 8.0) * 8.0);   //nyb 9-10 TOP


	// Boss Thwomp settings
	//
	//	nybble 5-6 		- Left Buffer in half tiles (minimum is 5 due to thwomp width)
	//	nybble 7-8 		- Left Buffer in half tiles (minimum is 3 due to thwomp width)
	//	nybble 9-10		- Top Buffer in half tiles (minimum is 8 due to thwomp height)
	//


	this->doStateChange(&StateID_Grow);

	this->onExecute();
	return true;
}

int daEnMegaDosun_c::onExecute() {
	acState.execute();

	if (this->isOutofScreen == false) {
		float rect[] = {this->_320, this->_324, this->spriteSomeRectX, this->spriteSomeRectY};
		int ret = this->outOfZone(this->pos, (float*)&rect, this->currentZoneID);
		if(ret) {
			this->OutOfScreenPosition = this->pos;

			this->isOutofScreen = true;
			doStateChange(&StateID_Outro);
		}
	}

	return true;
}

int daEnMegaDosun_c::onDraw() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);

	bodyModel.scheduleForDrawing();
	return true;
}

int daEnMegaDosun_c::onDelete() {
	return true;
}


// Grow State

	void daEnMegaDosun_c::beginState_Grow() {
		this->scale = (Vec){0.5, 0.5, 0.5};
		this->timer = 0;

		SetupKameck(this, Kameck);
	}

	void daEnMegaDosun_c::executeState_Grow() {

		bool ret;
		ret = GrowBoss(this, Kameck, 0.5, 1.0, 0, this->timer);

		if (ret) {
			PlaySound(this, SE_EMY_BIG_DOSSUN);
			doStateChange(&StateID_UpMove);
		}
		this->timer += 1;
	}
	void daEnMegaDosun_c::endState_Grow() {
		CleanupKameck(this, Kameck);
	}

// StateID_UpWait
	void daEnMegaDosun_c::beginState_UpWait() {
		belowSensor.flags =
			SENSOR_10000000 | SENSOR_1000000 | SENSOR_BREAK_BRICK |
			SENSOR_BREAK_BLOCK | SENSOR_100 | SENSOR_LINE;
		this->timer = 0;
	}
	void daEnMegaDosun_c::executeState_UpWait() {
		if(this->countdownTimer != 0) {
			this->countdownTimer--;
			return;
		}

		if (this->pos.x > this->rightBuffer) {
			Vec efPos = {this->pos.x + 38.0f, this->pos.y + 32.0f, 5500.0f};
			S16Vec nullRot = {0,0,0};
			Vec oneVec = {1.0f, 1.0f, 1.0f};
			SpawnEffect("Wm_en_dossunfall02", 0, &efPos, &nullRot, &oneVec);
			PlaySoundAsync(this, SE_OBJ_TEKKYU_G_CRASH);

			this->direction = 0;
		}

		if (this->pos.x < this->leftBuffer) {
			Vec efPos = {this->pos.x - 40.0f, this->pos.y + 32.0f, 5500.0f};
			S16Vec nullRot = {0,0,0};
			Vec oneVec = {1.0f, 1.0f, 1.0f};
			SpawnEffect("Wm_en_dossunfall02", 0, &efPos, &nullRot, &oneVec);
			PlaySoundAsync(this, SE_OBJ_TEKKYU_G_CRASH);

			this->direction = 1;
		}

		this->pos.x += (direction) ? 1.2 : -1.2;

		if (this->notFalling == 0) {
			if(this->CheckIfPlayerBelow(40.0, 256.0)) {
				this->doStateChange(&StateID_DownMoveWait);
				this->speed.y = 0.0;
			}
		}

		if (this->timer == 30) {
			this->notFalling = 0;
		}

		this->timer += 1;
	}
	void daEnMegaDosun_c::endState_UpWait() {
		this->notFalling = 1;
		return;
	}

// StateID_DownMoveWait
	void daEnMegaDosun_c::beginState_DownMoveWait() {
		this->anmVis.playState = 1;
		this->bodyModel.bindAnim(&this->anmVis, 0.5);
	}
	void daEnMegaDosun_c::executeState_DownMoveWait() {
		if(this->anmVis.isAnimationDone())
			this->doStateChange(&StateID_PuruMove);
		this->anmVis.process();
	}
	void daEnMegaDosun_c::endState_DownMoveWait() {
		return;
	}

// StateID_PuruMove
	void daEnMegaDosun_c::beginState_PuruMove() {
		this->puruMoveCounter = 8;
		this->shakeIndex = 0;
	}
	void daEnMegaDosun_c::executeState_PuruMove() {
		if(this->puruMoveCounter == 0) {
			this->shakePosXoffset = 0.0;
			this->doStateChange(&StateID_DownMove);
			return;
		}

		this->puruMoveCounter--;
		if((this->puruMoveCounter & 2) == 0)
			return;

		float _array[] = {2.0, 0.0};
		this->shakeIndex ^= 1;
		this->shakePosXoffset = _array[this->shakeIndex];
	}
	void daEnMegaDosun_c::endState_PuruMove() {
		return;
	}

// StateID_DownMove
	void daEnMegaDosun_c::beginState_DownMove() {
		this->speed.y = 0.0;
	}
	void daEnMegaDosun_c::executeState_DownMove() {
		this->HandleYSpeed();
		this->UpdateObjectPosBasedOnSpeedValuesReal();

		//FIXME what do I do? - bottom detection
		int ret = this->collMgr.calculateBelowCollisionWithSmokeEffect();
		if(!ret)
			return;

		if(!(ret & 0x400000)) {
			this->doStateChange(&StateID_DownWait);
			this->countdownTimer = 0x40;
			this->speed.y = 0.0;
			ShakeScreen(ScreenPositionClass, 0, 1, 0, 0);
			PlaySoundAsync(this, SE_EMY_BIG_DOSSUN);
		}
		else {
			belowSensor.flags = SENSOR_LINE;
			this->speed.y = 0.0;
			ShakeScreen(ScreenPositionClass, 0, 1, 0, 0);
			this->collMgr.clear2();
			PlaySoundAsync(this, SE_EMY_BIG_DOSSUN);
		}

		SpawnThwompEffects(this);
		Vec p = ConvertStagePositionIntoScreenPosition__Maybe(this->pos);
		AnotherSoundRelatedFunction(SoundRelatedClass, SE_EMY_BIG_DOSSUN, p, 0);
		//SoundRelatedClass.AnotherSoundRelatedFunction(SE_EMY_BIG_DOSSUN, p, 0);
	}
	void daEnMegaDosun_c::endState_DownMove() {
		return;
	}

// StateID_DownWait
	void daEnMegaDosun_c::beginState_DownWait() {
		return;
	}
	void daEnMegaDosun_c::executeState_DownWait() {
		if(this->countdownTimer == 0) {
			this->doStateChange(&StateID_UpMove);
		}
		else {
			this->countdownTimer--;
			if(this->countdownTimer == 0x20) {
				this->anmVis.playState = 3;
			}
		}

		this->anmVis.process();
	}
	void daEnMegaDosun_c::endState_DownWait() {
		return;
	}

// StateID_UpMove
	void daEnMegaDosun_c::beginState_UpMove() {
		this->collMgr.clear2();
	}
	void daEnMegaDosun_c::executeState_UpMove() {
		// this->speed.y = 0.0;
		// this->UpdateObjectPosBasedOnSpeedValuesReal();

		if (this->pos.y > this->topBuffer) {
			this->doStateChange(&StateID_UpWait);
			PlaySoundAsync(this, SE_OBJ_TEKKYU_L_CRASH);
			this->countdownTimer = 0xc;
		}
		else {
			this->pos.y += 1.5;
		}
	}
	void daEnMegaDosun_c::endState_UpMove() {
		return;
	}

// Outro
	void daEnMegaDosun_c::beginState_Outro() {
		OutroSetup(this);
		this->timer = 0;

		this->speed.y = 0.0;
		this->y_speed_inc = 0.0;
	}
	void daEnMegaDosun_c::executeState_Outro() {

		this->pos.y = this->OutOfScreenPosition.y + 280.0;

		if (this->timer == 0) {

			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_EMY_BIG_DOSSUN_DEAD, 1);

			S16Vec nullRot = {0,0,0};
			Vec twoVec = {2.0f, 2.0f, 2.0f};
			SpawnEffect("Wm_mr_stockitemuse_b", 0, &this->pos, &nullRot, &twoVec);
			SpawnEffect("Wm_mr_stockitemuse_c", 0, &this->pos, &nullRot, &twoVec);
		}

		if (this->timer == 60) {
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, STRM_BGM_SHIRO_BOSS_CLEAR, 1);
			BossGoalForAllPlayers();
		}

		if (this->timer == 120) {

			PlayerVictoryCries(this);
		}

		if (this->timer > 240) {
			ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE);
		}

		this->timer += 1;
	}
	void daEnMegaDosun_c::endState_Outro() { }
