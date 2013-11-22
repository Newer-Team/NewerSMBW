#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>

class daElectricLine : public dEn_c {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;

	dEn_c *Needles;
	u32 delay;
	u32 timer;
	char loops;

	static daElectricLine *build();

	USING_STATES(daElectricLine);
	DECLARE_STATE(Activate);
	DECLARE_STATE(Deactivate);
	DECLARE_STATE(Die);
};

daElectricLine *daElectricLine::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daElectricLine));
	return new(buffer) daElectricLine;
}

///////////////////////
// Externs and States
///////////////////////


	CREATE_STATE(daElectricLine, Activate);
	CREATE_STATE(daElectricLine, Deactivate);
	CREATE_STATE(daElectricLine, Die);



int daElectricLine::onCreate() {

	Vec temppos = this->pos;
	temppos.x += 24.0;

	// Settings for rotation: 0 = facing right, 1 = facing left, 2 = facing up, 3 = facing down
	char settings = 0;
	if (this->settings & 0x1) {
		settings = 1;
		temppos.x -= 32.0;
	}


	Needles = (daNeedles*)create(NEEDLE_FOR_KOOPA_JR_B, settings, &temppos, &this->rot, 0);
	Needles->doStateChange(&daNeedles::StateID_DemoWait);
	
	// Needles->aPhysics.info.category1 = 0x3;
	// Needles->aPhysics.info.bitfield1 = 0x4F;
	// Needles->aPhysics.info.bitfield2 = 0xffbafffe;

	// Delay in 1/6ths of a second
	this->delay = (this->settings >> 16) * 10;
	this->loops = (this->settings >> 4);

	// State Changers
	doStateChange(&StateID_Activate);

	this->onExecute();
	return true;
}

int daElectricLine::onDelete() {
	return true;
}

int daElectricLine::onExecute() {
	acState.execute();	
	return true;
}

int daElectricLine::onDraw() {
	return true;
}


// States:
//
// DemoWait - all nullsubs, does nothing
// DemoAwake - moves the spikes in their respective directions
// Idle - Fires off an infinity of effects for some reason.
// Die - removes physics, then nullsubs


///////////////
// Activate State
///////////////
	void daElectricLine::beginState_Activate() { 
		this->timer = this->delay;
		Needles->doStateChange(&daNeedles::StateID_Idle);
	}
	void daElectricLine::executeState_Activate() { 
		if (this->loops) {
			this->timer--;
			if (this->timer == 0) {
				this->loops += 1;
				doStateChange(&StateID_Deactivate);
			}
		}
	}
	void daElectricLine::endState_Activate() { }

///////////////
// Deactivate State
///////////////
	void daElectricLine::beginState_Deactivate() { 
		this->timer = this->delay; 
		Needles->removeMyActivePhysics();
		Needles->doStateChange(&daNeedles::StateID_DemoWait);
	}
	void daElectricLine::executeState_Deactivate() { 

		this->timer--;
		if (this->timer == 0) {
			doStateChange(&StateID_Activate);
		}
	}
	void daElectricLine::endState_Deactivate() { 
		Needles->addMyActivePhysics();
	}


///////////////
// Die State
///////////////
	void daElectricLine::beginState_Die() { Needles->doStateChange(&daNeedles::StateID_Die); }
	void daElectricLine::executeState_Die() { }
	void daElectricLine::endState_Die() { }

