#ifndef __KOOPATLAS_HUD_H
#define __KOOPATLAS_HUD_H

#include "koopatlas/core.h"
#include "texmapcolouriser.h"

class dWMHud_c : public dBase_c {
	public:
		dWMHud_c();

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		bool layoutLoaded;
		m2d::EmbedLayout_c layout;

		static dWMHud_c *build();
		static dWMHud_c *instance;

		bool doneFirstShow;
		void loadInitially();

		void enteredNode(dKPNode_s *node = 0);
		void leftNode();

		void hideFooter();
		void showFooter();

		void setupLives();

		void hideAll();
		void unhideAll();

	private:
		void playShowAnim(int id);
		void playHideAnim(int id);
		void loadHeaderInfo();

		bool willShowHeader;
		dKPNode_s *nodeForHeader;

		int displayedControllerType;
		void updatePressableButtonThingies();

		void loadFooterInfo();
		bool willShowFooter;
		bool isFooterVisible;

		dTexMapColouriser_c headerCol, footerCol;


		nw4r::lyt::Pane
			*N_IconPosXP_00[4];

		nw4r::lyt::Picture
			*Header_Centre, *Header_Right, *Footer,
			*NormalExitFlag, *SecretExitFlag,
			*StarCoinOff[3],
			*StarCoinOn[3],
			*P_marioFace_00, *P_luigiFace_00,
			*P_BkinoFace_00, *P_YkinoFace_00,
			*Star[3];

		nw4r::lyt::TextBox
			*LevelName, *LevelNameS,
			*LevelNumber, *LevelNumberS,
			*WorldName, *WorldNameS,
			*StarCoinCounter,
			*T_lifeNumber[4];
};

#endif

