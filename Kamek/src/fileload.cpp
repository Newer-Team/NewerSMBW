#include "fileload.h"


void *LoadFile(FileHandle *handle, const char *name) {

	int entryNum = DVDConvertPathToEntrynum(name);

	DVDHandle dvdhandle;
	if (!DVDFastOpen(entryNum, &dvdhandle)) {
		return 0;
	}

	handle->length = dvdhandle.length;
	handle->filePtr = EGG__Heap__alloc((handle->length+0x1F) & ~0x1F, 0x20, GetArchiveHeap());

	int ret = DVDReadPrio(&dvdhandle, handle->filePtr, (handle->length+0x1F) & ~0x1F, 0, 2);

	DVDClose(&dvdhandle);


	return handle->filePtr;
}

bool FreeFile(FileHandle *handle) {
	if (!handle) return false;

	if (handle->filePtr) {
		EGG__Heap__free(handle->filePtr, GetArchiveHeap());
	}

	handle->filePtr = 0;
	handle->length = 0;

	return true;
}




File::File() {
	m_loaded = false;
}

File::~File() {
	close();
}

bool File::open(const char *filename) {
	if (m_loaded)
		close();

	void *ret = LoadFile(&m_handle, filename);
	if (ret != 0)
		m_loaded = true;

	return (ret != 0);
}

/*bool File::openCompressed(const char *filename) {
	if (m_loaded)
		close();

	void *ret = LoadCompressedFile(&m_handle, filename);
	if (ret != 0)
		m_loaded = true;

	return (ret != 0);
}*/

void File::close() {
	if (!m_loaded)
		return;

	m_loaded = false;
	FreeFile(&m_handle);
}

bool File::isOpen() {
	return m_loaded;
}

void *File::ptr() {
	if (m_loaded)
		return m_handle.filePtr;
	else
		return 0;
}

u32 File::length() {
	if (m_loaded)
		return m_handle.length;
	else
		return 0xFFFFFFFF;
}

