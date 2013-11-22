#ifndef __KAMEK_STATE_H
#define __KAMEK_STATE_H

#include <common.h>

#ifdef REGION_PAL
	#define SAVED_STATE_TABLE (*((ObjState**)(0x8042A210)))
#endif

#ifdef REGION_NTSC
	#define SAVED_STATE_TABLE (*((ObjState**)(0x80429F30)))
#endif

struct ObjState {
	s16 area;
	s16 x;
	s16 y;
	s16 value;
};

s16 RestoreObjectState(ObjState *table, s16 area, float objX, float objY);
s32 StoreObjectState(ObjState *table, s16 area, s16 value, float objX, float objY);

#endif
