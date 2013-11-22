#ifndef __KOOPATLAS_SHOP_H
#define __KOOPATLAS_SHOP_H

#include "koopatlas/core.h"
#include "texmapcolouriser.h"

class dWMShop_c : public dActor_c {
	public:
		static dWMShop_c *build();
		static dWMShop_c *instance;

		dWMShop_c();

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();
		void specialDraw1();

		bool layoutLoaded;
		m2d::EmbedLayout_c layout;

		bool visible;
		float scaleEase;
		int timer;

		int selected, lastTopRowChoice;
		int shopKind;

		int coinsRemaining, timerForCoinCountdown;

		enum Animation {
			SHOW_ALL = 0,
			HIDE_ALL = 1,
			ACTIVATE_BUTTON = 2, // 3, 4, 5, 6, 7
			DEACTIVATE_BUTTON = 8, // 9, 10, 11, 12, 13
			COUNT_COIN = 14,
		};

		enum ItemTypes {
			MUSHROOM = 0,
			FIRE_FLOWER,
			PROPELLER,
			ICE_FLOWER,
			PENGUIN,
			MINI_SHROOM,
			STARMAN,
			HAMMER,
			ONE_UP,
			ITEM_TYPE_COUNT
		};

		enum _Constants {
			ITEM_COUNT = 12,
		};

		static const ItemTypes Inventory[10][12];

		nw4r::lyt::TextBox
			*Title, *TitleShadow,
			*CoinCount, *CoinCountShadow,
			*BackText, *BuyText;

		nw4r::lyt::Picture
			*BtnLeft[6], *BtnMid[6], *BtnRight[6];

		nw4r::lyt::Pane
			*Buttons[6], *Btn1Base, *Btn2Base;

		dTexMapColouriser_c leftCol, midCol, rightCol;

		class ShopModel_c {
			public:
				mHeapAllocator_c allocator;

				nw4r::g3d::ResFile res;
				m3d::mdl_c model;
				m3d::anmChr_c animation;

				float x, y, scaleFactor, scaleEase;
				bool isLakitu, playingNotEnough;

				void setupItem(float x, float y, ItemTypes type);
				void setupLakitu(int id);
				void execute();
				void draw();
				void playAnim(const char *name, float rate, char loop);
		};

		ShopModel_c *itemModels;
		ShopModel_c *lakituModel;

		void show(int shopNumber);

		void loadInfo();
		void loadModels();
		void deleteModels();

		void buyItem(int item);

		void showSelectCursor();

		dStateWrapper_c<dWMShop_c> state;

		USING_STATES(dWMShop_c);
		DECLARE_STATE(Hidden);
		DECLARE_STATE(ShowWait);
		DECLARE_STATE(ButtonActivateWait);
		DECLARE_STATE(CoinCountdown);
		DECLARE_STATE(Wait);
		DECLARE_STATE(HideWait);
};

#endif

