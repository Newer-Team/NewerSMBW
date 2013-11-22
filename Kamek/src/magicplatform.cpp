#include <game.h>
#include <dCourse.h>

class daEnMagicPlatform_c : public dEn_c {
	public:
		static daEnMagicPlatform_c *build();

		int onCreate();
		int onExecute();
		int onDelete();

		enum CollisionType {
			Solid = 0,
			SolidOnTop = 1,
			None = 2,
			ThinLineRight = 3,
			ThinLineLeft = 4,
			ThinLineTop = 5,
			ThinLineBottom = 6,
			NoneWithZ500 = 7
		};

		// Settings
		CollisionType collisionType;
		u8 rectID, moveSpeed, moveDirection, moveLength;

		u8 moveDelay, currentMoveDelay;

		bool doesMoveInfinitely;

		float moveMin, moveMax, moveDelta, moveBaseDelta;
		float *moveTarget;

		bool isMoving;
		void setupMovement();
		void handleMovement();

		Physics physics;
		StandOnTopCollider sotCollider;

		TileRenderer *renderers;
		int rendererCount;

		void findSourceArea();
		void createTiles();
		void deleteTiles();
		void updateTilePositions();

		void checkVisibility();
		void setVisible(bool shown);

		bool isVisible;

		int srcX, srcY;
		int width, height;
};

/*****************************************************************************/
// Glue Code
daEnMagicPlatform_c *daEnMagicPlatform_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daEnMagicPlatform_c));
	daEnMagicPlatform_c *c = new(buffer) daEnMagicPlatform_c;
	return c;
}

extern "C" void HurtMarioBecauseOfBeingSquashed(void *mario, dStageActor_c *squasher, int type);

static void PhysCB1(daEnMagicPlatform_c *one, dStageActor_c *two) {
	if (two->stageActorType != 1)
		return;

	// if left/right
	if (one->moveDirection <= 1)
		return;

	if (one->pos_delta.y > 0.0f)
		HurtMarioBecauseOfBeingSquashed(two, one, 1);
	else
		HurtMarioBecauseOfBeingSquashed(two, one, 9);
}

static void PhysCB2(daEnMagicPlatform_c *one, dStageActor_c *two) {
	if (two->stageActorType != 1)
		return;

	// if left/right
	if (one->moveDirection <= 1)
		return;

	if (one->pos_delta.y < 0.0f)
		HurtMarioBecauseOfBeingSquashed(two, one, 2);
	else
		HurtMarioBecauseOfBeingSquashed(two, one, 10);
}

static void PhysCB3(daEnMagicPlatform_c *one, dStageActor_c *two, bool unkMaybeNotBool) {
	if (two->stageActorType != 1)
		return;

	// if up/down
	if (one->moveDirection > 1)
		return;

	if (unkMaybeNotBool) {
		if (one->pos_delta.x > 0.0f)
			HurtMarioBecauseOfBeingSquashed(two, one, 6);
		else
			HurtMarioBecauseOfBeingSquashed(two, one, 12);
	} else {
		if (one->pos_delta.x < 0.0f)
			HurtMarioBecauseOfBeingSquashed(two, one, 5);
		else
			HurtMarioBecauseOfBeingSquashed(two, one, 11);
	}
}

static bool PhysCB4(daEnMagicPlatform_c *one, dStageActor_c *two) {
	return (one->pos_delta.y > 0.0f);
}

static bool PhysCB5(daEnMagicPlatform_c *one, dStageActor_c *two) {
	return (one->pos_delta.y < 0.0f);
}

static bool PhysCB6(daEnMagicPlatform_c *one, dStageActor_c *two, bool unkMaybeNotBool) {
	if (unkMaybeNotBool) {
		if (one->pos_delta.x > 0.0f)
			return true;
	} else {
		if (one->pos_delta.x < 0.0f)
			return true;
	}
	return false;
}

int daEnMagicPlatform_c::onCreate() {
	rectID = settings & 0xFF;

	moveSpeed = (settings & 0xF00) >> 8;
	moveDirection = (settings & 0x3000) >> 12;
	moveLength = ((settings & 0xF0000) >> 16) + 1;

	moveDelay = ((settings & 0xF00000) >> 20) * 6;

	collisionType = (CollisionType)((settings & 0xF000000) >> 24);

	doesMoveInfinitely = (settings & 0x10000000);

	if (settings & 0xE0000000) {
		int putItBehind = settings >> 29;
		pos.z = -3600.0f - (putItBehind * 16);
	}
	if (collisionType == NoneWithZ500)
		pos.z = 500.0f;

	setupMovement();

	findSourceArea();
	createTiles();

	float fWidth = width * 16.0f;
	float fHeight = height * 16.0f;

	switch (collisionType) {
		case Solid:
			physics.setup(this,
					0.0f, 0.0f, fWidth, -fHeight,
					(void*)&PhysCB1, (void*)&PhysCB2, (void*)&PhysCB3, 1, 0, 0);

			physics.callback1 = (void*)&PhysCB4;
			physics.callback2 = (void*)&PhysCB5;
			physics.callback3 = (void*)&PhysCB6;

			physics.addToList();
			break;
		case SolidOnTop:
			sotCollider.init(this,
					/*xOffset=*/0.0f, /*yOffset=*/0.0f,
					/*topYOffset=*/0,
					/*rightSize=*/fWidth, /*leftSize=*/0,
					/*rotation=*/0, /*_45=*/1
					);

			// What is this for. I dunno
			sotCollider._47 = 0xA;
			sotCollider.flags = 0x80180 | 0xC00;

			sotCollider.addToList();

			break;
		case ThinLineLeft: case ThinLineRight:
		case ThinLineTop: case ThinLineBottom:
			physics.setup(this,
				fWidth * (collisionType == ThinLineRight ? 0.875f : 0.0f),
				fHeight * (collisionType == ThinLineBottom ? -0.75f : 0.0f),
				fWidth * (collisionType == ThinLineLeft ? 0.125f : 1.0f),
				fHeight * (collisionType == ThinLineTop ? -0.25f : -1.0f),
				(void*)&PhysCB1, (void*)&PhysCB2, (void*)&PhysCB3, 1, 0, 0);

			physics.callback1 = (void*)&PhysCB4;
			physics.callback2 = (void*)&PhysCB5;
			physics.callback3 = (void*)&PhysCB6;

			physics.addToList();
			break;
	}

	return 1;
}

int daEnMagicPlatform_c::onDelete() {
	deleteTiles();

	switch (collisionType) {
		case ThinLineLeft: case ThinLineRight:
		case ThinLineTop: case ThinLineBottom:
		case Solid: physics.removeFromList(); break;
	}

	return 1;
}

int daEnMagicPlatform_c::onExecute() {
	handleMovement();

	checkVisibility();

	updateTilePositions();

	switch (collisionType) {
		case ThinLineLeft: case ThinLineRight:
		case ThinLineTop: case ThinLineBottom:
		case Solid: physics.update(); break;
		case SolidOnTop: sotCollider.update(); break;
	}

	return 1;
}

/*****************************************************************************/
// Movement
void daEnMagicPlatform_c::setupMovement() {
	float fMoveLength = 16.0f * moveLength;
	float fMoveSpeed = 0.2f * moveSpeed;

	switch (moveDirection) {
		case 0: // RIGHT
			moveTarget = &pos.x;
			moveMin = pos.x;
			moveMax = pos.x + fMoveLength;
			moveBaseDelta = fMoveSpeed;
			break;
		case 1: // LEFT
			moveTarget = &pos.x;
			moveMin = pos.x - fMoveLength;
			moveMax = pos.x;
			moveBaseDelta = -fMoveSpeed;
			break;
		case 2: // UP
			moveTarget = &pos.y;
			moveMin = pos.y;
			moveMax = pos.y + fMoveLength;
			moveBaseDelta = fMoveSpeed;
			break;
		case 3: // DOWN
			moveTarget = &pos.y;
			moveMin = pos.y - fMoveLength;
			moveMax = pos.y;
			moveBaseDelta = -fMoveSpeed;
			break;
	}

	if (spriteFlagNum == 0) {
		isMoving = (moveSpeed > 0);
		moveDelta = moveBaseDelta;
	} else {
		isMoving = false;
	}

	currentMoveDelay = 0;
}

void daEnMagicPlatform_c::handleMovement() {
	if (spriteFlagNum > 0) {
		// Do event checks
		bool flagOn = ((dFlagMgr_c::instance->flags & spriteFlagMask) != 0);

		if (isMoving) {
			if (!flagOn) {
				// Flag was turned off while moving, so go back
				moveDelta = -moveBaseDelta;
			} else {
				moveDelta = moveBaseDelta;
			}
		} else {
			if (flagOn) {
				// Flag was turned on, so start moving
				moveDelta = moveBaseDelta;
				isMoving = true;
			}
		}
	}

	if (!isMoving)
		return;

	if (currentMoveDelay > 0) {
		currentMoveDelay--;
		return;
	}

	// Do it
	bool goesForward = (moveDelta > 0.0f);
	bool reachedEnd = false;

	*moveTarget += moveDelta;

	// if we're set to move infinitely, never stop
	if (doesMoveInfinitely)
		return;

	if (goesForward) {
		if (*moveTarget >= moveMax) {
			*moveTarget = moveMax;
			reachedEnd = true;
		}
	} else {
		if (*moveTarget <= moveMin) {
			*moveTarget = moveMin;
			reachedEnd = true;
		}
	}

	if (reachedEnd) {
		if (spriteFlagNum > 0) {
			// If event, just do nothing.. depending on what side we are on
			if ((moveDelta > 0.0f && moveBaseDelta > 0.0f) || (moveDelta < 0.0f && moveBaseDelta < 0.0f)) {
				// We reached the end, so keep isMoving on for when we need to reverse
			} else {
				// We're back at the start, so turn it off
				isMoving = false;
			}
		} else {
			// Otherwise, reverse
			moveDelta = -moveDelta;
			currentMoveDelay = moveDelay;
		}
	}
}

/*****************************************************************************/
// Tile Renderers

void daEnMagicPlatform_c::findSourceArea() {
	mRect rect;
	dCourseFull_c::instance->get(GetAreaNum())->getRectByID(rectID, &rect);

	// Round the positions down/up to get the rectangle
	int left = rect.x;
	int right = left + rect.width;
	int top = -rect.y;
	int bottom = top + rect.height;

	left &= 0xFFF0;
	right = (right + 15) & 0xFFF0;

	top &= 0xFFF0;
	bottom = (bottom + 15) & 0xFFF0;

	// Calculate the actual stuff
	srcX = left >> 4;
	srcY = top >> 4;
	width = (right - left) >> 4;
	height = (bottom - top) >> 4;

	//OSReport("Area: %f, %f ; %f x %f\n", rect.x, rect.y, rect.width, rect.height);
	//OSReport("Source: %d, %d ; Size: %d x %d\n", srcX, srcY, width, height);
}


void daEnMagicPlatform_c::createTiles() {
	rendererCount = width * height;
	renderers = new TileRenderer[rendererCount];

	// copy all the fuckers over
	int baseWorldX = srcX << 4, worldY = srcY << 4, rendererID = 0;

	for (int y = 0; y < height; y++) {
		int worldX = baseWorldX;

		for (int x = 0; x < width; x++) {
			u16 *pExistingTile = dBgGm_c::instance->getPointerToTile(worldX, worldY, 0);

			if (*pExistingTile > 0) {
				TileRenderer *r = &renderers[rendererID];
				r->tileNumber = *pExistingTile;
				r->z = pos.z;
			}

			worldX += 16;
			rendererID++;
		}

		worldY += 16;
	}

}

void daEnMagicPlatform_c::deleteTiles() {
	if (renderers != 0) {
		setVisible(false);

		delete[] renderers;
		renderers = 0;
	}
}

void daEnMagicPlatform_c::updateTilePositions() {
	float baseX = pos.x;

	float y = -pos.y;

	int rendererID = 0;

	for (int yIdx = 0; yIdx < height; yIdx++) {
		float x = baseX;

		for (int xIdx = 0; xIdx < width; xIdx++) {
			TileRenderer *r = &renderers[rendererID];
			r->x = x;
			r->y = y;

			x += 16.0f;
			rendererID++;
		}

		y += 16.0f;
	}
}



void daEnMagicPlatform_c::checkVisibility() {
	float effectiveLeft = pos.x, effectiveRight = pos.x + (width * 16.0f);
	float effectiveBottom = pos.y - (height * 16.0f), effectiveTop = pos.y;

	ClassWithCameraInfo *cwci = ClassWithCameraInfo::instance;

	float screenRight = cwci->screenLeft + cwci->screenWidth;
	float screenBottom = cwci->screenTop - cwci->screenHeight;

	bool isOut = (effectiveLeft > screenRight) ||
		(effectiveRight < cwci->screenLeft) ||
		(effectiveTop < screenBottom) ||
		(effectiveBottom > cwci->screenTop);

	setVisible(!isOut);
}

void daEnMagicPlatform_c::setVisible(bool shown) {
	if (isVisible == shown)
		return;
	isVisible = shown;

	TileRenderer::List *list = dBgGm_c::instance->getTileRendererList(0);

	for (int i = 0; i < rendererCount; i++) {
		if (renderers[i].tileNumber > 0) {
			if (shown) {
				list->add(&renderers[i]);
			} else {
				list->remove(&renderers[i]);
			}
		}
	}
}

