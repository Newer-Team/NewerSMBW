#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include "boss.h"

// #define cField(TYPE, OFFSET) (*(TYPE*)(((u32)clown) + (OFFSET)))
// #define cPlayerOccupying cField(dStageActor_c*, 0x738)
// #define cModel cField(m3d::mdl_c*, 0xFEC)
// #define cTimer cField(u32, 0xFEC+sizeof(m3d::mdl_c))
// #define cAllocator cField(mHeapAllocator_c*, 0xFD0)

#define cPlayerOccupying (*(dStageActor_c**)(((u32)(clown)) + 0x738 ))
#define cAllocator ((mHeapAllocator_c*)(((u32)(clown)) + 0xFD0 ))
#define cModel ((m3d::mdl_c*)( ((u32)(clown)) + 0xFEC ))
#define cTimer (*(u32*)((u32)(clown) + sizeof(m3d::mdl_c) + 0xFEC ))

extern "C" int PClownCarExecute(dEn_c *clown);
extern "C" void PClownCarAfterCreate(dEn_c *clown, u32);
extern "C" int PClownCarDraw(dEn_c *clown);
extern "C" void PClownCarMove(dEn_c *clown);


const char* PCCarcNameList [] = {
	"koopaJr_clown_ply",
	NULL
};



int CConDraw(dEn_c *clown) {
	// setup cannon model
	clown->matrix.translation(clown->pos.x, clown->pos.y + 8.0, clown->pos.z-100.0);
	short newrotz = -0x2000;
	short newroty = ((clown->rot.y * 0x4000) / 0x800) - 0x4000;
	short newrotx;
	if (clown->rot.x < 0x8000) {
		newrotx = -clown->rot.x;
	}
	else {
		newrotx = clown->rot.x;
	}
	// OSReport("Angle?: %x, %x", clown->rot.y, newroty);
	clown->matrix.applyRotationYXZ(&clown->rot.x, &newroty, &newrotz);

	cModel->setDrawMatrix(clown->matrix);
	Vec sc = {0.25f, 0.5f, 0.25f};
	cModel->setScale(&sc);
	cModel->calcWorld(false);

	cModel->scheduleForDrawing();


	return PClownCarDraw(clown);
	// run normal clown function
}
extern dStateBase_c JrClownEndDemoState;
extern dStateBase_c JrClownDemoWaitState;
extern dStateBase_c ClownDemoWaitState;

int CConExecute(dEn_c *clown) {
	// A REALLY TERRIBLE HACK.
	float saveX = clown->pos.x;
	float saveY = clown->pos.y;

	int ret = PClownCarExecute(clown);

	dStateBase_c *state = clown->acState.getCurrentState();
	if (state == &JrClownEndDemoState || state == &JrClownDemoWaitState || state == &ClownDemoWaitState) {
		clown->pos.x = saveX;
		clown->pos.y = saveY;
		clown->speed.x = 0.0f;
		clown->speed.y = 0.0f;
	}
	return ret;
}

void CCafterCreate(dEn_c *clown, u32 param) {

	clown->scale.x *= 1.25;
	clown->scale.y *= 1.25;
	clown->scale.z *= 1.25;

	// setup the model
	nw4r::g3d::ResFile resFile;

	cAllocator->link(-1, GameHeaps[0], 0, 0x20);

	resFile.data = getResource("koopaJr_clown_ply", "g3d/cannon.brres");
	nw4r::g3d::ResMdl mdl = resFile.GetResMdl("Cannon");
	cModel->setup(mdl, cAllocator, 0x224, 1, 0);
	SetupTextures_MapObj(cModel, 0);

	cAllocator->unlink();

	// Original AfterCreate
	PClownCarAfterCreate(clown, param);

	int playerCount = 0;
	for (int i = 0; i < 4; i++)
		if (Player_Active[i])
			playerCount++;

	if ((clown->settings & 0xF) != 0) {
		int playerID = (clown->settings & 0xF) - 1;
		if (playerID >= playerCount)
			clown->Delete(1);
	}
}

void CConExecuteMove(dEn_c *clown) {

	u8 player = cPlayerOccupying->which_player;
	// OSReport("Angle = %x, %x, %x", (GetSpecificPlayerActor(player))->rot.y, (GetSpecificPlayerActor(player))->rot.x, (GetSpecificPlayerActor(player))->rot.z);
	// OSReport("Clown = %x, %x, %x", (clown)->rot.y, (clown)->rot.x, (clown)->rot.z);

	Vec tempPos;

	u32 buttonPushed = Remocon_GetPressed(GetRemoconMng()->controllers[cPlayerOccupying->which_player]);
	if (buttonPushed & 0x0100) {

		if (cTimer > 90) {
			if (clown->direction == 0) { // Going right
				tempPos = (Vec){clown->pos.x + 32.0f, clown->pos.y + 32.0f, 3564.0f};
				dStageActor_c *spawned = CreateActor(657, 0, tempPos, 0, 0);
				spawned->speed.x = 5.0;
			}
			else {
				tempPos = (Vec){clown->pos.x - 32.0f, clown->pos.y + 32.0f, 3564.0f};
				dStageActor_c *spawned = CreateActor(657, 0, tempPos, 0, 0);
				spawned->speed.x = -5.0;
			}

			S16Vec nullRot = {0,0,0};
			Vec efScale = {0.1f, 0.1f, 0.1f};
			SpawnEffect("Wm_en_killervanish", 0, &tempPos, &nullRot, &efScale);
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_OBJ_HOUDAI_S_SHOT, 1);

		cTimer = 0;
		}
	}

	cTimer++;

	ClassWithCameraInfo *cwci = ClassWithCameraInfo::instance;
	float leftBound = cwci->screenLeft + 12.0f;
	float rightBound = (cwci->screenLeft + cwci->screenWidth) - 12.0f;
	if (clown->pos.x < leftBound)
		clown->pos.x = leftBound;
	if (clown->pos.x > rightBound)
		clown->pos.x = rightBound;

	// run normal move
	PClownCarMove(clown);
}


extern "C" m3d::mdl_c *__ct__Q23m3d5mdl_cFv(m3d::mdl_c *mdl);
extern "C" mHeapAllocator_c *__ct__16mHeapAllocator_cFv(mHeapAllocator_c *al);
extern "C" dEn_c *__ct__20daJrClownForPlayer_cFv(dEn_c *clown);

dEn_c *newClownCtor(dEn_c *clown) {
	__ct__20daJrClownForPlayer_cFv(clown);
	__ct__16mHeapAllocator_cFv(cAllocator);
	__ct__Q23m3d5mdl_cFv(cModel);
	return clown;
}


extern "C" void __dt__Q23m3d5mdl_cFv(m3d::mdl_c *mdl, u32 willDelete);
extern "C" void __dt__16mHeapAllocator_cFv(mHeapAllocator_c *al, u32 willDelete);
extern "C" void __dt__20daJrClownForPlayer_cFv(dEn_c *clown, u32 willDelete);

extern "C" u32 sAllocatorFunc__FrmHeap;

void newClownDtor(dEn_c *clown, u32 willDelete) {
	void **al = (void **)(((u32)clown) + 0x524);
	if (*al != &sAllocatorFunc__FrmHeap) {
		OSReport("oh no! bad allocator %p\n", *al);
		*al = &sAllocatorFunc__FrmHeap;
	}

	__dt__Q23m3d5mdl_cFv(cModel, 0xFFFFFFFF);
	__dt__16mHeapAllocator_cFv(cAllocator, 0xFFFFFFFF);
	__dt__20daJrClownForPlayer_cFv(clown, willDelete);
}

extern "C" void JrClownForPlayer_playAccelSound() {
	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_PLY_CROWN_ACC, 1);
}







// Below is the projectile used byt eh clown car - Replaces WM_PAKKUN

class daClownShot : public dEn_c {
	int onCreate();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	m3d::mdl_c bodyModel;

	mEf::es2 effect;
	static daClownShot *build();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
};

void daClownShot::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) { }

daClownShot *daClownShot::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daClownShot));
	return new(buffer) daClownShot;
}


int daClownShot::onCreate() {
	allocator.link(-1, GameHeaps[0], 0, 0x20);
	this->resFile.data = getResource("koopaJr_clown_ply", "g3d/houdai_ball.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("houdai_ball");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	allocator.unlink();

	ActivePhysics::Info GreatBalls;

	GreatBalls.xDistToCenter = 0.0;
	GreatBalls.yDistToCenter = 0.0;
	GreatBalls.xDistToEdge = 8.0;
	GreatBalls.yDistToEdge = 7.0;

	GreatBalls.category1 = 0x3;
	GreatBalls.category2 = 0x0;
	GreatBalls.bitfield1 = 0x6F;
	GreatBalls.bitfield2 = 0xffbafffe;
	GreatBalls.unkShort1C = 0;
	GreatBalls.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &GreatBalls);
	this->aPhysics.addToList();


	// These fucking rects do something for the tile rect
	spriteSomeRectX = 8.0f;
	spriteSomeRectY = 8.0f;
	_320 = 0.0f;
	_324 = 0.0f;

	u32 flags = SENSOR_BREAK_BRICK | SENSOR_BREAK_BLOCK | SENSOR_80000000;
	static const lineSensor_s below(flags, 12<<12, 4<<12, 0<<12);
	static const pointSensor_s above(flags, 0<<12, 12<<12);
	static const lineSensor_s adjacent(flags, 6<<12, 9<<12, 6<<12);

	collMgr.init(this, &below, &above, &adjacent);
	collMgr.calculateBelowCollisionWithSmokeEffect();


	this->speed.y = 4.0;
	this->y_speed_inc = -0.1875;

	this->onExecute();
	return true;
}

int daClownShot::onDraw() {
	matrix.translation(this->pos.x, this->pos.y, this->pos.z);

	matrix.applyRotationYXZ(&this->rot.x, &this->rot.y, &this->rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(true);

	bodyModel.scheduleForDrawing();
	return true;
}


int daClownShot::onExecute() {
	HandleXSpeed();
	HandleYSpeed();
	doSpriteMovement();

	collMgr.calculateBelowCollisionWithSmokeEffect();
	collMgr.calculateAboveCollision(0);
	collMgr.calculateAdjacentCollision();
	if (collMgr.outputMaybe) {
		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_en_burst_m", 0, &pos, &nullRot, &oneVec);
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_OBJ_TARU_BREAK, 1);
		Delete(1);
		return true;
	}

	S16Vec nullRot = {0,0,0};
	Vec efScale = {0.7f, 0.7f, 0.7f};
	effect.spawn("Wm_en_killersmoke", 0, &pos, &nullRot, &efScale);

	float rect[] = {0.0, 0.0, 8.0, 8.0};
	int ret = this->outOfZone(this->pos, (float*)&rect, this->currentZoneID);
	if(ret) {
		this->Delete(1);
	}

	return true;
}




// This is for making clown shots able to kill other shit

extern "C" bool Amp_NewPreSpriteCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	// apThis = amp, apOther = other thing
	dEn_c *amp = (dEn_c*)apThis->owner;

	if (apOther->info.category2 == 9) {
		if (amp->collisionCat9_RollingObject(apThis, apOther))
			return true;
	} else if (apOther->owner->name == WM_PAKKUN) {
		amp->killByDieFall(apOther->owner);
		return true;
	}

	return false;
}

extern "C" void KazanRock_Explode(void *kazanRock);
extern "C" void KazanRock_OriginalCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther);
extern "C" void KazanRock_CollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (apOther->owner->name == WM_PAKKUN) {
		apThis->someFlagByte |= 2;
		KazanRock_Explode(apThis->owner);
	} else {
		KazanRock_OriginalCollisionCallback(apThis, apOther);
	}
}

