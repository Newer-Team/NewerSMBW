#include <game.h>
#include <stage.h>

extern char CurrentLevel, CurrentWorld;
extern u8 MaybeFinishingLevel[2];
extern "C" void ExitStageReal(int scene, int sceneParams, int powerupStoreType, int wipe);


extern "C" void ExitStageWrapper(int scene, int sceneParams, int powerupStoreType, int wipe) {
	// TO RE-ENABLE CUTSCENES, UNCOMMENT THIS
	// if (scene == WORLD_MAP && powerupStoreType == BEAT_LEVEL) {
	// 	if (CurrentWorld == 6 && CurrentLevel == STAGE_DOOMSHIP) {
	// 		if (MaybeFinishingLevel[0] == 6 && MaybeFinishingLevel[1] == STAGE_DOOMSHIP) {
	// 			// We're done with 7-38
	// 			ExitStage(MOVIE, 0x10000000, powerupStoreType, wipe);
	// 			return;
	// 		}
	// 	}
	// }

	ExitStageReal(scene, sceneParams, powerupStoreType, wipe);
}


extern "C" void EnterOpeningLevel() {
	DontShowPreGame = true;
	RESTART_CRSIN_LevelStartStruct.purpose = 0;
	RESTART_CRSIN_LevelStartStruct.world1 = 1;
	RESTART_CRSIN_LevelStartStruct.world2 = 1;
	RESTART_CRSIN_LevelStartStruct.level1 = 40;
	RESTART_CRSIN_LevelStartStruct.level2 = 40;
	RESTART_CRSIN_LevelStartStruct.areaMaybe = 0;
	RESTART_CRSIN_LevelStartStruct.entrance = 0xFF;
	RESTART_CRSIN_LevelStartStruct.unk4 = 0; // load replay
	DoSceneChange(RESTART_CRSIN, 0, 0);
}

