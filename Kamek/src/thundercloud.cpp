#include <common.h>
#include <game.h>
#include <g3dhax.h>

#include "boss.h"

const char* TLCarcNameList [] = {
	"tcloud",
	NULL
};

class dThunderCloud : public dEn_c {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;
	nw4r::g3d::ResFile resFile;
	m3d::anmChr_c anm;

	mEf::es2 bolt;
	mEf::es2 charge;

	float Baseline;
	u32 timer;
	int dying;
	char killFlag;
	bool stationary;
	float leader;
	pointSensor_s below;

	bool usingEvents;
	u64 eventFlag;

	ActivePhysics Lightning;

	void dieFall_Begin();
	void dieFall_Execute();
	static dThunderCloud *build();

	void updateModelMatrices();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther);

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);

	void powBlockActivated(bool isNotMPGP);

	void _vf148();
	void _vf14C();
	bool CreateIceActors();

	void lightningStrike();

	USING_STATES(dThunderCloud);
	DECLARE_STATE(Follow);
	DECLARE_STATE(Lightning);
	DECLARE_STATE(Wait);
};

dThunderCloud *dThunderCloud::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dThunderCloud));
	return new(buffer) dThunderCloud;
}


CREATE_STATE(dThunderCloud, Follow);
CREATE_STATE(dThunderCloud, Lightning);
CREATE_STATE(dThunderCloud, Wait);

void dThunderCloud::powBlockActivated(bool isNotMPGP) { }


// Collision Callbacks
	extern "C" void dAcPy_vf3F4(void* mario, void* other, int t);
	extern "C" bool BigHanaFireball(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther);
	extern "C" void *dAcPy_c__ChangePowerupWithAnimation(void * Player, int powerup);
	extern "C" int CheckExistingPowerup(void * Player);

	void dThunderCloud::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {

		if (this->counter_504[apOther->owner->which_player]) {
			if (apThis->info.category2 == 0x9) {
				int p = CheckExistingPowerup(apOther->owner);
				if (p != 3) {	// Powerups - 0 = small; 1 = big; 2 = fire; 3 = mini; 4 = prop; 5 = peng; 6 = ice; 7 = hammer
					dAcPy_c__ChangePowerupWithAnimation(apOther->owner, 3);
				}
				else { dAcPy_vf3F4(apOther->owner, this, 9); }
			}

			else { dAcPy_vf3F4(apOther->owner, this, 9); }
		}

		this->counter_504[apOther->owner->which_player] = 0x20;
	}

	void dThunderCloud::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); }
	bool dThunderCloud::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); return true; }
	bool dThunderCloud::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); return true; }
	bool dThunderCloud::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); return true; }
	bool dThunderCloud::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); return true; }
	bool dThunderCloud::collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); return true; }
	bool dThunderCloud::collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); return true; }

	bool dThunderCloud::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) { return BigHanaFireball(this, apThis, apOther); }
	bool dThunderCloud::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther) { return BigHanaFireball(this, apThis, apOther); }

	bool dThunderCloud::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (apThis->info.category2 == 0x9) { return true; }
		PlaySound(this, SE_EMY_DOWN);
		doStateChange(&StateID_DieFall);
		return true;
	}
	bool dThunderCloud::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->collisionCat13_Hammer(apThis, apOther);
		return true;
	}
	bool dThunderCloud::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (apThis->info.category2 == 0x9) { return true; }
		dEn_c::collisionCat3_StarPower(apThis, apOther);
		this->collisionCat13_Hammer(apThis, apOther);
		return true;
	}


	// These handle the ice crap
	void dThunderCloud::_vf148() {
		dEn_c::_vf148();
		doStateChange(&StateID_DieFall);
	}
	void dThunderCloud::_vf14C() {
		dEn_c::_vf14C();
		doStateChange(&StateID_DieFall);
	}

	extern "C" void sub_80024C20(void);
	extern "C" void __destroy_arr(void*, void(*)(void), int, int);
	//extern "C" __destroy_arr(struct DoSomethingCool, void(*)(void), int cnt, int bar);

	bool dThunderCloud::CreateIceActors() {
		this->Lightning.removeFromList();

		struct DoSomethingCool my_struct = { 0, (Vec){pos.x, pos.y - 16.0f, pos.z}, {1.75, 1.4, 1.5}, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	    this->frzMgr.Create_ICEACTORs( (void*)&my_struct, 1 );
	    __destroy_arr( (void*)&my_struct, sub_80024C20, 0x3C, 1 );
	    return true;
	}


void dThunderCloud::dieFall_Begin() {
	this->Lightning.removeFromList();
	this->timer = 0;
	this->dEn_c::dieFall_Begin();
}
void dThunderCloud::dieFall_Execute() {
	if (this->killFlag == 1) { return; }

	this->timer = this->timer + 1;

	this->dying = this->dying + 0.15;

	this->pos.x = this->pos.x + 0.15;
	this->pos.y = this->pos.y - ((-0.2 * (this->dying*this->dying)) + 5);

	this->dEn_c::dieFall_Execute();

	if (this->timer > 450) {

		if (((this->settings >> 28) > 0) || (stationary)) {
			this->Delete(1);
			this->killFlag = 1;
			return;
		}

		dStageActor_c *Player = GetSpecificPlayerActor(0);
		if (Player == 0) { Player = GetSpecificPlayerActor(1); }
		if (Player == 0) { Player = GetSpecificPlayerActor(2); }
		if (Player == 0) { Player = GetSpecificPlayerActor(3); }


		if (Player == 0) {
			this->pos.x = 0;
		} else {
			this->pos.x = Player->pos.x - 300;
		}

		this->pos.y = this->Baseline;

		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_en_blockcloud", 0, &pos, &nullRot, &oneVec);

		scale.x = scale.y = scale.z = 0.0f;
		this->aPhysics.addToList();
		doStateChange(&StateID_Follow);
	}
}


void dThunderCloud::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->anm.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->anm, unk2);
	this->anm.setUpdateRate(rate);
}

int dThunderCloud::onCreate() {

	// Setup the model
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->resFile.data = getResource("tcloud", "g3d/tcloud.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("cloud");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);

	bool ret;
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("cloud_wait");
	ret = this->anm.setup(mdl, anmChr, &this->allocator, 0);

	allocator.unlink();



	scale = (Vec){1.2f, 1.2f, 1.2f};

	// Scale and Physics
	ActivePhysics::Info Cloud;
	Cloud.xDistToCenter = 0.0;
	Cloud.yDistToCenter = 0.0;
	Cloud.category1 = 0x3;
	Cloud.category2 = 0x0;
	Cloud.bitfield1 = 0x4F;

	Cloud.bitfield2 = 0xffba7ffe;
	Cloud.xDistToEdge = 18.0f * scale.x;
	Cloud.yDistToEdge = 12.0f * scale.y;

	Cloud.unkShort1C = 0;
	Cloud.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &Cloud);
	this->aPhysics.addToList();

	below.x = 0;
	below.y = 0;
	collMgr.init(this, &below, 0, 0);

	// Some Settings
	this->Baseline = this->pos.y;
	this->dying = -5;
	this->killFlag = 0;
	this->pos.z = 5750.0f; // sun

	stationary 		= this->settings & 0xF;

	char eventNum	= (this->settings >> 16) & 0xFF;
	usingEvents = (stationary != 0) && (eventNum != 0);

	this->eventFlag = (u64)1 << (eventNum - 1);


	// State Change!
	if (stationary) { doStateChange(&StateID_Wait); }
	else 			{ doStateChange(&StateID_Follow); }

	this->onExecute();
	return true;
}

int dThunderCloud::onDelete() {
	return true;
}

int dThunderCloud::onExecute() {
	if (scale.x < 1.0f)
		scale.x = scale.y = scale.z = scale.x + 0.0375f;
	else
		scale.x = scale.y = scale.z = 1.0f;

	acState.execute();
	updateModelMatrices();
	bodyModel._vf1C();

	if ((dFlagMgr_c::instance->flags & this->eventFlag) && (!stationary)) {
		if (this->killFlag == 0 && acState.getCurrentState()->isNotEqual(&StateID_DieFall)) {
			this->kill();
			this->pos.y = this->pos.y + 800.0;
			this->killFlag = 1;
			doStateChange(&StateID_DieFall);
		}
	}

	return true;
}

int dThunderCloud::onDraw() {
	bodyModel.scheduleForDrawing();

	return true;
}


void dThunderCloud::updateModelMatrices() {
	// This won't work with wrap because I'm lazy.
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


// Follow State

void dThunderCloud::beginState_Follow() {
	this->timer = 0;
	this->bindAnimChr_and_setUpdateRate("cloud_wait", 1, 0.0, 1.0);
	this->rot.x = 0;
	this->rot.y = 0;
	this->rot.z = 0;
	PlaySound(this, SE_AMB_THUNDER_CLOUD);
}
void dThunderCloud::executeState_Follow() {

	S16Vec nullRot = {0,0,0};
	Vec efScale = {1.5f, 1.5f, 1.5f};
	charge.spawn("Wm_mr_electricshock_biri02_s", 0, &pos, &nullRot, &efScale);

	if(this->anm.isAnimationDone()) {
		this->anm.setCurrentFrame(0.0); }

	if (this->timer > 200) { this->doStateChange(&StateID_Lightning); }

	this->direction = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);

	float speedDelta;
	speedDelta = 0.05;

	if (this->direction == 0) { // Going Left
		this->speed.x = this->speed.x + speedDelta; //

		if (this->speed.x < 0) { this->speed.x += (speedDelta / 1.5); }
		if (this->speed.x < -6.0) { this->speed.x += (speedDelta * 2.0); }
	}
	else { // Going Right
		this->speed.x = this->speed.x - speedDelta;

		if (this->speed.x > 0) { this->speed.x -= (speedDelta / 1.5); }
		if (this->speed.x > 6.0) { this->speed.x -= (speedDelta * 2.0); }
	}

	this->HandleXSpeed();

	float yDiff;
	yDiff = (this->Baseline - this->pos.y) / 8;
	this->speed.y = yDiff;

	this->HandleYSpeed();

	this->UpdateObjectPosBasedOnSpeedValuesReal();

	this->timer = this->timer + 1;
}
void dThunderCloud::endState_Follow() {
	this->speed.y = 0;
}


// Wait State

void dThunderCloud::beginState_Wait() {
	this->timer = 0;
	this->bindAnimChr_and_setUpdateRate("cloud_wait", 1, 0.0, 1.0);
	this->rot.x = 0;
	this->rot.y = 0;
	this->rot.z = 0;
	PlaySound(this, SE_AMB_THUNDER_CLOUD);
}
void dThunderCloud::executeState_Wait() {

	S16Vec nullRot = {0,0,0};
	Vec efScale = {1.5f, 1.5f, 1.5f};
	charge.spawn("Wm_mr_electricshock_biri02_s", 0, &pos, &nullRot, &efScale);

	if(this->anm.isAnimationDone()) {
		this->anm.setCurrentFrame(0.0); }

	if ((this->settings >> 16) & 0xFF) {
		if (dFlagMgr_c::instance->flags & this->eventFlag) {
			this->doStateChange(&StateID_Lightning);
		}
	}
	else {
		if (this->timer > 200) { this->doStateChange(&StateID_Lightning); }
		timer += 1;
	}
}
void dThunderCloud::endState_Wait() { }


// Lightning State
static void lightningCallback(ActivePhysics *one, ActivePhysics *two) {
	if (one->owner->name == WM_BUBBLE && two->owner->name == WM_BUBBLE)
		return;
	dEn_c::collisionCallback(one, two);
}

void dThunderCloud::lightningStrike() {
	PlaySound(this, SE_OBJ_KAZAN_ERUPTION);

	float boltsize = (leader-14.0)/2;
	float boltpos = -boltsize - 14.0;

	ActivePhysics::Info Shock;
	Shock.xDistToCenter = 0.0;
	Shock.yDistToCenter = boltpos;
	Shock.category1 = 0x3;
	Shock.category2 = 0x9;
	Shock.bitfield1 = 0x4D;

	Shock.bitfield2 = 0x420;
	Shock.xDistToEdge = 12.0;
	Shock.yDistToEdge = boltsize;

	Shock.unkShort1C = 0;
	Shock.callback = &dEn_c::collisionCallback;

	this->Lightning.initWithStruct(this, &Shock);
	this->Lightning.addToList();
}

void dThunderCloud::beginState_Lightning() {
	float backupY = pos.y, backupYSpeed = speed.y;

	speed.x = 0.0;
	speed.y = -1.0f;

	u32 result = 0;
	while (result == 0 && below.y > (-30 << 16)) {
		pos.y = backupY;
		below.y -= 0x4000;
		//OSReport("Sending out leader to %d", below.y>>12);

		result = collMgr.calculateBelowCollisionWithSmokeEffect();
		if (result == 0) {
			u32 tb1 = collMgr.getTileBehaviour1At(pos.x, pos.y + (below.y >> 12), 0);
			if (tb1 & 0x8000 && !(tb1 & 0x20))
				result = 1;
		}
		//OSReport("Result %d", result);
	}

	if (result == 0) {
		OSReport("Couldn't find any ground, falling back to 13 tiles distance");

		leader = 13 * 16;
	} else {
		OSReport("Lightning strikes at %d", below.y>>12);

		leader = -(below.y >> 12);
	}
	below.y = 0;

	pos.y = backupY;
	speed.y = backupYSpeed;

	if (usingEvents) {
		timer = 2;
		this->bindAnimChr_and_setUpdateRate("thundershoot", 1, 0.0, 1.0);
		lightningStrike();
	} else {
		timer = 0;
	}
}
void dThunderCloud::executeState_Lightning() {
	S16Vec nullRot = {0,0,0};
	Vec efScale = {1.5f, 1.5f, 1.5f};

	switch (timer) {
		case 0:
			charge.spawn("Wm_en_birikyu", 0, &pos, &nullRot, &efScale);
			break;
		case 1:
			charge.spawn("Wm_en_birikyu", 0, &pos, &nullRot, &efScale);
			break;
		case 2:
			PlaySound(this, SE_BOSS_JR_ELEC_APP);
			PlaySound(this, SE_BOSS_JR_DAMAGE_ELEC);

			{
			float boltsize = (leader-14.0)/2;
			float boltpos = -boltsize - 14.0;

			Vec efPos = {pos.x, pos.y + boltpos, pos.z};
			Vec otherEfScale = {1.0f, boltsize/36.0f, 1.0f};
			bolt.spawn("Wm_jr_electricline", 0, &efPos, &nullRot, &otherEfScale);
			}
			break;
		case 3:
			this->Lightning.removeFromList();
			charge.spawn("Wm_mr_electricshock_biri02_s", 0, &pos, &nullRot, &efScale);
			break;
		case 4:
			charge.spawn("Wm_mr_electricshock_biri02_s", 0, &pos, &nullRot, &efScale);
			break;
		case 5:
			if (stationary) { doStateChange(&StateID_Wait); }
			else 			{ doStateChange(&StateID_Follow); }
			break;
		default:
			charge.spawn("Wm_mr_electricshock_biri02_s", 0, &pos, &nullRot, &efScale);
			break;
	}

	if(this->anm.isAnimationDone() && this->anm.getCurrentFrame() != 0.0) {
		if (timer == 2 && usingEvents) {
			if (dFlagMgr_c::instance->flags & eventFlag) {
			} else {
				this->Lightning.removeFromList();
				doStateChange(&StateID_Wait);
			}
		} else {
			timer++;
			if (timer == 2) {
				this->bindAnimChr_and_setUpdateRate("thundershoot", 1, 0.0, 1.0);
				lightningStrike();
			} else if (timer == 3) {
				this->bindAnimChr_and_setUpdateRate("cloud_wait", 1, 0.0, 1.0);
			}
		}
		this->anm.setCurrentFrame(0.0);
	}
}

void dThunderCloud::endState_Lightning() {
	this->timer = 0;
}


// Thundercloud center = 0
// Thundercloud bottom = -12
// Thundercloud boltpos = -boltsize/2 - 14.0
// Thundercloud boltsize = (leader-14.0)/2
// Thundercloud effSize = 36.0 [*2]
// Thundercloud effScale = boltsize / effSize
// Thundercloud effPos = -boltsize/2 - 14.0


