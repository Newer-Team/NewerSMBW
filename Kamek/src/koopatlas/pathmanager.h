#ifndef __KOOPATLAS_PATH_MANAGER_H
#define __KOOPATLAS_PATH_MANAGER_H

//#define WM_UNLOCK_DEBUGGING
#ifdef WM_UNLOCK_DEBUGGING
#define UnlockCmdReport OSReport
#else
#define UnlockCmdReport(...)
#endif

#include "koopatlas/mapdata.h"
#include "levelinfo.h"

extern void *SoundRelatedClass;
extern "C" void *MapSoundPlayer(void *SoundClass, int soundID, int unk);
extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);

enum CompletionMessageType {
	CMP_MSG_NULL = 0,
	CMP_MSG_COINS = 1,
	CMP_MSG_EXITS = 2,
	CMP_MSG_WORLD = 3,
	CMP_MSG_GLOBAL_COINS_EXC_W9 = 4,
	CMP_MSG_GLOBAL_COINS = 5,
	CMP_MSG_GLOBAL_EXITS = 6,
	CMP_MSG_EVERYTHING = 7
};

class dWMPathManager_c {
	public:
		void setup();
		~dWMPathManager_c();
		void execute();

		bool canUseExit(dKPPath_s *path) {
			OSReport("Checking usability of path %p\n", path);
			if (path) OSReport("Availability: %d\n", path->isAvailable);
			return (path != 0) && (path->isAvailable);
		}

		void startMovementTo(dKPPath_s *path);
		void moveThroughPath(int pressedDir);
		void activatePoint();
		void unlockAllPaths(char type);

		void copyWorldDefToSave(const dKPWorldDef_s *world);

		dKPLayer_s *pathLayer;

		bool firstPathDone;
		bool isMoving;
		dKPNode_s *currentNode;

		HermiteKey keysX[2];
		HermiteKey keysY[3];
		float timer;
		bool isJumping;
		float moveSpeed;

		bool forcedRotation;

		int scaleAnimProgress;
		bool isScalingUp;

		nw4r::snd::SoundHandle penguinSlideSound;

		bool swimming;

		dKPPath_s *currentPath;
		bool reverseThroughPath; // direction we are going through the path

		bool mustComplainToMapCreator;

		int newlyAvailablePaths;
		int newlyAvailableNodes;

		bool panningCameraToPaths;
		bool panningCameraFromPaths;
		int unlockingAlpha; // -1 if not used
		int countdownToFadeIn;
		int waitAfterUnlock;
		int waitBeforePanBack;

		bool savingForEnding;
		bool waitingForEndingTransition;

	private:
		void unlockPaths();
		void finalisePathUnlocks();
		bool evaluateUnlockCondition(u8 *&in, SaveBlock *save, int stack);
		int cachedTotalStarCoinCount;
		int cachedUnspentStarCoinCount;

	public:
		bool shouldRequestSave;
		bool isEnteringLevel;
		bool completionMessagePending;
		int dmGladDuration;
		int completionAnimDelay;
		int completionMessageType;
		int completionMessageWorldNum;

		bool calledEnteredNode;
		int levelStartWait;
		int waitAtStart;
		bool mustPlayAfterDeathAnim;
		bool mustPlayAfterWinAnim;
		int waitAfterInitialPlayerAnim;
		bool initialLoading;

		bool checkedForMoveAfterEndLevel;
		bool afterFortressMode;

		bool doingThings();

		dLevelInfo_c::entry_s *enteredLevel;


	private:
		int camMinX, camMinY, camMaxX, camMaxY;
		int nodeStackLength;
		bool camBoundsValid;
		void visitNodeForCamCheck(dKPNode_s *node);
		void findCameraBoundsForUnlockedPaths();
		void addNodeToCameraBounds(dKPNode_s *node);
};

#endif

