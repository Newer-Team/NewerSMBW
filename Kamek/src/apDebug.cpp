#include <game.h>

class APDebugDrawer : public m3d::proc_c {
	public:
		APDebugDrawer();

		bool amISetUp;
		mHeapAllocator_c allocator;

		void setMeUp();

		void drawMe();

		void drawOpa();
		void drawXlu();
};


static APDebugDrawer defaultInstance;
bool enableDebugMode = false;

int APDebugDraw() {
	if (enableDebugMode)
		defaultInstance.drawMe();
	return 1;
}


APDebugDrawer::APDebugDrawer() {
	amISetUp = false;
}

void APDebugDrawer::setMeUp() {
	allocator.setup(GameHeaps[0], 0x20);
	setup(&allocator);
}

void APDebugDrawer::drawMe() {
	if (!amISetUp) {
		setMeUp();
		amISetUp = true;
	}

	scheduleForDrawing();
}

void APDebugDrawer::drawOpa() {
	drawXlu();
}
void APDebugDrawer::drawXlu() {
	GXClearVtxDesc();

	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	GXSetNumIndStages(0);
	for (int i = 0; i < 0x10; i++)
		GXSetTevDirect(i);

	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanAmbColor(GX_COLOR0A0, (GXColor){255,255,255,255});
	GXSetChanMatColor(GX_COLOR0A0, (GXColor){255,255,255,255});
	GXSetNumTexGens(0);

	GXSetNumTevStages(1);
	GXSetNumIndStages(0);

	GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);

	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
//	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C1, GX_CC_C0, GX_CC_RASC, GX_CC_ZERO);
//	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
//	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_RASA, GX_CA_ZERO);
//	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

	GXSetZCompLoc(GX_FALSE);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
	GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

	GXSetFog(GX_FOG_NONE, 0, 0, 0, 0, (GXColor){0,0,0,0});
	GXSetFogRangeAdj(GX_FALSE, 0, 0);

	GXSetCullMode(GX_CULL_NONE);

	GXSetDither(GX_TRUE);
	GXSetLineWidth(18, GX_TO_ZERO);

	GXSetTevColor(GX_TEVREG0, (GXColor){255,255,255,255});
	GXSetTevColor(GX_TEVREG1, (GXColor){0,0,0,255});

	nw4r::g3d::Camera cam(GetCameraByID(GetCurrentCameraID()));
	Mtx matrix;
	cam.GetCameraMtx(&matrix);
	GXLoadPosMtxImm(matrix, 0);
	GXSetCurrentMtx(0);

	ActivePhysics *ap = ActivePhysics::globalListHead;

	while (ap) {
//		if (ap->owner->name == PLAYER)
//			OSReport("Player has : DistToC=%f,%f DistToEdge=%f,%f Pos=%f,%f Scale=%f,%f\n",
//					ap->info.xDistToCenter, ap->info.yDistToCenter,
//					ap->info.xDistToEdge, ap->info.yDistToEdge,
//					ap->owner->pos.x, ap->owner->pos.y,
//					ap->owner->scale.x, ap->owner->scale.y);

		u32 uptr = (u32)ap;
		u8 r = (uptr>>16)&0xFF;
		u8 g = (uptr>>8)&0xFF;
		u8 b = uptr&0xFF;
		u8 a = 0xFF;

		GXBegin(GX_LINES, GX_VTXFMT0, 10);

		float centreX = ap->owner->pos.x + ap->info.xDistToCenter;
		float centreY = ap->owner->pos.y + ap->info.yDistToCenter;
		float edgeDistX = ap->info.xDistToEdge;
		float edgeDistY = ap->info.yDistToEdge;

		float tlX = centreX - edgeDistX, tlY = centreY + edgeDistY;
		float trX = centreX + edgeDistX, trY = centreY + edgeDistY;

		float blX = centreX - edgeDistX, blY = centreY - edgeDistY;
		float brX = centreX + edgeDistX, brY = centreY - edgeDistY;

		switch (ap->collisionCheckType) {
			case 2: // vert trapezoid
				tlY = centreY + ap->trpValue0;
				trY = centreY + ap->trpValue1;
				blY = centreY + ap->trpValue2;
				brY = centreY + ap->trpValue3;
				break;
			case 3: // horz trapezoid
				tlX = centreX + ap->trpValue0;
				trX = centreX + ap->trpValue1;
				blX = centreX + ap->trpValue2;
				brX = centreX + ap->trpValue3;
				break;
		}

		// Top
		GXPosition3f32(tlX, tlY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(trX, trY, 9000.0f);
		GXColor4u8(r,g,b,a);

		// Left
		GXPosition3f32(tlX, tlY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(blX, blY, 9000.0f);
		GXColor4u8(r,g,b,a);

		// Right
		GXPosition3f32(trX, trY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(brX, brY, 9000.0f);
		GXColor4u8(r,g,b,a);

		// Bottom
		GXPosition3f32(blX, blY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(brX, brY, 9000.0f);
		GXColor4u8(r,g,b,a);

		// Diagonal
		GXPosition3f32(trX, trY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(blX, blY, 9000.0f);
		GXColor4u8(r,g,b,a);

		GXEnd();

		ap = ap->listPrev;
	}

	Physics *p = Physics::globalListHead;

	while (p) {
		u32 uptr = (u32)p;
		u8 r = (uptr>>16)&0xFF;
		u8 g = (uptr>>8)&0xFF;
		u8 b = uptr&0xFF;
		u8 a = 0xFF;

		GXBegin(GX_LINES, GX_VTXFMT0, 10);

		float tlX = p->unkArray[0].x;
		float tlY = p->unkArray[0].y;
		float trX = p->unkArray[3].x;
		float trY = p->unkArray[3].y;
		float blX = p->unkArray[1].x;
		float blY = p->unkArray[1].y;
		float brX = p->unkArray[2].x;
		float brY = p->unkArray[2].y;

		// Top
		GXPosition3f32(tlX, tlY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(trX, trY, 9000.0f);
		GXColor4u8(r,g,b,a);

		// Left
		GXPosition3f32(tlX, tlY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(blX, blY, 9000.0f);
		GXColor4u8(r,g,b,a);

		// Right
		GXPosition3f32(trX, trY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(brX, brY, 9000.0f);
		GXColor4u8(r,g,b,a);

		// Bottom
		GXPosition3f32(blX, blY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(brX, brY, 9000.0f);
		GXColor4u8(r,g,b,a);

		// Diagonal
		GXPosition3f32(trX, trY, 9000.0f);
		GXColor4u8(r,g,b,a);
		GXPosition3f32(blX, blY, 9000.0f);
		GXColor4u8(r,g,b,a);

		GXEnd();

		p = p->next;
	}


	// Basic Colliders
	BasicCollider *bc = BasicCollider::globalListHead;
	while (bc) {
		u32 uptr = (u32)bc;
		u8 r = (uptr>>16)&0xFF;
		u8 g = (uptr>>8)&0xFF;
		u8 b = uptr&0xFF;
		u8 a = 0xFF;

		switch (bc->type) {
			case 0: case 2:
				GXBegin(GX_LINES, GX_VTXFMT0, 2);
				GXPosition3f32(bc->leftX, bc->leftY, 9000.0f);
				GXColor4u8(r,g,b,a);
				GXPosition3f32(bc->rightX, bc->rightY, 9000.0f);
				GXColor4u8(r,g,b,a);
				GXEnd();
				break;
		}

		bc = bc->next;
	}


	// Now, the hardest one.. CollisionMgr_c!
	fBase_c *fb = 0;
	while ((fb = fBase_c::searchByBaseType(2, fb))) {
		u8 *testMe = ((u8*)fb) + 0x1EC;
		if (*((u32*)testMe) != 0x8030F6D0)
			continue;

		u32 uptr = (u32)fb;
		u8 r = u8((uptr>>16)&0xFF)+0x20;
		u8 g = u8((uptr>>8)&0xFF)-0x30;
		u8 b = u8(uptr&0xFF)+0x80;
		u8 a = 0xFF;

		dStageActor_c *ac = (dStageActor_c*)fb;

		sensorBase_s *sensors[4] = {
			ac->collMgr.pBelowInfo, ac->collMgr.pAboveInfo,
			ac->collMgr.pAdjacentInfo, ac->collMgr.pAdjacentInfo};

		for (int i = 0; i < 4; i++) {
			sensorBase_s *s = sensors[i];
			if (!s)
				continue;

			float mult = (i == 3) ? -1.0f : 1.0f;

			switch (s->flags & SENSOR_TYPE_MASK) {
				case SENSOR_POINT:
					GXBegin(GX_POINTS, GX_VTXFMT0, 1);
					GXPosition3f32(
							ac->pos.x + (mult * (s->asPoint()->x / 4096.0f)),
							ac->pos.y + (s->asPoint()->y / 4096.0f),
							8005.0f);
					GXColor4u8(r,g,b,a);
					GXEnd();
					break;
				case SENSOR_LINE:
					GXBegin(GX_LINES, GX_VTXFMT0, 2);
					if (i < 2) {
						GXPosition3f32(
								ac->pos.x + (s->asLine()->lineA / 4096.0f),
								ac->pos.y + (s->asLine()->distanceFromCenter / 4096.0f),
								8005.0f);
						GXColor4u8(r,g,b,a);
						GXPosition3f32(
								ac->pos.x + (s->asLine()->lineB / 4096.0f),
								ac->pos.y + (s->asLine()->distanceFromCenter / 4096.0f),
								8005.0f);
						GXColor4u8(r,g,b,a);
					} else {
						GXPosition3f32(
								ac->pos.x + (mult * (s->asLine()->distanceFromCenter / 4096.0f)),
								ac->pos.y + (s->asLine()->lineA / 4096.0f),
								8005.0f);
						GXColor4u8(r,g,b,a);
						GXPosition3f32(
								ac->pos.x + (mult * (s->asLine()->distanceFromCenter / 4096.0f)),
								ac->pos.y + (s->asLine()->lineB / 4096.0f),
								8005.0f);
						GXColor4u8(r,g,b,a);
					}
					GXEnd();
					break;
			}
		}
	}
}
