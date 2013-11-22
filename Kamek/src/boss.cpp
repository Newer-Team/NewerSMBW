#include "boss.h"



void DamagePlayer(dEn_c *actor, ActivePhysics *apThis, ActivePhysics *apOther) {

	actor->dEn_c::playerCollision(apThis, apOther);
	actor->_vf220(apOther->owner);

	// fix multiple player collisions via megazig
	actor->deathInfo.isDead = 0;
	actor->flags_4FC |= (1<<(31-7));
	if (apOther->owner->which_player == 255 ) {
		actor->counter_504[0] = 0;
		actor->counter_504[1] = 0;
		actor->counter_504[2] = 0;
		actor->counter_504[3] = 0;
	}
	else {
		actor->counter_504[apOther->owner->which_player] = 0;
	}
}


void SetupKameck(daBoss *actor, daKameckDemo *Kameck) {

	// Stop the BGM Music
	StopBGMMusic();

	// Set the necessary Flags and make Mario enter Demo Mode
	dStage32C_c::instance->freezeMarioBossFlag = 1;
	WLClass::instance->_4 = 4;
	WLClass::instance->_8 = 0;

	MakeMarioEnterDemoMode();

	// Make sure to use the correct position
	Vec pos = (Vec){actor->pos.x - 124.0f, actor->pos.y + 104.0f, 3564.0f};
	S16Vec rot = (S16Vec){0, 0, 0};

	// Create And use Kameck
	actor->Kameck = (daKameckDemo*)actor->createChild(KAMECK_FOR_CASTLE_DEMO, (dStageActor_c*)actor, 0, &pos, &rot, 0);
	actor->Kameck->doStateChange(&daKameckDemo::StateID_DemoWait);

}


void CleanupKameck(daBoss *actor, daKameckDemo *Kameck) {
	// Clean up the flags and Kameck
	dStage32C_c::instance->freezeMarioBossFlag = 0;
	WLClass::instance->_8 = 1;

	MakeMarioExitDemoMode();
	StartBGMMusic();

	actor->Kameck->Delete(1);
}


bool GrowBoss(daBoss *actor, daKameckDemo *Kameck, float initialScale, float endScale, float yPosModifier, int timer) {
	if (timer == 130) { actor->Kameck->doStateChange(&daKameckDemo::StateID_DemoSt); }
	if (timer == 400) { actor->Kameck->doStateChange(&daKameckDemo::StateID_DemoSt2); }

	float scaleSpeed, yPosScaling;

	if (timer == 150) { PlaySound(actor, SE_BOSS_IGGY_WANWAN_TO_L);  }

	if ((timer > 150) && (timer < 230)) {
		scaleSpeed = (endScale -initialScale) / 80.0;

		float modifier;

		modifier = initialScale + ((timer - 150) * scaleSpeed);

		actor->scale = (Vec){modifier, modifier, modifier};
		actor->pos.y = actor->pos.y + (yPosModifier/80.0);
	}

	if (timer == 360) {
		Vec tempPos = (Vec){actor->pos.x - 40.0f, actor->pos.y + 120.0f, 3564.0f};
		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_ob_greencoinkira", 0, &tempPos, &nullRot, &oneVec);
		SpawnEffect("Wm_mr_yoshiicehit_a", 0, &tempPos, &nullRot, &oneVec);
		SpawnEffect("Wm_mr_yoshiicehit_b", 0, &tempPos, &nullRot, &oneVec);
		SpawnEffect("Wm_ob_redringget", 0, &tempPos, &nullRot, &oneVec);
		SpawnEffect("Wm_ob_keyget01", 0, &tempPos, &nullRot, &oneVec);
		SpawnEffect("Wm_ob_greencoinkira_a", 0, &tempPos, &nullRot, &oneVec);
		SpawnEffect("Wm_ob_keyget01_c", 0, &tempPos, &nullRot, &oneVec);
	}

	if (timer > 420) { return true; }
	return false;
}


void OutroSetup(daBoss *actor) {
	actor->removeMyActivePhysics();

	StopBGMMusic();

	WLClass::instance->_4 = 5;
	WLClass::instance->_8 = 0;
	dStage32C_c::instance->freezeMarioBossFlag = 1;

	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_BOSS_CMN_DAMAGE_LAST, 1);
}


bool ShrinkBoss(daBoss *actor, Vec *pos, float scale, int timer) {
	// Adjust actor to equal the scale of your boss / 80.
	actor->scale.x -= scale / 80.0;
	actor->scale.y -= scale / 80.0;
	actor->scale.z -= scale / 80.0;

	// actor->pos.y += 2.0;

	if (timer == 30) {
		S16Vec nullRot = {0,0,0};
		Vec twoVec = {2.0f, 2.0f, 2.0f};
		SpawnEffect("Wm_ob_starcoinget_gl", 0, pos, &nullRot, &twoVec);
		SpawnEffect("Wm_mr_vshipattack_hosi", 0, pos, &nullRot, &twoVec);
		SpawnEffect("Wm_ob_keyget01_b", 0, pos, &nullRot, &twoVec);
	}

	if (actor->scale.x < 0) { return true; }
	else { return false; }
}


void BossExplode(daBoss *actor, Vec *pos) {
	actor->scale.x = 0.0;
	actor->scale.y = 0.0;
	actor->scale.z = 0.0;

	S16Vec nullRot = {0,0,0};
	Vec twoVec = {2.0f, 2.0f, 2.0f};
	SpawnEffect("Wm_ob_keyget02", 0, pos, &nullRot, &twoVec);
	actor->dying = 1;
	actor->timer = 0;

	nw4r::snd::SoundHandle handle;
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, STRM_BGM_SHIRO_BOSS_CLEAR, 1);

	//MakeMarioEnterDemoMode();
	BossGoalForAllPlayers();
}

void BossGoalForAllPlayers() {
	for (int i = 0; i < 4; i++) {
		daPlBase_c *player = GetPlayerOrYoshi(i);
		if (player)
			player->setAnimePlayStandardType(2);
	}
}


void PlayerVictoryCries(daBoss *actor) {
	UpdateGameMgr();
	/*nw4r::snd::SoundHandle handle1, handle2, handle3, handle4;

	dAcPy_c *players[4];
	for (int i = 0; i < 4; i++)
		players[i] = (dAcPy_c *)GetSpecificPlayerActor(i);

	if (players[0] && strcmp(players[0]->states2.getCurrentState()->getName(), "dAcPy_c::StateID_Balloon"))
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle1, SE_VOC_MA_CLEAR_BOSS, 1);
	if (players[1] && strcmp(players[1]->states2.getCurrentState()->getName(), "dAcPy_c::StateID_Balloon"))
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle2, SE_VOC_LU_CLEAR_BOSS, 1);
	if (players[2] && strcmp(players[2]->states2.getCurrentState()->getName(), "dAcPy_c::StateID_Balloon"))
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle3, SE_VOC_KO_CLEAR_BOSS, 1);
	if (players[3] && strcmp(players[3]->states2.getCurrentState()->getName(), "dAcPy_c::StateID_Balloon"))
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle4, SE_VOC_KO2_CLEAR_BOSS, 1);*/
}
