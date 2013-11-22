#include <game.h>
#include <g3dhax.h>


class SomethingAboutShrooms {
	public:
		m3d::mdl_c models[3];
		float scale, _C4, offsetToEdge;
		m3d::anmTexPat_c animations[3];

		struct info_s {
			const char *leftName;
			const char *middleName;
			const char *rightName;
			float size; // 8 for small, 16 for big
			const char *lrName;
			const char *middleNameAgain;
		};

		void setup(mAllocator_c *allocator,
				nw4r::g3d::ResFile *resFile, info_s *info,
				float length, float colour, float scale);

		// plus more methods I don't know

		void drawWithMatrix(float yOffset, mMtx *matrix);
};


class dRotatorThing_c {
	public:
		s16 _p5, output, _p1, _p2, _p3;
		s16 _p6, _p7, _p0;
		u32 someBool;

		void setup(s16 a, s16 b, s16 c, s16 d, s16 initialRotation, s16 f, s16 g, s16 h);
		s16 execute();
};


class daObjKinoko_c : public dStageActor_c {
	public:
		mHeapAllocator_c allocator;
		nw4r::g3d::ResFile resFile;
		SomethingAboutShrooms renderer;
		StandOnTopCollider colliders[3];
		dRotatorThing_c xRotator;
		dRotatorThing_c zRotator;
		u8 thickness, touchCompare;

		void loadModels(int thickness, float length, float colour, float scale);
		void addAllColliders();
		void updateAllColliders();
		void removeAllColliders();
		u8 checkIfTouchingObject();

		int onCreate();
		int onExecute();
		int onDraw();
		int onDelete();

		int creationHook();
		int drawHook();

		int original_onCreate();

		~daObjKinoko_c();
};


// This will replace Nintendo's onCreate
int daObjKinoko_c::creationHook() {
	original_onCreate();

	// if rotation is off, do nothing else
	if (!((settings >> 28) & 1))
		return 1;

	// OK, figure out the rotation
	u8 sourceRotation = (settings >> 24) & 0xF;

	// 0 is up. -0x4000 is right, 0x4000 is left ...
	s16 rotation;

	// We'll flip it later.
	// Thus: 0..7 rotates left (in increments of 0x800),
	// 8..15 rotates right (in increments of 0x800 too).
	// To specify facing up, well.. just use 0.

	if (sourceRotation < 8)
		rotation = (sourceRotation * 0x800) - 0x4000;
	else
		rotation = (sourceRotation * 0x800) - 0x3800;

	rotation = -rotation;

	rot.z = rotation;

	/* Original code: */
	int lengthInTiles = settings & 0xF;

	float sizeMult = (thickness == 1) ? 1.0f : 0.5f;
	float length = sizeMult * ((32.0f + (lengthInTiles * 16.0f)) - 16.0f);

	float topPos = (sizeMult * 16.0f);

	float cosThing = nw4r::math::CosFIdx((lengthInTiles * 16.0f) / 256.0f);
	float anotherThing = (4.0f + topPos) / cosThing;

	// Middle Collider
	colliders[0].init(this,
			/*xOffset=*/0.0f, /*yOffset=*/0.0f,
			/*topYOffset=*/topPos,
			/*rightSize=*/length, /*leftSize=*/-length,
			/*rotation=*/rotation, /*_45=*/1
			);

	colliders[0]._47 = 0;
	colliders[0].flags = 0x80180 | 0xC00;

	// Now get the info to move the colliders by ....
	float rotFIdx = ((float)rotation) / 256.0f;
	float sinRot, cosRot;
	nw4r::math::SinCosFIdx(&sinRot, &cosRot, rotFIdx);
	//OSReport("Rotation is %d, so rotFIdx is %f\n", rotation, rotFIdx);
	//OSReport("Sin: %f, Cos: %f\n", sinRot, cosRot);
	
	float leftXOffs = (cosRot * -length) - (sinRot * topPos);
	float leftYOffs = (sinRot * -length) + (cosRot * topPos);
	float rightXOffs = (cosRot * length) - (sinRot * topPos);
	float rightYOffs = (sinRot * length) + (cosRot * topPos);
	//OSReport("leftXOffs: %f, leftYOffs: %f\n", leftXOffs, leftYOffs);
	//OSReport("rightXOffs: %f, rightYOffs: %f\n", rightXOffs, rightYOffs);

	// Right Collider
	colliders[1].init(this,
			/*xOffset=*/rightXOffs, /*yOffset=*/rightYOffs,
			/*topYOffset=*/0.0f,
			/*rightSize=*/anotherThing, /*leftSize=*/0.0f,
			/*rotation=*/rotation - 0x2000, /*_45=*/1
			);

	colliders[1]._47 = 0;
	colliders[1].flags = 0x80100 | 0x800;

	// Left Collider
	colliders[2].init(this,
			/*xOffset=*/leftXOffs, /*yOffset=*/leftYOffs,
			/*topYOffset=*/0.0f,
			/*rightSize=*/0.0f, /*leftSize=*/-anotherThing,
			/*rotation=*/rotation + 0x2000, /*_45=*/1
			);

	colliders[2]._47 = 0;
	colliders[2].flags = 0x80080 | 0x400;

	return 1;
}


// This will replace Nintendo's onDraw
int daObjKinoko_c::drawHook() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationZ(&rot.z);
	matrix.applyRotationX(&xRotator.output);
	matrix.applyRotationZ(&zRotator.output);
	renderer.drawWithMatrix(0.0f, &matrix);

	return 1;
}


