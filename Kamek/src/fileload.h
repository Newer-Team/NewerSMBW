#ifndef __NEWER_FILELOAD_H
#define __NEWER_FILELOAD_H

#include <common.h>

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


int DVDConvertPathToEntrynum(const char *path);
bool DVDFastOpen(int entrynum, DVDHandle *handle);
int DVDReadPrio(DVDHandle *handle, void *buffer, int length, int offset, int unk);
bool DVDClose(DVDHandle *handle);

// EGG::Heap
void *EGG__Heap__alloc(unsigned long size, int unk, void *heap);
void EGG__Heap__free(void *ptr, void *heap);

void *LoadFile(FileHandle *handle, const char *name);
//void *LoadCompressedFile(FileHandle *handle, const char *name);
bool FreeFile(FileHandle *handle);

void inline *GetArchiveHeap() {
	return ArchiveHeap;

}


// C++ interface
class File {
public:
	File();
	~File();

	bool open(const char *filename);
	void close();

	bool openCompressed(const char *filename);

	bool isOpen();

	void *ptr();
	u32 length();

private:
	FileHandle m_handle;
	bool m_loaded;
};


// Todo: input stream maybe?

#endif

