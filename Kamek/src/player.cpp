#include "player.h"

// Please set your dStageActor_c's z coordinate
// Please check return for -1
char NearestPlayer(dStageActor_c* actor) {
	char nearest = -1;
	float current = 1000000000000000000000000000000.0;

	for(char ii = 0; ii < 4; ii++) {
		dStageActor_c* player = GetSpecificPlayerActor(ii);
		if(!player) {
			continue;
		}
				// actor->pos.x, actor->pos.y, actor->pos.z,
				// player->pos.x, player->pos.y, player->pos.z);
		float distance = VECDistance(&actor->pos, &player->pos);
		if(distance < current) {
			current = distance;
			nearest = ii;
		}
	}
	if(nearest < 0) {
	}
	return nearest;
}

void setNewActivePhysicsRect(dStageActor_c* actor, Vec* scale) {
	float amtX = scale->x;
	float amtY = scale->y;

	ActivePhysics::Info info;
	info.xDistToCenter = 0.0;
	info.yDistToCenter = 7.65 * amtY;
	info.xDistToEdge   = 4.0 * amtX;
	info.yDistToEdge   = 7.7 * amtY;

	info.category1  = actor->aPhysics.info.category1;
	info.category2  = actor->aPhysics.info.category2;
	info.bitfield1  = actor->aPhysics.info.bitfield1;
	info.bitfield2  = actor->aPhysics.info.bitfield2;
	info.unkShort1C = actor->aPhysics.info.unkShort1C;
	info.callback   = actor->aPhysics.info.callback;

	actor->aPhysics.removeFromList();
	actor->aPhysics.initWithStruct(actor, &info);
	actor->aPhysics.addToList();
}

void changeActivePhysicsRect(dStageActor_c* actor, float xc, float yc, float xe, float ye) {
	ActivePhysics::Info info;
	info.xDistToCenter = xc;
	info.yDistToCenter = yc;
	info.xDistToEdge   = xe;
	info.yDistToEdge   = ye;

	info.category1  = actor->aPhysics.info.category1;
	info.category2  = actor->aPhysics.info.category2;
	info.bitfield1  = actor->aPhysics.info.bitfield1;
	info.bitfield2  = actor->aPhysics.info.bitfield2;
	info.unkShort1C = actor->aPhysics.info.unkShort1C;
	info.callback   = actor->aPhysics.info.callback;

	actor->aPhysics.removeFromList();
	actor->aPhysics.initWithStruct(actor, &info);
	actor->aPhysics.addToList();
}

