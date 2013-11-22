#ifndef __KAMEK_STAGE_H
#define __KAMEK_STAGE_H

#include <common.h>
#include <course.h>

#define STAGE_01		0
#define STAGE_02		1
#define STAGE_03		2
#define STAGE_04		3
#define STAGE_05		4
#define STAGE_06		5
#define STAGE_07		6
#define STAGE_08		7
#define STAGE_09		8
#define STAGE_10		9
#define STAGE_11		10
#define STAGE_12		11
#define STAGE_13		12
#define STAGE_14		13
#define STAGE_15		14
#define STAGE_16		15
#define STAGE_17		16
#define STAGE_18		17
#define STAGE_19		18
#define STAGE_COIN		19
#define STAGE_GHOST		20
#define STAGE_TOWER1	21
#define STAGE_TOWER2	22
#define STAGE_CASTLE1	23
#define STAGE_CASTLE2	24
#define STAGE_TOAD1		25
#define STAGE_TOAD2		26
#define STAGE_TOAD3		27
#define STAGE_TOAD4		28
#define STAGE_ENEMY1	32
#define STAGE_ENEMY2	33
#define STAGE_ENEMY3	34
#define STAGE_WARP		35
#define STAGE_DOOMSHIP	37
#define STAGE_HOME		38
#define STAGE_TITLE		39
#define STAGE_PEACH		40
#define STAGE_STAFFROLL	41

enum StageGroup {
	Normal = 0,
	Ghost = 1,
	Tower = 2,
	Castle = 3,
	Toad = 4,
	Enemy = 5,
	Warp = 6,
	Level37 = 7,
	Doomship = 8,
	Home = 9,
	Peach = 10,
	Invalid = 11
};


//enum SceneTypes {
//	BOOT = 0,
//	AUTO_SELECT = 1,
//	SELECT = 2,
//	WORLD_MAP = 3,			*
//	WORLD_9_DEMO = 4,
//	STAGE = 5,
//	RESTART_CRSIN = 6,		*
//	CRSIN = 7,
//	MOVIE = 8,				*
//	GAMEOVER = 9,
//	GAME_SETUP = 10,		*
//	MULTI_PLAY_COURSE_SELECT = 11
//}	


enum SceneParameters {
	SOMETHING_TODO_WITH_MOVIE = 1,
	SOMETHING_TODO_WITH_MOVIE2 = 2,
	SOMETHING_TODO_WITH_MOVIE3 = 3
};

enum PowerupStoreTypes {
	BEAT_LEVEL = 0, // Keeps powerups
	LOSE_LEVEL = 1, // Loses everything
	EXIT_LEVEL = 2  // Loses powerups, resets to previous state
};


enum Wipes {
	FADE_OUT = 0,
	CIRCLE_WIPE = 1,
	BOWSER_WIPE = 2,
	GOO_WIPE_DOWN = 3,	
	MARIO_WIPE = 4,
	CIRCLE_WIPE_SLOW = 5,
	GLITCH_GASM = 6
};


void ExitStage(int scene, int sceneParams, int powerupStoreType, int wipe);

extern PowerupStoreTypes LastPowerupStoreType;




#endif















