#include <game.h>

class daBonePiece_c : public dStageActor_c {
	public:
		static daBonePiece_c *build();

		int onCreate();
		int onExecute();
		int onDraw();
		int onDelete();

		StandOnTopCollider collider;

		nw4r::g3d::ResFile resFile;
		mHeapAllocator_c allocator;
		m3d::mdl_c model;
};

/*****************************************************************************/
// Glue Code
daBonePiece_c *daBonePiece_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daBonePiece_c));
	daBonePiece_c *c = new(buffer) daBonePiece_c;
	return c;
}

int daBonePiece_c::onCreate() {
	// load the model
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	resFile.data = getResource("lift_torokko", "g3d/t00.brres");

	static char thing[] = "lift_torokko?";
	thing[0xC] = 'A' + (settings & 3);

	nw4r::g3d::ResMdl resmdl = resFile.GetResMdl(thing);
	model.setup(resmdl, &allocator, 0, 1, 0);
	SetupTextures_MapObj(&model, 0);

	allocator.unlink();

	// if rotation is off, do nothing else
	if ((settings >> 28) & 1) {
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
	}

	if ((settings >> 20) & 1) {
		rot.y = 0x8000;
	}

	collider.init(this,
			/*xOffset=*/0.0f, /*yOffset=*/0.0f,
			/*topYOffset=*/0,
			/*rightSize=*/16.0f, /*leftSize=*/-16.0f,
			/*rotation=*/rot.z, /*_45=*/1
			);

	collider._47 = 0xA;
	collider.flags = 0x80180 | 0xC00;

	collider.addToList();

	return true;
}

int daBonePiece_c::onDelete() {
	return true;
}

int daBonePiece_c::onExecute() {
	matrix.translation(pos.x, pos.y - 8.0f, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	model.setDrawMatrix(matrix);
	model.setScale(&scale);
	model.calcWorld(false);

	collider.update();

	return true;
}

int daBonePiece_c::onDraw() {
	model.scheduleForDrawing();

	return true;
}



