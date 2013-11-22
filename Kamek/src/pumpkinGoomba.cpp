#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>

const char* GParcNameList [] = {
	"kuribo",
	"pumpkin",
	"wing",
	NULL
};

class dGoombaPie : public dEn_c {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	m3d::mdl_c bodyModel;
	m3d::mdl_c burstModel;

	dStageActor_c *Goomber;
	u32 timer;
	bool isBursting;

	static dGoombaPie *build();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);

	void _vf148();
	void _vf14C();
	bool CreateIceActors();
	void addScoreWhenHit(void *other);

	USING_STATES(dGoombaPie);
	DECLARE_STATE(Follow);
	DECLARE_STATE(Burst);
};

dGoombaPie *dGoombaPie::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dGoombaPie));
	return new(buffer) dGoombaPie;
}

///////////////////////
// Externs and States
///////////////////////
	extern "C" void *EN_LandbarrelPlayerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther);
	extern "C" int SmoothRotation(short* rot, u16 amt, int unk2);
	extern "C" char usedForDeterminingStatePress_or_playerCollision(dEn_c* t, ActivePhysics *apThis, ActivePhysics *apOther, int unk1);
	extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);

	CREATE_STATE(dGoombaPie, Follow);
	CREATE_STATE(dGoombaPie, Burst);


////////////////////////
// Collision Functions
////////////////////////

	void pieCollisionCallback(ActivePhysics *one, ActivePhysics *two) {
		if (two->owner->name == EN_KURIBO) { return; }
		if (two->owner->name == EN_PATA_KURIBO) { return; }
		dEn_c::collisionCallback(one, two);
	}

	void dGoombaPie::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {

		char hitType;
		hitType = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 0);

		if(hitType == 1) {	// regular jump
			apOther->someFlagByte |= 2;
			doStateChange(&StateID_Burst);
		}
		else if(hitType == 3) {	// spinning jump or whatever?
			apOther->someFlagByte |= 2;
			doStateChange(&StateID_Burst);
		}
		else if(hitType == 0) {
			EN_LandbarrelPlayerCollision(this, apThis, apOther);
			if (this->pos.x > apOther->owner->pos.x) {
				this->direction = 1;
			}
			else {
				this->direction = 0;
			}
			doStateChange(&StateID_Burst);
		}

		// fix multiple player collisions via megazig
		deathInfo.isDead = 0;
		this->flags_4FC |= (1<<(31-7));
		this->counter_504[apOther->owner->which_player] = 0;
	}

	void dGoombaPie::spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) {}
	void dGoombaPie::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) { this->playerCollision(apThis, apOther); }
	bool dGoombaPie::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) { doStateChange(&StateID_Burst); return true; }
	bool dGoombaPie::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) { doStateChange(&StateID_Burst); return true; }
	bool dGoombaPie::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) { doStateChange(&StateID_Burst); return true; }
	bool dGoombaPie::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) { doStateChange(&StateID_Burst); return true; }
	bool dGoombaPie::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther){ doStateChange(&StateID_Burst); return true; }
	bool dGoombaPie::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) { doStateChange(&StateID_Burst); return true; }
	bool dGoombaPie::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther){ doStateChange(&StateID_Burst); return true; }
	bool dGoombaPie::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther){ doStateChange(&StateID_DieSmoke); return true; }
	bool dGoombaPie::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) { doStateChange(&StateID_DieSmoke); return true; }

	// These handle the ice crap
	void dGoombaPie::_vf148() {
		dEn_c::_vf148();
		doStateChange(&StateID_Burst);
	}
	void dGoombaPie::_vf14C() {
		dEn_c::_vf14C();
		doStateChange(&StateID_Burst);
	}

	DoSomethingCool goombIceBlock;

	extern "C" void sub_80024C20(void);
	extern "C" void __destroy_arr(void*, void(*)(void), int, int);

	bool dGoombaPie::CreateIceActors()
	{
	    struct DoSomethingCool goombIceBlock = { 0, this->pos, {2.5, 2.5, 2.5}, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	    this->frzMgr.Create_ICEACTORs( (void*)&goombIceBlock, 1 );
	    __destroy_arr( (void*)&goombIceBlock, sub_80024C20, 0x3C, 1 );
	    return true;
	}

	void dGoombaPie::addScoreWhenHit(void *other) {}


int dGoombaPie::onCreate() {

	// Model creation
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->resFile.data = getResource("pumpkin", "g3d/model.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("Pumpkin");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	// SetupTextures_Map(&bodyModel, 0);

	mdl = this->resFile.GetResMdl("FX_Pumpkin");
	burstModel.setup(mdl, &allocator, 0x224, 1, 0);
	// SetupTextures_Map(&burstModel, 0);

	allocator.unlink();


	// Other shit
	isBursting = false;
	this->scale = (Vec){0.39, 0.39, 0.39};

	ActivePhysics::Info HitMeBaby;

	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = 12.0;

	HitMeBaby.xDistToEdge = 8.0;
	HitMeBaby.yDistToEdge = 14.0;

	HitMeBaby.category1 = 0x3;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x01;
	HitMeBaby.bitfield2 = 0x820A0;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &pieCollisionCallback;

	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();


	// Remember to follow a goomba
	if ((settings & 0xF) == 0) {
		Goomber = (dStageActor_c*)create(EN_KURIBO, 0, &pos, &rot, 0); }
	else {
		Goomber = (dStageActor_c*)create(EN_PATA_KURIBO, 0, &pos, &rot, 0); }

	// State Changers
	doStateChange(&StateID_Follow);

	this->onExecute();
	return true;
}

int dGoombaPie::onDelete() {
	return true;
}

int dGoombaPie::onExecute() {
	acState.execute();
	this->pos = Goomber->pos;
	this->rot = Goomber->rot;
	return true;
}

int dGoombaPie::onDraw() {
	matrix.translation(pos.x, pos.y + 4.0, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	if (isBursting) {
		burstModel.setDrawMatrix(matrix);
		burstModel.setScale(&scale);
		burstModel.calcWorld(false);
		burstModel.scheduleForDrawing();
	} else {
		bodyModel.setDrawMatrix(matrix);
		bodyModel.setScale(&scale);
		bodyModel.calcWorld(false);
		bodyModel.scheduleForDrawing();
	}

	return true;
}



///////////////
// Follow State
///////////////
	void dGoombaPie::beginState_Follow() { }
	void dGoombaPie::executeState_Follow() { }
	void dGoombaPie::endState_Follow() { }

///////////////
// Burst State
///////////////
	void dGoombaPie::beginState_Burst() {
		this->timer = 0;
		isBursting = true;
		this->removeMyActivePhysics();
		S16Vec nullRot = {0,0,0};
		Vec twoVec = {2.0f, 2.0f, 2.0f};
		SpawnEffect("Wm_ob_eggbreak_yw", 0, &pos, &nullRot, &twoVec);
	}
	void dGoombaPie::executeState_Burst() {
		this->Delete(1);
	}
	void dGoombaPie::endState_Burst() { }

