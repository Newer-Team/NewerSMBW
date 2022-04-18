.text
.align 4
.set sp, 1

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

.extern StoreCompressionClassList

.extern SZSDecompClass
.extern LZDecompClass
.extern LHDecompClass
.extern LRCDecompClass
.extern RLDecompClass

.extern DecompBufferPointer

.extern AllocFromGameHeap1
.extern OSReport

.extern TryAndFindCompressedFile
.extern DVDConvertPathToEntrynum

.global InitCompression
InitCompression:
	stwu sp, -0x10(sp)
	mflr r0
	stw r0, 0x14(sp)
	# -- Prolog

#	lis r3, CMsg@h
#	ori r3, r3, CMsg@l
#	crclr 4*cr1+eq
#	bl OSReport

	lis r3, CCCL@h
	ori r3, r3, CCCL@l
	lis r4, CCCL_End@h
	ori r4, r4, CCCL_End@l
	bl StoreCompressionClassList

	# Allocate a buffer for decompression stuff
	li r3, 0x8B4
	bl AllocFromGameHeap1
	lis r4, DecompBufferPointer@h
	ori r4, r4, DecompBufferPointer@l
	stw r3, 0(r4)

#	mr r4, r3
#	lis r3, CMsg2@h
#	ori r3, r3, CMsg2@l
#	crclr 4*cr1+eq
#	bl OSReport

	# -- Epilog
	li r3, 1
	lwz r0, 0x14(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr


.global GetFileEntrynumNew
GetFileEntrynumNew:
	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r31, 0x1C(sp)
	stw r30, 0x18(sp)
	stw r29, 0x14(sp)
	# Compression Type pointer (byte*)
	mr r30, r4
	# Filename
	mr r29, r3

	# first, try compression
	# don't bother checking 5278, it's always on in NSMBW anyway
	bl TryAndFindCompressedFile
	cmpwi r3, -1
	bne returnEntrynum

	# didn't find it, so try the normal version
	mr r3, r29
	bl DVDConvertPathToEntrynum
	# return value gets passed through

	cmpwi r30, 0
	beq returnEntrynum
	li r0, 0
	stb r0, 0(r30)

returnEntrynum:
	lwz r31, 0x1C(sp)
	lwz r30, 0x18(sp)
	lwz r29, 0x14(sp)
	lwz r0, 0x24(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr


.data
CCCL:
	.long LHDecompClass
	.long LZDecompClass
CCCL_End:
	.long 0

#CMsg:
#	.string "Setting up fancy decompression!\n"

#CMsg2:
#	.string "Buffer: %p\n"

.align 4




