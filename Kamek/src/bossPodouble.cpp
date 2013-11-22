#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>
#include "boss.h"

void poodleCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther);

void poodleCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (apOther->owner->name == BROS_FIREBALL) { return; }
		else if (apOther->owner->name == BROS_ICEBALL) { return; }
		else { dEn_c::collisionCallback(apThis, apOther); }
	}

class daPodouble : public daBoss {
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	void updateHitboxSize();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;

	m3d::mdl_c bodyModel;
	m3d::mdl_c fogModel;
	//m3d::mdl_c fog2Model;

	m3d::anmChr_c fleeAnimation;
	m3d::anmTexSrt_c body;

	m3d::anmChr_c fogChr;
	m3d::anmTexSrt_c fogSRT;

	//m3d::anmChr_c fog2Chr;
	//m3d::anmTexSrt_c fog2SRT;


	char isFire;
	char goingUp;
	int timer;
	float dying;
	int damage;
	float Baseline;
	char isInvulnerable;
	int countdown;

	static daPodouble *build();

	// void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther);
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther);


	USING_STATES(daPodouble);
	DECLARE_STATE(Bounce);
	DECLARE_STATE(Spit);
	DECLARE_STATE(Damage);

	DECLARE_STATE(Grow);
	DECLARE_STATE(Outro);
	DECLARE_STATE(SyncDie);

};

daPodouble *daPodouble::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daPodouble));
	return new(buffer) daPodouble;
}

///////////////////////
// Externs and States
///////////////////////
	extern "C" int SmoothRotation(short* rot, u16 amt, int unk2);

	CREATE_STATE(daPodouble, Bounce);
	CREATE_STATE(daPodouble, Spit);
	CREATE_STATE(daPodouble, Damage);

	CREATE_STATE(daPodouble, Grow);
	CREATE_STATE(daPodouble, Outro);
	CREATE_STATE(daPodouble, SyncDie);


////////////////////////
// Collision Functions
////////////////////////

	void daPodouble::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) { DamagePlayer(this, apThis, apOther); }

	void daPodouble::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) 					 { this->playerCollision(apThis, apOther); }
	bool daPodouble::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) 	 {
		this->playerCollision(apThis, apOther);
		return true;
	}
	bool daPodouble::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
		this->playerCollision(apThis, apOther);
		return true;
	}
	bool daPodouble::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther)		 {
		this->playerCollision(apThis, apOther);
		return true;
	}

	bool daPodouble::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) 	{ return true; }
	bool daPodouble::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) { return true; }
	bool daPodouble::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther)		{ return true; }

	bool daPodouble::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
		apOther->owner->kill();
		return true;
	}

	bool daPodouble::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther){
		apOther->owner->Delete(1);
		if (this->isInvulnerable) { return true; }

		if (this->isFire == 0) {
			this->damage += 4;

			if (this->damage < 12)  { doStateChange(&StateID_Damage); }
			else 				    { doStateChange(&StateID_Outro); }
			return true;
		} else return false;
	}

	bool daPodouble::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
		if (this->isInvulnerable) { return true; }

		if (this->isFire == 0) {
			this->damage += 2;

			if (this->damage < 12)  { doStateChange(&StateID_Damage); }
			else 				    { doStateChange(&StateID_Outro); }
			return true;
		} else return false;
	}

	bool daPodouble::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
		apOther->owner->Delete(1);
		if (this->isInvulnerable) { return true; }

		if (this->isFire == 1) {
			if (apOther->owner->name == 104) { this->damage += 2; }
			else 						     { this->damage += 4; }

			if (this->damage < 12)  { doStateChange(&StateID_Damage); }
			else 				    { doStateChange(&StateID_Outro); }

			return true;
		}
		else { return false; }
	}


void daPodouble::updateHitboxSize() {
	aPhysics.info.xDistToEdge = 11.429f * scale.x;
	aPhysics.info.yDistToEdge = 11.429f * scale.y;
}


int daPodouble::onCreate() {

	this->isFire = this->settings >> 28;
	this->Baseline = this->pos.y - (float)((this->settings & 0xFF) * 0.8);


	allocator.link(-1, GameHeaps[0], 0, 0x20);

	// Fire or Ice
	if (this->isFire == 1) {
		this->resFile.data = getResource("bubble", "g3d/t00.brres");
	}
	else {
		this->resFile.data = getResource("bubble", "g3d/t05.brres");
	}

	// Body and anms
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("bubble");
	nw4r::g3d::ResAnmChr shit = resFile.GetResAnmChr(isFire ? "RedFlee" : "BlueFlee");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	fleeAnimation.setup(mdl, shit, &allocator, 0);

	nw4r::g3d::ResAnmTexSrt anmRes = this->resFile.GetResAnmTexSrt("bubble");
	this->body.setup(mdl, anmRes, &this->allocator, 0, 1);
	this->body.bindEntry(&this->bodyModel, anmRes, 0, 0);
	this->bodyModel.bindAnim(&this->body, 0.0);


	// Fog up and anms
	mdl = this->resFile.GetResMdl("bubble_fog");
	fogModel.setup(mdl, &allocator, 0x124, 1, 0);

	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("bubble_fog");
	this->fogChr.setup(mdl, anmChr, &this->allocator, 0);
	this->fogChr.bind(&this->fogModel, anmChr, 1);
	this->fogModel.bindAnim(&this->fogChr, 0.0);
	this->fogChr.setUpdateRate(1.0);

	anmRes = this->resFile.GetResAnmTexSrt("bubble_fog");
	this->fogSRT.setup(mdl, anmRes, &this->allocator, 0, 1);
	this->fogSRT.bindEntry(&this->fogModel, anmRes, 0, 0);
	this->fogModel.bindAnim(&this->fogSRT, 0.0);


	// Fog down and anms
	/*mdl = this->resFile.GetResMdl("bubble_fog2");
	fog2Model.setup(mdl, &allocator, 0x124, 1, 0);

	anmChr = this->resFile.GetResAnmChr("bubble_fog2");
	this->fog2Chr.setup(mdl, anmChr, &this->allocator, 0);
	this->fog2Chr.bind(&this->fog2Model, anmChr, 1);
	this->fogModel.bindAnim(&this->fog2Chr, 0.0);
	this->fog2Chr.setUpdateRate(1.0);

	anmRes = this->resFile.GetResAnmTexSrt("bubble_fog");
	this->fog2SRT.setup(mdl, anmRes, &this->allocator, 0, 1);
	this->fog2SRT.bindEntry(&this->fog2Model, anmRes, 0, 0);
	this->fog2Model.bindAnim(&this->fog2SRT, 0.0);*/



	allocator.unlink();


	// Stuff I do understand
	this->scale = (Vec){1.0, 1.0, 1.0};


	this->direction = (this->isFire) ? 0 : 1;
	this->countdown = (this->isFire) ? 90 : 0;

	this->rot.x = 0; // X is vertical axis
	this->rot.y = (direction) ? 0xD800 : 0x2800;
	this->rot.z = 0; // Z is ... an axis >.>

	this->speed.x = 0.0;
	this->speed.y = 0.0;
	this->max_speed.x = 0.0;
	this->x_speed_inc = 0.0;

	this->goingUp = 0;



	ActivePhysics::Info HitMeBaby;

	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = 0.0;

	HitMeBaby.xDistToEdge = 40.0;
	HitMeBaby.yDistToEdge = 40.0;

	HitMeBaby.category1 = 0x3;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x4F;
	HitMeBaby.bitfield2 = (this->isFire) ? 0x380626 : 0x380626;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &poodleCollisionCallback;

	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();

	doStateChange(&StateID_Grow);

	this->onExecute();
	return true;
}

int daPodouble::onDelete() {
	return true;
}

int daPodouble::onExecute() {
	acState.execute();

	float diff = 8.57f * scale.x;
	float checkY = pos.y - diff, checkLastY = last_pos.y - diff;

	VEC2 myPos = {pos.x, checkY};
	VEC2 outBlockPos;
	float outFloat;
	s16 outAngle;
	int result = dWaterManager_c::instance->queryPosition(&myPos, &outBlockPos, &outFloat, &outAngle, currentLayerID);

	if ((!isFire && result == 0) || (isFire && result == 1)) {
		if ((checkLastY <= outFloat && outFloat < checkY) || (checkY <= outFloat && outFloat < checkLastY)) {
			VEC3 efPos = {pos.x, checkY, 6500.0f};
			VEC3 efScale = {scale.x * 0.7f, scale.y * 0.7f, scale.z * 0.7f};
			SpawnEffect(isFire ? "Wm_en_magmawave" : "Wm_en_waterwave_in", 0, &efPos, 0, &efScale);

			if (checkLastY <= outFloat && outFloat < checkY) {
				dBgGm_c::instance->makeSplash(pos.x, checkY, 0x11);
			} else {
				dBgGm_c::instance->makeSplash(pos.x, checkY, 0x10);
			}
		}
	}

	return true;
}

int daPodouble::onDraw() {
//	if (this->speed.y >= 0) {
		matrix.translation(pos.x, pos.y, pos.z);

		fogModel.setDrawMatrix(matrix);
		fogModel.setScale(&scale);
		fogModel.calcWorld(false);

		fogModel.scheduleForDrawing();
		fogModel._vf1C();
		fogChr.process();

		//if(this->fogChr.isAnimationDone())
		//	this->fogChr.setCurrentFrame(0.0);

		this->fogSRT.process();
/*	}
	else {
		matrix.translation(pos.x, pos.y, pos.z);

		fog2Model.setDrawMatrix(matrix);
		fog2Model.setScale(&scale);
		fog2Model.calcWorld(false);

		fog2Model.scheduleForDrawing();
		fog2Model._vf1C();

		if(this->fog2Chr.isAnimationDone())
			this->fog2Chr.setCurrentFrame(0.0);

		this->fog2SRT.process();
	}*/

	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);

	bodyModel.scheduleForDrawing();
	bodyModel._vf1C();
	this->body.process();
	if (acState.getCurrentState() == &StateID_SyncDie)
		fleeAnimation.process();

	return true;
}


///////////////
// Grow State
///////////////
	void daPodouble::beginState_Grow() {
		this->timer = 0;
		if (isFire) { return; }

		SetupKameck(this, Kameck);
		this->scale = (Vec){1.0, 1.0, 1.0};
	}

	void daPodouble::executeState_Grow() {

		this->timer += 1;

		if (isFire) {
			float scaleSpeed;

			if (timer == 150) { PlaySound(this, SE_BOSS_IGGY_WANWAN_TO_L); }

			if ((timer > 150) && (timer < 230)) {
				scaleSpeed = (3.5 - 1.0) / 80.0;

				float modifier;

				modifier = 1.0 + ((timer - 150) * scaleSpeed);

				this->scale = (Vec){modifier, modifier, modifier};
				this->pos.y = this->pos.y + (18/80.0);
			}

			if (timer > 420) {
				PlaySound(this, SE_EMY_CS_MOVE_W8_BUBBLE_APP);
				doStateChange(&StateID_Bounce);
			}

			return;
		}

		bool ret;
		ret = GrowBoss(this, Kameck, 1.0, 3.5, 18, this->timer);

		if (this->timer == 382) {

			int players = GetActivePlayerCount();

			Vec tempPos = (Vec){this->pos.x - 190.0f, this->pos.y + 120.0f, 3564.0};
			S16Vec nullRot = {0,0,0};
			dStageActor_c *spawner = create(AC_YOSHI_EGG, 0x71010000, &tempPos, &nullRot, 0);
			spawner->speed.x = spawner->speed.x / 2.0;

			if (players > 1) {
				tempPos = (Vec){this->pos.x - 190.0f, this->pos.y + 120.0f, 3564.0f};
				S16Vec nullRot = {0,0,0};
				spawner = create(AC_YOSHI_EGG, 0xF1010000, &tempPos, &nullRot, 0);
				spawner->speed.x = spawner->speed.x / 2.0;
			}

			if (players > 2) {
				tempPos = (Vec){this->pos.x - 190.0f, this->pos.y + 120.0f, 3564.0f};
				S16Vec nullRot = {0,0,0};
				spawner->create(AC_YOSHI_EGG, 0x71010000, &tempPos, &nullRot, 0);
			}

			if (players > 3) {
				tempPos = (Vec){this->pos.x - 190.0f, this->pos.y + 120.0f, 3564.0f};
				S16Vec nullRot = {0,0,0};
				create(AC_YOSHI_EGG, 0xF1010000, &tempPos, &nullRot, 0);
			}
		}

		if (ret) {
			PlaySound(this, SE_EMY_CS_MOVE_W8_BUBBLE_APP);
			doStateChange(&StateID_Bounce);
		}
	}
	void daPodouble::endState_Grow() {
		this->Baseline = this->pos.y;
		if (isFire) { return; }

		CleanupKameck(this, Kameck);
	}


///////////////
// Bounce State
///////////////
	void daPodouble::beginState_Bounce() {
		this->rot.y = (direction) ? 0xD800 : 0x2800;
		this->rot.x = 0;

		this->max_speed.y = -5.0;
		this->speed.y = -1.0;
		this->y_speed_inc = -0.1875;

		this->goingUp = 0;
	}
	void daPodouble::executeState_Bounce() {

		if (this->countdown) {
			this->countdown--;
			return; }

		HandleYSpeed();
		doSpriteMovement();

		if (this->pos.y < this->Baseline) {
			this->speed.y = 7.5;
			this->goingUp = 1; }

		if (-0.1 < this->speed.y < 0.1) {
			if (this->goingUp == 1) { doStateChange(&StateID_Spit); } }


		// Check for stupid liquid junk
		Vec2 checkWhere = {this->pos.x, this->pos.y};
		checkLiquidImmersion(&checkWhere, 3.0f);

	}
	void daPodouble::endState_Bounce() {
		this->speed.y = 0.0;
		this->y_speed_inc = 0.0;
	}


///////////////
// Spit State
///////////////
	void daPodouble::beginState_Spit() { this->timer = 0; }
	void daPodouble::executeState_Spit() {

		if (this->timer < 0x40) {
			this->rot.x -= 0x80;
		}

		else if (this->timer < 0x48) {
			this->rot.x += 0x400;
		}

		else if (this->timer == 0x48) {
			if (this->isFire == 0) {
				S16Vec nullRot = {0,0,0};
				dStageActor_c *spawner = create(BROS_ICEBALL, 0x10, &this->pos, &nullRot, 0);
				*((u32 *) (((char *) spawner) + 0x3DC)) = this->id;

				int num;
				num = GenerateRandomNumber(20) - 10;
				float modifier = (float)(num) / 10.0;

				spawner->max_speed.y += modifier;
				spawner->speed.y += modifier;

				num = GenerateRandomNumber(20) - 10;
				modifier = (float)(num) / 10.0;

				spawner->max_speed.x += modifier;
				spawner->speed.x += modifier;


				PlaySoundAsync(this, SE_EMY_ICE_BROS_ICE);
				doStateChange(&StateID_Bounce);
			}
			else {
				S16Vec nullRot = {0,0,0};
				dStageActor_c *spawner = create(BROS_FIREBALL, 0, &this->pos, &nullRot, 0);

				int num;
				num = GenerateRandomNumber(20);
				float modifier = (float)(num) / 5.0;

				spawner->max_speed.y += modifier;
				spawner->speed.y += modifier;

				num = GenerateRandomNumber(20);
				modifier = (float)(num) / 10.0;

				spawner->max_speed.x += modifier;
				spawner->speed.x += modifier;

				PlaySoundAsync(this, SE_EMY_FIRE_BROS_FIRE);
				doStateChange(&StateID_Bounce);
			}
		}

		this->timer += 1;
	}
	void daPodouble::endState_Spit() {
		this->speed.y = -1.0;
		this->y_speed_inc = -0.1875;
	}


///////////////
// Damage State
///////////////
	void daPodouble::beginState_Damage() {
		this->timer = 0;
		this->isInvulnerable = 1;

		if (this->isFire == 0) {
			PlaySoundAsync(this, SE_OBJ_PNGN_ICE_THAW);
			S16Vec nullRot = {0,0,0};
			Vec twoVec = {2.0f, 2.0f, 2.0f};
			SpawnEffect("Wm_ob_iceevaporate", 0, &this->pos, &nullRot, &twoVec); }
		else {
			PlaySoundAsync(this, SE_EMY_FIRE_SNAKE_EXTINCT);
			S16Vec nullRot = {0,0,0};
			Vec threeVec = {3.0f, 3.0f, 3.0f};
			SpawnEffect("Wm_en_firesnk_icehitsmk_b", 0, &this->pos, &nullRot, &threeVec); }
	}
	void daPodouble::executeState_Damage() {

		int amt;
		amt = sin(this->timer * 3.14 / 4.0) * 0x2000;

		this->rot.y = amt;
		this->rot.y += (direction) ? 0xD800 : 0x2800;

		float targetScale = 3.5f - (damage * 0.175f);
		if (scale.x > targetScale) {
			float newScale = scale.x - 0.006f;
			if (newScale < targetScale)
				newScale = targetScale;
			scale.x = scale.y = scale.z = newScale;
			updateHitboxSize();
		}

		if (this->timer > 180) {
			doStateChange(&StateID_Bounce);
		}

		if ((this->timer == 60) || (this->timer == 80) || (this->timer == 100)) {
			if (this->isFire == 0) {
				S16Vec nullRot = {0,0,0};
				dStageActor_c *spawner = create(BROS_ICEBALL, 0x10, &this->pos, &nullRot, 0);
				*((u32 *) (((char *) spawner) + 0x3DC)) = this->id;
				PlaySoundAsync(this, SE_EMY_ICE_BROS_ICE);
			}
			else {
				S16Vec nullRot = {0,0,0};
				create(BROS_FIREBALL, 0, &this->pos, &nullRot, 0);
				PlaySoundAsync(this, SE_EMY_FIRE_BROS_FIRE);
			}
		}

		this->timer += 1;

	}
	void daPodouble::endState_Damage() {
		this->isInvulnerable = 0;
	}


///////////////
// Outro State
///////////////
	void daPodouble::beginState_Outro() {

		daPodouble *other = (daPodouble*)FindActorByType(SHIP_WINDOW, 0);
			if (other->id == this->id) {
				other = (daPodouble*)FindActorByType(SHIP_WINDOW, (Actor*)this);
			}
		other->doStateChange(&StateID_SyncDie);

		for (int i = 0; i < 4; i++) {
			dAcPy_c *player = (dAcPy_c*)GetSpecificPlayerActor(i);
			if (player && player->getYoshi()) {
				player->input.setFlag(dPlayerInput_c::NO_SHAKING);
			}
		}

		OutroSetup(this);
		this->timer = 0;

	}
	void daPodouble::executeState_Outro() {

		if (this->dying == 1) {
			if (this->timer > 180) { ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE); }
			if (this->timer == 60) { PlayerVictoryCries(this); }

			this->timer += 1;
			return;
		}

		bool ret;
		ret = ShrinkBoss(this, &this->pos, 3.5, this->timer);

		if (ret == true) {
			BossExplode(this, &this->pos);
			this->dying = 1;
			this->timer = 0;
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_EMY_BUBBLE_EXTINCT, 1);
		}

		this->timer += 1;

	}
	void daPodouble::endState_Outro() { }



///////////////
// SyncDie State
///////////////
	void daPodouble::beginState_SyncDie() {

		this->removeMyActivePhysics();
		this->timer = 0;

		nw4r::g3d::ResAnmChr anmChr = resFile.GetResAnmChr(isFire ? "RedFlee" : "BlueFlee");

		fleeAnimation.bind(&bodyModel, anmChr, 1);
		bodyModel.bindAnim(&fleeAnimation, 0.0);
		fleeAnimation.setUpdateRate(0.5f);

		fogChr.bind(&fogModel, anmChr, 1);
		fogModel.bindAnim(&fogChr, 0.0f);
		fogChr.setUpdateRate(0.5f);

		PlaySound(this, SE_EMY_GABON_ROCK_THROW);
	}
	void daPodouble::executeState_SyncDie() {
		if (fleeAnimation.isAnimationDone())
			Delete(0);
	}
	void daPodouble::endState_SyncDie() { }
