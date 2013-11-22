#include <common.h>
#include <game.h>
#include <g3dhax.h>

class daFireLaser_c : public dEn_c {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	static daFireLaser_c *build();

	int timer;
	float spitspeed;
	char direction;
	u64 eventFlag;

	USING_STATES(daFireLaser_c);
	DECLARE_STATE(pewpewpew);
};

daFireLaser_c *daFireLaser_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daFireLaser_c));
	return new(buffer) daFireLaser_c;
}


CREATE_STATE(daFireLaser_c, pewpewpew);




int daFireLaser_c::onCreate() {

	this->timer = 0;
	this->direction = this->settings & 0xF;
	this->spitspeed = 8.0;
	
	char eventNum	= (this->settings >> 16) & 0xFF;
	this->eventFlag = (u64)1 << (eventNum - 1);

	
	doStateChange(&StateID_pewpewpew);
	this->onExecute();
	return true;
}

int daFireLaser_c::onDelete() {
	return true;
}

int daFireLaser_c::onExecute() {
	acState.execute();
	return true;
}

int daFireLaser_c::onDraw() {
	return true;
}



// Pew Pew State

void daFireLaser_c::beginState_pewpewpew() { 
	this->timer = 0;
}
void daFireLaser_c::executeState_pewpewpew() { 
	
	
	if (dFlagMgr_c::instance->flags & this->eventFlag) {
		
		this->timer = this->timer + 1;
	
		if (this->timer < 20) {
			float xlaunch;
			float ylaunch;
			
			if (this->direction == 0) { 
				xlaunch = this->spitspeed;
				ylaunch = 0.0; }
			else if (this->direction == 1) { // SE
				xlaunch = this->spitspeed;
				ylaunch = this->spitspeed; }
			else if (this->direction == 2) { // S
				xlaunch = 0.0;
				ylaunch = this->spitspeed; }
			else if (this->direction == 3) { // SW
				xlaunch = -this->spitspeed;
				ylaunch = this->spitspeed; }
			else if (this->direction == 4) {	// W
				xlaunch = -this->spitspeed;
				ylaunch = 0.0; }
			else if (this->direction == 5) {	// NW
				xlaunch = -this->spitspeed;
				ylaunch = -this->spitspeed; }
			else if (this->direction == 6) {	// N
				xlaunch = 0.0;
				ylaunch = -this->spitspeed; }
			else if (this->direction == 7) {	// NE
				xlaunch = this->spitspeed;
				ylaunch = -this->spitspeed; }
			
			
			dStageActor_c *spawner = CreateActor(106, 0, this->pos, 0, 0);
			spawner->speed.x = xlaunch;
			spawner->speed.y = ylaunch;
		}
		
		if (this->timer > 60) {
			this->timer = 0;
		}

	}
	
	else { this->timer = 0; }
	
}
void daFireLaser_c::endState_pewpewpew() { 

}













