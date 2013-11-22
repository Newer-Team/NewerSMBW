#ifndef __KOOPATLAS_PLAYER_H
#define __KOOPATLAS_PLAYER_H

#include "koopatlas/core.h"
#include "poweruphax.h"
#include <playerAnim.h>


class daWMPlayer_c : public dActor_c {
	public:
		dPlayerModelHandler_c *modelHandler;

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		int currentAnim;
		float currentFrame;
		float currentUnk;
		float currentUpdateRate;

		bool hasEffect;
		const char *effectName;
		bool hasSound;
		bool step;
		int soundName;

		int timer;

		float jumpOffset;

		mEf::es2 effect;
		dHammerSuitRenderer_c hammerSuit;

		bool spinning;
		s16 targetRotY;

		void setTargetRotY(s16 value) {
			targetRotY = value;
			spinning = false;
		}
		void setSpinning() {
			spinning = true;
		}

		mHeapAllocator_c allocator;
		m3d::anmTexPat_c pats[4];
		void bindPats();

		void startAnimation(int id, float frame, float unk, float updateRate);

		static daWMPlayer_c *build();
		static daWMPlayer_c *instance;
};

#endif

