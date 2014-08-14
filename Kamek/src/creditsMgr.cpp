#include <game.h>
#include <sfx.h>
#include <dCourse.h>
#include <stage.h>
#include <playerAnim.h>
#include <newer.h>
void *EGG__Heap__alloc(unsigned long size, int unk, void *heap);
void EGG__Heap__free(void *ptr, void *heap);

extern char CameraLockEnabled;
extern VEC2 CameraLockPosition;

extern char isLockPlayerRotation;
extern s16 lockedPlayerRotation;

extern bool NoMichaelBuble;

mTexture_c efbTexture;
bool getNextEFB = false;
int thing = 0;

const char *CreditsFileList[] = {"CreditsBG", 0};

extern "C" void GXPixModeSync();
extern "C" void *MakeMarioEnterDemoMode();
extern "C" void *MakeMarioExitDemoMode();

struct FireworkInfo {
	const char *name;
	float xOffset, yOffset;
	int delay;
};

extern void *SoundRelatedClass;

extern u16 DanceValues_AnimSpeed; // 80427C2E
extern u8 DanceValues_DummyBlockAndGrass; // 8042A049
extern u8 DanceValues_Bahps; // 8042A04A
extern u8 DanceValues_CreditsControl; // 8042A04B

class dFlipbookRenderer_c : public m3d::proc_c {
	public:
		dFlipbookRenderer_c();
		~dFlipbookRenderer_c();

		mAllocator_c allocator;
		void drawOpa();
		void drawXlu();
		void execute();

		bool isEnabled;
		int flipFrame;

		int scheduledBG;

		char *tplBuffer[2];
		u32 tplBufferSize[2];
		GXTexObj bgTexObj[2];

		void loadNewBG(int bgID, bool isBackface);
};

struct danceInfo_s {
	u32 when;
	u8 animSpeed, dummyBlockFlag, bahpFlag, creditsFlag;
};

class dCreditsMgr_c : public dActorState_c {
	public:
		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		int currentPathID;

		bool isOutOfView() { return false; }
		Vec2 _vf70();

		dDvdLoader_c scriptLoader;
		const u8 *scriptPos;

		dFlipbookRenderer_c renderer;

		bool loadLayout();
		bool loadTitleLayout();
		bool layoutLoaded;
		bool titleLayoutLoaded;
		m2d::EmbedLayout_c layout;
		m2d::EmbedLayout_c titleLayout;

		bool titleLayoutVisible;

		int countdown;

		bool fireworks;
		int fireworksCountdown;
		int fwID;

		int fauxScrollFrame;
		float fauxScrollMinX, fauxScrollMaxX, fauxScrollY;

		VEC2 endingLockPositions[4];

		danceInfo_s *danceCommand;
		int danceTimer;

		nw4r::lyt::TextBox
			*Title, *TitleS,
			*Name, *NameS,
			*LeftName, *LeftNameS,
			*RightName, *RightNameS;
		nw4r::lyt::Pane
			*TitleContainer, *NamesContainer,
			*OneNameC, *TwoNamesC, *N_proportionC_00;

		void doAutoscroll(int pathID);
		void positionPlayers();

		void animComplete();

		void enableZoom();
		void disableZoom();
		void playerWinAnim();
		void playerLookUp();
		void theEnd();
		void exitStage();

		bool endingMode;

//		USING_STATES(dCreditsMgr_c);
//		DECLARE_STATE(Wait);
//		DECLARE_STATE(PlayLayoutAnim);
//		DECLARE_STATE(Flipping);

		static dCreditsMgr_c *build();
		static dCreditsMgr_c *instance;
};
// CREATE_STATE(dCreditsMgr_c, Wait);
// CREATE_STATE(dCreditsMgr_c, PlayLayoutAnim);
// CREATE_STATE(dCreditsMgr_c, Flipping);

dCreditsMgr_c *dCreditsMgr_c::instance = 0;

dCreditsMgr_c *dCreditsMgr_c::build() {
	void *buf = AllocFromGameHeap1(sizeof(dCreditsMgr_c));
	return new(buf) dCreditsMgr_c;
}


int dCreditsMgr_c::onCreate() {
	NoMichaelBuble = true;

	instance = this;

	if (!loadLayout())
		return false;
	if (!loadTitleLayout())
		return false;

	if (!scriptLoader.load("/NewerRes/NewerStaffRoll.bin"))
		return false;

	scriptPos = (const u8*)scriptLoader.buffer;

	getNextEFB = true; // make sure we have a texture

	renderer.allocator.setup(GameHeaps[0], 0x20);
	bool result = renderer.setup(&renderer.allocator);

	renderer.loadNewBG(0, false);

	//acState.setState(&StateID_Wait);

	return true;
}

int dCreditsMgr_c::onDelete() {
	instance = 0;

	isLockPlayerRotation = false;

	scriptLoader.unload();
	return layout.free() && titleLayout.free();
}
extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);
int dCreditsMgr_c::onExecute() {
	danceTimer++;
	if (danceCommand == 0)
		danceCommand = (danceInfo_s*)getResource("CreditsBG", "/Dance.bin");

	char *autoscrInfo = ((char*)dBgGm_c::instance) + 0x900AC;

	fauxScrollFrame++;
	if (fauxScrollFrame > 60)
		fauxScrollFrame = 60;
	float fsMult = (1.0f / 60.0f) * float(fauxScrollFrame);
	float interp = float(fauxScrollMinX) + ((fauxScrollMaxX - fauxScrollMinX) * fsMult);
	CameraLockPosition.x = interp;
	CameraLockPosition.y = fauxScrollY;

	if (endingMode) {
		for (int i = 0; i < 4; i++) {
			dAcPy_c *player;
			if ((player = dAcPy_c::findByID(i))) {
				if (!player->testFlag(0x24)) {
					player->setAnimePlayWithAnimID(0);
				}
				player->setFlag(0x24);
				player->rot.y = 0;
				player->speed.x = player->speed.y = player->speed.z = 0.0f;
				player->pos.x = endingLockPositions[i].x;
				player->pos.y = endingLockPositions[i].y;
			}
		}
	}

	if (fireworks) {
		fireworksCountdown--;
		if (fireworksCountdown <= 0) {
			static const FireworkInfo fwInfo[] = {
				{"Wm_ob_fireworks_g", 20.000000f, 49.000000f, 8}, // ends @ 8
				{"Wm_ob_fireworks_1up", 154.000000f, 80.000000f, 14}, // ends @ 22
				{"Wm_ob_fireworks_b", 168.000000f, 27.000000f, 33}, // ends @ 55
				{"Wm_ob_fireworks_1up", 416.000000f, 22.000000f, 33}, // ends @ 88
				{"Wm_ob_fireworks_y", 179.000000f, 11.000000f, 8}, // ends @ 96
				{"Wm_ob_fireworks_star", 9.000000f, 35.000000f, 25}, // ends @ 121
				{"Wm_ob_fireworks_y", 398.000000f, 29.000000f, 11}, // ends @ 132
				{"Wm_ob_fireworks_g", 127.000000f, 64.000000f, 21}, // ends @ 153
				{"Wm_ob_fireworks_star", 439.000000f, 66.000000f, 3}, // ends @ 156
				{"Wm_ob_fireworks_k", 320.000000f, 18.000000f, 31}, // ends @ 187
				{"Wm_ob_fireworks_p", 158.000000f, 47.000000f, 42}, // ends @ 229
				{"Wm_ob_fireworks_star", 127.000000f, 1.000000f, 29}, // ends @ 258
				{"Wm_ob_fireworks_k", 164.000000f, 50.000000f, 18}, // ends @ 276
				{"Wm_ob_fireworks_g", 365.000000f, 25.000000f, 39}, // ends @ 315
				{"Wm_ob_fireworks_k", 2.000000f, 78.000000f, 44}, // ends @ 359
				{"Wm_ob_fireworks_g", 309.000000f, 25.000000f, 42}, // ends @ 401
				{"Wm_ob_fireworks_star", 222.000000f, 78.000000f, 44}, // ends @ 445
				//{"Wm_ob_fireworks_y", 269.000000f, 23.000000f, 38}, // ends @ 483
				{0, 0.0f, 0.0f, 0},
			};

			fireworksCountdown = fwInfo[fwID].delay;
			float xOffs = fwInfo[fwID].xOffset;
			float yOffs = fwInfo[fwID].yOffset;
			VEC3 efPos = {10208.0f + xOffs, -304.0f - yOffs, pos.z + 200.0f};

			SpawnEffect(fwInfo[fwID].name, 0, &efPos, 0, 0);

			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_OBJ_GOAL_FIREWORKS, 1);

			fwID++;
			if (!fwInfo[fwID].name)
				fireworks = false;
		}
	}

	if (countdown > 0) {
		countdown--;
	} else if (!renderer.isEnabled) {
		if (!endingMode)
			isLockPlayerRotation = false;

		// Execute commands if we can
		bool exitInterpreter = false;
		while (scriptPos && !exitInterpreter) {
			int u16_top, u16_bottom;
			int whatAnim;
			u8 *staffCreditScore;
			nw4r::lyt::Pane *paneThing;

			const u8 *read = scriptPos;
			int cmdSize = *(read++);
			int cmdType = *(read++);

			scriptPos += cmdSize;

			switch (cmdType) {
				case 0: // Stop running script
					scriptPos = 0;
					exitInterpreter = true;
					break;

				case 1: // Delay
					u16_top = *(read++);
					u16_bottom = *(read++);
					countdown = (u16_top << 8) | u16_bottom;
					exitInterpreter = true;
					break;

				case 2: // Switch scene
					doAutoscroll(*(read++));
					break;

				case 3: // Switch scene and wait
					doAutoscroll(*(read++));
					exitInterpreter = true;
					break;

				case 4: // Show scores
					staffCreditScore = GetStaffCreditScore();
					staffCreditScore[0x279] = 1;
					// Hide the high score bit
					paneThing = *((nw4r::lyt::Pane**)(staffCreditScore + 0x274));
					paneThing->SetVisible(false);
					OSReport("Staff Credit Score object is at %p, going to show it\n", staffCreditScore);
					break;

				case 5: // Show text
					layout.enableNonLoopAnim(0);
					break;
				case 6: // Hide text
					layout.enableNonLoopAnim(1);
					break;

				case 7: { // Set names. FUN!
					int titleLength = *(read++);
					int nameCount = *(read++);

					WriteAsciiToTextBox(Title, (const char*)read);
					WriteAsciiToTextBox(TitleS, (const char*)read);

					read += titleLength;

					WriteAsciiToTextBox(Name, (const char*)read);
					WriteAsciiToTextBox(NameS, (const char*)read);

					float calcHeight = 29.0f * nameCount;
					TitleContainer->trans.y = (calcHeight * 0.5f) + 3.0f;

					OneNameC->SetVisible(true);
					TwoNamesC->SetVisible(false);

					} break;

				case 8:
					titleLayoutVisible = true;
					break;
				case 9:
					titleLayoutVisible = false;
					break;
				case 10:
					whatAnim = *(read++);
					titleLayout.enableNonLoopAnim(whatAnim);
					break;

				case 11:
					endingMode = true;
					break;
				case 12:
					enableZoom();
					break;
				case 13:
					playerWinAnim();
					break;
				case 14:
					disableZoom();
					break;
				case 15:
					playerLookUp();
					break;
				case 16:
					theEnd();
					break;
				case 17:
					exitStage();
					exitInterpreter = true;
					break;
				case 18:
					GetTheEnd()->willHide = true;
					break;
				case 19:
					fireworks = true;
					fireworksCountdown = 25;
					break;
				case 20:
					fireworks = false;
					break;
			}
		}
	}

	layout.execAnimations();
	layout.update();

	titleLayout.execAnimations();
	titleLayout.update();

	//acState.execute();
	renderer.execute();

	return true;
}

int dCreditsMgr_c::onDraw() {
	renderer.scheduleForDrawing();
	layout.scheduleForDrawing();
	if (titleLayoutVisible)
		titleLayout.scheduleForDrawing();
	return true;
}


bool dCreditsMgr_c::loadLayout() {
	if (!layoutLoaded) {
		if (!layout.loadArc("StaffRoll.arc", false))
			return false;

		layout.build("StaffRoll.brlyt");

		static const char *brlanNames[] = {
			"StaffRoll_show.brlan",
			"StaffRoll_hide.brlan",
		};
		static const char *groupNames[] = {
			"TheOnlyGroup", "TheOnlyGroup"
		};

		layout.loadAnimations(brlanNames, 2);
		layout.loadGroups(groupNames, (int[2]){0, 1}, 2);
		layout.disableAllAnimations();
		layout.resetAnim(0);

		static const char *tbNames[] = {
			"Title", "TitleS",
			"Name", "NameS",
			"LeftName", "LeftNameS",
			"RightName", "RightNameS"
		};
		static const char *paneNames[] = {
			"TitleContainer", "NamesContainer",
			"OneNameC", "TwoNamesC",
		};

		layout.getTextBoxes(tbNames, &Title, 8);
		layout.getPanes(paneNames, &TitleContainer, 4);

		layoutLoaded = true;
	}
	return layoutLoaded;
}

bool dCreditsMgr_c::loadTitleLayout() {
	if (!titleLayoutLoaded) {
		if (!titleLayout.loadArc("StaffRollTitle.arc", false))
			return false;

		titleLayout.build("StaffRollTitle.brlyt");

		static const char *brlanNames[] = {
			"StaffRollTitle_appear1.brlan",
			"StaffRollTitle_appear2.brlan",
		};
		static const char *groupNames[] = {
			"TheOnlyGroup", "TheOnlyGroup"
		};

		titleLayout.loadAnimations(brlanNames, 2);
		titleLayout.loadGroups(groupNames, (int[2]){0, 1}, 2);
		titleLayout.disableAllAnimations();
		titleLayout.resetAnim(1);

		float propScale = 1.3f;
		if (!IsWideScreen())
			propScale *= 0.85f;

		N_proportionC_00 = titleLayout.findPaneByName("N_proportionC_00");
		N_proportionC_00->trans.y = -130.0f;
		N_proportionC_00->scale.x = propScale;
		N_proportionC_00->scale.y = propScale;

		titleLayoutLoaded = true;
	}
	return titleLayoutLoaded;
}

extern "C" dCourse_c::rail_s *GetRail(int id);

void dCreditsMgr_c::doAutoscroll(int pathID) {
	OSReport("Activating Autoscroll with path %d\n", pathID);

	getNextEFB = true;
	renderer.isEnabled = true;
	renderer.flipFrame = 0;
	renderer.scheduledBG = pathID;
	MakeMarioEnterDemoMode();

	char *autoscrInfo = ((char*)dBgGm_c::instance) + 0x900AC;
	*((u8*)(autoscrInfo + 0x14)) = pathID;
	*((u8*)(autoscrInfo + 0x15)) = 1; // unk11
	*((u8*)(autoscrInfo + 0x16)) = 0; // atEnd Related
	*((u8*)(autoscrInfo + 0x17)) = 2; // atEnd
	*((u8*)(autoscrInfo + 0x18)) = 0; // mode
	*((u8*)(autoscrInfo + 0x1A)) = 1; // isAutoscrolling
	*((u32*)(((char*)dBgGm_c::instance) + 0x900EC)) = 0; // node ID
	*((u32*)(((char*)dBgGm_c::instance) + 0x900F0)) = 0; // ?

	currentPathID = pathID;

	dCourse_c::rail_s *rail = GetRail(pathID);
	dCourse_c *course = dCourseFull_c::instance->get(GetAreaNum());

	dCourse_c::railNode_s *firstNode = &course->railNode[rail->startNode];
	dCourse_c::railNode_s *secondNode = &course->railNode[rail->startNode+1];

	fauxScrollFrame = 0;
	fauxScrollMinX = firstNode->xPos;
	fauxScrollMaxX = secondNode->xPos;
	fauxScrollY = -firstNode->yPos;

	CameraLockEnabled = 1;

	// set directions
	isLockPlayerRotation = true;
	lockedPlayerRotation = endingMode ? 0 : 0x3000;

	for (int i = 0; i < 4; i++) {
		dAcPy_c *player;
		if ((player = dAcPy_c::findByID(i))) {
			player->direction = 0;
			player->rot.y = 0x3000;
		}
	}
}

void dCreditsMgr_c::animComplete() {
	positionPlayers();
}


void dCreditsMgr_c::positionPlayers() {
	dCourse_c *course = dCourseFull_c::instance->get(GetAreaNum());
	dCourse_c::nextGoto_s *entrance = course->getNextGotoByID(currentPathID);

	float diff = endingMode ? 48.0f : 24.0f;
	float playerPositions[4];
	playerPositions[0] = entrance->xPos;
	playerPositions[1] = playerPositions[0] - diff;
	playerPositions[2] = playerPositions[0] + diff;
	playerPositions[3] = playerPositions[0] + diff + diff;

	// This is annoying
	dAcPy_c *players[4];
	for (int i = 0; i < 4; i++)
		players[i] = dAcPy_c::findByID(i);

	static const int crap[4] = {0,1,3,2};

	int whichPos = 0;
	for (int i = 0; i < 4; i++) {
		dAcPy_c *player = 0;
		// Find the player matching this ID
		for (int j = 0; j < 4; j++) {
			if (Player_ID[players[j]->settings & 0xF] == crap[i]) {
				player = players[j];
				break;
			}
		}

		if (player) {
			player->pos.x = playerPositions[whichPos];
			player->pos.y = -(entrance->yPos + 16);
			player->direction = 0;
			player->rot.y = 0x3000;
			player->speed.x = player->speed.y = player->speed.z = 0.0f;
			dPlayerModelHandler_c *pmh = (dPlayerModelHandler_c*)(((u32)player) + 0x2A60);
			pmh->mdlClass->startAnimation(0, 1.0f, 0.0f, 0.0f);
			whichPos++;

			u32 *pInactivityCounter = (u32*)(((u32)player) + 0x480);
			*pInactivityCounter = 177;

			endingLockPositions[i].x = player->pos.x;
			endingLockPositions[i].y = player->pos.y;
		}
	}
}


void dCreditsMgr_c::enableZoom() {
	BgGmBase::manualZoomEntry_s &zoom = dBgGm_c::instance->manualZooms[0];
	zoom.x1 = 10218.0f;
	zoom.x2 = 11000.0f;
	zoom.y1 = -200.0f;
	zoom.y2 = -600.0f;

	zoom.x1 = 1100.0f;
	zoom.x2 = 1300.0f;
	zoom.y1 = -400.0f;
	zoom.y2 = -550.0f;
	zoom.zoomLevel = 7;
	zoom.unkValue6 = 0;
	zoom.firstFlag = 0;
}
void dCreditsMgr_c::disableZoom() {
	BgGmBase::manualZoomEntry_s &zoom = dBgGm_c::instance->manualZooms[0];
	zoom.unkValue6 = 100;
}
void dCreditsMgr_c::playerWinAnim() {
	// who won?
	// First, get the amounts
	u8 *amountsU8 = GetStaffCreditScore() + 0x288;
	int *playerAmounts = (int*)(amountsU8);
	int maxSoFar = 0;

	for (int i = 0; i < 4; i++) {
		if (playerAmounts[i] > maxSoFar)
			maxSoFar = playerAmounts[i];
	}

	if (maxSoFar == 0)
		return;

	// did anyone win?
	for (int i = 0; i < 4; i++) {
		if (playerAmounts[i] == maxSoFar) {
			dAcPy_c *player = dAcPy_c::findByID(i);
			if (!player)
				continue;

			player->setAnimePlayWithAnimID(goal_puton_capA);
			player->setFlag(0x24);

			static const int vocs[4] = {
				SE_VOC_MA_CLEAR_NORMAL,
				SE_VOC_LU_CLEAR_NORMAL,
				SE_VOC_KO_CLEAR_NORMAL,
				SE_VOC_KO2_CLEAR_NORMAL
			};
			dPlayerModelHandler_c *pmh = (dPlayerModelHandler_c*)(((u32)player) + 0x2A60);
			int voc = vocs[pmh->mdlClass->player_id_2];
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, voc, 1);

			int powerup = *((u32*)( 0x1090 + ((u8*)player) ));
			handle.SetPitch(powerup == 3 ? 1.5f : 1.0f);
		}
	}
}

void dCreditsMgr_c::playerLookUp() {
	_120 |= 8;
	lookAtMode = 2; // Higher maximum distance
}
void dCreditsMgr_c::theEnd() {
	GetTheEnd()->willShow = true;
}
void dCreditsMgr_c::exitStage() {
	SaveBlock *save = GetSaveFile()->GetBlock(-1);
	bool wasPreviouslyBeat = (save->bitfield & 2) != 0;
	save->bitfield |= 2;

	ExitStage(WORLD_MAP, wasPreviouslyBeat ? 0 : 0x20000000, BEAT_LEVEL, CIRCLE_WIPE);
}

Vec2 dCreditsMgr_c::_vf70() {
	return (const Vec2){10454.0f,-320.0f};
}



void EFBMagic2() {
	if (getNextEFB) {
		getNextEFB = false;

		GXRModeObj *ro = nw4r::g3d::G3DState::GetRenderModeObj();
		efbTexture.format = GX_TF_RGB565;
		efbTexture.width = ro->fbWidth;
		efbTexture.height = ro->efbHeight;
		efbTexture.wrapS = GX_CLAMP;
		efbTexture.wrapT = GX_CLAMP;

		if (efbTexture.getBuffer() == 0)
			efbTexture.allocateBuffer(GameHeaps[2]);

		GXSetTexCopySrc(0, 0, efbTexture.width, efbTexture.height);
		GXSetTexCopyDst(efbTexture.width, efbTexture.height, efbTexture.format, GX_FALSE);
		GXSetCopyFilter(GX_FALSE, 0, GX_FALSE, 0);
		GXCopyTex(efbTexture.getBuffer(), GX_FALSE);

		GXPixModeSync();
		GXInvalidateTexAll();
	}
}

void dFlipbookRenderer_c::execute() {
	if (flipFrame == 7) {
		loadNewBG(scheduledBG, true);
	}

	if (isEnabled) {
		flipFrame += 7;

		ClassWithCameraInfo *cwci = ClassWithCameraInfo::instance;
		if (flipFrame > int(cwci->screenWidth)) {
			loadNewBG(scheduledBG, false);
			dCreditsMgr_c::instance->animComplete();

			if (!dCreditsMgr_c::instance->endingMode)
				MakeMarioExitDemoMode();
			isEnabled = false;
			OSReport("DONE!\n");
		}
	}
}


static void setupGXForDrawingCrap() {
	GXSetNumChans(0);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanAmbColor(GX_COLOR0A0, (GXColor){255,255,255,255});
	GXSetChanMatColor(GX_COLOR0A0, (GXColor){255,255,255,255});
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_IDENTITY, GX_FALSE, GX_DTTIDENTITY);

	GXSetNumTevStages(1);
	GXSetNumIndStages(0);
	for (int i = 0; i < 0x10; i++)
		GXSetTevDirect(i);

	GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);

	GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);

	GXSetZCompLoc(GX_FALSE);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
	//GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_SET);
	GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

	GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, (GXColor){0,0,0,0});
	GXSetFogRangeAdj(GX_FALSE, 0, 0);

	GXSetAlphaUpdate(GX_TRUE);

	GXSetCullMode(GX_CULL_NONE);

	GXSetDither(GX_TRUE);

	GXSetTevColor(GX_TEVREG0, (GXColor){255,255,255,255});
	GXSetTevColor(GX_TEVREG1, (GXColor){255,255,255,255});
	GXSetTevColor(GX_TEVREG2, (GXColor){255,255,255,255});

	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

	nw4r::g3d::Camera cam(GetCameraByID(GetCurrentCameraID()));
	Mtx matrix;
	cam.GetCameraMtx(&matrix);
	GXLoadPosMtxImm(matrix, 0);
	GXSetCurrentMtx(0);

	GXClearVtxDesc();

	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);

	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
}

void dFlipbookRenderer_c::drawXlu() {
	if (!isEnabled || flipFrame < 2)
		return;

	setupGXForDrawingCrap();

	ClassWithCameraInfo *cwci = ClassWithCameraInfo::instance;

	float screenTop = cwci->screenTop + Stage80::instance->screenYOffset;
	float left = cwci->screenLeft + Stage80::instance->screenXOffset;
	float right = left + cwci->screenWidth;

	float halfWidth = (cwci->screenWidth * 0.5f);
	float midpoint = left + halfWidth;

	float value = (flipFrame / cwci->screenWidth);
	float sinThing = 50.0f * sin(3.14159f * value);

	bool drawBackside = (flipFrame > halfWidth);

	float xformedFlipEdge = right - flipFrame;

	// EFB SPECIFIC VERTICAL VARIABLES
	float efbEffectiveHeight = cwci->screenHeight;
	if (!IsWideScreen())
		efbEffectiveHeight *= 1.295f;

	float efbHalfHeight = (efbEffectiveHeight * 0.5f);

	float efbYCentre = screenTop - (cwci->screenHeight * 0.5f);
	if (!IsWideScreen())
		efbYCentre += 1.0f;
	float efbTop = efbYCentre + efbHalfHeight;
	float efbBottom = efbYCentre - efbHalfHeight;

	float efbExCoord = (efbEffectiveHeight + sinThing + sinThing) * 0.5f;
	// TPL SPECIFIC VERTICAL VARIABLES
	float tplHalfHeight = cwci->screenHeight * 0.5f;
	float tplTop = screenTop;
	float tplBottom = screenTop - cwci->screenHeight;

	float tplExCoord = (cwci->screenHeight + sinThing + sinThing) * 0.5f;


	// EFB Left
	efbTexture.load(GX_TEXMAP0);

	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	{
		// Bottom Right
		GXPosition3f32(midpoint, efbBottom, 9990.0f);
		GXNormal3f32(0.5f, 1.0f, 1.0f);
		// Top Right
		GXPosition3f32(midpoint, efbTop, 9990.0f);
		GXNormal3f32(0.5f, 0.0f, 1.0f);
		// Top Left
		GXPosition3f32(left, efbTop, 9990.0f);
		GXNormal3f32(0.0f, 0.0f, 1.0f);
		// Bottom Left
		GXPosition3f32(left, efbBottom, 9990.0f);
		GXNormal3f32(0.0f, 1.0f, 1.0f);
	}
	GXEnd();

	// TPL Right
	GXLoadTexObj(&bgTexObj[1], GX_TEXMAP0);

	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	{
		// Bottom Right
		GXPosition3f32(right, tplBottom, 9990.0f);
		GXNormal3f32(1.0f, 1.0f, 1.0f);
		// Top Right
		GXPosition3f32(right, tplTop, 9990.0f);
		GXNormal3f32(1.0f, 0.0f, 1.0f);
		// Top Left
		GXPosition3f32(midpoint, tplTop, 9990.0f);
		GXNormal3f32(0.5f, 0.0f, 1.0f);
		// Bottom Left
		GXPosition3f32(midpoint, tplBottom, 9990.0f);
		GXNormal3f32(0.5f, 1.0f, 1.0f);

	}
	GXEnd();


	if (!drawBackside) {
		// Flipping right side: EFB

		efbTexture.load(GX_TEXMAP0);

		GXBegin(GX_QUADS, GX_VTXFMT0, 4);

		// EFB Right (Flipped)
		// Bottom Left
		GXPosition3f32(midpoint, efbBottom, 9990.0f);
		GXNormal3f32(efbHalfHeight * 0.5f, efbHalfHeight, efbHalfHeight);
		// Top Left
		GXPosition3f32(midpoint, efbTop, 9990.0f);
		GXNormal3f32(efbHalfHeight * 0.5f, 0.0f, efbHalfHeight);
		// Top Right
		GXPosition3f32(xformedFlipEdge, efbTop + sinThing, 9990.0f);
		GXNormal3f32(efbExCoord, 0.0f, efbExCoord);
		// Bottom Right
		GXPosition3f32(xformedFlipEdge, efbBottom - sinThing, 9990.0f);
		GXNormal3f32(efbExCoord, efbExCoord, efbExCoord);

		GXEnd();

	} else {
		// Flipping left side

		GXLoadTexObj(&bgTexObj[1], GX_TEXMAP0);

		GXBegin(GX_QUADS, GX_VTXFMT0, 4);

		// TPL Left (Flipped))
		// Bottom Left
		GXPosition3f32(xformedFlipEdge, tplBottom - sinThing, 9990.0f);
		GXNormal3f32(0.0f, tplExCoord, tplExCoord);
		// Top Left
		GXPosition3f32(xformedFlipEdge, tplTop + sinThing, 9990.0f);
		GXNormal3f32(0.0f, 0.0f, tplExCoord);
		// Top Right
		GXPosition3f32(midpoint, tplTop, 9990.0f);
		GXNormal3f32(tplHalfHeight * 0.5f, 0.0f, tplHalfHeight);
		// Bottom Right
		GXPosition3f32(midpoint, tplBottom, 9990.0f);
		GXNormal3f32(tplHalfHeight * 0.5f, tplHalfHeight, tplHalfHeight);

		GXEnd();
	}
}


void dFlipbookRenderer_c::drawOpa() {
	setupGXForDrawingCrap();

	GXLoadTexObj(&bgTexObj[0], GX_TEXMAP0);

	ClassWithCameraInfo *cwci = ClassWithCameraInfo::instance;
	float left = cwci->screenLeft + Stage80::instance->screenXOffset;
	float right = left + cwci->screenWidth;
	float top = cwci->screenTop + Stage80::instance->screenYOffset;
	float bottom = top - cwci->screenHeight;

	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	GXPosition3f32(right, bottom, -4000.0f);
	GXNormal3f32(1.0f, 1.0f, 1.0f);
	GXPosition3f32(right, top, -4000.0f);
	GXNormal3f32(1.0f, 0.0f, 1.0f);
	GXPosition3f32(left, top, -4000.0f);
	GXNormal3f32(0.0f, 0.0f, 1.0f);
	GXPosition3f32(left, bottom, -4000.0f);
	GXNormal3f32(0.0f, 1.0f, 1.0f);
	GXEnd();
}

void dFlipbookRenderer_c::loadNewBG(int bgID, bool isBackface) {
	OSReport("Will load BG: %d\n", bgID);

	int setID = isBackface ? 1 : 0;

	char bgName[32];
	sprintf(bgName, isBackface ? "/Back%d.tpl.LZ" : "/BG%d.tpl.LZ", bgID);
	OSReport("Getting %s\n", bgName);

	u8 *sourceBuf = getResource("CreditsBG", bgName);
	u32 bufSize = CXGetUncompressedSize(sourceBuf);

	if (tplBuffer[setID] && (tplBufferSize[setID] != bufSize)) {
		OSReport("Current TPL buffer (%p) is size %d (0x%x), freeing\n", tplBuffer[setID], tplBufferSize[setID], tplBufferSize[setID]);
		EGG__Heap__free(tplBuffer[setID], GameHeaps[2]);
		tplBuffer[setID] = 0;
	}

	if (!tplBuffer[setID]) {
		OSReport("Allocating TPL buffer of size %d (0x%x)\n", bufSize, bufSize);
		tplBuffer[setID] = (char*)EGG__Heap__alloc(bufSize, 0x20, GameHeaps[2]);
		tplBufferSize[setID] = bufSize;
	}

	CXUncompressLZ(sourceBuf, tplBuffer[setID]);
	OSReport("Butts. Decompressing %p to %p.\n", sourceBuf, tplBuffer[setID]);

	TPLBind((TPLPalette*)tplBuffer[setID]);
	TPLImage *image = TPLGet((TPLPalette*)tplBuffer[setID], 0);
	TPLTexHeader *tex = image->texture;
	OSReport("Received TPLHeader %p; Data: %p; Size: %d x %d; Format; %d\n", tex, tex->data, tex->width, tex->height, tex->format);

	GXInitTexObj(&bgTexObj[setID], tex->data, tex->width, tex->height,
			tex->format, tex->wrapS, tex->wrapT, GX_FALSE);
}

dFlipbookRenderer_c::dFlipbookRenderer_c() {
	scheduledBG = -1;
}

dFlipbookRenderer_c::~dFlipbookRenderer_c() {
	for (int setID = 0; setID < 2; setID++) {
		if (tplBuffer[setID]) {
			EGG__Heap__free(tplBuffer[setID], GameHeaps[2]);
			tplBuffer[setID] = 0;
		}
	}
}



extern "C" void replayRecord();

void LoadDanceValues() {
	/*
	//OSReport("AnmSpd: %4d / DBAG: 0x%02x / Bahp: 0x%02x / Cred: 0x%02x\n",
	//	DanceValues_AnimSpeed, DanceValues_DummyBlockAndGrass, DanceValues_Bahps, DanceValues_CreditsControl);
	if (DanceValues_CreditsControl > 0)
		OSReport("[ORIG DANCE] Credits Control: 0x%02x\n", DanceValues_CreditsControl);
	// if (DanceValues_DummyBlockAndGrass > 0)
	// 	OSReport("[ORIG DANCE] DummyBlockAndGrass: 0x%02x\n", DanceValues_DummyBlockAndGrass);
	if (DanceValues_Bahps > 0)
		OSReport("[ORIG DANCE] Bahps: 0x%02x\n", DanceValues_Bahps);
	*/

	dCreditsMgr_c *cred = dCreditsMgr_c::instance;

	if (!cred)
		return;
	danceInfo_s *cmd = cred->danceCommand;
	if (!cmd)
		return;
	//OSReport("TIMER: %d\n", cred->danceTimer);

	if (cred->danceTimer == cmd->when) {
		//OSReport("Timer reached %d, triggering dance 0x%02x, next is at %d\n", cmd->when, cmd->bahpFlag, cmd[1].when);
		DanceValues_AnimSpeed = cmd->animSpeed;
		DanceValues_DummyBlockAndGrass = cmd->dummyBlockFlag;
		DanceValues_Bahps = cmd->bahpFlag;
		DanceValues_CreditsControl = cmd->creditsFlag;

		cred->danceCommand++;
	} else {
		DanceValues_AnimSpeed = 120;
		DanceValues_DummyBlockAndGrass = 0;
		DanceValues_Bahps = 0;
		DanceValues_CreditsControl = 0;
	}

	replayRecord();
}


