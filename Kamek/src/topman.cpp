#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>

const char* TMarcNameList [] = {
	"topman",
	NULL	
};

class daTopman : public dEn_c {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;

	m3d::mdl_c bodyModel;

	m3d::anmChr_c chrAnimation;

	int timer;
	char damage;
	char isDown;
	float XSpeed;
	u32 cmgr_returnValue;
	bool isBouncing;
	char isInSpace;
	char fromBehind;
	char isWaiting;
	char backFire;
	int directionStore;

	static daTopman *build();

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);
	void updateModelMatrices();
	bool calculateTileCollisions();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	// bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);

	void _vf148();
	void _vf14C();
	bool CreateIceActors();
	void addScoreWhenHit(void *other);

	USING_STATES(daTopman);
	DECLARE_STATE(Walk);
	DECLARE_STATE(Turn);
	DECLARE_STATE(Wait);
	DECLARE_STATE(KnockBack);
	DECLARE_STATE(Die);
};

daTopman *daTopman::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daTopman));
	return new(buffer) daTopman;
}

///////////////////////
// Externs and States
///////////////////////
	extern "C" void *EN_LandbarrelPlayerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther);

	//FIXME make this dEn_c->used...
	extern "C" char usedForDeterminingStatePress_or_playerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther, int unk1);
	extern "C" int SmoothRotation(short* rot, u16 amt, int unk2);


	CREATE_STATE(daTopman, Walk);
	CREATE_STATE(daTopman, Turn);
	CREATE_STATE(daTopman, Wait);
	CREATE_STATE(daTopman, KnockBack);
	CREATE_STATE(daTopman, Die);

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

	// Collision callback to help shy guy not die at inappropriate times and ruin the dinner

	void daTopman::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {

		char hitType;
		hitType = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 0);

		if(hitType == 1 || hitType == 2) {	// regular jump or mini jump
			this->_vf220(apOther->owner);
		} 
		else if(hitType == 3) {	// spinning jump or whatever?
			this->_vf220(apOther->owner);
		} 
		else if(hitType == 0) {
			EN_LandbarrelPlayerCollision(this, apThis, apOther);
			if (this->pos.x > apOther->owner->pos.x) {
				this->backFire = 1;
			}
			else {
				this->backFire = 0;
			}
			doStateChange(&StateID_KnockBack);
		} 

		// fix multiple player collisions via megazig
		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
		this->counter_504[apOther->owner->which_player] = 0;
	}

	void daTopman::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->playerCollision(apThis, apOther);
	}

	bool daTopman::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->dEn_c::playerCollision(apThis, apOther);
		this->_vf220(apOther->owner);

		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
		this->counter_504[apOther->owner->which_player] = 0;
		return true;
	}

	bool daTopman::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->collisionCatD_Drill(apThis, apOther);
		return true;
	}

	bool daTopman::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->collisionCatD_Drill(apThis, apOther);
		return true;
	}

	bool daTopman::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
		backFire = apOther->owner->direction ^ 1;
		// doStateChange(&StateID_KnockBack);
		doStateChange(&StateID_Die);
		return true;
	}

	bool daTopman::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther){
		doStateChange(&StateID_Die);
		return true;
	}

	bool daTopman::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
		doStateChange(&StateID_Die);
		return true;
	}

	bool daTopman::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther){
		backFire = apOther->owner->direction ^ 1;
		doStateChange(&StateID_KnockBack);
		return true;
	}

	bool daTopman::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther){
		backFire = apOther->owner->direction ^ 1;
		doStateChange(&StateID_KnockBack);
		return true;
	}

	bool daTopman::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
		backFire = apOther->owner->direction ^ 1;
		doStateChange(&StateID_KnockBack);
		return true;
	}

	// void daTopman::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
	// 	doStateChange(&StateID_DieFall);
	// }

	// These handle the ice crap
	void daTopman::_vf148() {
		dEn_c::_vf148();
		doStateChange(&StateID_Die);
	}
	void daTopman::_vf14C() {
		dEn_c::_vf14C();
		doStateChange(&StateID_Die);
	}

	DoSomethingCool my_struct;

	extern "C" void sub_80024C20(void);
	extern "C" void __destroy_arr(void*, void(*)(void), int, int);
	//extern "C" __destroy_arr(struct DoSomethingCool, void(*)(void), int cnt, int bar);

	bool daTopman::CreateIceActors()
	{
	    struct DoSomethingCool my_struct = { 0, this->pos, {2.5, 2.5, 2.5}, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	    this->frzMgr.Create_ICEACTORs( (void*)&my_struct, 1 );
	    __destroy_arr( (void*)&my_struct, sub_80024C20, 0x3C, 1 );
	    return true;
	}

	void daTopman::addScoreWhenHit(void *other) {}


bool daTopman::calculateTileCollisions() {
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
		max_speed.x = (direction == 1) ? -0.8f : 0.8f;
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

void daTopman::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->chrAnimation.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->chrAnimation, unk2);
	this->chrAnimation.setUpdateRate(rate);
}

int daTopman::onCreate() {

	this->deleteForever = true;
	
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
	this->max_speed.x = 0.8;
	this->x_speed_inc = 0.0;
	this->XSpeed = 0.8;

	this->isInSpace = this->settings & 0xF;
	this->isWaiting = (this->settings >> 4) & 0xF;
	this->fromBehind = 0;

	ActivePhysics::Info HitMeBaby;

	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = 12.0;

	HitMeBaby.xDistToEdge = 14.0;
	HitMeBaby.yDistToEdge = 12.0;		

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

	static const lineSensor_s below(12<<12, 4<<12, 0<<12);
	static const pointSensor_s above(0<<12, 12<<12);
	static const lineSensor_s adjacent(6<<12, 9<<12, 14<<12);

	collMgr.init(this, &below, &above, &adjacent);
	collMgr.calculateBelowCollisionWithSmokeEffect();

	cmgr_returnValue = collMgr.isOnTopOfTile();

	if (collMgr.isOnTopOfTile())
		isBouncing = false;
	else
		isBouncing = true;


	// State Changers
	bindAnimChr_and_setUpdateRate("begoman_wait2", 1, 0.0, 1.0); 
	if (this->isWaiting == 0) {
		doStateChange(&StateID_Walk); }
	else {
		doStateChange(&StateID_Wait); }

	this->onExecute();
	return true;
}

int daTopman::onDelete() {
	return true;
}

int daTopman::onExecute() {
	acState.execute();
	updateModelMatrices();
	
	float rect[] = {0.0, 0.0, 38.0, 38.0};
	int ret = this->outOfZone(this->pos, (float*)&rect, this->currentZoneID);
	if(ret) {
		this->Delete(1);
	}
	return true;
}

int daTopman::onDraw() {
	bodyModel.scheduleForDrawing();

	return true;
}

void daTopman::updateModelMatrices() {
	matrix.translation(pos.x, pos.y - 2.0, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


///////////////
// Walk State
///////////////
	void daTopman::beginState_Walk() {
		this->max_speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
		this->speed.x = (direction) ? -0.8f : 0.8f;

		this->max_speed.y = (this->isInSpace) ? -2.0 : -4.0;
		this->speed.y = 	(this->isInSpace) ? -2.0 : -4.0;
		this->y_speed_inc = (this->isInSpace) ? -0.09375 : -0.1875;
	}
	void daTopman::executeState_Walk() { 

		if (!this->isOutOfView()) {
			nw4r::snd::SoundHandle *handle = PlaySound(this, SE_BOSS_JR_CROWN_JR_RIDE);
			if (handle)
				handle->SetVolume(0.5f, 0); 
		}
	
		bool ret = calculateTileCollisions();
		if (ret) {
			doStateChange(&StateID_Turn);
		}
		bodyModel._vf1C();

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}
	}
	void daTopman::endState_Walk() { this->timer += 1; }


///////////////
// Turn State
///////////////
	void daTopman::beginState_Turn() {
		this->direction ^= 1;
		this->speed.x = 0.0;
	}
	void daTopman::executeState_Turn() { 

		bodyModel._vf1C();
		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		u16 amt = (this->direction == 0) ? 0x2800 : 0xD800;
		int done = SmoothRotation(&this->rot.y, amt, 0x800);

		if(done) {
			this->doStateChange(&StateID_Walk);
		}
	}
	void daTopman::endState_Turn() { }


///////////////
// Wait State
///////////////
	void daTopman::beginState_Wait() {
		this->max_speed.x = 0;
		this->speed.x = 0;

		this->max_speed.y = (this->isInSpace) ? -2.0 : -4.0;
		this->speed.y = 	(this->isInSpace) ? -2.0 : -4.0;
		this->y_speed_inc = (this->isInSpace) ? -0.09375 : -0.1875;
	}
	void daTopman::executeState_Wait() { 

		if (!this->isOutOfView()) {
			nw4r::snd::SoundHandle *handle = PlaySound(this, SE_BOSS_JR_CROWN_JR_RIDE);
			if (handle)
				handle->SetVolume(0.5f, 0); 
		}
	
		bool ret = calculateTileCollisions();
		if (ret) {
			doStateChange(&StateID_Turn);
		}

		bodyModel._vf1C();
		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}
	}
	void daTopman::endState_Wait() { }


///////////////
// Die State
///////////////
	void daTopman::beginState_Die() {
		dEn_c::dieFall_Begin();

		bindAnimChr_and_setUpdateRate("begoman_damage2", 1, 0.0, 1.0); 
		this->timer = 0;
	}
	void daTopman::executeState_Die() { 

		bodyModel._vf1C();

		PlaySound(this, SE_EMY_MECHAKOOPA_DAMAGE);
		if(this->chrAnimation.isAnimationDone()) {
			this->kill();
			this->Delete(this->deleteForever);
		}
	}
	void daTopman::endState_Die() { }


///////////////
// Knockback State
///////////////
	void daTopman::beginState_KnockBack() {
		bindAnimChr_and_setUpdateRate("begoman_damage", 1, 0.0, 0.75); 

		directionStore = direction;
		speed.x = (backFire) ? XSpeed*5.0f : XSpeed*-5.0f;
		max_speed.x = speed.x;
	}
	void daTopman::executeState_KnockBack() { 

		bool ret = calculateTileCollisions();
		this->speed.x = this->speed.x / 1.1;

		bodyModel._vf1C();
		if(this->chrAnimation.isAnimationDone()) {
			if (this->isWaiting == 0) {
				OSReport("Done being knocked back, going back to Walk state\n");
				doStateChange(&StateID_Walk); }
			else {
				OSReport("Done being knocked back, going back to Wait state\n");
				doStateChange(&StateID_Wait); }
		}

	}
	void daTopman::endState_KnockBack() { 
		direction = directionStore;
		bindAnimChr_and_setUpdateRate("begoman_wait2", 1, 0.0, 1.0); 
	}
	
