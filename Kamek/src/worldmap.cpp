#include "worldmap.h"

extern "C" void LoadMapScene();

dScNewerWorldMap_c *dScNewerWorldMap_c::instance = 0;


dScNewerWorldMap_c *dScNewerWorldMap_c::build() {
	// return new dScNewerWorldMap_c;

	void *buffer = AllocFromGameHeap1(sizeof(dScNewerWorldMap_c));
	dScNewerWorldMap_c *c = new(buffer) dScNewerWorldMap_c;


	instance = c;
	return c;
}


#define SELC_SETUP_DONE(sc) (*((bool*)(((u32)(sc))+0xD38)))

#define EASYP_SETUP_DONE(ep) (*((bool*)(((u32)(ep))+0x278)))
#define EASYP_ACTIVE(ep) (*((bool*)(((u32)(ep))+0x279)))

#define CSMENU_SETUP_DONE(csm) (*((bool*)(((u32)(csm))+0x270)))
#define CSMENU_ACTIVE(csm) (*((bool*)(((u32)(csm))+0x271)))
#define CSMENU_CHOICE_OK(csm) (*((bool*)(((u32)(csm))+0x272)))
#define CSMENU_UNK(csm) (*((bool*)(((u32)(csm))+0x273)))
#define CSMENU_CURRENT(csm) (*((int*)(((u32)(csm))+0x268)))

#define YESNO_SETUP_DONE(ynw) (*((bool*)(((u32)(ynw))+0x294)))
#define YESNO_VISIBLE(ynw) (*((bool*)(((u32)(ynw))+0x295)))
#define YESNO_CLOSE(ynw) (*((bool*)(((u32)(ynw))+0x296)))
#define YESNO_OPENING(ynw) (*((bool*)(((u32)(ynw))+0x297)))
#define YESNO_REFUSED(ynw) (*((bool*)(((u32)(ynw))+0x298)))
#define YESNO_CANCELLED(ynw) (*((bool*)(((u32)(ynw))+0x299)))
#define YESNO_CANCELLED2(ynw) (*((bool*)(((u32)(ynw))+0x29A)))
#define YESNO_CURRENT(ynw) (*((int*)(((u32)(ynw))+0x284)))
#define YESNO_TYPE(ynw) (*((int*)(((u32)(ynw))+0x28C)))

#define NPCHG_SETUP_DONE(npc) (*((bool*)(((u32)(npc))+0x67C)))
#define NPCHG_ACTIVE(npc) (*((bool*)(((u32)(npc))+0x67E)))
#define NPCHG_HIDE_FOR_EASYP(npc) (*((bool*)(((u32)(npc))+0x67F)))
#define NPCHG_READY(npc) (*((bool*)(((u32)(npc))+0x680)))
#define NPCHG_CCSB(npc,idx) (((void**)(((u32)(npc))+0x74))[(idx)])
#define NPCHG_CCSC(npc,idx) (((void**)(((u32)(npc))+0x84))[(idx)])
#define NPCHG_CCSA(npc,idx) (((void**)(((u32)(npc))+0x94))[(idx)])
#define NPCHG_CCI(npc,idx) (((void**)(((u32)(npc))+0xA4))[(idx)])
#define NPCHG_2DPLAYER(npc,idx) (((void**)(((u32)(npc))+0x64C))[(idx)])

#define STKI_SETUP_DONE(si) (*((bool*)(((u32)(si))+0x310)))
#define STKI_SHADOW(si) (*((void**)(((u32)(si))+0x310)))
#define STKI_2DPLAYER(si,idx) (((void**)(((u32)(si))+0x2E4))[(idx)])
#define STKI_ITEM(si,idx) (((void**)(((u32)(si))+0x2F4))[(idx)])
#define STKI_SHOW(si) (*((bool*)(((u32)(si))+0x8DD)))

#define SIS_SETUP_DONE(sis) (*((bool*)(((u32)(sis))+0x260)))

#define CCSB_ACTIVE(ccsb) (*((bool*)(((u32)(ccsb))+0x29C)))

#define CCSC_ACTIVE(ccsc) (*((bool*)(((u32)(ccsc))+0x2A1)))

#define PLAYER2D_SHOW_EASY_PAIRING(p2d) (*((bool*)(((u32)(p2d))+0x264)))

#define CONT_LIVES(cont,idx) (((int*)(((u32)(cont))+0x2B8))[(idx)])
#define CONT_SETUP_DONE(cont) (*((bool*)(((u32)(cont))+0x2D4)))
#define CONT_UNK1(cont) (*((bool*)(((u32)(cont))+0x2D5)))
#define CONT_UNK2(cont) (*((bool*)(((u32)(cont))+0x2D6)))
#define CONT_DONE(cont) (*((bool*)(((u32)(cont))+0x2D7)))
#define CONT_UNK3(cont) (*((bool*)(((u32)(cont))+0x2E0)))

inline u8 GetSwitchStatus() {
	return *((u8*)(((u32)GameMgr)+0x380));
}

inline void SetSwitchStatus(u8 stat) {
	*((u8*)(((u32)GameMgr)+0x380)) = stat;
}

#define STATE_START_DVD 0
#define STATE_LOAD_RES 1
#define STATE_END_DVD 2
#define STATE_SETUP_WAIT 3
#define STATE_LIMBO 4
#define STATE_CONTINUE_WAIT 5
#define STATE_NORMAL 6
#define STATE_OPT_CHANGE_WAIT 7
#define STATE_CSMENU 8
#define STATE_TITLE_CONFIRM_OPEN_WAIT 9
#define STATE_TITLE_CONFIRM_SELECT 10
#define STATE_TITLE_CONFIRM_HIT_WAIT 11
#define STATE_PLAYER_CHANGE_WAIT 12
#define STATE_EASY_PAIRING_WAIT 13
#define STATE_POWERUPS_WAIT 14
#define STATE_SAVE_OPEN 15
#define STATE_SAVE_SELECT 16
#define STATE_SAVE_WINDOW_CLOSE 17
#define STATE_SAVE_DO 18
#define STATE_SAVE_END_WINDOW 19
#define STATE_SAVE_END_CLOSE_WAIT 20
#define STATE_QUICKSAVE_OPEN 21
#define STATE_QUICKSAVE_SELECT 22
#define STATE_QUICKSAVE_WINDOW_CLOSE 23
#define STATE_QUICKSAVE_DO 24
#define STATE_QUICKSAVE_END_WINDOW 25
#define STATE_QUICKSAVE_END_CLOSE_WAIT 26
#define STATE_SAVE_ERROR 27

#define MENU_HEIGHT 15

const char *anim1 = "optionActivated.brlan";
const char *anim2 = "optionDeactivated.brlan";

const char *group1 = "G_opt00";
const char *group2 = "G_opt01";
const char *group3 = "G_opt02";
const char *group4 = "G_opt03";
const char *group5 = "G_opt04";
const char *group6 = "G_opt05";
const char *group7 = "G_opt06";
const char *group8 = "G_opt07";
const char *group9 = "G_opt08";
const char *group10 = "G_opt09";
const char *group11 = "G_opt10";
const char *group12 = "G_opt11";
const char *group13 = "G_opt12";
const char *group14 = "G_opt13";
const char *group15 = "G_opt14";


void dScNewerWorldMap_c::StartLevel() {
	LevelInfo_Entry *level = LevelInfo_GetLevels(this->levelInfo, this->currentPage);
	level += this->selections[this->currentPage];
	StartLevel(level);
}

void dScNewerWorldMap_c::StartLevel(LevelInfo_Entry *entry) {
	for (int i = 0; i < 4; i++) {
		bool isThere = QueryPlayerAvailability(i);
		int id = Player_ID[i];
		Player_Active[i] = isThere ? 1 : 0;
		if (!isThere) Player_Flags[i] = 0;
	}

	StartLevelInfo sl;
	sl.unk1 = 0;
	sl.unk2 = 0xFF;
	sl.unk3 = 0;
	sl.unk4 = 0;
	sl.purpose = 0;

	sl.world1 = entry->world;
	sl.world2 = entry->world;
	sl.level1 = entry->level;
	sl.level2 = entry->level;

	// hopefully this will fix the Star Coin issues
	SetSomeConditionShit(entry->world, entry->level, 2);

	ActivateWipe(WIPE_MARIO);

	DoStartLevel(GetGameMgr(), &sl);
}

void dScNewerWorldMap_c::SetTitle(const char *text) {
	unsigned short conv_buf[0x200];
	int length = strlen(text);
	if (length > 0x1FF)
		length = 0x1FF;

	for (int i = 0; i < length; i++) {
		conv_buf[i] = text[i];
	}
	conv_buf[length] = 0;

	void *textBox = EmbeddedLayout_FindTextBoxByName(this->layout, "ScreenTitle");
	TextBox_SetString(textBox, conv_buf, 0);
}

void dScNewerWorldMap_c::GenSBTitle() {
	char buf[0x100];
	sprintf(buf, "Switches: R:%s; G:%s; Y:%s; B:%s",
			((GetSwitchStatus() & 1) != 0) ? "On" : "Off",
			((GetSwitchStatus() & 2) != 0) ? "On" : "Off",
			((GetSwitchStatus() & 4) != 0) ? "On" : "Off",
			((GetSwitchStatus() & 8) != 0) ? "On" : "Off");
	this->SetTitle(buf);
}

void dScNewerWorldMap_c::GenText() {
	char buf[0x1FF];
	char paneNameBuf[0x20];
	char textBoxNameBuf[0x20];
	unsigned short wchars[0x1FF];

	SaveBlock *save = GetSaveFile()->GetBlock(-1);

	LevelInfo_Section *section = LevelInfo_GetSection(this->levelInfo, this->currentPage);
	LevelInfo_Entry *levels = LevelInfo_GetLevels(this->levelInfo, section);

	int count = section->levelCount;

	for (int i = 0; i < MENU_HEIGHT; i++) {
		sprintf(paneNameBuf, "Opt%02d", i);
		sprintf(textBoxNameBuf, "OptText%02d", i);
		void *pane = EmbeddedLayout_FindPaneByName(this->layout, paneNameBuf);
		void *textBox = EmbeddedLayout_FindTextBoxByName(this->layout, textBoxNameBuf);

		if (i < count) {
			// valid level
			PANE_FLAGS(pane) |= 1;

			u32 conds = save->GetLevelCondition(levels[i].world, levels[i].level);

			char cond1, cond2, cond3, cond4, cond5;
			cond1 = (conds & COND_NORMAL ? 'x' : '.');
			cond2 = (conds & COND_SECRET ? 'x' : '.');
			cond3 = (conds & COND_COIN1 ? 'x' : '.');
			cond4 = (conds & COND_COIN2 ? 'x' : '.');
			cond5 = (conds & COND_COIN3 ? 'x' : '.');

			sprintf(buf, "%s %c%c %c%c%c", LevelInfo_GetName(this->levelInfo, &levels[i]), cond1, cond2, cond3, cond4, cond5);

			for (int i = 0; i < 0x1FF; i++) {
				wchars[i] = buf[i];
				if (buf[i] == 0) break;
			}

			TextBox_SetString(textBox, wchars, 0);
		} else {
			// invalid, hide the pane
			PANE_FLAGS(pane) &= ~1;
		}
	}
}

int dScNewerWorldMap_c::onCreate() {

	LoadMapScene();
	GameSetup__LoadScene(0); // lol, stolen from GAME_SETUP

	this->layout = (Layout*)AllocFromGameHeap1(sizeof(Layout));
	if (!this->layout) {
		InfiniteLoop;
	}

	EmbeddedLayout_ctor(this->layout);
	EmbeddedLayout_LoadArc(this->layout, "NewerRes/wmap.arc");

	if (!EmbeddedLayout_Build(this->layout, "levelSelect.brlyt", 0)) {
		InfiniteLoop;
	}


	const char *anims[2] = {anim1, anim2};
	EmbeddedLayout_LoadBrlans(this->layout, anims, 2);

	const char *groups[30] = {
		group1, group2, group3, group4, group5,
		group6, group7, group8, group9, group10,
		group11, group12, group13, group14, group15,
		group1, group2, group3, group4, group5,
		group6, group7, group8, group9, group10,
		group11, group12, group13, group14, group15,
		};

	int mappings[30] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	EmbeddedLayout_LoadGroups(this->layout, groups, mappings, 30);

	EmbeddedLayout_DisableAllAnims(this->layout);

	for (int i = 0; i < 15; i++) {
		EmbeddedLayout_ResetAnimToInitialState(this->layout, i, false);
	}


	this->selectCursor = CreateParentedObject(SELECT_CURSOR, this, 0, 0);
	this->csMenu = CreateParentedObject(COURSE_SELECT_MENU, this, 0, 0);
	this->yesNoWindow = CreateParentedObject(YES_NO_WINDOW, this, 0, 0);
	this->numPeopleChange = CreateParentedObject(NUMBER_OF_PEOPLE_CHANGE, this, 0, 0);

	for (int i = 0; i < 4; i++) {
		void *ccsb = CreateParentedObject(CHARACTER_CHANGE_SELECT_BASE, this, i, 0);
		void *ccsc = CreateParentedObject(CHARACTER_CHANGE_SELECT_CONTENTS, this, i, 0);
		void *ccsa = CreateParentedObject(CHARACTER_CHANGE_SELECT_ARROW, this, i, 0);
		void *cci = CreateParentedObject(CHARACTER_CHANGE_INDICATOR, this, i, 0);

		NPCHG_CCSB(this->numPeopleChange, i) = ccsb;
		NPCHG_CCSC(this->numPeopleChange, i) = ccsc;
		NPCHG_CCSA(this->numPeopleChange, i) = ccsa;
		NPCHG_CCI(this->numPeopleChange, i) = cci;
	}

	this->continueObj = CreateParentedObject(CONTINUE, this, 0, 0);

	this->stockItem = CreateParentedObject(STOCK_ITEM, this, 0, 0);
	this->stockItemShadow = CreateParentedObject(STOCK_ITEM_SHADOW, this, 0, 0);
	STKI_SHADOW(this->stockItem) = this->stockItemShadow;

	this->easyPairing = CreateParentedObject(EASY_PAIRING, this, 0, 0);

	this->state = STATE_START_DVD;

	this->layout->drawOrder = 0;

	*CurrentDrawFunc = NewerMapDrawFunc;

	// level info
	this->levelInfo = LoadFile(&this->levelInfoFH, "/NewerRes/LevelInfo.bin");
	LevelInfo_Prepare(&this->levelInfoFH);

	// load the menu info
	SaveBlock *save = GetSaveFile()->GetBlock(-1);

	this->currentPage = save->current_world;

	// bounds check
	if (save->current_path_node >= 15)
		save->current_path_node = 0;

	int sCount = LevelInfo_GetSectionCount(this->levelInfo);
	this->selections = (int*)AllocFromGameHeap1(sizeof(int) * sCount);

	for (int i = 0; i < sCount; i++) {
		this->selections[i] = 0;
	}

	this->selections[this->currentPage] = save->current_path_node;

	// show button anim
	EmbeddedLayout_EnableNonLoopAnim(this->layout, save->current_path_node, false);

	this->GenText();
	this->GenSBTitle();

	return true;
}

int dScNewerWorldMap_c::onDelete() {
	EmbeddedLayout_FreeArc(this->layout);
	EmbeddedLayout_Free(this->layout);
	EmbeddedLayout_dtor(this->layout, false);
	FreeFromGameHeap1(this->layout);

	FreeFromGameHeap1(this->selections);

	FreeFile(&this->levelInfoFH);

	FreeScene(0);
	FreeScene(1);

	DVD_FreeFile(GetDVDClass2(), "SI_kinoko");
	DVD_FreeFile(GetDVDClass2(), "SI_fireflower");
	DVD_FreeFile(GetDVDClass2(), "SI_iceflower");
	DVD_FreeFile(GetDVDClass2(), "SI_penguin");
	DVD_FreeFile(GetDVDClass2(), "SI_propeller");
	DVD_FreeFile(GetDVDClass2(), "SI_star");

	return true;
}

int dScNewerWorldMap_c::onExecute() {

	if (QueryGlobal5758(0xFFFFFFFF)) return true;
	if (CheckIfWeCantDoStuff()) return true;

	/**************************************************************************/
	// Read Wiimote Buttons

	int heldButtons = Remocon_GetButtons(GetActiveRemocon());
	int nowPressed = Remocon_GetPressed(GetActiveRemocon());

	/**************************************************************************/
	// State Specific

	switch (this->state) {
		/**********************************************************************/
		// STATE_START_DVD : Set up DVD
		case STATE_START_DVD:

			DVD_Start();
			this->state = STATE_LOAD_RES;

			break;

		/**********************************************************************/
		// STATE_LOAD_RES : Load extra stuff we need
		case STATE_LOAD_RES:

			DVD_LoadFile(GetDVDClass(), "WorldMap", "SI_kinoko", 0);
			DVD_LoadFile(GetDVDClass(), "WorldMap", "SI_fireflower", 0);
			DVD_LoadFile(GetDVDClass(), "WorldMap", "SI_iceflower", 0);
			DVD_LoadFile(GetDVDClass(), "WorldMap", "SI_penguin", 0);
			DVD_LoadFile(GetDVDClass(), "WorldMap", "SI_propeller", 0);
			DVD_LoadFile(GetDVDClass(), "WorldMap", "SI_star", 0);
			//DVD_LoadFile(GetDVDClass(), "Object", "fruits_kusa_gake", 0);

			this->state = STATE_END_DVD;

			break;

		/**********************************************************************/
		// STATE_END_DVD : Wait for files to load, end DVD
		case STATE_END_DVD:

			if (!DVD_StillLoading(GetDVDClass2())) {
				if (DVD_End()) {
					this->state = STATE_SETUP_WAIT;
				}
			}

			break;

		/**********************************************************************/
		// STATE_SETUP_WAIT : Waiting for the world map managers to be set up
		case STATE_SETUP_WAIT: {

			bool success = true;

			success &= CSMENU_SETUP_DONE(this->csMenu);
			success &= SELC_SETUP_DONE(this->selectCursor);
			success &= NPCHG_SETUP_DONE(this->numPeopleChange);
			success &= YESNO_SETUP_DONE(this->yesNoWindow);
			success &= CONT_SETUP_DONE(this->continueObj);
			success &= STKI_SETUP_DONE(this->stockItem);
			success &= SIS_SETUP_DONE(this->stockItemShadow);
			success &= EASYP_SETUP_DONE(this->easyPairing);

			if (success) {
				// ok, now we can set up other required shit

				// first up: player models for Stocked Items
				for (int i = 0; i < 4; i++) {
					void *obj = CreateChildObject(WM_2D_PLAYER, this, i, 0, 0);
					STKI_2DPLAYER(this->stockItem,i) = obj;
					NPCHG_2DPLAYER(this->numPeopleChange,i) = obj;
				}

				// next: items for the Powerup screen
				for (int i = 0; i < 7; i++) {
					void *obj = CreateChildObject(WM_ITEM, this, i, 0, 0);
					STKI_ITEM(this->stockItem,i) = obj;
				}

				// now, check if we need to handle Continue
				if (CheckIfContinueShouldBeActivated()) {
					this->state = STATE_CONTINUE_WAIT;
					CONT_UNK1(this->continueObj) = true;
					CONT_UNK2(this->continueObj) = true;
					CONT_UNK3(this->continueObj) = false;
				} else {
					this->state = STATE_OPT_CHANGE_WAIT;
				}
			}

		} break;

		/**********************************************************************/
		// STATE_CONTINUE_WAIT : Waiting for the Continue anim to finish
		case STATE_CONTINUE_WAIT:

			if (CONT_DONE(this->continueObj)) {
				CONT_UNK1(this->continueObj) = 0;
				CONT_UNK2(this->continueObj) = 0;
				CONT_UNK3(this->continueObj) = 0;

				for (int i = 0; i < 4; i++) {
					int idx = SearchForIndexOfPlayerID(i);
					Player_Lives[Player_ID[idx]] = CONT_LIVES(this->continueObj, i);
				}

				this->state = STATE_OPT_CHANGE_WAIT;
			}

			break;

		/**********************************************************************/
		// STATE_NORMAL : Nothing related to the menu is going on
		case STATE_NORMAL: {

			int currentPage = this->currentPage;
			int currentSelection = this->selections[currentPage];
			int newPage = currentPage;
			int newSelection = currentSelection;

			// Activate the menu
			if (nowPressed & WPAD_PLUS) {
				CSMENU_ACTIVE(this->csMenu) = true;
				this->state = STATE_CSMENU;
			}

			if (nowPressed & WPAD_B) {
				SetSwitchStatus((GetSwitchStatus() + 1) & 15);
				this->GenSBTitle();
			}

			// Change the current level
			if ((nowPressed & WPAD_UP) && currentSelection > 0) {
				newSelection = currentSelection - 1;
			}

			if ((nowPressed & WPAD_DOWN) && currentSelection < (LevelInfo_GetSection(this->levelInfo, currentPage)->levelCount - 1)) {
				newSelection = currentSelection + 1;
			}

			// Change the current world
			if ((nowPressed & WPAD_LEFT) && currentPage > 0) {
				newPage = currentPage - 1;
			}

			if ((nowPressed & WPAD_RIGHT) && currentPage < (LevelInfo_GetSectionCount(this->levelInfo) - 1)) {
				newPage = currentPage + 1;
			}


			if (newPage != currentPage) {
				this->currentPage = newPage;
				this->GenText();

				// do this to let the rest of the code handle animations
				newSelection = this->selections[newPage];
			}

			if (newSelection != currentSelection) {
				EmbeddedLayout_DisableAllAnims(this->layout);

				// enable On animation
				EmbeddedLayout_EnableNonLoopAnim(this->layout, newSelection, false);

				// enable Off animation
				EmbeddedLayout_EnableNonLoopAnim(this->layout, currentSelection+15, false);

				this->selections[newPage] = newSelection;
				this->state = STATE_OPT_CHANGE_WAIT;
			}


			// save the info to the file
			if (currentSelection != newSelection || currentPage != newPage) {
				SaveBlock *save = GetSaveFile()->GetBlock(-1);
				save->current_world = newPage;
				save->current_path_node = newSelection;
			}


			// Enter the current level
			if (Wiimote_TestButtons(GetActiveWiimote(), WPAD_A | WPAD_TWO)) {
				this->StartLevel();
				this->state = STATE_LIMBO; // just in case
			}

			if (nowPressed & WPAD_ONE) {
				STKI_SHOW(this->stockItem) = true;
				this->state = STATE_POWERUPS_WAIT;
			}
		} break;

		/**********************************************************************/
		// STATE_OPT_CHANGE_WAIT : Waiting for the option change animation to
		//  finish playing
		case STATE_OPT_CHANGE_WAIT:

			if (!EmbeddedLayout_CheckIfAnimationIsOn(this->layout, -1)) {
				this->state = STATE_NORMAL;
			}

			break;
		/**********************************************************************/
		// STATE_CSMENU : The course select menu is currently being shown
		case STATE_CSMENU:

			// First off, check to see if it's been hidden
			if (!CSMENU_ACTIVE(this->csMenu)) {
				// That means something happened
				if (CSMENU_CHOICE_OK(this->csMenu)) {
					// Player pressed a button

					switch (CSMENU_CURRENT(this->csMenu)) {
						case 0:
							// Star Coins
							this->state = STATE_NORMAL;
							break;

						case 1:
							// Add/Drop Players
							this->state = STATE_PLAYER_CHANGE_WAIT;
							NPCHG_ACTIVE(this->numPeopleChange) = true;
							WpadShit(10);

							break;

						case 2:
							// Save or Quick Save
							if (GetSaveFile()->GetBlock(-1)->bitfield & 2) {
								this->state = STATE_SAVE_OPEN;
								YESNO_TYPE(this->yesNoWindow) = 1;
								YESNO_VISIBLE(this->yesNoWindow) = 1;

							} else {
								this->state = STATE_QUICKSAVE_OPEN;
								YESNO_TYPE(this->yesNoWindow) = 15;
								YESNO_VISIBLE(this->yesNoWindow) = 1;

							}

							break;

						case 3:
							// Title Screen
							this->state = STATE_TITLE_CONFIRM_OPEN_WAIT;
							YESNO_VISIBLE(this->yesNoWindow) = true;
							YESNO_TYPE(this->yesNoWindow) = 10;
							break;
					}

				} else {
					// Ok, change back to STATE_NORMAL
					this->state = STATE_NORMAL;
				}
			}

			break;

		/**********************************************************************/
		// STATE_TITLE_CONFIRM_OPEN_WAIT : Waiting for the "Go to Title Screen"
		// 	YesNoWindow to finish opening
		case STATE_TITLE_CONFIRM_OPEN_WAIT:

			if (!YESNO_OPENING(this->yesNoWindow)) {
				this->state = STATE_TITLE_CONFIRM_SELECT;
			}

			break;

		/**********************************************************************/
		// STATE_TITLE_CONFIRM_SELECT : Let the user choose an option on the
		// 	"Go to Title Screen" YesNoWindow.
		case STATE_TITLE_CONFIRM_SELECT:

			if (nowPressed & WPAD_LEFT) {
				// Select "OK!"
				YESNO_CURRENT(this->yesNoWindow) = 1;

			} else if (nowPressed & WPAD_RIGHT) {
				// Select "Cancel"
				YESNO_CURRENT(this->yesNoWindow) = 0;

			} else if (Wiimote_TestButtons(GetActiveWiimote(), WPAD_A | WPAD_TWO)) {
				// Pick the current option
				YESNO_CLOSE(this->yesNoWindow) = true;
				if (YESNO_CURRENT(this->yesNoWindow) != 1)
					YESNO_REFUSED(this->yesNoWindow) = true;
				this->state = STATE_TITLE_CONFIRM_HIT_WAIT;

			} else {
				// Cancel using B or 1
				if (CheckIfMenuShouldBeCancelledForSpecifiedWiimote(0)) {
					YESNO_CANCELLED(this->yesNoWindow) = true;
					YESNO_CURRENT(this->yesNoWindow) = true;
					this->state = STATE_TITLE_CONFIRM_HIT_WAIT;
				}
			}

			break;

		/**********************************************************************/
		// STATE_TITLE_CONFIRM_HIT_WAIT : Process the user's chosen option on
		// 	the "Go to Title Screen" YesNoWindow. Also, wait for the
		//  animation to be complete.
		case STATE_TITLE_CONFIRM_HIT_WAIT:

			if (!YESNO_OPENING(this->yesNoWindow)) {
				if (YESNO_CURRENT(this->yesNoWindow) == 1) {
					this->state = STATE_NORMAL;
				} else {
					this->state = STATE_LIMBO;
					StartTitleScreenStage(false, 0);
				}
			}

			break;

		/**********************************************************************/
		// STATE_PLAYER_CHANGE_WAIT : Wait for the user to do something on the
		// 	Add/Drop Players screen.
		case STATE_PLAYER_CHANGE_WAIT:

			if (NPCHG_READY(this->numPeopleChange)) {
				if (nowPressed & WPAD_PLUS) {
					// activate easy pairing. FUN !!
					NPCHG_HIDE_FOR_EASYP(this->numPeopleChange) = 1;

					for (int i = 0; i < 4; i++) {
						void *obj = NPCHG_2DPLAYER(this->numPeopleChange, i);
						void *ccsb = NPCHG_CCSB(this->numPeopleChange, i);
						void *ccsc = NPCHG_CCSC(this->numPeopleChange, i);

						PLAYER2D_SHOW_EASY_PAIRING(obj) = 1;
						CCSB_ACTIVE(ccsb) = 1;
						CCSC_ACTIVE(ccsc) = 1;
					}

					EASYP_ACTIVE(this->easyPairing) = 1;
					this->state = STATE_EASY_PAIRING_WAIT;
				}
			} else {
				if (!NPCHG_ACTIVE(this->numPeopleChange)) {
					this->state = STATE_NORMAL;
				}
			}

			break;

		/**********************************************************************/
		// STATE_EASY_PAIRING_WAIT : Wait for the user to exit Easy Pairing.
		case STATE_EASY_PAIRING_WAIT:

			if (!EASYP_ACTIVE(this->easyPairing)) {
				NPCHG_HIDE_FOR_EASYP(this->numPeopleChange) = 0;

				for (int i = 0; i < 4; i++) {
					void *obj = NPCHG_2DPLAYER(this->numPeopleChange, i);
					void *ccsb = NPCHG_CCSB(this->numPeopleChange, i);
					void *ccsc = NPCHG_CCSC(this->numPeopleChange, i);

					PLAYER2D_SHOW_EASY_PAIRING(obj) = 0;
					CCSB_ACTIVE(ccsb) = 0;
					CCSC_ACTIVE(ccsc) = 0;
				}

				this->state = STATE_PLAYER_CHANGE_WAIT;
				WpadShit(10);
			}

			break;

		/**********************************************************************/
		// STATE_POWERUPS_WAIT : Wait for the user to exit the Powerups screen.
		case STATE_POWERUPS_WAIT:

			if (!STKI_SHOW(this->stockItem)) {
				this->state = STATE_NORMAL;
			}

			break;

		/**********************************************************************/
		// STATE_SAVE_OPEN : Waiting for the "Save?" YesNoWindow to open
		case STATE_SAVE_OPEN:

			if (!YESNO_OPENING(this->yesNoWindow)) {
				this->state = STATE_SAVE_SELECT;
			}

			break;

		/**********************************************************************/
		// STATE_SAVE_SELECT : Let the user choose an option on the
		// 	"Save?" YesNoWindow.
		case STATE_SAVE_SELECT:

			if (nowPressed & WPAD_LEFT) {
				// Select "OK!"
				YESNO_CURRENT(this->yesNoWindow) = 1;

			} else if (nowPressed & WPAD_RIGHT) {
				// Select "Cancel"
				YESNO_CURRENT(this->yesNoWindow) = 0;

			} else if (Wiimote_TestButtons(GetActiveWiimote(), WPAD_A | WPAD_TWO)) {
				// Pick the current option
				YESNO_CLOSE(this->yesNoWindow) = true;

				if (YESNO_CURRENT(this->yesNoWindow) != 1)
					YESNO_CANCELLED2(this->yesNoWindow) = true;
				this->state = STATE_SAVE_WINDOW_CLOSE;

			} else {
				// Cancel using B or 1
				if (CheckIfMenuShouldBeCancelledForSpecifiedWiimote(0)) {
					YESNO_CANCELLED(this->yesNoWindow) = true;
					YESNO_CURRENT(this->yesNoWindow) = 1;
					this->state = STATE_SAVE_WINDOW_CLOSE;
				}
			}

			break;

		/**********************************************************************/
		// STATE_SAVE_WINDOW_CLOSE : Process the user's chosen option on the
		// 	"Save?" YesNoWindow. Also, wait for the animation to be complete.
		case STATE_SAVE_WINDOW_CLOSE:

			if (!YESNO_VISIBLE(this->yesNoWindow)) {
				if (YESNO_CURRENT(this->yesNoWindow) == 1) {
					this->state = STATE_NORMAL;
				} else {
					this->state = STATE_SAVE_DO;
					SaveGame(0, false);
				}
			}

			break;

		/**********************************************************************/
		// STATE_SAVE_DO : Save the game.
		case STATE_SAVE_DO:

			if (!GetSaveFile()->CheckIfWriting()) {
				if (GetSaveHandler()->CurrentError == 0) {
					YESNO_TYPE(this->yesNoWindow) = 2;
					YESNO_VISIBLE(this->yesNoWindow) = true;
					this->state = STATE_SAVE_END_WINDOW;
				} else {
					this->state = STATE_SAVE_ERROR;
				}
			}

			break;

		/**********************************************************************/
		// STATE_SAVE_END_WINDOW : Handle the Save End window.
		case STATE_SAVE_END_WINDOW:

			if (!YESNO_OPENING(this->yesNoWindow)) {
				if (Wiimote_TestButtons(GetActiveWiimote(), WPAD_A | WPAD_TWO)) {
					YESNO_CLOSE(this->yesNoWindow) = true;
					this->state = STATE_SAVE_END_CLOSE_WAIT;
				}
			}

			break;

		/**********************************************************************/
		// STATE_SAVE_END_CLOSE_WAIT : Wait for the Save End window to close.
		case STATE_SAVE_END_CLOSE_WAIT:

			if (!YESNO_OPENING(this->yesNoWindow)) {
				this->state = STATE_NORMAL;
			}

			break;

		/**********************************************************************/
		// STATE_QUICKSAVE_OPEN : Waiting for the "Save?" YesNoWindow to open
		case STATE_QUICKSAVE_OPEN:

			if (!YESNO_OPENING(this->yesNoWindow)) {
				this->state = STATE_QUICKSAVE_SELECT;
			}

			break;

		/**********************************************************************/
		// STATE_QUICKSAVE_SELECT : Let the user choose an option on the
		// 	"Save?" YesNoWindow.
		case STATE_QUICKSAVE_SELECT:

			if (nowPressed & WPAD_LEFT) {
				// Select "OK!"
				YESNO_CURRENT(this->yesNoWindow) = 1;

			} else if (nowPressed & WPAD_RIGHT) {
				// Select "Cancel"
				YESNO_CURRENT(this->yesNoWindow) = 0;

			} else if (Wiimote_TestButtons(GetActiveWiimote(), WPAD_A | WPAD_TWO)) {
				// Pick the current option
				YESNO_CLOSE(this->yesNoWindow) = true;

				if (YESNO_CURRENT(this->yesNoWindow) != 1)
					YESNO_CANCELLED2(this->yesNoWindow) = true;
				this->state = STATE_QUICKSAVE_WINDOW_CLOSE;

			} else {
				// Cancel using B or 1
				if (CheckIfMenuShouldBeCancelledForSpecifiedWiimote(0)) {
					YESNO_CANCELLED(this->yesNoWindow) = true;
					YESNO_CURRENT(this->yesNoWindow) = 1;
					this->state = STATE_QUICKSAVE_WINDOW_CLOSE;
				}
			}

			break;

		/**********************************************************************/
		// STATE_QUICKSAVE_WINDOW_CLOSE : Process the user's chosen option on
		// 	the "Save?" YesNoWindow. Also, wait for the animation to be complete
		case STATE_QUICKSAVE_WINDOW_CLOSE:

			if (!YESNO_VISIBLE(this->yesNoWindow)) {
				if (YESNO_CURRENT(this->yesNoWindow) == 1) {
					this->state = STATE_NORMAL;
				} else {
					this->state = STATE_QUICKSAVE_DO;
					SaveGame(0, true);
				}
			}

			break;

		/**********************************************************************/
		// STATE_QUICKSAVE_DO : Save the game.
		case STATE_QUICKSAVE_DO:

			if (!GetSaveFile()->CheckIfWriting()) {
				if (GetSaveHandler()->CurrentError == 0) {
					YESNO_TYPE(this->yesNoWindow) = 16;
					YESNO_VISIBLE(this->yesNoWindow) = true;
					this->state = STATE_QUICKSAVE_END_WINDOW;
				} else {
					this->state = STATE_SAVE_ERROR;
				}
			}

			break;

		/**********************************************************************/
		// STATE_QUICKSAVE_END_WINDOW : Handle the Save End window.
		case STATE_QUICKSAVE_END_WINDOW:

			if (!YESNO_OPENING(this->yesNoWindow)) {
				if (Wiimote_TestButtons(GetActiveWiimote(), WPAD_A | WPAD_TWO)) {
					YESNO_CLOSE(this->yesNoWindow) = true;
					YESNO_REFUSED(this->yesNoWindow) = true;
					this->state = STATE_QUICKSAVE_END_CLOSE_WAIT;
				}
			}

			break;

		/**********************************************************************/
		// STATE_QUICKSAVE_END_CLOSE_WAIT : Wait for Save End window to close
		case STATE_QUICKSAVE_END_CLOSE_WAIT:

			if (!YESNO_OPENING(this->yesNoWindow)) {
				if (YESNO_CURRENT(this->yesNoWindow) == 1) {
					this->state = STATE_NORMAL;
				} else {
					this->state = STATE_LIMBO;
					StartTitleScreenStage(false, 0);
				}
			}

			break;

	}

	if (this->state >= STATE_NORMAL) {
		EmbeddedLayout_Process(this->layout);
		EmbeddedLayout_UpdateMatrix(this->layout);
	}

	return true;
}

int dScNewerWorldMap_c::onDraw() {
	EmbeddedLayout_AddToDrawList(this->layout);

	return true;
}



void NewerMapDrawFunc() {
	int keepCamera = GetCurrentCameraID();

	// All drawing uses scene 1, since that's the only one loaded by GAME_SETUP.
	// Todo: Newer-specific scenes?

	// Stage 1
	SetupLYTDrawing();
	DrawAllLayoutsBeforeX(129);
	GXDrawDone(); // is all GXDrawDone really needed..?

	// Stage 2
	Reset3DState();
	SetCurrentCameraID(0);
	LinkScene(0);
	SceneCalcWorld(0);
	SceneCameraStuff(0);
	ChangeAlphaUpdate(false);
	CalcMaterial();
	DrawOpa();
	DrawXlu();
	UnlinkScene(0);
	GXDrawDone();

	//Reset3DState();
	//T3D::DrawQueue();

	// Stage 3
	Reset3DState();
	SetupLYTDrawing();
	DrawAllLayoutsAfterXandBeforeY(128, 146);
	GXDrawDone();

	// Stage 4
	RemoveAllFromScnRoot();
	Reset3DState();
	SetCurrentCameraID(1);

	DoSpecialDrawing1();
	LinkScene(1);
	SceneCalcWorld(1);
	SceneCameraStuff(1);
	CalcMaterial();
	DrawOpa();
	DrawXlu();

	// Stage 5
	if (GAMEMGR_GET_AFC(GameMgr)) {
		for (int i = 0; i < 4; i++) {
			RenderEffects(0, 11+i);
		}

		for (int i = 0; i < 4; i++) {
			RenderEffects(0, 7+i);
		}
	}

	RenderEffects(0, 2); // need to investigate how this thing works

	DrawAllLayoutsAfterX(145);
	ClearLayoutDrawList(); // this is REALLY IMPORTANT!

	UnlinkScene(1);

	// End
	SetCurrentCameraID(0);
}

