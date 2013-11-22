#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>

class daPengi : public dEn_c {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	nw4r::g3d::ResFile anmFile;

	m3d::mdl_c bodyModel;

	m3d::anmChr_c chrAnimation;

	int timer;
	char damage;
	char isDown;
	float XSpeed;
	u32 cmgr_returnValue;
	bool isBouncing;

	static daPengi *build();

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);
	void updateModelMatrices();
	bool calculateTileCollisions();

	// void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	void collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther);
	void collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	void collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther);
	void collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	void collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	void collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	void collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	// bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	void collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	void collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);

	void _vf148();
	void _vf14C();

	USING_STATES(daPengi);
	DECLARE_STATE(Walk);
	DECLARE_STATE(Turn);
	DECLARE_STATE(Die);
};

daPengi *daPengi::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daPengi));
	return new(buffer) daPengi;
}

///////////////////////
// Externs and States
///////////////////////

	//FIXME make this dEn_c->used...
	extern "C" char usedForDeterminingStatePress_or_playerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther, int unk1);
	extern "C" int SmoothRotation(short* rot, u16 amt, int unk2);


	CREATE_STATE(daPengi, Walk);
	CREATE_STATE(daPengi, Turn);
	CREATE_STATE(daPengi, Die);

////////////////////////
// Collision Functions
////////////////////////

	// Collision callback to help shy guy not die at inappropriate times and ruin the dinner

	void daPengi::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {

		char hitType;
		hitType = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 0);

		if(hitType == 1) {	// regular jump
			apOther->someFlagByte |= 2;
			doStateChange(&StateID_Die);
		} 
		else if(hitType == 3) {	// spinning jump or whatever?
			apOther->someFlagByte |= 2;
			doStateChange(&StateID_Die);
		} 
		else if(hitType == 0) {
			this->dEn_c::playerCollision(apThis, apOther);
			this->_vf220(apOther->owner);
		} 

		// fix multiple player collisions via megazig
		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
		if(apOther->owner->which_player < 4) {
			this->counter_504[apOther->owner->which_player] = 0;
		}
	}

	void daPengi::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->playerCollision(apThis, apOther);
	}

	void daPengi::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) {
		doStateChange(&StateID_Die);
	}

	void daPengi::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
		doStateChange(&StateID_Die);
	}

	void daPengi::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
		doStateChange(&StateID_Die);
	}

	void daPengi::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
		doStateChange(&StateID_Die);
	}

	void daPengi::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther){
		doStateChange(&StateID_Die);
	}

	void daPengi::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther){
		doStateChange(&StateID_Die);
	}

	void daPengi::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther){
		doStateChange(&StateID_DieSmoke);
	}

	void daPengi::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
		doStateChange(&StateID_DieSmoke);
	}

	// void daPengi::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
	// 	doStateChange(&StateID_DieFall);
	// }

	// These handle the ice crap
	void daPengi::_vf148() {
		dEn_c::_vf148();
		doStateChange(&StateID_Die);
	}
	void daPengi::_vf14C() {
		dEn_c::_vf14C();
		doStateChange(&StateID_Die);
	}

	void daPengi::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
		doStateChange(&StateID_Die);
	}

bool daPengi::calculateTileCollisions() {
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

void daPengi::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->anmFile.GetResAnmChr(name);
	this->chrAnimation.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->chrAnimation, unk2);
	this->chrAnimation.setUpdateRate(rate);
}

int daPengi::onCreate() {
	
	// Model creation	
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->resFile.data = getResource("penguin2", "g3d/finalpenguin.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("npc011_m1");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);


	// Animations start here
	this->anmFile.data = getResource("penguin2", "g3d/pengiAnm.brres");
	nw4r::g3d::ResAnmChr anmChr = this->anmFile.GetResAnmChr("npc011_idle");
	this->chrAnimation.setup(mdl, anmChr, &this->allocator, 0);

	allocator.unlink();

	// Stuff I do understand
	this->scale = (Vec){2.0, 2.0, 2.0};

	// this->pos.y = this->pos.y + 30.0; // X is vertical axis
	this->rot.x = 0; // X is vertical axis
	this->rot.y = 0xD800; // Y is horizontal axis
	this->rot.z = 0; // Z is ... an axis >.>
	this->direction = 1; // Heading left.
	
	this->speed.x = 0.0;
	this->speed.y = 0.0;
	this->max_speed.x = 0.8;
	this->x_speed_inc = 0.2;
	this->XSpeed = 0.8;



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
	
	doStateChange(&StateID_Walk);

	this->onExecute();
	return true;
}

int daPengi::onDelete() {
	return true;
}

int daPengi::onExecute() {
	acState.execute();
	updateModelMatrices();
	
	return true;
}

int daPengi::onDraw() {
	bodyModel.scheduleForDrawing();
	bodyModel._vf1C();

	return true;
}

void daPengi::updateModelMatrices() {
	matrix.translation(pos.x, pos.y - 2.0, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


///////////////
// Walk State
///////////////
	void daPengi::beginState_Walk() {
		//inline this piece of code
		bindAnimChr_and_setUpdateRate("npc011_walk", 1, 0.0, 1.5); 
		this->max_speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
		this->speed.x = (direction) ? -0.8f : 0.8f;

		this->max_speed.y = -4.0;
		this->speed.y = -4.0;
		this->y_speed_inc = -0.1875;
	}
	void daPengi::executeState_Walk() { 

		bool ret = calculateTileCollisions();
		if (ret) {
			doStateChange(&StateID_Turn);
		}

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}
	}
	void daPengi::endState_Walk() { }

///////////////
// Turn State
///////////////
	void daPengi::beginState_Turn() {
		bindAnimChr_and_setUpdateRate("npc011_turn", 1, 0.0, 1.0);

		this->direction ^= 1;
		this->speed.x = 0.0;
	}
	void daPengi::executeState_Turn() { 

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		u16 amt = (this->direction == 0) ? 0x2800 : 0xD800;
		int done = SmoothRotation(&this->rot.y, amt, 0x800);

		if(done) {
			this->doStateChange(&StateID_Walk);
		}
	}
	void daPengi::endState_Turn() { }



///////////////
// Die State
///////////////
	void daPengi::beginState_Die() {
		dEn_c::dieFall_Begin();

		bindAnimChr_and_setUpdateRate("npc011_damage", 1, 0.0, 1.0); 
		this->timer = 0;
	}
	void daPengi::executeState_Die() { 

		if(this->chrAnimation.isAnimationDone()) {
			this->kill();
			this->Delete(1);
		}
	}
	void daPengi::endState_Die() { }
