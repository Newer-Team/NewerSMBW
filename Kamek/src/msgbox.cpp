#include <common.h>
#include <game.h>
#include "sfx.h"
#include "msgbox.h"

// Replaces: EN_LIFT_ROTATION_HALF (Sprite 107; Profile ID 481 @ 80AF96F8)


dMsgBoxManager_c *dMsgBoxManager_c::instance = 0;
dMsgBoxManager_c *dMsgBoxManager_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dMsgBoxManager_c));
	dMsgBoxManager_c *c = new(buffer) dMsgBoxManager_c;

	instance = c;
	return c;
}

#define ANIM_BOX_APPEAR 0
#define ANIM_BOX_DISAPPEAR 1

extern int MessageBoxIsShowing;

/*****************************************************************************/
// Events
int dMsgBoxManager_c::onCreate() {
	if (!layoutLoaded) {
		if (!layout.loadArc("msgbox.arc", false))
			return false;

		static const char *brlanNames[2] = {
			"BoxAppear.brlan",
			"BoxDisappear.brlan",
		};

		static const char *groupNames[2] = {
			"G_Box", "G_Box",
		};

		layout.build("MessageBox.brlyt");

		if (IsWideScreen()) {
			layout.layout.rootPane->scale.x = 0.7711f;
		}

		layout.loadAnimations(brlanNames, 2);
		layout.loadGroups(groupNames, (int[2]){0,1}, 2);
		layout.disableAllAnimations();

		layout.drawOrder = 140;

		layoutLoaded = true;
	}

	visible = false;

	return true;
}

int dMsgBoxManager_c::onExecute() {
	state.execute();

	layout.execAnimations();
	layout.update();

	return true;
}

int dMsgBoxManager_c::onDraw() {
	if (visible) {
		layout.scheduleForDrawing();
	}
	
	return true;
}

int dMsgBoxManager_c::onDelete() {
	instance = 0;

	MessageBoxIsShowing = false;
	if (canCancel && StageC4::instance)
		StageC4::instance->_1D = 0; // disable no-pause
	msgDataLoader.unload();

	return layout.free();
}

/*****************************************************************************/
// Load Resources
CREATE_STATE_E(dMsgBoxManager_c, LoadRes);

void dMsgBoxManager_c::executeState_LoadRes() {
	if (msgDataLoader.load("/NewerRes/Messages.bin")) {
		state.setState(&StateID_Wait);
	} else {
	}
}

/*****************************************************************************/
// Waiting
CREATE_STATE_E(dMsgBoxManager_c, Wait);

void dMsgBoxManager_c::executeState_Wait() {
	// null
}

/*****************************************************************************/
// Show Box
void dMsgBoxManager_c::showMessage(int id, bool canCancel, int delay) {
	if (!this) {
		OSReport("ADD A MESSAGE BOX MANAGER YOU MORON\n");
		return;
	}

	// get the data file
	header_s *data = (header_s*)msgDataLoader.buffer;

	const wchar_t *title = 0, *msg = 0;

	for (int i = 0; i < data->count; i++) {
		if (data->entry[i].id == id) {
			title = (const wchar_t*)((u32)data + data->entry[i].titleOffset);
			msg = (const wchar_t*)((u32)data + data->entry[i].msgOffset);
			break;
		}
	}

	if (title == 0) {
		OSReport("Message Box: Message %08x not found\n", id);
		return;
	}

	layout.findTextBoxByName("T_title")->SetString(title);
	layout.findTextBoxByName("T_msg")->SetString(msg);

	this->canCancel = canCancel;
	this->delay = delay;
	layout.findPictureByName("button")->SetVisible(canCancel);

	state.setState(&StateID_BoxAppearWait);
}


CREATE_STATE(dMsgBoxManager_c, BoxAppearWait);

void dMsgBoxManager_c::beginState_BoxAppearWait() {
	visible = true;
	MessageBoxIsShowing = true;
	StageC4::instance->_1D = 1; // enable no-pause
	layout.enableNonLoopAnim(ANIM_BOX_APPEAR);

	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_SYS_KO_DIALOGUE_IN, 1);
}

void dMsgBoxManager_c::executeState_BoxAppearWait() {
	if (!layout.isAnimOn(ANIM_BOX_APPEAR)) {
		state.setState(&StateID_ShownWait);
	}
}

void dMsgBoxManager_c::endState_BoxAppearWait() { }

/*****************************************************************************/
// Wait For Player To Finish
CREATE_STATE(dMsgBoxManager_c, ShownWait);

void dMsgBoxManager_c::beginState_ShownWait() { }
void dMsgBoxManager_c::executeState_ShownWait() {
	if (canCancel) {
		int nowPressed = Remocon_GetPressed(GetActiveRemocon());

		if (nowPressed & WPAD_TWO)
			state.setState(&StateID_BoxDisappearWait);
	}

	if (delay > 0) {
		delay--;
		if (delay == 0)
			state.setState(&StateID_BoxDisappearWait);
	}
}
void dMsgBoxManager_c::endState_ShownWait() { }

/*****************************************************************************/
// Hide Box
CREATE_STATE(dMsgBoxManager_c, BoxDisappearWait);

void dMsgBoxManager_c::beginState_BoxDisappearWait() {
	layout.enableNonLoopAnim(ANIM_BOX_DISAPPEAR);

	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_SYS_DIALOGUE_OUT_AUTO, 1);
}

void dMsgBoxManager_c::executeState_BoxDisappearWait() {
	if (!layout.isAnimOn(ANIM_BOX_DISAPPEAR)) {
		state.setState(&StateID_Wait);

		for (int i = 0; i < 2; i++)
			layout.resetAnim(i);
		layout.disableAllAnimations();
	}
}

void dMsgBoxManager_c::endState_BoxDisappearWait() {
	visible = false;
	MessageBoxIsShowing = false;
	if (canCancel && StageC4::instance)
		StageC4::instance->_1D = 0; // disable no-pause
}



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
// Replaces: EN_BLUR (Sprite 152; Profile ID 603 @ 80ADD890)


class daEnMsgBlock_c : public daEnBlockMain_c {
public:
	TileRenderer tile;
	Physics::Info physicsInfo;

	int onCreate();
	int onDelete();
	int onExecute();

	void calledWhenUpMoveExecutes();
	void calledWhenDownMoveExecutes();

	void blockWasHit(bool isDown);

	USING_STATES(daEnMsgBlock_c);
	DECLARE_STATE(Wait);

	static daEnMsgBlock_c *build();
};


CREATE_STATE(daEnMsgBlock_c, Wait);


int daEnMsgBlock_c::onCreate() {
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
	tile.tileNumber = 0x98;

	doStateChange(&daEnMsgBlock_c::StateID_Wait);

	return true;
}


int daEnMsgBlock_c::onDelete() {
	TileRenderer::List *list = dBgGm_c::instance->getTileRendererList(0);
	list->remove(&tile);

	physics.removeFromList();

	return true;
}


int daEnMsgBlock_c::onExecute() {
	acState.execute();
	physics.update();
	blockUpdate();

	tile.setPosition(pos.x-8, -(16+pos.y), pos.z);
	tile.setVars(scale.x);

	// now check zone bounds based on state
	if (acState.getCurrentState()->isEqual(&StateID_Wait)) {
		checkZoneBoundaries(0);
	}

	return true;
}


daEnMsgBlock_c *daEnMsgBlock_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daEnMsgBlock_c));
	return new(buffer) daEnMsgBlock_c;
}


void daEnMsgBlock_c::blockWasHit(bool isDown) {
	pos.y = initialY;

	if (dMsgBoxManager_c::instance)
		dMsgBoxManager_c::instance->showMessage(settings);
	else
		Delete(false);

	physics.setup(this, &physicsInfo, 3, currentLayerID);
	physics.addToList();
	
	doStateChange(&StateID_Wait);
}



void daEnMsgBlock_c::calledWhenUpMoveExecutes() {
	if (initialY >= pos.y)
		blockWasHit(false);
}

void daEnMsgBlock_c::calledWhenDownMoveExecutes() {
	if (initialY <= pos.y)
		blockWasHit(true);
}



void daEnMsgBlock_c::beginState_Wait() {
}

void daEnMsgBlock_c::endState_Wait() {
}

void daEnMsgBlock_c::executeState_Wait() {
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


