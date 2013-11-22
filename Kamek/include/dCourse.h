#ifndef __DCOURSE_H
#define __DCOURSE_H

// Proper headers for Course, not the old shit

class dCourse_c {
	public:
		virtual ~dCourse_c();

		void loadFromFile(int courseID);

		struct tilesets_s {
			char name0[32];
			char name1[32];
			char name2[32];
			char name3[32];
		};

		struct settings_s {
			u64 defaultFlags;
			s16 courseFlag;
			s16 timeLimit;
			u8 _[4];
			u8 startEntrance;
			u8 __[3];
		};

		struct bounding_s {
			s32 yBoundNormalTop;
			s32 yBoundNormalBottom;
			s32 yBoundSpecialTop;
			s32 yBoundSpecialBottom;
			u16 entryID;
			u16 lockToBottom;
			u32 unknown;
		};

		struct unk_s {
			u16 unk1;
			u16 unk2;
			u16 unk3;
			u16 unk4;
		};

		struct bg_s {
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

		struct nextGoto_s {
			enum Flag {
				NO_ENTRY = 0x80,
				CONNECTED_PIPE = 8,
				LINK_FORWARD = 4,
				CONNECTED_REVERSE = 1,
			};

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

		struct sprite_s {
			u16 type;
			s16 xPos;
			s16 yPos;
			u16 eventIDs;
			u32 settings;
			u8 zoneID;
			u8 layerID;
			u16 unused;
		};

		struct load_s {
			u16 type;
			u16 unused;
		};

		struct zone_s {
			s16 xPos;
			s16 yPos;
			s16 xSize;
			s16 ySize;
			u16 modelShading;
			u16 terrainShading;
			u8 entryID;
			u8 boundingID;
			u8 scrollMode;
			u8 zoomMode;
			u8 unk10;
			u8 visibility;
			u8 fgID;
			u8 bgID;
			u8 mpBias;
			u8 unk3;
			u8 music;
			u8 audioModifier;
		};

		struct rect_s {
			s16 xPos;
			s16 yPos;
			s16 xSize;
			s16 ySize;
			u8 entryID;
			u8 unused[3];
		};

		struct cameraFlag_s {
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

		struct rail_s {
			enum Flag {
				LOOP = 2,
			};

			u8 entryID;
			u8 unknown;
			u16 startNode;
			u16 nodeCount;
			u16 flags;
		};

		struct railNode_s {
			u16 xPos;
			u16 yPos;
			float speed;
			float accel;
			short unk1;
			short unk2;
		};


		int areaNum;


		union {
			struct {
				tilesets_s *tilesets;
				settings_s *settings;
				bounding_s *bounding;
				unk_s *unk;
				bg_s *topBG;
				bg_s *bottomBG;
				nextGoto_s *nextGoto;
				sprite_s *sprite;
				load_s *load;
				zone_s *zone;
				rect_s *rect;
				cameraFlag_s *cameraFlag;
				rail_s *rail;
				railNode_s *railNode;
			};
			void *blocks[14];
		};

		union {
			struct {
				int tilesetsSize, settingsSize, boundingSize, unkSize;
				int topBGSize, bottomBGSize, nextGotoSize, spriteSize;
				int loadSize, zoneSize, rectSize, cameraFlagSize;
				int railSize, railNodeSize;
			};
			int blockSizes[14];
		};

		union {
			struct {
				int tilesetsCount, settingsCount, boundingCount, unkCount;
				int topBGCount, bottomBGCount, nextGotoCount, spriteCount;
				int loadCount, zoneCount, rectCount, cameraFlagCount;
				int railCount, railNodeCount;
			};
			int blockCounts[14];
		};

		sprite_s *zoneFirstSprite[64];
		int zoneSpriteCount[64];
		int zoneFirstSpriteIdx[64];



		bounding_s *getBoundingByID(u8 id);
		bg_s *getTopBGByID(u16 id);
		bg_s *getBottomBGByID(u16 id);
		nextGoto_s *getNextGotoByID(u8 id);
		zone_s *getZoneByID(u8 id, mRect *output = 0);

		u8 getZoneID(u8 id);
		u8 getBoundingIDForZone(u8 id);
		u8 getScrollForZone(u8 id);
		u8 getZoomForZone(u8 id);
		u8 getUnk10ForZone(u8 id);
		u8 getMusicForZone(u8 id);
		u8 getAudioModifierForZone(u8 id);
		u8 getVisibilityForZone(u8 id);
		u8 getTopBGIDForZone(u8 id);
		u8 getBottomBGIDForZone(u8 id);
		u16 getModelShadingForZone(u8 id);
		u16 getTerrainShadingForZone(u8 id);
		u8 getMPBiasForZone(u8 id);
		u16 getWidthForZone(u8 id);

		rect_s *getRectByID(u8 id, mRect *output = 0);

		u8 getZoneIDContainingPosition(Vec *pos);
		u8 getZoneIDContainingRect(mRect16 *rect);

		bool doesZoneContainPosition(Vec *pos, zone_s *zone);
		bool doesZoneContainRect(mRect16 *rect, zone_s *zone);
};


class dCourseFull_c {
	public:
		dCourse_c courses[4];

		void loadFromFile();

		dCourse_c *get(int id) {
			if (courses[id].zone)
				return &courses[id];
			else
				return 0;
		}

		static dCourseFull_c *instance;

		static void createOnHeap(/*EGG::Heap*/void *heap);
		static void deleteInstance();

		// might not be part of this class, dunno
		static void generateLevelFilename(int world, int level, char *outBuffer);
		static char levelFilename[10];
};

#endif

