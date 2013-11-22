.text

#include <asm_setup.S>

.align 4
.extern OSReport

.global ActorLogHook
ActorLogHook:
	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r3, 0x1C(sp)
	stw r4, 0x18(sp)
	stw r5, 0x14(sp)
	stw r6, 0x10(sp)

	lis r8, 0x8032
	ori r8, r8, 0xB58
	slwi r7, r3, 2
	lwzx r7, r8, r7

	lis r3, M@h
	ori r3, r3, M@l
	crclr 4*cr1+eq
	bl OSReport

	lwz r6, 0x10(sp)
	lwz r5, 0x14(sp)
	lwz r4, 0x18(sp)
	lwz r3, 0x1C(sp)
	lwz r0, 0x24(sp)
	mtlr r0
	addi sp, sp, 0x20

	# Return to __objCreate
	clrlslwi r31, r3, 16, 2
	blr

.data
M: .string "P:%p S:%08x T:%d N:%s\n"
