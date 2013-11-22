# Fucking Nintendo, how do they work
#include <asm_setup.S>

.extern isReplayEnabled

.extern IOS_Open
.extern IOS_Close
.extern IOS_Read
.extern IOS_Write
.extern OSGetTime
.extern OSTicksToCalendarTime
.extern OSReport
.extern CurrentWorld
.extern CurrentLevel
.extern CurrentStartedArea
.extern CurrentStartedEntrance
.extern snprintf
.extern GetSomeGlobalClass
.extern SomeUnknownClass5408
.extern SomeWipeClass
.extern continueFromReplayHookStart
.extern continueFromReplayEndHook
.extern returnFromRecorder
.extern QueryGlobal5758
.extern GetRandomSeed
.extern Player_ID
.extern Player_Powerup
.extern Player_Flags
.extern EGG__Heap__alloc__FUliPv
.extern EGG__Heap__free__FPvPv
.extern GameHeap2
.extern EggControllerClassPtrMaybe
.extern GameMgrP
.extern StrangeReplayValue1
.extern StrangeReplayValue2
.extern StrangeReplayValue3
.extern WiimotePtr1
.extern Player_ID
.extern RandomSeed
.extern dontSkipReplayPrepare
.extern doSkipReplayPrepare

# mode 1 = read, 2 = write?

.text
.align 4

.global replayStart
.global replayEnd
.global replayRecord

#.global luigiOverride
#luigiOverride:
#	#blr
#	lis r3, Player_ID@h
#	ori r3, r3, Player_ID@l
#	li r4, 1
#	stw r4, 0(r3)
#	blr

.global getAndSaveRandomSeed
getAndSaveRandomSeed: # b from 8091F930
	lis r3, RandomSeed@h
	ori r3, r3, RandomSeed@l
	lwz r3, 0(r3)
	lis r4, saveRandomSeed@h
	ori r4, r4, saveRandomSeed@l
	stw r3, 0(r4)
	
	mr r4, r3
	lis r3, str_gotSeed@h
	ori r3, r3, str_gotSeed@l
	crclr 4*cr1+eq
	bl OSReport
	
	lwz r0, 0x74(sp)
	mtlr r0
	addi sp, sp, 0x70
	blr


replayStart: # b from 809246E0
	lis r3, isReplayEnabled@h
	ori r3, r3, isReplayEnabled@l
	lbz r3, 0(r3)
	cmpwi r3, 100
	bne justSkipReplayStart

	bl OSGetTime
	lis r5, replayTime@h
	ori r5, r5, replayTime@l
	bl OSTicksToCalendarTime
	
	li r29, 0
	li r30, 0
	
replayStartLoop:
	
	lis r3, str_replayFileNameBuffer@h
	ori r3, r3, str_replayFileNameBuffer@l
	
	li r4, 64
	
	lis r5, str_replayFileNameFormat@h
	ori r5, r5, str_replayFileNameFormat@l
	
	lis r6, CurrentWorld@h
	ori r6, r6, CurrentWorld@l
	lbz r6, 0(r6)
	addi r6, r6, 1
	
	lis r7, CurrentLevel@h
	ori r7, r7, CurrentLevel@l
	lbz r7, 0(r7)
	addi r7, r7, 1
	
	lis r8, replayTime_hour@h
	ori r8, r8, replayTime_hour@l
	lwz r8, 0(r8)
	
	lis r9, replayTime_min@h
	ori r9, r9, replayTime_min@l
	lwz r9, 0(r9)
	
	mr r10, r29
	
	crclr 4*cr1+eq
	bl snprintf
	
	lis r4, str_replayFileNameBuffer@h
	ori r4, r4, str_replayFileNameBuffer@l
	lis r3, str_startMsg@h
	ori r3, r3, str_startMsg@l
	crclr 4*cr1+eq
	bl OSReport
	
	lis r3, str_replayFileNameBuffer@h
	ori r3, r3, str_replayFileNameBuffer@l
	
	li r4, 0x602 # O_WRITE | O_CREAT | O_TRUNC
	
	bl IOS_Open
	
	cmpwi r3, 0
	blt iosOpenFail
	
	lis r4, replayHandles@h
	ori r4, r4, replayHandles@l
	stwx r3, r4, r30
	
	mr r4, r3
	lis r3, str_iosOpenMsg@h
	ori r3, r3, str_iosOpenMsg@l
	crclr 4*cr1+eq
	bl OSReport
	
	# now write the header
	lis r4, replayHeaders@h
	ori r4, r4, replayHeaders@l
	lwzx r4, r4, r30
	
	lis r6, CurrentWorld@h
	ori r6, r6, CurrentWorld@l
	lis r7, CurrentLevel@h
	ori r7, r7, CurrentLevel@l
	lis r8, CurrentStartedArea@h
	ori r8, r8, CurrentStartedArea@l # WRONG
	lis r9, CurrentStartedEntrance@h
	ori r9, r9, CurrentStartedEntrance@l # WRONG
	
	lbz r6, 0(r6)
	lbz r7, 0(r7)
	
	stb r6, 0(r4)
	stb r7, 1(r4)
	stb r8, 2(r4)
	stb r9, 3(r4)
	
	lis r3, saveRandomSeed@h
	ori r3, r3, saveRandomSeed@l
	lwz r3, 0(r3)
	stw r3, 4(r4)
	
	# player ID specific stuff
	mr r5, r29
	slwi r5, r5, 2
	
	lis r3, Player_ID@h
	ori r3, r3, Player_ID@l
	lwzx r3, r3, r5
	slwi r6, r3, 2 # store this so we can get the powerup and flags
	stw r3, 0x10(r4)
	
	lis r3, Player_Powerup@h
	ori r3, r3, Player_Powerup@l
	lwzx r3, r3, r6
	stw r3, 0x14(r4)
	
	lis r3, Player_Flags@h
	ori r3, r3, Player_Flags@l
	lwzx r3, r3, r6
	stw r3, 0x18(r4)
	
	lis r3, GameMgrP@h
	ori r3, r3, GameMgrP@l
	lwz r3, 0(r3)
	lbz r3, 0x380(r3)
	stb r3, 0x21(r4)
	
	# now set up variables
	lis r4, replayFrameCounts@h
	ori r4, r4, replayFrameCounts@l
	li r3, 0
	stwx r3, r4, r30
	
	lis r3, replayFlags@h
	ori r3, r3, replayFlags@l
	li r4, 1
	not r0, r4
	lhz r4, 0(r3)
	clrlwi r0, r0, 16
	and r4, r4, r0
	slwi r5, r29, 1 # player ID * 2
	sthx r4, r3, r5
	
	li r3, 0
	lis r4, keepTiltValues@h
	ori r4, r4, keepTiltValues@l
	stwx r3, r4, r30
	
	lis r4, replayDataBufferSize@h
	ori r4, r4, replayDataBufferSize@l
	lwz r3, 0(r4)
	
	# alloc the memory
	li r4, 4
	lis r5, GameHeap2@h
	ori r5, r5, GameHeap2@l
	lwz r5, 0(r5)
	bl EGG__Heap__alloc__FUliPv
	
	lis r4, replayDataBuffers@h
	ori r4, r4, replayDataBuffers@l
	stwx r3, r4, r30
	
	lis r4, replayDataBufferPointers@h
	ori r4, r4, replayDataBufferPointers@l
	stwx r3, r4, r30
	
	mr r4, r3
	lis r3, str_bufferObtained@h
	ori r3, r3, str_bufferObtained@l
	crclr 4*cr1+eq
	bl OSReport
	
	b startNextReplay
	
iosOpenFail:
	mr r4, r3
	lis r3, str_iosOpenFailed@h
	ori r3, r3, str_iosOpenFailed@l
	crclr 4*cr1+eq
	bl OSReport
	
startNextReplay:
	
	# the loop
	addi r29, r29, 1
	addi r30, r30, 4
	cmpwi r29, 4
	blt replayStartLoop
	
	# and done!
justSkipReplayStart:
	li r3, 1
	b continueFromReplayHookStart # 809246E4



magicalReplayCheck:
	stwu sp, -0x10(sp)
	mflr r0
	stw r0, 0x14(sp)
	stw r31, 0x0C(sp)
	mr r31, r3
	
	bl GetSomeGlobalClass
	
	lwz r0, 0x1D0(r3)
	cmpwi r0, 2
	bne no_flag_20
	
	ori r31, r31, 0x20
	b done_flag_20
	
no_flag_20:
	li r4, 0x20
	not r4, r4
	and r31, r31, r4
	
done_flag_20:
	lis r3, SomeUnknownClass5408@h # 8042A578
	ori r3, r3, SomeUnknownClass5408@l
	lwz r3, 0(r3)
	lbz r3, 4(r3)
	
	cmpwi r3, 0
	beq no_flag_40
	
	ori r31, r31, 0x40
	b done_flag_40
	
no_flag_40:
	li r4, 0x40
	not r4, r4
	and r31, r31, r4
	
done_flag_40:
	lis r3, SomeWipeClass@h # 8042A720
	ori r3, r3, SomeWipeClass@l
	lwz r3, 0(r3)
	lwz r12, 0(r3)
	lwz r12, 0x10(r12)
	mtctr r12
	bctrl
	
	subi r0, r3, 1
	cntlzw r0, r0
	srwi. r0, r0, 5
	beq set_flag_2
	
	li r4, 2
	not r4, r4
	and r31, r31, r4
	b done_flag_2
	
set_flag_2:
	ori r31, r31, 2
	
done_flag_2:
	li r3, 1
	bl QueryGlobal5758
	
	cmpwi r3, 0
	beq no_flag_4
	
	ori r31, r31, 4
	b done_flag_4
	
no_flag_4:
	li r4, 4
	not r4, r4
	and r31, r31, r4
	
done_flag_4:
	li r3, 4
	bl QueryGlobal5758
	
	cmpwi r3, 0
	beq no_flag_100
	
	ori r31, r31, 0x100
	b done_flag_100
	
no_flag_100:
	li r4, 0x100
	not r4, r4
	and r31, r31, r4
	
done_flag_100:
	li r3, 2
	bl QueryGlobal5758
	
	cmpwi r3, 0
	beq no_flag_8
	
	ori r31, r31, 8
	b done_flag_8
	
no_flag_8:
	li r4, 8
	not r4, r4
	and r31, r31, r4
	
done_flag_8:
	# okay, we're done
	mr r3, r31
	
	lwz r0, 0x14(sp)
	lwz r31, 0x0C(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr





replayEnd: # bl from 80102238
	lis r4, isReplayEnabled@h
	ori r4, r4, isReplayEnabled@l
	lbz r4, 0(r4)
	cmpwi r4, 100
	bnelr

	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r31, 0x1C(sp)
	stw r30, 0x18(sp)
	stw r29, 0x14(sp)
	
	cmpwi r3, 1
	bne justLeave
	
	li r29, 0
	li r30, 0
	
replayEndLoop:
	lis r31, replayHandles@h
	ori r31, r31, replayHandles@l
	add r31, r31, r30 # becomes a pointer to the handle, not the handle itself
	
	lwz r3, 0(r31)
	cmpwi r3, 0
	beq dontEndThisReplay
	
	lis r4, replayHeaders@h
	ori r4, r4, replayHeaders@l
	lwzx r4, r4, r30
	li r5, 0x40
	bl IOS_Write
	
	lwz r3, 0(r31)
	lis r4, replayDataBuffers@h
	ori r4, r4, replayDataBuffers@l
	lwzx r4, r4, r30
	lis r5, replayDataBufferPointers@h # calc written size
	ori r5, r5, replayDataBufferPointers@l
	lwzx r5, r5, r30
	subf r5, r4, r5
	bl IOS_Write
	
	lwz r3, 0(r31)
	lis r4, replayFooter@h
	ori r4, r4, replayFooter@l
	li r5, 4
	bl IOS_Write
	
	lwz r3, 0(r31)
	bl IOS_Close
	
	li r3, 0
	stw r3, 0(r31)
	
	lis r4, replayDataBuffers@h
	ori r4, r4, replayDataBuffers@l
	lwzx r3, r4, r30
	li r4, 0
	bl EGG__Heap__free__FPvPv
	
	lis r4, replayDataBuffers@h
	ori r4, r4, replayDataBuffers@l
	li r0, 0
	stwx r0, r4, r30
	
	lis r3, str_endMsg@h
	ori r3, r3, str_endMsg@l
	lis r4, replayFrameCounts@h
	ori r4, r4, replayFrameCounts@l
	lwzx r4, r4, r30
	crclr 4*cr1+eq
	bl OSReport
	
	# before we leave, set the flag again
	lis r3, replayFlags@h
	ori r3, r3, replayFlags@l
	add r3, r3, r29
	add r3, r3, r29
	lhz r4, 0(r3)
	ori r4, r4, 1
	sth r4, 0(r3)
	
dontEndThisReplay:
	
	# now loop
	addi r29, r29, 1
	addi r30, r30, 4
	cmpwi r29, 4
	blt replayEndLoop
	
	li r3, 1
	
justLeave:
	
	lwz r0, 0x24(sp)
	lwz r31, 0x1C(sp)
	lwz r30, 0x18(sp)
	lwz r29, 0x14(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr
	#b continueFromReplayEndHook # 8010223C




replayRecord:
	lis r3, isReplayEnabled@h
	ori r3, r3, isReplayEnabled@l
	lbz r3, 0(r3)
	cmpwi r3, 100
	bnelr

	# fun shit.
	stwu sp, -0x10(sp)
	mflr r0
	stw r0, 0x14(sp)
	stw r31, 0xC(sp)
	stw r30, 8(sp)
	
	li r30, 0
	
recordLoop:
	# get the controller class
	lis r31, EggControllerClassPtrMaybe@h
	ori r31, r31, EggControllerClassPtrMaybe@l
	lwz r31, 0(r31)
	slwi r3, r30, 2
	addi r3, r3, 4
	lwzx r31, r31, r3
	
	# get player number
	mr r4, r30
	
	lis r3, replayFlags@h
	ori r3, r3, replayFlags@l
	add r3, r3, r4
	add r3, r3, r4 # get pointer to flags
	lhz r3, 0(r3)
	
	bl magicalReplayCheck
	lwz r5, 4(r31)
	lis r4, replayFlags@h
	ori r4, r4, replayFlags@l
	add r4, r4, r5
	add r4, r4, r5
	sth r3, 0(r4)
	
	#mr r4, r3
	#lis r3, str_debugFlags@h
	#ori r3, r3, str_debugFlags@l
	#crclr 4*cr1+eq
	#bl OSReport
	
	#lis r3, replayFlags@h
	#ori r3, r3, replayFlags@l
	#lhz r3, 0(r3)
	cmpwi r3, 0
	bne goToNextRecorder
	
	# set this to player number * 4
	# r31 is our controller class
	lwz r9, 4(r31)
	slwi r9, r9, 2
	
	lis r3, replayFrameCounts@h
	ori r3, r3, replayFrameCounts@l
	lwzx r4, r3, r9
	addi r4, r4, 1
	stwx r4, r3, r9
	
	# put together the data
	# first grab a pointer
	lis r7, replayDataBufferPointers@h
	ori r7, r7, replayDataBufferPointers@l
	lwzx r6, r7, r9
	
	cmpwi r6, 0
	beq goToNextRecorder
	
	# first off, grab the initial flags
	li r3, 0
	
	lbz r4, 0x8C(r31) # shake flag
	cmpwi r4, 1
	bne dontShake
	oris r3, r3, 0x2000
dontShake:
	
	# now do the tilt stuff
	lis r4, keepTiltValues@h
	ori r4, r4, keepTiltValues@l
	lwzx r0, r4, r9 # last value
	lhz r5, 0x8E(r31) # new value
	stwx r5, r4, r9 # store new value
	cmpw r5, r0
	beq tiltDidntChange
	oris r3, r3, 0x80
tiltDidntChange:
	
	# ok, so we decided that, now write the flags + weird value + etc!
	stw r3, 0(r6)
	
	# calculate the weird value
	li r3, 0
	
	lis r10, StrangeReplayValue1@h
	ori r10, r10, StrangeReplayValue1@l
	lhz r10, 0(r10)
	slwi r10, r10, 16
	or r3, r3, r10
	
	lis r10, StrangeReplayValue2@h
	ori r10, r10, StrangeReplayValue2@l
	lbz r10, 0(r10)
	slwi r10, r10, 8
	or r3, r3, r10
	
	lis r10, StrangeReplayValue3@h
	ori r10, r10, StrangeReplayValue3@l
	lbz r10, 0(r10)
	or r3, r3, r10
	
	stw r3, 4(r6)
	
	# buttons
	lwz r3, 0xC(r31)
	li r4, 0x0F0F
	and r3, r3, r4
	stw r3, 8(r6)
	
	addi r6, r6, 0xC # we wrote 0xC bytes
	
	# now the tilt segment
	beq dontWriteTilt
	sth r5, 0(r6)
	li r3, 0
	sth r3, 2(r6)
	addi r6, r6, 4
	
dontWriteTilt:
	# are we done now?!
	# I hope so.
	
	lis r7, replayDataBufferPointers@h
	ori r7, r7, replayDataBufferPointers@l
	stwx r6, r7, r9
	
	# before we do this. check if we should end the replay
	# the raw button presses are at 0x18 in the Wiimote class
	lis r5, WiimotePtr1@h
	ori r5, r5, WiimotePtr1@l
	lwzx r3, r5, r9
	lwz r3, 0x18(r3)
	li r4, 0x0400 # B button mask
	and. r3, r3, r4
	beq goToNextRecorder
	
	# end it!
	li r3, 1
	bl replayEnd
	b leaveRecordLoop
	
goToNextRecorder:
	
	addi r30, r30, 1
	cmpwi r30, 4
	blt recordLoop
	
leaveRecordLoop:
	
	lwz r31, 0xC(sp)
	lwz r30, 8(sp)
	lwz r0, 0x14(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr



.global ChooseReplayStateIfNeeded
ChooseReplayStateIfNeeded:
	lis r5, isReplayEnabled@h
	ori r5, r5, isReplayEnabled@l
	lbz r5, 0(r5)
	cmpwi r5, 100
	beq useReplayState
	lis r4, Crsin_EffectResWaitProc@h
	ori r4, r4, Crsin_EffectResWaitProc@l
	blr
useReplayState:
	lis r4, Crsin_ReplayResWaitProc@h
	ori r4, r4, Crsin_ReplayResWaitProc@l
	blr

.extern Crsin_EffectResWaitProc
.extern Crsin_ReplayResWaitProc

.global SkipReplayPrepareIfNeeded
SkipReplayPrepareIfNeeded:
	mr r24, r3
	lis r5, isReplayEnabled@h
	ori r5, r5, isReplayEnabled@l
	lbz r5, 0(r5)
	cmpwi r5, 100
	beq yesSkipIt
	b dontSkipReplayPrepare
yesSkipIt:
	b doSkipReplayPrepare


.data

replayFlags: .short 1, 1, 1, 1
.align 4

replayHandles: .long 0, 0, 0, 0

replayFrameCounts: .long 0, 0, 0, 0

replayTime:
replayTime_sec:	.long 0
replayTime_min:	.long 0
replayTime_hour:	.long 0
replayTime_mday:	.long 0
replayTime_mon:	.long 0
replayTime_year:	.long 0
replayTime_wday:	.long 0
replayTime_yday:	.long 0
replayTime_msec:	.long 0
replayTime_usec:	.long 0

str_replayFileNameFormat:
	.string "file/Replay_%02d-%02d_from_%02d-%02d_p%d.bin"

str_replayFileNameBuffer:
	.byte 0,0,0,0,0,0,0,0
	.byte 0,0,0,0,0,0,0,0
	.byte 0,0,0,0,0,0,0,0
	.byte 0,0,0,0,0,0,0,0
	.byte 0,0,0,0,0,0,0,0
	.byte 0,0,0,0,0,0,0,0
	.byte 0,0,0,0,0,0,0,0
	.byte 0,0,0,0,0,0,0,0

str_startMsg:
	.string "*** Replay Recorder by Treeki -- Starting to record. Filename: %s\n"

str_iosOpenMsg:
	.string "*** Replay Recorder -- IOS_Open returned: %d\n"

str_iosOpenFailed:
	.string "*** Replay Recorder -- IOS_Open failed! It returned: %d -- The replay will not be saved.\n"

str_bufferObtained:
	.string "*** Replay Recorder -- Allocated replay buffer at %p\n"

str_endMsg:
	.string "*** Replay Recorder -- Recording complete. %d frames saved.\n"

str_debugFlags:
	.string "DEBUG:%04x\n"

str_gotSeed:
	.string "Got random seed: %08x\n"

.align 4

replayHeaders:
	.long replayHeader0
	.long replayHeader1
	.long replayHeader2
	.long replayHeader3

replayHeader0:
replayHeader0_world:		.byte 0
replayHeader0_level:		.byte 0
replayHeader0_area:		.byte 0
replayHeader0_entrance:	.byte 0
replayHeader0_seed:		.long 0
replayHeader0_padding08:	.long 0xFFFFFFFF, 0xFFFFFFFF
replayHeader0_playerID:	.long 0
replayHeader0_powerup:	.long 0
replayHeader0_playerFlg:	.long 0
replayHeader0_padding1C:	.long 0xFFFFFFFF
replayHeader0_padding20:	.byte 0xFF
replayHeader0_switchFlg:	.byte 0
replayHeader0_padding22:	.byte 0xFF, 0xFF
replayHeader0_padding24:	.long 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
replayHeader0_padding30:	.long 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
replayHeader1:
replayHeader1_world:		.byte 0
replayHeader1_level:		.byte 0
replayHeader1_area:		.byte 0
replayHeader1_entrance:	.byte 0
replayHeader1_seed:		.long 0
replayHeader1_padding08:	.long 0xFFFFFFFF, 0xFFFFFFFF
replayHeader1_playerID:	.long 0
replayHeader1_powerup:	.long 0
replayHeader1_playerFlg:	.long 0
replayHeader1_padding1C:	.long 0xFFFFFFFF
replayHeader1_padding20:	.byte 0xFF
replayHeader1_switchFlg:	.byte 0
replayHeader1_padding22:	.byte 0xFF, 0xFF
replayHeader1_padding24:	.long 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
replayHeader1_padding30:	.long 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
replayHeader2:
replayHeader2_world:		.byte 0
replayHeader2_level:		.byte 0
replayHeader2_area:		.byte 0
replayHeader2_entrance:	.byte 0
replayHeader2_seed:		.long 0
replayHeader2_padding08:	.long 0xFFFFFFFF, 0xFFFFFFFF
replayHeader2_playerID:	.long 0
replayHeader2_powerup:	.long 0
replayHeader2_playerFlg:	.long 0
replayHeader2_padding1C:	.long 0xFFFFFFFF
replayHeader2_padding20:	.byte 0xFF
replayHeader2_switchFlg:	.byte 0
replayHeader2_padding22:	.byte 0xFF, 0xFF
replayHeader2_padding24:	.long 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
replayHeader2_padding30:	.long 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
replayHeader3:
replayHeader3_world:		.byte 0
replayHeader3_level:		.byte 0
replayHeader3_area:		.byte 0
replayHeader3_entrance:	.byte 0
replayHeader3_seed:		.long 0
replayHeader3_padding08:	.long 0xFFFFFFFF, 0xFFFFFFFF
replayHeader3_playerID:	.long 0
replayHeader3_powerup:	.long 0
replayHeader3_playerFlg:	.long 0
replayHeader3_padding1C:	.long 0xFFFFFFFF
replayHeader3_padding20:	.byte 0xFF
replayHeader3_switchFlg:	.byte 0
replayHeader3_padding22:	.byte 0xFF, 0xFF
replayHeader3_padding24:	.long 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
replayHeader3_padding30:	.long 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF

# header size is 0x40 bytes

replayFooter: .long 0xFFFFFFFF

replayDataBuffers: .long 0, 0, 0, 0
replayDataBufferPointers: .long 0, 0, 0, 0
replayDataBufferSize: .long 0x80000

saveRandomSeed: .long 0


keepTiltValues: .long 0, 0, 0, 0

