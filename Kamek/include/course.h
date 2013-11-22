#ifndef __KAMEK_COURSE_H
#define __KAMEK_COURSE_H

#include <common.h>

struct BGDatObject_t {
	s16 objType;
	s16 x;
	s16 y;
	s16 width;
	s16 height;
};

struct Block1 {
	char tileset0[32];
	char tileset1[32];
	char tileset2[32];
	char tileset3[32];
};

#define COURSE_FLAG_WRAP 1

struct Block2 {
	u64 defaultEvents;
	s16 courseFlags;
	s16 timeLimit;
	u8 unk1;
	u8 unk2;
	u8 unk3;
	u8 unk4;
	u8 startEntrance;
	u8 unk5;
	u8 unk6;
	u8 unk7;
};

struct Block3 {
	s32 yBoundNormalTop;
	s32 yBoundNormalBottom;
	s32 yBoundSpecialTop;
	s32 yBoundSpecialBottom;
	u16 entryID;
	u16 lockToBottom;
	u32 unknown;
};

struct Block4 {
	u16 unk1;
	u16 unk2;
	u16 unk3;
	u16 unk4;
};

struct BGSettings {
	u16 entryID;
	u16 xScrollRate;
	u16 yScrollRate;
	u16 yPosOffset;
	u16 xPosOffset;
	u16 fileID1;
	u16 fileID2;
	u16 fileID3;
	u16 unk1;
	u16 scale;
	u32 unk2;
};

#define ENTRANCE_FLAG_NO_ENTRY 0x80
#define ENTRANCE_FLAG_CONNECTED_PIPE 8
#define ENTRANCE_FLAG_LINK_TO_FORWARD 4
#define ENTRANCE_FLAG_CONNECTED_REVERSE 1

struct Block7 {
	s16 xPos;
	s16 yPos;
	s16 cameraXPos;
	s16 cameraYPos;
	u8 entryID;
	u8 destArea;
	u8 destEntrance;
	u8 type;
	u8 unk1;
	u8 zoneID;
	u8 layerID;
	u8 pathID;
	u16 flags;
	u16 unk2;
};

struct Block8 {
	u16 type;
	s16 xPos;
	s16 yPos;
	u16 eventIDs;
	u32 settings;
	u8 zoneID;
	u8 layerID;
	u16 unused;
};

struct Block9 {
	u16 type;
	u16 unused;
};

struct Block10 {
	s16 xPos;
	s16 yPos;
	s16 xSize;
	s16 ySize;
	s16 objShading;
	s16 bgShading;
	u8 zoneID;
	u8 boundingID;
	u8 scrollMode;
	u8 zoomMode;
	u8 unk1;
	u8 lightMode;
	u8 fgID;
	u8 bgID;
	u8 unk2;
	u8 unk3;
	u8 music;
	u8 audioModifier;
};

struct Block11 {
	s16 xPos;
	s16 yPos;
	s16 xSize;
	s16 ySize;
	u8 entryID;
	u8 unused[3];
};

struct Block12 {
	u32 unk1;
	u32 unk2;
	u32 unk3;
	u8 unk4;
	u8 scrollMode;
	u8 zoomMode;
	u8 unk5;
	u16 unk6;
	u8 eventID;
	u8 unk7;
};

#define PATH_FLAG_LOOP 2

struct Block13 {
	u8 pathID;
	u8 unknown;
	u16 startNode;
	u16 nodeCount;
	u16 flags;
};

struct Block14 {
	u16 xPos;
	u16 yPos;
	float speed;
	float accel;
	short unk1;
	short unk2;
};

#endif
