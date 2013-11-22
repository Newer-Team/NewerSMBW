#include <game.h>
#include <sfx.h>
const char *ChestnutFileList[] = {"chestnut", 0};

class daEnChestnut_c : public dEn_c {
	public:
		static daEnChestnut_c *build();

		mHeapAllocator_c allocator;
		nw4r::g3d::ResFile resFile;
		m3d::mdl_c model;
		m3d::anmChr_c animation;

		void playAnimation(const char *name, bool loop = false);
		void playLoopedAnimation(const char *name) {
			playAnimation(name, true);
		}

		int objNumber;
		int starCoinNumber;
		bool ignorePlayers;
		bool breaksBlocks;
		float shakeWindow, fallWindow;

		int timeSpentExploding;

		lineSensor_s belowSensor;

		float nearestPlayerDistance();

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		void spawnObject();

		bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
		bool collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther);

		bool CreateIceActors();

		u32 canBePowed();
		void powBlockActivated(bool isNotMPGP);

		USING_STATES(daEnChestnut_c);
		DECLARE_STATE(Idle);
		DECLARE_STATE(Shake);
		DECLARE_STATE(Fall);
		DECLARE_STATE(Explode);
};

CREATE_STATE(daEnChestnut_c, Idle);
CREATE_STATE(daEnChestnut_c, Shake);
CREATE_STATE(daEnChestnut_c, Fall);
CREATE_STATE(daEnChestnut_c, Explode);

daEnChestnut_c *daEnChestnut_c::build() {
	void *buf = AllocFromGameHeap1(sizeof(daEnChestnut_c));
	return new(buf) daEnChestnut_c;
}


int daEnChestnut_c::onCreate() {
	// Get settings
	int texNum = settings & 0xF;
	int rawScale = (settings & 0xF0) >> 4;
	starCoinNumber = (settings & 0xF00) >> 8;
	ignorePlayers = ((settings & 0x1000) != 0);
	breaksBlocks = ((settings & 0x2000) != 0);
	objNumber = (settings & 0xF0000) >> 16;

	if ((settings & 0x4000) != 0) {
		shakeWindow = 96.0f;
		fallWindow = 64.0f;
	} else {
		shakeWindow = 64.0f;
		fallWindow = 32.0f;
	}

	// Set up models
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	char rfName[64];
	sprintf(rfName, "g3d/t%02d.brres", texNum);

	resFile.data = getResource("chestnut", rfName);

	nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("kuribo_iga");
	nw4r::g3d::ResAnmChr resAnm = resFile.GetResAnmChr("wait");

	model.setup(resMdl, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&model, 0);

	animation.setup(resMdl, resAnm, &allocator, 0);

	allocator.unlink();


	// Scale us
	scale.x = scale.y = scale.z = (1.0f + (float(rawScale) * 0.5f));

	// Physics and crap
	ActivePhysics::Info ccInfo;
	ccInfo.xDistToCenter = 0.0f;
	ccInfo.xDistToEdge = 12.0f * scale.x;
	ccInfo.yDistToCenter = 1.0f + (12.0f * scale.y);
	ccInfo.yDistToEdge = 12.0f * scale.y;

	ccInfo.category1 = 3;
	ccInfo.category2 = 0;
	ccInfo.bitfield1 = 0x6F;
	ccInfo.bitfield2 = 0xFFBAFFFE;

	ccInfo.unkShort1C = 0;
	ccInfo.callback = &dEn_c::collisionCallback;

	aPhysics.initWithStruct(this, &ccInfo);
	aPhysics.addToList();

	// WE'RE READY
	doStateChange(&StateID_Idle);

	return true;
}

void daEnChestnut_c::playAnimation(const char *name, bool loop) {
	animation.bind(&model, resFile.GetResAnmChr(name), !loop);
	model.bindAnim(&animation, 0.0f);
	animation.setUpdateRate(1.0f);
}

int daEnChestnut_c::onDelete() {
	aPhysics.removeFromList();
	return true;
}

int daEnChestnut_c::onExecute() {
	acState.execute();

	matrix.translation(pos.x, pos.y, pos.z);

	model.setDrawMatrix(matrix);
	model.setScale(&scale);
	model.calcWorld(false);

	model._vf1C();

	return true;
}

int daEnChestnut_c::onDraw() {
	model.scheduleForDrawing();

	return true;
}

float daEnChestnut_c::nearestPlayerDistance() {
	float bestSoFar = 10000.0f;

	for (int i = 0; i < 4; i++) {
		if (dAcPy_c *player = dAcPy_c::findByID(i)) {
			if (strcmp(player->states2.getCurrentState()->getName(), "dAcPy_c::StateID_Balloon")) {
				float thisDist = abs(player->pos.x - pos.x);
				if (thisDist < bestSoFar)
					bestSoFar = thisDist;
			}
		}
	}

	return bestSoFar;
}



void daEnChestnut_c::beginState_Idle() {
	playLoopedAnimation("wait");
}
void daEnChestnut_c::endState_Idle() { }


void daEnChestnut_c::executeState_Idle() {
	if (ignorePlayers)
		return;

	float dist = nearestPlayerDistance();

	if (dist < fallWindow)
		doStateChange(&StateID_Fall);
	else if (dist < shakeWindow)
		doStateChange(&StateID_Shake);
}



void daEnChestnut_c::beginState_Shake() {
	playLoopedAnimation("shake");
	animation.setUpdateRate(2.0f);

	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_PLY_CLIMB_KUSARI, 1);
}
void daEnChestnut_c::endState_Shake() { }

void daEnChestnut_c::executeState_Shake() {
	float dist = nearestPlayerDistance();

	if (dist >= shakeWindow)
		doStateChange(&StateID_Idle);
	else if (dist < fallWindow)
		doStateChange(&StateID_Fall);
}



void daEnChestnut_c::beginState_Fall() {
	animation.setUpdateRate(0.0f); // stop animation

	int size = 12*scale.x;

	belowSensor.flags = SENSOR_LINE;
	if (breaksBlocks)
		belowSensor.flags |= SENSOR_10000000 | SENSOR_BREAK_BLOCK | SENSOR_BREAK_BRICK;
	// 10000000 makes it pass through bricks

	belowSensor.lineA = -size << 12;
	belowSensor.lineB = size << 12;
	belowSensor.distanceFromCenter = 0;

	collMgr.init(this, &belowSensor, 0, 0);

	speed.y = 0.0f;
	y_speed_inc = -0.1875f;
	max_speed.y = -4.0f;

	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_DEMO_OP_PRESENT_THROW_2308f, 1);
}

void daEnChestnut_c::endState_Fall() { }

void daEnChestnut_c::executeState_Fall() {
	HandleYSpeed();
	doSpriteMovement();
	UpdateObjectPosBasedOnSpeedValuesReal();

	if (collMgr.calculateBelowCollision() & (~0x400000)) {
		doStateChange(&StateID_Explode);
	}
}



void daEnChestnut_c::beginState_Explode() {
	OSReport("Entering Explode\n");
	playAnimation("break");
	animation.setUpdateRate(2.0f);

	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_DEMO_OP_LAND_JR_0983f, 1);

	timeSpentExploding = 0;
}
void daEnChestnut_c::endState_Explode() { }

void daEnChestnut_c::executeState_Explode() {
	timeSpentExploding++;

	if (timeSpentExploding == 10) {
		S16Vec efRot = {0,0,0};
		SpawnEffect("Wm_en_burst_ss", 0, &pos, &efRot, &scale);
		spawnObject();
	}

	if (animation.isAnimationDone()) {
		Delete(1);
	}
}



bool daEnChestnut_c::CreateIceActors() {
	animation.setUpdateRate(0.0f);
	
	IceActorSpawnInfo info;
	info.flags = 0;
	info.pos = pos;
	info.pos.y -= (6.0f * info.scale.y);
	info.scale.x = info.scale.y = info.scale.z = scale.x * 1.35f;
	for (int i = 0; i < 8; i++)
		info.what[0] = 0.0f;

	return frzMgr.Create_ICEACTORs(&info, 1);
}

u32 daEnChestnut_c::canBePowed() {
	return true;
}
void daEnChestnut_c::powBlockActivated(bool isNotMPGP) {
	if (!isNotMPGP)
		return;

	dStateBase_c *state = acState.getCurrentState();
	if (state == &StateID_Idle || state == &StateID_Shake)
		doStateChange(&StateID_Fall);
}

bool daEnChestnut_c::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
	SpawnEffect("Wm_en_igafirehit", 0, &pos, &rot, &scale);

	if (acState.getCurrentState() != &StateID_Explode)
		doStateChange(&StateID_Explode);

	return true;
}

bool daEnChestnut_c::collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->_vf220(apOther->owner);
	return true;
}


void daEnChestnut_c::spawnObject() {
	VEC3 acPos = pos;

	static const u32 things[] = {
		EN_KURIBO, 0,
		EN_TOGEZO, 0,
		EN_COIN_JUMP, 0,
		EN_ITEM, 0x05000009,
		EN_STAR_COIN, 0x10000000,
	};

	u32 acSettings = things[objNumber*2+1];

	if (objNumber == 4) {
		acSettings |= (starCoinNumber << 8);
		acPos.x -= 12.0f;
		acPos.y += 32.0f;
	}

	aPhysics.removeFromList();

	OSReport("Crap %d, %d, %08x\n", objNumber, things[objNumber*2], acSettings);
	dStageActor_c *ac =
		dStageActor_c::create((Actors)things[objNumber*2], acSettings, &acPos, 0, currentLayerID);

	S16Vec efRot = {0,0,0};
	SpawnEffect("Wm_ob_itemsndlandsmk", 0, &pos, &efRot, &scale);

	if (objNumber == 0) {
		dEn_c *en = (dEn_c*)ac;
		en->direction = 1;
		en->rot.y = -8192;
	}
}
