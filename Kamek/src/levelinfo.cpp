#include "levelinfo.h"

dDvdLoader_c s_levelInfoLoader;
bool s_levelInfoLoaded = false;

dLevelInfo_c dLevelInfo_c::s_info;

bool LoadLevelInfo() {
	if (s_levelInfoLoaded)
		return true;

	void *data = s_levelInfoLoader.load("/NewerRes/LevelInfo.bin");
	if (data) {
		dLevelInfo_c::s_info.load(data);
		s_levelInfoLoaded = true;
		return true;
	}

	return false;
}


void dLevelInfo_c::load(void *buffer) {
	data = (header_s*)buffer;

	// decode all the level names
	for (int sect = 0; sect < sectionCount(); sect++) {
		// parse this section
		section_s *thisSect = getSectionByIndex(sect);

		for (int lev = 0; lev < thisSect->levelCount; lev++) {
			entry_s *level = &thisSect->levels[lev];

			if (level->levelSlot < 42)
				SetSomeConditionShit(level->worldSlot, level->levelSlot, level->flags);

			char *name = (char*)getNameForLevel(level);

			for (int i = 0; i < level->nameLength+1; i++) {
				name[i] -= 0xD0;
			}
		}
	}
}

dLevelInfo_c::entry_s *dLevelInfo_c::searchBySlot(int world, int level) {
	for (int i = 0; i < sectionCount(); i++) {
		section_s *sect = getSectionByIndex(i);

		for (int j = 0; j < sect->levelCount; j++) {
			entry_s *entry = &sect->levels[j];
			if (entry->worldSlot == world && entry->levelSlot == level)
				return entry;
		}
	}

	return 0;
}

dLevelInfo_c::entry_s *dLevelInfo_c::searchByDisplayNum(int world, int level) {
	for (int i = 0; i < sectionCount(); i++) {
		section_s *sect = getSectionByIndex(i);

		for (int j = 0; j < sect->levelCount; j++) {
			entry_s *entry = &sect->levels[j];
			if (entry->displayWorld == world && entry->displayLevel == level)
				return entry;
		}
	}

	return 0;
}


void UpdateFSStars() {
	dLevelInfo_c *li = &dLevelInfo_c::s_info;
	SaveBlock *save = GetSaveFile()->GetBlock(-1);

	bool coinsNormal = true, exitsNormal = true;
	bool coinsW9 = true, exitsW9 = true;

	for (int i = 0; i < li->sectionCount(); i++) {
		dLevelInfo_c::section_s *sect = li->getSectionByIndex(i);

		for (int j = 0; j < sect->levelCount; j++) {
			dLevelInfo_c::entry_s *entry = &sect->levels[j];

			// Levels only
			if (!(entry->flags & 2))
				continue;

			u32 conds = save->GetLevelCondition(entry->worldSlot, entry->levelSlot);

			if (entry->displayWorld == 9) {
				if ((conds & COND_COIN_ALL) != COND_COIN_ALL)
					coinsW9 = false;
				if (entry->flags & 0x10)
					if (!(conds & COND_NORMAL))
						exitsW9 = false;
				if (entry->flags & 0x20)
					if (!(conds & COND_SECRET))
						exitsW9 = false;
			} else {
				if ((conds & COND_COIN_ALL) != COND_COIN_ALL)
					coinsNormal = false;
				if (entry->flags & 0x10)
					if (!(conds & COND_NORMAL))
						exitsNormal = false;
				if (entry->flags & 0x20)
					if (!(conds & COND_SECRET))
						exitsNormal = false;
			}
		}
	}

	bool beatGame = (save->GetLevelCondition(7, 23) & COND_NORMAL) != 0;

//	save->bitfield &= ~0x3E;
	save->bitfield &= ~0x3C;
	save->bitfield |=
//		(beatGame ? 2 : 0) |
		(exitsNormal ? 4 : 0) |
		(coinsNormal ? 8 : 0) |
		(exitsW9 ? 0x10 : 0) |
		(coinsW9 ? 0x20 : 0);

	OSReport("FS Stars updated: Status: Game beaten: %d, Normal exits: %d, Normal coins: %d, W9 exits: %d, W9 coins: %d\n", beatGame, exitsNormal, coinsNormal, exitsW9, coinsW9);
}

