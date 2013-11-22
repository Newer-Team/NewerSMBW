#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>


const char* SGarcNameList [] = {
	"shyguy",
	"iron_ball",
	NULL
};

// Shy Guy Settings
//
// Nybble 5: Shy Guy Types
//		0 - Walker
//		1 - Pacing Walker
//		2 - Sleeper
//		3 - Jumper
// 		4 - Judo Master
// 		5 - Spike Thrower
// 		6 - Ballooneer Horizontal
// 		7 - Ballooneer Vertical
// 		8 - Ballooneer Circular
//		9 - Walking Giant
// 		10 - Pacing Giant
//
// Nybble 9: Distance Moved
//		# - Distance for Pacing Walker, Pacing Giants, and Ballooneers
//
// If I add items in the balloons....
// I_kinoko, I_fireflower, I_propeller_model, I_iceflower, I_star, I_penguin - model names
// anmChr - wait2

void shyCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther);
void ChucklesAndKnuckles(ActivePhysics *apThis, ActivePhysics *apOther);
void balloonSmack(ActivePhysics *apThis, ActivePhysics *apOther);

class daShyGuy : public dEn_c {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	nw4r::g3d::ResFile anmFile;
	nw4r::g3d::ResFile balloonFile;
	// nw4r::g3d::ResFile carryFile;

	m3d::mdl_c bodyModel;
	m3d::mdl_c balloonModel;
	m3d::mdl_c balloonModelB;
	// m3d::mdl_c carryModel;1

	m3d::anmChr_c chrAnimation;
	// m3d::anmChr_c carryAnm;

	mEf::es2 effect;

	int timer;
	int jumpCounter;
	int baln;
	float dying;
	float Baseline;
	char damage;
	char isDown;
	char renderBalloon;
	Vec initialPos;
	int distance;
	float XSpeed;
	u32 cmgr_returnValue;
	bool isBouncing;
	float balloonSize;
	char backFire;
	char spikeTurn;
	int directionStore;
	dStageActor_c *spikeA;
	dStageActor_c *spikeB;
	bool stillFalling;

	StandOnTopCollider giantRider;
	ActivePhysics Chuckles;
	ActivePhysics Knuckles;
	ActivePhysics balloonPhysics;

	static daShyGuy *build();

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);
	void updateModelMatrices();
	bool calculateTileCollisions();

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
	// bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther);

	void _vf148();
	void _vf14C();
	bool CreateIceActors();

	bool willWalkOntoSuitableGround();

	USING_STATES(daShyGuy);
	DECLARE_STATE(Walk);
	DECLARE_STATE(Turn);
	DECLARE_STATE(RealWalk);
	DECLARE_STATE(RealTurn);
	DECLARE_STATE(Jump);
	DECLARE_STATE(Sleep);
	DECLARE_STATE(Balloon_H);
	DECLARE_STATE(Balloon_V);
	DECLARE_STATE(Balloon_C);
	DECLARE_STATE(Judo);
	DECLARE_STATE(Spike);

	DECLARE_STATE(GoDizzy);
	DECLARE_STATE(BalloonDrop);
	DECLARE_STATE(FireKnockBack);
	DECLARE_STATE(FlameHit);
	DECLARE_STATE(Recover);

	DECLARE_STATE(Die);

	public: void popBalloon();
	int type;
};

daShyGuy *daShyGuy::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daShyGuy));
	return new(buffer) daShyGuy;
}

///////////////////////
// Externs and States
///////////////////////
	extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);

	//FIXME make this dEn_c->used...
	extern "C" char usedForDeterminingStatePress_or_playerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther, int unk1);
	extern "C" int SomeStrangeModification(dStageActor_c* actor);
	extern "C" void DoStuffAndMarkDead(dStageActor_c *actor, Vec vector, float unk);
	extern "C" int SmoothRotation(short* rot, u16 amt, int unk2);
	// extern "C" void addToList(StandOnTopCollider *self);

	extern "C" bool HandlesEdgeTurns(dEn_c* actor);


	CREATE_STATE(daShyGuy, Walk);
	CREATE_STATE(daShyGuy, Turn);
	CREATE_STATE(daShyGuy, RealWalk);
	CREATE_STATE(daShyGuy, RealTurn);
	CREATE_STATE(daShyGuy, Jump);
	CREATE_STATE(daShyGuy, Sleep);
	CREATE_STATE(daShyGuy, Balloon_H);
	CREATE_STATE(daShyGuy, Balloon_V);
	CREATE_STATE(daShyGuy, Balloon_C);
	CREATE_STATE(daShyGuy, Judo);
	CREATE_STATE(daShyGuy, Spike);

	CREATE_STATE(daShyGuy, GoDizzy);
	CREATE_STATE(daShyGuy, BalloonDrop);
	CREATE_STATE(daShyGuy, FireKnockBack);
	CREATE_STATE(daShyGuy, FlameHit);
	CREATE_STATE(daShyGuy, Recover);

	CREATE_STATE(daShyGuy, Die);

////////////////////////
// Collision Functions
////////////////////////

	bool actorCanPopBalloon(dStageActor_c *ac) {
		int n = ac->name;
		return n == PLAYER || n == YOSHI ||
			n == PL_FIREBALL || n == ICEBALL ||
			n == YOSHI_FIRE || n == HAMMER;
	}
	// Collision callback to help shy guy not die at inappropriate times and ruin the dinner

	void shyCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther) {
		int t = ((daShyGuy*)apThis->owner)->type;
		if (t == 6 || t == 7 || t == 8) {
			// Should I do something about ice blocks here?
			if (actorCanPopBalloon(apOther->owner))
				((daShyGuy*)apThis->owner)->popBalloon();
		}

		if ((apOther->owner->name == 89) && (t == 5)) { return; }

		dEn_c::collisionCallback(apThis, apOther);
	}

	void ChucklesAndKnuckles(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (apOther->owner->name != PLAYER) { return; }
		((dEn_c*)apThis->owner)->_vf220(apOther->owner);
	}

	void balloonSmack(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (((daShyGuy*)apThis->owner)->frzMgr._mstate == 0) {
			if (actorCanPopBalloon(apOther->owner))
				((daShyGuy*)apThis->owner)->popBalloon();
		}
	}

	void daShyGuy::spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
		u16 name = ((dEn_c*)apOther->owner)->name;

		if (name == EN_COIN || name == EN_EATCOIN || name == AC_BLOCK_COIN || name == EN_COIN_JUGEM || name == EN_COIN_ANGLE
			|| name == EN_COIN_JUMP || name == EN_COIN_FLOOR || name == EN_COIN_VOLT || name == EN_COIN_WIND
			|| name == EN_BLUE_COIN || name == EN_COIN_WATER || name == EN_REDCOIN || name == EN_GREENCOIN
			|| name == EN_JUMPDAI || name == EN_ITEM)
			{ return; }

		if (acState.getCurrentState() == &StateID_RealWalk) {

			pos.x = ((pos.x - ((dEn_c*)apOther->owner)->pos.x) > 0) ? pos.x + 1.5 : pos.x - 1.5;
			// pos.x = direction ? pos.x + 1.5 : pos.x - 1.5;
			doStateChange(&StateID_RealTurn); }

		if (acState.getCurrentState() == &StateID_FireKnockBack) {
			float distance = pos.x - ((dEn_c*)apOther->owner)->pos.x;
			pos.x = pos.x + (distance / 4.0);
		}

		dEn_c::spriteCollision(apThis, apOther);
	}

	void daShyGuy::popBalloon() {
		doStateChange(&StateID_BalloonDrop);
	}

	void daShyGuy::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
		dStateBase_c *stateVar;
		dStateBase_c *deathState;

		char hitType;
		if (this->type < 6) {  // Regular Shy Guys
			stateVar = &StateID_GoDizzy;
			deathState = &StateID_Die;
		}
		else { // Ballooneers
			stateVar = &StateID_BalloonDrop;
			deathState = &StateID_Die;
		}


		if (this->isDown == 0) {
			hitType = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 2);
		}
		else { // Shy Guy is in downed mode
			hitType = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 0);
		}

		if(hitType == 1) {	// regular jump
			apOther->someFlagByte |= 2;
			if (this->isDown == 0) {
				this->playEnemyDownSound1();
				if (damage >= 1) {
					doStateChange(deathState); }
				else {
					doStateChange(stateVar); }
				damage++;
			}
			else { // Shy Guy is in downed mode - kill it with fire
				this->playEnemyDownSound1();
				doStateChange(deathState);
			}
		}
		else if(hitType == 3) {	// spinning jump or whatever?
			apOther->someFlagByte |= 2;
			if (this->isDown == 0) {
				this->playEnemyDownSound1();
				if (damage >= 1) {
					doStateChange(deathState); }
				else {
					doStateChange(stateVar); }
				damage++;
			}
			else { // Shy Guy is in downed mode - kill it with fire
				this->playEnemyDownSound1();
				doStateChange(deathState);
			}
		}
		else if(hitType == 0) {
			this->dEn_c::playerCollision(apThis, apOther);
			this->_vf220(apOther->owner);
		}
		// else if(hitType == 2) { \\ Minimario? }
	}

	void daShyGuy::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->playerCollision(apThis, apOther);
	}
	bool daShyGuy::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) {
		PlaySound(this, SE_EMY_DOWN);
		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_mr_hardhit", 0, &pos, &nullRot, &oneVec);
		//addScoreWhenHit accepts a player parameter.
		//DON'T DO THIS:
		// this->addScoreWhenHit(this);
		doStateChange(&StateID_Die);
		return true;
	}
	bool daShyGuy::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
		return this->collisionCatD_Drill(apThis, apOther);
	}
	bool daShyGuy::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
		return this->collisionCatD_Drill(apThis, apOther);
	}
	bool daShyGuy::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
		return this->collisionCatD_Drill(apThis, apOther);
	}
	bool daShyGuy::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther){
		return this->collisionCatD_Drill(apThis, apOther);
	}
	bool daShyGuy::collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther){
		return this->collisionCatD_Drill(apThis, apOther);
	}
	bool daShyGuy::collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther){
		return this->collisionCatD_Drill(apThis, apOther);
	}
	bool daShyGuy::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
		StageE4::instance->spawnCoinJump(pos, 0, 2, 0);
		return this->collisionCatD_Drill(apThis, apOther);
	}

	bool daShyGuy::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther){
		bool wut = dEn_c::collisionCat3_StarPower(apThis, apOther);
		doStateChange(&StateID_Die);
		return wut;
	}

	bool daShyGuy::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther){
		doStateChange(&StateID_DieSmoke);
		return true;
	}
	bool daShyGuy::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->damage += 1;

		dStateBase_c *stateVar;
		stateVar = &StateID_DieSmoke;

		if (this->type < 6) {  // Regular Shy Guys Except Jumper

			backFire = apOther->owner->direction ^ 1;

			// if (this->isDown == 0) {
			// 	stateVar = &StateID_FireKnockBack;
			// }
			// else {
				StageE4::instance->spawnCoinJump(pos, 0, 1, 0);
				doStateChange(&StateID_DieSmoke);
			// }
		}
		else { // Ballooneers
			stateVar = &StateID_FlameHit;
		}

		if (this->damage > 1) {
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_EMY_DOWN, 1);
			StageE4::instance->spawnCoinJump(pos, 0, 1, 0);
			doStateChange(&StateID_DieSmoke);
		}
		else {
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_EMY_KURIBO_L_DAMAGE_01, 1);
			doStateChange(stateVar);
		}
		return true;
	}

	// void daShyGuy::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {

	// 	dEn_C::collisionCat2_IceBall_15_YoshiIce(apThis, apOther);
	// }

	// These handle the ice crap
	void daShyGuy::_vf148() {
		dEn_c::_vf148();
		doStateChange(&StateID_Die);
	}
	void daShyGuy::_vf14C() {
		dEn_c::_vf14C();
		doStateChange(&StateID_Die);
	}

	extern "C" void sub_80024C20(void);
	extern "C" void __destroy_arr(void*, void(*)(void), int, int);
	//extern "C" __destroy_arr(struct DoSomethingCool, void(*)(void), int cnt, int bar);

	bool daShyGuy::CreateIceActors() {
		struct DoSomethingCool my_struct = { 0, this->pos, {1.2, 1.5, 1.5}, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	    this->frzMgr.Create_ICEACTORs( (void*)&my_struct, 1 );
	    __destroy_arr( (void*)&my_struct, sub_80024C20, 0x3C, 1 );
	    chrAnimation.setUpdateRate(0.0f);
	    return true;
	}

bool daShyGuy::calculateTileCollisions() {
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
		max_speed.x = (direction == 1) ? -XSpeed : XSpeed;
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

void daShyGuy::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->anmFile.GetResAnmChr(name);
	this->chrAnimation.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->chrAnimation, unk2);
	this->chrAnimation.setUpdateRate(rate);
}

int daShyGuy::onCreate() {

	this->type = this->settings >> 28 & 0xF;
	int baln = this->settings >> 24 & 0xF;
	this->distance = this->settings >> 12 & 0xF;

	stillFalling = 0;

	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->deleteForever = 1;

	// Balloon Specifics
	if (type == 6 || type == 7 || type == 8) {
		this->renderBalloon = 1;

		this->balloonFile.data = getResource("shyguy", "g3d/balloon.brres");
		nw4r::g3d::ResMdl mdlB = this->balloonFile.GetResMdl("ballon");
		balloonModel.setup(mdlB, &allocator, 0x224, 1, 0);
		balloonModelB.setup(mdlB, &allocator, 0x224, 1, 0);

		ActivePhysics::Info iballoonPhysics;

		iballoonPhysics.xDistToCenter = 0.0;
		iballoonPhysics.yDistToCenter = -18.0;
		iballoonPhysics.xDistToEdge   = 13.0;
		iballoonPhysics.yDistToEdge   = 12.0;

		iballoonPhysics.category1  = 0x3;
		iballoonPhysics.category2  = 0x0;
		iballoonPhysics.bitfield1  = 0x4f;
		iballoonPhysics.bitfield2  = 0xffbafffe;
		iballoonPhysics.unkShort1C = 0x0;
		iballoonPhysics.callback   = balloonSmack;

		balloonPhysics.initWithStruct(this, &iballoonPhysics);
		balloonPhysics.addToList();


		// if (baln != 0) {
		// 	char *itemArc;
		// 	char *itemBrres;
		// 	char *itemMdl;

		// 	if (baln == 1) {
		// 		itemArc		= "I_kinoko";
		// 		itemBrres	= "g3d/I_kinoko.brres";
		// 		itemMdl		= "I_kinoko";
		// 	}
		// 	else if (baln == 2) {
		// 		itemArc		= "I_fireflower";
		// 		itemBrres	= "g3d/I_fireflower.brres";
		// 		itemMdl		= "I_fireflower";
		// 	}
		// 	else if (baln == 3) {
		// 		itemArc		= "I_propeller";
		// 		itemBrres	= "g3d/I_propeller.brres";
		// 		itemMdl		= "I_propeller_model";
		// 	}
		// 	else if (baln == 4) {
		// 		itemArc		= "I_iceflower";
		// 		itemBrres	= "g3d/I_iceflower.brres";
		// 		itemMdl		= "I_iceflower";
		// 	}
		// 	else if (baln == 5) {
		// 		itemArc		= "I_star";
		// 		itemBrres	= "g3d/I_star.brres";
		// 		itemMdl		= "I_star";
		// 	}
		// 	else if (baln == 6) {
		// 		itemArc		= "I_penguin";
		// 		itemBrres	= "g3d/I_penguin.brres";
		// 		itemMdl		= "I_penguin";
		// 	}

		// 	this->carryFile.data = getResource(itemArc, itemBrres);
		// 	nw4r::g3d::ResMdl mdlC = this->carryFile.GetResMdl(itemMdl);
		// 	carryModel.setup(mdlC, &allocator, 0x224, 1, 0);

		// 	nw4r::g3d::ResAnmChr anmChrC = this->carryFile.GetResAnmChr("wait2");
		// 	this->carryAnm.setup(mdlC, anmChrC, &this->allocator, 0);

		// 	this->carryAnm.bind(&this->carryModel, anmChrC, 1);
		// 	this->carryModel.bindAnim(&this->carryAnm, 0.0);
		// 	this->carryAnm.setUpdateRate(1.0);
		// }
	}
	else {this->renderBalloon = 0;}


	// Shy Guy Colours
	if (type == 1 || type == 8 || type == 10) {
		this->resFile.data = getResource("shyguy", "g3d/ShyGuyBlue.brres");
		distance = 1;
	}
	else if (type == 5) {
		this->resFile.data = getResource("shyguy", "g3d/ShyGuyGreen.brres");
	}
	else if (type == 3) {
		this->resFile.data = getResource("shyguy", "g3d/ShyGuyCyan.brres");
	}
	else if (type == 4) {
		this->resFile.data = getResource("shyguy", "g3d/ShyGuyPurple.brres");
	}
	else {
		this->resFile.data = getResource("shyguy", "g3d/ShyGuyRed.brres");
	}
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("body_h");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);


	// Animations start here
	this->anmFile.data = getResource("shyguy", "g3d/ShyGuyAnimations.brres");
	nw4r::g3d::ResAnmChr anmChr = this->anmFile.GetResAnmChr("c18_IDLE_R");
	this->chrAnimation.setup(mdl, anmChr, &this->allocator, 0);

	allocator.unlink();

	// Stuff I do understand

	this->scale = (Vec){20.0, 20.0, 20.0};

	this->pos.y += 36.0;
	this->rot.x = 0; // X is vertical axis
	this->rot.y = 0xD800; // Y is horizontal axis
	this->rot.z = 0; // Z is ... an axis >.>
	this->direction = 1; // Heading left.

	this->speed.x = 0.0;
	this->speed.y = 0.0;
	this->max_speed.x = 0.6;
	this->x_speed_inc = 0.15;
	this->Baseline = this->pos.y;
	this->XSpeed = 0.6;
	this->balloonSize = 1.5;


	ActivePhysics::Info HitMeBaby;

	// Note: if this gets changed, also change the point where the default
	// values are assigned after de-ballooning
	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = 12.0;
	HitMeBaby.xDistToEdge = 8.0;
	HitMeBaby.yDistToEdge = 12.0;
	if (renderBalloon) {
		HitMeBaby.yDistToCenter = 9.0f;
		HitMeBaby.yDistToEdge = 9.0f;
	}

	HitMeBaby.category1 = 0x3;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x6F;
	HitMeBaby.bitfield2 = 0xffbafffe;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &shyCollisionCallback;

	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();


	// Tile collider

	// These fucking rects do something for the tile rect
	spriteSomeRectX = 28.0f;
	spriteSomeRectY = 32.0f;
	_320 = 0.0f;
	_324 = 16.0f;

	// These structs tell stupid collider what to collide with - these are from koopa troopa
	static const lineSensor_s below(-5<<12, 5<<12, 0<<12);
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

	if (type == 0) {
		bindAnimChr_and_setUpdateRate("c18_EV_WIN_2_R", 1, 0.0, 1.5);
		doStateChange(&StateID_RealWalk);
	}
	else if (type == 1) {
		bindAnimChr_and_setUpdateRate("c18_EV_WIN_2_R", 1, 0.0, 1.5);
		doStateChange(&StateID_RealWalk);
	}
	else if (type == 2) {
		doStateChange(&StateID_Sleep);
	}
	else if (type == 3) {
		doStateChange(&StateID_Jump);
	}
	else if (type == 4) {
		// Chuckles is left, Knuckles is Right
		ActivePhysics::Info iChuckles;
		ActivePhysics::Info iKnuckles;

		iChuckles.xDistToCenter = -27.0;
		iChuckles.yDistToCenter = 12.0;
		iChuckles.xDistToEdge   = 27.0;
		iChuckles.yDistToEdge   = 10.0;

		iKnuckles.xDistToCenter = 27.0;
		iKnuckles.yDistToCenter = 12.0;
		iKnuckles.xDistToEdge   = 27.0;
		iKnuckles.yDistToEdge   = 10.0;

		iKnuckles.category1  = iChuckles.category1  = 0x3;
		iKnuckles.category2  = iChuckles.category2  = 0x0;
		iKnuckles.bitfield1  = iChuckles.bitfield1  = 0x4F;
		iKnuckles.bitfield2  = iChuckles.bitfield2  = 0x0;
		iKnuckles.unkShort1C = iChuckles.unkShort1C = 0x0;
		iKnuckles.callback   = iChuckles.callback   = ChucklesAndKnuckles;

		Chuckles.initWithStruct(this, &iChuckles);
		Knuckles.initWithStruct(this, &iKnuckles);

		doStateChange(&StateID_Judo);
	}
	else if (type == 5) {
		doStateChange(&StateID_Spike);
	}
	else if (type == 6) {
		doStateChange(&StateID_Balloon_H);
	}
	else if (type == 7) {
		doStateChange(&StateID_Balloon_V);
	}
	else if (type == 8) {
		doStateChange(&StateID_Balloon_C);
	}

	this->onExecute();
	return true;
}

int daShyGuy::onDelete() {
	return true;
}

int daShyGuy::onExecute() {
	acState.execute();
	updateModelMatrices();
	bodyModel._vf1C();

	return true;
}

int daShyGuy::onDraw() {
	bodyModel.scheduleForDrawing();

	if (this->renderBalloon == 1) {
		balloonModel.scheduleForDrawing();
		balloonModelB.scheduleForDrawing();
	}

	// if (this->baln > 0) {
	// 	carryModel.scheduleForDrawing();
	// 	carryModel._vf1C();

	// 	if(this->carryAnm.isAnimationDone())
	// 		this->carryAnm.setCurrentFrame(0.0);
	// }

	return true;
}

void daShyGuy::updateModelMatrices() {
	// This won't work with wrap because I'm lazy.

	if (this->frzMgr._mstate == 1)
		matrix.translation(pos.x, pos.y, pos.z);
	else
		matrix.translation(pos.x, pos.y - 2.0, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);

	if (this->renderBalloon == 1) {
		matrix.translation(pos.x, pos.y - 32.0, pos.z);

		balloonModel.setDrawMatrix(matrix);
		balloonModel.setScale(balloonSize, balloonSize, balloonSize);
		balloonModel.calcWorld(false);

		balloonModelB.setDrawMatrix(matrix);
		balloonModelB.setScale(balloonSize, balloonSize, balloonSize);
		balloonModelB.calcWorld(false);
	}

	// if (this->baln > 0) {
	// 	matrix.applyRotationYXZ(0,0,0);
	// 	matrix.translation(pos.x+40.0, pos.y - 28.0, pos.z + 1000.0);

	// 	carryModel.setDrawMatrix(matrix);
	// 	carryModel.setScale(21.0, 21.0, 21.0);
	// 	carryModel.calcWorld(false);
	// }

}

///////////////
// Walk State
///////////////
	void daShyGuy::beginState_Walk() {
		this->timer = 0;
		this->rot.y = (direction) ? 0xD800 : 0x2800;

		this->max_speed.x = 0.0;
		this->speed.x = 0.0;
		this->x_speed_inc = 0.0;
	}
	void daShyGuy::executeState_Walk() {
		chrAnimation.setUpdateRate(1.5f);

		this->pos.x += (direction) ? -0.4 : 0.4;

		if (this->timer > (this->distance * 32)) {
			doStateChange(&StateID_Turn);
		}

		if(this->chrAnimation.isAnimationDone())
			this->chrAnimation.setCurrentFrame(0.0);

		this->timer = this->timer + 1;
	}
	void daShyGuy::endState_Walk() {
	}

///////////////
// Turn State
///////////////
	void daShyGuy::beginState_Turn() {
		// bindAnimChr_and_setUpdateRate("c18_IDLE_R", 1, 0.0, 1.0);
		this->direction ^= 1;
		this->speed.x = 0.0;
	}
	void daShyGuy::executeState_Turn() {

		if(this->chrAnimation.isAnimationDone())
			this->chrAnimation.setCurrentFrame(0.0);

		u16 amt = (this->direction == 0) ? 0x2800 : 0xD800;
		int done = SmoothRotation(&this->rot.y, amt, 0x800);

		if(done) {
			this->doStateChange(&StateID_Walk);
		}
	}
	void daShyGuy::endState_Turn() {
	}

///////////////
// Jump State
///////////////
	void daShyGuy::beginState_Jump() {
		this->max_speed.x = 0.0;
		this->speed.x = 0.0;
		this->x_speed_inc = 0.0;

		this->timer = 0;
		this->jumpCounter = 0;
	}
	void daShyGuy::executeState_Jump() {

		// Always face Mario
		u8 facing = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);

		if (facing != this->direction) {
			this->direction = facing;
			this->rot.y = (direction) ? 0xD800 : 0x2800;
		}

		// Shy Guy is on ground
		if (this->pos.y < this->Baseline) {

			bindAnimChr_and_setUpdateRate("c18_IDLE_R", 1, 0.0, 1.0);

			this->timer = this->timer + 1;

			// Make him wait for 0.5 seconds
			if (this->timer > 30) {

				if(this->chrAnimation.isAnimationDone())
					this->chrAnimation.setCurrentFrame(0.0);

				this->speed.x = 0;
				this->speed.y = 0;
			}

			// Then Jump!
			else {
				if (this->jumpCounter == 3) { this->jumpCounter = 0; }

				this->pos.y = this->Baseline + 1;
				this->timer = 0;
				this->jumpCounter = this->jumpCounter + 1;


				if (this->jumpCounter == 3) {
					bindAnimChr_and_setUpdateRate("c18_NORMAL_STEAL_R", 1, 0.0, 1.0);
					this->speed.y = 8.0;
					PlaySoundAsync(this, SE_PLY_JUMPDAI_HIGH);
				}
				else {
					bindAnimChr_and_setUpdateRate("c18_EV_WIN_1_R", 1, 0.0, 1.0);
					this->speed.y = 6.0;
					PlaySoundAsync(this, SE_PLY_JUMPDAI);
				}

			}
		}

		// While he's jumping, it's time for gravity.
		else {

			this->speed.y = this->speed.y - 0.15;

			if (this->jumpCounter == 3) {
				if(this->chrAnimation.isAnimationDone())
					this->chrAnimation.setCurrentFrame(0.0);
			}
			else {
				if(this->chrAnimation.isAnimationDone())
					this->chrAnimation.setCurrentFrame(0.0);
			}
		}

		this->HandleXSpeed();
		this->HandleYSpeed();
		this->UpdateObjectPosBasedOnSpeedValuesReal();
	}
	void daShyGuy::endState_Jump() {
	}

///////////////
// Sleep State
///////////////
	void daShyGuy::beginState_Sleep() {
		bindAnimChr_and_setUpdateRate("c18_EV_LOSE_2_R", 1, 0.0, 1.0);
		this->rot.y = 0x0000;
	}
	void daShyGuy::executeState_Sleep() {
		if(this->chrAnimation.isAnimationDone())
			this->chrAnimation.setCurrentFrame(0.0);
	}
	void daShyGuy::endState_Sleep() {
	}

///////////////
// Balloon H State
///////////////
	void daShyGuy::beginState_Balloon_H() {
		bindAnimChr_and_setUpdateRate("c18_L_DMG_F_3_R", 1, 0.0, 1.0);
		this->timer = 0;
		this->initialPos = this->pos;
		this->rot.x = 0xFE00;
		this->rot.y = 0;
	}
	void daShyGuy::executeState_Balloon_H() {

		// Makes him bob up and down
		this->pos.y = this->initialPos.y + ( sin(this->timer * 3.14 / 60.0) * 6.0 );

		// Makes him move side to side
		this->pos.x = this->initialPos.x + ( sin(this->timer * 3.14 / 600.0) * (float)this->distance * 8.0);

		this->timer = this->timer + 1;

		if(this->chrAnimation.isAnimationDone())
			this->chrAnimation.setCurrentFrame(0.0);

	}
	void daShyGuy::endState_Balloon_H() {
	}

///////////////
// Balloon V State
///////////////
	void daShyGuy::beginState_Balloon_V() {
		bindAnimChr_and_setUpdateRate("c18_L_DMG_F_3_R", 1, 0.0, 1.0);
		this->timer = 0;
		this->initialPos = this->pos;
		this->rot.x = 0xFE00;
		this->rot.y = 0;
	}
	void daShyGuy::executeState_Balloon_V() {
		// Makes him bob up and down
		this->pos.x = this->initialPos.x + ( sin(this->timer * 3.14 / 60.0) * 6.0 );

		// Makes him move side to side
		this->pos.y = this->initialPos.y + ( sin(this->timer * 3.14 / 600.0) * (float)this->distance * 8.0 );

		this->timer = this->timer + 1;

		if(this->chrAnimation.isAnimationDone())
			this->chrAnimation.setCurrentFrame(0.0);
	}
	void daShyGuy::endState_Balloon_V() {
	}

///////////////
// Balloon C State
///////////////
	void daShyGuy::beginState_Balloon_C() {
		bindAnimChr_and_setUpdateRate("c18_L_DMG_F_3_R", 1, 0.0, 1.0);
		this->timer = 0;
		this->initialPos = this->pos;
		this->rot.x = 0xFE00;
		this->rot.y = 0;
	}
	void daShyGuy::executeState_Balloon_C() {
		// Makes him bob up and down
		this->pos.x = this->initialPos.x + ( sin(this->timer * 3.14 / 600.0) * (float)this->distance * 8.0 );

		// Makes him move side to side
		this->pos.y = this->initialPos.y + ( cos(this->timer * 3.14 / 600.0) * (float)this->distance * 8.0 );

		this->timer = this->timer + 1;

		if(this->chrAnimation.isAnimationDone())
			this->chrAnimation.setCurrentFrame(0.0);
	}
	void daShyGuy::endState_Balloon_C() {
	}

///////////////
// Judo State
///////////////
	void daShyGuy::beginState_Judo() {
		this->max_speed.x = 0.0;
		this->speed.x = 0.0;
		this->x_speed_inc = 0.0;
		this->pos.y -= 4.0;

		this->timer = 0;
	}
	void daShyGuy::executeState_Judo() {

	// chargin 476? 673? 760? 768? 808? 966?
		if (this->timer == 0) { bindAnimChr_and_setUpdateRate("c18_OB_IDLE_R", 1, 0.0, 1.0); }

		this->timer = this->timer + 1;

		if (this->timer == 80) {
			if (this->direction == 1) {
				Vec efPos = {this->pos.x + 7.0f, this->pos.y + 14.0f, this->pos.z - 5500.0f};
				S16Vec nullRot = {0,0,0};
				Vec efScale = {0.8f, 0.8f, 0.8f};
				SpawnEffect("Wm_ob_keyget02_lighit", 0, &efPos, &nullRot, &efScale);
			}
			else {
				Vec efPos = {this->pos.x - 7.0f, this->pos.y + 14.0f, this->pos.z + 5500.0f};
				S16Vec nullRot = {0,0,0};
				Vec efScale = {0.8f, 0.8f, 0.8f};
				SpawnEffect("Wm_ob_keyget02_lighit", 0, &efPos, &nullRot, &efScale);
			}
		}

		if (this->timer < 120) {
			// Always face Mario
			u8 facing = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);

			if (facing != this->direction) {
				this->direction = facing;
				if (this->direction == 1) {
					this->rot.y = 0xD800;
				}
				else {
					this->rot.y = 0x2800;
				}
			}


			if(this->chrAnimation.isAnimationDone())
				this->chrAnimation.setCurrentFrame(0.0);
		}

		else if (this->timer == 120) {
			bindAnimChr_and_setUpdateRate("c18_H_CUT_R", 1, 0.0, 1.0);

		}

		else if (this->timer == 132) {
			PlaySoundAsync(this, SE_EMY_CRASHER_PUNCH);

			if (this->direction == 1) {
				Vec efPos = {this->pos.x - 18.0f, this->pos.y + 16.0f, this->pos.z};
				S16Vec nullRot = {0,0,0};
				Vec efScale = {1.5f, 1.5f, 1.5f};
				SpawnEffect("Wm_mr_wallkick_b_l", 0, &efPos, &nullRot, &efScale);
				Chuckles.addToList();
			}
			else {
				Vec efPos = {this->pos.x + 18.0f, this->pos.y + 16.0f, this->pos.z};
				S16Vec nullRot = {0,0,0};
				Vec efScale = {1.5f, 1.5f, 1.5f};
				SpawnEffect("Wm_mr_wallkick_s_r", 0, &efPos, &nullRot, &efScale);
				Knuckles.addToList();
			}
		}

		else {

			if(this->chrAnimation.isAnimationDone()) {
				if (this->direction == 1) {
					Vec efPos = {this->pos.x - 38.0f, this->pos.y + 16.0f, this->pos.z};
					S16Vec nullRot = {0,0,0};
					Vec oneVec = {1.0f, 1.0f, 1.0f};
					SpawnEffect("Wm_mr_wirehit_hit", 0, &efPos, &nullRot, &oneVec);
					Chuckles.removeFromList();
				}
				else {
					Vec efPos = {this->pos.x + 38.0f, this->pos.y + 16.0f, this->pos.z};
					S16Vec nullRot = {0,0,0};
					Vec oneVec = {1.0f, 1.0f, 1.0f};
					SpawnEffect("Wm_mr_wirehit_hit", 0, &efPos, &nullRot, &oneVec);
					Knuckles.removeFromList();
				}

				this->timer = 0;
				PlaySoundAsync(this, SE_EMY_BIG_PAKKUN_DAMAGE_1);
			}
		}
	}
	void daShyGuy::endState_Judo() {
	}

///////////////
// Spike State
///////////////
	void daShyGuy::beginState_Spike() {
		this->timer = 80;
		spikeTurn = 0;

		this->max_speed.x = 0.0;
		this->speed.x = 0.0;
		this->x_speed_inc = 0.0;
		this->pos.y -= 4.0;
	}
	void daShyGuy::executeState_Spike() {

		if (this->timer == 0) { bindAnimChr_and_setUpdateRate("c18_OB_IDLE_R", 1, 0.0, 1.0); }

		if (this->timer < 120) {
			// Always face Mario
			u8 facing = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);

			if (facing != this->direction) {
				this->direction = facing;
				if (this->direction == 1) {
					this->rot.y = 0xD800;
				}
				else {
					this->rot.y = 0x2800;
				}
			}

			if(this->chrAnimation.isAnimationDone())
				this->chrAnimation.setCurrentFrame(0.0);
		}

		else if (this->timer == 120) {
			bindAnimChr_and_setUpdateRate("c18_H_SHOT_R", 1, 0.0, 1.0);
		}

		else if (this->timer == 160) {
			PlaySound(this, SE_EMY_KANIBO_THROW);

			Vec pos;
			pos.x = this->pos.x;
			pos.y = this->pos.y;
			pos.z = this->pos.z;
			dStageActor_c *spawned;

			if (this->direction == 1) {
				spawned = CreateActor(89, 0x2, pos, 0, 0);
				spawned->scale.x = 0.9;
				spawned->scale.y = 0.9;
				spawned->scale.z = 0.9;

				spawned->speed.x = -2.0;
				spawned->speed.y = 2.0;
			}
			else {
				spawned = CreateActor(89, 0x12, pos, 0, 0);
				spawned->scale.x = 0.9;
				spawned->scale.y = 0.9;
				spawned->scale.z = 0.9;

				spawned->speed.x = 2.0;
				spawned->speed.y = 2.0;
			}
		}

		else {

			if(this->chrAnimation.isAnimationDone()) {
				this->timer = 0;
				return;
			}
		}

		this->timer = this->timer + 1;

	}
	void daShyGuy::endState_Spike() {
	}

///////////////
// Real Walk State
///////////////
bool daShyGuy::willWalkOntoSuitableGround() {
	static const float deltas[] = {2.5f, -2.5f};
	VEC3 checkWhere = {
			pos.x + deltas[direction],
			4.0f + pos.y,
			pos.z};

	u32 props = collMgr.getTileBehaviour2At(checkWhere.x, checkWhere.y, currentLayerID);

	//if (getSubType(props) == B_SUB_LEDGE)
	if (((props >> 16) & 0xFF) == 8)
		return false;

	float someFloat = 0.0f;
	if (collMgr.sub_800757B0(&checkWhere, &someFloat, currentLayerID, 1, -1)) {
		if (someFloat < checkWhere.y && someFloat > (pos.y - 5.0f))
			return true;
	}

	return false;
}


	void daShyGuy::beginState_RealWalk() {
		//inline this piece of code
		this->max_speed.x = (this->direction) ? -this->XSpeed : this->XSpeed;
		this->speed.x = (direction) ? -0.6f : 0.6f;

		this->max_speed.y = -4.0;
		this->speed.y = -4.0;
		this->y_speed_inc = -0.1875;
	}
	void daShyGuy::executeState_RealWalk() {
		chrAnimation.setUpdateRate(1.5f);

		// if (distance) {
		// 	// What the fuck. Somehow, having this code makes the shyguy not
		// 	// fall through solid-on-top platforms...
		// 	bool turn = collMgr.isOnTopOfTile();
		// 	if (!turn) {
		// 		if (!stillFalling) {
		// 			stillFalling = true;
		// 			pos.x = direction ? pos.x + 1.5 : pos.x - 1.5;
		// 			doStateChange(&StateID_RealTurn);
		// 		}
		// 	} else
		// }


		if (distance) {
			if (collMgr.isOnTopOfTile()) {
				stillFalling = false;

				if (!willWalkOntoSuitableGround()) {
					pos.x = direction ? pos.x + 1.5 : pos.x - 1.5;
					doStateChange(&StateID_RealTurn);
				}
			}
			else {
				if (!stillFalling) {
					stillFalling = true;
					pos.x = direction ? pos.x + 1.5 : pos.x - 1.5;
					doStateChange(&StateID_RealTurn);
				}
			}
		}

		bool ret = calculateTileCollisions();
		if (ret) {
			doStateChange(&StateID_RealTurn);
		}

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}
	}
	void daShyGuy::endState_RealWalk() { }

///////////////
// Real Turn State
///////////////
	void daShyGuy::beginState_RealTurn() {

		this->direction ^= 1;
		this->speed.x = 0.0;
	}
	void daShyGuy::executeState_RealTurn() {

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		u16 amt = (this->direction == 0) ? 0x2800 : 0xD800;
		int done = SmoothRotation(&this->rot.y, amt, 0x800);

		if(done) {
			this->doStateChange(&StateID_RealWalk);
		}
	}
	void daShyGuy::endState_RealTurn() {
	}

///////////////
// GoDizzy State
///////////////
	void daShyGuy::beginState_GoDizzy() {
		bindAnimChr_and_setUpdateRate("c18_L_DMG_F_1_R", 1, 0.0, 1.0);

		// SpawnEffect("Wm_en_spindamage", 0, &(Vec){this->pos.x, this->pos.y + 24.0, 0}, &(S16Vec){0,0,0}, &(Vec){1.0, 1.0, 1.0});

		this->max_speed.x = 0;
		this->speed.x = 0;
		this->x_speed_inc = 0;

		this->max_speed.y = -4.0;
		this->speed.y = -4.0;
		this->y_speed_inc = -0.1875;

		this->timer = 0;
		this->jumpCounter = 0;
		this->isDown = 1;
	}
	void daShyGuy::executeState_GoDizzy() {
		calculateTileCollisions();

		Vec efPos = {this->pos.x, this->pos.y + 24.0f, 0};
		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		effect.spawn("Wm_en_spindamage", 0, &efPos, &nullRot, &oneVec);

		if (this->jumpCounter == 0) {
			if(this->chrAnimation.isAnimationDone()) {
				this->jumpCounter = 1;
				bindAnimChr_and_setUpdateRate("c18_L_DMG_F_3_R", 1, 0.0, 1.0);
			}
		}

		else {
			if(this->chrAnimation.isAnimationDone()) {
				this->chrAnimation.setCurrentFrame(0.0);
			}

			if (this->timer > 600) {
				doStateChange(&StateID_Recover);
				damage = 0;
			}

			this->timer += 1;
		}
	}
	void daShyGuy::endState_GoDizzy() {}

///////////////
// BalloonDrop State
///////////////
	void daShyGuy::beginState_BalloonDrop() {
		bindAnimChr_and_setUpdateRate("c18_C_BLOCK_BREAK_R", 1, 0.0, 2.0);

		this->max_speed.x = 0.0;
		this->speed.x = 0.0;
		this->x_speed_inc = 0.0;

		this->max_speed.y = -2.0;
		this->speed.y = -2.0;
		this->y_speed_inc = -0.1875;

		this->isDown = 1;
		this->renderBalloon = 0;

		// char powerup;

		// if (baln == 1) {
		// 	powerup		= 0x0B000007;
		// }
		// else if (baln == 2) {
		// 	powerup		= 0x0B000009;
		// }
		// else if (baln == 3) {
		// 	powerup		= 0x0B000001;
		// }
		// else if (baln == 4) {
		// 	powerup		= 0x0C00000E;
		// }
		// else if (baln == 5) {
		// 	powerup		= 0x0C000015;
		// }
		// else if (baln == 6) {
		// 	powerup		= 0x0C000011;
		// }
		// CreateActor(60, powerup, (Vec){pos.x, pos.y - 28.0, pos.z}, 0, 0);
		// this->baln = 0;

		balloonPhysics.removeFromList();
		Vec efPos = {this->pos.x, this->pos.y - 32.0f, 0};
		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_en_explosion_ln", 0, &efPos, &nullRot, &oneVec);
		PlaySound(this, SE_PLY_BALLOON_BRAKE);

		if (this->type != 8)
			this-distance == 0;

		type = 0;
	}
	void daShyGuy::executeState_BalloonDrop() {

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		bool ret = calculateTileCollisions();

		if (speed.y == 0.0) {
			Vec efPos = {this->pos.x, this->pos.y, 0};
			S16Vec nullRot = {0,0,0};
			Vec oneVec = {1.0f, 1.0f, 1.0f};
			SpawnEffect("Wm_en_sndlandsmk_s", 0, &efPos, &nullRot, &oneVec);
			doStateChange(&StateID_GoDizzy);

			aPhysics.info.yDistToCenter = 12.0f;
			aPhysics.info.yDistToEdge = 12.0f;
		}
	}
	void daShyGuy::endState_BalloonDrop() {
	}

///////////////
// FireKnockBack State
///////////////
	void daShyGuy::beginState_FireKnockBack() {
		bindAnimChr_and_setUpdateRate("c18_C_BLOCK_BREAK_R", 1, 0.0, 1.0);

		// Backfire 0 == Fireball to the right
		// Backfire 1 == Fireball to the left

		directionStore = this->direction;
		speed.x = (this->backFire) ? this->XSpeed : -this->XSpeed;
		speed.x *= 1.2f;
		max_speed.x = speed.x;
		x_speed_inc = 0.0f;
	}
	void daShyGuy::executeState_FireKnockBack() {

		calculateTileCollisions();
		// move backwards here
		this->speed.x = this->speed.x / 1.02f;

		if(this->chrAnimation.isAnimationDone()) {
			if (aPhysics.result1 == 0 && aPhysics.result2 == 0 && aPhysics.result3 == 0) {
				bindAnimChr_and_setUpdateRate("c18_EV_WIN_2_R", 1, 0.0, 1.5);
				doStateChange(&StateID_RealWalk);
			}
		}
	}
	void daShyGuy::endState_FireKnockBack() {
		this->direction = directionStore;
	}

///////////////
// FlameHit State
///////////////
	void daShyGuy::beginState_FlameHit() {
		bindAnimChr_and_setUpdateRate("c18_C_BLOCK_BREAK_R", 1, 0.0, 1.0);
	}
	void daShyGuy::executeState_FlameHit() {

		if(this->chrAnimation.isAnimationDone()) {
			if (type == 6) {
				doStateChange(&StateID_Balloon_H);
			}
			else if (type == 7) {
				doStateChange(&StateID_Balloon_V);
			}
			else if (type == 8) {
				doStateChange(&StateID_Balloon_C);
			}
		}
	}
	void daShyGuy::endState_FlameHit() {}

///////////////
// Recover State
///////////////
	void daShyGuy::beginState_Recover() {
		bindAnimChr_and_setUpdateRate("c18_L_DMG_F_4_R", 1, 0.0, 1.0);
	}
	void daShyGuy::executeState_Recover() {

		calculateTileCollisions();

		if(this->chrAnimation.isAnimationDone()) {
			if (type == 3) {
				doStateChange(&StateID_Jump);
			}
			else {
				bindAnimChr_and_setUpdateRate("c18_EV_WIN_2_R", 1, 0.0, 1.5);
				doStateChange(&StateID_RealWalk);
			}
		}
	}
	void daShyGuy::endState_Recover() {
		this->isDown = 0;
		this->rot.y = (direction) ? 0xD800 : 0x2800;
	}

///////////////
// Die State
///////////////
	void daShyGuy::beginState_Die() {
		// dEn_c::dieFall_Begin();
		this->removeMyActivePhysics();

		bindAnimChr_and_setUpdateRate("c18_C_BLOCK_BREAK_R", 1, 0.0, 2.0);
		this->timer = 0;
		this->dying = -10.0;
		this->Baseline = this->pos.y;
		this->rot.y = 0;
		this->rot.x = 0;

		if (type > 5 && type < 9) {
			this->renderBalloon = 0;
			Vec efPos = {this->pos.x, this->pos.y - 32.0f, 0};
			S16Vec nullRot = {0,0,0};
			Vec oneVec = {1.0f, 1.0f, 1.0f};
			SpawnEffect("Wm_en_explosion_ln", 0, &efPos, &nullRot, &oneVec);
		}
	}
	void daShyGuy::executeState_Die() {

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);
		}

		this->timer += 1;

		// this->pos.x += 0.5;
		this->pos.y = Baseline + (-0.2 * dying * dying) + 20.0;

		this->dying += 0.5;

		if (this->timer > 450) {
			OSReport("Killing");
			this->kill();
			this->Delete(this->deleteForever);
		}

		// dEn_c::dieFall_Execute();

	}
	void daShyGuy::endState_Die() {
	}

