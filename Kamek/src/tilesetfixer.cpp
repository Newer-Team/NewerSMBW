#include <common.h>
#include <game.h>

const char *GetTilesetName(void *cls, int areaNum, int slotNum);

void DoFixes(int areaNumber, int slotNumber);
void SwapObjData(u8 *data, int slotNumber);

extern "C" void *OriginalTilesetLoadingThing(void *, void *, int, int);

// Main hook
void *TilesetFixerHack(void *cls, void *heap, int areaNum, int layerNum) {
	if (layerNum == 0) {
		for (int i = 1; i < 4; i++) {
			DoFixes(areaNum, i);
		}
	}

	return OriginalTilesetLoadingThing(cls, heap, areaNum, layerNum);
}



// File format definitions
struct ObjLookupEntry {
	u16 offset;
	u8 width;
	u8 height;
};


void DoFixes(int areaNumber, int slotNumber) {
	// This is where it all starts
	const char *tsName = GetTilesetName(BGDatClass, areaNumber, slotNumber);

	if (tsName == 0 || tsName[0] == 0) {
		return;
	}


	char untHDname[64], untname[64];
	snprintf(untHDname, 64, "BG_unt/%s_hd.bin", tsName);
	snprintf(untname, 64, "BG_unt/%s.bin", tsName);

	u32 unt_hd_length;
	void *bg_unt_hd_data = DVD_GetFile(GetDVDClass2(), tsName, untHDname, &unt_hd_length);
	void *bg_unt = DVD_GetFile(GetDVDClass2(), tsName, untname);


	ObjLookupEntry *lookups = (ObjLookupEntry*)bg_unt_hd_data;

	int objCount = unt_hd_length / sizeof(ObjLookupEntry);

	for (int i = 0; i < objCount; i++) {
		// process each object
		u8 *thisObj = (u8*)((u32)bg_unt + lookups[i].offset);

		SwapObjData(thisObj, slotNumber);
	}
}


void SwapObjData(u8 *data, int slotNumber) {
	// rudimentary parser which will hopefully work

	while (*data != 0xFF) {
		u8 cmd = *data;

		if (cmd == 0xFE || (cmd & 0x80) != 0) {
			data++;
			continue;
		}

		if ((data[2] & 3) != 0) {
			data[2] &= 0xFC;
			data[2] |= slotNumber;
		}
		data += 3;
	}

}

