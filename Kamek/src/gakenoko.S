# GAKE CLASS LAYOUT
# 
# 524: SomeMHeapAllocatorSubclass
# 540: 0 [[ResFile maybe?]]
# 544: mdl_c
# 584: SomeModelAnimationClass
# 5BC: ResAnmTexPat (maybe?)
# 5C0: Some type of animation: 80329890
# 5C4: Set to 0 in ctor
# 5C8: Set to 0 in ctor
# 5CC: mAllocator_c
# 5F4: EGG::Effect subclass 80329CA0
# 
# 
# NET CLASS LAYOUT
# 
# 524: 0
# 528: SomeMHeapAllocatorSubclass
# 544: ResFile
# 548: mdl_c
# 588: SomeModelAnimationClass
# 5C0: ResAnmTexPat
# 5C4: Some type of animation: 80329890
# 5C8: Set to 0 in ctor
# 5CC: Set to 0 in ctor
# 5D0: mAllocator_c

.extern mHeapAllocatorSubclass_Link
.extern mHeapAllocatorSubclass_UnLink
.extern RetrieveFileFromArc
.extern GetResMdl
.extern m3d__mdl_c__DoStuff
.extern sub_80064BD0
.extern GetResAnmChr
.extern SomeModelAnimationClass_Setup
.extern GetResAnmTexPat
.extern sub_80166970
.extern sub_80166D10
.extern daEnGakeNoko_c__StateID_FoolMove
.extern daEnGakeNoko_c__StateID_Move

#ifndef __MWERKS__
.set r0,0;   .set r1,1;   .set r2,2; .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31; .set f0,0; .set f2,2; .set f3,3
#endif

.set sp,1
.set rtoc,2
.set this,31

.text

.align 4
.global gakenoko_create

.set AnmTexPat,0x24
.set ResMdlForTexPat,0x20
.set AnmChr,0x1C
.set ResMdlForChr,0x18
.set ResMdlForScnMdl,0x14
.set AnmTexPatAgain,0x10

# Stack layout:
# 0x34 : LR
# 0x30 : SP
# 0x2C : save this
# 0x28 : save r30
# 0x24 : AnmTexPat
# 0x20 : ResMdlForTexPat
# 0x1C : AnmChr
# 0x18 : ResMdlForChr
# 0x14 : ResMdlForScnMdl
# 0x10 : AnmTexPatAgain

# Local register vars:
# r31 = this
# r30 = resmdl

gakenoko_create:
	stwu r1, -0x30(sp)
	mflr r0
	stw r0, 0x34(sp)
	stw this, 0x2C(sp)
	stw r30, 0x28(sp)
	
	mr this, r3
	
	# Create a heap
	addi r3, r3, 0x524		# SomeMHeapAllocatorSubclass
	li r4, -1
	lis r5, 0x8037
	lwz r5, 0x7F48(r5)
	li r6, 0
	li r7, 0x20
	bl mHeapAllocatorSubclass_Link
	
	# Get ResFile
	lwz r3, -0x5668(r13)
	addi r3, r3, 4
	lis r4, arcName@h
	ori r4, r4, arcName@l
	lis r5, fileName@h
	ori r5, r5, fileName@l
	bl RetrieveFileFromArc
	
	stw r3, 0x540(this)
	
	# Get model
	addi r3, this, 0x540	# ResFile
	lis r4, arcName@h
	ori r4, r4, arcName@l
	bl GetResMdl
	
	mr r30, r3
	stw r3, ResMdlForScnMdl(sp) # store resmdl
	
	# Do scnmdl stuff
	addi r3, this, 0x544	# mdl_c
	addi r4, sp, ResMdlForScnMdl
	addi r5, this, 0x524	# SomeMHeapAllocatorSubclass
	li r6, 0x7fff
	li r7, 1
	li r8, 0
	bl m3d__mdl_c__DoStuff
	
	addi r3, this, 0x544	# mdl_c
	bl sub_80064BD0
	
	# Handle anim
	addi r3, this, 0x540	# ResFile
	lis r4, animName@h
	ori r4, r4, animName@l
	bl GetResAnmChr
	
	stw r3, AnmChr(sp)
	
	# Set it up
	stw r30, ResMdlForChr(sp)
	
	addi r3, this, 0x584	# SomeModelAnimationClass
	addi r4, sp, ResMdlForChr
	addi r5, sp, AnmChr
	addi r6, this, 0x524	# SomeMHeapAllocatorSubclass
	li r7, 0
	bl SomeModelAnimationClass_Setup
	
	# Now handle the texpat anim
	addi r3, this, 0x540	# ResFile
	lis r4, arcName@h
	ori r4, r4, arcName@l
	bl GetResAnmTexPat
	
	stw r3, 0x5BC(this)		# ResAnmTexPat
	stw r3, AnmTexPat(sp)
	
	# Set it up
	stw r30, ResMdlForTexPat(sp)
	
	addi r3, this, 0x5C0	# Weird Animation
	addi r4, sp, ResMdlForTexPat
	addi r5, sp, AnmTexPat
	addi r6, this, 0x524	# SomeMHeapAllocatorSubclass
	li r7, 0
	li r8, 1
	bl sub_80166970
	
	
	lwz r0, 0x5BC(this)
	stw r0, AnmTexPatAgain(sp)
	
	addi r3, this, 0x5C0	# Weird Animation
	addi r4, this, 0x544	# mdl_c
	addi r5, sp, AnmTexPatAgain
	li r6, 0
	li r7, 1
	bl sub_80166D10 # this is BindEntry
	
	# Unbind the MHeapAllocatorSubclass
	addi r3, this, 0x524	# SomeMHeapAllocatorSubclass
	bl mHeapAllocatorSubclass_UnLink
	
	# Model's done
	# Set a state
	lwz r3, 4(this)
	clrlwi. r3, r3, 31
	beq do_move
	lis r4, daEnGakeNoko_c__StateID_FoolMove@h
	ori r4, r4, daEnGakeNoko_c__StateID_FoolMove@l
	b chosen_state
do_move:
	lis r4, daEnGakeNoko_c__StateID_Move@h
	ori r4, r4, daEnGakeNoko_c__StateID_Move@l
chosen_state:
	mr r3, this
	lwz r12, 0x60(this)
	lwz r12, 0xD4(r12)
	mtctr r12
	bctrl

	# Set him to red
	lwz r3, 4(this)
	clrlwi. r3, r3, 31
	bne do_not_do_red
	
.extern setEntryByte34__Q23m3d11anmTexPat_cFUci
	addi r3, this, 0x5C0
	li r4, 1
	li r5, 0
	bl setEntryByte34__Q23m3d11anmTexPat_cFUci

.extern bindAnim__Q23m3d5mdl_cFPQ23m3d6banm_cf
	addi r3, this, 0x544
	addi r4, this, 0x5C0
	lis r5, zero_ptr@h
	ori r5, r5, zero_ptr@l
	lfs f1, 0(r5)
	bl bindAnim__Q23m3d5mdl_cFPQ23m3d6banm_cf

.extern setFrameForEntry__Q23m3d11anmTexPat_cFfi
	addi r3, this, 0x5C0
	li r4, 0
	lis r5, one_ptr@h
	ori r5, r5, one_ptr@l
	lfs f1, 0(r5)
	bl setFrameForEntry__Q23m3d11anmTexPat_cFfi

.extern setUpdateRateForEntry__Q23m3d11anmTexPat_cFfi
	addi r3, this, 0x5C0
	li r4, 0
	lis r5, zero_ptr@h
	ori r5, r5, zero_ptr@l
	lfs f1, 0(r5)
	bl setFrameForEntry__Q23m3d11anmTexPat_cFfi

do_not_do_red:

	# ActivePhysics for 2021
	addi r3, this, 0x148
	mr r4, this
	lis r5, 0x802E
	ori r5, r5, 0xE9B8
	.extern ActivePhysics_InitWithStruct
	.extern ActivePhysics_AddToList
	bl ActivePhysics_InitWithStruct
	addi r3, this, 0x148
	bl ActivePhysics_AddToList

	# Face the wall
	li r3, 0x8000
	sth r3, 0x102(this)
	
	li r3, 1
	
	# Clean up stack
	lwz r0, 0x34(sp)
	lwz this, 0x2C(sp)
	lwz r30, 0x28(sp)
	mtlr r0
	addi sp, sp, 0x30
	
	blr


.data

.align 4
zero_ptr: .float 0.0
one_ptr: .float 1.0

fileName:
	.string "g3d/nokonokoB.brres"

arcName:
	.string "nokonokoB"

animName:
	.string "net_walk2"

debugString:
	.string "X> %08x\n"
