/*
 * Newer Super Mario Bros. Wii
 * World Maps
 *
 * Wish me luck. That's all I'm saying. --Treeki
 * Started: 03/06/2010; 11:52pm
 */

// TODO: add Save Error state

//#define WM_DEBUGGING

#include <common.h>
#include <game.h>

#include "layoutlib.h"
#include "fileload.h"
#include "levelinfo_old.h"

#ifdef WM_DEBUGGING
#define MapReport OSReport
#else
inline void MapReport(const char *str, ...) { }
#endif

void NewerMapDrawFunc();


// WORLD MAP CLASS LAYOUT
class dScNewerWorldMap_c : public dScene_c {
public:
	Layout *layout;
	int currentPage;
	int *selections;
	int state;
	void *csMenu;
	void *selectCursor;
	void *numPeopleChange;
	void *yesNoWindow;
	void *continueObj;
	void *stockItem;
	void *stockItemShadow;
	void *easyPairing;

	void *levelInfo;
	FileHandle levelInfoFH;

	void StartLevel(LevelInfo_Entry *entry);

	void StartLevel();
	void GenText();
	void GenSBTitle();
	void SetTitle(const char *text);

	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	static dScNewerWorldMap_c *build();
	static dScNewerWorldMap_c *instance;
};

