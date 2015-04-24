#include "koopatlas/starcoin.h"
#include <game.h>

dWMStarCoin_c *dWMStarCoin_c::instance = 0;

dWMStarCoin_c *dWMStarCoin_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dWMStarCoin_c));
	dWMStarCoin_c *c = new(buffer) dWMStarCoin_c;

	instance = c;
	return c;
}

dWMStarCoin_c::dWMStarCoin_c() : state(this) {
	layoutLoaded = false;
	visible = false;
	state.setState(&StateID_Hidden);
}

CREATE_STATE(dWMStarCoin_c, Hidden);
CREATE_STATE(dWMStarCoin_c, ShowWait);
CREATE_STATE(dWMStarCoin_c, ShowSectionWait);
CREATE_STATE(dWMStarCoin_c, Wait);
CREATE_STATE(dWMStarCoin_c, HideSectionWait);
CREATE_STATE(dWMStarCoin_c, HideWait);

int dWMStarCoin_c::onCreate() {

	if (!layoutLoaded) {
		bool gotFile = layout.loadArc("StarCoins.arc", false);
		if (!gotFile)
			return false;

		bool output = layout.build("StarCoins.brlyt");

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
		}

		static const char *brlanNames[] = {
			"StarCoins_Show.brlan",
			"StarCoins_ShowSection.brlan",
			"StarCoins_HideSection.brlan",
			"StarCoins_ShowArrow.brlan",
			"StarCoins_HideArrow.brlan",
		};
		static const char *groupNames[] = {
			"base", "section", "section", "leftArrow", "leftArrow", "rightArrow", "rightArrow"
		};

		layout.loadAnimations(brlanNames, 5);
		layout.loadGroups(groupNames, (int[7]){0, 1, 2, 3, 4, 3, 4}, 7);
		layout.disableAllAnimations();

		layout.drawOrder = 1;

		for (int col = 0; col < COLUMN_COUNT; col++) {
			for (int shine = 0; shine < SHINE_COUNT; shine++) {
				char name[8];
				sprintf(name, "Shine%d%d", col, shine);
				Shine[col][shine] = layout.findPictureByName(name);
			}

			for (int row = 0; row < ROW_COUNT; row++) {
				char lname[12];
				sprintf(lname, "LevelName%d%d", col, row);
				LevelName[col][row] = layout.findTextBoxByName(lname);

				char coname[16], cname[8];
				for (int i = 0; i < 3; i++) {
					sprintf(coname, "CoinOutline%d%d%d", col, row, i);
					CoinOutline[col][row][i] = layout.findPictureByName(coname);

					sprintf(cname, "Coin%d%d%d", col, row, i);
					Coin[col][row][i] = layout.findPictureByName(cname);
				}
			}
		}

		static const char *tbNames[] = {
			"LeftTitle", "RightTitle", "TotalCoinCount", "UnspentCoinCount",
			"EarnedCoinCount", "EarnedCoinMax", "BtnBackText",
		};
		layout.getTextBoxes(tbNames, &LeftTitle, 7);

		static const char *picNames[] = {
			"DPadLeft", "DPadRight",
		};
		layout.getPictures(picNames, &DPadLeft, 2);

		DPadLeft->SetVisible(false);
		DPadRight->SetVisible(false);

		layoutLoaded = true;
	}

	return true;
}


int dWMStarCoin_c::onDelete() {
	return layout.free();
}


void dWMStarCoin_c::show() {
	if (state.getCurrentState() == &StateID_Hidden)
		state.setState(&StateID_ShowWait);
}


int dWMStarCoin_c::onExecute() {
	state.execute();

	if (visible) {
		layout.execAnimations();
		layout.update();
	}

	return true;
}

int dWMStarCoin_c::onDraw() {
	if (visible)
		layout.scheduleForDrawing();

	return true;
}


void dWMStarCoin_c::showLeftArrow() {
	if (!isLeftArrowVisible) {
		isLeftArrowVisible = true;
		layout.enableNonLoopAnim(SHOW_LEFT_ARROW);
		DPadLeft->SetVisible(true);
	}
}

void dWMStarCoin_c::showRightArrow() {
	if (!isRightArrowVisible) {
		isRightArrowVisible = true;
		layout.enableNonLoopAnim(SHOW_RIGHT_ARROW);
		DPadRight->SetVisible(true);
	}
}

void dWMStarCoin_c::hideLeftArrow() {
	if (isLeftArrowVisible) {
		isLeftArrowVisible = false;
		layout.enableNonLoopAnim(HIDE_LEFT_ARROW);
		DPadLeft->SetVisible(false);
	}
}

void dWMStarCoin_c::hideRightArrow() {
	if (isRightArrowVisible) {
		isRightArrowVisible = false;
		layout.enableNonLoopAnim(HIDE_RIGHT_ARROW);
		DPadRight->SetVisible(false);
	}
}

void dWMStarCoin_c::setLeftArrowVisible(bool value) {
	if (value)
		showLeftArrow();
	else
		hideLeftArrow();
}

void dWMStarCoin_c::setRightArrowVisible(bool value) {
	if (value)
		showRightArrow();
	else
		hideRightArrow();
}


bool dWMStarCoin_c::canScrollLeft() const {
	return (currentSectionIndex > 0);
}
bool dWMStarCoin_c::canScrollRight() const {
	return (currentSectionIndex < (availableSectionCount - 1));
}

void dWMStarCoin_c::loadInfo() {
	WriteBMGToTextBox(BtnBackText, GetBMG(), 3, 1, 0);

	int unspentCoins = getUnspentStarCoinCount();
	int coins = getStarCoinCount();

	WriteNumberToTextBox(&unspentCoins, UnspentCoinCount, false);
	WriteNumberToTextBox(&coins, TotalCoinCount, false);

	currentSection = -1;
	currentSectionIndex = -1;
	availableSectionCount = 0;

	SaveBlock *save = GetSaveFile()->GetBlock(-1);
	int wantedSection = save->newerWorldID;

	// figure out which sections should be available
	for (int i = 0; i < dLevelInfo_c::s_info.sectionCount(); i++) {
		dLevelInfo_c::section_s *section = dLevelInfo_c::s_info.getSectionByIndex(i);

		bool haveLevels = false;
		for (int j = 0; j < section->levelCount; j++) {
			dLevelInfo_c::entry_s *l = &section->levels[j];
			if (l->flags & 2) {
				if (save->GetLevelCondition(l->worldSlot, l->levelSlot) & COND_UNLOCKED) {
					haveLevels = true;
					break;
				}
			}
		}

		if (haveLevels) {
			if (i == wantedSection) {
				currentSection = wantedSection;
				currentSectionIndex = availableSectionCount;
			}
			sectionIndices[availableSectionCount++] = i;
		}
	}

	// if we didn't find the wanted one, use the first one available
	if (currentSectionIndex == -1) {
		currentSectionIndex = 0;
		currentSection = sectionIndices[0];
	}
}

void dWMStarCoin_c::loadSectionInfo() {
	dLevelInfo_c::entry_s *visibleLevels[COLUMN_COUNT][ROW_COUNT];

	// reset everything... everything
	for (int i = 0; i < COLUMN_COUNT; i++) {
		for (int j = 0; j < SHINE_COUNT; j++)
			Shine[i][j]->SetVisible(false);

		for (int j = 0; j < ROW_COUNT; j++) {
			visibleLevels[i][j] = 0;

			LevelName[i][j]->SetVisible(false);

			for (int k = 0; k < 3; k++) {
				CoinOutline[i][j][k]->SetVisible(false);
				Coin[i][j][k]->SetVisible(false);
			}
		}
	}

	// get everything we'll need
	SaveBlock *save = GetSaveFile()->GetBlock(-1);
	dLevelInfo_c *linfo = &dLevelInfo_c::s_info;

	dLevelInfo_c::entry_s *names[COLUMN_COUNT];
	for (int i = 0; i < COLUMN_COUNT; i++)
		names[i] = linfo->searchByDisplayNum(currentSection, 100+i);

	bool useSubworlds = (COLUMN_COUNT > 1) && names[1];

	int currentPosition[COLUMN_COUNT];
	int currentColumn = 0; // only incremented in single-subworld mode

	for (int i = 0; i < COLUMN_COUNT; i++)
		currentPosition[i] = 0;

	dLevelInfo_c::section_s *section = linfo->getSectionByIndex(currentSection);

	int earnedCoins = 0, earnableCoins = 0;
	// earnedCoins is calculated later

	for (int i = 0; i < section->levelCount; i++) {
		dLevelInfo_c::entry_s *level = &section->levels[i];

		// only pay attention to real levels
		if (!(level->flags & 2))
			continue;

		earnableCoins += 3;

		// is this level unlocked?
		u32 conds = save->GetLevelCondition(level->worldSlot, level->levelSlot);

		if (!(conds & COND_UNLOCKED))
			continue;

		// well, let's give it a slot
		if (useSubworlds) {
			currentColumn = (level->flags & 0x400) ? 1 : 0;
		} else {
			if (currentPosition[currentColumn] >= ROW_COUNT)
				currentColumn++;
		}

		visibleLevels[currentColumn][currentPosition[currentColumn]++] = level;
	}

	// if the first column is empty, then move the second one over
	if (currentPosition[0] == 0 && useSubworlds) {
		for (int i = 0; i < currentPosition[1]; i++) {
			visibleLevels[0][i] = visibleLevels[1][i];
			visibleLevels[1][i] = 0;
		}

		names[0] = names[1];
		names[1] = 0;
	}

	// if the second column is empty, remove its name
	if (currentPosition[1] == 0 && useSubworlds)
		names[1] = 0;

	// work out the names
	WriteAsciiToTextBox(LeftTitle, linfo->getNameForLevel(names[0]));
	if (names[1])
		WriteAsciiToTextBox(RightTitle, linfo->getNameForLevel(names[1]));
	RightTitle->SetVisible(names[1] != 0);

	// load all level info
	for (int col = 0; col < COLUMN_COUNT; col++) {
		for (int row = 0; row < ROW_COUNT; row++) {
			dLevelInfo_c::entry_s *level = visibleLevels[col][row];
			if (!level)
				continue;

			u32 conds = save->GetLevelCondition(level->worldSlot, level->levelSlot);

			if (!(row & 1)) {
				int shineID = row / 2;
				if (shineID < SHINE_COUNT)
					Shine[col][shineID]->SetVisible(true);
			}

			for (int coin = 0; coin < 3; coin++) {
				CoinOutline[col][row][coin]->SetVisible(true);

				if (conds & (COND_COIN1 << coin)) {
					Coin[col][row][coin]->SetVisible(true);
					earnedCoins++;
				}
			}

			LevelName[col][row]->SetVisible(true);
			WriteAsciiToTextBox(LevelName[col][row], linfo->getNameForLevel(level));
		}
	}

	// set up coin things
	WriteNumberToTextBox(&earnedCoins, EarnedCoinCount, false);
	WriteNumberToTextBox(&earnableCoins, EarnedCoinMax, false);
}


void dWMStarCoin_c::beginState_Hidden() { }
void dWMStarCoin_c::executeState_Hidden() { }
void dWMStarCoin_c::endState_Hidden() { }

static const int secretCode[] = {
	WPAD_UP,WPAD_UP,WPAD_DOWN,WPAD_DOWN,
	WPAD_LEFT,WPAD_RIGHT,WPAD_LEFT,WPAD_RIGHT,
	WPAD_ONE,WPAD_TWO,0
};
static const int secretCodeButtons = WPAD_UP|WPAD_DOWN|WPAD_LEFT|WPAD_RIGHT|WPAD_ONE|WPAD_TWO;
static int secretCodeIndex = 0;
static int minusCount = 0;
extern bool enableHardMode;
extern bool enableDebugMode;
extern u8 isReplayEnabled;

void dWMStarCoin_c::beginState_ShowWait() {
	visible = true;
	loadInfo();
	layout.enableNonLoopAnim(SHOW_ALL);
	layout.resetAnim(SHOW_SECTION);
	layout.resetAnim(SHOW_LEFT_ARROW);
	layout.resetAnim(SHOW_RIGHT_ARROW);

	secretCodeIndex = 0;
	minusCount = 0;
}
void dWMStarCoin_c::executeState_ShowWait() {
	if (!layout.isAnimOn(SHOW_ALL))
		state.setState(&StateID_ShowSectionWait);
}
void dWMStarCoin_c::endState_ShowWait() { }

void dWMStarCoin_c::beginState_ShowSectionWait() {
	loadSectionInfo();
	layout.enableNonLoopAnim(SHOW_SECTION);

	if (canScrollLeft())
		showLeftArrow();
	if (canScrollRight())
		showRightArrow();
}
void dWMStarCoin_c::executeState_ShowSectionWait() {
	if (!layout.isAnimOn(SHOW_SECTION))
		state.setState(&StateID_Wait);
}
void dWMStarCoin_c::endState_ShowSectionWait() { }

void dWMStarCoin_c::showSecretMessage(const wchar_t *title, const wchar_t **body, int lineCount, const wchar_t **body2, int lineCount2) {
	LeftTitle->SetVisible(true);
	LeftTitle->SetString(title);
	RightTitle->SetVisible(false);

	for (int c = 0; c < COLUMN_COUNT; c++) {
		for (int i = 0; i < SHINE_COUNT; i++)
			Shine[c][i]->SetVisible(false);
		for (int r = 0; r < ROW_COUNT; r++) {
			LevelName[c][r]->SetVisible(false);
			for (int i = 0; i < 3; i++) {
				CoinOutline[c][r][i]->SetVisible(false);
				Coin[c][r][i]->SetVisible(false);
			}
		}
	}

	for (int i = 0; i < lineCount; i++) {
		LevelName[0][i]->SetVisible(true);
		LevelName[0][i]->SetString(body[i]);
	}

	if (body2) {
		for (int i = 0; i < lineCount2; i++) {
			LevelName[1][i]->SetVisible(true);
			LevelName[1][i]->SetString(body2[i]);
		}
	}
}

void dWMStarCoin_c::beginState_Wait() { }
void dWMStarCoin_c::executeState_Wait() {
	int nowPressed = Remocon_GetPressed(GetActiveRemocon());

	if ((GetActiveRemocon()->heldButtons == 0xc10) && (nowPressed & 0xc10)) { // A, B, and Plus

		const int lineCountOn = 9, lineCountOff = 2;
		static const wchar_t *linesOn[lineCountOn] = {
			L"You've activated Hard Mode!",
			L" ",
			L"In Hard Mode, Mario will die",
			L"any time he takes damage, and",
			L"the timer will be more strict.",
			L" ",
			L"So treasure your Yoshi, and",
			L"hold on to your hat, you're",
			L"in for a wild ride!",
		};
		static const wchar_t *linesOff[lineCountOff] = {
			L"Hard Mode has been",
			L"turned off.",
		};

		if (!enableHardMode) {
			enableHardMode = true;
			OSReport("Hard Mode enabled!\n");
			MapSoundPlayer(SoundRelatedClass, SE_VOC_MA_CS_COURSE_IN_HARD, 1);
			showSecretMessage(L"Hard Mode", linesOn, lineCountOn);
		} else {
			enableHardMode = false;
			OSReport("Hard Mode disabled!\n");
			showSecretMessage(L"Classic Mario", linesOff, lineCountOff);
		}
		return;
	}

	if (nowPressed & secretCodeButtons) {
		int nextKey = secretCode[secretCodeIndex];
		if (nowPressed & nextKey) {
			secretCodeIndex++;
			if (secretCode[secretCodeIndex] == 0) {
				secretCodeIndex = 0;
				MapSoundPlayer(SoundRelatedClass, SE_VOC_MA_THANK_YOU, 1);
				//enableDebugMode = !enableDebugMode;
				//OSReport("Debug mode toggled!\n");
				const int lineCountOn = 9, lineCountOff = 2;
				static const wchar_t *linesOn[lineCountOn] = {
					L"The experimental Replay",
					L"Recording feature has",
					L"been enabled. Enjoy!",
					L"You'll find your Replays",
					L"on your SD or USB, depending",
					L"on where Newer's files are.",
					L"It might not work, so",
					L"save your game before you",
					L"play a level!",
				};
				static const wchar_t *linesOff[lineCountOff] = {
					L"Replay Recording",
					L"turned off.",
				};

				if (isReplayEnabled != 100) {
					isReplayEnabled = 100;
					OSReport("Replay Recording enabled!\n");
					showSecretMessage(L"Nice!", linesOn, lineCountOn);
				} else {
					isReplayEnabled = 0;
					OSReport("Replay Recording disabled!\n");
					showSecretMessage(L"Nice!", linesOff, lineCountOff);
				}
			}
			return;
		} else {
			secretCodeIndex = 0;
		}
	}

	if (nowPressed & WPAD_MINUS) {
		minusCount++;
		if (minusCount >= 16) {
			minusCount = 0;

			enableDebugMode = !enableDebugMode;

			if (enableDebugMode) {
				MapSoundPlayer(SoundRelatedClass, SE_VOC_MA_GET_PRIZE, 1);

				const int msgCount = 9;
				static const wchar_t *msg[msgCount] = {
					L"You've found the Totally",
					L"Secret Collision Debug Mode.",
					L"We used this to make the",
					L"hitboxes on our custom sprites",
					L"and bosses suck less. Awesome,",
					L"right?!",
					L"Actually, I did it just to waste",
					L"some time, but it ended up",
					L"being pretty useful!",
				};
				const int msgCount2 = 9;
				static const wchar_t *msg2[msgCount2] = {
					L"And yes, I know it doesn't show",
					L"a couple of things properly",
					L"like round objects and rolling",
					L"hills and so on.",
					L"Can't have it all, can you?",
					L"Wonder if Nintendo had",
					L"something like this...",
					L"",
					L"    Treeki, 9th February 2013",
				};
				showSecretMessage(L"Groovy!", msg, msgCount, msg2, msgCount2);
			} else {
				const int msgCount = 6;
				static const wchar_t *msg[msgCount] = {
					L"You've turned off the Totally",
					L"Secret Collision Debug Mode.",
					L"",
					L"... and no, I'm not going to write",
					L"another ridiculously long",
					L"message to go here. Sorry!",
				};
				static const wchar_t *hiddenMsg[] = {
					L"If you found these messages by",
					L"looking through strings in the DLCode",
					L"file, then... that's kind of cheating.",
					L"Though I can't say I wouldn't do the",
					L"same!",
					L"You won't actually see this in game",
					L"btw :p So why am I bothering with linebreaks anyway? I dunno. Oh well.",
					L"Also, don't put this message on TCRF. Or do! Whatever. :(",
				};
				showSecretMessage(L"Groovy!", msg, msgCount, hiddenMsg, 0);
			}
		}
	} else if (nowPressed & WPAD_ONE) {
		MapSoundPlayer(SoundRelatedClass, SE_SYS_DIALOGUE_OUT_AUTO, 1);
		willExit = true;
		state.setState(&StateID_HideSectionWait);
	} else if ((nowPressed & WPAD_LEFT) && canScrollLeft()) {
		currentSection = sectionIndices[--currentSectionIndex];
		willExit = false;
		state.setState(&StateID_HideSectionWait);
	} else if ((nowPressed & WPAD_RIGHT) && canScrollRight()) {
		currentSection = sectionIndices[++currentSectionIndex];
		willExit = false;
		state.setState(&StateID_HideSectionWait);
	}
}
void dWMStarCoin_c::endState_Wait() { }

void dWMStarCoin_c::beginState_HideSectionWait() {
	layout.enableNonLoopAnim(HIDE_SECTION);
	if (willExit) {
		hideLeftArrow();
		hideRightArrow();
	} else {
		setLeftArrowVisible(canScrollLeft());
		setRightArrowVisible(canScrollRight());
	}
}
void dWMStarCoin_c::executeState_HideSectionWait() {
	if (!layout.isAnimOn(HIDE_SECTION)) {
		if (willExit)
			state.setState(&StateID_HideWait);
		else
			state.setState(&StateID_ShowSectionWait);
	}
}
void dWMStarCoin_c::endState_HideSectionWait() { }

void dWMStarCoin_c::beginState_HideWait() {
	layout.enableNonLoopAnim(SHOW_ALL, true);
	layout.grpHandlers[SHOW_ALL].frameCtrl.flags = 3; // NO_LOOP | REVERSE
}
void dWMStarCoin_c::executeState_HideWait() {
	if (!layout.isAnimOn(SHOW_ALL))
		state.setState(&StateID_Hidden);
}
void dWMStarCoin_c::endState_HideWait() {
	visible = false;
}

