#include <asm_setup.S>

.text
.extern OSReport

.extern enableNonLoopAnim__Q23m2d13EmbedLayout_cFib
.global FSShowCD, FSHideCD
FSShowCD:
	addi r3, r3, 0x74
	li r4, 0x2A
	b enableNonLoopAnim__Q23m2d13EmbedLayout_cFib

FSHideCD:
	addi r3, r3, 0x74
	li r4, 0x2B
	b enableNonLoopAnim__Q23m2d13EmbedLayout_cFib

.global FSShowCD_OpenWait
FSShowCD_OpenWait:
	addi r3, r31, 0x74
	li r4, 0x2A
	bl enableNonLoopAnim__Q23m2d13EmbedLayout_cFib
	lwz r0, 0x14(r1)
	lwz r31, 0xC(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

#.global FSHideCD_IWOSAEW
#FSHideCD_IWOSAEW:
#	addi r3, r31, 0x74
#	li r4, 0x2B
#	bl enableNonLoopAnim__Q23m2d13EmbedLayout_cFib
#	lwz r0, 0x14(r1)
#	lwz r31, 0xC(r1)
#	mtlr r0
#	addi r1, r1, 0x10
#	blr

.global DFNiceTitle
DFNiceTitle:
	mr r3, r4
	lis r4, DFTitle@h
	ori r4, r4, DFTitle@l
	lwz r6, 0xE44(r30)
	addi r6, r6, '1'
	sth r6, 0xA(r4)
	li r5, 0
	li r6, 6
	lwz r12, 0(r3)
	lwz r12, 0x80(r12)
	mtctr r12
	bctr

.extern findTextBoxByName__Q23m2d17EmbedLayoutBase_cCFPCc
.extern findPictureByName__Q23m2d17EmbedLayoutBase_cCFPCc
.extern InsertIntIntoTextBox1
.extern GrabExitInfoForFileSelect
.global DFNiceWorldName
DFNiceWorldName:
	stwu r1, -0x30(r1)
	mflr r0
	stw r0, 0x34(r1)
	stw r20, 0x2C(r1)
	stw r21, 0x28(r1)
	stw r22, 0x24(r1)
	# r1+0x20 is reserved for the stupid ints in textboxes
	mr r20, r4

	# Savefile is in r31
	# World Name field is in r20

	lhz r4, 0x734(r31)
	cmpwi r4, 1000
	blt useNormalWName
	lis r4, WarpZoneName@h
	ori r4, r4, WarpZoneName@l
	b chosenWName

useNormalWName:
	lis r4, ConvertedWorldName@h
	ori r4, r4, ConvertedWorldName@l
	mr r3, r4
	mr r5, r31
	li r6, 32
	mtctr r6
convWNameLoop:
	lbz r6, 0x6FC(r5)
	sth r6, 0(r3)
	addi r3, r3, 2
	addi r5, r5, 1
	bdnz convWNameLoop

chosenWName:
	mr r3, r20
	li r5, 0
	lwz r12, 0(r3)
	lwz r12, 0x7C(r12)
	mtctr r12
	bctrl

	# now set the colours
	# Text colours: 0x71C, hint colours: 0x724
	lwz r3, 0x71C(r31)
	stw r3, 0xDC(r20)
	lwz r3, 0x720(r31)
	stw r3, 0xE0(r20)

	addi r3, r30, 0x74
	lis r4, Picture_00@h
	ori r4, r4, Picture_00@l
	bl findPictureByName__Q23m2d17EmbedLayoutBase_cCFPCc
	lwz r4, 0x724(r31)
	stw r4, 0xD8(r3)
	stw r4, 0xDC(r3)
	lwz r4, 0x728(r31)
	stw r4, 0xE0(r3)
	stw r4, 0xE4(r3)

	lis r3, Out_Stuff@h
	ori r3, r3, Out_Stuff@l
	mr r4, r31
	bl GrabExitInfoForFileSelect
	lwz r21, 0(r3)
	lwz r22, 4(r3)

	# Now, subtract the paid star coins
	# lhz r3, 0x66(r31)
	# sub r21, r21, r3

	# r21: Star Coin Count
	# r22: Exit Count

	# And write everything ...
	addi r3, r30, 0x74
	lis r4, ExitCount@h
	ori r4, r4, ExitCount@l
	bl findTextBoxByName__Q23m2d17EmbedLayoutBase_cCFPCc

	stw r22, 0x20(r1)
	mr r4, r3
	addi r3, r1, 0x20
	li r5, 1
	bl InsertIntIntoTextBox1

	addi r3, r30, 0x74
	lis r4, StarCoinCount@h
	ori r4, r4, StarCoinCount@l
	bl findTextBoxByName__Q23m2d17EmbedLayoutBase_cCFPCc

	lis r4, NewSCCWidth@h
	ori r4, r4, NewSCCWidth@l
	lfs f1, 0(r4)
	stfs f1, 0x4C(r3)

	stw r21, 0x20(r1)
	mr r4, r3
	addi r3, r1, 0x20
	li r5, 0
	bl InsertIntIntoTextBox1

.extern _Z28ResetAllCompletionCandidatesv
.extern _Z28ClearOldPathAvailabilityDatav
	bl _Z28ResetAllCompletionCandidatesv
	bl _Z28ClearOldPathAvailabilityDatav

	lwz r22, 0x24(r1)
	lwz r21, 0x28(r1)
	lwz r20, 0x2C(r1)
	lwz r0, 0x34(r1)
	mtlr r0
	addi r1, r1, 0x30
	blr

.global FSDebugStates
FSDebugStates:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	stw r30, 0x8(r1)
	mr r31, r3
	mr r30, r4

	lis r3, FSStateDebugStr@h
	ori r3, r3, FSStateDebugStr@l
	crclr 4*cr1+eq
	lwz r4, 4(r4)
	bl OSReport

	mr r3, r31
	mr r4, r30
	lwzu r12, 0x18(r3)
	lwz r12, 0x18(r12)
	mtctr r12
	bctrl

	lwz r31, 0xC(r1)
	lwz r30, 0x8(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global DefaultSavefileInfo
DefaultSavefileInfo:
	addi r4, r3, 0x6FC
	lis r5, DefaultSavefileInfoData@h
	ori r5, r5, DefaultSavefileInfoData@l
	lis r6, DefaultSavefileInfoDataEnd@h
	ori r6, r6, DefaultSavefileInfoDataEnd@l
DSFICopyLoop:
	lwz r7, 0(r5)
	stw r7, 0(r4)
	addi r4, r4, 4
	addi r5, r5, 4
	cmpw r5, r6
	blt DSFICopyLoop
	blr

.extern SaveFileInstance
.extern SaveFileBuffer
.extern GetBlock__8SaveFileFi
.extern UpdateSavefileChecksums
.global FixUpExistingSavefile
FixUpExistingSavefile:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	stw r30, 8(r1)
	mr r30, r3
	
	li r31, 0
fixupNextBlock:
	lis r3, SaveFileInstance@h
	ori r3, r3, SaveFileInstance@l
	lwz r3, 0(r3)
	mr r4, r31
	bl GetBlock__8SaveFileFi

	lbz r4, 2(r3)
	clrlwi. r4, r4, 31
	beq dontFixThisOne
	bl DefaultSavefileInfo
dontFixThisOne:

	# this is the one whose checksum is checked
	lis r3, SaveFileBuffer@h
	ori r3, r3, SaveFileBuffer@l
	addi r3, r3, 0x6A0
	mulli r4, r31, 0x980
	add r3, r3, r4
	lbz r4, 2(r3)
	clrlwi. r4, r4, 31
	beq dontFixThisOne2
	bl DefaultSavefileInfo
dontFixThisOne2:

	addi r31, r31, 1
	cmpwi r31, 6
	blt fixupNextBlock

	lis r3, SaveFileBuffer@h
	ori r3, r3, SaveFileBuffer@l
	subi r3, r3, 0x20
	bl UpdateSavefileChecksums

	lis r3, SaveFileInstance@h
	ori r3, r3, SaveFileInstance@l
	lwz r3, 0(r3)
	bl UpdateSavefileChecksums

	mr r3, r30
	lwz r30, 8(r1)
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr


.section .ctors
.long FixUpExistingSavefile

.data
FSStateDebugStr: .string "State: %s\n"

.global FSDummyBrlanName, FSDummyGroup, FSDummyPane
FSDummyBrlanName: .string "fileSelectBase_18_DummyAnim.brlan"
FSDummyGroup: .string "DummyGroup"
FSDummyPane: .string "DummyPane"

.global A00_window
A00_window: .string "A00_window"
.global ShowEraseCopy, HideEraseCopy
ShowEraseCopy: .string "fileSelectBase_18_ShowEraseCopy.brlan"
HideEraseCopy: .string "fileSelectBase_18_HideEraseCopy.brlan"
.global N_posFile1_00
N_posFile1_00: .string "N_posFile1_00"
.global CDHolderGroup
CDHolderGroup: .string "CDHolderGroup"

NormalExitStr: .string "<Normal> %d-%d\n"
SecretExitStr: .string "<Secret> %d-%d\n"
StarCoinCount: .string "StarCoinCount"
ExitCount: .string "ExitCount"
Picture_00: .string "Picture_00"

.align 4
NewSCCWidth: .float 100.0

.global Out_Stuff
Out_Stuff:
.long 0, 0

.align 4
DFTitle:
.short 'F','i','l','e',' ','X',0
ConvertedWorldName:
.short 0,0,0,0,0,0,0,0,0,0,0,0 # 12
.short 0,0,0,0,0,0,0,0,0,0,0,0 # 12
.short 0,0,0,0,0,0,0,0 # 8

WarpZoneName:
.short 'W','a','r','p',' ','Z','o','n','e',0

.align 4
DefaultSavefileInfoData:
.string "Yoshi's Island" #15
.byte 0,0,0,0,0,0,0,0,0,0,0 # 11
.byte 0,0,0,0,0,0 # 6
.long 0xFFFF99FF,0x1FB423FF
.long 0x173714FF,0x3C9135FF
.long 0xFFFF99FF,0x1FB423FF
.short 0x75
.byte 0x2E,0xB
.byte 1,1
.byte 0,10

DefaultSavefileInfoDataEnd:
.long 0
