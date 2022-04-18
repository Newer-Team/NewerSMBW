struct FileHandle {
	void *filePtr;
	int length;
};

struct DVDHandle {
	int unk1;		// 00
	int unk2;		// 04
	int unk3;		// 08
	int unk4;		// 0C
	int unk5;		// 10
	int unk6;		// 14
	int unk7;		// 18
	int unk8;		// 1C
	int unk9;		// 20
	int unk10;		// 24
	int unk11;		// 28
	int unk12;		// 2C
	int address;	// 30
	int length;		// 34
	int unk13;		// 38
};


void OSReport(const char *format, ...);

typedef struct { unsigned char r, g, b, a; } GXColor;
void OSFatal(GXColor fg, GXColor bg, const char *msg);

int DVDConvertPathToEntrynum(const char *path);
bool DVDFastOpen(int entrynum, DVDHandle *handle);
int DVDReadPrio(DVDHandle *handle, void *buffer, int length, int offset, int unk);
bool DVDClose(DVDHandle *handle);

void *EGG__Heap__alloc(unsigned long size, int unk, void *heap);
void EGG__Heap__free(void *ptr, void *heap);

void inline *GetArchiveHeap() {
#ifdef REGION_PAL
	return *((void**)0x8042A72C);
#endif
#ifdef REGION_NTSC
	return *((void**)0x8042A44C);
#endif
}

extern "C" {
void ApplyKamekPatch(void *buf);
}



bool NewerBootstrap() {
	GXColor errorfg, errorbg;
	errorfg.r = 255;
	errorfg.g = 255;
	errorfg.b = 255;
	errorbg.r = 0;
	errorbg.g = 0;
	errorbg.b = 0;
	
	OSReport("* Loading sysfile.\n");
	
	int entryNum = DVDConvertPathToEntrynum("/NewerRes/System.bin");
	if (entryNum == -1) {
		OSFatal(errorfg, errorbg, "FATAL ERROR: Sysfile not found.");
	}
	
	DVDHandle dvdhandle;
	if (!DVDFastOpen(entryNum, &dvdhandle)) {
		OSFatal(errorfg, errorbg, "FATAL ERROR: Cannot open sysfile.");
	}
	
	OSReport("* Sysfile opened, %d bytes. Reading...\n", dvdhandle.length);
	
	void *filePtr = EGG__Heap__alloc((dvdhandle.length+0x1F) & ~0x1F, 0x20, GetArchiveHeap());
	if (!filePtr) {
		OSFatal(errorfg, errorbg, "FATAL ERROR: Failed to allocate memory.");
	}
	
	int ret = DVDReadPrio(&dvdhandle, filePtr, (dvdhandle.length+0x1F) & ~0x1F, 0, 2);
	
	OSReport("* Reading complete.\n");
	
	ApplyKamekPatch(filePtr);
	
	OSReport("* Sysfile has been loaded.\n");
	
	EGG__Heap__free(filePtr, GetArchiveHeap());
	DVDClose(&dvdhandle);
	
	return true;
}
