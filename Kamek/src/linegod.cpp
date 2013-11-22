#include <common.h>
#include <game.h>

// TODO: make "No Deactivation"

struct BgActor {
	u16 def_id;		// 0x00
	u16 x;			// 0x02
	u16 y;			// 0x04
	u8 layer;		// 0x06
	u8 EXTRA_off;	// 0x07
	u32 actor_id;	// 0x08
};

struct BgActorDef {
	u32 tilenum;
	u16 actor;
	u8 _06[2];
	float x;
	float y;
	float z;
	float width;
	float height;
	u32 extra_var;
};

struct dBgActorManager_c {
	u32 vtable;		// 0x00
	u8 _04[0x34];	// 0x04
	BgActor *array;	// 0x38
	u32 count;		// 0x3C
	u32 type;		// 0x40
};

extern dBgActorManager_c *dBgActorManager;

extern BgActorDef *BgActorDefs;

struct BG_GM_hax {
	u8 _00[0x8FE64];
	float _0x8FE64;
	float _0x8FE68;
	float _0x8FE6C;
	float _0x8FE70;
};

extern BG_GM_hax *BG_GM_ptr;

// Regular class is 0x3D0.
// Let's add stuff to the end just to be safe.
// Size is now 0x400
// 80898798 38600400

#define LINEGOD_FUNC_ACTIVATE	0
#define LINEGOD_FUNC_DEACTIVATE	1

struct LineGod {
	u32 id;			// 0x00
	u32 settings;	// 0x04
	u16 name;		// 0x08
	u8 _0A[6];		// 0x0A
	u8 _10[0x9C];	// 0x10
	float x;		// 0xAC
	float y;		// 0xB0
	float z;		// 0xB4
	u8 _B8[0x318];	// 0xB8
	u64 eventFlag;	// 0x3D0
	u8 func;		// 0x3D4
	u8 width;		// 0x3D5
	u8 height;		// 0x3D6
	u8 lastEvState;	// 0x3D7
	BgActor *ac[8];	// 0x3D8
};


fBase_c *FindActorByID(u32 id);

u16 *GetPointerToTile(BG_GM_hax *self, u16 x, u16 y, u16 layer, short *blockID_p, bool unused);



void LineGod_BuildList(LineGod *self);
bool LineGod_AppendToList(LineGod *self, BgActor *ac);
void LineGod_Update(LineGod *self);


bool LineGod_Create(LineGod *self) {
	char eventNum	= (self->settings >> 24)	& 0xFF;
	self->eventFlag = (u64)1 << (eventNum - 1);
	
	
	
	self->func		= (self->settings)			& 1;
	self->width		= (self->settings >> 4)		& 15;
	self->height	= (self->settings >> 8)		& 15;
	
	self->lastEvState = 0xFF;
	
	LineGod_BuildList(self);
	LineGod_Update(self);
	
	return true;
}

bool LineGod_Execute(LineGod *self) {
	LineGod_Update(self);
	return true;
}

void LineGod_BuildList(LineGod *self) {
	for (int clearIdx = 0; clearIdx < 8; clearIdx++) {
		self->ac[clearIdx] = 0;
	}
	
	

	float gLeft = self->x - (BG_GM_ptr->_0x8FE64 - fmod(BG_GM_ptr->_0x8FE64, 16));
	float gTop = self->y - (BG_GM_ptr->_0x8FE6C - fmod(BG_GM_ptr->_0x8FE6C, 16));

	// 1 unit padding to avoid catching stuff that is not in our rectangle
	Vec grect1 = (Vec){
		gLeft + 1, gTop - (self->height * 16) + 1, 0
	};

	Vec grect2 = (Vec){
		gLeft + (self->width * 16) - 1, gTop - 1, 0
	};

	
	for (int i = 0; i < dBgActorManager->count; i++) {
		BgActor *ac = &dBgActorManager->array[i];

		// the Def width/heights are padded with 8 units on each side
		// except for one of the steep slopes, which differs for no reason

		BgActorDef *def = &BgActorDefs[ac->def_id];
		float aXCentre = (ac->x * 16) + def->x;
		float aYCentre = (-ac->y * 16) + def->y;

		float xDistToCentre = (def->width - 16) / 2;
		float yDistToCentre = (def->height - 16) / 2;

		Vec arect1 = (Vec){
			aXCentre - xDistToCentre, aYCentre - yDistToCentre, 0
		};
		
		Vec arect2 = (Vec){
			aXCentre + xDistToCentre, aYCentre + yDistToCentre, 0
		};

		if (RectanglesOverlap(&arect1, &arect2, &grect1, &grect2))
			LineGod_AppendToList(self, ac);
	}
}

bool LineGod_AppendToList(LineGod *self, BgActor *ac) {
	
	for (int search = 0; search < 8; search++) {
		if (self->ac[search] == 0) {
			self->ac[search] = ac;
			return true;
		}
	}
	
	return false;
}

void LineGod_Update(LineGod *self) {
	
	u8 newEvState = 0;
	if (dFlagMgr_c::instance->flags & self->eventFlag)
		newEvState = 1;
	
	if (newEvState == self->lastEvState)
		return;
	
	u16 x_bias = (BG_GM_ptr->_0x8FE64 / 16);
	u16 y_bias = -(BG_GM_ptr->_0x8FE6C / 16);
	
	
	u8 offState;
	if (self->func == LINEGOD_FUNC_ACTIVATE)
		offState = (newEvState == 1) ? 1 : 0;
	else
		offState = (newEvState == 1) ? 0 : 1;
	
	
	for (int i = 0; i < 8; i++) {
		if (self->ac[i] != 0) {
			BgActor *ac = self->ac[i];
			
			
			ac->EXTRA_off = offState;
			if (offState == 1 && ac->actor_id != 0) {
				fBase_c *assoc_ac = FindActorByID(ac->actor_id);
				if (assoc_ac != 0)
					assoc_ac->Delete();
				ac->actor_id = 0;
			}
			
			u16 *tile = GetPointerToTile(BG_GM_ptr, (ac->x + x_bias) * 16, (ac->y + y_bias) * 16, 0, 0, 0);
			if (offState == 1)
				*tile = 0;
			else
				*tile = BgActorDefs[ac->def_id].tilenum;
			
		}
	}
	
	
	
	self->lastEvState = newEvState;
}
