#include <asm_setup.S>

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
