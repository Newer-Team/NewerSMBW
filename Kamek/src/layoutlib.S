.extern LayoutHelper_Link

#include <asm_setup.S>

.extern LoadFile__FP10FileHandlePCc
.extern FreeFile__FP10FileHandle

.text

.set this,31




.global EmbeddedLayout_LoadArc
EmbeddedLayout_LoadArc:
	# stack setup
	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r31, 0x1C(sp)
	stw r30, 0x18(sp)

	# r31 is a pointer to the LayoutHelper (must be converted)
	# r30 stores the filename
	addi r31, r3, 0xAC
	mr r30, r4

	# check to see if an arc has already been loaded
	lwz r0, 0xBC(r3)
	cmpwi r0, 0
	bne return_0

	# ok, no
	# create a FileHandle where a NSMBWFile would be
	# don't know if this will work but it's worth a try
	addi r3, r31, 0xC0
	mr r4, r30
	bl LoadFile__FP10FileHandlePCc

	# check retval
	cmpwi r3, 0
	stw r3, 0xBC(r31)
	beq return_0

	# link it
	mr r4, r3
	mr r3, r31
	lis r5, arcString@h
	ori r5, r5, arcString@l
	bl LayoutHelper_Link

	# now add a pointer to the LayoutHelper into the Layout at offset 0x84
	# turn r3 into an EmbeddedLayout pointer
	subi r3, r31, 0xAC
	stw r31, 0x84(r3)

	b return_1

return_0:
	li r3, 0
	b return

return_1:
	li r3, 1

return:
	lwz r30, 0x18(sp)
	lwz r31, 0x1C(sp)
	lwz r0, 0x24(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr

.align 4

.global EmbeddedLayout_FreeArc
EmbeddedLayout_FreeArc:
	# stack setup
	stwu sp, -0x10(sp)
	mflr r0
	stw r0, 0x14(sp)
	stw r31, 0xC(sp)

	# r31 is a pointer to the LayoutHelper (must be converted)
	addi r31, r3, 0xAC

	# - if an arc has been loaded, then free the NSMBWFile
	lwz r0, 0xBC(r3)
	cmpwi r0, 0
	beq xreturn_1

	# no, so free it
	addi r3, r31, 0xC0
	bl FreeFile__FP10FileHandle

	li r3, 0
	stw r3, 0xBC(r31)

xreturn_1:
	li r3, 1
	lwz r31, 0xC(sp)
	lwz r0, 0x14(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr



.data
arcString:
	.string "arc"

.align 4

