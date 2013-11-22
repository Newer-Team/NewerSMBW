#include <game.h>
#include <sfx.h>

//#define REIMPLEMENT

extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);

class daChengeBlock_c : public dStageActor_c {
	static daChengeBlock_c *build();

	u32 _394;
	u64 initialFlags;

	enum Action { Destroy, Create };
	enum Pattern { Fill, CheckerA, CheckerB };

	int width, height;
	Action action;
	int blockType;
	int isPermanent;
	Pattern pattern;
	u16 hasTriggered;

	u32 _3BC;


	int onCreate();
	int onExecute();

	void doStuff(Action action, bool wasCalledOnCreation);
	void tryToTrigger();
};

#ifdef REIMPLEMENT
daChengeBlock_c *daChengeBlock_c::build() {
	return new(AllocFromGameHeap1(sizeof(daChengeBlock_c))) daChengeBlock_c;
}



int daChengeBlock_c::onCreate() {
	hasTriggered = 0;

	height = settings & 0xF;
	width = (settings & 0xF0) >> 4;
	blockType = (settings & 0xF000) >> 12;
	pattern = (Pattern)((settings & 0x30000) >> 16);
	isPermanent = (settings & 0xF0000000) >> 28;

	if (width == 0)
		width++;
	if (height == 0)
		height++;

	action = (Action)((settings & 0xF00) >> 8);

	initialFlags = dFlagMgr_c::instance->flags & dStageActor_c::creatingFlagMask;

	if (initialFlags) {
		if (action == Destroy) {
			doStuff(Destroy, true);
		} else {
			doStuff(Create, true);
			hasTriggered = true;
		}

		if (isPermanent)
			return 2;
	}

	return 1;
}


int daChengeBlock_c::onExecute() {
	tryToTrigger();

	if (!hasTriggered)
		checkZoneBoundaries(0);

	return 1;
}


#endif // REIMPLEMENT

// Red, Brick, Blank/Unused, Stone, Wood, Blank
static const u16 Tiles[] = {124, 2, 12, 123, 15, 0};

void daChengeBlock_c::doStuff(Action action, bool wasCalledOnCreation) {
	u16 perTilePatternFlag = 1, perRowPatternFlag = 1;

	u16 worldX = ((u16)pos.x) & 0xFFF0;
	u16 baseWorldX = worldX;
	u16 worldY = ((u16)(-pos.y)) & 0xFFF0;

	if (pattern == CheckerB) {
		perTilePatternFlag = 0;
		perRowPatternFlag = 0;
	}

	u16 tile;
	if (action != Destroy) {
		if (blockType & 8) {
			// Specify a tile number
			tile = 0x8000 | ((blockType & 3) << 8) | ((settings & 0xFF00000) >> 20);
		} else {
			// fall through
			tile = Tiles[blockType];
		}
	} else {
		tile = 0;
	}

	for (u16 y = 0; y < height; y++) {
		for (u16 x = 0; x < width; x++) {
			if (perTilePatternFlag) {
				u16 *pExistingTile = dBgGm_c::instance->getPointerToTile(worldX, worldY, currentLayerID);
				u16 existingTile = pExistingTile ? *pExistingTile : 0;

				dBgGm_c::instance->placeTile(worldX, worldY, currentLayerID, tile);

				if (!wasCalledOnCreation) {
					Vec effectPos;

					if (action == Destroy) {
						if (blockType != 2) {
							effectPos.x = ((float)(worldX)) + 8.0f;
							effectPos.y = ((float)(-worldY)) - 8.0f;
							effectPos.z = pos.z;

							u16 shardType;
							switch (existingTile) {
								case 0x30: shardType = 0; break;
								case 0x31: shardType = 3; break;
								case 0x32: shardType = 4; break;
								case 0x33: shardType = 2; break;
								case 0x34: shardType = 1; break;
								case 0x37: shardType = 5; break;
								default: shardType = 0xFFFF;
							}

							if (!(settings & 0x40000)) {
								if (shardType == 0xFFFF) {
									SpawnEffect("Wm_en_burst_ss", 0, &effectPos, 0, 0);
								} else {
									u32 sets = (shardType << 8) | 3;
									effectPos.y -= 8;
									dEffectBreakMgr_c::instance->spawnTile(&effectPos, sets, 0);
								}
							}

							if (!(settings & 0x80000)) {
								Vec2 soundPos;
								ConvertStagePositionToScreenPosition(&soundPos, &effectPos);
								SoundPlayingClass::instance2->PlaySoundAtPosition(SE_OBJ_BLOCK_BREAK, &soundPos, 0);
							}
						}
					} else {
						effectPos.x = ((float)(worldX)) + 8.0f;
						effectPos.y = ((float)(-worldY)) - 8.0f;
						effectPos.z = pos.z;

						if (!(settings & 0x40000)) {
							if (blockType != 2) {
								SpawnEffect("Wm_en_burst_ss", 0, &effectPos, 0, 0);
							}
						}
					}
				}
			}

			if (pattern != Fill) {
				perTilePatternFlag ^= 1;
			}

			worldX += 16;
		}

		if (pattern != Fill) {
			perRowPatternFlag ^= 1;
			perTilePatternFlag = perRowPatternFlag;
		}

		worldX = baseWorldX;
		worldY += 16;
	}
}


#ifdef REIMPLEMENT
void daChengeBlock_c::tryToTrigger() {
	u64 result = spriteFlagMask & dFlagMgr_c::instance->flags;

	if (action == Destroy) {
		if (result & initialFlags) {
			if (result) {
				doStuff(Destroy, false);
				hasTriggered = true;
			} else {
				doStuff(Create, false);
				hasTriggered = false;
			}

			initialFlags = result;

			if (isPermanent)
				fBase_c::Delete();
		}
	} else {
		if (result & initialFlags) {
			if (result) {
				doStuff(Create, false);
				hasTriggered = true;
			} else {
				doStuff(Destroy, false);
				hasTriggered = false;
			}

			initialFlags = result;

			if (isPermanent)
				fBase_c::Delete();
		}
	}
}
#endif

