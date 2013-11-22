.text

#include <asm_setup.S>

.align 4
.extern GameMgrP
.extern BG_GM_ptr
.extern _restgpr_27

.global BG_GM_InitRedSwitchFlag_Patch
BG_GM_InitRedSwitchFlag_Patch:
	lis r5, GameMgrP@h
	ori r5, r5, GameMgrP@l
	lwz r5, 0(r5)

	addis r4, r3, 9
	lbz r0, 0x380(r5)
	stb r0, 0xAA8(r4)
	blr

.global CheckBehaviourOverride_EndPatch
CheckBehaviourOverride_EndPatch:
	lis r5, BG_GM_ptr@h
	ori r5, r5, BG_GM_ptr@l
	lwz r5, 0(r5)

	# Get the switch value
	addis r5, r5, 9
	lbz r5, 0xAA8(r5)

	# not sure why this happens
	cmpwi r28, 0
	bne returnRegularBehaviour

	# now do the rest of the checks:
	cmplwi r29, 0x58
	bne cboNotRed
	andi. r3, r5, 1
	beq cboNotRed
	b returnSbBehaviour
cboNotRed:
	cmplwi r29, 0x5A
	bne cboNotGreen
	andi. r3, r5, 2
	beq cboNotGreen
	b returnSbBehaviour
cboNotGreen:
	cmplwi r29, 0x56
	bne cboNotYellow
	andi. r3, r5, 4
	beq cboNotYellow
	b returnSbBehaviour
cboNotYellow:
	cmplwi r29, 0x54
	bne cboNotBlue
	andi. r3, r5, 8
	beq cboNotBlue
	b returnSbBehaviour
cboNotBlue:
	b returnRegularBehaviour

	# done!
returnSbBehaviour:
	li r4, 3
	li r3, 8
	b cboReturn
returnRegularBehaviour:
	mr r4, r27
	mr r3, r28
cboReturn:
	addi r11, r1, 0x20
	bl _restgpr_27
	lwz r0, 0x24(r1)
	mtlr r0
	addi r1, r1, 0x20
	blr

.global ProcessTileNum_EndPatch
ProcessTileNum_EndPatch:
	lis r5, BG_GM_ptr@h
	ori r5, r5, BG_GM_ptr@l
	lwz r5, 0(r5)
	addis r5, r5, 9
	lbz r5, 0xAA8(r5)

	cmplwi r30, 0x58
	bne not_red
	andi. r3, r5, 1
	beq not_red
	li r3, 0x57
	b ptnReturn
not_red:
	cmplwi r30, 0x5A
	bne not_green
	andi. r3, r5, 2
	beq not_green
	li r3, 0x59
	b ptnReturn
not_green:
	cmplwi r30, 0x56
	bne not_yellow
	andi. r3, r5, 4
	beq not_yellow
	li r3, 0x55
	b ptnReturn
not_yellow:
	cmplwi r30, 0x54
	bne not_blue
	andi. r3, r5, 8
	beq not_blue
	li r3, 0x53
	b ptnReturn
not_blue:
	mr r3, r30
ptnReturn:
	lwz r0, 0x14(r1)
	lwz r31, 0xC(r1)
	lwz r30, 0x8(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

