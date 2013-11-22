#include <common.h>
#include <game.h>

const char *FlipBlockFileList[] = {"block_rotate", 0};

class daEnFlipBlock_c : public daEnBlockMain_c {
public:
	Physics::Info physicsInfo;

	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	void calledWhenUpMoveExecutes();
	void calledWhenDownMoveExecutes();

	void blockWasHit(bool isDown);

	bool playerOverlaps();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	m3d::mdl_c model;

	int flipsRemaining;

	USING_STATES(daEnFlipBlock_c);
	DECLARE_STATE(Wait);
	DECLARE_STATE(Flipping);

	static daEnFlipBlock_c *build();
};


CREATE_STATE(daEnFlipBlock_c, Wait);
CREATE_STATE(daEnFlipBlock_c, Flipping);


int daEnFlipBlock_c::onCreate() {
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	resFile.data = getResource("block_rotate", "g3d/block_rotate.brres");
	model.setup(resFile.GetResMdl("block_rotate"), &allocator, 0, 1, 0);
	SetupTextures_MapObj(&model, 0);

	allocator.unlink();



	blockInit(pos.y);

	physicsInfo.x1 = -8;
	physicsInfo.y1 = 8;
	physicsInfo.x2 = 8;
	physicsInfo.y2 = -8;

	physicsInfo.otherCallback1 = &daEnBlockMain_c::OPhysicsCallback1;
	physicsInfo.otherCallback2 = &daEnBlockMain_c::OPhysicsCallback2;
	physicsInfo.otherCallback3 = &daEnBlockMain_c::OPhysicsCallback3;

	physics.setup(this, &physicsInfo, 3, currentLayerID);
	physics.flagsMaybe = 0x260;
	physics.callback1 = &daEnBlockMain_c::PhysicsCallback1;
	physics.callback2 = &daEnBlockMain_c::PhysicsCallback2;
	physics.callback3 = &daEnBlockMain_c::PhysicsCallback3;
	physics.addToList();

	doStateChange(&daEnFlipBlock_c::StateID_Wait);

	return true;
}


int daEnFlipBlock_c::onDelete() {
	physics.removeFromList();

	return true;
}


int daEnFlipBlock_c::onExecute() {
	acState.execute();
	physics.update();
	blockUpdate();

	// now check zone bounds based on state
	if (acState.getCurrentState()->isEqual(&StateID_Wait)) {
		checkZoneBoundaries(0);
	}

	return true;
}


int daEnFlipBlock_c::onDraw() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	model.setDrawMatrix(matrix);
	model.setScale(&scale);
	model.calcWorld(false);
	model.scheduleForDrawing();

	return true;
}


daEnFlipBlock_c *daEnFlipBlock_c::build() {

	void *buffer = AllocFromGameHeap1(sizeof(daEnFlipBlock_c));
	daEnFlipBlock_c *c = new(buffer) daEnFlipBlock_c;


	return c;
}


void daEnFlipBlock_c::blockWasHit(bool isDown) {
	pos.y = initialY;

	doStateChange(&StateID_Flipping);
}



void daEnFlipBlock_c::calledWhenUpMoveExecutes() {
	if (initialY >= pos.y)
		blockWasHit(false);
}

void daEnFlipBlock_c::calledWhenDownMoveExecutes() {
	if (initialY <= pos.y)
		blockWasHit(true);
}



void daEnFlipBlock_c::beginState_Wait() {
}

void daEnFlipBlock_c::endState_Wait() {
}

void daEnFlipBlock_c::executeState_Wait() {
	int result = blockResult();

	if (result == 0)
		return;

	if (result == 1) {
		doStateChange(&daEnBlockMain_c::StateID_UpMove);
		anotherFlag = 2;
		isGroundPound = false;
	} else {
		doStateChange(&daEnBlockMain_c::StateID_DownMove);
		anotherFlag = 1;
		isGroundPound = true;
	}
}


void daEnFlipBlock_c::beginState_Flipping() {
	flipsRemaining = 7;
	physics.removeFromList();
}
void daEnFlipBlock_c::executeState_Flipping() {
	if (isGroundPound)
		rot.x += 0x800;
	else
		rot.x -= 0x800;

	if (rot.x == 0) {
		flipsRemaining--;
		if (flipsRemaining <= 0) {
			if (!playerOverlaps())
				doStateChange(&StateID_Wait);
		}
	}
}
void daEnFlipBlock_c::endState_Flipping() {
	physics.setup(this, &physicsInfo, 3, currentLayerID);
	physics.addToList();
}



bool daEnFlipBlock_c::playerOverlaps() {
	dStageActor_c *player = 0;

	Vec myBL = {pos.x - 8.0f, pos.y - 8.0f, 0.0f};
	Vec myTR = {pos.x + 8.0f, pos.y + 8.0f, 0.0f};

	while ((player = (dStageActor_c*)fBase_c::search(PLAYER, player)) != 0) {
		float centerX = player->pos.x + player->aPhysics.info.xDistToCenter;
		float centerY = player->pos.y + player->aPhysics.info.yDistToCenter;

		float left = centerX - player->aPhysics.info.xDistToEdge;
		float right = centerX + player->aPhysics.info.xDistToEdge;

		float top = centerY + player->aPhysics.info.yDistToEdge;
		float bottom = centerY - player->aPhysics.info.yDistToEdge;

		Vec playerBL = {left, bottom + 0.1f, 0.0f};
		Vec playerTR = {right, top - 0.1f, 0.0f};

		if (RectanglesOverlap(&playerBL, &playerTR, &myBL, &myTR))
			return true;
	}

	return false;
}

