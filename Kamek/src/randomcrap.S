.text
#include <asm_setup.S>
.global _Z3absf
_Z3absf:
	fabs f1, f1
	frsp f1, f1
	blr

.global JrFloorFireHax
JrFloorFireHax:
	stw r0, 0x120(r30)
	lwz r4, 4(r30)
	clrrwi. r4, r4, 28
	beqlr
	li r4, 0
	stw r4, 0x778(r30)
	lis r4, 0x100
	stw r4, 0x774(r30)
	stw r4, 0x77C(r30)
	blr

.global TryAndFixScoreGlitch
TryAndFixScoreGlitch:
	lwz r0, 0x4A10(r3)

	lis r8, 0x8000
	cmplw r3, r8
	bgtlr

	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global ThwompNewAutoDestruction
ThwompNewAutoDestruction:
	lfs f1, 0xB0(r31)
	lis r3, ThwompDestroyAt@h
	ori r3, r3, ThwompDestroyAt@l
	lfs f2, 0(r3)
	fcmpo cr0, f1, f2
	bgt dontDestroyMe
	mr r3, r31
.extern Delete__13dStageActor_cFUc
	lbz r4, 0x390(r3)
	bl Delete__13dStageActor_cFUc
dontDestroyMe:
	li r3, 1
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.data
.align 4
ThwompDestroyAt: .float -7533.0
.text
.global TitleScreenFucker
.extern RESTART_CRSIN_LevelStartStruct
.extern SaveFileBuffer
.extern enableHardMode
TitleScreenFucker:
	lis 	r4, enableHardMode@h
	ori 	r4, r4, enableHardMode@l

	li 		r3, 0
	stw		r3, 0(r4)

	lis r3, RESTART_CRSIN_LevelStartStruct@h
	ori r3, r3, RESTART_CRSIN_LevelStartStruct@l
	li r4, 0
	stb r4, 7(r3)
	
	# now get the FUCKING WORLD and FUCKING LEVEL
	lis r4, SaveFileBuffer@h
	ori r4, r4, SaveFileBuffer@l
	#lwz r4, 0(r4)
	lbz r5, 6(r4)
	extsb r5, r5
	mulli r5, r5, 0x980
	addi r4, r4, 0x6A0
	add r4, r4, r5

	# do we need to use Quick Save? get the QS block
	addi r4, r4, 0x1C80
	lbz r6, 2(r4)
	clrlwi. r6, r6, 31
	beq quickSaveExists
	subi r4, r4, 0x1C80
quickSaveExists:

	# now we have the save block in r4
	lbz r6, 2(r4)
	clrlwi. r6, r6, 31
	beq saveIsNotNew
	li r5, 0
	li r6, 9
	b chosenTitle
saveIsNotNew:
	lbz r5, 0x73A(r4)
	lbz r6, 0x73B(r4)
chosenTitle:
	cmpwi r5, 0
	bne isntWorld1_1
	cmpwi r6, 0
	bne isntWorld1_1
	li r5, 0
	li r6, 9
isntWorld1_1:
	stb r5, 0xC(r3)
	stb r6, 0xD(r3)
	blr


.global mpPauseFix1
.global mpPauseFix2
.global mpPauseFix3
.extern continueFromMpPauseFix1
.extern continueFromMpPauseFix2
.extern continueFromMpPauseFix3

mpPauseFix3:
	bl CheckIfSinglePlayer
	cmpwi r3, 1
	li r0, 30
	beq isSingle
	li r0, 1
isSingle:
	stb r0, 0x153E(r29)
	mr r3, r29
	b continueFromMpPauseFix3

mpPauseFix1:
	lis r5, continueFromMpPauseFix1@h
	ori r5, r5, continueFromMpPauseFix1@l
	b mpPauseFix
mpPauseFix2:
	lis r5, continueFromMpPauseFix2@h
	ori r5, r5, continueFromMpPauseFix2@l
	b mpPauseFix

.global mpPauseFix4
mpPauseFix4:
	b CheckIfSinglePlayer
.extern CheckIfSinglePlayer

mpPauseFix:
	stwu r1, -0x20(r1)
	mflr r0
	stw r0, 0x24(r1)
	stw r31, 0x1C(r1)
	stw r30, 0x18(r1)
	mr r31, r3
	mr r30, r4

	bl CheckIfSinglePlayer
	cmpwi r3, 1
	bne DontDoThis
	mr r3, r31
	mr r4, r30
	mtctr r5
	bctrl
DontDoThis:
	
	lwz r30, 0x18(r1)
	lwz r31, 0x1C(r1)
	lwz r0, 0x24(r1)
	mtlr r0
	addi r1, r1, 0x20
	blr

.global newClimbSounds
newClimbSounds:
	li r7, 0x134
	b effectNewClimbSounds
.global newClimbSoundsJump
newClimbSoundsJump:
	li r7, 0x136
	b effectNewClimbSounds
.global newClimbSoundsJumpS
newClimbSoundsJumpS:
	li r7, 0x137
	b effectNewClimbSounds
.global newClimbSoundsJumpSS
newClimbSoundsJumpSS:
	li r7, 0x138
	b effectNewClimbSounds
.global newClimbSoundsLand
newClimbSoundsLand:
	li r7, 0x139
	b effectNewClimbSounds
.global newClimbSoundsAttack
newClimbSoundsAttack:
	li r7, 0x13A
	b effectNewClimbSounds


.extern StagePtr
effectNewClimbSounds:
	lis r6, StagePtr@h
	ori r6, r6, StagePtr@l
	lwz r6, 0(r6)
	lhz r6, 0x120C(r6)

	cmpwi r6, 14
	beq pngn
	cmpwi r6, 0x903
	beq pngn
	cmpwi r6, 0x40E
	beq climb
	mr r4, r7
	blr
pngn:
	li r4, 0xE1
	blr
climb:
	li r4, 0xC9
	blr

.global TextWriterCrap
.extern TextWriterCrapContinue
TextWriterCrap:
	lis r6, 0xFFFF
	ori r6, r6, 0xFF00
	lwz r0, 8(r5)
	or r0, r0, r6
	stw r0, 0x18(r1)
	lwz r0, 0xC(r5)
	or r0, r0, r6
	stw r0, 0x1C(r1)
	lwz r0, 0x10(r5)
	or r0, r0, r6
	stw r0, 0x20(r1)
	lwz r0, 0x14(r5)
	or r0, r0, r6
	stw r0, 0x24(r1)

	b TextWriterCrapContinue

.global HeapChangeAttempt
HeapChangeAttempt:
	lis 4, 0x8037
	ori 4, 4, 0x7F50
	lwz 4, 0(4)
	blr


.extern OSReport
.global UniversalFunctionLog
UniversalFunctionLog:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	stw r30, 0x8(r1)
	mr r31, r3
	mr r30, r4

	crclr 4*cr1+eq
	mr r4, r31
	#Loop through callstack, kind of
	lwz r3, 0(r1)
	lwz r5, 4(r3)
	lwz r3, 0(r3)
	lwz r6, 4(r3)
	lwz r3, 0(r3)
	lwz r7, 4(r3)
	lwz r3, 0(r3)
	lwz r8, 4(r3)
	lwz r3, 0(r3)
	lwz r9, 4(r3)
	lwz r3, 0(r3)
	lwz r10, 4(r3)
	#lwz r10, 0x14(r29)

	lis r3, UniLogStr@h
	ori r3, r3, UniLogStr@l
	bl OSReport
	
	mr r3, r31
	mr r4, r30
	lwz r30, 0x8(r1)
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global EndSCM
EndSCM:
	lis r3, SCMs@h
	ori r3, r3, SCMs@l
	mr r4, r31
	crclr 4*cr1+eq
	bl OSReport

	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global PowEvent
PowEvent:
	subi r3, r3, 0x4CC
	lwz r3, 4(r3)
	clrlwi. r3, r3, 24
	beqlr
	subi r4, r3, 1
	lis r3, instance__10dFlagMgr_c@h
	ori r3, r3, instance__10dFlagMgr_c@l
	lwz r3, 0(r3)
	li r5, 0
	li r6, 1
	li r7, 0
	li r8, 0
	li r9, 0
	b set__10dFlagMgr_cFUcibbbUi

.extern set__10dFlagMgr_cFUcibbbUi
.extern instance__10dFlagMgr_c

.global GiganticBackgrounds
GiganticBackgrounds:
	lis r3, BGScale@h
	ori r3, r3, BGScale@l
	lbz r4, 0x10(r3)
	cmpwi r4, 0
	beq NoBGScale
	lwz r4, 0x4E4(r31)
	cmplwi r4, 0
	beq dontUseBack
	addi r3, r3, 8
dontUseBack:

	lfs f1, 0(r3)
	stfs f1, 0xDC(r31)
	lfs f1, 4(r3)
	stfs f1, 0xE0(r31)
NoBGScale:
	lwz r31, 0x5C(r1)
	lwz r0, 0x64(r1)
	mtlr r0
	addi r1, r1, 0x60
	blr

.global ExitStageReal
.extern ExitStageRealRest
ExitStageReal:
	stwu r1, -0x20(r1)
	b ExitStageRealRest


.global FixFireballSensorSetup
.extern ContinueFromFixFireballSensorSetup
FixFireballSensorSetup:
	mr r3, r30
	lwz r12, 0x60(r3)
	lwz r12, 0x11C(r12)
	mtctr r12
	bctrl
	mr r3, r30
	b ContinueFromFixFireballSensorSetup

.global BigMetSlidingFix
BigMetSlidingFix:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	lwz r4, 4(r5)
	lwz r12, 0x60(r3)
	lwz r12, 0x220(r12)
	mtctr r12
	bctrl
	li r3, 1
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global FuckLineLiftWithARake
.extern ContinueFromFuckLineLiftActivation
FuckLineLiftWithARake:
	stfs f0, 0x324(r28)
	lis r4, sfsdfsdf@h
	ori r4, r4, sfsdfsdf@l
	lfs f0, 0(r4)
	stfs f0, 0x318(r28)
	stfs f0, 0x31C(r28)
	blr
.global AlsoFuckTheLineLiftActivation
AlsoFuckTheLineLiftActivation:
	stwu r1, -0x10(r1)
	lwz r6, 4(r3)
	extrwi. r6, r6, 4,24
	bne myLineLiftActivator
	b ContinueFromFuckLineLiftActivation
myLineLiftActivator:
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	li r31, 0
	stw r30, 0x8(r1)
	lwz r30, 0x518(r3)
	b llaStartLoop
llaLoop:
	lwz r3, 4(r30)
	lbz r0, 0x38C(r3)
	cmplwi r0, 1
	bne llaNotPlayer
	addi r31, r31, 1
	b llaNext
llaNotPlayer:
	cmplwi r0, 2
	bne llaNext
	lwz r12, 0x60(r3)
	lwz r12, 0x6C(r12)
	mtctr r12
	bctrl
	lbz r0, 0(r3)
	extsb r0, r0
	cmplwi r0, 4
	bge llaNext
	addi r31, r31, 1
llaNext:
	lwz r30, 0xC(r30)
llaStartLoop:
	cmpwi r30, 0
	bne llaLoop

	cmpwi r31, 0
	beq llaReturnZero
	li r3, 1
	b llaReturn
llaReturnZero:
	li r3, 0
llaReturn:
	lwz r31, 0xC(r1)
	lwz r30, 8(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.data
sfsdfsdf: .float 512.0

.text

.global RedRingEventControl
RedRingEventControl:
	lwz r10, 4(r31)
	srwi. r10, r10, 28
	beq dontResetRREDelay
	li r5, 0
dontResetRREDelay:
	b set__10dFlagMgr_cFUcibbbUi

.global EFB_Crap
.extern _Z9EFBMagic2v
.extern ContinueFromEFBMagic2
EFB_Crap:
	bl _Z9EFBMagic2v
	lwz r3, 4(r30)
	b ContinueFromEFBMagic2

.data
.global BGScaleFront, BGScaleBack, BGScaleEnabled
BGScale:
BGScaleFront: .float 1.0, 1.0
BGScaleBack: .float 1.0, 1.0
BGScaleEnabled: .byte 0

#AllocLog:
#	stwu r1, -0x20(r1)
#	mflr r0
#	stw r0, 0x24(r1)
#	stw r31, 0x1C(r1)
#	stw r30, 0x18(r1)
#	stw r29, 0x14(r1)
#	stw r28, 0x10(r1)
#	mr r31, r5
#	mr r30, r4
#	mr r29, r3
#
#.extern ArchiveHeap
#	lis r3, ArchiveHeap@h
#	ori r3, r3, ArchiveHeap@l
#	lwz r3, 0(r3)
#	lwz r3, 0x10(r3)
#	cmpw r3, r29
#	bne dontLogMe
#
#	lis r3, WillAlloc@h
#	ori r3, r3, WillAlloc@l
#	crclr 4*cr1+eq
#	bl OSReport
#
#	mr r3, r29
#	mr r4, r30
#	mr r5, r31
#	bl RealAlloc
#	mr r28, r3
#
#	lis r3, Alloced@h
#	ori r3, r3, Alloced@l
#	mr r4, r28
#	crclr 4*cr1+eq
#	bl OSReport
#
#	mr r3, r28
#	
#	b doneHere
#dontLogMe:
#	mr r3, r29
#	mr r4, r30
#	mr r5, r31
#	bl RealAlloc
#doneHere:
#	lwz r28, 0x10(r1)
#	lwz r29, 0x14(r1)
#	lwz r30, 0x18(r1)
#	lwz r31, 0x1C(r1)
#	lwz r0, 0x24(r1)
#	mtlr r0
#	addi r1, r1, 0x20
#	blr
#
#FreeLog:
#	stwu r1, -0x20(r1)
#	mflr r0
#	stw r0, 0x24(r1)
#	stw r30, 0x18(r1)
#	stw r29, 0x14(r1)
#	mr r30, r4
#	mr r29, r3
#
#	lis r3, ArchiveHeap@h
#	ori r3, r3, ArchiveHeap@l
#	lwz r3, 0(r3)
#	lwz r3, 0x10(r3)
#	lwz r5, -0xC(r4)
#	cmpw r3, r29
#	bne dontLogMe2
#	lis r3, WillFree@h
#	ori r3, r3, WillFree@l
#	crclr 4*cr1+eq
#	bl OSReport
#dontLogMe2:
#
#	mr r3, r29
#	mr r4, r30
#	bl RealFree
#
#	lwz r29, 0x14(r1)
#	lwz r30, 0x18(r1)
#	lwz r0, 0x24(r1)
#	mtlr r0
#	addi r1, r1, 0x20
#	blr
#
#RealAlloc:
#	stwu r1, -0x20(r1)
#	lis r12, 0x801D
#	ori r12, r12, 0x45A4
#	mtctr r12
#	bctr
#
#RealFree:
#	stwu r1, -0x20(r1)
#	lis r12, 0x801D
#	ori r12, r12, 0x4854
#	mtctr r12
#	bctr
#
#CRAPPYALLOC:
#	lis r3, 0x801D
#	ori r3, r3, 0x45A0
#	lis r4, AllocLog@h
#	ori r4, r4, AllocLog@l
#	lis r5, 0x4800
#	sub r6, r4, r3
#	clrlwi r6, r6, 6
#	or r5, r5, r6
#	stw r5, 0(r3)
#	dcbst r0, r3
#	sync
#	icbi r0, r3
#	lis r3, 0x801D
#	ori r3, r3, 0x4850
#	lis r4, FreeLog@h
#	ori r4, r4, FreeLog@l
#	lis r5, 0x4800
#	sub r6, r4, r3
#	clrlwi r6, r6, 6
#	or r5, r5, r6
#	stw r5, 0(r3)
#	dcbst r0, r3
#	sync
#	icbi r0, r3
#	blr

.text
.align 4
.global ReplaceExceptionOccurred
ReplaceExceptionOccurred:
	lis r3, NewExceptionOccurred@h
	ori r3, r3, NewExceptionOccurred@l
	blr

.data
NewExceptionOccurred:
.string "Whoops! Newer 1.0.2 has crashed.\nPlease take a screenshot of the info below\nand send it to newer@newerteam.com.\nYou can scroll through this report using the D-Pad.\n"
UniLogStr: .string "[ULog] returning %8x to: %x > %x > %x > %x > %x > %x\n"
#WillAlloc: .string "[AH] Will allocate %x bytes aligned by %x\n"
#Alloced: .string "[AH] Allocated %p\n"
#WillFree: .string "[AH] Freeing %p which is %x bytes\n"

SCMs: .string "Setting music to %d\n"

.global Rrun
Rrun: .string "Rrun"
