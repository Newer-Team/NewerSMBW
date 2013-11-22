# memory debugger - fun!
# GX stuff is implemented in C++, this just gets the lowlevel info

#define HEAP_EGGRootMEM1 1
#define HEAP_EGGRootMEM2 2
#define HEAP_Archive 3

.extern cppGXStart
.extern cppGXEnd
.extern cppProcessHeap
.extern MEMGetTotalFreeSizeForExpHeap
.extern GameHeap1
.extern GameHeap2
.extern ArchiveHeap
.extern OSReport
.extern dSys_c__RootHeapMEM1
.extern dSys_c__RootHeapMEM2

#include <asm_setup.S>
.set this,31

.text

.align 4

.global doHeapBar

doHeapBar:
	stwu sp, -0x10(sp)
	mflr r0
	stw r0, 0x14(sp)
	
	stw this, 0xC(sp)
	
	#lis r3, EGGTSystem_Pointer@h
	#ori r3, r3, EGGTSystem_Pointer@l
	#lwz this, 0(r3)
	
	bl cppGXStart
	
	#lwz r3, 0x18(this)
	#lis r3, dSys_c__RootHeapMEM1@h
	#ori r3, r3, dSys_c__RootHeapMEM1@l
	lis r3, GameHeap1@h
	ori r3, r3, GameHeap1@l
	lwz r3, 0(r3)
	li r4, HEAP_EGGRootMEM1
	li r5, 0
	bl asmProcessHeap
	
	#lwz r3, 0x1C(this)
	#lis r3, dSys_c__RootHeapMEM2@h
	#ori r3, r3, dSys_c__RootHeapMEM2@l
	lis r3, GameHeap2@h
	ori r3, r3, GameHeap2@l
	lwz r3, 0(r3)
	li r4, HEAP_EGGRootMEM2
	li r5, 0
	bl asmProcessHeap

	lis r3, ArchiveHeap@h
	ori r3, r3, ArchiveHeap@l
	lwz r3, 0(r3)
	li r4, HEAP_Archive
	li r5, 0
	bl asmProcessHeap
	
	lis r3, dSys_c__RootHeapMEM2@h
	ori r3, r3, dSys_c__RootHeapMEM2@l
	lwz r3, 0(r3)
	lwz r3, 0x10(r3)
	lwz r4, 0x18(r3)
	lwz r5, 0x1C(r3)
	lis r6, 0x8000
	lwz r6, 0x3128(r6)
	lis r3, THE_THING@h
	ori r3, r3, THE_THING@l
	#bl OSReport

	bl cppGXEnd
	
	lwz this, 0xC(sp)
	lwz r0, 0x14(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr


.global asmProcessHeap

asmProcessHeap:
	
	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	
	stw r31, 0x1C(sp)
	stw r30, 0x18(sp)
	stw r29, 0x14(sp)
	
	mr r31, r3
	mr r30, r4
	mr r29, r5
	
	# get all the required data from the heap
	# cppProcessHeap(u32 freeSize, u32 heapSize, int heapID, char *name)
	
	# get free size first so the function doesn't destroy our volatile regs
	# because I'm too lazy to save them >.>
	lwz r3, 0x10(r31)
	bl MEMGetTotalFreeSizeForExpHeap
	
	# get name
	lwz r6, 0x34(r31)
	
	# get heapID
	mr r5, r30
	
	# get heap size
	# this was a fucking pain to figure out -_-
	lwz r4, 0x10(r31)		# get the MEMiHeapHead*
	lwz r7, 0x18(r4)		# get heapStart
	lwz r8, 0x1C(r4)		# get heapEnd
	sub r4, r8, r7			# heapSize = heapEnd - heapStart probably
	
	# now do it
	cmpwi r29, 1
	beq justreport
	bl cppProcessHeap
	b gdhbdgdfg
justreport:
	mr r5, r4
	mr r4, r3
	lis r3, dgdfgdfg@h
	ori r3, r3, dgdfgdfg@l
	crclr 4*cr1+eq
	bl OSReport
gdhbdgdfg:
	
	lwz r29, 0x14(sp)
	lwz r30, 0x18(sp)
	lwz r31, 0x1C(sp)
	lwz r0, 0x24(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr

.data
THE_THING: .string "A thing: %x - %x and %x\n"
dgdfgdfg: .string "FREE: %x SIZE: %x\n"
.align 4

