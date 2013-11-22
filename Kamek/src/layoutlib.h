#ifndef __NEWER_LAYOUTLIB_H
#define __NEWER_LAYOUTLIB_H

/* Treeki's LayoutLib for New Super Mario Bros Wii */
#include <common.h>

typedef float MTX[3][4];

struct nw4r__lyt__Layout {
	void *vtable;
	void *field_04;
	void *field_08;
	void *field_0C;
	void *rootPane;
	void *field_14;
	void *field_18;
	void *field_1C;
};

struct nw4r__lyt__DrawInfo {
	// 0x00
	void *vtable;
	MTX matrix;
	
	// 0x34
	float left;
	float top;
	float right;
	
	// 0x40
	float bottom;
	float scale_x;
	float scale_y;
	float opacity;
	
	// 0x50
	unsigned char widescreenFlag;
};

struct FrameCtrl_c {
	
};

struct Anm_c {
	FrameCtrl_c *fc_ptr;
	void *brlanHandler;
	void *field_08;
	char field_0C;
	char pad[3];
	FrameCtrl_c fc;
};

struct Layout {
	void *field_00;
	void *field_04;
	void *vtable;
	unsigned char drawOrder; // 0x0C
	char pad1[3];
	
	// offset 0x10
	nw4r__lyt__Layout layout; // Actually m2d::Layout_c, but the struct is the same
	
	// offset 0x30
	nw4r__lyt__DrawInfo drawInfo;
	
	// offset 0x84
	//LayoutHelper *field_84;
	void *field_84;
	float posX; // 0x88
	float posY; // 0x8C
	float clipX; // 0x90
	float clipY; // 0x94
	float clipWidth; // 0x98
	float clipHeight; // 0x9C
	unsigned char clipEnabled; // 0xA0
	char pad2[3];
	unsigned int animEnabled; // 0xA4
	void *field_A8;
	
	// offset 0xAC
	//LayoutHelper lh; // actually a m2d::ResAccLoader_c
	char lh[0xD4]; // don't feel like figuring this one out -_-
	
	// offset 0x180
	//BrlanHandler *brlanHandlers; // 0x180, points to brlan handlers
	void *brlanHandlers;
	//GrpHandler *grpHandlers; // 0x184
	void *grpHandlers;
	char *field_188; // char array, probably "anim enabled"
	int brlanCount; // 0x18C
	int grpCount; // 0x190
	int field_194; // no idea
};


void EmbeddedLayout_ctor(Layout *self); // 0x800C89A0
void EmbeddedLayout_dtor(Layout *self, bool del); // 0x800C89F0
void EmbeddedLayout_Free(Layout *self); // 0x800C9A20
bool EmbeddedLayout_LoadArcOld(Layout *self, const char *path, bool isLangSpecific); // 0x800C8D00

extern "C" {
bool EmbeddedLayout_LoadArc(Layout *self, const char *path); // custom
bool EmbeddedLayout_FreeArc(Layout *self); // custom
};

void EmbeddedLayout_LoadBrlans(Layout *self, const char **names, int count); // 0x800C90A0
void EmbeddedLayout_LoadGroups(Layout *self, const char **names, int *brlanLinkIDs, int count); // 0x800C91E0

void EmbeddedLayout_AddToDrawList(Layout *self); // 0x80163990

void *EmbeddedLayout_FindPaneByName(Layout *self, const char *name);
void *EmbeddedLayout_FindTextBoxByName(Layout *self, const char *name);
void *EmbeddedLayout_FindPictureByName(Layout *self, const char *name);
void *EmbeddedLayout_FindWindowByName(Layout *self, const char *name);

void EmbeddedLayout_EnableNonLoopAnim(Layout *self, int animNum, bool pointlessShit); // 0x800C93E0
void EmbeddedLayout_EnableLoopAnim(Layout *self, int animNum); // 0x800C9470
void EmbeddedLayout_ResetAnimToInitialState(Layout *self, int animNum, bool isLoop_maybe); // 0x800C94C0
void EmbeddedLayout_DisableAnim(Layout *self, int animNum); // 0x800C9580
void EmbeddedLayout_DisableAllAnims(Layout *self); // 0x800C95F0
void EmbeddedLayout_Process(Layout *self); // 0x800C9650
bool EmbeddedLayout_CheckIfAnimationIsOn(Layout *self, int animNum); // 0x800C9700
bool EmbeddedLayout_CheckIfAnyAnimationsAreOn(Layout *self); // 0x800C9730


typedef bool (*__EmbeddedLayout_Build_type)(Layout*, const char*, void*);
inline bool EmbeddedLayout_Build(Layout *self, const char *brlytName, void *lh) {
	VF_BEGIN(__EmbeddedLayout_Build_type, self, 5, 8)
		return VF_CALL(self, brlytName, lh);
	VF_END;
}

typedef void (*__EmbeddedLayout_UpdateMatrix_type)(Layout*);
inline void EmbeddedLayout_UpdateMatrix(Layout *self) {
	VF_BEGIN(__EmbeddedLayout_UpdateMatrix_type, self, 4, 8)
		VF_CALL(self);
	VF_END;
}

typedef void (*__TextBox_SetString_type)(void*, unsigned short const *, unsigned short);
inline void TextBox_SetString(void *self, unsigned short const *str, unsigned short unk) {
	VF_BEGIN(__TextBox_SetString_type, self, 31, 0)
		VF_CALL(self, str, unk);
	VF_END;
}

#define PANE_FLAGS(pane) (*((u8*)(((u32)(pane))+0xBB)))

#endif
