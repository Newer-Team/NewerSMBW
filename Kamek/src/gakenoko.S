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

#include <asm_setup.S>
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
	bl sub_80166D10
	
	# Unbind the MHeapAllocatorSubclass
	addi r3, this, 0x524	# SomeMHeapAllocatorSubclass
	bl mHeapAllocatorSubclass_UnLink
	
	
	# Model's done
	# Set a state
	mr r3, this
	lis r4, daEnGakeNoko_c__StateID_FoolMove@h
	ori r4, r4, daEnGakeNoko_c__StateID_FoolMove@l
	
	lwz r12, 0x60(this)
	lwz r12, 0xD4(r12)
	mtctr r12
	
	
	li r3, 1
	
	# Clean up stack
	lwz r0, 0x34(sp)
	lwz this, 0x2C(sp)
	lwz r30, 0x28(sp)
	mtlr r0
	addi sp, sp, 0x30
	
	blr


.data

fileName:
	.string "g3d/nokonokoB.brres"

arcName:
	.string "nokonokoB"

animName:
	.string "net_walk2"

debugString:
	.string "X> %08x\n"
