#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <playerAnim.h>
#include <stage.h>

extern "C" void *MakeMarioEnterDemoMode();
extern "C" void *MakeMarioExitDemoMode();
extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);
extern "C" void *StopBGMMusic();
extern "C" void *StartBGMMusic();
// extern "C" void *SoundRelatedClass;

// THIS IS A DUMB NAME
extern bool NoMichaelBuble;


// Controls the tempo for the songs.
// Lower numbers are faster, by a ratio factor.

int Tempo[16] = {13,11,12,12,
				 8,8,7,15,
				 15,8,10,16,
				 12,10,8,8};

int Songs[16][4][16][3] = {

	// 15

 	// First number is the block, Second is the note/sfx, Third is timing: 30 is one quarter note at 120 bpm, 0,0 ends the sequence

	// Some possibles
	// SMW Donut Plains
	// SMW Forest of Illusion
	// Paper Mario Battle Theme
	// Thwomp Volcano: PiT
	// SMRPG Forest?
	// SMRPG Nimbus Land?
	// Teehee Valley: Superstar Saga
	// SML Theme
	// Mario Sunshine Main Theme
	// Mario Kart DS Theme

	{ // Song 1 - Super Mario Bros Melody |***  |
		{{3,17,0},{3,17,2},{3,17,6},{1,13,10},{3,17,12},{5,20,16},{0,0,0}},
		{{1,13,0},{5,8,3},{3,5,6},{6,10,9},{7,12,11},{6,11,13},{6,10,14},{0,0,0}},
		{{1,13,0},{1,13,1},{1,13,3},{1,13,5},{2,15,6},{3,17,8},{1,13,9},{7,10,11},{6,8,12},{0,0,0}},
		{{1,13,0},{1,13,1},{1,13,3},{1,13,5},{2,15,6},{3,17,7},{0,0,0}}
	},

	{ // Song 2 - Super Mario Bros. Underground |**** |
		{{7,12,0},{7,24,2},{5,9,4},{5,21,6},{6,10,8},{6,22,10},{0,0,0}},
		{{3,5,0},{3,17,2},{1,2,4},{1,14,6},{2,3,8},{2,15,10},{0,0,0}},
		{{2,15,0},{1,14,1},{1,13,2},{7,12,3},{2,15,5},{1,14,7},{5,8,9},{4,7,11},{1,13,13},{0,0,0}},
		{{7,12,0},{4,18,1},{3,17,2},{2,16,3},{6,22,4},{5,21,5},{4,20,6},{2,15,10},{6,11,12},{6,10,14},{5,9,16},{5,8,18},{0,0,0}}
	},

	{ // Song 3 - SMB Starman |**   |
		{{4,6,0},{4,6,2},{4,6,4},{2,3,6},{4,6,7},{4,6,9},{2,3,11},{4,6,12},{2,3,13},{4,6,14},{0,0,0}},
		{{3,5,0},{3,5,2},{3,5,4},{1,1,6},{3,5,7},{3,5,9},{1,1,11},{3,5,12},{1,1,13},{3,5,14},{0,0,0}},
		{{4,18,0},{4,18,2},{4,18,4},{2,15,6},{4,18,7},{4,18,9},{2,15,11},{4,18,12},{2,15,13},{4,18,14},{0,0,0}},
		{{3,17,0},{3,17,2},{3,17,4},{1,13,6},{3,17,7},{3,17,9},{1,13,11},{3,17,12},{1,13,13},{3,17,14},{0,0,0}}
	},

	{ // Song 4 - SMB3 Clouds |*    |
		{{1,8,0},{6,20,4},{1,8,12},{2,9,16},{7,21,20},{0,0,0}},
		{{6,23,0},{5,20,4},{1,8,8},{3,17,16},{4,18,20},{1,14,24},{6,11,32},{0,0,0}},
		{{1,8,0},{6,20,4},{1,8,12},{2,9,16},{7,21,20},{0,0,0}},
		{{6,23,0},{5,20,4},{1,8,8},{3,17,16},{4,18,20},{1,14,24},{6,11,32},{0,0,0}}
	},

	{ // Song 5 - SMB3 Hammer Bros |*****|
		{{2,16,0},{3,17,2},{2,16,4},{3,17,6},{2,15,8},{1,13,10},{6,10,12},{1,13,14},{0,0,0}},
		{{2,16,0},{3,17,2},{2,16,4},{3,17,6},{2,15,8},{1,13,10},{6,10,12},{1,13,14},{1,13,18},{3,16,20},{2,15,21},{1,13,22},{6,10,26},{1,13,28},{0,0,0}},
		{{4,17,0},{4,17,2},{5,19,4},{6,20,6},{7,21,8},{4,17,10},{4,17,14},{3,15,16},{3,15,18},{2,14,20},{3,15,22},{0,0,0}},
		{{2,16,0},{3,17,2},{2,16,4},{3,17,6},{2,15,8},{1,13,10},{6,10,12},{1,13,14},{5,17,18},{5,17,20},{5,17,22},{5,17,24},{5,17,26},{5,17,28},{5,17,30},{0,0,0}}
	},

	{ // Song 6 - SMB3 Underwater |***  |
		{{1,8,0},{3,13,2},{6,20,4},{5,19,6},{7,22,10},{0,0,0}},
		{{5,19,14},{6,20,16},{4,17,18},{3,13,20},{1,8,22},{2,10,26},{0,0,0}},
		{{1,7,0},{2,8,2},{4,13,4},{6,20,6},{5,19,8},{7,22,12},{0,0,0}},
		{{5,19,16},{6,20,18},{0,0,0}}

		// Fuckin' too hard
		// {{1,8,0},{3,13,2},{6,20,4},{5,19,6},{7,22,10},{5,19,14},{6,20,16},{4,17,18},{3,13,20},{1,8,22},{2,10,26},{0,0,0}},
		// {{1,7,0},{2,8,2},{4,13,4},{6,20,6},{5,19,8},{7,22,12},{5,19,16},{6,20,18},{0,0,0}},
		// {{2,10,0},{3,13,2},{6,22,4},{5,21,6},{7,24,10},{6,22,14},{4,20,16},{1,8,18},{6,22,20},{4,20,22},{0,0,0}},
		// {{1,8,0},{2,11,2},{5,20,4},{4,18,6},{5,20,10},{3,16,14},{6,22,16},{5,20,24},{0,0,0}}
	},

	{ // Song 7 - SMW Castle |*    |
		{{5,9,0},{4,6,8},{1,1,12},{2,3,16},{4,6,20},{0,0,0}},
		{{1,1,0},{4,6,4},{7,13,8},{5,9,12},{4,8,20},{0,0,0}},
		{{3,16,0},{2,15,4},{1,13,8},{2,15,12},{1,10,20},{0,0,0}},
		{{2,15,0},{1,13,4},{2,15,8},{3,16,12},{1,15,20},{0,0,0}}
	},

	{ // Song 8 - SMW Theme |**   |
		{{6,10,0},{4,6,4},{1,1,7},{2,3,8},{4,6,9},{4,6,11},{0,0,0}},
		{{2,3,0},{1,1,1},{4,6,3},{4,6,5},{7,13,7},{6,10,9},{5,8,12},{0,0,0}},
		{{6,10,0},{4,6,4},{1,1,7},{2,3,8},{4,6,9},{4,6,11},{5,8,15},{6,10,16},{4,6,17},{1,1,18},{2,3,20},{4,6,23},{0,0,0}},
		{{1,13,0},{2,15,2},{1,13,4},{2,15,6},{1,13,8},{1,1,11},{7,11,12},{6,10,13},{5,8,14},{4,6,16},{0,0,0}}
	},

	{ // Song 9 - Yoshi Story Theme |*    |
		{{5,17,0},{6,19,4},{5,17,5},{6,19,6},{5,17,8},{0,0,0}},
		{{3,14,0},{4,15,4},{3,14,5},{4,15,6},{3,14,8},{0,0,0}},
		{{5,17,0},{6,19,4},{5,17,5},{6,19,6},{5,17,8},{0,0,0}},
		{{4,15,0},{3,14,1},{4,15,2},{3,14,4},{2,12,8},{1,10,9},{2,12,10},{1,10,12},{0,0,0}}
	},

	{ // Song 10 - SM64 Peaches Castle |***  |
		{{4,8,0},{5,10,6},{4,8,10},{3,7,14},{4,8,16},{6,13,18},{7,17,20},{0,0,0}},
		{{3,6,0},{4,8,6},{3,6,10},{2,5,14},{3,6,16},{5,12,18},{6,15,20},{0,0,0}},
		{{2,5,0},{3,6,6},{2,5,10},{1,4,14},{2,5,16},{4,10,18},{5,13,20},{0,0,0}},
		{{4,10,2},{5,12,4},{6,13,6},{7,15,8},{6,13,10},{7,15,12},{4,10,14},{5,12,18},{4,10,20},{5,12,22},{6,13,24},{6,13,28},{6,13,34},{6,13,36},{6,13,38},{0,0,0}}
	},

	{ // Song 11 - SM64 Koopa Road |***  |
		{{3,6,0},{1,1,10},{3,6,12},{7,16,14},{6,15,16},{5,11,18},{3,6,20},{0,0,0}},
		{{3,6,0},{2,4,6},{3,6,10},{5,11,12},{4,9,14},{5,11,16},{0,0,0}},
		{{3,6,0},{1,1,10},{3,6,12},{7,16,14},{6,15,16},{5,11,18},{3,6,20},{0,0,0}},
		{{1,1,0},{3,6,2},{7,18,4},{6,16,6},{5,15,10},{4,13,20},{5,15,22},{0,0,0}}
	},

	{ // Song 12 - Frappe Snowland |***  |
		{{1,14,2},{2,15,3},{3,17,4},{6,22,5},{3,17,7},{0,0,0}},
		{{2,15,0},{1,14,2},{2,15,4},{1,14,5},{6,10,7},{3,5,9},{4,7,10},{3,5,11},{0,0,0}},
		{{1,14,2},{2,15,3},{3,17,4},{6,22,5},{3,17,7},{0,0,0}},
		{{2,15,0},{1,14,2},{2,15,4},{1,14,5},{6,10,7},{6,10,9},{7,12,10},{6,10,11},{0,0,0}}
	},

	{ // Song 13 - Ghost Luigi Mansion theme |**** |
		{{7,17,2},{7,17,4},{7,17,6},{7,17,8},{4,13,12},{7,17,14},{6,16,16},{3,12,20},{0,0,0}},
		{{5,15,2},{5,15,4},{5,15,6},{5,15,8},{3,12,12},{5,15,14},{4,13,16},{2,11,22},{3,12,24},{1,5,28},{0,0,0}},
		{{7,17,2},{7,17,4},{7,17,6},{7,17,8},{4,13,12},{7,17,14},{6,16,16},{3,12,20},{0,0,0}},
		{{5,15,2},{5,15,4},{5,15,6},{5,15,8},{3,12,12},{5,15,14},{7,17,16},{5,15,18},{4,13,20},{3,12,22},{1,10,24},{0,0,0}}
	},

	{ // Song 14 - Desert |**   |
		{{6,9,0},{4,6,4},{3,5,12},{3,5,16},{4,6,18},{0,0,0}},
		{{6,9,0},{4,6,4},{3,5,12},{3,5,16},{4,6,18},{0,0,0}},
		{{4,13,0},{3,12,20},{5,14,22},{6,17,24},{5,14,26},{4,13,32},{0,0,0}},
		{{1,9,0},{2,11,2},{3,12,4},{5,14,8},{4,13,12},{0,0,0}}
	},

	{ // Song 15 - Gusty Garden Galaxy |*****|
		{{5,14,0},{4,13,10},{6,16,12},{5,14,14},{3,9,16},{1,6,26},{2,7,28},{3,9,30},{3,9,32},{2,7,38},{0,0,0}},
		{{6,16,0},{5,15,10},{7,18,12},{6,16,14},{4,14,16},{3,13,22},{2,11,26},{3,13,30},{2,11,32},{1,9,38},{0,0,0}},
		{{6,21,0},{5,20,10},{7,23,12},{6,21,14},{4,19,16},{3,18,22},{4,19,32},{3,18,42},{6,21,44},{4,19,46},{3,18,48},{1,16,54},{0,0,0}},
		{{5,18,0},{4,17,10},{6,19,12},{5,18,14},{4,16,16},{3,15,22},{5,18,26},{4,16,30},{2,14,32},{1,13,38},{2,14,44},{4,16,48},{0,0,0}}
	},

	{ // Song 16 - Overworld or Cave
		{{0,0,0}},
		{{0,0,0}},
		{{0,0,0}},
		{{0,0,0}}
	}
};

const char* Prizes[10][4] = {
	{ "I_kinoko", 		"g3d/I_kinoko.brres", 			"I_kinoko", 			"wait2" },
	{ "I_fireflower", 	"g3d/I_fireflower.brres", 		"I_fireflower", 		"wait2" },
	{ "I_propeller", 	"g3d/I_propeller.brres",	 	"I_propeller_model", 	"wait2" },
	{ "I_iceflower", 	"g3d/I_iceflower.brres", 		"I_iceflower", 			"wait2" },
	{ "I_penguin", 		"g3d/I_penguin.brres", 			"I_penguin", 			"wait2" },
	{ "I_kinoko_bundle","g3d/I_mini_kinoko.brres", 		"I_mini_kinoko", 		"wait2" },
	{ "I_star", 		"g3d/I_star.brres", 			"I_star", 				"wait2" },
	{ "I_hammer", 		"g3d/I_fireflower.brres", 		"I_fireflower", 			"wait2" },
	{ "I_kinoko_bundle","g3d/I_life_kinoko.brres", 		"I_life_kinoko", 		"wait2" },
	{ "obj_coin", 		"g3d/obj_coin.brres", 			"obj_coin", 			"wait2" }
};

const char* Notes[24] = {
	"sfx/3C",
	"sfx/3C#",
	"sfx/3D",
	"sfx/3D#",
	"sfx/3E",
	"sfx/3F",
	"sfx/3F#",
	"sfx/3G",
	"sfx/3G#",
	"sfx/3A",
	"sfx/3A#",
	"sfx/3B",
	"sfx/4C",
	"sfx/4C#",
	"sfx/4D",
	"sfx/4D#",
	"sfx/4E",
	"sfx/4F",
	"sfx/4F#",
	"sfx/4G",
	"sfx/4G#",
	"sfx/4A",
	"sfx/4A#",
	"sfx/4B"
};

const char* SAarcNameList [] = {
	"obj_coin",
	"I_hammer",
	"I_star",
	"block_light",
	"block_light_color",
	"I_kinoko_bundle",
	NULL
};



/*****************************************************************************/
// Playing frickin' sounds

extern "C" void PlaySoundWithFunctionB4(void *spc, nw4r::snd::SoundHandle *handle, int id, int unk);
static nw4r::snd::StrmSoundHandle handle;

u8 hijackMusicWithSongName(const char *songName, int themeID, bool hasFast, int channelCount, int trackCount, int *wantRealStreamID);

void BonusMusicPlayer(int id) {
	if (handle.Exists())
		handle.Stop(0);

	int sID;
	hijackMusicWithSongName(Notes[id], -1, false, 2, 1, &sID);
	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, sID, 1);
}



/*****************************************************************************/
// The Prize Model

class dSongPrize: public dEn_c {
public:
	int onCreate();
	int onDelete();
	int onExecute();
	int beforeDraw();
	int onDraw();

	void doSpin();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	m3d::mdl_c bodyModel;
	m3d::anmChr_c aw;

	int queue;
	int p;
	int timer;

	HermiteKey keysX[0x10];
	unsigned int Xkey_count;
	HermiteKey keysY[0x10];
	unsigned int Ykey_count;
	HermiteKey keysS[0x10];
	unsigned int Skey_count;

	USING_STATES(dSongPrize);
	DECLARE_STATE(Wait);
	DECLARE_STATE(Shrink);
	DECLARE_STATE(Spin);

	static dSongPrize *build();
};

CREATE_STATE(dSongPrize, Wait);
CREATE_STATE(dSongPrize, Shrink);
CREATE_STATE(dSongPrize, Spin);

dSongPrize *dSongPrize::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dSongPrize));
	return new(buffer) dSongPrize;
}

int dSongPrize::onCreate() {

	// Settings
	queue = this->settings & 0xF;
	int prize = this->settings >> 16;
	scale = (Vec){ 3.0, 3.0, 3.0 };

	p = prize;


	// Model creation
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	resFile.data = getResource(Prizes[p][0], Prizes[p][1]);
	nw4r::g3d::ResMdl mdl = resFile.GetResMdl(Prizes[p][2]);
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Item(&bodyModel, 0); // 800B42B0


	// Animation Assignment
	nw4r::g3d::ResAnmChr anmChr = resFile.GetResAnmChr("wait2");
	aw.setup(mdl, anmChr, &allocator, 0);
	aw.bind(&bodyModel, anmChr, 1);
	bodyModel.bindAnim(&aw, 0.0);
	aw.setUpdateRate(1.0);
	allocator.unlink();


	// Change State
	doStateChange(&dSongPrize::StateID_Wait);

	return true;
}

int dSongPrize::onDelete() {
	return true;
}

int dSongPrize::onExecute() {
	acState.execute();
	return true;
}

int dSongPrize::beforeDraw() { return 1; }
int dSongPrize::onDraw() {

	if (p == 9) {
		matrix.translation(pos.x, pos.y + (8.0 * scale.y), pos.z);
	}
	else {
		matrix.translation(pos.x, pos.y, pos.z);
	}

	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);

	bodyModel.calcWorld(false);


	bodyModel.scheduleForDrawing();
	bodyModel._vf1C();

	if(this->aw.isAnimationDone())
		this->aw.setCurrentFrame(0.0);

	return true;
}

void dSongPrize::beginState_Wait() {}
void dSongPrize::executeState_Wait() {}
void dSongPrize::endState_Wait() {}

void dSongPrize::beginState_Shrink() {
	this->timer = 0;

	Xkey_count = 2;
	Ykey_count = 2;
	Skey_count = 2;

	// /* keysX[i] = { frame, value, slope }; */
	keysX[0] = (HermiteKey){ 0.0f, pos.x, 0.8f };
	keysY[0] = (HermiteKey){ 0.0f, pos.y, 0.8f };
	keysS[0] = (HermiteKey){ 0.0f, 3.0f, -0.8f };

	keysX[1] = (HermiteKey){ 60.0f, pos.x + (30.0f * queue) - 172.0f, 1.0f };
	keysY[1] = (HermiteKey){ 60.0f, pos.y + 64.0f, 1.0f };
	keysS[1] = (HermiteKey){ 60.0f, 1.0f, 0.0f };
}
void dSongPrize::executeState_Shrink() {
	float modX = GetHermiteCurveValue(timer, keysX, Xkey_count);
	float modY = GetHermiteCurveValue(timer, keysY, Ykey_count);
	//float modS = GetHermiteCurveValue(timer, keysS, Skey_count);
	float modS = 3.0f - (timer * (2.0f / 60.0f));

	pos = (Vec){ modX, modY, pos.z };
	scale = (Vec){ modS, modS, modS };

	if (timer == 60) { doStateChange(&StateID_Wait); }

	timer += 1;
}
void dSongPrize::endState_Shrink() {}

void dSongPrize::beginState_Spin() {
	this->timer = 0;

	Xkey_count = 2;
	keysX[0] = (HermiteKey){ 0.0f, 0.0f, 0.0f };
	keysX[1] = (HermiteKey){ 20.0f, 65535.0f, 0.0f };
}
void dSongPrize::executeState_Spin() {
	float modX = GetHermiteCurveValue(timer, keysX, Xkey_count);
	rot.y = (int)modX;

	Vec efPos = {pos.x-10.0f, pos.y-2.0f, pos.z-100.0f};
	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};
	if (timer == 20) { SpawnEffect("Wm_ob_flagget", 0, &efPos, &nullRot, &oneVec); }
	if (timer == 30) { doStateChange(&StateID_Wait); }

	timer += 1;
}
void dSongPrize::endState_Spin() {}



class dSongBlock : public daEnBlockMain_c {
public:
	Physics::Info physicsInfo;

	int onCreate();
	int onDelete();
	int onExecute();
	int beforeDraw();
	int onDraw();

	mHeapAllocator_c allocator;
	nw4r::g3d::ResFile resFile;
	m3d::mdl_c bodyModel;
	m3d::mdl_c glowModel;
	m3d::mdl_c deadModel;
	m3d::mdl_c deadglowModel;

	StageActorLight light;
	mHeapAllocator_c allocatorB;

	int note;
	int isGlowing;
	int glowTimer;
	int isDead;
	int standAlone;
	int item;

	void calledWhenUpMoveExecutes();
	void calledWhenDownMoveExecutes();
	void blockWasHit(bool isDown);
	void glow();
	void unglow();

	USING_STATES(dSongBlock);
	DECLARE_STATE(Wait);

	static dSongBlock *build();
};

/*****************************************************************************/
// Sing Along

class dSingAlong : public dStageActor_c {
	public:
		static dSingAlong *instance;
		static dSingAlong *build();

		void RegisterNote(int note);
		void addPowerups();

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		//int beforeExecute() { return true; }
		int afterExecute(int) { return true; }

		dSingAlong() : state(this, &StateID_Intro) { }

		mHeapAllocator_c allocator;

		dSongPrize *PrizeModel;
		dSongPrize *Pa;
		dSongPrize *Pb;
		dSongPrize *Pc;
		dSongPrize *Pd;

		dSongBlock *SBa;
		dSongBlock *SBb;
		dSongBlock *SBc;
		dSongBlock *SBd;
		dSongBlock *SBe;
		dSongBlock *SBf;
		dSongBlock *SBg;

		int song;
		int prize[4];
 		int chorus;
		int currentNote;
		int endNote;
		int timer;
		int counter;
		int Powerups[10];
		int isResponding;
		int success;

		dStateWrapper_c<dSingAlong> state;

		// Intro, Call, Response, Display Prize, Failure, Win, Collect Prizes
		USING_STATES(dSingAlong);
		DECLARE_STATE(Intro);
		DECLARE_STATE(Call);
		DECLARE_STATE(Response);
		DECLARE_STATE(Prize);
		DECLARE_STATE(Failure);
		DECLARE_STATE(Win);
		DECLARE_STATE(Mistake);
};

dSingAlong *dSingAlong::instance = 0;
dSingAlong *dSingAlong::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dSingAlong));
	dSingAlong *c = new(buffer) dSingAlong;

	instance = c;
	return c;
}


/*****************************************************************************/
// Events
int dSingAlong::onCreate() {
	NoMichaelBuble = true;
	StageC4::instance->_1D = 1; // enable no-pause

	// Load in the settings
	this->song = this->settings & 0xF;
	this->prize[0] = (this->settings >> 28) & 0xF;
	this->prize[1] = (this->settings >> 24) & 0xF;
	this->prize[2] = (this->settings >> 20) & 0xF;
	this->prize[3] = (this->settings >> 16) & 0xF;
	this->chorus = -1;
	this->currentNote = 0;
	this->success = 0;

	this->Powerups[0] = 0; // Mushroom
	this->Powerups[1] = 0; // Fireflower
	this->Powerups[2] = 0; // Propeller
	this->Powerups[3] = 0; // Iceflower
	this->Powerups[4] = 0; // Penguin
	this->Powerups[5] = 0; // MiniShroom
	this->Powerups[6] = 0; // Starman
	this->Powerups[7] = 0; // Hammer
	this->Powerups[8] = 0; // 1-ups
	this->Powerups[9] = 0; // Coins

	// Create and prepare the blocks
	S16Vec rot = (S16Vec){0,0,0};
	float x = pos.x;
	float y = pos.y - 40.0;
	float z = pos.z;

	Vec blockPos = {x-96.0f, y, z};
	SBa = (dSongBlock*)create(WM_KILLER, 1, &blockPos, &rot, 0);
	blockPos.x += 32.0f;
	SBb = (dSongBlock*)create(WM_KILLER, 2, &blockPos, &rot, 0);
	blockPos.x += 32.0f;
	SBc = (dSongBlock*)create(WM_KILLER, 3, &blockPos, &rot, 0);
	blockPos.x += 32.0f;
	SBd = (dSongBlock*)create(WM_KILLER, 4, &blockPos, &rot, 0);
	blockPos.x += 32.0f;
	SBe = (dSongBlock*)create(WM_KILLER, 5, &blockPos, &rot, 0);
	blockPos.x += 32.0f;
	SBf = (dSongBlock*)create(WM_KILLER, 6, &blockPos, &rot, 0);
	blockPos.x += 32.0f;
	SBg = (dSongBlock*)create(WM_KILLER, 7, &blockPos, &rot, 0);

	// // Trigger the intro state
	state.setState(&StateID_Intro);
	isResponding = 0;

	return true;
}

int dSingAlong::onExecute() {
	state.execute();

	return true;
}

int dSingAlong::onDraw() { return true; }

int dSingAlong::onDelete() {
	instance = 0;
	return 1;
}

/*****************************************************************************/
// Register a Note being played by the players
void dSingAlong::RegisterNote(int note) {
	nw4r::snd::SoundHandle handle;

	if (isResponding == 1) {

		if (note == Songs[song][chorus][currentNote][0]) {
			BonusMusicPlayer(Songs[song][chorus][currentNote][1]-1);
			// MapSoundPlayer(SoundRelatedClass, Notes[Songs[song][chorus][currentNote][1]-1], 1);
			currentNote += 1;
		}
		else {
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_MG_CMN_WIN_CLOSE, 1);
			isResponding = 0;
			state.setState(&StateID_Mistake);
		}
	}
}

/*****************************************************************************/
// Intro
CREATE_STATE(dSingAlong, Intro);

void dSingAlong::beginState_Intro() {}
void dSingAlong::endState_Intro() {}
void dSingAlong::executeState_Intro() {
	MakeMarioEnterDemoMode();
	state.setState(&StateID_Prize);
}

//*****************************************************************************/
// Prize
CREATE_STATE(dSingAlong, Prize);

void dSingAlong::beginState_Prize() {
	this->timer = 120;
}
void dSingAlong::executeState_Prize() {

	if ((timer == 120) && (chorus >= 0)) { // Play a nice success sound, and wait a second
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_MG_IH_PAIR_OK, 1);
		//PlaySound(this, SE_MG_IH_PAIR_OK); // SE_MG_IH_NICE or SE_MG_UH_NICE

		int p;
		p = prize[chorus];
		this->Powerups[p] += 1;
	}

	if ((timer == 60) && (chorus >= 0)) {
		if (chorus == 0) { Pa = PrizeModel; }
		if (chorus == 1) { Pb = PrizeModel; }
		if (chorus == 2) { Pc = PrizeModel; }
		if (chorus == 3) { Pd = PrizeModel; }

		PrizeModel->doStateChange(&dSongPrize::StateID_Shrink);
	}

	if (timer == 0) {
		chorus += 1;

		if (chorus == 4) {
			if (success != 4)
				state.setState(&StateID_Failure);
			else
				state.setState(&StateID_Win);
			return;
		}

		Vec efPos = {pos.x, pos.y+32.0f, pos.z+500.0f};
		S16Vec nullRot = {0,0,0};
		Vec efScale = {1.5f, 1.5f, 1.5f};
		SpawnEffect("Wm_en_blockcloud", 0, &efPos, &nullRot, &efScale);
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_OBJ_ITEM_APPEAR, 1);
		//PlaySound(this, SE_OBJ_ITEM_APPEAR); // SE_OBJ_GOOD_ITEM_APPEAR

		PrizeModel = (dSongPrize*)create(WM_SINKSHIP, chorus + (prize[chorus] << 16), &pos, &rot, 0);
	}

	if (timer == -90) {
		state.setState(&StateID_Call);
	}

	timer -= 1;
}
void dSingAlong::endState_Prize() {}


//*****************************************************************************/
// Call
CREATE_STATE(dSingAlong, Call);

void dSingAlong::beginState_Call() {
	timer = 0;
	currentNote = 0;
}
void dSingAlong::executeState_Call() {
	if (timer == (Songs[song][chorus][currentNote][2] * Tempo[song])) {

		BonusMusicPlayer(Songs[song][chorus][currentNote][1]-1);

		Vec effPos;

		if      (Songs[song][chorus][currentNote][0] == 1) { SBa->glow(); effPos = SBa->pos; }
		else if (Songs[song][chorus][currentNote][0] == 2) { SBb->glow(); effPos = SBb->pos; }
		else if (Songs[song][chorus][currentNote][0] == 3) { SBc->glow(); effPos = SBc->pos; }
		else if (Songs[song][chorus][currentNote][0] == 4) { SBd->glow(); effPos = SBd->pos; }
		else if (Songs[song][chorus][currentNote][0] == 5) { SBe->glow(); effPos = SBe->pos; }
		else if (Songs[song][chorus][currentNote][0] == 6) { SBf->glow(); effPos = SBf->pos; }
		else if (Songs[song][chorus][currentNote][0] == 7) { SBg->glow(); effPos = SBg->pos; }

		Vec ringPos = {effPos.x, effPos.y+8.0f, effPos.z-100.0f};
		S16Vec nullRot = {0,0,0};
		Vec oneVec = {1.0f, 1.0f, 1.0f};
		SpawnEffect("Wm_en_vshit_ring", 0, &ringPos, &nullRot, &oneVec);

		currentNote += 1;

		if (Songs[song][chorus][currentNote][0] == 0) {
			state.setState(&StateID_Response);
		}
	}

	timer += 1;
}
void dSingAlong::endState_Call() {
	MakeMarioExitDemoMode();
}


/*****************************************************************************/
// Response
CREATE_STATE(dSingAlong, Response);

void dSingAlong::beginState_Response() {
	timer = 0;
	currentNote = 0;
	isResponding = 1;
}
void dSingAlong::executeState_Response() {
	if (Songs[song][chorus][currentNote][0] == 0) {
		isResponding = 0;
		MakeMarioEnterDemoMode();
		this->success = this->success + 1;
		state.setState(&StateID_Prize);
	}
}
void dSingAlong::endState_Response() {}


//*****************************************************************************/
// Prize
CREATE_STATE(dSingAlong, Mistake);

void dSingAlong::beginState_Mistake() {
	SBa->unglow();
	SBb->unglow();
	SBc->unglow();
	SBd->unglow();
	SBe->unglow();
	SBf->unglow();
	SBg->unglow();

	MakeMarioEnterDemoMode();
	this->timer = 120;
	chorus += 1;
}
void dSingAlong::executeState_Mistake() {

	if (timer == 60) {
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_MG_IH_NOPAIR_NG, 1);

		Vec efPos = {pos.x, pos.y+32.0f, pos.z+500.0f};
		S16Vec nullRot = {0,0,0};
		Vec efScale = {1.5f, 1.5f, 1.5f};
		SpawnEffect("Wm_en_blockcloud", 0, &efPos, &nullRot, &efScale);
		PrizeModel->Delete(1);
	}

	if (chorus >= 4 && timer == 15) {
		state.setState(&StateID_Failure);
		return;
	}

	if (timer == 0) {
		Vec efPos = {pos.x, pos.y+32.0f, pos.z+500.0f};
		S16Vec nullRot = {0,0,0};
		Vec efScale = {1.5f, 1.5f, 1.5f};
		SpawnEffect("Wm_en_blockcloud", 0, &efPos, &nullRot, &efScale);
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_OBJ_ITEM_APPEAR, 1);

		PrizeModel = (dSongPrize*)create(WM_SINKSHIP, chorus + (prize[chorus] << 16), &pos, &rot, 0);
	}

	if (timer == -90) {
		state.setState(&StateID_Call);
	}

	timer -= 1;
}
void dSingAlong::endState_Mistake() {

	if (chorus != 4) {
		SBa->isDead = 0;
		SBb->isDead = 0;
		SBc->isDead = 0;
		SBd->isDead = 0;
		SBe->isDead = 0;
		SBf->isDead = 0;
		SBg->isDead = 0;
	}
}

/*****************************************************************************/
// Failure
CREATE_STATE(dSingAlong, Failure);

void dSingAlong::beginState_Failure() {
	this->timer = 0;

	MakeMarioEnterDemoMode();
}
void dSingAlong::executeState_Failure() {
	if (timer == 5) {
		if (success == 0) {
			for (int i = 0; i < 4; i++)
				if (dAcPy_c *player = dAcPy_c::findByID(i))
					player->setAnimePlayWithAnimID(dm_surprise);
		}
		else {
			for (int i = 0; i < 4; i++)
				if (dAcPy_c *player = dAcPy_c::findByID(i)) {
					player->setAnimePlayWithAnimID(dm_glad);
					player->setFlag(0x24);
				}
		}
	}

	// Play a success/failure sound
	if (timer == 20) {
		StopBGMMusic();
		nw4r::snd::SoundHandle handle;

		if (success == 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, STRM_BGM_MINIGAME_FANFARE_BAD, 1);
		else
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, STRM_BGM_MINIGAME_FANFARE_GOOD, 1);
	}

	// Delete the big powerup with a poof if it's fail
	// if (timer == 30*1) {
	// 	SpawnEffect("Wm_en_blockcloud", 0, &(Vec){pos.x, pos.y+32.0, pos.z+500.0}, &(S16Vec){0,0,0}, &(Vec){1.5, 1.5, 1.5});
	// 	nw4r::snd::SoundHandle handle;
	// 	PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_MG_IH_NOPAIR_OK, 1);
	// 	PrizeModel->Delete(1);
	// 	if (chorus == 0)
	// 		timer += 140;
	// }

	// Make the banked powerups do a little dance/effect
	// Play a sound for each powerup gained
	if (timer == 30*2 || timer == 30*3 || timer == 30*4 || timer == 30*5) {
		dSongPrize *dancer = 0;
		if (timer == 30*2)
			dancer = Pa;
		else if (timer == 30*3)
			dancer = Pb;
		else if (timer == 30*4)
			dancer = Pc;
		else if (timer == 30*5)
			dancer = Pd;

		if (dancer) {
			dancer->doStateChange(&dSongPrize::StateID_Spin);
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_OBJ_ITEM_FROM_KINOPIO, 1);
		}
		else {
			timer += 29;
		}
	}

	// If victory, make mario do a little dance/sound
	if (timer == 30*7) {
		nw4r::snd::SoundHandle handle1, handle2, handle3, handle4;

		if (GetSpecificPlayerActor(0) != 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle1, (success == 0) ? SE_VOC_MA_CS_COURSE_MISS : SE_VOC_MA_CLEAR_MULTI, 1);
		if (GetSpecificPlayerActor(1) != 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle2, (success == 0) ? SE_VOC_LU_CS_COURSE_MISS : SE_VOC_LU_CLEAR_MULTI, 1);
		if (GetSpecificPlayerActor(2) != 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle3, (success == 0) ? SE_VOC_KO_CS_COURSE_MISS : SE_VOC_KO_CLEAR_MULTI, 1);
		if (GetSpecificPlayerActor(3) != 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle4, (success == 0) ? SE_VOC_KO2_CS_COURSE_MISS : SE_VOC_KO2_CLEAR_MULTI, 1);
	}


	// Add the powerups and exit the stage
	if (timer == 30*9) {
		this->addPowerups();
		ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE);
	}
	timer += 1;
}
void dSingAlong::endState_Failure() {}


/*****************************************************************************/
// Win
CREATE_STATE(dSingAlong, Win);

void dSingAlong::beginState_Win() {
	this->timer = 0;
	MakeMarioEnterDemoMode();
}
void dSingAlong::executeState_Win() {
	if (timer == 5)
		for (int i = 0; i < 4; i++)
			if (dAcPy_c *player = dAcPy_c::findByID(i)) {
				player->setAnimePlayWithAnimID(dm_glad);
				player->setFlag(0x24);
			}

	// Play a success/failure sound
	if (timer == 30) {
		nw4r::snd::SoundHandle handle;
		PlaySoundWithFunctionB4(SoundRelatedClass, &handle, STRM_BGM_MINIGAME_FANFARE_GOOD, 1);
		StopBGMMusic();
	}

	// Make the banked powerups do a little dance/effect
	// Play a sound for each powerup gained
	if (timer == 30*3 || timer == 30*4 || timer == 30*5 || timer == 30*6) {
		dSongPrize *dancer = 0;
		if (timer == 30*3)
			dancer = Pa;
		else if (timer == 30*4)
			dancer = Pb;
		else if (timer == 30*5)
			dancer = Pc;
		else if (timer == 30*6)
			dancer = Pd;

		if (dancer) {
			dancer->doStateChange(&dSongPrize::StateID_Spin);
			nw4r::snd::SoundHandle handle;
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle, SE_OBJ_ITEM_FROM_KINOPIO, 1);
		}
		else {
			timer += 29;
		}
	}

	// If victory, make mario do a little dance/sound
	if (timer == 30*8) {
		nw4r::snd::SoundHandle handle1, handle2, handle3, handle4;

		if (GetSpecificPlayerActor(0) != 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle1, SE_VOC_MA_CLEAR_MULTI, 1);
		if (GetSpecificPlayerActor(1) != 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle2, SE_VOC_LU_CLEAR_MULTI, 1);
		if (GetSpecificPlayerActor(2) != 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle3, SE_VOC_KO_CLEAR_MULTI, 1);
		if (GetSpecificPlayerActor(3) != 0)
			PlaySoundWithFunctionB4(SoundRelatedClass, &handle4, SE_VOC_KO2_CLEAR_MULTI, 1);

		Vec efPos = {pos.x-80.0f, pos.y+20.0f, pos.z+500.0f};
		S16Vec nullRot = {0,0,0};
		Vec efScale = {0.75f, 0.75f, 0.75f};
		SpawnEffect("Wm_ob_fireworks_y", 0, &efPos, &nullRot, &efScale);
	}

	if (timer == (int)30*8.5) {
		Vec efPos = {pos.x+108.0f, pos.y+32.0f, pos.z+500.0f};
		S16Vec nullRot = {0,0,0};
		Vec efScale = {0.75f, 0.75f, 0.75f};
		SpawnEffect("Wm_ob_fireworks_b", 0, &efPos, &nullRot, &efScale);
	}

	if (timer == 30*9) {
		Vec efPos = {pos.x, pos.y+50.0f, pos.z+500.0f};
		S16Vec nullRot = {0,0,0};
		Vec efScale = {0.75f, 0.75f, 0.75f};
		SpawnEffect("Wm_ob_fireworks_g", 0, &efPos, &nullRot, &efScale);
	}


	// Add the powerups and exit the stage
	if (timer == 30*11) {
		this->addPowerups();
		ExitStage(WORLD_MAP, 0, BEAT_LEVEL, MARIO_WIPE);
	}
	timer += 1;
}
void dSingAlong::endState_Win() {}


/*****************************************************************************/
// Add Powerups at the End of the Stage
void dSingAlong::addPowerups() {
	SaveFile *file = GetSaveFile();
	SaveBlock *block = file->GetBlock(file->header.current_file);

	for (int i = 0; i < 8; i++) { // Change this to 8 to support hammers
		block->powerups_available[i] = block->powerups_available[i] + this->Powerups[i];

		if (block->powerups_available[i] > 99) { block->powerups_available[i] = 99; }
	}

	for (int i = 0; i < 4; i++) { // Make sure all players get the reward!
		block->player_coins[i] = (this->Powerups[9] * 50) + block->player_coins[i];

		for (;block->player_coins[i] < 100; block->player_coins[i] - 100) {
			block->player_coins[i] = 1 + block->player_coins[i];
		}

		block->player_lives[i] = this->Powerups[8] + block->player_lives[i];
		if (block->player_lives[i] > 99) { block->player_lives[i] = 99; }
	}

	return;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
// Replaces: Nothing yet



CREATE_STATE(dSongBlock, Wait);


dSongBlock *dSongBlock::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dSongBlock));
	return new(buffer) dSongBlock;
}


int dSongBlock::onCreate() {

	// Settings
	this->note = this->settings & 0xF;
	this->standAlone = (this->settings >> 4) & 0xF;
	this->item = (this->settings >> 16) & 0xFF;

	// Model creation
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	char modelName [24];
	sprintf(modelName, "g3d/block_light_%d.brres", this->note);

	this->resFile.data = getResource("block_light", modelName);
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("block_light");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_MapObj(&bodyModel, 0); // 800B42B0

	char glowName [25];
	sprintf(glowName, "g3d/block_light_g%d.brres", this->note);

	this->resFile.data = getResource("block_light", glowName);
	mdl = this->resFile.GetResMdl("block_light");
	glowModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_MapObj(&glowModel, 0); // 800B42B0



	// Dead Models
	this->resFile.data = getResource("block_light", "g3d/block_light_8.brres");
	mdl = this->resFile.GetResMdl("block_light");
	deadModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_MapObj(&deadModel, 0); // 800B42B0

	this->resFile.data = getResource("block_light", "g3d/block_light_g8.brres");
	mdl = this->resFile.GetResMdl("block_light");
	deadglowModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_MapObj(&deadglowModel, 0); // 800B42B0

	allocator.unlink();

	// Glow related
	isGlowing = 0;
	glowTimer = 0;
	isDead = 0;

	light.init(&allocatorB, 2);


	// Block Physics
	blockInit(pos.y);

	physicsInfo.x1 = -8;
	physicsInfo.y1 = 16;
	physicsInfo.x2 = 8;
	physicsInfo.y2 = 0;

	physicsInfo.otherCallback1 = &daEnBlockMain_c::OPhysicsCallback1;
	physicsInfo.otherCallback2 = &daEnBlockMain_c::OPhysicsCallback2;
	physicsInfo.otherCallback3 = &daEnBlockMain_c::OPhysicsCallback3;

	physics.setup(this, &physicsInfo, 3, currentLayerID);
	physics.flagsMaybe = 0x260;
	physics.callback1 = &daEnBlockMain_c::PhysicsCallback1;
	physics.callback2 = &daEnBlockMain_c::PhysicsCallback2;
	physics.callback3 = &daEnBlockMain_c::PhysicsCallback3;
	physics.addToList();

	// Change State
	doStateChange(&dSongBlock::StateID_Wait);

	return true;
}

int dSongBlock::onDelete() {
	physics.removeFromList();
	return true;
}

int dSongBlock::onExecute() {
	acState.execute();
	physics.update();
	blockUpdate();

	if (glowTimer > 0) { glowTimer--; }
	else { isGlowing = 0; }

	light.pos.x = pos.x;
	light.pos.y = pos.y+8.0;
	light.pos.z = pos.z;

	light.size = 226.0;
	light.update();

	// now check zone bounds based on state
	if (acState.getCurrentState()->isEqual(&StateID_Wait)) {
		checkZoneBoundaries(0);
	}

	return true;
}

int dSongBlock::beforeDraw() {
	light.draw();
	return dStageActor_c::beforeDraw();
}

int dSongBlock::onDraw() {
	matrix.translation(pos.x, pos.y+8.0, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	if (isDead == 0) {
		if (isGlowing == 0) {
			bodyModel.setDrawMatrix(matrix);
			bodyModel.setScale(&scale);
			bodyModel.calcWorld(false);

			bodyModel.scheduleForDrawing();
		}

		else {
			glowModel.setDrawMatrix(matrix);
			glowModel.setScale(&scale);
			glowModel.calcWorld(false);

			glowModel.scheduleForDrawing();
		}
	}
	else {
		if (isGlowing == 0) {
			deadModel.setDrawMatrix(matrix);
			deadModel.setScale(&scale);
			deadModel.calcWorld(false);

			deadModel.scheduleForDrawing();
		}

		else {
			deadglowModel.setDrawMatrix(matrix);
			deadglowModel.setScale(&scale);
			deadglowModel.calcWorld(false);

			deadglowModel.scheduleForDrawing();
		}
	}

	return true;
}

void dSongBlock::blockWasHit(bool isDown) {
	pos.y = initialY;

	Vec ringPos = {pos.x, pos.y+8.0f, pos.z-100.0f};
	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};
	SpawnEffect("Wm_en_vshit_ring", 0, &ringPos, &nullRot, &oneVec);
	Vec keygetPos = {pos.x, pos.y+8.0f, pos.z-100.0f};
	Vec keygetScale = {0.5f, 0.5f, 0.5f};
	SpawnEffect("Wm_ob_keyget02_lighit", 0, &keygetPos, &nullRot, &keygetScale);

	if (item > 0) {
		Vec itemPos = {pos.x, pos.y, pos.z};
		create(EN_ITEM, item, &itemPos, &rot, 0);
	}

	if (standAlone) {

		int play;
		// C, C#, D, D#, E, F, F#, G, G#, A, A#, B

		if 		(note == 0) { play = 12; }
		else if (note == 1) { play = 12; }
		else if (note == 2) { play = 14; }
		else if (note == 3) { play = 16; }
		else if (note == 4) { play = 17; }
		else if (note == 5) { play = 19; }
		else if (note == 6) { play = 21; }
		else if (note == 7) { play = 23; }
		else if (note == 8) { play = 0; }

		// nw4r::snd::SoundHandle handle;
		// PlaySoundWithFunctionB4(SoundRelatedClass, &handle, Notes[play], 1);
		BonusMusicPlayer(play);
	}
	else {
		dSingAlong::instance->RegisterNote(this->note);
	}

	physics.setup(this, &physicsInfo, 3, currentLayerID);
	physics.addToList();

	doStateChange(&StateID_Wait);
}

void dSongBlock::calledWhenUpMoveExecutes() {
	if (initialY >= pos.y)
		blockWasHit(false);
}

void dSongBlock::calledWhenDownMoveExecutes() {
	if (initialY <= pos.y)
		blockWasHit(true);
}

void dSongBlock::glow() {
	isGlowing = 1;
	isDead = 0;
	glowTimer = 15;
}

void dSongBlock::unglow() {
	isDead = 1;
	Vec efPos = {pos.x, pos.y+8.0f, pos.z-100.0f};
	S16Vec nullRot = {0,0,0};
	Vec efScale = {0.4f, 0.4f, 0.4f};
	SpawnEffect("Wm_en_sanbohit_smk", 0, &efPos, &nullRot, &efScale);
}

void dSongBlock::beginState_Wait() {}
void dSongBlock::endState_Wait() {}
void dSongBlock::executeState_Wait() {
	int result = blockResult();

	if (result == 0)
		return;

	if (result == 1) {
		glow();
		doStateChange(&daEnBlockMain_c::StateID_UpMove);
		anotherFlag = 2;
		isGroundPound = false;
	} else {
		glow();
		doStateChange(&daEnBlockMain_c::StateID_DownMove);
		anotherFlag = 1;
		isGroundPound = true;
	}
}


