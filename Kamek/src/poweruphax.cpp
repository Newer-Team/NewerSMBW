#include <common.h>
#include <game.h>
#include <g3dhax.h>


void ThwompHammer(dEn_c *thwomp, ActivePhysics *apThis, ActivePhysics *apOther) {
	if (thwomp->name == 0x51) {
		thwomp->dEn_c::collisionCat13_Hammer(apThis, apOther);
	}
	return;
}

void BooHammer(dEn_c *boo, ActivePhysics *apThis, ActivePhysics *apOther) {
	if (boo->name == 0xB0) {
		boo->dEn_c::collisionCat13_Hammer(apThis, apOther);
	}
	return;
}

void UrchinHammer(dEn_c *urchin, ActivePhysics *apThis, ActivePhysics *apOther) {
	return;
}


#include "poweruphax.h"

void SetCullModeForMaterial(m3d::mdl_c *model, int materialID, u32 cullMode);


dHammerSuitRenderer_c *dHammerSuitRenderer_c::build() {
	return new dHammerSuitRenderer_c;
}

dHammerSuitRenderer_c::dHammerSuitRenderer_c() { } dHammerSuitRenderer_c::~dHammerSuitRenderer_c() {
}

void dHammerSuitRenderer_c::setup(dPlayerModelHandler_c *handler) {
	setup(handler, 0);
}

void dHammerSuitRenderer_c::setup(dPlayerModelHandler_c *handler, int sceneID) {
	victim = (dPlayerModel_c*)handler->mdlClass;

	allocator.link(-1, GameHeaps[0], 0, 0x20);

	nw4r::g3d::ResFile rf(getResource("hammerM", "g3d/suit.brres"));

	if (victim->player_id_2 <= 1) {
		nw4r::g3d::ResMdl rm = rf.GetResMdl((victim->player_id_2 == 0) ? "marioHelmet" : "luigiHelmet");
		helmet.setup(rm, &allocator, 0, 1, 0);
		SetupTextures_MapObj(&helmet, sceneID);
	}

	const char *shellNames[] = {
		"shell", "shell", "shell", "shell", "shell"
	};
	shell.setup(rf.GetResMdl(shellNames[victim->player_id_2]), &allocator, 0, 1, 0);
	SetupTextures_MapObj(&shell, sceneID);

	allocator.unlink();


	victimModel = &victim->models[0].body;
	nw4r::g3d::ResMdl *playerResMdl =
		(nw4r::g3d::ResMdl*)(((u32)victimModel->scnObj) + 0xE8);

	//headNodeID = playerResMdl->GetResNode("player_head").GetID();
	if (victim->player_id_2 <= 1) {
		nw4r::g3d::ResNode face_1 = playerResMdl->GetResNode("face_1");
		headNodeID = face_1.GetID();
	}

	nw4r::g3d::ResNode skl_root = playerResMdl->GetResNode("skl_root");
	rootNodeID = skl_root.GetID();
}

void dHammerSuitRenderer_c::draw() {
	if (victim->powerup_id != 7)
		return;

	if (victim->player_id_2 <= 1) {
		// Materials: 2=hair 3=hat; Modes: BACK=visible ALL=invisible
		SetCullModeForMaterial(&victim->getCurrentModel()->head, 3, GX_CULL_ALL);

		Mtx headMtx;
		victimModel->getMatrixForNode(headNodeID, &headMtx);

		helmet.setDrawMatrix(&headMtx);
		helmet.setScale(1.0f, 1.0f, 1.0f);
		helmet.calcWorld(false);

		helmet.scheduleForDrawing();
	}

	Mtx rootMtx;
	victimModel->getMatrixForNode(rootNodeID, &rootMtx);

	shell.setDrawMatrix(&rootMtx);
	shell.setScale(1.0f, 1.0f, 1.0f);
	shell.calcWorld(false);

	shell.scheduleForDrawing();
}








// NEW VERSION
void CrapUpPositions(Vec *out, const Vec *in);

void dStockItem_c::setScalesOfSomeThings() {
	nw4r::lyt::Pane *ppos = N_forUse_PPos[playerCount];

	int howManyPlayers = 0;
	for (int i = 0; i < 4; i++) {
		if (isPlayerActive[i]) {
			int picID = getIconPictureIDforPlayer(howManyPlayers);
			int charID = Player_ID[i];

			if (picID != 24) {
				nw4r::lyt::Picture *pic = P_icon[picID];

				Vec in, out;

				in.x = pic->effectiveMtx[0][3];
				in.y = pic->effectiveMtx[1][3];
				in.z = pic->effectiveMtx[2][3];

				CrapUpPositions(&out, &in);

				u8 *wmp = (u8*)player2d[charID];
				*((float*)(wmp+0xAC)) = out.x;
				*((float*)(wmp+0xB0)) = out.y;
				*((float*)(wmp+0xB4)) = out.z;
				*((float*)(wmp+0x220)) = 0.89999998f;
				*((float*)(wmp+0x224)) = 0.89999998f;
				*((float*)(wmp+0x228)) = 0.89999998f;
				*((float*)(wmp+0x25C)) = 26.0f;
			}
			howManyPlayers++;
		}
	}


	for (int i = 0; i < 8; i++) {
		u8 *item = (u8*)newItemPtr[i];

		nw4r::lyt::Pane *icon = newIconPanes[i];

		Vec in, out;
		in.x = icon->effectiveMtx[0][3];
		in.y = icon->effectiveMtx[1][3];
		in.z = icon->effectiveMtx[2][3];

		CrapUpPositions(&out, &in);

		*((float*)(item+0xAC)) = out.x;
		*((float*)(item+0xB0)) = out.y;
		*((float*)(item+0xB4)) = out.z;
		*((float*)(item+0x1F4)) = P_buttonBase[i]->scale.x;
		*((float*)(item+0x1F8)) = P_buttonBase[i]->scale.y;
		*((float*)(item+0x1FC)) = 1.0f;
	}


	nw4r::lyt::Pane *shdRoot = shadow->rootPane;
	shdRoot->trans.x = N_stockItem->effectiveMtx[0][3];
	shdRoot->trans.y = N_stockItem->effectiveMtx[1][3];
	shdRoot->trans.z = N_stockItem->effectiveMtx[2][3];
	shdRoot->scale.x = N_stockItem_01->effectiveMtx[0][0];
	shdRoot->scale.y = N_stockItem_01->effectiveMtx[1][1];

	for (int i = 0; i < 7; i++)
		shadow->buttonBases[i]->scale = newButtonBase[i]->scale;
	shadow->hammerButtonBase->scale = newButtonBase[7]->scale;
}


