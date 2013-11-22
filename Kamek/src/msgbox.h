#ifndef MSGBOX_H
#define MSGBOX_H 
#include <common.h>
#include <game.h>

class dMsgBoxManager_c : public dStageActor_c {
	public:
		void showMessage(int id, bool canCancel=true, int delay=-1);

		dMsgBoxManager_c() : state(this, &StateID_Wait) { }

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		int beforeExecute() { return true; }
		int afterExecute(int) { return true; }
		int beforeDraw() { return true; }
		int afterDraw(int) { return true; }

		m2d::EmbedLayout_c layout;

		bool layoutLoaded;
		bool visible;

		bool canCancel;
		int delay;

		dStateWrapper_c<dMsgBoxManager_c> state;

		USING_STATES(dMsgBoxManager_c);
		DECLARE_STATE(Wait);
		DECLARE_STATE(BoxAppearWait);
		DECLARE_STATE(ShownWait);
		DECLARE_STATE(BoxDisappearWait);

		static dMsgBoxManager_c *instance;
		static dMsgBoxManager_c *build();
};
#endif /* MSGBOX_H */
