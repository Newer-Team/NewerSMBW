#include "koopatlas/pathmanager.h"
#include "koopatlas/core.h"
#include "koopatlas/hud.h"
#include "koopatlas/player.h"
#include "koopatlas/map.h"
#include "koopatlas/camera.h"
#include <sfx.h>
#include <stage.h>

extern "C" void PlaySoundWithFunctionB4(void *spc, nw4r::snd::SoundHandle *handle, int id, int unk);

u8 MaybeFinishingLevel[2] = {0xFF,0xFF};
u8 LastLevelPlayed[2] = {0xFF,0xFF};
bool CanFinishCoins = false;
bool CanFinishExits = false;
bool CanFinishWorld = false;
bool CanFinishAlmostAllCoins = false;
bool CanFinishAllCoins = false;
bool CanFinishAllExits = false;
bool CanFinishEverything = false;
void ResetAllCompletionCandidates() {
	// This is called by File Select, btw
	MaybeFinishingLevel[0] = 0xFF;
	LastLevelPlayed[0] |= 0x80;
	CanFinishCoins = false;
	CanFinishExits = false;
	CanFinishWorld = false;
	CanFinishAlmostAllCoins = false;
	CanFinishAllCoins = false;
	CanFinishAllExits = false;
	CanFinishEverything = false;
}

int getPressedDir(int buttons) {
	if (buttons & WPAD_LEFT) return 0;
	else if (buttons & WPAD_RIGHT) return 1;
	else if (buttons & WPAD_UP) return 2;
	else if (buttons & WPAD_DOWN) return 3;
	return -1;
}

void dWMPathManager_c::setup() {
	dScKoopatlas_c *wm = dScKoopatlas_c::instance;

	isMoving = false;
	isJumping = false;
	scaleAnimProgress = -1;
	timer = 0.0;
	currentPath = 0;
	reverseThroughPath = false;

	shouldRequestSave = ((wm->settings & 0x80000) != 0);
	checkedForMoveAfterEndLevel = ((wm->settings & 0x40000) != 0);
	afterFortressMode = ((wm->settings & 0x20000) != 0);

	pathLayer = wm->mapData.pathLayer;

	SpammyReport("setting up PathManager\n");
	SaveBlock *save = GetSaveFile()->GetBlock(-1);

	mustComplainToMapCreator = false;

	SpammyReport("Unlocking paths\n");
	isEnteringLevel = false;
	levelStartWait = -1;
	unlockPaths();

	waitAfterInitialPlayerAnim = -1;
	if (LastPowerupStoreType == LOSE_LEVEL) {
		mustPlayAfterDeathAnim = true;
		daWMPlayer_c::instance->visible = false;
		LastPowerupStoreType = BEAT_LEVEL;
	} else if (LastPowerupStoreType == BEAT_LEVEL && LastLevelPlayed[0] < 0x80 && !wm->isAfter8Castle) {
		mustPlayAfterWinAnim = true;
		daWMPlayer_c::instance->visible = false;
	}

	SpammyReport("done\n");

	// Figure out what path node to start at
	if (wm->settings & 0x10000000) {
		// Start off from a "Change"
		u8 changeID = (wm->settings >> 20) & 0xFF;
		SpammyReport("entering at Change ID %d\n", changeID);
		SpammyReport("Path layer: %p\n", pathLayer);
		SpammyReport("Node count: %d\n", pathLayer->nodeCount);

		bool found = false;

		for (int i = 0; i < pathLayer->nodeCount; i++) {
			dKPNode_s *node = pathLayer->nodes[i];
			SpammyReport("Checking node: %p\n", node);

			if (node->type == dKPNode_s::CHANGE && node->thisID == changeID) {
				found = true;
				currentNode = node;

				SpammyReport("Found CHANGE node: %d %p\n", changeID, node);

				// figure out where we should move to
				dKPPath_s *exitTo = 0;

				for (int i = 0; i < 4; i++) {
					dKPPath_s *candidateExit = node->exits[i];
					//OSReport("Candidate exit: %p\n", candidateExit);
					if (!candidateExit)
						continue;

					// find out if this path is a candidate
					dKPNode_s *srcNode = node;
					dKPPath_s *path = candidateExit;

					while (true) {
						dKPNode_s *destNode = (path->start == srcNode) ? path->end : path->start;
						//OSReport("Path: %p nodes %p to %p\n", path, srcNode, destNode);
						int ct = destNode->getAvailableExitCount();
						//OSReport("Dest Node available exits: %d; type: %d\n", ct, destNode->type);
						if (destNode == node || ct > 2 || destNode->type == dKPNode_s::LEVEL || destNode->type == dKPNode_s::CHANGE) {
							exitTo = candidateExit;
							//OSReport("Accepting this node\n");
							break;
						}

						if (ct == 1)
							break;

						// where to next?
						path = destNode->getOppositeAvailableExitTo(path);
						srcNode = destNode;
					}

					if (exitTo)
						break;
				}

				if (!exitTo)
					exitTo = node->getAnyExit();
				startMovementTo(exitTo);
				break;
			}
		}

		if (!found) {
			currentNode = pathLayer->nodes[0];
			mustComplainToMapCreator = true;
		}

		waitAtStart = 1;
	} else {
		if (!countdownToFadeIn)
			waitAtStart = 50;
		else
			waitAtStart = 1;

		if (wm->isFirstPlay)
			waitAtStart = 280;

		SpammyReport("saved path node: %d\n", save->current_path_node);
		if (save->current_path_node >= pathLayer->nodeCount) {
			SpammyReport("out of bounds (%d), using node 0\n", pathLayer->nodeCount);
			currentNode = pathLayer->nodes[0];
		} else {
			int butts = Remocon_GetButtons(GetActiveRemocon());
			if ((butts & WPAD_MINUS) && (butts & WPAD_PLUS) && (butts & WPAD_ONE))
				currentNode = pathLayer->nodes[0];
			else
				currentNode = pathLayer->nodes[save->current_path_node];
			SpammyReport("OK %p\n", currentNode);
		}

		int findW = -1, findL = -1;
		bool storeIt = true;

		if (wm->isAfterKamekCutscene) {
			findW = 8;
			findL = 1;
		} else if (wm->isAfter8Castle) {
			findW = 8;
			findL = 5;
		} else if (wm->isEndingScene) {
			findW = 80;
			findL = 80;
			storeIt = false;
		}
		if (findW > -1) {
			// look for the 8-1 node
			for (int i = 0; i < pathLayer->nodeCount; i++) {
				dKPNode_s *node = pathLayer->nodes[i];
				if (node->type == dKPNode_s::LEVEL && node->levelNumber[0] == findW && node->levelNumber[1] == findL) {
					currentNode = node;
					if (storeIt)
						save->current_path_node = i;
					break;
				}
			}
		}
	}

	for (int i = 0; i < pathLayer->nodeCount; i++)
		if (pathLayer->nodes[i]->type == dKPNode_s::LEVEL)
			pathLayer->nodes[i]->setupNodeExtra();

	// did we just beat a level?
	if (MaybeFinishingLevel[0] != 0xFF) {
		if (save->CheckLevelCondition(MaybeFinishingLevel[0], MaybeFinishingLevel[1], COND_NORMAL)) {
			shouldRequestSave = true;
		}
	}

	// have we got any completions?
	if (LastLevelPlayed[0] < 0x80) {
		u32 conds = save->GetLevelCondition(LastLevelPlayed[0], LastLevelPlayed[1]);
		dLevelInfo_c::entry_s *whatEntry =
			dLevelInfo_c::s_info.searchBySlot(LastLevelPlayed[0], LastLevelPlayed[1]);

		// how many exits?
		int exits = 0, maxExits = 0;
		if (whatEntry->flags & 0x10) {
			maxExits++;
			if (conds & COND_NORMAL)
				exits++;
		}
		if (whatEntry->flags & 0x20) {
			maxExits++;
			if (conds & COND_SECRET)
				exits++;
		}

		completionMessageWorldNum = whatEntry->displayWorld;

		// now do all the message checks
		int flag = 0, totalFlag = 0;
		if (CanFinishCoins) {
			totalFlag |= 1;
			if ((conds & COND_COIN_ALL) == COND_COIN_ALL) {
				flag |= 1;
				completionMessageType = CMP_MSG_COINS;
			}
		}
		if (CanFinishExits) {
			totalFlag |= 2;
			if (exits == maxExits) {
				flag |= 2;
				completionMessageType = CMP_MSG_EXITS;
			}
		}
		if (CanFinishWorld && flag == totalFlag) {
			shouldRequestSave = true;
			completionMessageType = CMP_MSG_WORLD; 
		}

		if (CanFinishAlmostAllCoins) {
			if ((conds & COND_COIN_ALL) == COND_COIN_ALL)
				completionMessageType = CMP_MSG_GLOBAL_COINS_EXC_W9;
		}

		int gFlag = 0, gTotalFlag = 0;
		if (CanFinishAllCoins) {
			gTotalFlag |= 1;
			if ((conds & COND_COIN_ALL) == COND_COIN_ALL) {
				gFlag |= 1;
				completionMessageType = CMP_MSG_GLOBAL_COINS;
			}
		}
		if (CanFinishAllExits) {
			gTotalFlag |= 2;
			if (exits == maxExits) {
				gFlag |= 2;
				completionMessageType = CMP_MSG_GLOBAL_EXITS;
			}
		}
		if (CanFinishEverything && gFlag == gTotalFlag) {
			save->titleScreenWorld = 3;
			save->titleScreenLevel = 10;

			shouldRequestSave = true;
			completionMessageType = CMP_MSG_EVERYTHING;
		}
	}

	ResetAllCompletionCandidates();

	if (wm->isAfterKamekCutscene || wm->isAfter8Castle || wm->isEndingScene)
		copyWorldDefToSave(wm->mapData.findWorldDef(1));

	finalisePathUnlocks();
}

static u8 *PathAvailabilityData = 0;
static u8 *NodeAvailabilityData = 0;

void ClearOldPathAvailabilityData() {
	// This is called by File Select
	if (PathAvailabilityData) {
		delete[] PathAvailabilityData;
		PathAvailabilityData = 0;
	}
	if (NodeAvailabilityData) {
		delete[] NodeAvailabilityData;
		NodeAvailabilityData = 0;
	}
}

dWMPathManager_c::~dWMPathManager_c() {
	bool entering8_25 = (MaybeFinishingLevel[0] == 7) && (MaybeFinishingLevel[1] == 24);
	if (!isEnteringLevel && !entering8_25) {
		ClearOldPathAvailabilityData();
	}

	if (isEnteringLevel) {
		ResetAllCompletionCandidates();

		SaveBlock *save = GetSaveFile()->GetBlock(-1);
		if ((enteredLevel->displayLevel >= 21 && enteredLevel->displayLevel <= 27 && enteredLevel->displayLevel != 26)
				|| (enteredLevel->displayLevel >= 29 && enteredLevel->displayLevel <= 42)) {
			if (!save->CheckLevelCondition(enteredLevel->worldSlot, enteredLevel->levelSlot, COND_NORMAL)) {
				MaybeFinishingLevel[0] = enteredLevel->worldSlot;
				MaybeFinishingLevel[1] = enteredLevel->levelSlot;
			}
		}

		LastLevelPlayed[0] = enteredLevel->worldSlot;
		LastLevelPlayed[1] = enteredLevel->levelSlot;

		// Now, a fuckton of checks for the various possible things we can finish!
		dLevelInfo_c *li = &dLevelInfo_c::s_info;
		u32 theseConds = save->GetLevelCondition(enteredLevel->worldSlot, enteredLevel->levelSlot);

		int coinCount = 0, exitCount = 0;
		int globalCoinCount = 0, globalCoinCountExcW9 = 0, globalExitCount = 0;

		int totalCoinCount = 0, totalExitCount = 0;
		int totalGlobalCoinCount = 0, totalGlobalCoinCountExcW9 = 0, totalGlobalExitCount = 0;

		for (int sIdx = 0; sIdx < li->sectionCount(); sIdx++) {
			dLevelInfo_c::section_s *sect = li->getSectionByIndex(sIdx);

			for (int lIdx = 0; lIdx < sect->levelCount; lIdx++) {
				dLevelInfo_c::entry_s *entry = &sect->levels[lIdx];
				u32 entryConds = save->GetLevelCondition(entry->worldSlot, entry->levelSlot);

				// Only track actual levels
				if (!(entry->flags & 2))
					continue;

				// Counts for this world
				if (entry->displayWorld == enteredLevel->displayWorld) {
					totalCoinCount++;
					if ((entryConds & COND_COIN_ALL) == COND_COIN_ALL)
						coinCount++;

					// Normal exit
					if (entry->flags & 0x10) {
						totalExitCount++;
						if (entryConds & COND_NORMAL)
							exitCount++;
					}

					// Secret exit
					if (entry->flags & 0x20) {
						totalExitCount++;
						if (entryConds & COND_SECRET)
							exitCount++;
					}
				}

				// Counts for everywhere
				totalGlobalCoinCount++;
				if ((entryConds & COND_COIN_ALL) == COND_COIN_ALL)
					globalCoinCount++;

				if (entry->displayWorld != 9) {
					totalGlobalCoinCountExcW9++;
					if ((entryConds & COND_COIN_ALL) == COND_COIN_ALL)
						globalCoinCountExcW9++;
				}

				// Normal exit
				if (entry->flags & 0x10) {
					totalGlobalExitCount++;
					if (entryConds & COND_NORMAL)
						globalExitCount++;
				}

				// Secret exit
				if (entry->flags & 0x20) {
					totalGlobalExitCount++;
					if (entryConds & COND_SECRET)
						globalExitCount++;
				}
			}
		}

		// I'm using gotos. SUE ME.
		// Anyhow, don't consider non-levels for this.
		if (!(enteredLevel->flags & 2))
			goto cannotFinishAnything;

		// So.. are we candidates for any of these?
		int everythingFlag = 0, gEverythingFlag = 0;
		if (coinCount == totalCoinCount)
			everythingFlag |= 1;
		if (exitCount == totalExitCount)
			everythingFlag |= 2;
		if (globalCoinCount == totalGlobalCoinCount)
			gEverythingFlag |= 1;
		if (globalExitCount == totalGlobalExitCount)
			gEverythingFlag |= 2;

		// Check if we could obtain every star coin
		if ((theseConds & COND_COIN_ALL) != COND_COIN_ALL) {
			if ((coinCount + 1) == totalCoinCount) {
				CanFinishCoins = true;
				everythingFlag |= 1;
			}
			if ((globalCoinCount + 1) == totalGlobalCoinCount) {
				CanFinishAllCoins = true;
				gEverythingFlag |= 1;
			}
			if ((globalCoinCountExcW9 + 1) == totalGlobalCoinCountExcW9)
				CanFinishAlmostAllCoins = true;
		}

		// Check if we could obtain every exit
		int elExits = 0, elTotalExits = 0;
		if (enteredLevel->flags & 0x10) {
			elTotalExits++;
			if (theseConds & COND_NORMAL)
				elExits++;
		}
		if (enteredLevel->flags & 0x20) {
			elTotalExits++;
			if (theseConds & COND_SECRET)
				elExits++;
		}

		if ((elExits + 1) == elTotalExits) {
			if ((exitCount + 1) == totalExitCount) {
				CanFinishExits = true;
				everythingFlag |= 2;
			}
			if ((globalExitCount + 1) == totalGlobalExitCount) {
				CanFinishAllExits = true;
				gEverythingFlag |= 2;
			}
		}

		// And could we obtain EVERYTHING?
		if ((CanFinishCoins || CanFinishExits) && everythingFlag == 3)
			CanFinishWorld = true;
		if ((CanFinishAllCoins || CanFinishAllExits) && gEverythingFlag == 3)
			CanFinishEverything = true;
	}

cannotFinishAnything:
	if (penguinSlideSound.Exists())
		penguinSlideSound.Stop(5);
}

void dWMPathManager_c::unlockPaths() {
	u8 *oldPathAvData = PathAvailabilityData;
	PathAvailabilityData = new u8[pathLayer->pathCount];

	u8 *oldNodeAvData = NodeAvailabilityData;
	NodeAvailabilityData = new u8[pathLayer->nodeCount];

	SpammyReport("Unlocking paths\n");

	// unlock all needed paths
	for (int i = 0; i < pathLayer->pathCount; i++) {
		dKPPath_s *path = pathLayer->paths[i];

		PathAvailabilityData[i] = path->isAvailable;

		//SpammyReport("Path %d: %d\n", i, path->isAvailable);
		// if this path is not "always available", then nuke its alpha
		path->setLayerAlpha((path->isAvailable == dKPPath_s::ALWAYS_AVAILABLE) ? 255 : 0);
	}

	SaveBlock *save = GetSaveFile()->GetBlock(-1);

	cachedTotalStarCoinCount = getStarCoinCount();
	cachedUnspentStarCoinCount = cachedTotalStarCoinCount - save->spentStarCoins;

	u8 *in = (u8*)dScKoopatlas_c::instance->mapData.data->unlockData;
	SpammyReport("UNLOCKING PATHS: Unlock data @ %p\n", in);

	int cmdID = 0;

	while (*in != 0) {
		UnlockCmdReport("[%p] Cmd %d: Evaluating condition\n", in, cmdID);
		// begin processing a block
		bool value = evaluateUnlockCondition(in, save, 0);
		UnlockCmdReport("[%p] Cmd %d: Condition evaluated, result: %d\n", in, cmdID, value);
		//UnlockCmdReport("Unlock condition: %d\n", value);

		// get what it's supposed to affect
		// for now we'll assume that it affects one or more paths
		u8 affectedCount = *(in++);
		UnlockCmdReport("[%p] Cmd %d: Affects %d path(s)\n", in, cmdID, affectedCount);

		for (int i = 0; i < affectedCount; i++) {
			u8 one = *(in++);
			u8 two = *(in++);
			u16 pathID = (one << 8) | two;
			UnlockCmdReport("[%p] Cmd %d: Affected %d: PathID: %d\n", in, cmdID, i, pathID);

			dKPPath_s *path = pathLayer->paths[pathID];
			UnlockCmdReport("[%p] Cmd %d: Affected %d: Path: %p\n", in, cmdID, i, path);
			path->isAvailable = value ? dKPPath_s::AVAILABLE : dKPPath_s::NOT_AVAILABLE;
			UnlockCmdReport("[%p] Cmd %d: Affected %d: IsAvailable written\n", in, cmdID, i);
			PathAvailabilityData[pathID] = value ? dKPPath_s::AVAILABLE : dKPPath_s::NOT_AVAILABLE;
			UnlockCmdReport("[%p] Cmd %d: Affected %d: AvailabilityData written\n", in, cmdID, i);
			// NEWLY_AVAILABLE is set later, when that stuff is figured out

			path->setLayerAlpha(value ? 255 : 0);
			UnlockCmdReport("[%p] Cmd %d: Affected %d: Layer alpha applied\n", in, cmdID, i);
		}

		UnlockCmdReport("[%p] Cmd %d: %d affected path(s) processed\n", in, cmdID, affectedCount);

		cmdID++;
	}

	SpammyReport("UNLOCKING PATHS: All complete @ %p\n", in);

	for (int i = 0; i < pathLayer->nodeCount; i++) {
		dKPNode_s *node = pathLayer->nodes[i];
		NodeAvailabilityData[i] = node->isUnlocked();

		if (node->type == node->LEVEL && node->isUnlocked() && node->levelNumber[1] != 99) {
			save->completions[node->levelNumber[0]-1][node->levelNumber[1]-1] |= COND_UNLOCKED;
		}
	}

	// did anything become newly available?!
	newlyAvailablePaths = 0;
	newlyAvailableNodes = 0;

	dScKoopatlas_c *wm = dScKoopatlas_c::instance;
	bool forceFlag = (wm->isAfter8Castle || wm->isAfterKamekCutscene);

	if (!wm->isEndingScene && (oldPathAvData || forceFlag)) {
		for (int i = 0; i < pathLayer->pathCount; i++) {
			if ((PathAvailabilityData[i] > 0) && (forceFlag || oldPathAvData[i] == 0)) {
				if (forceFlag && PathAvailabilityData[i] == dKPPath_s::ALWAYS_AVAILABLE)
					continue;
	
				dKPPath_s *path = pathLayer->paths[i];
				path->isAvailable = dKPPath_s::NEWLY_AVAILABLE;
				newlyAvailablePaths++;

				// set this path's alpha to 0, we'll fade it in later
				path->setLayerAlpha(0);
			}
		}

		// check nodes too
		for (int i = 0; i < pathLayer->nodeCount; i++) {
			if ((NodeAvailabilityData[i] > 0) && (forceFlag || oldNodeAvData[i] == 0)) {
				dKPNode_s *node = pathLayer->nodes[i];
				node->isNew = true;
				newlyAvailableNodes++;
			}
		}
	}

	if (oldPathAvData) {
		delete[] oldPathAvData;
		delete[] oldNodeAvData;
	}

	if (wm->isEndingScene) {
		dKPNode_s *yoshiHouse = 0;
		for (int i = 0; i < pathLayer->nodeCount; i++) {
			dKPNode_s *node = pathLayer->nodes[i];

			if (node->type != dKPNode_s::LEVEL)
				continue;
			if (node->levelNumber[0] != 1)
				continue;
			if (node->levelNumber[1] != 41)
				continue;

			yoshiHouse = node;
			break;
		}

		if (yoshiHouse) {
			dKPNode_s *currentNode = yoshiHouse;
			dKPPath_s *nextPath = yoshiHouse->rightExit;

			while (true) {
				if (nextPath->isAvailable == dKPPath_s::AVAILABLE) {
					nextPath->isAvailable = dKPPath_s::NEWLY_AVAILABLE;
					newlyAvailablePaths++;
					nextPath->setLayerAlpha(0);
				}

				dKPNode_s *nextNode = nextPath->getOtherNodeTo(currentNode);
				if (!nextNode)
					break;

				if (nextNode->isUnlocked()) {
					nextNode->isNew = true;
					newlyAvailableNodes++;
				}

				currentNode = nextNode;
				nextPath = nextNode->getOppositeExitTo(nextPath);

				if (!nextPath)
					break;
			}
		}
	}

	// now set all node alphas
	for (int i = 0; i < pathLayer->nodeCount; i++) {
		dKPNode_s *node = pathLayer->nodes[i];

		node->setLayerAlpha((node->isUnlocked() & !node->isNew) ? 255 : 0);
	}
}

void dWMPathManager_c::finalisePathUnlocks() {
	// if anything was new, set it as such
	if (newlyAvailablePaths || newlyAvailableNodes) {
		countdownToFadeIn = 30;
		findCameraBoundsForUnlockedPaths();
	}
	unlockingAlpha = -1;
}

bool dWMPathManager_c::evaluateUnlockCondition(u8 *&in, SaveBlock *save, int stack) {
	UnlockCmdReport("[%p] CondStk:%d begin\n", in, stack);
	u8 controlByte = *(in++);

	u8 conditionType = (controlByte >> 6);
	UnlockCmdReport("[%p] CondStk:%d control byte: %d; condition type: %d\n", in, stack, controlByte, conditionType);

	if (conditionType == 0) {
		u8 subConditionType = controlByte & 0x3F;
		switch (subConditionType) {
			case 0: case 1: case 2: case 3:
				{
					u8 one = *(in++);
					u8 two = *(in++);

					int compareOne = (one & 0x80) ? cachedUnspentStarCoinCount : cachedTotalStarCoinCount;
					int compareTwo = ((one & 0x7F) << 8) | two;

					switch (subConditionType) {
						case 0:
							return compareOne == compareTwo;
						case 1:
							return compareOne != compareTwo;
						case 2:
							return compareOne < compareTwo;
						case 3:
							return compareOne > compareTwo;
					}
				}

			case 15:
				UnlockCmdReport("[%p] CondStk:%d end, returning CONSTANT 1\n", in, stack);
				return true;
			default:
				UnlockCmdReport("[%p] CondStk:%d unknown subCondition %d, returning 0\n", in, stack, subConditionType);
				return false;
		}
	}

	if (conditionType == 1) {
		// Simple level

		bool isSecret = (controlByte & 0x10);
		u8 worldNumber = controlByte & 0xF;
		u8 levelNumber = *(in++);
		UnlockCmdReport("[%p] CondStk:%d level, w:%d l:%d secret:%d\n", in, stack, worldNumber, levelNumber, isSecret);

		u32 conds = save->GetLevelCondition(worldNumber, levelNumber);
		UnlockCmdReport("[%p] CondStk:%d returning for level conditions: %d / %x\n", in, stack, conds, conds);

		if (isSecret)
			return (conds & COND_SECRET) != 0;
		else
			return (conds & COND_NORMAL) != 0;
	}

	// Type: 2 = AND, 3 = OR
	bool isAnd = (conditionType == 2);
	bool isOr = (conditionType == 3);

	bool value = isOr ? false : true;

	u8 termCount = (controlByte & 0x3F) + 1;
	UnlockCmdReport("[%p] CondStk:%d and:%d or:%d startValue:%d termCount:%d\n", in, stack, isAnd, isOr, value, termCount);

	for (int i = 0; i < termCount; i++) {
		bool what = evaluateUnlockCondition(in, save, stack+1);

		if (isOr)
			value |= what;
		else
			value &= what;
	}

	UnlockCmdReport("[%p] CondStk:%d end, returning %d\n", in, stack, value);
	return value;
}


bool dWMPathManager_c::doingThings() {
	if (isEnteringLevel || (waitAfterUnlock > 0) || (completionAnimDelay > 0) ||
			(waitAtStart > 0) || (waitAfterInitialPlayerAnim > 0) ||
			panningCameraToPaths || panningCameraFromPaths ||
			(waitBeforePanBack > 0) || !initialLoading ||
			(countdownToFadeIn > 0) || (unlockingAlpha != -1) ||
			dScKoopatlas_c::instance->isEndingScene)
		return true;

	if (isMoving)
		return true;

	return false;
}

void dWMPathManager_c::execute() {
	dScKoopatlas_c *wm = dScKoopatlas_c::instance;

	if (isEnteringLevel) {
		if (levelStartWait > 0) {
			levelStartWait--;
			if (levelStartWait == 0) {
				dScKoopatlas_c::instance->startLevel(enteredLevel);
			}
		}
		return;
	}

	if (waitAtStart > 0) {
		waitAtStart--;
		if (waitAtStart == 0) {
			if (mustPlayAfterDeathAnim) {
				daWMPlayer_c::instance->visible = true;
				daWMPlayer_c::instance->startAnimation(ending_wait, 1.0f, 0.0f, 0.0f);
				waitAfterInitialPlayerAnim = 60;

				nw4r::snd::SoundHandle something;
				PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_VOC_MA_CS_COURSE_MISS, 1);
			} else if (mustPlayAfterWinAnim) {
				daWMPlayer_c::instance->visible = true;
				if (dScKoopatlas_c::instance->isAfter8Castle) {
					waitAfterInitialPlayerAnim = 1;
				} else {
					daWMPlayer_c::instance->startAnimation(dm_surp_wait, 1.0f, 0.0f, 0.0f);
					waitAfterInitialPlayerAnim = 38;

					nw4r::snd::SoundHandle something;
					if (!wm->isEndingScene)
						PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_VOC_MA_CS_JUMP, 1);
				}
			}
		}
		return;
	}

	if (waitAfterInitialPlayerAnim > 0) {
		waitAfterInitialPlayerAnim--;
		if (waitAfterInitialPlayerAnim == 0)
			daWMPlayer_c::instance->startAnimation(wait_select, 1.0f, 0.0f, 0.0f);
		if (mustPlayAfterWinAnim && (waitAfterInitialPlayerAnim == 9) && !wm->isEndingScene) {
			nw4r::snd::SoundHandle something;
			PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_PLY_LAND_ROCK, 1);
		}
		return;
	}


	// handle path fading
	if (countdownToFadeIn > 0) {
		countdownToFadeIn--;
		if (countdownToFadeIn <= 0) {
			if (camBoundsValid) {
				dWorldCamera_c::instance->currentX = currentNode->x;
				dWorldCamera_c::instance->currentY = -currentNode->y;
				dWorldCamera_c::instance->panToBounds(camMinX, camMinY, camMaxX, camMaxY);

				panningCameraToPaths = true;
			} else
				unlockingAlpha = 0;

			nw4r::snd::SoundHandle something;
			PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_OBJ_GEN_LOAD, 1);
		} else {
			return;
		}
	}

	if (panningCameraToPaths) {
		if (dWorldCamera_c::instance->panning)
			return;
		panningCameraToPaths = false;
		unlockingAlpha = 0;
	}

	if (unlockingAlpha != -1) {
		unlockingAlpha += 3;

		for (int i = 0; i < pathLayer->pathCount; i++) {
			dKPPath_s *path = pathLayer->paths[i];

			if (path->isAvailable == dKPPath_s::NEWLY_AVAILABLE)
				path->setLayerAlpha(unlockingAlpha);
		}

		for (int i = 0; i < pathLayer->nodeCount; i++) {
			dKPNode_s *node = pathLayer->nodes[i];

			if (node->isNew)
				node->setLayerAlpha(unlockingAlpha);
		}

		if (unlockingAlpha == 255) {
			// we've reached the end
			unlockingAlpha = -1;
			nw4r::snd::SoundHandle something;
			PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_OBJ_GEN_NEW_COURSE, 1);

			waitAfterUnlock = 15;

			for (int i = 0; i < pathLayer->nodeCount; i++) {
				dKPNode_s *node = pathLayer->nodes[i];

				if (node->isNew && node->type == dKPNode_s::LEVEL) {
					Vec efPos = {float(node->x), float(-node->y), 3300.0f};
					S16Vec efRot = {0x2000,0,0};
					Vec efScale = {0.8f,0.8f,0.8f};
					SpawnEffect("Wm_cs_pointlight", 0, &efPos, &efRot, &efScale);
				}
			}
		}

		return;
	}

	if (waitAfterUnlock > 0) {
		waitAfterUnlock--;
		if (waitAfterUnlock == 0)
			waitBeforePanBack = 20;
		return;
	}

	if (waitBeforePanBack > 0) {
		waitBeforePanBack--;
		if (waitBeforePanBack == 0 && camBoundsValid) {
			dWorldCamera_c::instance->panToPosition(
					currentNode->x, -currentNode->y,
					STD_ZOOM);
			panningCameraFromPaths = true;
		}
		return;
	}

	if (panningCameraFromPaths) {
		if (dWorldCamera_c::instance->panning)
			return;
		panningCameraFromPaths = false;
		dWorldCamera_c::instance->followPlayer = true;
	}

	if (dmGladDuration > 0) {
		dmGladDuration--;
		if (dmGladDuration == 60) {
			nw4r::snd::SoundHandle something;
			PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_VOC_MA_CLEAR_MULTI, 1);
		} else if (dmGladDuration == 0) {
			daWMPlayer_c::instance->startAnimation(wait_select, 1.0f, 0.0f, 0.0f);
		}
		return;
	}

	if (completionAnimDelay > 0) {
		completionAnimDelay--;
		if (completionAnimDelay == 0)
			completionMessagePending = true;
		return;
	}

	// just in case
	if (completionMessagePending)
		return;

	if (completionMessageType > 0) {
		OSReport("We have a completion message type: %d\n", completionMessageType);

		int whichSound;
		if (completionMessageType == CMP_MSG_GLOBAL_COINS) {
			whichSound = STRM_BGM_STAR_COIN_CMPLT_ALL;
			completionAnimDelay = 240 - 154;
		} else if (completionMessageType == CMP_MSG_EVERYTHING) {
			whichSound = STRM_BGM_ALL_CMPLT_5STARS;
			completionAnimDelay = 216 - 154;
		} else {
			whichSound = STRM_BGM_STAR_COIN_CMPLT_WORLD;
			completionAnimDelay = 1;//138;
		}

		nw4r::snd::SoundHandle something;
		PlaySoundWithFunctionB4(SoundRelatedClass, &something, whichSound, 1);

		daWMPlayer_c::instance->startAnimation(coin_comp, 1.0f, 0.0f, 0.0f);
		dmGladDuration = 154;

		return;
	}

	if (dScKoopatlas_c::instance->isEndingScene) {
		// WE GO NO FURTHER
		if (!waitingForEndingTransition) {
			if (!savingForEnding) {
				savingForEnding = true;

				SaveBlock *save = GetSaveFile()->GetBlock(-1);
				save->titleScreenWorld = 3;
				save->titleScreenLevel = 9;

				SaveGame(0, false);
			} else {
				if (!GetSaveFile()->CheckIfWriting()) {
					StartTitleScreenStage(false, 0);
					waitingForEndingTransition = true;
				}
			}
		}
		return;
	}

	if (shouldRequestSave && !checkedForMoveAfterEndLevel) {
		checkedForMoveAfterEndLevel = true;

		static const int endLevels[11][3] = {
			{1, 38, 1}, // W1 right
			{2, 38, 2}, // W2 up
			{3, 38, 0}, // W3 left
			{4, 38, 1}, // W4 right
			{5, 38, 1}, // W5 right
			{6, 38, 2}, // W6 up
			{7, 38, 1}, // W7 right
			{10, 5, 2}, // GW up
			{10, 10, 0}, // MM left
			{10, 15, 3}, // CC down
			{10, 25, 1}, // SC right
		};

		int w = (LastLevelPlayed[0] & 0x7F) + 1;
		int l = LastLevelPlayed[1] + 1;

		for (int i = 0; i < 11; i++) {
			if (endLevels[i][0] == w && endLevels[i][1] == l) {
				afterFortressMode = true;
				startMovementTo(currentNode->exits[endLevels[i][2]]);
				return;
			}
		}
	}

	if (afterFortressMode) {
		if (isMoving)
			moveThroughPath(-1);
		else
			afterFortressMode = false;
		return;
	}

	if (shouldRequestSave) {
		dScKoopatlas_c::instance->showSaveWindow();
		shouldRequestSave = false;
		return;
	}

	if (!initialLoading) {
		dScKoopatlas_c::instance->startMusic();
		dWMHud_c::instance->loadInitially();
		initialLoading = true;
		return;
	}

	int nowPressed = Remocon_GetPressed(GetActiveRemocon());

	// Left, right, up, down
	int pressedDir = getPressedDir(nowPressed);

	if (isMoving) {
		moveThroughPath(pressedDir);
	} else {
		if (pressedDir >= 0) {
			// Use an exit if possible
			if (canUseExit(currentNode->exits[pressedDir])) {
				startMovementTo(currentNode->exits[pressedDir]);
			} else {
				// TODO: maybe remove this? got to see how it looks
				static s16 directions[] = {-0x4000,0x4000,-0x7FFF,0};
				daWMPlayer_c::instance->setTargetRotY(directions[pressedDir]);
			}
		} else if (nowPressed & WPAD_TWO) {
			activatePoint();
		}
	}
}


void dWMPathManager_c::startMovementTo(dKPPath_s *path) {
	SpammyReport("moving to path %p [%d,%d to %d,%d]\n", path, path->start->x, path->start->y, path->end->x, path->end->y);

	if (!path->isAvailable) { return; }
	if (currentNode && dWMHud_c::instance)
		dWMHud_c::instance->leftNode();

	calledEnteredNode = false;

	isMoving = true;
	reverseThroughPath = (path->end == currentNode);

	currentPath = path;

	// calculate direction of the path
	short deltaX = path->end->x - path->start->x;
	short deltaY = path->end->y - path->start->y;
	u16 direction = (u16)(atan2(deltaX, deltaY) / ((M_PI * 2) / 65536.0));

	if (reverseThroughPath) {
		direction += 0x8000;
	}

	daWMPlayer_c *player = daWMPlayer_c::instance;


	// Consider adding these as options
	// wall_walk_l = 60,
	// wall_walk_r = 61,

	// hang_walk_l = 65,
	// hang_walk_r = 66,

	static const struct {
		PlayerAnim anim;
		float animParam1, animParam2;
		s16 forceRotation;
		float forceSpeed;
		SFX repeatSound, initialSound;
		const char *repeatEffect, *initialEffect;
	} Animations[] = {
		// Walking
		{run,2.0f,10.0f, -1,-1.0f, SE_PLY_FOOTNOTE_DIRT,SE_NULL, 0,0},
		{run,2.0f,10.0f, -1,-1.0f, SE_PLY_FOOTNOTE_CS_SAND,SE_NULL, "Wm_mr_foot_sand",0},
		{run,2.0f,10.0f, -1,-1.0f, SE_PLY_FOOTNOTE_CS_SNOW,SE_NULL, "Wm_mr_foot_snow",0},
		{run,2.0f,10.0f, -1,-1.0f, SE_PLY_FOOTNOTE_CS_WATER,SE_NULL, "Wm_mr_foot_water",0},

		// Jumping
		{jump,1.0f,1.0f, -1,2.5f, SE_NULL,SE_PLY_JUMP, 0,0},
		{jump,1.0f,10.0f, -1,2.5f, SE_NULL,SE_PLY_JUMP, 0,0},
		{jump,1.0f,10.0f, -1,2.5f, SE_NULL,SE_PLY_JUMP, 0,0},

		// Jump water (actually cannon)
		{dm_notice,1.0f,10.0f, -1,-1.0f, SE_NULL,SE_VOC_MA_CANNON_SHOT, 0,0},

		// Ladder up, left, right
		{pea_plant,1.2f,10.0f, -0x7FFF,1.5f, SE_PLY_FOOTNOTE_CS_ROCK_CLIMB,SE_NULL, 0,0},
		{tree_climb,1.2f,10.0f, -0x4000,1.5f, SE_PLY_FOOTNOTE_CS_ROCK_CLIMB,SE_NULL, 0,0},
		{tree_climb,1.2f,10.0f, 0x4000,1.5f, SE_PLY_FOOTNOTE_CS_ROCK_CLIMB,SE_NULL, 0,0},
		// Fall (default?)
		{run,2.0f,10.0f, -1,-1.0f, SE_PLY_FOOTNOTE_DIRT,SE_NULL, 0,0},

		// Swim
		{swim_wait,1.2f,10.0f, -1,2.0f, SE_PLY_SWIM,SE_NULL, "Wm_mr_waterswim",0},
		// Run
		{b_dash2,3.0f,10.0f, -1,5.0f, SE_PLY_FOOTNOTE_DIRT,SE_NULL, 0,0},
		// Pipe
		{wait,2.0f,10.0f, 0,1.0f, SE_NULL,SE_PLY_DOKAN_IN_OUT, 0,0},
		// Door
		{wait,2.0f,10.0f, -0x7FFF,0.2f, SE_NULL,SE_OBJ_DOOR_OPEN, 0,0},

		// TJumped
		{Tjumped,2.0f,0.0f, -1,-1.0f, SE_NULL,SE_NULL, 0,0},

		// Enter cave, this is handled specially
		{run,1.0f,10.0f, -1,1.0f, SE_NULL,SE_NULL, 0,0},

		// Cannon 2
		{dm_noti_wait,1.0f,10.0f, -1,-1.0f, SE_NULL,SE_VOC_MA_CANNON_SHOT, 0,0},

		// Invisible, this is handled specially
		{wait,2.0f,10.0f, -1,1.0f, SE_NULL,SE_NULL, 0,0},
	};

	isJumping = (path->animation >= dKPPath_s::JUMP && path->animation <= dKPPath_s::JUMP_SAND);

	float playerScale = 1.6f;

	if (path->animation == dKPPath_s::ENTER_CAVE_UP) {
		scaleAnimProgress = 60;
		// what direction does this path go in?
		static s16 directions[] = {-0x4000,0x4000,-0x7FFF,0};
		isScalingUp = (deltaY < 0) ^ reverseThroughPath;

		if (!isScalingUp)
			playerScale = 0.0f;
	} else if (scaleAnimProgress >= 0) {
		// Keep the current scale
		playerScale = player->scale.x;
	}

	player->visible = (path->animation != dKPPath_s::INVISIBLE);
	player->scale.x = player->scale.y = player->scale.z = playerScale;

	int id = (path->animation >= dKPPath_s::MAX_ANIM) ? 0 : (int)path->animation;
	int whichAnim = Animations[id].anim;
	float updateRate = Animations[id].animParam1;
	if (whichAnim == swim_wait) {
		if (player->modelHandler->mdlClass->powerup_id == 3) {
			whichAnim = b_dash;
			updateRate = 2.5f;
		} else if (player->modelHandler->mdlClass->powerup_id == 5)
			whichAnim = P_slip;
	}

	if (Animations[id].forceRotation != -1) {
		forcedRotation = true;
		player->setTargetRotY(Animations[id].forceRotation);
	} else if (id == dKPPath_s::JUMP_WATER || id == dKPPath_s::RESERVED_18) {
		// keep the current rotation
		forcedRotation = true;
		PlayerAnimStates[dm_notice].playsOnce = 0;

		nw4r::snd::SoundHandle something;
		PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_OBJ_WARP_CANNON_SHOT, 1);

		dWMMap_c::instance->spinLaunchStar();
	} else {
		forcedRotation = false;
		player->setTargetRotY(direction);
	}

	player->startAnimation(whichAnim, updateRate, Animations[id].animParam2, 0.0f);

	moveSpeed = (Animations[id].forceSpeed >= 0.0f) ? Animations[id].forceSpeed : 3.0f;
	moveSpeed = path->speed * moveSpeed * 1.3f;
	if (path->animation == dKPPath_s::SWIM) {
		if (player->modelHandler->mdlClass->powerup_id == 3)
			moveSpeed *= 1.1f;
		else if (player->modelHandler->mdlClass->powerup_id == 5)
			moveSpeed *= 2.0f;
	}

	if (Animations[id].repeatEffect) {
		player->hasEffect = true;
		player->effectName = Animations[id].repeatEffect;
	} else {
		player->hasEffect = false;
	}

	if (Animations[id].repeatSound != SE_NULL) {
		player->hasSound = true;
		player->soundName = Animations[id].repeatSound;
	} else {
		player->hasSound = false;
	}

	if (Animations[id].initialEffect) {
		SpawnEffect(Animations[id].initialEffect, 0, &player->pos, 0, &player->scale);
	}

	if (path->animation == dKPPath_s::SWIM) {
		if (player->modelHandler->mdlClass->powerup_id == 5) {
			if (!swimming) {
				nw4r::snd::SoundHandle something;
				if (firstPathDone)
					PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_VOC_MA_PNGN_SLIDE, 1);
				PlaySoundWithFunctionB4(SoundRelatedClass, &penguinSlideSound, SE_EMY_PENGUIN_SLIDE, 1);
			}
			player->hasSound = false;
		} else if (player->modelHandler->mdlClass->powerup_id == 3) {
			player->hasSound = true;
			player->soundName = SE_PLY_FOOTNOTE_WATER;
		}
		swimming = true;
	} else {
		if (swimming && penguinSlideSound.Exists()) {
			penguinSlideSound.Stop(10);
			penguinSlideSound.DetachSound();
		}
		swimming = false;

		if (Animations[id].initialSound != SE_NULL) {
			nw4r::snd::SoundHandle something;
			PlaySoundWithFunctionB4(SoundRelatedClass, &something, Animations[id].initialSound, 1);

			if (Animations[id].initialSound == SE_PLY_JUMP) {
				nw4r::snd::SoundHandle something2;
				PlaySoundWithFunctionB4(SoundRelatedClass, &something2, SE_VOC_MA_CS_JUMP, 1);
				something2.SetPitch(player->modelHandler->mdlClass->powerup_id == 3 ? 1.5f : 1.0f);
			}
		}
	}
}

void dWMPathManager_c::moveThroughPath(int pressedDir) {
	dKPNode_s *from, *to;

	from = reverseThroughPath ? currentPath->end : currentPath->start;
	to = reverseThroughPath ? currentPath->start : currentPath->end;

	daWMPlayer_c *player = daWMPlayer_c::instance;

	if (pressedDir >= 0 && !calledEnteredNode) {
		int whatDirDegrees = ((int)(atan2(to->x-from->x, to->y-from->y) / ((M_PI * 2) / 360.0)) + 360) % 360;
		// dirs are: left, right, up, down
		int whatDir;
		if (whatDirDegrees >= 225 && whatDirDegrees <= 315)
			whatDir = 1; // moving Left, so reversing requires Right
		else if (whatDirDegrees >= 45 && whatDirDegrees <= 135)
			whatDir = 0; // moving Right, so reversing requires Left
		else if (whatDirDegrees > 135 && whatDirDegrees < 225)
			whatDir = 3; // moving Up, so reversing requires Down
		else if (whatDirDegrees > 315 || whatDirDegrees < 45)
			whatDir = 2; // moving Down, so reversing requires Up
		OSReport("Delta: %d, %d; Degrees: %d (Atan result is %f); Calced dir is %d; Pressed dir is %d\n", to->x-from->x, to->y-from->y, whatDirDegrees, atan2(to->x-from->x,to->y-from->y), whatDir, pressedDir);

		if (whatDir == pressedDir) {
			// are we using a forbidden animation?
			static const dKPPath_s::Animation forbidden[] = {
				dKPPath_s::JUMP, dKPPath_s::JUMP_SAND,
				dKPPath_s::JUMP_SNOW, dKPPath_s::JUMP_WATER,
				dKPPath_s::PIPE, dKPPath_s::DOOR,
				dKPPath_s::ENTER_CAVE_UP, dKPPath_s::INVISIBLE,
				dKPPath_s::RESERVED_18, dKPPath_s::MAX_ANIM
			};
			bool allowed = true;
			for (int i = 0;;i++) {
				if (forbidden[i] == dKPPath_s::MAX_ANIM)
					break;
				if (forbidden[i] == currentPath->animation)
					allowed = false;
			}

			if (allowed) {
				reverseThroughPath = !reverseThroughPath;
				if (!forcedRotation)
					player->setTargetRotY(player->targetRotY + 0x8000);
				// start over with the reversed path!
				moveThroughPath(-1);
				return;
			}
		}
	}


	if (scaleAnimProgress >= 0) {
		float soFar = scaleAnimProgress * (1.6f / 60.0f);
		float sc = isScalingUp ? soFar : (1.6f - soFar);
		player->scale.x = player->scale.y = player->scale.z = sc;

		scaleAnimProgress--;
	}


	Vec move = (Vec){float(to->x - from->x), float(to->y - from->y), 0};
	VECNormalize(&move, &move);
	VECScale(&move, &move, moveSpeed);

	if (isJumping) {
		bool isFalling;
		if (from->y == to->y) {
			float xDistance = to->x - from->x;
			if (xDistance < 0)
				xDistance = -xDistance;
			float currentPoint = max(to->x, from->x) - player->pos.x;
			player->jumpOffset = (xDistance / 3.0f) * sin((currentPoint / xDistance) * 3.1415f);

			if (to->x > from->x) // Moving right
				isFalling = (player->pos.x > (to->x - (move.x * 10.0f)));
			else // Moving left
				isFalling = (player->pos.x < (to->x - (move.x * 10.0f)));

		} else {
			float ys = (float)from->y;
			float ye = (float)to->y;
			float midpoint = (from->y + to->y) / 2;

			float top, len;
			if (ys > ye) { len = ys - ye; top = ys - midpoint + 10.0; }
			else		 { len = ye - ys; top = ye - midpoint + 10.0; }

			if (len == 0.0) { len = 2.0; }

			float a;
			if (timer > 0.0) { a = -timer; }
			else			 { a =  timer; }


			player->jumpOffset = -sin(a * 3.14 / len) * top;
			timer -= move.y;

			if (ye > ys) // Moving down
				isFalling = (-player->pos.y) > (ye - (move.y * 10.0f));
			else // Moving up
				isFalling = (-player->pos.y) < (ye - (move.y * 10.0f));
		}
		if (isFalling)
			player->startAnimation(jumped, 1.0f, 10.0f, 0.0f);
	}

	player->pos.x += move.x;
	player->pos.y -= move.y;

	// what distance is left?
	if (to->type == dKPNode_s::LEVEL && !calledEnteredNode) {
		Vec toEndVec = {to->x - player->pos.x, to->y + player->pos.y, 0.0f};
		float distToEnd = VECMag(&toEndVec);
		//OSReport("Distance: %f; To:%d,%d; Player:%f,%f; Diff:%f,%f\n", distToEnd, to->x, to->y, player->pos.x, player->pos.y, toEndVec.x, toEndVec.y);

		if (distToEnd < 64.0f && dWMHud_c::instance) {
			calledEnteredNode = true;
			dWMHud_c::instance->enteredNode(to);
		}
	}

	// Check if we've reached the end yet
	if (
			(((move.x > 0) ? (player->pos.x >= to->x) : (player->pos.x <= to->x)) &&
			 ((move.y > 0) ? (-player->pos.y >= to->y) : (-player->pos.y <= to->y)))
			||
			(from->x == to->x && from->y == to->y)
	   ) {

		currentNode = to;
		player->pos.x = to->x;
		player->pos.y = -to->y;

		isJumping = false;
		timer = 0.0;

		SpammyReport("reached path end (%p) with type %d\n", to, to->type);

		bool reallyStop = false;

		if (to->type == dKPNode_s::LEVEL) {
			// Always stop on levels
			reallyStop = true;
		} else if (to->type == dKPNode_s::CHANGE || to->type == dKPNode_s::WORLD_CHANGE || to->type == dKPNode_s::PASS_THROUGH) {
			// If there's only one exit here, then stop even though
			// it's a passthrough node
			reallyStop = (to->getAvailableExitCount() == 1);
		} else {
			// Quick check: do we *actually* need to stop on this node?
			// If it's a junction with more than two exits, but only two are open,
			// take the opposite open one
			if (!dScKoopatlas_c::instance->warpZoneHacks && to->getExitCount() > 2 && to->getAvailableExitCount() == 2)
				reallyStop = false;
			else
				reallyStop = true;
		}

		if (to->type == dKPNode_s::WORLD_CHANGE) {
			// Set the current world info
			SaveBlock *save = GetSaveFile()->GetBlock(-1);

			OSReport("Activating world change %d\n", to->worldID);
			const dKPWorldDef_s *world = dScKoopatlas_c::instance->mapData.findWorldDef(to->worldID);
			if (world) {
				bool visiblyChange = true;
				if (strncmp(save->newerWorldName, world->name, 32) == 0) {
					OSReport("Already here, but setting BGM track\n");
					visiblyChange = false;
				}

				OSReport("Found!\n");
				copyWorldDefToSave(world);

				bool wzHack = false;
				if (dScKoopatlas_c::instance->warpZoneHacks) {
					save->hudHintH += 1000;

					if (world->worldID > 0) {
						dLevelInfo_c *linfo = &dLevelInfo_c::s_info;
						dLevelInfo_c::entry_s *lastLevel;
						if (world->worldID == 0)
							lastLevel = linfo->searchByDisplayNum(1, 27);
						else if (world->worldID != 7)
							lastLevel = linfo->searchByDisplayNum(world->worldID-1, lastLevelIDs[world->worldID-1]);
						else
							lastLevel = linfo->searchByDisplayNum(7, 3);

						if (lastLevel) {
							wzHack = !(save->GetLevelCondition(lastLevel->worldSlot,lastLevel->levelSlot) & COND_NORMAL);
						}

						// another stupid thing
						if (world->worldID == 7 && wzHack)
							if (save->GetLevelCondition(9,24) & COND_NORMAL)
								wzHack = false;
					}
				}

				if (wzHack) {
					save->hudHintH = 2000;
					dWMHud_c::instance->hideFooter();
				} else {
					if (visiblyChange && dWMHud_c::instance)
						dWMHud_c::instance->showFooter();
				}

				dKPMusic::play(world->trackID);

			} else if (to->worldID == 0) {
				OSReport("No world\n");
				save->currentMapMusic = 0;
				dKPMusic::play(0);
				save->newerWorldName[0] = 0;
				if (dWMHud_c::instance)
					dWMHud_c::instance->hideFooter();
			} else {
				OSReport("Not found!\n");
			}
		}

		if (to->type == dKPNode_s::CHANGE) {
			// Go to another map

			// should we continue moving?
			if (to->getAvailableExitCount() == 1) {
				OSReport("Stopping");
				isMoving = false;
			} else {
				OSReport("Continuing");
				startMovementTo(to->getOppositeAvailableExitTo(currentPath));
			}

			SaveBlock *save = GetSaveFile()->GetBlock(-1);
			SpammyReport("node: %x, %s", to->destMap, to->destMap);
			save->current_world = dScKoopatlas_c::instance->getIndexForMapName(to->destMap);

			SpammyReport("Change to map ID %d (%s), entrance ID %d\n", save->current_world, to->destMap, to->foreignID);

			dScKoopatlas_c::instance->keepMusicPlaying = true;
			ActivateWipe(to->transition);
			u32 saveFlag = (shouldRequestSave ? 0x80000 : 0);
			saveFlag |= (checkedForMoveAfterEndLevel ? 0x40000 : 0);
			saveFlag |= (afterFortressMode ? 0x20000 : 0);
			DoSceneChange(WORLD_MAP, 0x10000000 | (to->foreignID << 20) | saveFlag, 0);

		} else if (reallyStop) {
			// Stop here
			bool movingAgain = false;
			player->hasEffect = false;
			player->hasSound = false;

			SpammyReport("stopping here\n");

			isMoving = false;
			swimming = false;

			SaveBlock *save = GetSaveFile()->GetBlock(-1);
			save->current_path_node = pathLayer->findNodeID(to);
			if (!calledEnteredNode && dWMHud_c::instance)
				dWMHud_c::instance->enteredNode();

			// Should we continue here? (Requested by Jason)
			int held = Remocon_GetButtons(GetActiveRemocon());
			int pressedDir = getPressedDir(held);
			if (pressedDir >= 0) {
				// Use an exit if possible
				if (canUseExit(currentNode->exits[pressedDir])) {
					startMovementTo(currentNode->exits[pressedDir]);
					movingAgain = true;
				} else {
					// TODO: maybe remove this? got to see how it looks
					static s16 directions[] = {-0x4000,0x4000,-0x7FFF,0};
					daWMPlayer_c::instance->setTargetRotY(directions[pressedDir]);
				}
			}

			if (!movingAgain)
				player->startAnimation(wait_select, 1.2, 10.0, 0.0);

		} else {
			startMovementTo(to->getOppositeAvailableExitTo(currentPath));
			SpammyReport("passthrough node, continuing to next path\n");
		}
	}

	firstPathDone = true;
}

void dWMPathManager_c::copyWorldDefToSave(const dKPWorldDef_s *world) {
	SaveBlock *save = GetSaveFile()->GetBlock(-1);

	strncpy(save->newerWorldName, world->name, 32);
	save->newerWorldName[31] = 0;
	save->newerWorldID = world->worldID;
	save->currentMapMusic = world->trackID;

	for (int i = 0; i < 2; i++) {
		save->fsTextColours[i] = world->fsTextColours[i];
		save->fsHintColours[i] = world->fsHintColours[i];
		save->hudTextColours[i] = world->hudTextColours[i];
	}

	save->hudHintH = world->hudHintH;
	save->hudHintS = world->hudHintS;
	save->hudHintL = world->hudHintL;

	if (save->titleScreenWorld == 3 && save->titleScreenLevel == 10)
		return;
	save->titleScreenWorld = world->titleScreenWorld;
	save->titleScreenLevel = world->titleScreenLevel;
}

void dWMPathManager_c::activatePoint() {
	if (levelStartWait >= 0)
		return;

	if (currentNode->type == dKPNode_s::LEVEL) {
		int w = currentNode->levelNumber[0] - 1;
		int l = currentNode->levelNumber[1] - 1;

		if (l == 98) {
			dWMShop_c::instance->show(w);
			dWMHud_c::instance->hideAll();
			dScKoopatlas_c::instance->state.setState(&dScKoopatlas_c::instance->StateID_ShopWait);
			return;
		}

		if ((l >= 29) && (l <= 36)) {
			SaveBlock *save = GetSaveFile()->GetBlock(-1);
			u32 conds = save->GetLevelCondition(w, l);

			SpammyReport("Toad House Flags: %x", conds);
			if (conds & 0x30) { 
				nw4r::snd::SoundHandle something;
				PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_SYS_INVALID, 1);
				return;
			}
		}

		nw4r::snd::SoundHandle something;
		PlaySoundWithFunctionB4(SoundRelatedClass, &something, SE_SYS_GAME_START, 1);

		nw4r::snd::SoundHandle something2;
		PlaySoundWithFunctionB4(SoundRelatedClass, &something2, (Player_Powerup[0] == 3) ? SE_VOC_MA_PLAYER_DECIDE_MAME: SE_VOC_MA_CS_COURSE_IN, 1);

		daWMPlayer_c::instance->startAnimation(course_in, 1.2, 10.0, 0.0);
		daWMPlayer_c::instance->setTargetRotY(0);

		isEnteringLevel = true;
		levelStartWait = 40;
		enteredLevel = dLevelInfo_c::s_info.searchBySlot(w, l);

		dKPMusic::stop();
	}
}


void dWMPathManager_c::unlockAllPaths(char type) {

	if (type == 0 || type == 1) {
		SaveBlock *save = GetSaveFile()->GetBlock(-1);
		for (int j = 0; j < 10; j++) {
			for (int h = 0; h < 0x2A; h++) {
				if (j != 1 || h != 9)
					save->completions[j][h] = (type == 1) ? 0x17 : 0x37;
			}
		}
		unlockPaths();
	}

	// Unlocks current path, regular and secret
	if (type == 2) {
		if (currentNode->type == dKPNode_s::LEVEL) {
			int w = currentNode->levelNumber[0] - 1;
			int l = currentNode->levelNumber[1] - 1;

			SaveBlock *save = GetSaveFile()->GetBlock(-1);
			save->completions[w][l] = 0x37;
			unlockPaths();
		}
	}

	// Can't change node models - the price we pay for not using anims
	// for (int i = 0; i < pathLayer->nodeCount; i++) {
	// 	dKPNode_s *node = pathLayer->nodes[i];
	// 	node->setupNodeExtra();
	// }

}




void dWMPathManager_c::findCameraBoundsForUnlockedPaths() {
	dKPMapData_c *data = &dScKoopatlas_c::instance->mapData;

	camMinX = 10000;
	camMaxX = 0;
	camMinY = 10000;
	camMaxY = 0;

	nodeStackLength = 0;
	for (int i = 0; i < data->pathLayer->nodeCount; i++)
		data->pathLayer->nodes[i]->reserved1 = false;

	visitNodeForCamCheck(currentNode);
	OSReport("Worked out camera bounds: %d,%d to %d,%d with validity %d\n", camMinX, camMinY, camMaxX, camMaxY, camBoundsValid);
}

void dWMPathManager_c::visitNodeForCamCheck(dKPNode_s *node) {
	// Yay.
	nodeStackLength++;
	node->reserved1 = true;

	for (int i = 0; i < 4; i++) {
		dKPPath_s *path = node->exits[i];
		if (!path)
			continue;

		OSReport("Checking path %p, whose status is %d\n", path, path->isAvailable);
		if (path->isAvailable == dKPPath_s::NEWLY_AVAILABLE) {
			addNodeToCameraBounds(path->start);
			addNodeToCameraBounds(path->end);
		}

		// Should we follow the other node?
		dKPNode_s *otherNode = path->getOtherNodeTo(node);

		if (otherNode->reserved1)
			continue;
		if (otherNode->type == otherNode->LEVEL) {
			OSReport("Not travelling to %p because it's level %d-%d\n", otherNode, otherNode->levelNumber[0], otherNode->levelNumber[1]);
			continue;
		}
		if (otherNode->type == otherNode->CHANGE) {
			OSReport("Not travelling to %p because it's a change\n", otherNode);
			continue;
		}
		if (otherNode->type == otherNode->WORLD_CHANGE) {
			OSReport("Not travelling to %p because it's a world change\n", otherNode);
			continue;
		}

		visitNodeForCamCheck(otherNode);
	}

	nodeStackLength--;
}

void dWMPathManager_c::addNodeToCameraBounds(dKPNode_s *node) {
	camBoundsValid = true;
	OSReport("Adding node to camera bounds: %p at %d,%d\n", node, node->x, node->y);

	if (node->x < camMinX)
		camMinX = node->x;
	if (node->x > camMaxX)
		camMaxX = node->x;
	if (node->y < camMinY)
		camMinY = node->y;
	if (node->y > camMaxY)
		camMaxY = node->y;
}

