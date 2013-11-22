#ifndef __NEWER_LEVELINFO_OLD_H
#define __NEWER_LEVELINFO_OLD_H

#include <common.h>
#include "fileload.h"

struct LevelInfo_Header {
	u32 magic;
	u32 sectionCount;
};

struct LevelInfo_Section {
	u32 levelCount;
};

struct LevelInfo_Entry {
	u8 worldSlot;
	u8 levelSlot;
	u8 displayWorld;
	u8 displayLevel;
	u8 nameLength;
	u8 reserved3;
	u16 flags;
	u32 nameOffset;
};

inline u32 LevelInfo_GetSectionCount(void *file) {
	return ((LevelInfo_Header*)file)->sectionCount;
}

inline u32 *LevelInfo_GetOffsets(void *file) {
	return (u32*)(((LevelInfo_Header*)file)+1);
}

inline LevelInfo_Section *LevelInfo_GetSection(void *file, int id) {
	u32 offs = LevelInfo_GetOffsets(file)[id];
	return (LevelInfo_Section*)(((char*)file)+offs);
};

inline LevelInfo_Entry *LevelInfo_GetLevels(void *file, LevelInfo_Section *section) {
	return (LevelInfo_Entry*)(section+1);
}

inline LevelInfo_Entry *LevelInfo_GetLevels(void *file, int sectionID) {
	return (LevelInfo_Entry*)(LevelInfo_GetSection(file, sectionID)+1);
}

inline char *LevelInfo_GetName(void *file, LevelInfo_Entry *entry) {
	return ((char*)file)+entry->nameOffset;
}

void LevelInfo_Prepare(FileHandle *fh);
LevelInfo_Entry *LevelInfo_SearchSlot(void *file, int world, int level);


#endif
