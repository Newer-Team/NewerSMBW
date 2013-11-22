#include "levelinfo_old.h"

void LevelInfo_Prepare(FileHandle *fh) {
	void *file = fh->filePtr;
	
	// decrypt all the level names
	for (int sect = 0; sect < LevelInfo_GetSectionCount(file); sect++) {
		// parse this section
		LevelInfo_Section *thisSect = LevelInfo_GetSection(file, sect);
		LevelInfo_Entry *levels = LevelInfo_GetLevels(file, thisSect);
		
		for (int lev = 0; lev < thisSect->levelCount; lev++) {
			LevelInfo_Entry *level = &levels[lev];
			
			char *name = LevelInfo_GetName(file, level);
			
			for (int i = 0; i < level->nameLength+1; i++) {
				name[i] -= 0xD0;
			}
		}
	}
}

LevelInfo_Entry *LevelInfo_SearchSlot(void *file, int world, int level) {
	for (int i = 0; i < LevelInfo_GetSectionCount(file); i++) {
		LevelInfo_Section *sect = LevelInfo_GetSection(file, i);
		
		for (int j = 0; j < sect->levelCount; j++) {
			LevelInfo_Entry *entry = &LevelInfo_GetLevels(file, sect)[j];
			if (entry->worldSlot == world && entry->levelSlot == level)
				return entry;
		}
	}
	
	return 0;
}
