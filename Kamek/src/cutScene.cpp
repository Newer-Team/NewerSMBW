#include "cutScene.h"

dScCutScene_c *dScCutScene_c::instance = 0;

dScCutScene_c *dScCutScene_c::build() {
	// return new dScCutScene_c;
	void *buffer = AllocFromGameHeap1(sizeof(dScCutScene_c));
	dScCutScene_c *c = new(buffer) dScCutScene_c;

	instance = c;
	return c;
}

dScCutScene_c::dScCutScene_c() {
	data = 0;
	layout = 0;
	sceneLoaders = 0;

	yesNoWindow = (dYesNoWindow_c*)CreateParentedObject(YES_NO_WINDOW, this, 0, 0);
	CreateParentedObject(SELECT_CURSOR, this, 0, 0);
}

dScCutScene_c::~dScCutScene_c() {
	if (layout)
		delete layout;

	if (sceneLoaders)
		delete[] sceneLoaders;
}


static const char *CutsceneNames[] = {
	"/CS/Opening.cs",
	"/CS/Kamek.cs",
	"/CS/Ending.cs"
};

static const char *WideCutsceneNames[] = {
	"/CS/WideOpening.cs",
	"/CS/Kamek.cs",
	"/CS/Ending.cs"
};


int dScCutScene_c::onCreate() {
	*CurrentDrawFunc = CutSceneDrawFunc;

	currentScene = -1;

	int csNumber = settings >> 28;
	const char *csName = IsWideScreen() ?
		WideCutsceneNames[csNumber] : CutsceneNames[csNumber];

	if (settingsLoader.load(csName)) {
		// only deal with this once!
		if (data) return 1;

		data = (dMovieData_s*)settingsLoader.buffer;

		// fix up the settings
		for (int i = 0; i < data->sceneCount; i++) {
			data->scenes[i] =
				(dMovieScene_s*)((u32)data + (u32)data->scenes[i]);

			data->scenes[i]->sceneName =
				(char*)((u32)data + (u32)data->scenes[i]->sceneName);
		}

		sceneLoaders = new dDvdLoader_c[data->sceneCount];

		nextScene = 0;

		return 1;
	}

	return 0;
}

int dScCutScene_c::onDelete() {
	if (layout)
		return layout->free();

	for (int i = 0; i < data->sceneCount; i++) {
		sceneLoaders[i].unload();
	}
	settingsLoader.unload();

	return true;
}

int dScCutScene_c::onExecute() {
	int nowPressed = Remocon_GetPressed(GetActiveRemocon());

	switch (yesNoStage) {
		case 1:
			// Opening
			if (!yesNoWindow->animationActive)
				yesNoStage = 2;
			return true;
		case 2:
			// Opened
			if (nowPressed & WPAD_LEFT)
				yesNoWindow->current = 1;
			else if (nowPressed & WPAD_RIGHT)
				yesNoWindow->current = 0;
			else if (Wiimote_TestButtons(GetActiveWiimote(), WPAD_A | WPAD_TWO)) {
				yesNoWindow->close = true;

				if (yesNoWindow->current != 1)
					yesNoWindow->hasBG = true;
				yesNoStage = 3;

			} else {
				// Cancel using B or 1
				if (CheckIfMenuShouldBeCancelledForSpecifiedWiimote(0)) {
					yesNoWindow->cancelled = true;
					yesNoWindow->current = 1;
					yesNoStage = 3;
				}
			}
			return true;
		case 3:
			// Closing
			if (!yesNoWindow->visible) {
				if (yesNoWindow->current == 1) {
					yesNoStage = 0;
					for (int i = 0; i < 40; i++)
						if (handles[i].Exists()) {
							handles[i].Pause(false, 3);
						}
				} else {
					yesNoStage = 4;
					goToNextScene();
				}
			}
			return true;
		case 4:
			// Waiting for exit transition
			return true;
	}

	if (yesNoWindow->layoutLoaded && (nowPressed & WPAD_PLUS)) {
		yesNoStage = 1;
		yesNoWindow->type = 3;
		yesNoWindow->visible = true;

		for (int i = 0; i < 40; i++)
			if (handles[i].Exists()) {
				handles[i].Pause(true, 3);
			}
		return true;
	}

	// deal with loading first

	// what do we want to load?
	if (currentScene >= 0 || nextScene >= 0) {
		int whatToLoad = (nextScene >= 0) ? nextScene : (currentScene + 1);

		sceneLoaders[whatToLoad].load(data->scenes[whatToLoad]->sceneName);
	}


	// now, do all other processing

	if (currentScene >= 0) {
		if (!layout->isAnyAnimOn()) {
			// we're at the end
			// what now?

			if ((currentScene + 1) == data->sceneCount) {
				goToNextScene();
			} else {
				nextScene = currentScene + 1;
				OSReport("switching to scene %d\n", nextScene);
			}

			sceneLoaders[currentScene].unload();
			currentScene = -1;
			layout->loader.buffer = 0;
			layout->free();
			delete layout;
			layout = 0;
			return true;
		}

		frameOffset++;

		// check if we gotta do anything
		dMovieScene_s *scene = data->scenes[currentScene];
		int cmdsParsed = 0;
		int cmdOffset = 0;
		while (cmdsParsed < scene->commandCount) {
			u32 cmdType = scene->command[cmdOffset++];
			cmdsParsed++;

			switch (cmdType) {
				case 1:
					{
						u32 delay = scene->command[cmdOffset++];
						u32 soundID = scene->command[cmdOffset++];
						if (delay != frameOffset)
							continue;

						nw4r::snd::SoundHandle *handle = 0;
						for (int i = 20; i < 40; i++) {
							if (!handles[i].Exists()) {
								handle = &handles[i];
								break;
							}
						}
						if (handle)
							PlaySoundWithFunctionB4(SoundRelatedClass, handle, soundID, 1);
					} break;
				case 2:
					{
						u32 handleID = scene->command[cmdOffset++];
						u32 delay = scene->command[cmdOffset++];
						u32 soundID = scene->command[cmdOffset++];
						if (delay != frameOffset)
							continue;

						PlaySoundWithFunctionB4(SoundRelatedClass, &handles[handleID], soundID, 1);
					} break;
				case 3:
					{
						u32 handleID = scene->command[cmdOffset++];
						u32 delay = scene->command[cmdOffset++];
						u32 frameCount = scene->command[cmdOffset++];
						if (delay != frameOffset)
							continue;

						if (handles[handleID].Exists()) {
							handles[handleID].Stop(frameCount);
							handles[handleID].DetachSound();
						}
					} break;
			}
		}

		if (nowPressed & WPAD_ONE) {
			OSReport("Currently: Scene %d; Frame %d\n", currentScene, frameOffset);
		}

		layout->execAnimations();
		layout->update();
	}
	
	if (nextScene >= 0) {
		// is this scene loaded yet?
		if (sceneLoaders[nextScene].buffer) {
			currentScene = nextScene;

			OSReport("Loading scene %d\n", currentScene);

			layout = new m2d::EmbedLayout_c;
			layout->loader.buffer = sceneLoaders[nextScene].buffer;
			layout->loader.attachArc(layout->loader.buffer, "arc");
			layout->resAccPtr = &layout->loader;

			bool result = layout->build("cutscene.brlyt");
			OSReport("Result: %d\n", result);
			layout->loadAnimations((const char *[1]){"cutscene.brlan"}, 1);
			layout->loadGroups((const char *[1]){"cutscene"}, (int[1]){0}, 1);
			layout->disableAllAnimations();
			layout->enableNonLoopAnim(0);

			u8 widescreenFlag = data->scenes[nextScene]->widescreenFlag;
			if (widescreenFlag == 1 || widescreenFlag == 3) {
				// Native on 16:9, letterboxed on 4:3
				if (IsWideScreen()) {
					if (widescreenFlag == 3) {
						layout->layout.rootPane->trans.x = -100.0f;
						layout->layout.rootPane->scale.x = 0.75f;
					}
				} else {
					layout->clippingEnabled = true;
					layout->clipX = 0;
					layout->clipY = 52;
					layout->clipWidth = 640;
					layout->clipHeight = 352;
					layout->layout.rootPane->scale.x = 0.7711f;
					layout->layout.rootPane->scale.y = 0.7711f;
				}
			} else if (widescreenFlag == 0) {
				// Native on 4:3, black bars on 16:9
				if (IsWideScreen()) {
					layout->clippingEnabled = true;
					layout->clipX = 66;
					layout->clipY = 0;
					layout->clipWidth = 508;
					layout->clipHeight = 456;
					layout->layout.rootPane->scale.x = 0.794f;
				}
			}

			layout->execAnimations();
			layout->update();

			OSReport("Loaded scene %d\n", currentScene);

			nextScene = -1;
			frameOffset = 0;
		}
	}

	return true;
}

int dScCutScene_c::onDraw() {
	if (currentScene >= 0)
		layout->scheduleForDrawing();

	return true;
}

void CutSceneDrawFunc() {
	Reset3DState();
	SetupLYTDrawing();
	DrawAllLayoutsBeforeX(0x81);
	RenderEffects(0, 3);
	RenderEffects(0, 2);
	GXDrawDone();
	RemoveAllFromScnRoot();
	Reset3DState();
	SetCurrentCameraID(1);
	DoSpecialDrawing1();
	SetCurrentCameraID(0);
	for (int i = 0; i < 4; i++)
		RenderEffects(0, 0xB+i);
	for (int i = 0; i < 4; i++)
		RenderEffects(0, 7+i);
	GXDrawDone();
	// Leaving out some stuff here
	DrawAllLayoutsAfterX(0x80);
	ClearLayoutDrawList();
	SetCurrentCameraID(0);
}


void dScCutScene_c::goToNextScene() {
	// we're TOTALLY done!
	OSReport("playback complete\n");
	int nsmbwMovieType = settings & 3;
	int newerMovieType = settings >> 28;

	for (int i = 0; i < 40; i++)
		if (handles[i].Exists())
			handles[i].Stop(5);

	switch (newerMovieType) {
		case 0:
			// OPENING
			switch (nsmbwMovieType) {
				case 0:
					SaveGame(0, false);
					DoSceneChange(WORLD_MAP, 0x80000000, 0);
					break;
				case 1:
					StartTitleScreenStage(false, 0);
					break;
			}
			break;

		case 1:
			// KAMEK (W7 => W8)
			ActivateWipe(WIPE_MARIO);
			DoSceneChange(WORLD_MAP, 0x40000000, 0);
			break;
	}
}

