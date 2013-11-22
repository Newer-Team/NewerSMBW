/* KoopAtlas Map Engine
 * Created by Treeki
 */

#ifndef __KOOPATLAS_H
#define __KOOPATLAS_H

#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>

#include "levelinfo.h"
#include "koopatlas/mapdata.h"
#include "koopatlas/shop.h"
#include "koopatlas/starcoin.h"
#include "koopatlas/hud.h"
#include "koopatlas/pathmanager.h"
#include "koopatlas/mapmusic.h"

#define WM_DEBUGGING
//#define WM_SPAMMY_DEBUGGING

#ifdef WM_DEBUGGING
#define MapReport OSReport
#else
#define MapReport(...)
#endif

#ifdef WM_SPAMMY_DEBUGGING
#define SpammyReport OSReport
#else
#define SpammyReport(...)
#endif

void NewerMapDrawFunc();

#define WM_HUD WM_DANCE_PAKKUN
#define WM_SHOP WM_TOGEZO
#define WM_STARCOIN WM_GHOST

class daWMPlayer_c;
class dWMMap_c;
class dWMHud_c;
class dWMShop_c;
class dWMStarCoin_c;
class dWorldCamera_c;

class dScKoopatlas_c : public dScene_c {
	public:
		dScKoopatlas_c();

		FunctionChain initChain;

		dStateWrapper_c<dScKoopatlas_c> state;

		USING_STATES(dScKoopatlas_c);
		DECLARE_STATE(Limbo);
		DECLARE_STATE(ContinueWait);
		DECLARE_STATE(Normal);
		DECLARE_STATE(CompletionMsg);
		DECLARE_STATE(CompletionMsgHideWait);
		DECLARE_STATE(CSMenu);
		DECLARE_STATE(TitleConfirmOpenWait);
		DECLARE_STATE(TitleConfirmSelect);
		DECLARE_STATE(TitleConfirmHitWait);
		DECLARE_STATE(PlayerChangeWait);
		DECLARE_STATE(EasyPairingWait);
		DECLARE_STATE(PowerupsWait);
		DECLARE_STATE(ShopWait);
		DECLARE_STATE(CoinsWait);
		DECLARE_STATE(SaveOpen);
		DECLARE_STATE(SaveSelect);
		DECLARE_STATE(SaveWindowClose);
		DECLARE_STATE(SaveDo);
		DECLARE_STATE(SaveEndWindow);
		DECLARE_STATE(SaveEndCloseWait);
#ifdef QUICK_SAVE
		DECLARE_STATE(QuickSaveOpen);
		DECLARE_STATE(QuickSaveSelect);
		DECLARE_STATE(QuickSaveWindowClose);
		DECLARE_STATE(QuickSaveDo);
		DECLARE_STATE(QuickSaveEndWindow);
		DECLARE_STATE(QuickSaveEndCloseWait);
#endif
		DECLARE_STATE(SaveError);

		void *csMenu;
		void *selectCursor;
		void *numPeopleChange;
		dYesNoWindow_c *yesNoWindow;
		void *continueObj;
		dStockItem_c *stockItem;
		dStockItemShadow_c *stockItemShadow;
		void *easyPairing;


		int onCreate();
		int onDelete();
		int onExecute();

		static dScKoopatlas_c *build();
		static dScKoopatlas_c *instance;


		daWMPlayer_c *player;
		dWMHud_c *hud;
		dWMMap_c *map;
		dWMShop_c *shop;
		dWMStarCoin_c *coins;

		int currentMapID;
		const char *mapPath;
		dKPMapData_c mapData;
		dWMPathManager_c pathManager;

		dDvdLoader_c mapListLoader;

		bool isFirstPlay;
		bool isAfterKamekCutscene;
		bool isAfter8Castle;
		bool isEndingScene;
		void startMusic();

		bool warpZoneHacks;

		int mustFixYesNoText;

		u32 iterateMapList(u32(*callback)(u32,const char *,int,int), u32 userData, int *ptrIndex = 0);
		const char *getMapNameForIndex(int index);
		int getIndexForMapName(const char *name);

		void startLevel(dLevelInfo_c::entry_s *level);

		bool canDoStuff();
		bool mapIsRunning();

		void showSaveWindow();

		bool keepMusicPlaying;
};

extern void *_8042A788;
void somethingAboutSound(void *beef);

#endif

