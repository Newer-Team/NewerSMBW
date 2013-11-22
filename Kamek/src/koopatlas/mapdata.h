#ifndef __KOOPATLAS_MAPDATA_H
#define __KOOPATLAS_MAPDATA_H

#include <game.h>
#include <g3dhax.h>

// forward declarations
struct dKPLayer_s;

class dKPNodeExtra_c;

/******************************************************************************
 * Doodads
 ******************************************************************************/
struct dKPDoodad_s {
	struct animation_s {
		enum LoopTypes {
			CONTIGUOUS, LOOP, REVERSE_LOOP
		};

		enum CurveTypes {
			LINEAR, SIN, COS
		};

		enum AnimTypes {
			X_POS, Y_POS, ANGLE, X_SCALE, Y_SCALE, OPACITY
		};

		LoopTypes loop;
		CurveTypes curve;
		int frameCount;
		AnimTypes type;
		int start, end;
		int delay, delayOffset;

		u32 baseTick;
		bool isReversed;
	};

	float x, y;
	float width, height;
	float angle;

	GXTexObj *texObj;
	int animationCount;
	animation_s animations[1]; // variable size
};

/******************************************************************************
 * Paths
 ******************************************************************************/
struct dKPPath_s;

struct dKPNode_s {
	enum NodeTypes {
		PASS_THROUGH, STOP, LEVEL, CHANGE, WORLD_CHANGE
	};

	short x, y;
	union {
		dKPPath_s *exits[4];
		struct {
			dKPPath_s *leftExit;
			dKPPath_s *rightExit;
			dKPPath_s *upExit;
			dKPPath_s *downExit;
		};
	};
	dKPLayer_s *tileLayer, *doodadLayer;

	u8 reserved1, reserved2, reserved3;
	u8 type;

	bool isNew;

	dKPNodeExtra_c *extra;

	// The union is placed at the very end so we can leave out padding in the
	// kpbin
	union {
		struct { u8 levelNumber[2]; bool hasSecret; };
		struct { const char *destMap; u8 thisID, foreignID, transition, _; };
		struct { u8 worldID, __[3]; };
	};

	dKPPath_s *getAnyExit() {
		for (int i = 0; i < 4; i++)
			if (exits[i])
				return exits[i];
		return 0;
	}

	bool isUnlocked();
	void setupNodeExtra();

	dKPPath_s *getOppositeExitTo(dKPPath_s *path, bool mustBeAvailable=false);
	dKPPath_s *getOppositeAvailableExitTo(dKPPath_s *path) {
		return getOppositeExitTo(path, true);
	}

	int getExitCount(bool mustBeAvailable=false);
	int getAvailableExitCount() {
		return getExitCount(true);
	}

	void setLayerAlpha(u8 alpha);
};

struct dKPPath_s {
	enum Availability {
		NOT_AVAILABLE = 0,
		AVAILABLE = 1,
		NEWLY_AVAILABLE = 2,
		ALWAYS_AVAILABLE = 3
	};

	enum Animation {
		WALK = 0, WALK_SAND = 1, WALK_SNOW = 2, WALK_WATER = 3,
		JUMP = 4, JUMP_SAND = 5, JUMP_SNOW = 6, JUMP_WATER = 7,
		LADDER = 8, LADDER_LEFT = 9, LADDER_RIGHT = 10, FALL = 11,
		SWIM = 12, RUN = 13, PIPE = 14, DOOR = 15,
		TJUMPED = 16, ENTER_CAVE_UP = 17, RESERVED_18 = 18, INVISIBLE = 19,
		MAX_ANIM = 20
	};

	dKPNode_s *start, *end;
	dKPLayer_s *tileLayer, *doodadLayer;

	dKPNode_s *getOtherNodeTo(dKPNode_s *n) {
		return (n == start) ? end : start;
	}

	u8 isAvailable; // computed on-the-fly - default from Koopatlas is NOT or ALWAYS
	u8 isSecret;
	u8 _padding[2];
	float speed;
	Animation animation;

	void setLayerAlpha(u8 alpha);
};

/******************************************************************************
 * Tying It All Together
 ******************************************************************************/
struct dKPLayer_s {
	enum LayerTypes {
		OBJECTS, DOODADS, PATHS
	};

	LayerTypes type;
	u8 alpha;
	u8 _padding[3];

	typedef u16 sector_s[16][16];

	union {
		struct {
			GXTexObj *tileset;
			union {
				int sectorBounds[4];
				struct {
					int sectorLeft;
					int sectorTop;
					int sectorRight;
					int sectorBottom;
				};
			};
			union {
				int bounds[4];
				struct {
					int left;
					int top;
					int right;
					int bottom;
				};
			};
			u16 indices[1]; // variable size
		};

		struct {
			int doodadCount;
			dKPDoodad_s *doodads[1]; // variable size
		};

		struct {
			int nodeCount;
			dKPNode_s **nodes;
			int pathCount;
			dKPPath_s **paths;
		};
	};

	int findNodeID(dKPNode_s *node);
};

struct dKPWorldDef_s {
	const char *name;
	GXColor fsTextColours[2];
	GXColor fsHintColours[2];
	GXColor hudTextColours[2];
	u16 hudHintH;
	s8 hudHintS, hudHintL;
	u8 key, trackID;
	u8 worldID;
	u8 titleScreenWorld;
	u8 titleScreenLevel;
	u8 padding[3];
};

struct dKPMapFile_s {
	u32 magic;
	int version;

	int layerCount;
	dKPLayer_s **layers;

	int tilesetCount;
	GXTexObj *tilesets;

	u8 *unlockData;

	dKPLayer_s::sector_s *sectors;

	const char *backgroundName;

	dKPWorldDef_s *worlds;
	int worldCount;
};

class dKPMapData_c {
	private:
		dDvdLoader_c m_fileLoader;

		template <typename T>
			inline T* fixRef(T*& indexAsPtr) {
				unsigned int index = (unsigned int)indexAsPtr;
				if (index == 0xFFFFFFFF || index == 0)
					indexAsPtr = 0;
				else
					indexAsPtr = (T*)(((char*)data) + index);
				return indexAsPtr;
			}

		template <typename T>
			inline T* fixRefSafe(T*& indexAsPtr) {
				unsigned int index = (unsigned int)indexAsPtr;
				if (index == 0xFFFFFFFF || index == 0)
					indexAsPtr = 0;
				else if (index < 0x80000000)
					indexAsPtr = (T*)(((char*)data) + index);
				return indexAsPtr;
			}

		inline void fixTexObjSafe(GXTexObj *obj) {
			if (obj->val[3] >= 0x10000000) {
				obj->val[3] = (((u32)data) & 0x7FFFFFFF) + (obj->val[3] - 0x10000000);
				obj->val[3] >>= 5;
			}
		}

		bool fixedUp;
		bool tilesetsLoaded;

		void fixup();

		bool loadTilesets();
		void unloadTilesets();


	public:
		dDvdLoader_c *tilesetLoaders;
		dKPMapFile_s *data;
		dKPLayer_s *pathLayer;

		dDvdLoader_c bgLoader;

		dKPNodeExtra_c *levelNodeExtraArray;

		const dKPWorldDef_s *findWorldDef(int id) const;

		dKPMapData_c();
		bool load(const char *filename);
		~dKPMapData_c();
};


// Currently only available for LEVEL nodes
class dKPNodeExtra_c {
	public:
		mHeapAllocator_c mallocator;
		mMtx matrix;
		m3d::mdl_c model;
};

#endif

