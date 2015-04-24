#include "koopatlas/hud.h"
#include <newer.h>

dWMHud_c *dWMHud_c::instance = 0;

dWMHud_c *dWMHud_c::build() {

	void *buffer = AllocFromGameHeap1(sizeof(dWMHud_c));
	dWMHud_c *c = new(buffer) dWMHud_c;


	instance = c;
	return c;
}

dWMHud_c::dWMHud_c() {
	layoutLoaded = false;
	displayedControllerType = -1;
	isFooterVisible = false;
}

enum WMHudAnimation {
	SHOW_LIVES = 0,
	SHOW_HEADER,
	SHOW_FOOTER,
	HIDE_ALL,
	UNHIDE_ALL,
};


int dWMHud_c::onCreate() {
	if (!layoutLoaded) {
		bool gotFile = layout.loadArc("MapHUD.arc", false);
		if (!gotFile)
			return false;

		bool output = layout.build("maphud.brlyt");

		layout.layout.rootPane->trans.x = -112.0f;
		if (IsWideScreen()) {
			layout.layout.rootPane->scale.x = 0.735f;
		} else {
			layout.clippingEnabled = true;
			layout.clipX = 0;
			layout.clipY = 52;
			layout.clipWidth = 640;
			layout.clipHeight = 352;
			layout.layout.rootPane->scale.x = 0.731f;
			layout.layout.rootPane->scale.y = 0.7711f;
			layout.drawInfo._50 &= ~0x20;
		}

		static const char *brlanNames[] = {
			"MapHUD_ShowMain.brlan", "MapHUD_ShowHeader.brlan",
			"MapHUD_HideAll.brlan", "MapHUD_UnhideAll.brlan",
		};
		static const char *groupNames[] = {
			"G_Lives", "G_Header", "G_Footer",
			"G_Hideables", "G_Hideables",
		};

		layout.loadAnimations(brlanNames, 4);
		layout.loadGroups(groupNames, (int[5]){0, 1, 0, 2, 3}, 5);
		layout.disableAllAnimations();

		layout.enableNonLoopAnim(SHOW_LIVES);
		layout.resetAnim(SHOW_FOOTER);
		layout.resetAnim(SHOW_HEADER);
		layout.resetAnim(HIDE_ALL);

		static const char *tbNames[2] = {"MenuButtonInfo", "ItemsButtonInfo"};
		layout.setLangStrings(tbNames, (int[2]){12, 15}, 4, 2);

		static const char *paneNames[] = {
			"N_IconPos1P_00", "N_IconPos2P_00",
			"N_IconPos3P_00", "N_IconPos4P_00"
		};
		layout.getPanes(paneNames, &N_IconPosXP_00[0], 4);

		static const char *pictureNames[] = {
			"Header_Centre", "Header_Right", "Footer",
			"NormalExitFlag", "SecretExitFlag",
			"StarCoinOff0", "StarCoinOff1", "StarCoinOff2",
			"StarCoinOn0", "StarCoinOn1", "StarCoinOn2",
			"P_marioFace_00", "P_luigiFace_00",
			"P_BkinoFace_00", "P_YkinoFace_00",
			"Star0", "Star1", "Star2"
		};
		layout.getPictures(pictureNames, &Header_Centre, 18);

		static const char *textBoxNames[] = {
			"LevelName", "LevelNameS",
			"LevelNumber", "LevelNumberS",
			"WorldName", "WorldNameS",
			"StarCoinCounter",
			"T_lifeNumber_00", "T_lifeNumber_01",
			"T_lifeNumber_02", "T_lifeNumber_03"
		};
		layout.getTextBoxes(textBoxNames, &LevelName, 11);

		headerCol.setTexMap(Header_Right->material->texMaps);
		headerCol.applyAlso(Header_Centre->material->texMaps);
		footerCol.setTexMap(Footer->material->texMaps);

		layoutLoaded = true;

		layout.drawOrder = 0;

		willShowHeader = false;
		willShowFooter = false;

		loadFooterInfo();

		setupLives();
	}

	return true;
}


void dWMHud_c::loadInitially() {
	if (doneFirstShow)
		return;

	doneFirstShow = true;

	SaveBlock *save = GetSaveFile()->GetBlock(-1);
	willShowFooter = (save->newerWorldName[0] != 0) && (save->hudHintH != 2000);

	if (!dScKoopatlas_c::instance->pathManager.isMoving)
		enteredNode();
}


int dWMHud_c::onDelete() {
	dWMHud_c::instance = 0;

	if (!layoutLoaded)
		return true;

	return layout.free();
}


int dWMHud_c::onExecute() {
	if (!layoutLoaded)
		return true;

	if (willShowHeader && (!(layout.isAnimOn(SHOW_HEADER)))) {
		willShowHeader = false;
		loadHeaderInfo();
		playShowAnim(SHOW_HEADER);
	}

	if (willShowFooter && (!(layout.isAnimOn(SHOW_FOOTER)))) {
		willShowFooter = false;
		isFooterVisible = true;
		loadFooterInfo();
		playShowAnim(SHOW_FOOTER);
	}

	setupLives(); // FUCK IT
	updatePressableButtonThingies();

	int scCount = getUnspentStarCoinCount();
	WriteNumberToTextBox(&scCount, StarCoinCounter, false);

	layout.execAnimations();
	layout.update();

	return true;
}


int dWMHud_c::onDraw() {
	if (!layoutLoaded)
		return true;

	layout.scheduleForDrawing();

	return true;
}


void dWMHud_c::hideAll() {
	if (!layout.isAnimOn(HIDE_ALL))
		layout.enableNonLoopAnim(HIDE_ALL);
	layout.grpHandlers[HIDE_ALL].frameCtrl.flags = 1; // NO_LOOP
}
void dWMHud_c::unhideAll() {
	if (!layout.isAnimOn(HIDE_ALL))
		layout.enableNonLoopAnim(HIDE_ALL, true);
	layout.grpHandlers[HIDE_ALL].frameCtrl.flags = 3; // NO_LOOP | REVERSE
}




void dWMHud_c::playShowAnim(int id) {
	if (!this || !this->layoutLoaded) return;

	layout.enableNonLoopAnim(id);
}

void dWMHud_c::playHideAnim(int id) {
	if (!this || !this->layoutLoaded) return;

	if (!layout.isAnimOn(id)) {
		layout.enableNonLoopAnim(id, true);
	}
	layout.grpHandlers[id].frameCtrl.flags = 3; // NO_LOOP | REVERSE
	if (id == SHOW_FOOTER)
		isFooterVisible = false;
}


void dWMHud_c::loadHeaderInfo() {
	dLevelInfo_c *levelInfo = &dLevelInfo_c::s_info;

	dLevelInfo_c::entry_s *infEntry = levelInfo->searchBySlot(
			nodeForHeader->levelNumber[0]-1, nodeForHeader->levelNumber[1]-1);

	if (infEntry == 0) {
		LevelName->SetString(L"Unknown Level Name!");
		LevelNameS->SetString(L"Unknown Level Name!");
		return;
	}

	// LEVEL NAME
	wchar_t convertedLevelName[100];
	const char *sourceLevelName = levelInfo->getNameForLevel(infEntry);
	int charCount = 0;
	
	while (*sourceLevelName != 0 && charCount < 99) {
		convertedLevelName[charCount] = *sourceLevelName;
		sourceLevelName++;
		charCount++;
	}
	convertedLevelName[charCount] = 0;

	LevelName->SetString(convertedLevelName);
	LevelNameS->SetString(convertedLevelName);

	// a hack because I don't feel like editing the rlyt
	LevelName->size.x = LevelNameS->size.x = 400.0f;

	// LEVEL NUMBER
	wchar_t levelNumber[16];
	getNewerLevelNumberString(infEntry->displayWorld, infEntry->displayLevel, levelNumber);

	LevelNumber->SetString(levelNumber);

	// make the picture shadowy
	int sidx = 0;
	while (levelNumber[sidx]) {
		if (levelNumber[sidx] == 11) {
			levelNumber[sidx+1] = 0x200 | (levelNumber[sidx+1]&0xFF);
			sidx += 2;
		}
		sidx++;
	}
	LevelNumberS->SetString(levelNumber);

	nw4r::ut::TextWriter tw2;
	tw2.font = LevelNumber->font;
	tw2.SetFontSize(LevelNumber->fontSizeX, LevelNumber->fontSizeY);
	tw2.lineSpace = LevelNumber->lineSpace;
	tw2.charSpace = LevelNumber->charSpace;
	if (LevelNumber->tagProc != 0)
		tw2.tagProcessor = LevelNumber->tagProc;

	float currentPos = tw2.CalcStringWidth(levelNumber, wcslen(levelNumber));
	currentPos += LevelNumber->trans.x + 12.0f;

	// INFO
	int w = nodeForHeader->levelNumber[0] - 1;
	int l = nodeForHeader->levelNumber[1] - 1;

	u32 conds = GetSaveFile()->GetBlock(-1)->GetLevelCondition(w, l);
	// States: 0 = invisible, 1 = visible, 2 = faded
	int normalState = 0, secretState = 0;

	if ((conds & COND_BOTH_EXITS) && (infEntry->flags & 0x30) == 0x30) {
		// If this level has two exits and one of them is already collected,
		// then show the faded flags
		normalState = 2;
		secretState = 2;
	}

	if ((conds & COND_NORMAL) && (infEntry->flags & 0x10))
		normalState = 1;
	if ((conds & COND_SECRET) && (infEntry->flags & 0x20))
		secretState = 1;

	NormalExitFlag->trans.x = currentPos;
	NormalExitFlag->alpha = (normalState == 2) ? 80 : 255;
	NormalExitFlag->SetVisible(normalState > 0);
	if (normalState > 0)
		currentPos += NormalExitFlag->size.x;

	SecretExitFlag->trans.x = currentPos;
	SecretExitFlag->alpha = (secretState == 2) ? 80 : 255;
	SecretExitFlag->SetVisible(secretState > 0);
	if (secretState > 0)
		currentPos += SecretExitFlag->size.x;

	// are star coins enabled or not?
	bool haveSC = (infEntry->flags & 2);

	for (int i = 0; i < 3; i++) {
		bool flag = (conds & (COND_COIN1 << i));
		StarCoinOn[i]->SetVisible(flag);
		StarCoinOff[i]->SetVisible(haveSC);
		if (haveSC) {
			StarCoinOff[i]->trans.x = currentPos;
			currentPos += StarCoinOff[i]->size.x + 4.0f;
		}
	}

	// SIZE THING
	nw4r::ut::TextWriter tw;
	tw.font = LevelName->font;
	tw.SetFontSize(LevelName->fontSizeX, LevelName->fontSizeY);
	tw.lineSpace = LevelName->lineSpace;
	tw.charSpace = LevelName->charSpace;
	if (LevelName->tagProc != 0)
		tw.tagProcessor = LevelName->tagProc;

	float width = tw.CalcStringWidth(convertedLevelName, charCount);
	float totalWidth = width + LevelName->trans.x - 20.0f;
	if (totalWidth < currentPos)
		totalWidth = currentPos;
	Header_Centre->size.x = totalWidth;
	Header_Right->trans.x = totalWidth;

	SaveBlock *save = GetSaveFile()->GetBlock(-1);
	headerCol.colourise(save->hudHintH%1000, save->hudHintS, save->hudHintL);
}


void dWMHud_c::loadFooterInfo() {
	SaveBlock *save = GetSaveFile()->GetBlock(-1);

	wchar_t convertedWorldName[32];
	int i;
	for (i = 0; i < 32; i++) {
		convertedWorldName[i] = save->newerWorldName[i];
		if (convertedWorldName[i] == 0)
			break;
	}
	convertedWorldName[31] = 0;

	WorldName->SetString(convertedWorldName);
	WorldNameS->SetString(convertedWorldName);

	WorldName->colour1 = save->hudTextColours[0];
	WorldName->colour2 = save->hudTextColours[1];

	footerCol.colourise(save->hudHintH%1000, save->hudHintS, save->hudHintL);

	// figure out if stars are needed
	// Star 0: world is complete
	// Star 1: all exits complete
	// Star 2: all star coins obtained
	
	bool starVisibility[3];
	starVisibility[0] = false;

	dLevelInfo_c *linfo = &dLevelInfo_c::s_info;
	dLevelInfo_c::entry_s *lastLevel = linfo->searchByDisplayNum(save->newerWorldID, lastLevelIDs[save->newerWorldID]);
	if (lastLevel) {
		starVisibility[0] = (save->GetLevelCondition(lastLevel->worldSlot,lastLevel->levelSlot) & COND_NORMAL);
	}

	// now calculate the other two
	starVisibility[1] = true;
	starVisibility[2] = true;

	dLevelInfo_c::section_s *sect = linfo->getSectionByIndex(save->newerWorldID);

	for (int i = 0; i < sect->levelCount; i++) {
		dLevelInfo_c::entry_s *entry = &sect->levels[i];

		if (entry->flags & 2) {
			u32 conds = save->GetLevelCondition(entry->worldSlot, entry->levelSlot);

			if (((entry->flags & 0x10) && !(conds & COND_NORMAL)) ||
					((entry->flags & 0x20) && !(conds & COND_SECRET)))
						starVisibility[1] = false;

			if ((conds & COND_COIN_ALL) != COND_COIN_ALL)
				starVisibility[2] = false;
		}
	}

	if (save->newerWorldID == 15) {
		starVisibility[0] = false;
		starVisibility[1] = false;
		starVisibility[2] = false;
	}

	float startX = Star[0]->trans.x;
	for (int i = 0; i < 3; i++) {
		Star[i]->SetVisible(starVisibility[i]);
		Star[i]->trans.x = startX;
		if (starVisibility[i]) {
			startX += Star[i]->size.x + 4.0f;
		}
	}

	WorldName->trans.x = startX + 4.0f;
	WorldNameS->trans.x = startX + 6.0f;
}



void dWMHud_c::enteredNode(dKPNode_s *node) {
	if (node == 0)
		node = dScKoopatlas_c::instance->pathManager.currentNode;

	if (node->type == dKPNode_s::LEVEL && doneFirstShow) {
		willShowHeader = true;
		nodeForHeader = node;
	}
}

void dWMHud_c::leftNode() {
	if (layout.grpHandlers[SHOW_HEADER].frameCtrl.currentFrame > 0.1f) {
		// not hidden

		if ((layout.isAnimOn(SHOW_HEADER) && !(layout.grpHandlers[SHOW_HEADER].frameCtrl.flags & 2))
				|| (!layout.isAnimOn(SHOW_HEADER))) {
			// currently being shown, OR fully shown already
			playHideAnim(SHOW_HEADER);
		}
	}
}


void dWMHud_c::hideFooter() {
	if (isFooterVisible)
		playHideAnim(SHOW_FOOTER);
}

void dWMHud_c::showFooter() {
	if (!doneFirstShow)
		return;
	willShowFooter = true;
	if (isFooterVisible)
		playHideAnim(SHOW_FOOTER);
}


void dWMHud_c::setupLives() {
	static const int LogicalPlayerIDs[] = {0,1,3,2};

	P_marioFace_00->SetVisible(false);
	P_luigiFace_00->SetVisible(false);
	P_BkinoFace_00->SetVisible(false);
	P_YkinoFace_00->SetVisible(false);

	int playerCount = 0;

	for (int i = 0; i < 4; i++) {
		// The part in setupLives()
		int playerID = LogicalPlayerIDs[i];
		int slotID = SearchForIndexOfPlayerID(playerID);
		int lives = Player_Lives[slotID];
		int length = 2;

		WriteNumberToTextBox(&lives, &length, T_lifeNumber[slotID], true);

		// The part in setupIconThings()
		if (QueryPlayerAvailability(slotID)) {
			playerCount++;

			nw4r::lyt::Pane *facePane = (&P_marioFace_00)[playerID];
			facePane->trans = N_IconPosXP_00[playerCount - 1]->trans;
			facePane->SetVisible(true);
		}
	}

	for (int i = 0; i < 4; i++)
		N_IconPosXP_00[i]->SetVisible(false);
	N_IconPosXP_00[playerCount - 1]->SetVisible(true);
}

void dWMHud_c::updatePressableButtonThingies() {
	int cntType = RemoconMng->controllers[0]->controllerType;

	if (cntType != displayedControllerType) {
		displayedControllerType = cntType;

		int beef = (cntType == 0) ? 0 : 1;
		GameMgrP->currentControllerType = beef;

		WriteBMGToTextBox(
				layout.findTextBoxByName("ItemsButtonInfo"),
				GetBMG(), 4, 15, 0);
	}
}


