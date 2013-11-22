#include <common.h>
#include <actors.h>
#include <stage.h>
#include "levelinfo_old.h"
#include "fileload.h"
#include "layoutlib.h"

//#define DEBUG_NAMES

#ifndef DEBUG_NAMES
#endif

extern char CurrentLevel;
extern char CurrentWorld;

int DoNames(int state) {
	int wnum = (int)CurrentWorld;
	int lnum = (int)CurrentLevel;
	
	// Skip the title screen
	// and only process the code if the State is set to 1
	// (the screen has been initialised)
	if (state == 1 && lnum != STAGE_TITLE) {
		// grab the CRSIN object
		Actor *ptr = FindActorByType(CRSIN, 0);
		
		// FIX !!!!!
		
		if (ptr != 0) {
			void *worldObj = EmbeddedLayout_FindTextBoxByName((Layout*)((u32)ptr+0xB0), "TXT_WorldName");
			void *levelObj = EmbeddedLayout_FindTextBoxByName((Layout*)((u32)ptr+0xB0), "TXT_LevelName");
			if (worldObj == 0 || levelObj == 0) return state;
			
			/*char *file = RetrieveFileFromArc(ARC_TABLE, "Mario", "newer/names.bin");
			char *worldname = file + (wnum * 0x40);
			char *levelname = file + 0x280 + (wnum * 0xA80) + (lnum * 0x40);*/
			FileHandle fh;
			void *info = LoadFile(&fh, "/NewerRes/LevelInfo.bin");
			
			LevelInfo_Prepare(&fh);
			LevelInfo_Entry *entry = LevelInfo_SearchSlot(info, wnum, lnum);
			char *worldname = LevelInfo_GetName(info, entry);
			char *levelname = "";
			
			void *vtable = *((void**)levelObj);
			void *funcaddr = *((void**)((u32)vtable+0x7C));
			int (*SetString)(void*, unsigned short*, unsigned short);
			SetString = (int(*)(void*, unsigned short*, unsigned short))funcaddr;
			
			unsigned short wbuffer[0x40], lbuffer[0x40];
			for (int i = 0; i < 0x40; i++) {
				wbuffer[i] = (unsigned short)worldname[i];
				lbuffer[i] = (unsigned short)levelname[i];
			}
			
			SetString(worldObj, wbuffer, 0);
			SetString(levelObj, lbuffer, 0);
			
			FreeFile(&fh);
		}
	} else {
	}
	
	return state;
}

int DoNamesTest2(int state, u32 ptr) {
	return DoNames(state);
}
