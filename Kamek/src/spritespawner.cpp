#include <game.h>

class dSpriteSpawner_c : public dStageActor_c {
	public:
		static dSpriteSpawner_c *build();

		u64 classicEventOverride;
		Actors profileID;
		bool respawn;
		u32 childSettings;
		u32 childID;

		int onCreate();
		int onExecute();
};

/*****************************************************************************/
// Glue Code
dSpriteSpawner_c *dSpriteSpawner_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dSpriteSpawner_c));
	dSpriteSpawner_c *c = new(buffer) dSpriteSpawner_c;
	return c;
}


int dSpriteSpawner_c::onCreate() {
	char classicEventNum = (settings >> 28) & 0xF;
	classicEventOverride = (classicEventNum == 0) ? 0 : ((u64)1 << (classicEventNum - 1));

	profileID = (Actors)((settings >> 16) & 0x7FF);
	respawn = (settings >> 27) & 1;

	u16 tempSet = settings & 0xFFFF;
	childSettings =
		(tempSet & 3) | ((tempSet & 0xC) << 2) |
		((tempSet & 0x30) << 4) | ((tempSet & 0xC0) << 6) |
		((tempSet & 0x300) << 8) | ((tempSet & 0xC00) << 10) |
		((tempSet & 0x3000) << 12) | ((tempSet & 0xC000) << 14);

	return true;
}


int dSpriteSpawner_c::onExecute() {
	u64 effectiveFlag = classicEventOverride | spriteFlagMask;

	if (dFlagMgr_c::instance->flags & effectiveFlag) {
		if (!childID) {
			dStageActor_c *newAc = dStageActor_c::create(profileID, childSettings, &pos, 0, 0);
			childID = newAc->id;
		}
	} else {
		if (respawn)
			return true;

		if (childID) {
			dStageActor_c *ac = (dStageActor_c*)fBase_c::search(childID);
			if (ac) {
				pos = ac->pos;
				ac->Delete(1);
			}
			childID = 0;
		}
	}

	if (respawn) {
		if (childID) {
			dStageActor_c *ac = (dStageActor_c*)fBase_c::search(childID);

			if (!ac) {
				dStageActor_c *newAc = dStageActor_c::create(profileID, childSettings, &pos, 0, 0);
				childID = newAc->id;
			}
		}
	}

	return true;

}

