#ifndef __ENEMY_H
#define __ENEMY_H

#include <common.h>
#include <game.h>
#include <g3dhax.h>

/* All Addresses NTSCv2 */

//FIXME :(
extern "C" void HandlesOverallScale(Vec3* scale);

class daEnemy_c : public dEn_c {
public:
	int _vf58() { return 8; }		// 809a54e0 // daEnAkoya_c
	void _vf5C() { return; }		// 809a54d0 // daEnAkoya_c
	int _vf68() { return 1; }		// 809a5520	// daEnAkoya_c
	//u8* _vf6C();		// 809a3550	// daEnAkoya_c
	//FIXME
	u8* ptrToWhichPlayer() { return &this->which_player; }	// 809a3550	// daEnAkoya_c
	int _vf74() { return 1; }		// 809a5510	// dActorState_c
	void _vf7C() { return; }		// 809a5500	// dActorState_c
	void _vfB8() { return; }		// 809a54f0	// dActorState_c
	void _vf228() { HandlesOverallScale(&this->initialScale); }	// 809a5560 // smthg w/InitlScale
	void _vf23C() { return; }		// 809a5550 // nullsub
	void _vf240() { return; }		// 809a5540 // nullsub
	int _vf244() { return 0; }		// 809a5530 // return 0;

	//void setupBodyModel();			//FIXME
};

#endif

