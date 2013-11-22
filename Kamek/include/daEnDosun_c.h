#ifndef __DOSUN_H
#define __DOSUN_H

#include <daEnemy_c.h>

/* All Addresses NTSCv2 */

class daEnDosun_c : public daEnemy_c {
public:
	//vtable			// 80ae8268
	//ctor - Normal		// 809f5ad0
	//ctor - Big		// 809f5b60
	int onCreate();		// 809f5bf0
	int onDelete();		// 809f5fd0
	int onExecute();	// 809f5e50
	int onDraw();		// 809f5ea0
	//~daEnBigDosun_c();	// 809f7140
	void kill();		// 809f7110
	void killedByLevelClear();		// 809f6810
	void collisionCat3_StarPower(ActivePhysics* apThis, ActivePhysics *apOther);	// 809f6050
	void _vf108(ActivePhysics* apThis, ActivePhysics* apOther);	// 809f5fe0
	void collisionCatD_Drill(ActivePhysics* apThis, ActivePhysics* apOther);	// 809f7130
	void _vf110(ActivePhysics* apThis, ActivePhysics* apOther);	// 809f6090
	void collisionCat7_GroundPound(ActivePhysics* apThis, ActivePhysics* apOther);	// 809f7120
	void collisionCat9_RollingObject(ActivePhysics* apThis, ActivePhysics* apOther);	// 809f70a0
	void collisionCat1_Fireball_E_Explosion(ActivePhysics* apThis, ActivePhysics* apOther);	// 809f70e0
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics* apThis, ActivePhysics* apOther);	// 809f70b0
	void _vf144(int something);	// 809f6120
	//State Functions
	void DieFall_Begin();	// 809f6e00
	void DieFall_Execute();	// 809f6f10
	void DieFall_End();		// 809f6f00
	void Ice_Begin();		// 809f7020
	void Ice_Execute();		// 809f7090
	void Ice_End();			// 809f7080
	bool CreateIceActors();	// 809f6260

	mHeapAllocator_c allocator;		// _524
	nw4r::g3d::ResFile resFile;		// _540
	m3d::mdl_c bodyModel;			// _544
	m3d::anmVis_c anmVis;			// _584
	u32 collision_struct[4];		// _5BC
	float shakePosXoffset;			// _5CC shakePosXoffset
	u32 bigOrSmall;					// _5D0 0=small,1=big
	u32 holdsFromSettings;			// _5D4 0=normal,1=???,2=alwaysFalling
	u16 puruMoveCounter;			// _5D8
	u16 shakeIndex;					// _5DA 0=shake,1=normal
	u32 countdownTimer;				// _5DC
	u32 dieCounter;					// _5E0
	float unusedXoffset;			// _5E4
	float unusedYoffset;			// _5E8
	u32 _5EC;						//      padding

	/* don't have params yet */
	void setupBodyModel();			// 809f63f0
	void s_809F6530();				// 809f6530
	void spawnEffects();			// 809f66a0

	USING_STATES(daEnDosun_c);
	REF_NINTENDO_STATE(UpWait);
	REF_NINTENDO_STATE(DownMoveWait);
	REF_NINTENDO_STATE(PuruMove);
	REF_NINTENDO_STATE(DownMove_Kameck);
	REF_NINTENDO_STATE(DownMove);
	REF_NINTENDO_STATE(DownWait);
	REF_NINTENDO_STATE(UpMove);
	REF_NINTENDO_STATE(DieFall);
	REF_NINTENDO_STATE(Ice);
};

#endif

