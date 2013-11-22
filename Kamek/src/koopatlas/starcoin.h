#ifndef __KOOPATLAS_STARCOIN_H
#define __KOOPATLAS_STARCOIN_H

#include "koopatlas/core.h"

class dWMStarCoin_c : public dActor_c {
	public:
		dWMStarCoin_c();

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		bool layoutLoaded;
		m2d::EmbedLayout_c layout;

		bool visible;
		void show();

		static dWMStarCoin_c *build();
		static dWMStarCoin_c *instance;

		enum Animation {
			SHOW_ALL = 0,
			SHOW_SECTION,
			HIDE_SECTION,
			SHOW_LEFT_ARROW,
			HIDE_LEFT_ARROW,
			SHOW_RIGHT_ARROW,
			HIDE_RIGHT_ARROW
		};

		enum _Constants {
			ROW_COUNT = 9,
			COLUMN_COUNT = 2,
			SHINE_COUNT = 5,
			MAX_SECTION_COUNT = 16,
		};

		int currentSection;
		int currentSectionIndex;
		int sectionIndices[MAX_SECTION_COUNT];
		int availableSectionCount;

		bool isLeftArrowVisible, isRightArrowVisible;
		bool willExit;

		bool canScrollLeft() const;
		bool canScrollRight() const;
		void loadInfo();
		void loadSectionInfo();

		void showLeftArrow();
		void showRightArrow();
		void hideLeftArrow();
		void hideRightArrow();
		void setLeftArrowVisible(bool value);
		void setRightArrowVisible(bool value);

		void showSecretMessage(const wchar_t *title, const wchar_t **body, int lineCount, const wchar_t **body2 = 0, int lineCount2 = 0);

		nw4r::lyt::Picture
			*Shine[COLUMN_COUNT][SHINE_COUNT],
			*CoinOutline[COLUMN_COUNT][ROW_COUNT][3],
			*Coin[COLUMN_COUNT][ROW_COUNT][3],
			*DPadLeft, *DPadRight;

		nw4r::lyt::TextBox
			*LevelName[COLUMN_COUNT][ROW_COUNT],
			*LeftTitle, *RightTitle,
			*TotalCoinCount, *UnspentCoinCount,
			*EarnedCoinCount, *EarnedCoinMax,
			*BtnBackText;

		dStateWrapper_c<dWMStarCoin_c> state;

		USING_STATES(dWMStarCoin_c);
		DECLARE_STATE(Hidden);
		DECLARE_STATE(ShowWait);
		DECLARE_STATE(ShowSectionWait);
		DECLARE_STATE(Wait);
		DECLARE_STATE(HideSectionWait);
		DECLARE_STATE(HideWait);
};

#endif

