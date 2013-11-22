#include "koopatlas/player.h"

daWMPlayer_c *daWMPlayer_c::instance;
static const char *mdlNames[] = {"MB_model", "SMB_model", "PLMB_model", "PMB_model"};
static const char *patNames[] = {"PB_switch_swim", "PB_switch_swim", "PLMB_switch_swim", "PLB_switch_swim"};

int daWMPlayer_c::onCreate() {

	this->modelHandler = new dPlayerModelHandler_c(0);
	// loadModel(u8 player_id, int powerup_id, int unk);
	// Unk is some kind of mode: 0=in-game, 1=map, 2=2D

	// Here we do a bit of a hack
	//this->modelHandler->loadModel(0, 3, 1);
	dPlayerModel_c *pm = (dPlayerModel_c*)modelHandler->mdlClass;

	pm->mode_maybe = 1;
	pm->player_id_1 = 0;
	pm->allocator.link(0xC000, GameHeaps[0], 0, 0x20);
	pm->prepare();

	for (int i = 0; i < 4; i++) {
		nw4r::g3d::ResMdl mdl = pm->modelResFile.GetResMdl(mdlNames[i]);
		nw4r::g3d::ResAnmTexPat pat = pm->modelResFile.GetResAnmTexPat(patNames[i]);

		pats[i].setup(mdl, pat, &pm->allocator, 0, 1);
	}

	pm->allocator.unlink();
	pm->setPowerup(3);
	pm->finaliseModel();

	pm->startAnimation(0, 1.2, 10.0, 0.0);
	modelHandler->setSRT((Vec){0.0,100.0,-100.0}, (S16Vec){0,0,0}, (Vec){2.0,2.0,2.0});

	hammerSuit.setup(this->modelHandler);

	pos = (Vec){0.0f,0.0f,3000.0f};
	rot = (S16Vec){0x1800,0,0};
	scale = (Vec){1.6f,1.6f,1.6f};

	hasEffect = false;
	hasSound = false;
	step = false;
	effectName = "";
	soundName = 0;
	timer = 0;
	jumpOffset = 0.0;

	// -1 or 0xC000? normally we use -1 but Player uses 0xC000....
	//allocator.link(0xC000, GameHeaps[0], 0, 0x20);
	//allocator.unlink();

	return true;
}

int daWMPlayer_c::onDelete() {
	delete modelHandler;

	return true;
}


int daWMPlayer_c::onExecute() {
	if (Player_Flags[0] & 1) {
		modelHandler->mdlClass->enableStarColours();
		modelHandler->mdlClass->enableStarEffects();
		dKPMusic::playStarMusic();
	}

	if (spinning)
		rot.y += 0xC00;
	else
		SmoothRotation(&rot.y, targetRotY, 0xC00);

	if (dScKoopatlas_c::instance->mapIsRunning())
		dScKoopatlas_c::instance->pathManager.execute();

	this->modelHandler->update();
	pats[((dPlayerModel_c*)modelHandler->mdlClass)->currentPlayerModelID].process();

	mMtx myMatrix;
	myMatrix.scale(scale.x, scale.y, scale.z);
	myMatrix.applyTranslation(pos.x, pos.y + jumpOffset, pos.z);
	if (dScKoopatlas_c::instance->warpZoneHacks && (currentAnim == jump || currentAnim == jumped))
		myMatrix.applyTranslation(0, 0, 600.0f);
	myMatrix.applyRotationX(&rot.x);
	myMatrix.applyRotationY(&rot.y);
	// Z is unused for now
	modelHandler->setMatrix(myMatrix);

	if (dScKoopatlas_c::instance->mapIsRunning()) {
		if (hasEffect) { 
			Vec effPos = {pos.x, pos.y, 3300.0f};
			effect.spawn(effectName, 0, &effPos, &rot, &scale);
		}

		if (hasSound) {
			timer++;

			if (timer == 12) {
				if (step) { MapSoundPlayer(SoundRelatedClass, soundName, 1); step = false; }
				else { MapSoundPlayer(SoundRelatedClass, soundName+1, 1); step = true; }
				timer = 0;
			}

			if (timer > 12) { timer = 0; }
		}
	}

	return true;
}

int daWMPlayer_c::onDraw() {
	if (!visible)
		return true;
	if (dScKoopatlas_c::instance->isEndingScene)
		return true;

	this->modelHandler->draw();
	hammerSuit.draw();

	return true;
}


void daWMPlayer_c::startAnimation(int id, float frame, float unk, float updateRate) {
	if (id == currentAnim && frame == currentFrame && unk == currentUnk && updateRate == currentUpdateRate)
		return;

	bool isOldSwimming = (currentAnim == swim_wait);
	bool isNewSwimming = (id == swim_wait);

	currentAnim = id;
	currentFrame = frame;
	currentUnk = unk;
	currentUpdateRate = updateRate;
	this->modelHandler->mdlClass->startAnimation(id, frame, unk, updateRate);

	if (isOldSwimming != isNewSwimming)
		bindPats();
}

void daWMPlayer_c::bindPats() {
	dPlayerModel_c *pm = (dPlayerModel_c*)modelHandler->mdlClass;
	int id = pm->currentPlayerModelID;

	static const float frames[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 3.0f};
	float frame = frames[pm->powerup_id];
	if (currentAnim == swim_wait)
		frame += (pm->powerup_id == 4 || pm->powerup_id == 5) ? 1.0f : 4.0f;

	nw4r::g3d::ResAnmTexPat pat = pm->modelResFile.GetResAnmTexPat(patNames[id]);
	pats[id].bindEntry(
			&pm->models[id].body,
			&pat,
			0, 4);
	pats[id].setUpdateRateForEntry(0.0f, 0);
	pats[id].setFrameForEntry(frame, 0);

	pm->models[id].body.bindAnim(&pats[id]);
}


daWMPlayer_c *daWMPlayer_c::build() {

	void *buffer = AllocFromGameHeap1(sizeof(daWMPlayer_c));
	daWMPlayer_c *c = new(buffer) daWMPlayer_c;


	instance = c;
	return c;
}

