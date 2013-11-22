#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>
#include "boss.h"

class daDreidel : public daBoss {
public:
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;

	m3d::mdl_c bodyModel;

	m3d::anmChr_c chrAnimation;

	char isDown;
	float XSpeed;
	u32 cmgr_returnValue;
	bool isBouncing;
	char isInSpace;
	char fromBehind;
	char isInvulnerable;
	int isInvulnerableCountdown;
	int isTurningCountdown;
	char charging;
	int flashing;

	float dying;

	static daDreidel *build();

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);
	void updateModelMatrices();
	bool calculateTileCollisions();

	void powBlockActivated(bool isNotMPGP);

	void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);
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

	USING_STATES(daDreidel);
	DECLARE_STATE(Walk);
	DECLARE_STATE(Turn);
	DECLARE_STATE(KnockBack);

	DECLARE_STATE(ChargePrep);
	DECLARE_STATE(Charge);
	DECLARE_STATE(ChargeRecover);
	DECLARE_STATE(Damage);

	DECLARE_STATE(Grow);
	DECLARE_STATE(Outro);

};

daDreidel *daDreidel::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daDreidel));
	return new(buffer) daDreidel;
}

///////////////////////
// Externs and States
///////////////////////
	extern "C" void *EN_LandbarrelPlayerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther);
	// CalculateDistanceFromActorsNextPosToFurthestPlayer???
	extern "C" int SmoothRotation(short* rot, u16 amt, int unk2);


	CREATE_STATE(daDreidel, Walk);
	CREATE_STATE(daDreidel, Turn);
	CREATE_STATE(daDreidel, KnockBack);

	CREATE_STATE(daDreidel, ChargePrep);
	CREATE_STATE(daDreidel, Charge);
	CREATE_STATE(daDreidel, ChargeRecover);
	CREATE_STATE(daDreidel, Damage);

	CREATE_STATE(daDreidel, Grow);
	CREATE_STATE(daDreidel, Outro);

	// 	begoman_attack2"	// wobble back and forth tilted forwards
	// 	begoman_attack3"	// Leaned forward, antennae extended
	// 	begoman_damage"		// Bounces back slightly
	// 	begoman_damage2"	// Stops spinning and wobbles to the ground like a top
	// 	begoman_stand"		// Stands still, waiting
	// 	begoman_wait"		// Dizzily Wobbles
	// 	begoman_wait2"		// spins around just slightly
	// 	begoman_attack"		// Rocks backwards, and then attacks to an upright position, pulsing out his antennae


////////////////////////
// Collision Functions
////////////////////////
	// void topCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther);

	// void topCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther) {
	// 	OSReport("Collided with %d", apOther->owner->name);
	// 	if (apOther->owner->name != 041) {
	// 		dEn_c::collisionCallback(apThis, apOther);
	// 	}
	// 	else {
	// 		daDreidel *actor = (daDreidel*)apThis->owner;
	// 		actor->damage += 1;
	// 		actor->doStateChange(&daDreidel::StateID_Damage);

	// 		if (actor->damage > 2) { actor->doStateChange(&daDreidel::StateID_Outro); }
	// 	}
	// }
	void daDreidel::addScoreWhenHit(void *other) { };

	void daDreidel::spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (apOther->owner->name == 390) { //time to get hurt
			OSReport("YO SUP I'M A TOPMAN AND I'M COLLIDING WITH A FUCKING WALL [%d]\n", damage);
			if (this->isInvulnerable) {
				OSReport("I'm invulnerable so I'm ignoring it\n");
				return;
			}
			this->damage += 1;
			OSReport("I'm increasing my damage to %d\n", damage);
			doStateChange(&StateID_Damage);

			if (this->damage == 3) { doStateChange(&StateID_Outro); }
		}
		else { dEn_c::spriteCollision(apThis, apOther); }
	}

	void daDreidel::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {


		char hitType;
		hitType = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 0);

		if(hitType > 0) {
			this->_vf220(apOther->owner);
			this->counter_504[apOther->owner->which_player] = 0;
		} else {

			if (this->charging == 1) {

				char MarioPos;

				if (this->pos.x < apOther->owner->pos.x) { MarioPos = 0; } // Mario is to the right
				else 									 { MarioPos = 1; } // Mario is to the left


				if (this->direction != MarioPos) {	// Mario is stnading behind the boss
					EN_LandbarrelPlayerCollision(this, apThis, apOther);
					if (MarioPos == 1)  { this->direction = 1; }
					else 				{ this->direction = 0; }

					doStateChange(&StateID_KnockBack);
				}

				else { // Mario is standing in front of the boss
					this->_vf220(apOther->owner);
					EN_LandbarrelPlayerCollision(this, apThis, apOther);
				}
			}

			else if (this->isInvulnerable == 0) {

				if (this->pos.x > apOther->owner->pos.x) {
					this->direction = 1;
				}
				else {
					this->direction = 0;
				}

				this->counter_504[apOther->owner->which_player] = 0;
				EN_LandbarrelPlayerCollision(this, apThis, apOther);
				doStateChange(&StateID_KnockBack);
			}
			else {
				this->counter_504[apOther->owner->which_player] = 0;
				EN_LandbarrelPlayerCollision(this, apThis, apOther);
			}
		}

		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
	}

	void daDreidel::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->playerCollision(apThis, apOther);
	}

	bool daDreidel::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) {
		DamagePlayer(this, apThis, apOther);
		return true;
	}
	bool daDreidel::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
		DamagePlayer(this, apThis, apOther);
		return true;
	}
	bool daDreidel::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
		DamagePlayer(this, apThis, apOther);
		return true;
	}

	bool daDreidel::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (this->isInvulnerable == 0) {
			doStateChange(&StateID_KnockBack);
		}
		return true;
	}
	bool daDreidel::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther){
		if (this->isInvulnerable == 0) {
			doStateChange(&StateID_KnockBack);
		}
		return true;
	}


	bool daDreidel::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther){ return true; }
	bool daDreidel::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) { return true; }
	bool daDreidel::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther){ return true; }
	bool daDreidel::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) { return true; }
	bool daDreidel::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) { return true; }

void daDreidel::powBlockActivated(bool isNotMPGP) { }


bool daDreidel::calculateTileCollisions() {
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

	float xDelta = pos.x - last_pos.x;
	if (xDelta >= 0.0f)
		direction = 0;
	else
		direction = 1;

	if (collMgr.isOnTopOfTile()) {
		// Walking into a tile branch

		if (cmgr_returnValue == 0)
			isBouncing = true;

		if (speed.x != 0.0f) {
			//playWmEnIronEffect();
		}

		speed.y = 0.0f;

		// u32 blah = collMgr.s_80070760();
		// u8 one = (blah & 0xFF);
		// static const float incs[5] = {0.00390625f, 0.0078125f, 0.015625f, 0.0234375f, 0.03125f};
		// x_speed_inc = incs[one];
		max_speed.x = (direction == 1) ? -1.0f : 1.0f;
	} else {
		x_speed_inc = 0.0f;
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
		return true;
	}
	return false;
}

void daDreidel::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->chrAnimation.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->chrAnimation, unk2);
	this->chrAnimation.setUpdateRate(rate);
}

int daDreidel::onCreate() {

	// Model creation
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->resFile.data = getResource("topman", "g3d/begoman_spike.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("begoman");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Map(&bodyModel, 0);


	// Animations start here
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("begoman_wait");
	this->chrAnimation.setup(mdl, anmChr, &this->allocator, 0);

	allocator.unlink();

	// Stuff I do understand
	this->scale = (Vec){0.2, 0.2, 0.2};

	// this->pos.y = this->pos.y + 30.0; // X is vertical axis
	this->rot.x = 0; // X is vertical axis
	this->rot.y = 0xD800; // Y is horizontal axis
	this->rot.z = 0; // Z is ... an axis >.>
	this->direction = 1; // Heading left.

	this->speed.x = 0.0;
	this->speed.y = 0.0;
	this->max_speed.x = 1.1;
	this->x_speed_inc = 0.0;
	this->XSpeed = 1.1;

	this->isInSpace = this->settings & 0xF;
	this->fromBehind = 0;
	this->flashing = 0;


	ActivePhysics::Info HitMeBaby;

	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = 24.0;

	HitMeBaby.xDistToEdge = 28.0;
	HitMeBaby.yDistToEdge = 24.0;

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
	spriteSomeRectX = 28.0f;
	spriteSomeRectY = 32.0f;
	_320 = 0.0f;
	_324 = 16.0f;

	// These structs tell stupid collider what to collide with - these are from koopa troopa
	static const lineSensor_s below(12<<12, 4<<12, 0<<12);
	static const pointSensor_s above(0<<12, 12<<12);
	static const lineSensor_s adjacent(6<<12, 9<<12, 6<<12);

	collMgr.init(this, &below, &above, &adjacent);
	collMgr.calculateBelowCollisionWithSmokeEffect();

	cmgr_returnValue = collMgr.isOnTopOfTile();

	if (collMgr.isOnTopOfTile())
		isBouncing = false;
	else
		isBouncing = true;


	// State Changers
	bindAnimChr_and_setUpdateRate("begoman_wait2", 1, 0.0, 1.0);
	doStateChange(&StateID_Grow);

	this->onExecute();
	return true;
}

int daDreidel::onDelete() {
	return true;
}

int daDreidel::onExecute() {
	acState.execute();
	updateModelMatrices();

	return true;
}

int daDreidel::onDraw() {

	if (this->isInvulnerable == 1) {
		this->flashing++;
	}

	if (this->flashing < 5) {
		bodyModel.scheduleForDrawing();
	}

	bodyModel._vf1C();

	if (this->flashing > 8) { this->flashing = 0; }

	return true;
}

void daDreidel::updateModelMatrices() {
	matrix.translation(pos.x, pos.y - 2.0, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}



///////////////
// Grow State
///////////////
	void daDreidel::beginState_Grow() {
		bindAnimChr_and_setUpdateRate("begoman_wait2", 1, 0.0, 0.75);

		this->timer = 0;

		SetupKameck(this, Kameck);
	}

	void daDreidel::executeState_Grow() {

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		this->timer += 1;

		bool ret;
		ret = GrowBoss(this, Kameck, 0.2, 0.4, 0, this->timer);

		if (ret) {
			PlaySound(this, SE_EMY_MECHAKOOPA_BOUND);
			doStateChange(&StateID_Walk);
		}
	}
	void daDreidel::endState_Grow() {
		this->chrAnimation.setUpdateRate(1.0);
		CleanupKameck(this, Kameck);
	}


///////////////
// Walk State
///////////////
	void daDreidel::beginState_Walk() {
		this->max_speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
		this->speed.x = (direction) ? -1.2f : 1.2f;

		this->max_speed.y = (this->isInSpace) ? -2.0 : -4.0;
		this->speed.y = 	(this->isInSpace) ? -2.0 : -4.0;
		this->y_speed_inc = (this->isInSpace) ? -0.09375 : -0.1875;

		this->isTurningCountdown = 0;
	}
	void daDreidel::executeState_Walk() {

		if (this->isInvulnerableCountdown > 0) {
			this->isInvulnerableCountdown--;
		}
		else {
			this->isInvulnerable = 0;
			this->flashing = 0;
		}

		PlaySound(this, SE_BOSS_JR_CROWN_JR_RIDE); // 5

		bool ret = calculateTileCollisions();
		if (ret) {
			doStateChange(&StateID_Turn);
		}

		u8 dir = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);
		if (dir != this->direction) {
			this->isTurningCountdown++;
		}

		if (this->isTurningCountdown > 60) { doStateChange(&StateID_Turn); }

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}
	}
	void daDreidel::endState_Walk() { this->timer += 1; }


///////////////
// Turn State
///////////////
	void daDreidel::beginState_Turn() {
		this->direction ^= 1;
		this->speed.x = 0.0;
	}
	void daDreidel::executeState_Turn() {

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		u16 amt = (this->direction == 0) ? 0x2800 : 0xD800;
		int done = SmoothRotation(&this->rot.y, amt, 0x800);

		if(done) {
			if (this->damage > 0) 	{ doStateChange(&StateID_ChargePrep); }
			else 					{ doStateChange(&StateID_Walk); }
		}
	}
	void daDreidel::endState_Turn() { this->rot.y = (this->direction) ? 0xD800 : 0x2800; }


///////////////
// Knockback State
///////////////
	void daDreidel::beginState_KnockBack() {
		bindAnimChr_and_setUpdateRate("begoman_damage", 1, 0.0, 0.65);

		this->max_speed.x = (this->direction) ? 6.5f : -6.5f;
		this->speed.x = (this->direction) ? 6.5f : -6.5f;
	}
	void daDreidel::executeState_KnockBack() {

		bool ret = calculateTileCollisions();
		if (ret) {
			this->max_speed.x = -this->max_speed.x;
			this->speed.x = -this->speed.x;
		}
		this->speed.x = this->speed.x / 1.08;

		bodyModel._vf1C();
		if(this->chrAnimation.isAnimationDone()) {
			if (this->damage > 0) 	{ doStateChange(&StateID_ChargePrep); }
			else 					{ doStateChange(&StateID_Walk); }
		}

	}
	void daDreidel::endState_KnockBack() {
		if (this->rot.y == 0x2800) {
			// CreateEffect(&this->pos, &(S16Vec){0,0,0}, &(Vec){1.0, 1.0, 1.0}, 175);
			this->direction = 0;
		}
		else {
			// CreateEffect(&this->pos, &(S16Vec){0,0,0}, &(Vec){1.0, 1.0, 1.0}, 192);
			this->direction = 1;
		}
		// this->direction ^= 1;
		bindAnimChr_and_setUpdateRate("begoman_wait2", 1, 0.0, 1.0);
	}


///////////////
// ChargePrep State
///////////////
	void daDreidel::beginState_ChargePrep() {
		bindAnimChr_and_setUpdateRate("begoman_attack", 1, 0.0, 0.9);
	}
	void daDreidel::executeState_ChargePrep() {
		if(this->chrAnimation.isAnimationDone()) {
			doStateChange(&StateID_Charge);
		}
	}
	void daDreidel::endState_ChargePrep() { }


///////////////
// Charge State
///////////////
	void daDreidel::beginState_Charge() {
		bindAnimChr_and_setUpdateRate("begoman_attack3", 1, 0.0, 1.0);
		this->timer = 0;
		this->isTurningCountdown = 0;

		this->max_speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
		this->speed.x = (direction) ? -2.6f : 2.6f;

		this->max_speed.y = (this->isInSpace) ? -2.0 : -4.0;
		this->speed.y = 	(this->isInSpace) ? -2.0 : -4.0;
		this->y_speed_inc = (this->isInSpace) ? -0.09375 : -0.1875;

		this->charging = 1;
	}
	void daDreidel::executeState_Charge() {

		if (this->isInvulnerableCountdown > 0) {
			this->isInvulnerableCountdown--;
		}
		else {
			this->isInvulnerable = 0;
			this->flashing = 0;
		}

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		bool ret = calculateTileCollisions();
		if (ret) {
			doStateChange(&StateID_ChargeRecover);
		}

		u8 dir = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);
		if (dir != this->direction) {
			this->isTurningCountdown++;
			this->speed.x = this->speed.x / 1.04;
		}

		if (this->isTurningCountdown > 90) { doStateChange(&StateID_Turn); }
	}
	void daDreidel::endState_Charge() {
		this->charging = 0;

		this->counter_504[0] = 0;
		this->counter_504[1] = 0;
		this->counter_504[2] = 0;
		this->counter_504[3] = 0;
	}


///////////////
// ChargeRecover State
///////////////
	void daDreidel::beginState_ChargeRecover() {
		bindAnimChr_and_setUpdateRate("begoman_stand", 1, 0.0, 0.5);
	}
	void daDreidel::executeState_ChargeRecover() {
		if(this->chrAnimation.isAnimationDone()) {
			doStateChange(&StateID_Turn);
		}
	}
	void daDreidel::endState_ChargeRecover() { }


///////////////
// Damage State
///////////////
	void daDreidel::beginState_Damage() {
		this->isInvulnerable = 1;
		bindAnimChr_and_setUpdateRate("begoman_wait", 1, 0.0, 0.75);
		this->timer = 0;

		PlaySound(this, SE_PLY_TOUCH_BIRI);
		PlaySound(this, SE_BOSS_JR_ELEC_SPARK);
		PlaySound(this, SE_EMY_BIRIKYU_SPARK);

		Vec front = {this->pos.x-4.0f, this->pos.y+8.0f, 5500.0f};
		Vec back = {this->pos.x-4.0f, this->pos.y+8.0f, -5500.0f};

		Vec sparkScale = {2.5f, 2.5f, 1.5f};
		SpawnEffect("Wm_ob_cmnspark", 0, &front, 0, &sparkScale);
		Vec lineScale = {1.5f, 1.5f, 1.5f};
		SpawnEffect("Wm_mr_wirehit_line", 0, &front, 0, &lineScale);
		Vec kickScale = {1.5f, 1.5f, 1.5f};
		SpawnEffect("Wm_mr_kick_grain", 0, &front, 0, &kickScale);
		Vec glowScale = {2.5f, 2.5f, 2.5f};
		SpawnEffect("Wm_mr_wirehit_glow", 0, &back, 0, &glowScale);
	}
	void daDreidel::executeState_Damage() {

		if(this->chrAnimation.isAnimationDone()) {
			this->timer += 1;
			if (this->timer == 1) {

				u8 dir = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);
				if (dir != this->direction) {
					doStateChange(&StateID_Turn);
				}
				else {
					if (this->damage > 1) 	{ doStateChange(&StateID_ChargePrep); }
					else 					{ doStateChange(&StateID_Walk); }
				}
			}
			else {
			}
			this->chrAnimation.setCurrentFrame(0.0);
		}
	}
	void daDreidel::endState_Damage() {
		bindAnimChr_and_setUpdateRate("begoman_wait2", 1, 0.0, 1.0);
		this->isInvulnerableCountdown = 90;
	}


///////////////
// Outro State
///////////////
	void daDreidel::beginState_Outro() {
		OutroSetup(this);
	}
	void daDreidel::executeState_Outro() {

		if (this->dying == 1) {
			if (this->timer > 180) { ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE); }
			if (this->timer == 60) { PlayerVictoryCries(this); }

			this->timer += 1;
			return;
		}

		bool ret;
		ret = ShrinkBoss(this, &this->pos, 0.5, this->timer);

		if (ret == true) 	{
			BossExplode(this, &this->pos);
			this->dying = 1;
			this->timer = 0;
		}
		else 		{ PlaySound(this, SE_EMY_BUBBLE_EXTINCT); }

		this->timer += 1;
	}
	void daDreidel::endState_Outro() { }
