.text

#include <asm_setup.S>
.extern OSReport
.extern strcmp

.global RlanLoadDebugHook
.extern m2d__AnmResHandler_c__Load
RlanLoadDebugHook:
	stwu r1, -0x20(r1)
	mflr r0
	stw r0, 0x24(r1)
	stw r21, 0x1C(r1)
	stw r22, 0x18(r1)
	stw r23, 0x14(r1)
	stw r24, 0x10(r1)
	stw r25, 0xC(r1)

	mr r21, r3
	mr r22, r4
	mr r23, r5
	mr r24, r6
	mr r25, r7

	lis r3, RlanLoadStr@h
	ori r3, r3, RlanLoadStr@l
	mr r4, r29 # rlan num
	mr r5, r28 # rlan count
	mr r6, r21 # anmreshandler
	mr r7, r22 # rlan name
	mr r8, r23 # resAcc
	mr r9, r24 # layout
	crclr 4*cr1+eq
	bl OSReport

	lis r3, BreakOnRlan@h
	ori r3, r3, BreakOnRlan@l
	mr r4, r22
	bl strcmp
	cmpwi r3, 0
	bne thatsFine
	lis r3, 0xDEAD
	ori r3, r3, 0xBEEF
thatsFine:
	mr r3, r21
	mr r4, r22
	mr r5, r23
	mr r6, r24
	mr r7, r25
	bl m2d__AnmResHandler_c__Load
	mr r25, r3

#	lis r3, RlanGrpCount@h
#	ori r3, r3, RlanGrpCount@l
#	lwz r4, 0x18(r21)
#	crclr 4*cr1+eq
#	bl OSReport

	# Thing: r22
	# ID: r23
	# Count: r24
	li r23, 0
	lwz r24, 0x18(r21)
	lwz r22, 0x14(r21)
	b startGroupReportLoop
groupReportLoop:

	lis r3, RlanGroupStr@h
	ori r3, r3, RlanGroupStr@l

	mr r4, r23 # ID
	mr r5, r24 # count
	lwz r6, 0(r22) # group
	addi r7, r6, 0x18
	lwz r8, 4(r22) # animTransform

	crclr 4*cr1+eq
	bl OSReport

	addi r23, r23, 1
	addi r22, r22, 8

startGroupReportLoop:
	cmpw r23, r24
	blt groupReportLoop

	mr r3, r25

	lwz r21, 0x1C(r1)
	lwz r22, 0x18(r1)
	lwz r23, 0x14(r1)
	lwz r24, 0x10(r1)
	lwz r25, 0xC(r1)
	lwz r0, 0x24(r1)
	mtlr r0
	addi r1, r1, 0x20
	blr




.global AnmLoadDebugHook
.extern m2d__Anm_c__Load

AnmLoadDebugHook:
	stwu r1, -0x20(r1)
	mflr r0
	stw r0, 0x24(r1)
	stw r21, 0x1C(r1)
	stw r22, 0x18(r1)
	stw r23, 0x14(r1)

	mr r21, r3
	mr r22, r4
	mr r23, r5

	lis r3, AnmLoadStr@h
	ori r3, r3, AnmLoadStr@l
	mr r4, r30 # anim num
	mr r5, r29 # anim count
	lwz r6, 0(r28) # rlan ID
	mr r7, r21 # anim
	mr r8, r22 # anmreshandler
	mr r9, r23 # group name
	crclr 4*cr1+eq
	bl OSReport

	mr r3, r21
	mr r4, r22
	mr r5, r23
	bl m2d__Anm_c__Load

	lwz r21, 0x1C(r1)
	lwz r22, 0x18(r1)
	lwz r23, 0x14(r1)
	lwz r0, 0x24(r1)
	mtlr r0
	addi r1, r1, 0x20
	blr


.data
.align 4
AnmLoadStr: .string "[Anm %d/%d] Anm[%d] @ %p, ResHandler @ %p, Group: %s\n"
RlanLoadStr: .string "[Rlan %d/%d] ResHandler: %p, Rlan Name: %s, ResAcc: %p, Layout: %p\n"
#RlanGrpCount: .string "%d group(s):\n"
RlanGroupStr: .string "--[%d/%d] %p : %s - AnimTransform: %p\n"
BreakOnRlan: .string "fileSelectBase_18_DummyAnim.brlan"

