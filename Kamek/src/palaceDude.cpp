#include <game.h>
#include <stage.h>
#include "msgbox.h"
#include "boss.h" // for stopbgmmusic and crap

const char *PalaceDudeFileList[] = {"OpeningScene", 0};

class dPalaceDude_c : public dStageActor_c {
	public:
		static dPalaceDude_c *build();

		bool hasBeenActivated;
		bool hasExitedStage;
		int onExecute();
};

/*****************************************************************************/
// Glue Code
dPalaceDude_c *dPalaceDude_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dPalaceDude_c));
	dPalaceDude_c *c = new(buffer) dPalaceDude_c;
	return c;
}


int dPalaceDude_c::onExecute() {
	if (dFlagMgr_c::instance->flags & spriteFlagMask) {
		if (!hasBeenActivated) {
//			OSReport("Activating Palace Dude\n");
			hasBeenActivated = true;

			int delay;
			if (!(settings & 0xF000000)) {
				delay = 270;
				StopBGMMusic();
				SaveBlock *save = GetSaveFile()->GetBlock(-1);
				GameMgrP->switchPalaceFlag |= (1 << (settings >> 28));
			} else {
				delay = 1020;
			}

			dMsgBoxManager_c::instance->showMessage(
				settings & 0xFFFFFF, // message ID
				false, // cannot cancel
				delay // delay
				);
		}
	}

	if (hasBeenActivated) {
		if (hasExitedStage)
			return true;
//		OSReport("Palace Dude is activated, %d\n", dMsgBoxManager_c::instance->visible);
		if (!dMsgBoxManager_c::instance->visible) {
//			OSReport("Exiting\n");
			u32 wmsettings = 0;
			if (settings & 0xF000000) {
				SaveGame(0, false);
				wmsettings = 0x80000000;
			}
			ExitStage(WORLD_MAP, wmsettings, BEAT_LEVEL, MARIO_WIPE);
			hasExitedStage = true;
		}
	}

	return true;

}


