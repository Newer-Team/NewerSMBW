#include <common.h>
#include <game.h>
#include <dCourse.h>

struct LevelSpecial {
	u32 id;			// 0x00
	u32 settings;	// 0x04
	u16 name;		// 0x08
	u8 _0A[6];		// 0x0A
	u8 _10[0x9C];	// 0x10
	float x;		// 0xAC
	float y;		// 0xB0
	float z;		// 0xB4
	u8 _B8[0x318];	// 0xB8
	// Any variables you add to the class go here; starting at offset 0x3D0
	u64 eventFlag;	// 0x3D0
	u8 type;		// 0x3D4
	u8 effect;		// 0x3D5
	u8 lastEvState;	// 0x3D6
	u8 func;		// 0x3D7
	u32 keepTime;
	u32 setTime;
};


extern u16 TimeStopFlag;
extern u32 AlwaysDrawFlag;
extern u32 AlwaysDrawBranch;

extern float MarioDescentRate;
extern float MarioJumpMax;
extern float MarioJumpArc;
extern float MiniMarioJumpArc;
// extern float MarioSize;

extern float GlobalSpriteSize;
extern float GlobalSpriteSpeed;
extern float GlobalRiderSize;
extern char SizerOn;
extern char ZOrderOn;
extern int GlobalStarsCollected;

extern VEC2 BGScaleFront;
extern VEC2 BGScaleBack;
extern char BGScaleEnabled;

extern u32 GameTimer;

extern char CameraLockEnabled;
extern VEC2 CameraLockPosition;
extern char isLockPlayerRotation;

#define time *(u32*)((GameTimer) + 0x4)


static const float GlobalSizeFloatModifications [] = {1, 0.25, 0.5, 0.75, 1.25, 1.5, 1.75, 2, 2.5, 3, 4, 5, 6, 7, 8, 10 };
static const float GlobalRiderFloatModifications [] = {1, 0.6, 0.7, 0.9, 1, 1, 1, 1.1, 1.25, 1.5, 2, 2.5, 3, 3.5, 4, 5};
static const float BGScaleChoices[] = {0.1f, 0.15f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.9f, 1.0f, 1.125f, 1.25f, 1.5f, 1.75f, 2.0f, 2.25f, 2.5f};

bool NoMichaelBuble = false;

void LevelSpecial_Update(LevelSpecial *self);
bool ResetAfterLevel();

#define ACTIVATE	1
#define DEACTIVATE	0

fBase_c *FindActorByID(u32 id);


extern "C" void dAcPy_vf294(void *Mario, dStateBase_c *state, u32 unk);
void MarioStateChanger(void *Mario, dStateBase_c *state, u32 unk) {
	//OSReport("State: %p, %s", state, state->getName());

	if ((strcmp(state->getName(), "dAcPy_c::StateID_Balloon") == 0) && (NoMichaelBuble)) { return; }

	dAcPy_vf294(Mario, state, unk);
}

bool ResetAfterLevel(bool didItWork) {
	// TimeStopFlag = 0;
	MarioDescentRate = -4;
	MarioJumpMax = 3.628;
	MarioJumpArc = 2.5;
	MiniMarioJumpArc = 2.5;
	// MarioSize = 1.0;
	GlobalSpriteSize = 1.0;
	GlobalSpriteSpeed = 1.0;
	GlobalRiderSize = 1.0;
	SizerOn = 0;
	AlwaysDrawFlag = 0x9421FFF0;
	AlwaysDrawBranch = 0x7C0802A6;
	ZOrderOn = 0;
	GlobalStarsCollected = 0;
	NoMichaelBuble = false;
	BGScaleEnabled = 0;
	CameraLockEnabled = 0;
	isLockPlayerRotation = false;
	return didItWork;
}

void FuckinBubbles() {
	dCourse_c *course = dCourseFull_c::instance->get(GetAreaNum());
	bool thing = false;

	int zone = GetZoneNum();
	for (int i = 0; i < course->zoneSpriteCount[zone]; i++) {
		dCourse_c::sprite_s *spr = &course->zoneFirstSprite[zone][i];
		if (spr->type == 246 && (spr->settings & 0xF) == 8)
			thing = true;
	}

	if (thing) {
		OSReport("DISABLING EXISTING BUBBLES.\n");
		for (int i = 0; i < 4; i++)
			Player_Flags[i] &= ~4;
	}
}

bool LevelSpecial_Create(LevelSpecial *self) {
	char eventNum	= (self->settings >> 24)	& 0xFF;
	self->eventFlag = (u64)1 << (eventNum - 1);
	
	self->keepTime  = 0;
	
	self->type		= (self->settings)			& 15;
	self->effect	= (self->settings >> 4)		& 15;
	self->setTime	= (self->settings >> 8)     & 0xFFFF;

	self->lastEvState = 0xFF;
	
	LevelSpecial_Update(self);
	
	return true;
}

bool LevelSpecial_Execute(LevelSpecial *self) {
	if (self->keepTime > 0) {
		time = self->keepTime; }

	LevelSpecial_Update(self);
	return true;
}


void LevelSpecial_Update(LevelSpecial *self) {
	
	u8 newEvState = 0;
	if (dFlagMgr_c::instance->flags & self->eventFlag)
		newEvState = 1;
	
	if (newEvState == self->lastEvState)
		return;
		
	
	u8 offState;
	if (newEvState == ACTIVATE)
	{
		offState = (newEvState == 1) ? 1 : 0;

		switch (self->type) {
			// case 1:											// Time Freeze
			// 	TimeStopFlag = self->effect * 0x100;
			// 	break;
				
			case 2:											// Stop Timer
				self->keepTime  = time;
				break;
		
	
			case 3:											// Mario Gravity
				if (self->effect == 0)
				{											//Low grav
					MarioDescentRate = -2;
					MarioJumpArc = 0.5;
					MiniMarioJumpArc = 0.5;
					MarioJumpMax = 4.5;
				}
				else
				{											//Anti-grav
					MarioDescentRate = 0.5;
					MarioJumpArc = 4.0;
					MiniMarioJumpArc = 4.0;
					MarioJumpMax = 0.0;
				}
				break;
	
			case 4:											// Set Time
				time = (self->setTime << 0xC) - 1; // Possibly - 0xFFF?
				break;


			case 5:											// Global Enemy Size
				SizerOn = 3;

				GlobalSpriteSize = GlobalSizeFloatModifications[self->effect];
				GlobalRiderSize = GlobalRiderFloatModifications[self->effect];
				GlobalSpriteSpeed = GlobalRiderFloatModifications[self->effect];

				AlwaysDrawFlag = 0x38600001;
				AlwaysDrawBranch = 0x4E800020;
				break;
	
			case 6:											// Individual Enemy Size
				AlwaysDrawFlag = 0x38600001;
				AlwaysDrawBranch = 0x4E800020;

				if (self->effect == 0)
				{	
					SizerOn = 1;							// Nyb 5
				}
				else
				{											
					SizerOn = 2;							// Nyb 7
				}
				break;
		
			case 7:											// Z Order Hack
				ZOrderOn = 1;
				break;

			case 8:
				NoMichaelBuble = true;
				break;

			case 9:
				BGScaleEnabled = true;
				BGScaleFront.x = BGScaleChoices[(self->settings >> 20) & 15];
				BGScaleFront.y = BGScaleChoices[(self->settings >> 16) & 15];
				BGScaleBack.x = BGScaleChoices[(self->settings >> 12) & 15];
				BGScaleBack.y = BGScaleChoices[(self->settings >> 8) & 15];
				break;

			default:
				break;
		}
	}
	
	else
	{
		offState = (newEvState == 1) ? 0 : 1;

		switch (self->type) {
			// case 1:											// Time Freeze
			// 	TimeStopFlag = 0;
			// 	break;
				
			case 2:											// Stop Timer
				self->keepTime  = 0;
				break;
		
	
			case 3:											// Mario Gravity
				MarioDescentRate = -4;
				MarioJumpArc = 2.5;
				MiniMarioJumpArc = 2.5;
				MarioJumpMax = 3.628;
				break;
	
			case 4:											// Mario Size
				break;
		
			case 5:											// Global Enemy Size
				SizerOn = 0;

				GlobalSpriteSize = 1.0;
				GlobalRiderSize = 1.0;
				GlobalSpriteSpeed = 1.0;

				AlwaysDrawFlag = 0x9421FFF0;
				AlwaysDrawBranch = 0x7C0802A6;
				break;

			case 6:											// Individual Enemy Size
				SizerOn = 0;

				AlwaysDrawFlag = 0x9421FFF0;
				AlwaysDrawBranch = 0x7C0802A6;
				break;
		
			case 7:											// Z Order Hack
				ZOrderOn = 0;
				break;
				
			case 8:
				NoMichaelBuble = false;
				break;

			case 9:
				BGScaleEnabled = false;
				break;
	
			default:
				break;
		}
	}




	
	
	self->lastEvState = newEvState;
}
