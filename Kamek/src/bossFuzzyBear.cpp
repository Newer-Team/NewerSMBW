#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>
#include "boss.h"


class daFuzzyBear_c : public daBoss {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;
	nw4r::g3d::ResFile resFile;
	m3d::anmChr_c animationChr;

	int timer;
	char BigBossFuzzyBear;
	float Baseline;
	float AreaWidthLeft;
	float AreaWidthRight;
	float LaunchSpeedShort;
	float LaunchSpeedHigh;
	char dying;
	float storeSpeed;
	Vec initialPos;
	char RolyBounces;
	Vec RolyPos;
	char falldown;
	char damage;
	char roly;
	char isInvulnerable;

	static daFuzzyBear_c *build();

	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);
	void setupBodyModel();
	void updateModelMatrices();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);

	USING_STATES(daFuzzyBear_c);
	DECLARE_STATE(Grow);
	DECLARE_STATE(Bounce);
	// DECLARE_STATE(Needles);
	//DECLARE_STATE(Spray);
	DECLARE_STATE(RolyPoly);
	DECLARE_STATE(Wait);
	DECLARE_STATE(Outro);
};

daFuzzyBear_c *daFuzzyBear_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daFuzzyBear_c));
	return new(buffer) daFuzzyBear_c;
}



CREATE_STATE(daFuzzyBear_c, Grow);
CREATE_STATE(daFuzzyBear_c, Bounce);
// CREATE_STATE(daFuzzyBear_c, Needles);
//CREATE_STATE(daFuzzyBear_c, Spray);
CREATE_STATE(daFuzzyBear_c, RolyPoly);
CREATE_STATE(daFuzzyBear_c, Wait);
CREATE_STATE(daFuzzyBear_c, Outro);



void daFuzzyBear_c::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) { DamagePlayer(this, apThis, apOther); }
void daFuzzyBear_c::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) { DamagePlayer(this, apThis, apOther); }
bool daFuzzyBear_c::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true; // added by skawo request

	if (this->isInvulnerable == 1) { return true; }

	this->timer = 0;
	PlaySound(this, SE_BOSS_KOOPA_FIRE_DISAPP);

	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};
	SpawnEffect("Wm_mr_fireball_hit", 0, &apOther->owner->pos, &nullRot, &oneVec);
	this->damage++;
	if (this->damage > 14) { doStateChange(&StateID_Outro); }
	return true;
}
bool daFuzzyBear_c::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
	apOther->someFlagByte |= 2;

	dActor_c *block = apOther->owner;
	dEn_c *mario = (dEn_c*)block;

	mario->speed.y = -mario->speed.y;
	mario->pos.y += mario->speed.y;

	if (mario->direction == 0) { mario->speed.x = 4.0; }
	else					  { mario->speed.x = -4.0; }

	mario->doSpriteMovement();
	mario->doSpriteMovement();
	return true;
}
bool daFuzzyBear_c::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->counter_504[apOther->owner->which_player] = 0;
	return this->collisionCat9_RollingObject(apThis, apOther);
}
bool daFuzzyBear_c::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {

	dActor_c *block = apOther->owner;
	dEn_c *blah = (dEn_c*)block;

	//OSReport("Fuzzy was hit by an actor %d (%p); its field 12C is %x\n", block->name, block, blah->_12C);
	if (block->name == AC_LIGHT_BLOCK) {
		//OSReport("It's a light block, and its state is %s\n", blah->acState.getCurrentState()->getName());
		if (blah->_12C & 3 || strcmp(blah->acState.getCurrentState()->getName(), "daLightBlock_c::StateID_Throw")) {
			//OSReport("Ignoring this!\n");
			return true;
		}
	}

	if (blah->direction == 0) { blah->direction = 1; this->roly = 1; }
	else					  { blah->direction = 0; this->roly = 0; }

	blah->speed.x = -blah->speed.x;
	blah->pos.x += blah->speed.x;

	if (blah->speed.y < 0) {
		blah->speed.y = -blah->speed.y; }

	blah->doSpriteMovement();
	blah->doSpriteMovement();

	if (this->isInvulnerable == 1) {

		if (blah->direction == 0) { blah->direction = 1; }
		else					  { blah->direction = 0; }

		return true;
	}

	this->pos.x += blah->speed.x;

	this->timer = 0;
	this->damage = this->damage + 5;


	PlaySound(this, SE_EMY_BLOW_PAKKUN_DOWN);
	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};
	SpawnEffect("Wm_mr_kickhit", 0, &blah->pos, &nullRot, &oneVec);

	if (this->damage > 14) { doStateChange(&StateID_Outro); }
	else { doStateChange(&StateID_RolyPoly); }
	return true;
}
bool daFuzzyBear_c::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {

	if (this->isInvulnerable == 1) { return true; }

	dActor_c *block = apOther->owner;
	dEn_c *blah = (dEn_c*)block;

	if (blah->direction == 0) { blah->direction = 1; this->roly = 1; }
	else					  { blah->direction = 0; this->roly = 0; }

	PlaySound(this, SE_EMY_BIG_PAKKUN_DAMAGE_1);
	this->timer = 0;
	this->damage += 5;

	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};
	SpawnEffect("Wm_mr_kick_glow", 0, &apOther->owner->pos, &nullRot, &oneVec);

	if (this->damage > 14) { doStateChange(&StateID_Outro); }
	else { doStateChange(&StateID_RolyPoly); }
	return true;
}

bool daFuzzyBear_c::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
}
bool daFuzzyBear_c::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther) {
	return true;
}
bool daFuzzyBear_c::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther) {
	DamagePlayer(this, apThis, apOther);
	return true;
}




void daFuzzyBear_c::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->animationChr.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->animationChr, unk2);
	this->animationChr.setUpdateRate(rate);
}


void daFuzzyBear_c::setupBodyModel() {
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	this->resFile.data = getResource("chorobon", "g3d/chorobon.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("chorobon");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);

	bool ret;
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("run");
	ret = this->animationChr.setup(mdl, anmChr, &this->allocator, 0);

	allocator.unlink();
}


int daFuzzyBear_c::onCreate() {

	setupBodyModel();


	this->BigBossFuzzyBear = this->settings >> 28;
	this->scale = (Vec){1.0, 1.0, 1.0};


	ActivePhysics::Info HitMeBaby;
	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = 0.0;

	if (BigBossFuzzyBear == 0) {
		HitMeBaby.xDistToEdge = 30.0;
		HitMeBaby.yDistToEdge = 30.0; }
	else {
		HitMeBaby.xDistToEdge = 35.0;
		HitMeBaby.yDistToEdge = 35.0; }

	HitMeBaby.category1 = 0x3;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x4F;
	HitMeBaby.bitfield2 = 0x8028E;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &dEn_c::collisionCallback;


	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();


	this->pos.y = this->pos.y + 6;
	this->rot.x = 0; // X is vertical axis
	this->rot.y = 0; // Y is horizontal axis
	this->rot.z = 0; // Z is ... an axis >.>
	this->direction = 0; // Heading left.

	pos.z = 3000.0f;

	this->speed.x = 0;
	this->LaunchSpeedShort = ((this->settings >> 20) & 0xF) * 10.0;
	this->LaunchSpeedHigh = ((this->settings >> 24) & 0xF) * 10.0;

	this->AreaWidthRight = this->settings & 0xFF;
	this->AreaWidthLeft = (this->settings >> 8) & 0xFF;

	this->initialPos = this->pos;
	this->storeSpeed = 0;
	this->falldown = 0;
	this->roly = 0;
	this->damage = 0;
	this->isInvulnerable = 0;
	this->dying = 0;
	this->disableEatIn();


	bindAnimChr_and_setUpdateRate("run", 1, 0.0, 1.0);

	doStateChange(&StateID_Grow);

	this->onExecute();
	return true;
}

int daFuzzyBear_c::onDelete() {
	return true;
}

int daFuzzyBear_c::onExecute() {
	acState.execute();
	updateModelMatrices();

	if(this->animationChr.isAnimationDone())
		this->animationChr.setCurrentFrame(0.0);

	return true;
}


int daFuzzyBear_c::onDraw() {
	bodyModel.scheduleForDrawing();
	bodyModel._vf1C();
	return true;
}


void daFuzzyBear_c::updateModelMatrices() {
	// This won't work with wrap because I'm lazy.
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


// Grow State

void daFuzzyBear_c::beginState_Grow() {
	this->timer = 0;

	SetupKameck(this, Kameck);
	this->scale = (Vec){1.0, 1.0, 1.0};
}

void daFuzzyBear_c::executeState_Grow() {

	this->timer += 1;

	bool ret;
	if (BigBossFuzzyBear == 1) {
		ret = GrowBoss(this, Kameck, 1.0, 3.0, 25, this->timer); }
	else {
		ret = GrowBoss(this, Kameck, 1.0, 2.5, 18, this->timer); }

	if (ret) { doStateChange(&StateID_Bounce);	}

}
void daFuzzyBear_c::endState_Grow() {
	this->Baseline = this->pos.y;

	CleanupKameck(this, Kameck);
}



// Bounce State

void daFuzzyBear_c::beginState_Bounce() {

	if (this->direction == 0) { this->speed.x = 1.0; }
	else 					 { this->speed.x = -1.0; }

	if (this->storeSpeed != 0) { this->speed.x = this->storeSpeed; }

	this->timer = 20;
}
void daFuzzyBear_c::executeState_Bounce() {

	float wallDistance, scaleDown, scaleUp, scaleBase;

	if (BigBossFuzzyBear == 0) {
		wallDistance = 32.0;
		scaleDown = 24.0;
		scaleUp = 10.0;
		scaleBase = 2.5;
	}
	else {
		wallDistance = 38.0;
		scaleDown = 32.0;
		scaleUp = 12.0;
		scaleBase = 3.0;
	}


	if (this->falldown == 1) { this->speed.x = 0; this->timer = 0; }


	// Check for walls

	if (this->pos.x <= this->initialPos.x - ((17 * 16.0) + wallDistance))  { // Hit left wall, head right.
		this->speed.x = -this->speed.x;
		this->direction = 1;
		this->pos.x = this->pos.x + 1.0; }

	if (this->pos.x >= this->initialPos.x + ((7.5 * 16.0) - wallDistance))  { // Hit right wall, head left.
		this->speed.x = -this->speed.x;
		this->direction = 0;
		this->pos.x = this->pos.x - 1.0; }


	// Check if we're on the ground.

	if (this->pos.y < this->Baseline) {

		this->falldown = 0;

		this->timer = this->timer + 1;
		if (this->speed.x != 0) {
			this->storeSpeed = this->speed.x; }
		this->speed.x = 0;
		this->speed.y = 0;


		if (this->timer < 10) {
			float modifier;
			modifier = scaleBase - (this->timer * 0.1);

			this->scale.y = modifier;
			this->pos.y = this->pos.y + (scaleDown/10.0);
			if (this->pos.y > this->Baseline) { this->pos.y = this->Baseline - 1.0; }
		}
		else if (this->timer == 10) {
			Vec tempPos = (Vec){this->pos.x, this->pos.y - wallDistance, 5500.0};
			S16Vec nullRot = {0,0,0};
			Vec oneVec = {1.0f, 1.0f, 1.0f};
			SpawnEffect("Wm_mr_beachlandsmk", 0, &tempPos, &nullRot, &oneVec);
		}
		else {
			float modifier;
			modifier = (scaleBase - 1.0) + ((this->timer - 10) * 0.1);

			this->scale.y = modifier;
			this->pos.y = this->pos.y + (scaleUp/10.0);
			if (this->pos.y > this->Baseline) { this->pos.y = this->Baseline - 1.0; }
			PlaySound(this, SE_PLY_JUMPDAI);
		}

		if (this->timer > 20) {

			int randChoice;

			randChoice = GenerateRandomNumber(5);
			if (randChoice == 0) { doStateChange(&StateID_Wait); }

			randChoice = GenerateRandomNumber(4);
			if (randChoice == 0) { this->speed.y = 8.0; }
			else { this->speed.y = 6.0; }

			this->timer = 0;
			this->pos.y = this->Baseline + 1;

			this->speed.x = this->storeSpeed;
		 }
	}

	else { this->speed.y = this->speed.y - 0.1; } // Gravity


	this->HandleXSpeed();
	this->HandleYSpeed();

	this->UpdateObjectPosBasedOnSpeedValuesReal();

}

void daFuzzyBear_c::endState_Bounce() { }





// Needles State - shoots out some black icicles

// void daFuzzyBear_c::beginState_Needles() {
// 	this->timer = 0;
// 	this->speed.y = 0;
// 	this->speed.x = 0;
// 	OSReport("Fuzzy Needle State Begin");
// }
// void daFuzzyBear_c::executeState_Needles() {
// 	float origScale;

// 	this->speed.y = 0;
// 	this->speed.x = 0;

// 	if (BigBossFuzzyBear == 0) {
// 		origScale = 2.5;
// 	}
// 	else {
// 		origScale = 3.0;
// 	}

// 	this->timer = this->timer + 1;
// 	OSReport("Needle Timer: %d", this->timer);

// 	if (this->timer <= 120) {
// 		this->scale.y = (sin(this->timer * 3.14 / 5.0) / 2.0) + origScale; // 3 shakes per second, exactly 24 shakes overall
// 		this->scale.x = (sin(this->timer * 3.14 / 5.0) / 2.0) + origScale; // 3 shakes per second, exactly 24 shakes overall

// 		if (this->timer == 30) {
// 			dStageActor_c *spawner = CreateActor(339, 0, this->pos, 0, 0);
// 			spawner->speed.x = -6.0;
// 			spawner->speed.y = 0.0;
// 			spawner->scale = (Vec){1.0, 1.0, 1.0};
// 		}

// 		if (this->timer == 45) {
// 			dStageActor_c *spawner = CreateActor(339, 0, this->pos, 0, 0);
// 			spawner->speed.x = 6.0;
// 			spawner->speed.y = 6.0;
// 			spawner->scale = (Vec){1.0, 1.0, 1.0};
// 		}

// 		if (this->timer == 60) {
// 			dStageActor_c *spawner = CreateActor(339, 0, this->pos, 0, 0);
// 			spawner->speed.x = 0.0;
// 			spawner->speed.y = 6.0;
// 			spawner->scale = (Vec){1.0, 1.0, 1.0};
// 		}

// 		if (this->timer == 75) {
// 			dStageActor_c *spawner = CreateActor(339, 0, this->pos, 0, 0);
// 			spawner->speed.x = -6.0;
// 			spawner->speed.y = 6.0;
// 			spawner->scale = (Vec){1.0, 1.0, 1.0};
// 		}

// 		if (this->timer == 90) {
// 			dStageActor_c *spawner = CreateActor(339, 0, this->pos, 0, 0);
// 			spawner->speed.x = -6.0;
// 			spawner->speed.y = 0.0;
// 			spawner->scale = (Vec){1.0, 1.0, 1.0};
// 		}
// 	}
// 	else { doStateChange(&StateID_Bounce); }

// 	this->HandleXSpeed();
// 	this->HandleYSpeed();

// 	this->UpdateObjectPosBasedOnSpeedValuesReal();

// }
// void daFuzzyBear_c::endState_Needles() { OSReport("Fuzzy Needle State End"); }

// Spray State - jumps in the air and shakes out some small fuzzies

/*void daFuzzyBear_c::beginState_Spray() {
	this->timer = 0;
	this->speed.y = 7.0;
	this->speed.x = 0.0;
}
void daFuzzyBear_c::executeState_Spray() {

	this->speed.x = 0.0;

	if (this->speed.y < 1.0) {
		this->speed.y = 0;

		if (this->timer < 120) {

			this->rot.y = sin(this->timer * 3.14 / 5) * 4000; // 3 shakes per second, exactly 24 shakes overall

			int randChoice;
			randChoice = GenerateRandomNumber(18); // 1.3 Fuzzies per second, 6 fuzzies overall

			if (randChoice == 0) {
				int randChoiceX, randChoiceY;
				randChoiceX = GenerateRandomNumber(92);
				randChoiceY = GenerateRandomNumber(48);

				float xa, ya;
				xa = randChoiceX - 48.0;
				ya = randChoiceY - 24.0;

				CreateActor(144, 0, (Vec){this->pos.x + xa, this->pos.y + ya, this->pos.z}, 0, 0);
			}
		}

		else { doStateChange(&StateID_Bounce); }

		this->timer = this->timer + 1;

	}

	else { this->speed.y = this->speed.y - 0.1; } // Gravity


	this->HandleXSpeed();
	this->HandleYSpeed();

	this->UpdateObjectPosBasedOnSpeedValuesReal();

}
void daFuzzyBear_c::endState_Spray() {
	this->rot.y = 0;
	this->timer = 20;
	this->falldown = 1;
}*/


// Roly Poly State - Rolls from left to right, bounces off both walls, and returns to original position.

void daFuzzyBear_c::beginState_RolyPoly() {

	this->isInvulnerable = 1;

	if (this->roly == 1) {
		this->direction = 1;
		this->speed.x = 12.0; }
	else 				 {
		this->direction = 0;
		this->speed.x = -12.0; }

	this->speed.y = 0;
	this->RolyBounces = 0;
	this->RolyPos = this->pos;

	if (BigBossFuzzyBear == 0) {
		this->scale = (Vec){2.5, 2.5, 2.5};
	}
	else {
		this->scale = (Vec){3.0, 3.0, 3.0};
	}

	this->timer = 0;
}
void daFuzzyBear_c::executeState_RolyPoly() {
	float wallDistance, scaleDown, scaleUp;

	PlaySound(this, SE_OBJ_TEKKYU_G_CRASH);

	if (this->pos.y > this->Baseline) { this->pos.y -= 2.0; }
	else {
		this->pos.y = this->Baseline - 1.0;

		Vec tempPos = (Vec){this->pos.x, this->pos.y - 34.0f, 5500.0f};

		if (this->timer & 0x1) {
			S16Vec nullRot = {0,0,0};
			Vec efScale = {0.4f, 0.4f, 0.4f};
			SpawnEffect("Wm_ob_icehitsmk", 0, &tempPos, &nullRot, &efScale);
		}
	 }

	if (this->direction == 0) { // is even
		this->pos.x = this->pos.x - 3.0; }
	else { // is odd
		this->pos.x = this->pos.x + 3.0; }

	if (BigBossFuzzyBear == 0) {
		wallDistance = 38.0;
	}
	else {
		wallDistance = 50.0;
	}

	this->timer += 1;

	if (this->pos.x <= this->initialPos.x - ((17 * 16.0) + wallDistance))  { // Hit left wall, head right.
		this->speed.x = -this->speed.x;
		this->direction = 1;
		this->pos.x = this->pos.x + 1.0;
		this->RolyBounces = this->RolyBounces + 1;
		}

	if (this->pos.x >= this->initialPos.x + ((7.5 * 16.0) - wallDistance))  { // Hit right wall, head left.
		this->speed.x = -this->speed.x;
		this->direction = 0;
		this->pos.x = this->pos.x - 1.0;
		this->RolyBounces = this->RolyBounces + 1;
		}


	if (this->direction == 1) { this->rot.z = this->rot.z - 0x400; }
	else 					  { this->rot.z = this->rot.z + 0x400; }

	if (this->RolyBounces == 2) {
		if ((this->pos.x > this->RolyPos.x -20.0) && (this->pos.x < this->RolyPos.x + 20.0)) {
			this->speed.x = 0;
			if (this->rot.z == 0) { doStateChange(&StateID_Bounce); } }
	}



//	this->HandleXSpeed();
//	this->HandleYSpeed();

//	this->doSpriteMovement();
//	this->UpdateObjectPosBasedOnSpeedValuesReal();

 }
void daFuzzyBear_c::endState_RolyPoly() {
	this->rot.z = 0;
	this->isInvulnerable = 0;
	this->timer = 0;
}





void daFuzzyBear_c::beginState_Wait() { this->timer = 0;}
void daFuzzyBear_c::executeState_Wait() {

	this->timer = this->timer + 1;

	if (this->timer > 60) {
		doStateChange(&StateID_Bounce);
		//int randChoice;

		//if (BigBossFuzzyBear == 1) { doStateChange(&StateID_Spray); }
		//else 					   { doStateChange(&StateID_Bounce); }
	}
}
void daFuzzyBear_c::endState_Wait() { }



void daFuzzyBear_c::beginState_Outro() {
	OutroSetup(this);
}
void daFuzzyBear_c::executeState_Outro() {

	if (this->dying == 1) {
		if (this->timer > 180) {
			ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE);
		}
		if (this->timer == 60) { PlayerVictoryCries(this); }

		this->timer += 1;
		return;
	}

	bool ret;
	ret = ShrinkBoss(this, &this->pos, 2.75, this->timer);

	if (ret == true) 	{
		BossExplode(this, &this->pos);
		this->dying = 1;
		this->timer = 0;
	}

	this->timer += 1;

}
void daFuzzyBear_c::endState_Outro() { }





