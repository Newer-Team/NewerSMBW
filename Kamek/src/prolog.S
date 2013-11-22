.text
.align 4
#include <asm_setup.S>

.extern __ctor_loc
.extern __ctor_end
.extern OSReport

.global Prolog
Prolog:
	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r31, 0x1C(sp)
	stw r30, 0x18(sp)
	stw r29, 0x14(sp)
	# -- Go!

	li r30, 0

	lis r31, __ctor_loc@h
	ori r31, r31, __ctor_loc@l
	lis r29, __ctor_end@h
	ori r29, r29, __ctor_end@l

	lis r3, PMsg@h
	ori r3, r3, PMsg@l
	mr r4, r31
	mr r5, r29
	crclr 4*cr1+eq
	bl OSReport

	b startLoop

loop:
	mtctr r12
	bctrl
	addi r31, r31, 4
	addi r30, r30, 1
startLoop:
	lwz r12, 0(r31)
	cmpw r31, r29
	bne loop

	lis r3, PMsg2@h
	ori r3, r3, PMsg2@l
	mr r4, r30
	crclr 4*cr1+eq
	bl OSReport

	# -- Done
	li r3, 1
	lwz r31, 0x1C(sp)
	lwz r30, 0x18(sp)
	lwz r29, 0x14(sp)
	lwz r0, 0x24(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr


.data
PMsg:
	.string "Newer Super Mario Bros. Wii - Hacks by Treeki and Tempus 2009-2013\nNewer v1.0.2\n.ctors: %p - %p\n"

PMsg2:
	.string "%d inits called\n"

.align 4


