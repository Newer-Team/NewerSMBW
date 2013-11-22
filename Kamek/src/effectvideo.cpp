#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>


extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);


class EffectVideo : public dEn_c {
	int onCreate();
	int onExecute();
	int onDelete();

	u64 eventFlag;
	s32 timer;
	u32 delay;

	u32 effect;
	u8 type;
	float scale;

	static EffectVideo *build();

};


EffectVideo *EffectVideo::build() {
	void *buffer = AllocFromGameHeap1(sizeof(EffectVideo));
	return new(buffer) EffectVideo;
}


int EffectVideo::onCreate() {
	
	this->timer = 0;

	char eventNum	= (this->settings >> 24) & 0xFF;

	this->eventFlag = (u64)1 << (eventNum - 1);
	
	this->type		= (this->settings >> 16) & 0xF;
	this->effect	= this->settings & 0xFFF;
	this->scale		= float((this->settings >> 20) & 0xF) / 4.0;
	this->delay		= (this->settings >> 12) & 0xF * 30;
	
	if (this->scale == 0.0) { this->scale = 1.0; }

	this->onExecute();
	return true;
}


int EffectVideo::onDelete() {
	return true;
}


int EffectVideo::onExecute() {

	if (dFlagMgr_c::instance->flags & this->eventFlag) {

		if (this->timer == this->delay) {

			if (this->type == 0) { // Plays a sound
				PlaySoundAsync(this, this->effect);
			}
	
			else {	// Plays an Effect

				const char *efName = 0;

				switch (this->effect) {
					case 1: efName = "Wm_mr_cmnsndlandsmk"; break;
					case 3: efName = "Wm_en_landsmoke"; break;
					case 5: efName = "Wm_en_sndlandsmk"; break;
					case 6: efName = "Wm_en_sndlandsmk_s"; break;
					case 43: efName = "Wm_ob_cmnspark"; break;
					case 159: efName = "Wm_mr_hardhit_grain"; break;
					case 177: efName = "Wm_ob_greencoinkira_b"; break;
					case 193: efName = "Wm_mr_electricshock_biri01_s"; break;
					case 216: efName = "Wm_en_blockcloud"; break;
					case 365: efName = "Wm_en_kuribobigsplit"; break;
					case 514: efName = "Wm_ob_fireworks_y"; break;
					case 517: efName = "Wm_ob_fireworks_b"; break;
					case 520: efName = "Wm_ob_fireworks_g"; break;
					case 523: efName = "Wm_ob_fireworks_p"; break;
					case 526: efName = "Wm_ob_fireworks_k"; break;
					case 533: efName = "Wm_ob_fireworks_1up"; break;
					case 540: efName = "Wm_ob_fireworks_star"; break;
					case 843: efName = "Wm_en_explosion_smk"; break;
					case 864: efName = "Wm_en_obakedoor_sm"; break;
					case 865: efName = "Wm_en_obakedoor_ic"; break;
				}

				if (efName != 0) {
					S16Vec nullRot = {0,0,0};
					Vec efScale = {scale,scale,scale};
					SpawnEffect(efName, 0, &this->pos, &nullRot, &efScale);
				}
			}
	
			this->timer = 0;
			if (this->delay == 0) { this->delay = -1; }
		}
		
		this->timer += 1;
	}
	return true;
}

