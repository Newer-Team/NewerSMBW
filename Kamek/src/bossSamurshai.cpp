#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include "boss.h"

extern "C" void *MapSoundPlayer(void *SoundRelatedClass, int soundID, int unk);
extern "C" void dAcPy_vf3F8(void* player, dEn_c* monster, int t);

const char* SSarcNameList [] = {
	"Shynja",
	NULL
};

void ChucksAndKnucks(ActivePhysics *apThis, ActivePhysics *apOther);

class daSamurshai : public daBoss {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	m3d::mdl_c bodyModel;
	m3d::anmChr_c chrAnimation;

	mEf::es2 effect;

	int isBigBoss;
	int jumpCounter;
	char isDown;
	Vec initialPos;
	float XSpeed;
	u32 cmgr_returnValue;
	dStageActor_c *chosenOne;
	bool topHurts;
	bool slowDown;
	bool isBouncing;
	bool walkStateIsCharging;
	float amountCharged;

	ActivePhysics Chuckles;
	ActivePhysics Knuckles;

	static daSamurshai *build();

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);
	void updateModelMatrices();
	bool calculateTileCollisions();

	void powBlockActivated(bool isNotMPGP);

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);

	void addScoreWhenHit(void *other);
	int randomPlayer();

	bool isNearWall();

	USING_STATES(daSamurshai);

	DECLARE_STATE(Intro);
	DECLARE_STATE(Walk);
	DECLARE_STATE(Turn);
	DECLARE_STATE(Chop);
	DECLARE_STATE(ChargeSlash);
	DECLARE_STATE(Uppercut);
	DECLARE_STATE(SpinAttack);
	DECLARE_STATE(Damage);
	DECLARE_STATE(Outro);
};

daSamurshai *daSamurshai::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daSamurshai));
	return new(buffer) daSamurshai;
}

///////////////////////
// Externs and States
///////////////////////
	extern "C" int SmoothRotation(short* rot, u16 amt, int unk2);

	CREATE_STATE(daSamurshai, Intro);
	CREATE_STATE(daSamurshai, Walk);
	CREATE_STATE(daSamurshai, Turn);
	CREATE_STATE(daSamurshai, Chop);
	CREATE_STATE(daSamurshai, ChargeSlash);
	CREATE_STATE(daSamurshai, Uppercut);
	CREATE_STATE(daSamurshai, SpinAttack);
	CREATE_STATE(daSamurshai, Damage);
	CREATE_STATE(daSamurshai, Outro);


////////////////////////
// Collision Functions
////////////////////////

	void ChucksAndKnucks(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (((dEn_c*)apOther->owner)->name == PLAYER)
			((dEn_c*)apThis->owner)->_vf220(apOther->owner);
	}

	void daSamurshai::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {

		char hitType = 0;
		// this is shit code
		dStateBase_c *whatState = acState.getCurrentState();
		if (whatState == &StateID_Damage) {
			// nothing
		} else if (whatState == &StateID_Walk && walkStateIsCharging) {
			// also nothing
		} else {
			hitType = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 2);
		}

		if (hitType == 2) {
			// Mini jump
			apOther->someFlagByte |= 2;
		} else if (hitType > 0) {
			apOther->someFlagByte |= 2;
			if (this->isDown == 0) {
				this->playEnemyDownSound1();
				damage += 5;
				if (damage >= 15) { doStateChange(&StateID_Outro); }
				else { doStateChange(&StateID_Damage); }
			}
		}
		else {
			this->dEn_c::playerCollision(apThis, apOther);
			this->_vf220(apOther->owner);
		}

		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
		this->counter_504[apOther->owner->which_player] = 0;
	}

	void daSamurshai::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); }
	bool daSamurshai::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) {
		return collisionCat7_GroundPound(apThis, apOther);
	}
	bool daSamurshai::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
		apOther->someFlagByte |= 2;

		if (this->isDown == 0) {
			damage += 5;
			if (damage >= 15) { doStateChange(&StateID_Outro); }
			else { doStateChange(&StateID_Damage); }
		}

		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
		this->counter_504[apOther->owner->which_player] = 5;
		bouncePlayerWhenJumpedOn(apOther->owner);
		return true;
	}
	bool daSamurshai::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
		return this->collisionCat7_GroundPound(apThis, apOther);
	}
	bool daSamurshai::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (this->isDown == 0) {
			damage += 3;
			if (damage >= 15) { doStateChange(&StateID_Outro); }
			else { doStateChange(&StateID_Damage); }
		}
		return true;
	}
	bool daSamurshai::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther){
		return true;
	}
	bool daSamurshai::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther){
		dAcPy_vf3F8(apOther->owner, this, 3);

		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
		this->counter_504[apOther->owner->which_player] = 0;
		return true;
	}
	bool daSamurshai::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther){
		//damage += 4;
		//SpawnEffect("Wm_mr_fireball_hit", 0, &apOther->owner->pos, &apOther->owner->rot, &apOther->owner->scale);
		//PlaySoundAsync(this, SE_OBJ_FIREBALL_DISAPP);
		//if (damage >= 15) { doStateChange(&StateID_Outro); }
		return true;
	}
	bool daSamurshai::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
		//damage += 1;
		//SpawnEffect("Wm_mr_fireball_hit", 0, &apOther->owner->pos, &apOther->owner->rot, &apOther->owner->scale);
		//PlaySoundAsync(this, SE_OBJ_FIREBALL_DISAPP);
		//if (damage >= 15) { doStateChange(&StateID_Outro); }
		return true;
	}
	bool daSamurshai::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
		return true;
	}
	bool daSamurshai::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
		//damage += 2;
		//this->spawnHitEffectAtPosition((Vec2){apOther->owner->pos.x, apOther->owner->pos.y});
		//if (damage >= 15) { doStateChange(&StateID_Outro); }
		return true;
	}


	void daSamurshai::addScoreWhenHit(void *other) { }
	void daSamurshai::powBlockActivated(bool isNotMPGP) { }

	bool daSamurshai::calculateTileCollisions() {
		// Returns true if sprite should turn, false if not.

		HandleXSpeed();
		HandleYSpeed();
		doSpriteMovement();

		cmgr_returnValue = collMgr.isOnTopOfTile();
		collMgr.calculateBelowCollisionWithSmokeEffect();

		if (isBouncing) {
			stuffRelatingToCollisions(0.1875f, 1.0f, 0.5f);
			if (speed.y != 0.0f)
				isBouncing = false;
		}

		if (collMgr.isOnTopOfTile()) {
			// Walking into a tile branch

			if (cmgr_returnValue == 0)
				isBouncing = true;

			speed.y = 0.0f;
		}

		// Bouncing checks
		if (_34A & 4) {
			Vec v = (Vec){0.0f, 1.0f, 0.0f};
			collMgr.pSpeed = &v;

			if (collMgr.calculateAboveCollision(collMgr.outputMaybe))
				speed.y = 0.0f;

			collMgr.pSpeed = &speed;

		} else {
			if (collMgr.calculateAboveCollision(collMgr.outputMaybe))
				speed.y = 0.0f;
		}

		collMgr.calculateAdjacentCollision(0);

		// Switch Direction
		if (collMgr.outputMaybe & (0x15 << direction)) {
			if (collMgr.isOnTopOfTile()) {
				isBouncing = true;
			}
			pos.x += direction ? 2.0 : -2.0;
			return true;
		}
		return false;
	}

void daSamurshai::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->chrAnimation.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->chrAnimation, unk2);
	this->chrAnimation.setUpdateRate(rate);
}

int daSamurshai::onCreate() {

	isBigBoss = settings & 0xF;

	// Model Setup
		allocator.link(-1, GameHeaps[0], 0, 0x20);

		this->resFile.data = getResource("Shynja", "g3d/Shynja.brres");
		nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("Shynja");
		bodyModel.setup(mdl, &allocator, 0x224, 1, 0);

		// Animations start here
		nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("c18_IDLE_R");
		this->chrAnimation.setup(mdl, anmChr, &this->allocator, 0);

		allocator.unlink();

	// Character Setup

		scale = (Vec){25.0f, 25.0f, 25.0f};

		pos.y = pos.y - 16.0 * 3.0;
		rot.x = 0; // X is vertical axis
		rot.y = 0xD800; // Y is horizontal axis
		rot.z = 0; // Z is ... an axis >.>
		direction = 1; // Heading left.

		speed.x = 0.0;
		speed.y = 0.0;
		XSpeed = 2.5;
		max_speed.x = 50.0;
		initialPos = pos;
		topHurts = false;

	// Physics
		ActivePhysics::Info HitMeBaby;

		HitMeBaby.xDistToCenter = 0.0;
		HitMeBaby.yDistToCenter = 15.0;

		HitMeBaby.xDistToEdge = 13.0;
		HitMeBaby.yDistToEdge = 15.0;

		HitMeBaby.category1 = 0x3;
		HitMeBaby.category2 = 0x0;
		HitMeBaby.bitfield1 = 0x4F;
		HitMeBaby.bitfield2 = 0xffbafffe;
		HitMeBaby.unkShort1C = 0;
		HitMeBaby.callback = &dEn_c::collisionCallback;

		this->aPhysics.initWithStruct(this, &HitMeBaby);
		this->aPhysics.addToList();

	// Tile collider

		// These fucking rects do something for the tile rect
		spriteSomeRectX = 48.0f;
		spriteSomeRectY = 36.0f;
		_320 = 0.0f;
		_324 = 18.0f;

		// These structs tell stupid collider what to collide with - these are from koopa troopa
		static const lineSensor_s below(-0<<12, 0<<12, 0<<12);
		static const pointSensor_s above(0<<12, 12<<12);
		static const lineSensor_s adjacent(6<<12, 9<<12, 6<<12);

		collMgr.init(this, &below, &above, &adjacent);
		collMgr.calculateBelowCollisionWithSmokeEffect();

		cmgr_returnValue = collMgr.isOnTopOfTile();

		if (collMgr.isOnTopOfTile())
			isBouncing = false;
		else
			isBouncing = true;

	// Sword Physics
		// Chuckles is left, Knuckles is Right
		ActivePhysics::Info iChuckles;
		ActivePhysics::Info iKnuckles;

		iChuckles.xDistToCenter = -27.0;
		iChuckles.yDistToCenter = 8.0;
		iChuckles.xDistToEdge   = 27.0;
		iChuckles.yDistToEdge   = 6.0;

		iKnuckles.xDistToCenter = 27.0;
		iKnuckles.yDistToCenter = 8.0;
		iKnuckles.xDistToEdge   = 27.0;
		iKnuckles.yDistToEdge   = 6.0;

		iKnuckles.category1  = iChuckles.category1  = 0x3;		iKnuckles.category2  = iChuckles.category2  = 0x0;
		iKnuckles.bitfield1  = iChuckles.bitfield1  = 0x4F;
		iKnuckles.bitfield2  = iChuckles.bitfield2  = 0x6;
		iKnuckles.unkShort1C = iChuckles.unkShort1C = 0x0;
		iKnuckles.callback   = iChuckles.callback   = ChucksAndKnucks;

		Chuckles.initWithStruct(this, &iChuckles);
		Knuckles.initWithStruct(this, &iKnuckles);

	doStateChange(&StateID_Intro);

	this->onExecute();
	return true;
}

int daSamurshai::onDelete() {
	return true;
}
int daSamurshai::onExecute() {
	acState.execute();
	updateModelMatrices();

	return true;
}
int daSamurshai::onDraw() {
	bodyModel.scheduleForDrawing();
	bodyModel._vf1C();

	return true;
}
void daSamurshai::updateModelMatrices() {
	// This won't work with wrap because I'm lazy.
	matrix.translation(pos.x, pos.y + 0.0, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


///////////////
// Intro State
///////////////
	void daSamurshai::beginState_Intro() {
		this->timer = 0;

		// Stop the BGM Music
		StopBGMMusic();

		// Set the necessary Flags and make Mario enter Demo Mode
		dStage32C_c::instance->freezeMarioBossFlag = 1;
		WLClass::instance->_4 = 4;
		WLClass::instance->_8 = 0;

		MakeMarioEnterDemoMode();

		// Make sure to use the correct position
		Vec KamekPos = (Vec){pos.x - 124.0f, pos.y + 104.0f, 3564.0f};
		S16Vec KamekRot = (S16Vec){0, 0, 0};

		rot.y = 0x2800; // Y is horizontal axis
		speed.x = 0.0;
		speed.y = 0.0;

		pos.x = pos.x - 224.0;
		pos.y = pos.y + 320.0;

		// Create And use Kameck
		Kameck = (daKameckDemo*)createChild(KAMECK_FOR_CASTLE_DEMO, (dStageActor_c*)this, 0, &KamekPos, &KamekRot, 0);
		Kameck->doStateChange(&daKameckDemo::StateID_DemoSt);
		Kameck->pos.x = Kameck->pos.x - 32.0;
	}

	void daSamurshai::executeState_Intro() {
		this->timer += 1;

		OSReport("Timer: %d", timer);
		if (timer == 230) { bindAnimChr_and_setUpdateRate("c18_INTRO", 1, 0.0, 1.0); }

		if ((timer > 220) && (timer < 240)) {
			pos.x += (224.0 / 20.0);
			pos.y -= (320.0 / 20.0);
		}

		if (timer == 230) {
			S16Vec nullRot = {0,0,0};
			Vec petalScale = {1.5f, 1.5f, 1.5f};
			Vec ringPos = {pos.x, pos.y + 16.0f, pos.z};
			Vec ringScale = {8.0f, 0.1f, 1.5f};
			SpawnEffect("Wm_en_hanapetal", 0, &pos, &nullRot, &petalScale);
			SpawnEffect("Wm_ob_itemget_ring", 0, &ringPos, &nullRot, &ringScale);
			MapSoundPlayer(SoundRelatedClass, SE_OBJ_WOOD_BOX_BREAK, 1);
			MapSoundPlayer(SoundRelatedClass, SE_BOSS_KAMECK_DOWN, 1);
			this->Kameck->doStateChange(&daKameckDemo::StateID_DieFall);
		}

		if ((timer > 230) && (timer < 350)) {
			Kameck->pos.x += (200.0 / 120.0);
			Kameck->pos.y -= (260.0 / 120.0);
		}

		int done = 0;
		if ((timer > 330) && (done == 0)) {
			u16 amt = (this->direction == 0) ? 0x2800 : 0xD800;
			done = SmoothRotation(&this->rot.y, amt, 0x2000);
		}

		if (timer == 400) { bindAnimChr_and_setUpdateRate("c18_IDLE_R", 1, 0.0, 1.0); }

		if (timer == 500) {
			MapSoundPlayer(SoundRelatedClass, SE_BOSS_WENDY_RING_BOUND, 1);
			walkStateIsCharging = false;
			doStateChange(&StateID_Walk);
		}
	}
	void daSamurshai::endState_Intro() {
		CleanupKameck(this, Kameck);

		rot.y = 0xD800; // Y is horizontal axis
		direction = 1; // Heading left.
		XSpeed = 2.0;
		max_speed.x = 50.0;
	}

///////////////
// Walk State
///////////////
	int daSamurshai::randomPlayer() {
		int players[4];
		int playerCount = 0;

		for (int i = 0; i < 4; i++) {
			if (Player_Active[i] != 0 && Player_Lives[Player_ID[i]] > 0) {
				players[playerCount] = i;
				playerCount++;
			}
		}
		return players[MakeRandomNumber(playerCount)];
	}

	void daSamurshai::beginState_Walk() {
		Chuckles.removeFromList();
		Knuckles.removeFromList();

		bindAnimChr_and_setUpdateRate(walkStateIsCharging ? "c18_CHARGE" : "c18_RUNNING", 1, 0.0, 0.5);
		chosenOne = GetSpecificPlayerActor(this->randomPlayer());

		speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
		if (walkStateIsCharging)
			speed.x *= 1.7f;

		this->max_speed.y = -4.0;
		this->speed.y = -4.0;
		this->y_speed_inc = -0.1875;

		OSReport("Speed: %f / %f", speed.x, max_speed.x);
	}
	void daSamurshai::executeState_Walk() {
		MapSoundPlayer(SoundRelatedClass, SE_EMY_MOUSE_WALK, 1);

		float xDistance = pos.x - chosenOne->pos.x;
		float yDistance = pos.y - chosenOne->pos.y;

		// OSReport("Distance: %f, %f", xDistance, yDistance);
		if (!walkStateIsCharging && (xDistance >  64.0) && (direction == 0)) { doStateChange(&StateID_Turn); }
		if (!walkStateIsCharging && (xDistance < -64.0) && (direction == 1)) { doStateChange(&StateID_Turn); }

		if (xDistance < 0.0) { xDistance = -xDistance; }

		if (walkStateIsCharging) {
			amountCharged += abs(speed.x);

			// should we stop charging?
			if (amountCharged > 480.0f && !isNearWall()) {
				speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
				doStateChange(&StateID_Uppercut);
			}
		}

		// Condition for Chop
		if (!walkStateIsCharging && xDistance < 32.0) { doStateChange(&StateID_Chop); }

		// Condition For Charge Slash
		if (!walkStateIsCharging && isBigBoss) {
			if ((xDistance < 96.0) && (xDistance > 64.0)) {
				int charge = MakeRandomNumber(100);
				if (charge > 95) { doStateChange(&StateID_ChargeSlash); }
			}
		}

		// Aerial Attacks!
		if (!walkStateIsCharging && yDistance < -24.0) {

			// Condition For Spin Attack
			// if (xDistance < 32.0) { doStateChange(&StateID_SpinAttack); }

			// Condition for Uppercut
			if ((xDistance > 48.0) && (xDistance < 64.0) && !isNearWall())
				doStateChange(&StateID_Uppercut);
		}

		bool ret = calculateTileCollisions();
		if (ret) {
			doStateChange(&StateID_Turn);
		}

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}
	}
	void daSamurshai::endState_Walk() { }

///////////////
// Turn State
///////////////
	void daSamurshai::beginState_Turn() {
		bindAnimChr_and_setUpdateRate("c18_RUNNING", 1, 0.0, 0.5);
		this->speed.x = (direction) ? 0.5f : -0.5f;
		this->direction ^= 1;
		x_speed_inc = 0.0;
	}
	void daSamurshai::executeState_Turn() {

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		u16 amt = (this->direction == 0) ? 0x2800 : 0xD800;
		int done = SmoothRotation(&this->rot.y, amt, 0x1000);

		if(done) {
			this->doStateChange(&StateID_Walk);
		}
	}
	void daSamurshai::endState_Turn() { }

///////////////
// Chop State
///////////////
	void daSamurshai::beginState_Chop() {
		bindAnimChr_and_setUpdateRate("c18_H_CUT_R", 1, 0.0, 1.0);
		chrAnimation.setCurrentFrame(0.0);
		timer = 0;

	}
	void daSamurshai::executeState_Chop() {

		speed.x = speed.x / 1.5;

		if (chrAnimation.getCurrentFrame() == 15.0) {
			MapSoundPlayer(SoundRelatedClass, SE_EMY_CRASHER_PUNCH, 1);
			if (this->direction == 1) {
				Vec efPos = {pos.x - 18.0f, pos.y + 16.0f, pos.z-200.0f};
				S16Vec nullRot = {0,0,0};
				Vec efScale = {2.5f, 0.5f, 1.5f};
				SpawnEffect("Wm_ob_itemget_ring", 0, &efPos, &nullRot, &efScale);
				Chuckles.addToList();
			}
			else {
				Vec efPos = {pos.x + 18.0f, pos.y + 16.0f, pos.z-200.0f};
				S16Vec nullRot = {0,0,0};
				Vec efScale = {2.5f, 0.5f, 1.5f};
				SpawnEffect("Wm_ob_itemget_ring", 0, &efPos, &nullRot, &efScale);
				Knuckles.addToList();
			}
		}

		if (chrAnimation.getCurrentFrame() == 20.0) {
			MapSoundPlayer(SoundRelatedClass, SE_OBJ_WOOD_BOX_BREAK, 1);
			if (this->direction == 1) {
				S16Vec nullRot = {0,0,0};

				Vec hitPos = {pos.x - 38.0f, pos.y + 16.0f, pos.z};
				Vec oneVec = {1.0f, 1.0f, 1.0f};
				SpawnEffect("Wm_en_hit", 0, &hitPos, &nullRot, &oneVec);

				Vec petalPos = {pos.x - 38.0f, pos.y + 16.0f, pos.z};
				Vec twoVec = {2.0f, 2.0f, 2.0f};
				SpawnEffect("Wm_en_hanapetal", 0, &petalPos, &nullRot, &twoVec);

				Chuckles.removeFromList();
			}
			else {
				S16Vec nullRot = {0,0,0};

				Vec hitPos = {pos.x + 38.0f, pos.y + 16.0f, pos.z};
				Vec oneVec = {1.0f, 1.0f, 1.0f};
				SpawnEffect("Wm_en_hit", 0, &hitPos, &nullRot, &oneVec);

				Vec petalPos = {pos.x + 38.0f, pos.y + 16.0f, pos.z};
				Vec twoVec = {2.0f, 2.0f, 2.0f};
				SpawnEffect("Wm_en_hanapetal", 0, &petalPos, &nullRot, &twoVec);

				Knuckles.removeFromList();
			}
		}

		if(this->chrAnimation.isAnimationDone()) {
			timer += 1;
			if (timer > 45) {
				walkStateIsCharging = false;
				doStateChange(&StateID_Walk);
			}
		}
	}
	void daSamurshai::endState_Chop() { }

///////////////
// ChargeSlash State
///////////////
	void daSamurshai::beginState_ChargeSlash() {
		timer = 0;
		slowDown = false;
		topHurts = false;
		speed.x = 0.0;
	}
	void daSamurshai::executeState_ChargeSlash() {

		// End if the animation is finally over
		if ((chrAnimation.isAnimationDone()) && (slowDown) ) {
			walkStateIsCharging = false;
			doStateChange(&StateID_Walk);
			return;
		}

		// What to do if he hits a wall
		bool ret = calculateTileCollisions();
		if (ret) {
			OSReport("Hit the wall");
			bindAnimChr_and_setUpdateRate("c18_R_BLOCK_BREAK_R", 1, 0.0, 1.0);
			speed.x = (direction) ? 0.5f : -0.5f;
			topHurts = false;
			slowDown = true;
			timer = 500;
		}

		if (timer == 500) { speed.x = speed.x / 1.05; return; }

		// Begin the charge effect
		if (timer == 0) { bindAnimChr_and_setUpdateRate("c18_OB_IDLE_R", 1, 0.0, 0.5);
			if (direction == 1) {
				Vec efPos = {pos.x + 7.0f, pos.y + 14.0f, pos.z - 5500.0f};
				S16Vec nullRot = {0,0,0};
				Vec efScale = {0.8f, 0.8f, 0.8f};
				SpawnEffect("Wm_ob_keyget02_lighit", 0, &efPos, &nullRot, &efScale);
			} else {
				Vec efPos = {pos.x - 7.0f, pos.y + 14.0f, pos.z + 5500.0f};
				S16Vec nullRot = {0,0,0};
				Vec efScale = {0.8f, 0.8f, 0.8f};
				SpawnEffect("Wm_ob_keyget02_lighit", 0, &efPos, &nullRot, &efScale);
			}
		}

		// Start to cut
		if (timer == 60) { bindAnimChr_and_setUpdateRate("c18_H_CUT_R", 1, 0.0, 1.0); }

		// After enough charging, speed on!
		if (timer == 70) { speed.x = (direction) ? -16.0f : 16.0f; chrAnimation.setUpdateRate(0.0); }

		// He should not be able to be hurt for a while
		if (timer == 72) { topHurts = true; }

		OSReport("Speed: %f @ %d", speed.x, timer);
		float absSpeed;
		if (speed.x < 0.0) { absSpeed = -speed.x; }
		else			   { absSpeed =  speed.x; }

		// Can be bopped again when he gets slow enough
		if (absSpeed < 1.0) { topHurts = false; chrAnimation.setUpdateRate(1.0); }

		// During the dash
		if (absSpeed > 0.0) {
			// Slow that horsey down, tiger
			if (slowDown) { speed.x = speed.x / 1.5; return; }

			S16Vec nullRot = {0,0,0};

			Vec iceslipPos = {pos.x, pos.y+8.0f, pos.z};
			Vec iceslipScale = {1.5f, 1.5f, 1.5f};
			effect.spawn("Wm_mr_p_iceslip", 0, &iceslipPos, &nullRot, &iceslipScale);

			Vec petalScale = {1.5f, 1.5f, 1.5f};
			SpawnEffect("Wm_en_hanapetal", 0, &pos, &nullRot, &petalScale);

			Vec ringPos = {pos.x, pos.y + 16.0f, pos.z};
			Vec ringScale = {4.0f, 0.1f, 1.5f};
			SpawnEffect("Wm_ob_itemget_ring", 0, &ringPos, &nullRot, &ringScale);

			if (timer == 75) { slowDown = true; }
			// Positive if Mario is left of Samurai, negative if he is to the right
			// float xDistance = pos.x - chosenOne->pos.x;

			// // direction 1 is going left, direction 0 is going right
			// if ((xDistance > 0) && (direction == 0)) {
			// 	SpawnEffect("Wm_ob_itemget_ring", 0, &(Vec){pos.x, pos.y + 16.0, pos.z}, &(S16Vec){0,0,0}, &(Vec){4.0, 0.1, 1.5});
			// 	slowDown = true; }

			// if ((xDistance < 0) && (direction == 1)) {
			// 	SpawnEffect("Wm_ob_itemget_ring", 0, &(Vec){pos.x, pos.y + 16.0, pos.z}, &(S16Vec){0,0,0}, &(Vec){4.0, 0.1, 1.5});
			// 	slowDown = true; }
		}

		this->timer += 1;
	}
	void daSamurshai::endState_ChargeSlash() {
		slowDown = false;
	}

///////////////
// Uppercut State
///////////////
	void daSamurshai::beginState_Uppercut() {
		bindAnimChr_and_setUpdateRate("c18_H_SHOT_R", 1, 0.0, 2.0);
		slowDown = false;
		timer = 0;
	}
	void daSamurshai::executeState_Uppercut() {

		if ((slowDown) && (this->chrAnimation.isAnimationDone())) {
			timer++;
			if (timer > 45) {
				walkStateIsCharging = false;
				doStateChange(&StateID_Walk);
			}
		}

		if (slowDown) {
			return;
		}

		if (this->chrAnimation.getCurrentFrame() == 34.0) {
			topHurts = true;
			MapSoundPlayer(SoundRelatedClass, SE_EMY_CRASHER_PUNCH, 1);
			MapSoundPlayer(SoundRelatedClass, SE_BOSS_CMN_JUMP_M, 1);

			speed.y = 4.0;
			y_speed_inc = -0.1875;

			if (this->direction == 1) {
				Vec ringPos = {pos.x - 48.0f, pos.y + 32.0f, pos.z};
				S16Vec nullRot = {0,0,0};
				Vec ringScale = {0.5f, 1.5f, 1.5f};
				effect.spawn("Wm_ob_itemget_ring", 0, &ringPos, &nullRot, &ringScale);
				Chuckles.addToList();
			}
			else {
				Vec ringPos = {pos.x + 48.0f, pos.y + 32.0f, pos.z};
				S16Vec nullRot = {0,0,0};
				Vec ringScale = {0.5f, 1.5f, 1.5f};
				effect.spawn("Wm_ob_itemget_ring", 0, &ringPos, &nullRot, &ringScale);
				Knuckles.addToList();
			}
		}

		if (this->chrAnimation.getCurrentFrame() == 64.0) {
			if (topHurts) {
				if (this->direction == 1) {
					S16Vec nullRot = {0,0,0};

					Vec hitPos = {pos.x - 38.0f, pos.y + 16.0f, pos.z};
					Vec oneVec = {1.0f, 1.0f, 1.0f};
					SpawnEffect("Wm_en_hit", 0, &hitPos, &nullRot, &oneVec);

					Vec petalPos = {pos.x - 38.0f, pos.y + 16.0f, pos.z};
					Vec twoVec = {2.0f, 2.0f, 2.0f};
					SpawnEffect("Wm_en_hanapetal", 0, &petalPos, &nullRot, &twoVec);

					Chuckles.removeFromList();
				}
				else {
					S16Vec nullRot = {0,0,0};

					Vec hitPos = {pos.x + 38.0f, pos.y + 16.0f, pos.z};
					Vec oneVec = {1.0f, 1.0f, 1.0f};
					SpawnEffect("Wm_en_hit", 0, &hitPos, &nullRot, &oneVec);

					Vec petalPos = {pos.x + 38.0f, pos.y + 16.0f, pos.z};
					Vec twoVec = {2.0f, 2.0f, 2.0f};
					SpawnEffect("Wm_en_hanapetal", 0, &petalPos, &nullRot, &twoVec);

					Knuckles.removeFromList();
				}

				MapSoundPlayer(SoundRelatedClass, SE_OBJ_WOOD_BOX_BREAK, 1);
				topHurts = false;
			}
		}

		if ((this->chrAnimation.getCurrentFrame() > 34.0) && (speed.y == 0)) {
			speed.x = 0.0f;
		}

		if ((this->chrAnimation.isAnimationDone()) && (speed.y == 0.0)) {
			bindAnimChr_and_setUpdateRate("c18_DIVING_STEAL_R", 1, 0.0, 1.0);
			slowDown = true;
		}

		bool ret = calculateTileCollisions();
	}
	void daSamurshai::endState_Uppercut() { slowDown = false; }

///////////////
// SpinAttack State
///////////////
	void daSamurshai::beginState_SpinAttack() {
		bindAnimChr_and_setUpdateRate("c18_NORMAL_STEAL_R", 1, 0.0, 1.0);
	}
	void daSamurshai::executeState_SpinAttack() {

		if((int)this->chrAnimation.getCurrentFrame() == 20) {
			topHurts = true;
			PlaySoundAsync(this, SE_EMY_CRASHER_PUNCH);

			speed.y = 3.0;
			y_speed_inc = -0.1875;

			if (this->direction == 1) { Chuckles.addToList(); }
			else { Knuckles.addToList(); }
		}

		if (((int)chrAnimation.getCurrentFrame() > 20) && ((int)this->chrAnimation.getCurrentFrame() < 60)) {
			scale = (Vec){1.0, 1.0, 1.0};
			if (this->direction == 1) {
				Vec efPos = {pos.x - 18.0f, pos.y + 16.0f, pos.z};
				S16Vec nullRot = {0,0,0};
				effect.spawn("Wm_mr_spinjump", 0, &efPos, &nullRot, &scale);
			}
			else {
				Vec efPos = {pos.x + 18.0f, pos.y + 16.0f, pos.z};
				S16Vec nullRot = {0,0,0};
				effect.spawn("Wm_mr_spinjump", 0, &efPos, &nullRot, &scale);
			}
		}

		if((int)this->chrAnimation.getCurrentFrame() == 65) {
			topHurts = false;
			PlaySoundAsync(this, SE_EMY_CRASHER_PUNCH);

			if (this->direction == 1) { Chuckles.removeFromList(); }
			else { Knuckles.removeFromList(); }
		}

		if ((this->chrAnimation.isAnimationDone()) && (speed.y == 0.0)) {
			walkStateIsCharging = false;
			doStateChange(&StateID_Walk);
		}

		bool ret = calculateTileCollisions();

	}
	void daSamurshai::endState_SpinAttack() {
	}

///////////////
// Damage State
///////////////
	void daSamurshai::beginState_Damage() {
		bindAnimChr_and_setUpdateRate("c18_L_DMG_F_1_R", 1, 0.0, 1.0);

		this->max_speed.x = 0;
		this->speed.x = 0;
		this->x_speed_inc = 0;

		this->max_speed.y = -2.0;
		this->speed.y = -2.0;
		this->y_speed_inc = -0.1875;

		this->timer = 0;
		this->isDown = 1;

		Chuckles.removeFromList();
		Knuckles.removeFromList();
		aPhysics.removeFromList();

		Vec efPos = {pos.x, pos.y + 16.0f, pos.z};
		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_ob_switch", 0, &efPos, &nullRot, &oneVec);
		MapSoundPlayer(SoundRelatedClass, SE_BOSS_CMN_DAMAGE_DEF, 1);
	}
	void daSamurshai::executeState_Damage() {
		calculateTileCollisions();

		Vec efPos = {pos.x, pos.y + 40.0f, 0};
		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		effect.spawn("Wm_en_spindamage", 0, &efPos, &nullRot, &oneVec);

		if (this->chrAnimation.isAnimationDone()) {
			if 		(timer == 0) { bindAnimChr_and_setUpdateRate("c18_L_DMG_F_3_R", 1, 0.0, 1.0); timer = 1; }
			else if (timer == 1) { bindAnimChr_and_setUpdateRate("c18_L_DMG_F_4_R", 1, 0.0, 1.0); timer = 2; }
			else if (timer == 2) {
				walkStateIsCharging = true;
				amountCharged = 0.0f;
				doStateChange(&StateID_Walk);
			}
		}
	}
	void daSamurshai::endState_Damage() {
		aPhysics.addToList();

		this->isDown = 0;
		this->rot.y = (direction) ? 0xD800 : 0x2800;
	}

///////////////
// Outro State
///////////////
	void daSamurshai::beginState_Outro() {

		bindAnimChr_and_setUpdateRate("c18_SP_BLOCK_R", 1, 0.0, 1.0);
		OutroSetup(this);
		aPhysics.removeFromList();

		this->max_speed.x = 0;
		this->speed.x = 0;
		this->x_speed_inc = 0;

		this->max_speed.y = -2.0;
		this->speed.y = -2.0;
		this->y_speed_inc = -0.1875;

		this->timer = 0;
		this->isDown = 1;

		Chuckles.removeFromList();
		Knuckles.removeFromList();
		MapSoundPlayer(SoundRelatedClass, SE_BOSS_CMN_DAMAGE_LAST, 1);

	}
	void daSamurshai::executeState_Outro() {
		calculateTileCollisions();

		if (this->dying == 1) {

			if (this->timer > 180) { ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE); }
			if (this->timer == 60) { PlayerVictoryCries(this); }

			this->timer += 1;
			return;
		}

		if ((chrAnimation.getCurrentFrame() == 41.0) || (chrAnimation.getCurrentFrame() == 62.0)) {
			Vec efPos = {pos.x, pos.y - 8.0f, pos.z + 500.0f};
			S16Vec nullRot = {0,0,0};
			Vec oneVec = {1.0f, 1.0f, 1.0f};
			SpawnEffect("Wm_en_landsmoke_s", 0, &efPos, &nullRot, &oneVec);
		}

		if (chrAnimation.isAnimationDone()) {
			S16Vec nullRot = {0,0,0};

			Vec efPos = {pos.x + 8.0f, pos.y - 8.0f, pos.z + 500.0f};
			Vec efScale = {1.75f, 1.75f, 1.75f};
			SpawnEffect("Wm_ob_cmnshotstar", 0, &efPos, &nullRot, &efScale);

			Vec hitPos = {pos.x + 8.0f, pos.y, pos.z + 500.0f};
			Vec oneVec = {1.0f, 1.0f, 1.0f};
			SpawnEffect("Wm_mr_wirehit_hit", 0, &hitPos, &nullRot, &oneVec);

			MapSoundPlayer(SoundRelatedClass, STRM_BGM_SHIRO_BOSS_CLEAR, 1);
			BossGoalForAllPlayers();

			this->dying = 1;
			this->timer = 0;
		}

		this->timer += 1;
	}
	void daSamurshai::endState_Outro() { }

bool daSamurshai::isNearWall() {
	// back up our current settings
	VEC3 savePos = pos;
	VEC3 saveSpeed = speed;
	int saveDirection = direction;

	float checkLeft = (direction == 0) ? 8.0f : 104.0f;
	float checkRight = (direction == 0) ? 104.0f : 8.0f;

	bool result = false;

	speed.x = -0.1f;
	speed.y = 0.0f;
	pos.x = savePos.x - checkLeft;
	direction = 1; // left
	if (collMgr.calculateAdjacentCollision())
		result = true;

	speed.x = 0.1f;
	speed.y = 0.0f;
	pos.x = savePos.x + checkRight;
	direction = 0; // right
	if (collMgr.calculateAdjacentCollision())
		result = true;

	// restore our settings
	pos = savePos;
	speed = saveSpeed;
	direction = saveDirection;

	return result;
}

