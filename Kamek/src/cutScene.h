#ifndef __CUT_SCENE_H
#define __CUT_SCENE_H

#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>

void CutSceneDrawFunc();

struct dMovieData_s;

class dScCutScene_c : public dScene_c {
	public:
		dScCutScene_c();
		~dScCutScene_c();

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		void goToNextScene();

		int currentScene;
		int nextScene;
		int frameOffset;

		nw4r::snd::SoundHandle handles[40];

		dMovieData_s *data;

		dDvdLoader_c settingsLoader;
		dDvdLoader_c *sceneLoaders;

		m2d::EmbedLayout_c *layout;

		dYesNoWindow_c *yesNoWindow;
		int yesNoStage;

		static dScCutScene_c *build();
		static dScCutScene_c *instance;
};


struct dMovieScene_s {
	char *sceneName;
	u8 widescreenFlag;
	u32 commandCount;
	u32 command[1];
};

struct dMovieData_s {
	u32 magic;
	u32 sceneCount;
	dMovieScene_s *scenes[1];
};

#endif

