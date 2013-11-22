#include <common.h>
#include <game.h>


class SpriteSpawnerTimed : public dStageActor_c {
public:
	int onCreate();
	int onExecute();

	static SpriteSpawnerTimed *build();

	u64 eventFlag;	// 0x3D0
	u16 type;		// 0x3D4
	u32 inheritSet;	// 0x3D6
	u8 lastEvState;	// 0x3DA
	u32 timer;
};


SpriteSpawnerTimed *SpriteSpawnerTimed::build() {
	void *buffer = AllocFromGameHeap1(sizeof(SpriteSpawnerTimed));
	return new(buffer) SpriteSpawnerTimed;
}



int SpriteSpawnerTimed::onCreate() {

	char eventNum	= (this->settings >> 28)	& 0xF;

	this->eventFlag = (u64)1 << (eventNum - 1);
	this->type		= (this->settings >> 16) & 0xFFF;
	
	short tempSet = this->settings & 0xFFFF;
	this->inheritSet = (tempSet & 3) | ((tempSet & 0xC) << 2) | ((tempSet & 0x30) << 4) | ((tempSet & 0xC0) << 6) | ((tempSet & 0x300) << 8) | ((tempSet & 0xC00) << 10) | ((tempSet & 0x3000) << 12) | ((tempSet & 0xC000) << 14);
	
	this->timer = 0;
	
	return true;
}

int SpriteSpawnerTimed::onExecute() {

	if (dFlagMgr_c::instance->flags & this->eventFlag) {		 // If the event is on
		if (this->timer < 1) {						// If the timer is empty
			CreateActor(this->type, this->inheritSet, this->pos, 0, 0);
			this->timer = 120;
		}		

		this->timer--;
	}

	else {
		this->timer = 0;
	}

	return true;
}
