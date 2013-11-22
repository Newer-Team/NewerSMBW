.text
#include <asm_setup.S>
.extern loadArc__Q23m2d13EmbedLayout_cFPCcb
.extern loadAnimations__Q23m2d13EmbedLayout_cFPPCci
.extern loadGroups__Q23m2d13EmbedLayout_cFPPCcPCii
.extern getPanes__Q23m2d13EmbedLayout_cCFPPCcPPQ34nw4r3lyt4Panei

.global GenericCorseClear
GenericCorseClear:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	mr r31, r3

	addi r3, r31, 0x70
	lis r4, CorseClearArcName@h
	ori r4, r4, CorseClearArcName@l
	li r5, 0
	bl loadArc__Q23m2d13EmbedLayout_cFPCcb

	cmpwi r3, 0
	bne didLoad
	li r3, 0
	b return
didLoad:
	addi r3, r31, 0x70
	lis r4, CorseClearBrlytName@h
	ori r4, r4, CorseClearBrlytName@l
	li r5, 0
	lwz r12, 8(r3)
	lwz r12, 0x14(r12)
	mtctr r12
	bctrl

	addi r3, r31, 0x70
	lis r4, CorseClearAnims@h
	ori r4, r4, CorseClearAnims@l
	li r5, 3
	bl loadAnimations__Q23m2d13EmbedLayout_cFPPCci

	addi r3, r31, 0x70
	lis r4, CorseClearGroups@h
	ori r4, r4, CorseClearGroups@l
	lis r5, CorseClearGroupAnimIDs@h
	ori r5, r5, CorseClearGroupAnimIDs@l
	li r6, 3
	bl loadGroups__Q23m2d13EmbedLayout_cFPPCcPCii

	lwz r3, 0x90(r31)
	stw r3, 0x36C(r31)

	addi r3, r31, 0x70
	lis r4, CorseClearPaneNames@h
	ori r4, r4, CorseClearPaneNames@l
	addi r5, r31, 0x370
	li r6, 1
	bl getPanes__Q23m2d13EmbedLayout_cCFPPCcPPQ34nw4r3lyt4Panei

	li r3, 1
return:
	lwz r0, 0x14(r1)
	lwz r31, 0xC(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.data
CorseClearArcName: .string "corseClear.arc"
CorseClearBrlytName: .string "corseClear_EngEU_00.brlyt"
CCInText: .string "corseClear_EngEU_00_inText.brlan"
CCLoopText: .string "corseClear_EngEU_00_loopText.brlan"
CCOutText: .string "corseClear_EngEU_00_outText.brlan"
A00_inText: .string "A00_inText"
A02_loopText: .string "A02_loopText"
A01_outText: .string "A01_outText"
N_marioClear_00: .string "N_marioClear_00"
.align 4
CorseClearGroupAnimIDs: .long 0, 1, 2
CorseClearAnims: .long CCInText, CCLoopText, CCOutText
CorseClearGroups: .long A00_inText, A02_loopText, A01_outText
CorseClearPaneNames: .long N_marioClear_00

