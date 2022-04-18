#ifndef __MWERKS__
.set r0,0;   .set r1,1;   .set r2,2; .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31; .set f0,0; .set f1,1; .set f2,2
.set f3,3; .set f4,4; .set f5,5; .set f6,6; .set f8,8
.set f31,31; .set f30,30
#endif


.text

.extern OSReport
.extern Actor_GetVec3WithValuesAdded
.extern ConvertStagePositionIntoScreenPosition__Maybe
.extern AnotherSoundRelatedFunction
.extern YetAnotherSoundPlayer
.extern CheckIfPlayingSound
.extern StopSoundBasic
.extern StopSoundRelated
.extern SoundClassRelated
.extern BufferToCheckIfPlaying

.align 4



# PlaySound(dActor_c *, int soundID)


.global PlaySound
PlaySound:
	stwu      r1, -0x30(r1)
	mflr      r0
	stw       r0, 0x34(r1)
	stw       r31, 0x2C(r1)
	stw       r30, 0x28(r1)
	mr        r30, r4

	# Gets Player Position
	mr        r4, r3                          # object
	addi      r3, r1, 0x10                    # dest
	bl        Actor_GetVec3WithValuesAdded
	
	# Gives an accurate position for the sound
	addi      r3, r1, 8
	addi      r4, r1, 0x10
	bl        ConvertStagePositionIntoScreenPosition__Maybe
	
	# Checks if the passed sound is already playing
	lis       r4, BufferToCheckIfPlaying@h
	ori       r4, r4, BufferToCheckIfPlaying@l
	lwz       r3, 0(r4)
	mr        r4, r30
	bl CheckIfPlayingSound

	# Go away if it is
	cmpwi r3, 0
	li r3, 0
	bne GoAwaySoundGuy
	
	# If we're good to go, play the sound!
	lis       r31, SoundClassRelated@h
	ori       r31, r31, SoundClassRelated@l
	lwz       r3, 0(r31)
	addi      r5, r1, 8
	mr        r4, r30
	li        r6, 0
	bl        YetAnotherSoundPlayer
	
	b GoAwaySoundGuy
	
GoAwaySoundGuy:
	lwz       r30, 0x28(r1)
	lwz       r31, 0x2C(r1)
	lwz       r0, 0x34(r1)
	mtlr      r0
	addi      r1, r1, 0x30
	blr



.global PlaySoundAsync
PlaySoundAsync:
	stwu      r1, -0x30(r1)
	mflr      r0
	stw       r0, 0x34(r1)
	stw       r31, 0x2C(r1)
	stw       r30, 0x28(r1)
	mr        r30, r4

	# Gets Player Position
	mr        r4, r3                          # object
	addi      r3, r1, 0x10                    # dest
	bl        Actor_GetVec3WithValuesAdded
	
	# Gives an accurate position for the sound
	addi      r3, r1, 8
	addi      r4, r1, 0x10
	bl        ConvertStagePositionIntoScreenPosition__Maybe
		
	# If we're good to go, play the sound!
	lis       r31, SoundClassRelated@h
	ori       r31, r31, SoundClassRelated@l
	lwz       r3, 0(r31)
	addi      r5, r1, 8
	mr        r4, r30
	li        r6, 0
	bl        YetAnotherSoundPlayer
	
	lwz       r30, 0x28(r1)
	lwz       r31, 0x2C(r1)
	lwz       r0, 0x34(r1)
	mtlr      r0
	addi      r1, r1, 0x30
	blr




# StopSound(int soundID)

.global StopSound
StopSound:

	stwu      r1, -0x10(r1)
	mflr      r0
	stw       r0, 0x14(r1)

	mr        r4, r3
	lwz       r3, -0x5218(r13)
	bl        StopSoundBasic
	
	lwz       r0, 0x14(r1)
	mtlr      r0
	
	blr




# No where else to put this at the time, fuckit
.global KnockMarioBack
KnockMarioBack:

    stwu      r1, -0x10(r1)
    mflr      r0
    stw       r0, 0x14(r1)
    stw       r31, 0xC(r1)
    stw       r30, 0x8(r1)

	lwz       r3, 4(r3)
	mr        r4, r4
	li        r5, 3
	lwz       r12, 0x60(r3)
	lwz       r12, 0x3F4(r12)
	mtctr     r12
	bctrl

    lwz       r0, 0x14(r1)
    lwz       r4, 0xC(r1)
    lwz       r3, 0x8(r1)
    mtlr      r0
    addi      r1, r1, 0x10
	blr