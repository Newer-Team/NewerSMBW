#include <game.h>

class RandomTileData {
public:
	enum Type {
		CHECK_NONE = 0,
		CHECK_HORZ = 1,
		CHECK_VERT = 2,
		CHECK_BOTH = 3
	};

	enum Special {
		SP_NONE = 0,
		SP_VDOUBLE_TOP = 1,
		SP_VDOUBLE_BOTTOM = 2
	};

	class NameList {
	public:
		u32 count;
		u32 offsets[1]; // variable size

		const char *getName(int index) {
			return ((char*)this) + offsets[index];
		}

		bool contains(const char *name) {
			for (int i = 0; i < count; i++) {
				if (strcmp(name, getName(i)) == 0)
					return true;
			}

			return false;
		}
	};

	class Entry {
	public:
		u8 lowerBound, upperBound;
		u8 count, type;
		u32 tileNumOffset;

		u8 *getTileNums() {
			return ((u8*)this) + tileNumOffset;
		}
	};

	class Section {
	public:
		u32 nameListOffset;
		u32 entryCount;
		Entry entries[1]; // variable size

		NameList *getNameList() {
			return (NameList*)(((u32)this) + nameListOffset);
		}
	};

	u32 magic;
	u32 sectionCount;
	u32 offsets[1]; // variable size

	Section *getSection(int id) {
		return (Section*)(((char*)this) + offsets[id]);
	}

	Section *getSection(const char *name);

	static RandomTileData *instance;
};

class RTilemapClass : public TilemapClass {
public:
	// NEWER ADDITIONS
	RandomTileData::Section *sections[4];
};

RandomTileData::Section *RandomTileData::getSection(const char *name) {
	for (int i = 0; i < sectionCount; i++) {
		RandomTileData::Section *sect = getSection(i);

		if (sect->getNameList()->contains(name))
			return sect;
	}

	return 0;
}


// Real tile handling code

RandomTileData *RandomTileData::instance = 0;

dDvdLoader_c RandTileLoader;

// This is a bit hacky but I'm lazy
bool LoadLevelInfo();

extern "C" bool RandTileLoadHook() {
	// OSReport("Trying to load...");
	void *buf = RandTileLoader.load("/NewerRes/RandTiles.bin");
	bool LIresult = LoadLevelInfo();
	if (buf == 0) {
		// OSReport("Failed.\n");
		return false;
	} else {
		// OSReport("Successfully loaded RandTiles.bin [%p].\n", buf);
		RandomTileData::instance = (RandomTileData*)buf;
		return LIresult;
	}
}


extern "C" void IdentifyTilesets(RTilemapClass *self) {
	self->_C0C = 0xFFFFFFFF;

	for (int i = 0; i < 4; i++) {
		const char *tilesetName = BGDatClass::instance->getTilesetName(self->areaID, i);

		self->sections[i] = RandomTileData::instance->getSection(tilesetName);
		// OSReport("[%d] Chose %p for %s\n", i, self->sections[i], tilesetName);
	}
}

extern "C" void TryAndRandomise(RTilemapClass *self, BGRender *bgr) {
	int fullTile = bgr->tileToPlace & 0x3FF;
	int tile = fullTile & 0xFF;
	int tileset = fullTile >> 8;

	RandomTileData::Section *rtSect = self->sections[tileset];
	if (rtSect == 0)
		return;

	for (int i = 0; i < rtSect->entryCount; i++) {
		RandomTileData::Entry *entry = &rtSect->entries[i];

		if (tile >= entry->lowerBound && tile <= entry->upperBound) {
			// Found it!!
			// Try to make one until we meet the conditions
			u8 type = entry->type & 3;
			u8 special = entry->type >> 2;

			u8 *tileNums = entry->getTileNums();
			u16 chosen = 0xFF;

			// If it's the top special, then ignore this tile, we'll place that one
			// once we choose the bottom one
			if (special == RandomTileData::SP_VDOUBLE_TOP)
				break;

			u16 *top = 0, *left = 0, *right = 0, *bottom = 0;
			if (type == RandomTileData::CHECK_HORZ || type == RandomTileData::CHECK_BOTH) {
				left = self->getPointerToTile((bgr->curX - 1) * 16, bgr->curY * 16);
				right = self->getPointerToTile((bgr->curX + 1) * 16, bgr->curY * 16);
			}

			if (type == RandomTileData::CHECK_VERT || type == RandomTileData::CHECK_BOTH) {
				top = self->getPointerToTile(bgr->curX * 16, (bgr->curY - 1) * 16);
				bottom = self->getPointerToTile(bgr->curX * 16, (bgr->curY + 1) * 16);
			}

			int attempts = 0;
			while (true) {
				// is there even a point to using that special random function?
				chosen = (tileset << 8) | tileNums[MakeRandomNumberForTiles(entry->count)];

				// avoid infinite loops
				attempts++;
				if (attempts > 5)
					break;

				if (top != 0 && *top == chosen)
					continue;
				if (bottom != 0 && *bottom == chosen)
					continue;
				if (left != 0 && *left == chosen)
					continue;
				if (right != 0 && *right == chosen)
					continue;
				break;
			}

			bgr->tileToPlace = chosen;

			if (special == RandomTileData::SP_VDOUBLE_BOTTOM) {
				if (top == 0)
					top = self->getPointerToTile(bgr->curX * 16, (bgr->curY - 1) * 16);

				*top = (chosen - 0x10);
			}

			return;
		}
	}
}


