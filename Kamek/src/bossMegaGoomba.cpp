#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include "boss.h"

extern "C" void *StageScreen;

const char* MGarcNameList [] = {
	"kuriboBig",
	"kuriboBoss",
	NULL	
};

class daMegaGoomba_c : public dEn_c {
	public:
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	m3d::mdl_c bodyModel;
	m3d::anmChr_c animationChr;

	float timer;
	float dying;

	lineSensor_s belowSensor;
	lineSensor_s adjacentSensor;

	ActivePhysics leftTrapAPhysics, rightTrapAPhysics;
	ActivePhysics stalkAPhysics;

	HermiteKey keysX[0x10];
	unsigned int Xkey_count;
	HermiteKey keysY[0x10];
	unsigned int Ykey_count;

	char life;
	bool already_hit;

	float XSpeed;
	float JumpHeight;
	float JumpDist;
	float JumpTime;

	char isBigBoss;
	char isPanic;

	bool takeHit(char count);

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);
	
	void dieFall_Begin();
	void dieFall_Execute();
	static daMegaGoomba_c *build();

	void setupBodyModel();
	void setupCollision();

	void updateModelMatrices();

	void stunPlayers();
	void unstunPlayers();

	bool hackOfTheCentury;

	bool playerStunned[4];

	void removeMyActivePhysics();
	void addMyActivePhysics();

	int tryHandleJumpedOn(ActivePhysics *apThis, ActivePhysics *apOther);

	void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	void addScoreWhenHit(void *other);
	bool _vf120(ActivePhysics *apThis, ActivePhysics *apOther);
	bool _vf110(ActivePhysics *apThis, ActivePhysics *apOther);
	bool _vf108(ActivePhysics *apThis, ActivePhysics *apOther);

	void powBlockActivated(bool isNotMPGP);

	void dieOther_Begin();
	void dieOther_Execute();
	void dieOther_End();

	USING_STATES(daMegaGoomba_c);
	DECLARE_STATE(Shrink);
	DECLARE_STATE(Walk);
	DECLARE_STATE(Turn);
};


daMegaGoomba_c *daMegaGoomba_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daMegaGoomba_c));
	return new(buffer) daMegaGoomba_c;
}


void daMegaGoomba_c::removeMyActivePhysics() {
	aPhysics.removeFromList();
	stalkAPhysics.removeFromList();
	leftTrapAPhysics.removeFromList();
	rightTrapAPhysics.removeFromList();
}

void daMegaGoomba_c::addMyActivePhysics() {
	aPhysics.addToList();
	stalkAPhysics.addToList();
	leftTrapAPhysics.addToList();
	rightTrapAPhysics.addToList();
}


void setNewActivePhysicsRect(daMegaGoomba_c *actor, Vec *scale) {
	float amtX = scale->x * 0.5f;
	float amtY = scale->y * 0.5f;

	actor->belowSensor.flags = SENSOR_LINE;
	actor->belowSensor.lineA = s32((amtX * -28.0f) * 4096.0f);
	actor->belowSensor.lineB = s32((amtX * 28.0f) * 4096.0f);
	actor->belowSensor.distanceFromCenter = 0;

	actor->adjacentSensor.flags = SENSOR_LINE;
	actor->adjacentSensor.lineA = s32((amtY * 4.0f) * 4096.0f);
	actor->adjacentSensor.lineB = s32((amtY * 32.0f) * 4096.0f);
	actor->adjacentSensor.distanceFromCenter = s32((amtX * 46.0f) * 4096.0f);

	u8 cat1 = 3, cat2 = 0;
	u32 bitfield1 = 0x6f, bitfield2 = 0xffbafffe;

	ActivePhysics::Info info = {
		0.0f, amtY*57.0f, amtX*20.0f, amtY*31.0f,
		cat1, cat2, bitfield1, bitfield2, 0, &dEn_c::collisionCallback};
	actor->aPhysics.initWithStruct(actor, &info);

	// Original trapezium was -12,12 to -48,48
	ActivePhysics::Info left = {
		amtX*-32.0f, amtY*55.0f, amtX*12.0f, amtY*30.0f,
		cat1, cat2, bitfield1, bitfield2, 0, &dEn_c::collisionCallback};
	actor->leftTrapAPhysics.initWithStruct(actor, &left);
	actor->leftTrapAPhysics.trpValue0 = amtX * 12.0f;
	actor->leftTrapAPhysics.trpValue1 = amtX * 12.0f;
	actor->leftTrapAPhysics.trpValue2 = amtX * -12.0f;
	actor->leftTrapAPhysics.trpValue3 = amtX * 12.0f;
	actor->leftTrapAPhysics.collisionCheckType = 3;

	ActivePhysics::Info right = {
		amtX*32.0f, amtY*55.0f, amtX*12.0f, amtY*30.0f,
		cat1, cat2, bitfield1, bitfield2, 0, &dEn_c::collisionCallback};
	actor->rightTrapAPhysics.initWithStruct(actor, &right);
	actor->rightTrapAPhysics.trpValue0 = amtX * -12.0f;
	actor->rightTrapAPhysics.trpValue1 = amtX * -12.0f;
	actor->rightTrapAPhysics.trpValue2 = amtX * -12.0f;
	actor->rightTrapAPhysics.trpValue3 = amtX * 12.0f;
	actor->rightTrapAPhysics.collisionCheckType = 3;

	ActivePhysics::Info stalk = {
		0.0f, amtY*12.0f, amtX*28.0f, amtY*12.0f,
		cat1, cat2, bitfield1, bitfield2, 0, &dEn_c::collisionCallback};
	actor->stalkAPhysics.initWithStruct(actor, &stalk);

}


//FIXME make this dEn_c->used...
extern "C" int SomeStrangeModification(dStageActor_c* actor);
extern "C" void DoStuffAndMarkDead(dStageActor_c *actor, Vec vector, float unk);
extern "C" int SmoothRotation(short* rot, u16 amt, int unk2);

void daMegaGoomba_c::powBlockActivated(bool isNotMPGP) {
}

CREATE_STATE(daMegaGoomba_c, Shrink);
CREATE_STATE(daMegaGoomba_c, Walk);
CREATE_STATE(daMegaGoomba_c, Turn);


//TODO better fix for possible bug with sign (ex. life=120; count=-9;)
bool daMegaGoomba_c::takeHit(char count) {
	OSReport("Taking a hit!\n");
	if(!this->already_hit) {
		int c = count;
		int l = this->life;
		if(l - c > 127) {
			c = 127 - l;
		}
		this->life -= c;
		// this->XSpeed += 0.10;

		// float rate = this->animationChr.getUpdateRate();
		// this->animationChr.setUpdateRate(rate+0.05);
		this->JumpHeight += 12.0;
		this->JumpDist += 12.0;
		this->JumpTime += 5.0;
		doStateChange(&StateID_Shrink);
		this->already_hit = true;
	}
	return (life <= 0) ? true : false;
}

#define ACTIVATE	1
#define DEACTIVATE	0

extern "C" void *EN_LandbarrelPlayerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther);
void daMegaGoomba_c::spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	//HE'S TOO BADASS TO STOP FOR SMALLER GOOMBAS
	#if 0
		float me = apThis->firstFloatArray[3];
		if(((this->direction == 1) && (me > 0.0)) || ((this->direction == 0) && (me < 0.0))) {
			dStateBase_c* state = this->acState.getCurrentState();
			if(!state->isEqual(&StateID_Turn)) {
				doStateChange(&StateID_Turn);
			}
		}
	#endif
}
void daMegaGoomba_c::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) { 
	if (apThis == &stalkAPhysics) {
		dEn_c::playerCollision(apThis, apOther);
		return;
	}

	/* * * * * * * * * * * * * * * * * * * * *
	 * 0=normal??,1=dontHit,2=dontKill
	 * daEnBrosBase_c ::player = 0
	 * daEnBrosBase_c::yoshi   = 0
	 * daEnPipePirahna::player = 1
	 * daEnPipePirahna::yoshi  = 1
	 * daEnKuriboBase_c::player = 0
	 * daEnKuriboBase_c::yoshi  = 0
	 * daEnLargeKuribo_c::player = 0
	 * daEnLargeKuribo_c::yoshi  = 2
	 * daEnNokonoko_c::player = 0
	 * daEnNokonoko_c::yoshi  = 0
	 * daEnSubBoss_c     = 2
	 *
	 * * * * * * * * * * * * * * * * * * * * */
	//FIXME rename and make part of dStageActor_c
	//unk=0 does _vfs, unk=1 does playSeCmnStep
	//char ret = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 0);

	if (tryHandleJumpedOn(apThis, apOther) == 0) {
		this->dEn_c::playerCollision(apThis, apOther);
		this->_vf220(apOther->owner);
		this->counter_504[apOther->owner->which_player] = 180;
	}
}

int daMegaGoomba_c::tryHandleJumpedOn(ActivePhysics *apThis, ActivePhysics *apOther) {
	float saveBounce = EnemyBounceValue;
	EnemyBounceValue = 5.2f;

	char ret = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 2);

	EnemyBounceValue = saveBounce;

	if(ret == 1 || ret == 3) {
		apOther->someFlagByte |= 2;
		if(this->takeHit(1)) {
			// kill me
			VEC2 eSpeed = {speed.x, speed.y};
			killWithSpecifiedState(apOther->owner, &eSpeed, &dEn_c::StateID_DieOther);
		}
	}

	return ret;
}
bool daMegaGoomba_c::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) { 
	if (this->counter_504[apOther->owner->which_player] > 0) { return false; }
	VEC2 eSpeed = {speed.x, speed.y};
	killWithSpecifiedState(apOther->owner, &eSpeed, &dEn_c::StateID_DieOther);
	return true;
}
bool daMegaGoomba_c::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) { 
	return collisionCat7_GroundPound(apThis, apOther);
}

bool daMegaGoomba_c::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
}
bool daMegaGoomba_c::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
}
extern "C" void dAcPy_vf3F8(void* player, dEn_c* monster, int t);
bool daMegaGoomba_c::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (tryHandleJumpedOn(apThis, apOther) == 0) {
		dAcPy_vf3F8(apOther->owner, this, 3);
		this->counter_504[apOther->owner->which_player] = 0xA;
	}
	return true;
}
bool daMegaGoomba_c::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
	if(this->takeHit(1))
		doStateChange(&StateID_DieFall);
	return true;
}
bool daMegaGoomba_c::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther) {
	if(this->takeHit(1))
		doStateChange(&StateID_DieFall);
	return true;
}
bool daMegaGoomba_c::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) {
	return collisionCat7_GroundPound(apThis, apOther);
}
bool daMegaGoomba_c::collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daMegaGoomba_c::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daMegaGoomba_c::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther) {
	if(this->takeHit(1))
		doStateChange(&StateID_DieFall);
	return true;
}
void daMegaGoomba_c::addScoreWhenHit(void *other) {}
bool daMegaGoomba_c::_vf120(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true; // Replicate existing broken behaviour
}
bool daMegaGoomba_c::_vf110(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true; // Replicate existing broken behaviour
}
bool daMegaGoomba_c::_vf108(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true; // Replicate existing broken behaviour
}

void daMegaGoomba_c::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->animationChr.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->animationChr, unk2);
	this->animationChr.setUpdateRate(rate);
}

void daMegaGoomba_c::dieFall_Begin() {
	this->dEn_c::dieFall_Begin();
	PlaySound(this, SE_EMY_KURIBO_L_DAMAGE_03);
}
void daMegaGoomba_c::dieFall_Execute() {
	
	this->timer = this->timer + 1.0;
	
	this->dying = this->dying + 0.15;
	
	this->pos.x = this->pos.x + 0.15;
	this->pos.y = this->pos.y + ((-0.2 * (this->dying*this->dying)) + 5);
	
	this->dEn_c::dieFall_Execute();
}

void daMegaGoomba_c::setupBodyModel() {
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->resFile.data = getResource("kuriboBoss", "g3d/kuriboBoss.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("kuriboBig");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);

	bool ret;
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("walk");
	ret = this->animationChr.setup(mdl, anmChr, &this->allocator, 0);
	this->bindAnimChr_and_setUpdateRate("walk", 1, 0.0, 0.2);

	allocator.unlink();
}

void daMegaGoomba_c::setupCollision() {
	//POINTLESS WITH GROWTH
	this->scale.x = this->scale.y = this->scale.z = 0.666;

	this->collMgr.init(this, &belowSensor, 0, &adjacentSensor);

	char foo = this->appearsOnBackFence;
	this->pos_delta2.x = 0.0;
	this->pos_delta2.y = 16.0;
	this->pos_delta2.z = 0.0;

	this->pos.z = (foo == 0) ? 1500.0 : -2500.0;

	this->_518 = 2;

	//NOT NEEDED
	//this->doStateChange(&StateID_Walk);
}

int daMegaGoomba_c::onCreate() {
	/*80033230 daEnLkuribo_c::onCreate()*/
	this->setupBodyModel();
	this->max_speed.y = -4.0;
	this->direction = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);
	this->rot.y = (this->direction) ? 0xE000 : 0x2000;
	this->_518 = 2;

	isBigBoss = this->settings & 0xF;
	this->animationChr.setCurrentFrame(69.0);

	aPhysics.addToList();
	stalkAPhysics.addToList();
	leftTrapAPhysics.addToList();
	rightTrapAPhysics.addToList();

	this->_120 |= 0x200;

	this->_36D = 0;
	this->setupCollision();

	//HOMEMADE//
	speed.y = 0.0;
	dying = 0.0;
	rot.x = rot.z = 0;
	life = 3;
	already_hit = false;
	this->x_speed_inc = 0.1;
	this->pos.y -= 16.0;

	// 2.0 is good final speed
	this->XSpeed = 0.2;
	this->JumpHeight = 48.0;
	this->JumpDist = 64.0;
	this->JumpTime = 50.0;

	// doStateChange(&StateID_Grow);

	scale.x = 4.0f;
	scale.y = 4.0f;
	scale.z = 4.0f;
	setNewActivePhysicsRect(this, &this->scale);
	doStateChange(&StateID_Walk);

	this->onExecute();
	return true;
}

int daMegaGoomba_c::onDelete() {
	unstunPlayers();
	return true;
}

int daMegaGoomba_c::onExecute() {
	//80033450
	acState.execute();
	if (!hackOfTheCentury) {
		hackOfTheCentury = true;
	} else {
		checkZoneBoundaries(0);
	}
	updateModelMatrices();

	return true;
}

int daMegaGoomba_c::onDraw() {
	bodyModel.scheduleForDrawing();
	return true;
}


void daMegaGoomba_c::updateModelMatrices() {
	// This won't work with wrap because I'm lazy.
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}



// Shrink State
void daMegaGoomba_c::beginState_Shrink() {
	this->timer = 1.0;
	Xkey_count = 4;
	keysX[0] = (HermiteKey){  0.0f, this->scale.y,         0.5f };
	keysX[1] = (HermiteKey){ 10.0f, this->scale.y - 0.75f, 0.5f };
	keysX[2] = (HermiteKey){ 20.0f, this->scale.y - 0.35f, 0.5f };
	keysX[3] = (HermiteKey){ 39.0f, this->scale.y - 0.75f, 0.5f };

	// disable being hit
	Vec tempVec = (Vec){0.0, 0.0, 0.0};
	setNewActivePhysicsRect(this,  &tempVec );
}
void daMegaGoomba_c::executeState_Shrink() { 
	this->timer += 1.0;
	
	float modifier = GetHermiteCurveValue(this->timer, this->keysX, Xkey_count);
	this->scale = (Vec){modifier, modifier, modifier};

	if(this->timer == 2.0)
		PlaySound(this, SE_EMY_KURIBO_L_DAMAGE_02);

	if (this->timer > 40.0) { doStateChange(&StateID_Walk); }
}
void daMegaGoomba_c::endState_Shrink() {
	// enable being hit
	setNewActivePhysicsRect(this, &this->scale);
	this->already_hit = false;
}



// Turn State
void daMegaGoomba_c::beginState_Turn() {
	this->direction ^= 1;
	this->speed.x = 0.0;
}
void daMegaGoomba_c::executeState_Turn() { 
	this->bodyModel._vf1C();

	this->HandleYSpeed();
	this->doSpriteMovement();

	/*this->_vf2D0();	//nullsub();*/
	int ret = SomeStrangeModification(this);

	if(ret & 1)
		this->speed.y = 0.0;
	if(ret & 4)
		this->pos.x = this->last_pos.x;
	DoStuffAndMarkDead(this, this->pos, 1.0);
	u16 amt = (this->direction == 0) ? 0x2000 : 0xE000;
	int done = SmoothRotation(&this->rot.y, amt, 0x80);
	if(done) {
		this->doStateChange(&StateID_Walk);
	}

	int frame = (int)(this->animationChr.getCurrentFrame() * 5.0);
	if ((frame == 100) || (frame == 325) || (frame == 550) || (frame == 775)) {
		ShakeScreen(StageScreen, 0, 1, 0, 0);
		stunPlayers();
		PlaySound(this, SE_BOSS_MORTON_GROUND_SHAKE);
	}

	if (isBigBoss) {
		if ((frame == 250) || (frame == 500) || (frame == 700) || (frame == 900))
			unstunPlayers();
	}
	else {
		if ((frame == 200) || (frame == 425) || (frame == 650) || (frame == 875))
			unstunPlayers();
	}
}
void daMegaGoomba_c::endState_Turn() {
	this->max_speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
}


// Walk State
void daMegaGoomba_c::beginState_Walk() {
	//inline this piece of code
	//YOU SUCK, WHOEVER ADDED THIS LINE OF CODE AND MADE ME SPEND AGES
	//HUNTING DOWN WHAT WAS BREAKING TURNING. -Treeki
	//this->direction = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);
	this->speed.x = this->speed.z = 0.0;
	this->max_speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
	this->speed.y = -4.0;
	this->y_speed_inc = -0.1875;
}
void daMegaGoomba_c::executeState_Walk() { 
	/* 800345e0 - daEnLkuribo_c::executeState_Walk() */
	this->bodyModel._vf1C();
	//HOMEMADE//
	this->HandleXSpeed();
	this->HandleYSpeed();
	this->doSpriteMovement();
	u16 amt = (this->direction == 0) ? 0x2000 : 0xE000;
	SmoothRotation(&this->rot.y, amt, 0x200);
	/*this->_vf2D0();	//nullsub();*/
	int ret = SomeStrangeModification(this);
	if(ret & 1)
		this->speed.y = 0.0;
	u32 bitfield = this->collMgr.outputMaybe;
	if(bitfield & (0x15<<this->direction)) {
		this->pos.x = this->last_pos.x;
		this->doStateChange(&StateID_Turn);
		//this->acState.setField10ToOne();
	}
	/*u32 bitfield2 = this->collMgr.adjacentTileProps[this->direction];
	if(bitfield2) {
		this->doStateChange(&StateID_Turn);
	}*/
	DoStuffAndMarkDead(this, this->pos, 1.0);


	int frame = (int)(this->animationChr.getCurrentFrame() * 5.0);
	if ((frame == 100) || (frame == 325) || (frame == 550) || (frame == 775)) {
		ShakeScreen(StageScreen, 0, 1, 0, 0);
		stunPlayers();
		PlaySound(this, SE_BOSS_MORTON_GROUND_SHAKE);
	}

	if (isBigBoss) {
		if ((frame == 250) || (frame == 500) || (frame == 700) || (frame == 900))
			unstunPlayers();
	}
	else {
		if ((frame == 200) || (frame == 425) || (frame == 650) || (frame == 875))
			unstunPlayers();
	}

	if(this->animationChr.isAnimationDone()) {
		this->animationChr.setCurrentFrame(0.0);

		int new_dir = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, pos);
		if(this->direction != new_dir)
			doStateChange(&StateID_Turn);
	}
}
void daMegaGoomba_c::endState_Walk() { }





extern "C" void stunPlayer(void *, int);
extern "C" void unstunPlayer(void *);

void daMegaGoomba_c::stunPlayers() {
	for (int i = 0; i < 4; i++) {
		playerStunned[i] = false;

		dStageActor_c *player = GetSpecificPlayerActor(i);
		if (player) {
			if (player->collMgr.isOnTopOfTile() && player->currentZoneID == currentZoneID) {
				stunPlayer(player, 1);
				playerStunned[i] = true;
			}
		}
	}
}

void daMegaGoomba_c::unstunPlayers() {
	for (int i = 0; i < 4; i++) {
		dStageActor_c *player = GetSpecificPlayerActor(i);
		if (player && playerStunned[i]) {
			unstunPlayer(player);
		}
	}
}



void daMegaGoomba_c::dieOther_Begin() {
	animationChr.bind(&bodyModel, resFile.GetResAnmChr("damage"), true);
	bodyModel.bindAnim(&animationChr, 2.0f);
	speed.x = speed.y = speed.z = 0.0f;
	removeMyActivePhysics();

	PlaySound(this, SE_EMY_KURIBO_L_SPLIT_HPDP);

	rot.y = 0;
	counter_500 = 60;
}

void daMegaGoomba_c::dieOther_End() {
	dEn_c::dieOther_End();
}

void daMegaGoomba_c::dieOther_Execute() {
	bodyModel._vf1C();
	if (counter_500 == 0) {
		S16Vec nullRot = {0,0,0};
		Vec vecFive = {5.0f, 5.0f, 5.0f};
		SpawnEffect("Wm_ob_icebreaksmk", 0, &pos, &nullRot, &vecFive);
		Delete(1);
	}
}

