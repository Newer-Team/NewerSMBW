#ifndef __MWERKS__
.set r0,0;   .set r1,1;   .set r2,2; .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31; .set f0,0; .set f2,2; .set f3,3
.set f1,1;   .set f30,30; .set f31,31
#endif

.set sp,1

.global ApplyKamekPatch
ApplyKamekPatch:

	# STACK SETUP
	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r31, 0x1C(sp)
	stw r30, 0x18(sp)
	stw r29, 0x14(sp)
	
	# MAIN FUNCTION
	# r31 is our pointer within the data
	# r30 is our destination pointer
	# r29 is used as a "work register"

	# set r31 to the data pointer - 4
	subi r31, r3, 4

	# iterate through it until we find 0xFFFFFFFF
Looper:
	# obtain 0xFFFFFFFF so we can use it for the check
	li r29, 0
	subi r29, r29, 1
	
	lwzu r30, 4(r31)
	cmpw r30, r29
	beq ExitLoop
	
	# ok it's part of the patch, let's check it
	lis r29, 0x8000
	and. r29, r29, r30
	beq BlockPatch
	
	# it's a single patch
	lwzu r29, 4(r31)
	stw r29, 0(r30)
	
	# fix cache
	clrrwi r3, r30, 5
	dcbst r0, r3
	sync
	icbi r0, r3
	
	b NextIter
	
BlockPatch:
	# first int is the count
	mtctr r30
	
	# second int is the address
	lwzu r30, 4(r31)
	subi r30, r30, 4
	
BlockPatchLoop:
	# copy over an int
	lwzu r29, 4(r31)
	stwu r29, 4(r30)
	
	# fix cache
	clrrwi r3, r30, 5
	dcbst r0, r3
	sync
	icbi r0, r3
	
	bdnz BlockPatchLoop

NextIter:
	b Looper

ExitLoop:
	sync
	isync
	
	# FIX STACK
	lwz r29, 0x14(sp)
	lwz r30, 0x18(sp)
	lwz r31, 0x1C(sp)
	lwz r0, 0x24(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr
