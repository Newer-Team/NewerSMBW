#include <common.h>
#include <game.h>

// Patches MIST_INTERMITTENT (sprite 239)

class daEnEventBlock_c : public daEnBlockMain_c {
public:
	enum Mode {
		TOGGLE_EVENT = 0,
		SWAP_EVENTS = 1
	};

	TileRenderer tile;
	Physics::Info physicsInfo;

	u8 event1;
	u8 event2;
	Mode mode;

	void equaliseEvents();

	int onCreate();
	int onDelete();
	int onExecute();

	void calledWhenUpMoveExecutes();
	void calledWhenDownMoveExecutes();

	void blockWasHit(bool isDown);

	USING_STATES(daEnEventBlock_c);
	DECLARE_STATE(Wait);

	static daEnEventBlock_c *build();
};


CREATE_STATE(daEnEventBlock_c, Wait);


int daEnEventBlock_c::onCreate() {
	blockInit(pos.y);

	physicsInfo.x1 = -8;
	physicsInfo.y1 = 16;
	physicsInfo.x2 = 8;
	physicsInfo.y2 = 0;

	physicsInfo.otherCallback1 = &daEnBlockMain_c::OPhysicsCallback1;
	physicsInfo.otherCallback2 = &daEnBlockMain_c::OPhysicsCallback2;
	physicsInfo.otherCallback3 = &daEnBlockMain_c::OPhysicsCallback3;

	physics.setup(this, &physicsInfo, 3, currentLayerID);
	physics.flagsMaybe = 0x260;
	physics.callback1 = &daEnBlockMain_c::PhysicsCallback1;
	physics.callback2 = &daEnBlockMain_c::PhysicsCallback2;
	physics.callback3 = &daEnBlockMain_c::PhysicsCallback3;
	physics.addToList();

	TileRenderer::List *list = dBgGm_c::instance->getTileRendererList(0);
	list->add(&tile);

	tile.x = pos.x - 8;
	tile.y = -(16 + pos.y);
	tile.tileNumber = 0x97;

	mode = (Mode)((settings >> 16) & 0xF);
	event1 = ((settings >> 8) & 0xFF) - 1;
	event2 = (settings & 0xFF) - 1;

	equaliseEvents();

	doStateChange(&daEnEventBlock_c::StateID_Wait);

	return true;
}


int daEnEventBlock_c::onDelete() {
	TileRenderer::List *list = dBgGm_c::instance->getTileRendererList(0);
	list->remove(&tile);

	physics.removeFromList();

	return true;
}


int daEnEventBlock_c::onExecute() {
	acState.execute();
	physics.update();
	blockUpdate();

	tile.setPosition(pos.x-8, -(16+pos.y), pos.z);
	tile.setVars(scale.x);

	equaliseEvents();

	bool isActive = dFlagMgr_c::instance->active(event2);

	tile.tileNumber = (isActive ? 0x96 : 0x97);

	// now check zone bounds based on state
	if (acState.getCurrentState()->isEqual(&StateID_Wait)) {
		checkZoneBoundaries(0);
	}

	return true;
}


daEnEventBlock_c *daEnEventBlock_c::build() {

	void *buffer = AllocFromGameHeap1(sizeof(daEnEventBlock_c));
	daEnEventBlock_c *c = new(buffer) daEnEventBlock_c;


	return c;
}


void daEnEventBlock_c::equaliseEvents() {
	if (mode != SWAP_EVENTS)
		return;

	bool f1 = dFlagMgr_c::instance->active(event1);
	bool f2 = dFlagMgr_c::instance->active(event2);

	if (!f1 && !f2) {
		dFlagMgr_c::instance->set(event1, 0, true, false, false);
	}

	if (f1 && f2) {
		dFlagMgr_c::instance->set(event2, 0, false, false, false);
	}
}


void daEnEventBlock_c::blockWasHit(bool isDown) {
	pos.y = initialY;

	if (mode == TOGGLE_EVENT) {
		if (dFlagMgr_c::instance->active(event2))
			dFlagMgr_c::instance->set(event2, 0, false, false, false);
		else
			dFlagMgr_c::instance->set(event2, 0, true, false, false);

	} else if (mode == SWAP_EVENTS) {
		if (dFlagMgr_c::instance->active(event1)) {
			dFlagMgr_c::instance->set(event1, 0, false, false, false);
			dFlagMgr_c::instance->set(event2, 0, true, false, false);
		} else {
			dFlagMgr_c::instance->set(event1, 0, true, false, false);
			dFlagMgr_c::instance->set(event2, 0, false, false, false);
		}
	}

	physics.setup(this, &physicsInfo, 3, currentLayerID);
	physics.addToList();
	
	doStateChange(&StateID_Wait);
}



void daEnEventBlock_c::calledWhenUpMoveExecutes() {
	if (initialY >= pos.y)
		blockWasHit(false);
}

void daEnEventBlock_c::calledWhenDownMoveExecutes() {
	if (initialY <= pos.y)
		blockWasHit(true);
}



void daEnEventBlock_c::beginState_Wait() {
}

void daEnEventBlock_c::endState_Wait() {
}

void daEnEventBlock_c::executeState_Wait() {
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


