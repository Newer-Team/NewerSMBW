.text
.align 4
#include <asm_setup.S>

.extern returnFromGPSFASixth
.extern daEnItem_c__GetWhetherPlayerCanGetPowerupOrNot
.extern dAcPy_c__ChangePowerupWithAnimation
.extern PlayPlayerSound
.extern CreateActor
.extern PlayerProjectileShooting
.extern Actor_SearchByName
.extern SomeTable_802F5440
.extern SomeTable_802F5580
.extern SomeTable_802F56C0
.extern BlahTable
.extern continuePlumberSetPowerupTexture
.extern doneSettingThePowerupTexture
.extern doneSettingThePowerupTexture2
.extern returnFromPowerupSoundChange

.global DisableHatForHammer
DisableHatForHammer:
	lwz r0, 0x17C(r31)
	clrlwi. r0, r0, 31
	beq DontDrawTheFuckingHat_
	lbz r0, 0x152(r31)
	cmpwi r0, 7
	beq DontDrawTheFuckingHat_
	b DrawTheFuckingHat
DontDrawTheFuckingHat_:
	b DontDrawTheFuckingHat # this is a hack to avoid implementing R_PPC_REL14
.extern DontDrawTheFuckingHat
.extern DrawTheFuckingHat

.global GetInfoFromDumbTable
GetInfoFromDumbTable:
	lis r3, SomeTable@h
	ori r3, r3, SomeTable@l

	slwi r4, r4, 2
	slwi r0, r5, 6

	lwzx r3, r3, r4
	add r3, r3, r0
	blr




.global FukidashiFix
FukidashiFix:
	lis r7, ZeroFloat@h
	ori r7, r7, ZeroFloat@l
	
	lis r8, FukidashiTable2@h
	ori r8, r8, FukidashiTable2@l
	
	mulli r6, r4, 0x20
	slwi r0, r5, 2
	add r0, r0, r6
	
	lfs f0, 0(r7)
	lfsx f1, r8, r0
	
	stfs f0, 0x1DC(r3)
	stfs f1, 0x1E0(r3)
	stfs f0, 0x1E4(r3)
	
	blr




.global PlayerGetHeight
PlayerGetHeight:
	lwz r0, 0x14D4(r3)
	
	lwz r4, 0x1090(r3)
	slwi r4, r4, 3
	
	cmpwi r0, 0
	beq useNormal
	cmpwi r0, 1
	bne useDucking
	
useNormal:
	lis r3, PlayerHeightsNormal@h
	ori r3, r3, PlayerHeightsNormal@l
	b gotOffset
	
useDucking:
	lis r3, PlayerHeightsDucking@h
	ori r3, r3, PlayerHeightsDucking@l
	
gotOffset:
	lfsx f1, r3, r4
	blr



.global WtfEver
WtfEver:
	lis r3, WtfEverTable@h
	ori r3, r3, WtfEverTable@l
	lfsx f0, r3, r0
	blr



.global YetAnother
YetAnother:
	lis r3, YetAnotherTable@h
	ori r3, r3, YetAnotherTable@l
	lfsx f0, r3, r0
	blr




.global GetWeirdScaleTable
GetWeirdScaleTable:
	lis r5, WeirdScaleTable@h
	ori r5, r5, WeirdScaleTable@l
	lfsx f1, r5, r0
	blr





.global GetAsdfTable
GetAsdfTable:
	extsb r0, r4
	cmpwi r0, -1
	bne dontGetThisOne
	
	lwz r0, 0x1090(r3)
	extsb r4, r0
	
dontGetThisOne:
	extsb r0, r4
	
	lis r3, AsdfTable@h
	ori r3, r3, AsdfTable@l
	lbzx r3, r3, r0
	
	blr





.global GetBlahTableOffset
GetBlahTableOffset:
	lis r5, BlahTableOffsets@h
	ori r5, r5, BlahTableOffsets@l
	lwzx r30, r5, r4
	blr


.global DealWithBlahTable
DealWithBlahTable:
	lis r5, BlahTableOffsets@h
	ori r5, r5, BlahTableOffsets@l
	
	# points to BlahTable
	lis r4, BlahTable@h
	ori r4, r4, BlahTable@l
	
	# get blahtableoffset entry based on powerup
	lwz r0, 0x1090(r3)
	slwi r0, r0, 2
	lwzx r0, r5, r0
	
	# now get the entry from the blahtable
	slwi r0, r0, 2
	lwzx r4, r4, r0
	
	# do shit here
	
	lfs f3, 0(r4)
	stfs f3, 0x14E4(r3)
	stfs f3, 0x164(r3)
	
	lfs f1, 0x2A70(r3)
	lfs f0, 0x2A74(r3)
	
	lfs f2, 4(r4)
	fadds f1, f2, f1
	fadds f0, f0, f1
	stfs f0, 0x14E8(r3)
	stfs f0, 0x168(r3)
	
	lfs f0, 0x8(r4)
	lfs f1, 0xC(r4)
	stfs f0, 0x16C(r3)
	stfs f1, 0x170(r3)
	
	# hope I did this right
	blr





.global GetPowerupScaleFloatAddr_r6_trash_r0_valshl2_r4_dest
GetPowerupScaleFloatAddr_r6_trash_r0_valshl2_r4_dest:
	lis r6, PowerupScaleFloats@h
	ori r6, r6, PowerupScaleFloats@l
	add r4, r6, r0
	blr

.global GetPowerupScaleFloatAddr_r6_trash_r0_valshl2_r3_dest
GetPowerupScaleFloatAddr_r6_trash_r0_valshl2_r3_dest:
	lis r6, PowerupScaleFloats@h
	ori r6, r6, PowerupScaleFloats@l
	add r3, r6, r0
	blr

.global GetPowerupScaleFloatAddr_r8_trash_r0_valshl2_r7_dest
GetPowerupScaleFloatAddr_r8_trash_r0_valshl2_r7_dest:
	lis r8, PowerupScaleFloats@h
	ori r8, r8, PowerupScaleFloats@l
	add r7, r8, r0
	blr

.global GetPowerupScaleFloatAddr_fixForSixth
GetPowerupScaleFloatAddr_fixForSixth:
	lis r5, PowerupScaleFloats@h
	ori r5, r5, PowerupScaleFloats@l
	add r4, r5, r4
	b returnFromGPSFASixth



.global PlumberSetPowerupFix
PlumberSetPowerupFix:
	lbz r0, 0x152(r31) # get powerup id
	
	cmpwi r0, 2
	beq doFireTex
	
	cmpwi r0, 6
	beq doIceTex
	
	cmpwi r0, 7
	beq doHammerTex
	
	li r4, 0
	b gotTexID
	
doFireTex:
	li r4, 1
	b gotTexID
	
doIceTex:
	li r4, 2
	b gotTexID
	
doHammerTex:
	li r4, 3
	
gotTexID:
	lwz r12, 0(r31)
	mr r3, r31
	lwz r12, 0x34(r12)
	mtctr r12
	bctrl
	
	b doneSettingThePowerupTexture



.global PlumberSetPowerupFix2
PlumberSetPowerupFix2:
	cmpwi r31, 2
	beq doFireTex2
	
	cmpwi r31, 6
	beq doIceTex2
	
	cmpwi r31, 7
	beq doHammerTex2
	
	li r4, 0
	b gotTexID2
	
doFireTex2:
	li r4, 1
	b gotTexID2
	
doIceTex2:
	li r4, 2
	b gotTexID2
	
doHammerTex2:
	li r4, 3
	
gotTexID2:
	lwz r3, 0x2A64(r30)
	lwz r12, 0(r3)
	lwz r12, 0x34(r12)
	mtctr r12
	bctrl
	
	b doneSettingThePowerupTexture2




.global PlumberSetPowerupTextureFix
PlumberSetPowerupTextureFix:
	stb r4, 0x153(r3)
	
	lis r5, PowerupTextures@h
	ori r5, r5, PowerupTextures@l
	
	slwi r4, r4, 2
	lfsx f31, r5, r4
	
	b continuePlumberSetPowerupTexture



.global SetHammerToEnItemDCA
SetHammerToEnItemDCA:
	bl daEnItem_c__GetWhetherPlayerCanGetPowerupOrNot
	cmpwi r3, 1
	bne DontSetHammer
	
	li r0, 5
	sth r0, 0xDCA(r31)
	
DontSetHammer:
	lwz r0, 0x14(sp)
	lwz r31, 0xC(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr

.global SetHammerToEnItemDCASpecial
SetHammerToEnItemDCASpecial:
	cmplwi r4, 0x19
	b _not19
	li r0, 0xD
	sth r0, 0xDCA(r3)
_not19:
	cmplwi r4, 6
	bnelr
	li r0, 5
	sth r0, 0xDCA(r3)
	blr


#.global EnItem_BindAnimation_Fix
#EnItem_BindAnimation_Fix:
#	lhz r0, 0xDCA(r3)
#	cmpwi r0, 5
#	beqlr
#	stwu sp, -0x20(sp)
#	b EnItem_BindAnimation_Continued


.global FixPowerupSound
FixPowerupSound:
	#lis r3, DBString@h
	#ori r3, r3, DBString@l
	#lwz r4, 0x1090(r29)
	#crclr 4*cr1+eq
	#bl OSReport

	lwz r5, 0x14DC(r29) # Original powerup
	lwz r0, 0x1090(r29) # New powerup
	# If NEW is 1 (big) and OLD is not 0 or 3, then play 273 (powerdown)
	cmpwi r0, 1
	bne dontConsiderPowerdown

	cmpwi r5, 0
	beq dontConsiderPowerdown
	cmpwi r5, 3
	beq dontConsiderPowerdown
	# If the player is here, he was downgraded from a higher powerup to Big Mario
	# So play the powerdown sound
	mr r3, r29
	li r4, 273
	li r5, 0
	b returnFromPowerupSoundChange

dontConsiderPowerdown:
	lis r3, PowerupSounds@h
	ori r3, r3, PowerupSounds@l
	slwi r4, r0, 2
	lwzx r4, r3, r4
	mr r3, r29
	li r5, 0

	b returnFromPowerupSoundChange


.global WeirdAnimLoadHack
WeirdAnimLoadHack:
	cmplwi r4, 5 #hammersuit
	beqlr
	cmplwi r4, 6 #1up
	beqlr
	# neither of those succeeded
	crclr 4*cr0+eq
	blr



.global TryToGiveMarioHammerSuit
TryToGiveMarioHammerSuit:
	stwu sp, -0x10(sp)
	mflr r0
	stw r0, 0x14(sp)
	
	# check the obtained item number
	cmplwi r3, 5
	bne IsntGettingHammerSuit
	
	# check the current player powerup
	cmplwi r4, 7
	bne DontHaveHammerSuit
	
	li r28, 0
	li r29, 2
	b ReturnFromTTGMHS
	
DontHaveHammerSuit:
	mr r3, r30
	li r4, 7
	bl dAcPy_c__ChangePowerupWithAnimation
	
	cmpwi r3, 0
	beq ReturnFromTTGMHS
	
	li r29, 1
	
IsntGettingHammerSuit:
ReturnFromTTGMHS:
	# end of function
	lwz r0, 0x14(sp)
	mtlr r0
	addi sp, sp, 0x10
	
	# leftover instruction from what we replaced to add the bl
	cmpwi r29, 0
	blr



.global ThrowHammer
ThrowHammer:
	# continues on from PlayerProjectileShooting
	
	# get powerup
	lwz r3, 0x1090(r30)
	cmplwi r3, 7
	bne ReturnFromThrowHammer
	
	mr r3, r30
	li r4, 498
	li r5, 0
	bl PlayPlayerSound
	
	li r3, 0x73			# Hammer
	li r4, 0				# Settings
	addi r5, sp, 0x2C		# Position
	li r6, 0				# Rotation
	lbz r7, 0x38F(r30)	# Layer
	bl CreateActor
	
	# Store player ID
	lwz r4, 0(r30)
	stw r4, 0x450(r3)
	
	# store player number, but add 0x100 to differentiate it from 0
	lbz r4, 0x38D(r30)
	addi r4, r4, 0x100
	stw r4, 0x458(r3)
	
	# Fix scale
	lis r4, OneFloat@h
	ori r4, r4, OneFloat@l
	lfs f0, 0(r4)
	stfs f0, 0xDC(r3)
	stfs f0, 0xE0(r3)
	stfs f0, 0xE4(r3)
	
	mr r4, r30
	lwz r12, 0x60(r3)
	lwz r12, 0x8C(r12)
	mtctr r12
	bctrl
	
ReturnFromThrowHammer:
	lwz r0, 0xC4(sp)
	psq_l f31, 0xB8(sp), 1, 0
	lfd f31, 0xB0(sp)
	psq_l f30, 0xA8(sp), 1, 0
	lfd f30, 0xA0(sp)
	lwz r31, 0x9C(sp)
	lwz r30, 0x98(sp)
	mtlr r0
	addi sp, sp, 0xC0
	blr



.global MoreProjectileStuff_Fix
MoreProjectileStuff_Fix:
	lwz r3, 0x1090(r31)
	cmplwi r3, 7
	bne ReturnFromMPSFix
	
	mr r3, r31
	li r4, 1
	bl PlayerProjectileShooting
	
ReturnFromMPSFix:
	lwz r0, 0x14(sp)
	lwz r31, 0xC(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr



.global ProjectileShootCheck_Fix
ProjectileShootCheck_Fix:
	lwz r3, 0x1090(r31)
	cmplwi r3, 7
	bne Return0fromPSCFix
	
	lbz r3, 0x38D(r31)
	li r4, 0
	extsb r3, r3
	bl CheckHammerLimit
	
	cmpwi r3, 0
	beq Return0fromPSCFix
	
	b Return1fromPSCFix

Return0fromPSCFix:
	li r3, 0
	b ReturnFromPSCFix
	
Return1fromPSCFix:
	li r3, 1
	
ReturnFromPSCFix:
	lwz r0, 0x14(sp)
	lwz r31, 0xC(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr


.global CheckHammerLimit
CheckHammerLimit:
	stwu sp, -0x10(sp)
	mflr r0
	stw r0, 0x14(sp)
	stw r31, 0xC(sp)
	stw r30, 0x8(sp)
	
	# r30 = player number
	mr r30, r3
	addi r30, r30, 0x100
	
	# r31 = hammer count so far
	li r31, 0
	
	# search for every Hammer
	
	li r3, 0
	b startOrContinueLoop
loop:
	
	lwz r4, 0x458(r3)
	cmpw r4, r30
	bne startOrContinueLoop
	
	addi r31, r31, 1
	
startOrContinueLoop:
	mr r4, r3
	li r3, 0x73
	bl Actor_SearchByName
	
	cmpwi r3, 0
	bne loop
	
	# now if the current count is >= 2, return 0
	# otherwise return 1
	cmpwi r31, 2
	bge tooManyHammers
	
	li r3, 1
	b gotHammerStatus
	
tooManyHammers:
	li r3, 0
	
gotHammerStatus:
	lwz r0, 0x14(sp)
	lwz r31, 0xC(sp)
	lwz r30, 0x8(sp)
	mtlr r0
	addi sp, sp, 0x10
	blr




.extern continueToadPowerupSet
.global ToadPowerupFix
ToadPowerupFix:
	cmpwi r4, 3
	beq isHammerForToad

	lis r8, PowerupTextures@h
	ori r8, r8, PowerupTextures@l
	slwi r9, r4, 2
	lfsx f1, r8, r9
	fadds f31, f31, f1
	b continueToadPowerupSet

isHammerForToad:
	lis r8, ToadTextures@h
	ori r8, r8, ToadTextures@l
	slwi r9, r0, 2
	lfsx f31, r8, r9
	b continueToadPowerupSet






.global CheckExistingPowerup
CheckExistingPowerup:

	lwz r3, 0x14E0(r3)
	blr





.global AcPyCtorHS
AcPyCtorHS:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	mr r31, r3

	bl _ZN21dHammerSuitRenderer_c5buildEv
	stw r3, 0x2D08(r31)

	mr r3, r31
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global AcPyDtorHS
AcPyDtorHS:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	stw r30, 8(r1)

	mr r31, r4
	mr r30, r3

	cmpwi r3, 0
	beq oops2
	lwz r3, 0x2D08(r30)
	li r4, 1
	bl _ZN21dHammerSuitRenderer_cD1Ev
oops2:
	
	mr r3, r30
	mr r4, r31
	bl RealAcPyDtor

	lwz r30, 8(r1)
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global AcPySetMeUpHS
AcPySetMeUpHS:
	# partway in: r31 = this
	lwz r3, 0x2D08(r31)
	addi r4, r31, 0x2A60
	bl _ZN21dHammerSuitRenderer_c5setupEP21dPlayerModelHandler_c

	# epilogue
	li r3, 1
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global AcPyDrawHS
AcPyDrawHS:
	# partway in: r31 = this
	lwz r3, 0x2D08(r31)
	bl _ZN21dHammerSuitRenderer_c4drawEv

	# epilogue
	li r3, 1
	lwz r0, 0x14(r1)
	lwz r31, 0xC(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr



# DO IT AGAIN
.global Wm2dPlayerCtorHS
Wm2dPlayerCtorHS:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	mr r31, r3

	bl _ZN21dHammerSuitRenderer_c5buildEv
	stw r3, 0x384(r31)

	mr r3, r31
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global Wm2dPlayerDtorHS
Wm2dPlayerDtorHS:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	stw r30, 8(r1)

	mr r31, r4
	mr r30, r3

	cmpwi r3, 0
	beq oops
	lwz r3, 0x384(r30)
	li r4, 1
	bl _ZN21dHammerSuitRenderer_cD1Ev
oops:
	
	mr r3, r30
	mr r4, r31
	bl RealWm2dPlayerDtor

	lwz r30, 8(r1)
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.global Wm2dPlayerSetMeUpHS
Wm2dPlayerSetMeUpHS:
	# partway in: r27 = this
	lwz r3, 0x384(r27)
	lwz r4, 0x128(r27)
	li r5, 1
	bl _ZN21dHammerSuitRenderer_c5setupEP21dPlayerModelHandler_ci

	# epilogue
	addi r11, r1, 0x20
	bl _restgpr_27
	lwz r0, 0x24(r1)
	mtlr r0
	addi r1, r1, 0x20
	blr

.global Wm2dPlayerDrawHS
Wm2dPlayerDrawHS:
	# dicks (a very insightful comment)
	lbz r0, 0x261(r3)
	cmpwi r0, 0
	beqlr

	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)
	mr r31, r3

	lwz r3, 0x384(r31)
	bl _ZN21dHammerSuitRenderer_c4drawEv

	lwz r3, 0x128(r31)
	bl draw__21dPlayerModelHandler_cFv

	lwz r0, 0x14(r1)
	lwz r31, 0xC(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr

.extern _restgpr_27
.extern RealAcPyDtor
.extern RealWm2dPlayerDtor
.extern _ZN21dHammerSuitRenderer_c5buildEv
.extern _ZN21dHammerSuitRenderer_cD1Ev
.extern _ZN21dHammerSuitRenderer_c5setupEP21dPlayerModelHandler_c
.extern _ZN21dHammerSuitRenderer_c5setupEP21dPlayerModelHandler_ci
.extern _ZN21dHammerSuitRenderer_c4drawEv
.extern draw__21dPlayerModelHandler_cFv

.extern GabonRock_DropOutState
.extern GabonRock_CrushState


.global HammerGabonRock
HammerGabonRock:
	# r27 = apOther
	# r28 = apThis
	# r29 = objOther
	# r31 = objThis
	lbz r0, 0x2D(r27)
	cmpwi r0, 19
	bne hgrExit

	addi r3, r31, 0x394
	lis r4, GabonRock_CrushState@h
	ori r4, r4, GabonRock_CrushState@l
	lwz r12, 0(r3)
	lwz r12, 0x18(r12)
	mtctr r12
	bctrl

	lbz r0, 0xA1(r28)
	ori r0, r0, 2
	stb r0, 0xA1(r28)
hgrExit:
	lwz r27, 0x2C(r1)
	lwz r28, 0x30(r1)
	lwz r29, 0x34(r1)
	lwz r30, 0x38(r1)
	lwz r31, 0x3C(r1)
	lwz r0, 0x44(r1)
	mtlr r0
	addi r1, r1, 0x40
	blr

.global HammerCallbackGabonRockOverride
.extern forceHCGROPushBack
.extern continueFromHCGRO
HammerCallbackGabonRockOverride:
	lhz r3, 8(r30) #objThis
	cmpwi r3, 115 #HAMMER
	bne continueHCGRO

	lhz r3, 8(r30) #objOther
	cmpwi r3, 374 #GABON_ROCK
	bne continueHCGRO

	b forceHCGROPushBack

continueHCGRO:
	lbz r0, 0x2D(r4) #to fix the branch we nuked
	b continueFromHCGRO


.data

PowerupSounds:
	.long 273, 274, 274, 275, 276, 277, 274, 274


SomeTable:
	.long SomeTable_802F5580
	.long SomeTable_802F56C0
	.long SomeTable_802F56C0
	.long SomeTable_802F5440
	.long SomeTable_802F56C0
	.long SomeTable_802F56C0
	.long SomeTable_802F56C0
	.long SomeTable_802F56C0

ZeroFloat:
	.float 0.0

OneFloat:
	.float 1.0

FukidashiTable2:
	.float 20.0, 32.0, 32.0, 12.0, 38.0, 33.0, 32.0, 32.0
	.float 20.0, 34.0, 34.0, 12.0, 39.0, 35.0, 34.0, 34.0
	.float 20.0, 29.0, 29.0, 11.0, 34.0, 29.0, 29.0, 29.0
	.float 20.0, 29.0, 29.0, 11.0, 34.0, 29.0, 29.0, 29.0

PlayerHeightsNormal:
	.float 16.0, 27.0, 27.0, 12.0, 27.0, 27.0, 27.0, 27.0

PlayerHeightsDucking:
	.float 13.0, 21.0, 21.0, 9.0, 21.0, 21.0, 21.0, 21.0

WtfEverTable:
	.float 4.5, 10.0, 10.0, 1.0, 10.0, 9.0, 10.0, 10.0

YetAnotherTable:
	.float 0.0, -4.0, -4.0, 4.0, -6.0, -2.0, -4.0, -4.0

WeirdScaleTable:
	.float 16.0, 31.0, 31.0, 6.0, 34.0, 31.0, 31.0, 31.0

AsdfTable:
	.byte 1, 2, 2, 0, 2, 2, 2, 2

BlahTableOffsets:
	.long 1, 2, 2, 0, 3, 2, 2, 2

PowerupScaleFloats:
	.long 0xDEADBEEF, 0xDEADBEEF # padding
	.float 1.0
	.float 1.0
	.float 1.0
	.float 0.6
	.float 1.0
	.float 1.0
	.float 1.0
	.float 1.0

PowerupTextures:
	.float 0.0
	.float 1.0
	.float 2.0
	.float 3.0
ToadTextures:
	.long 0xDEADBEEF, 0xDEADBEEF
	.float 7.0, 8.0, 6.0

.global BrosArcFileName
BrosArcFileName:
	.string "hammerM"

.global I_hammerArcFilename
I_hammerArcFilename:
	.string "I_hammer"

.global I_hammerResFile
I_hammerResFile:
#	.string "g3d/I_hammer.brres"
	.string "g3d/I_fireflower.brres"

.global I_hammerModelName
I_hammerModelName:
#	.string "I_hammer"
	.string "I_fireflower"

.global g3d_I_kinoko_brres
g3d_I_kinoko_brres: .string "g3d/I_kinoko.brres"

.global I_propeller_model
I_propeller_model: .string "I_propeller_model"

.global SI_kinoko
SI_kinoko: .string "SI_kinoko"

.global SI_hammer
SI_hammer: .string "SI_hammer"

g3d: .string "g3d/"

.text
.global AddHSToBattleItem_FixWait2Anim
AddHSToBattleItem_FixWait2Anim:
	lis r4, wait2@h
	ori r4, r4, wait2@l
	blr

.global InsertLostG3D
InsertLostG3D:
	lis r4, g3d@h
	ori r4, r4, g3d@l
	blr
.data
wait2: .string "wait2"

#PowerupTexDebugStr:
#	.string "Plumber::SetPowerupTexture() called on class %p with texnum %d lr=%p\n"

#DBString:
#	.string "PW:%d\n"

#.global BrosModelFilename
#BrosModelFilename:
#	.string "g3d/bros.brres"

#.global HammerModelName
#HammerModelName:
#	.string "bros_hammer"

.text
.extern continueFromStrongBox
.global StrongboxInit
StrongboxInit:
	# MODE
	li r4, 2
	stw r4, 0x8E0(r30)
	# ITEM COUNT (nybble 8)
	extrwi r4, r7, 4, 12
	stw r4, 0x8E8(r30)
	b continueFromStrongBox

.extern continueFromStrongBox2
.extern AddStockPowerup
.extern create__13dStageActor_cF6ActorsUiP4VEC3P6S16VecUc
.global StrongboxGiveBattleItem
StrongboxGiveBattleItem:
	# save a reg we can use
	stw r29, 0x24(r1)

	# get number of current item
	lhz r6, 0x8FA(r30)
	# create a shift amount...
	slwi r6, r6, 2
	lwz r7, 4(r30)
	# ..and use it to get the item
	srw r7, r7, r6
	clrlwi r29, r7, 28

	# now give it
	lis r4, IndexItemsForStockItem@h
	ori r4, r4, IndexItemsForStockItem@l
	lbzx r4, r4, r29
	bl AddStockPowerup

	# and then copy the position
	lfs f1, 0xAC(r30)
	stfs f1, 0x10(r1)
	lfs f1, 0xB0(r30)
	lfs f2, 0x8D8(r30)
	fsubs f1, f1, f2
	stfs f1, 0x14(r1)
	lfs f1, 0xB4(r30)
	stfs f1, 0x18(r1)

	li r3, 0x25
	slwi r4, r31, 4
	or r4, r4, r29
	addi r5, r1, 0x10
	li r6, 0
	li r7, 0
	bl create__13dStageActor_cF6ActorsUiP4VEC3P6S16VecUc

	# restore our reg
	lwz r29, 0x24(r1)
	b continueFromStrongBox2

.data
IndexItemsForStockItem:
.byte 0,1,2,3,5,6,4,7


SLGWindow: .string "A00_Window"
SLGBtn0: .string "B00_itemButton"
SLGBtn1: .string "B01_itemButton"
SLGBtn2: .string "B02_itemButton"
SLGBtn3: .string "B03_itemButton"
SLGBtn4: .string "B04_itemButton"
SLGBtn5: .string "B05_itemButton"
SLGBtn6: .string "B06_itemButton"
SLGBtn7: .string "HammerGroup"
SLButtonBase00: .string "P_buttonBase_00"
SLButtonBase01: .string "P_buttonBase_01"
SLButtonBase02: .string "P_buttonBase_02"
SLButtonBase03: .string "P_buttonBase_03"
SLButtonBase04: .string "P_buttonBase_04"
SLButtonBase05: .string "P_buttonBase_05"
SLButtonBase06: .string "P_buttonBase_06"
SLButtonBase07: .string "P_buttonBase_07"
SLIcon0: .string "P_iconKinoko_00"
SLIcon1: .string "P_iconFlower_00"
SLIcon2: .string "P_iconPro_00"
SLIcon3: .string "P_iconIce_00"
SLIcon4: .string "P_iconPen_00"
SLIcon5: .string "P_mameKinoko_00"
SLIcon6: .string "P_iconStar_00"
SLIcon7: .string "P_iconHammer_00"
SLIconPane0: .string "N_iconKinoko_00"
SLIconPane1: .string "N_iconFlower_00"
SLIconPane2: .string "N_iconPro_00"
SLIconPane3: .string "N_iconIce_00"
SLIconPane4: .string "N_iconPen_00"
SLIconPane5: .string "N_mameKinoko_00"
SLIconPane6: .string "N_iconStar_00"
SLIconPane7: .string "N_iconHammer_00"
.align 4
SILayoutGroupNames:
.long SLGWindow
.long SLGBtn0,SLGBtn1,SLGBtn2,SLGBtn6,SLGBtn4,SLGBtn5,SLGBtn3
.long SLGBtn0,SLGBtn1,SLGBtn2,SLGBtn6,SLGBtn4,SLGBtn5,SLGBtn3
.long SLGBtn0,SLGBtn1,SLGBtn2,SLGBtn6,SLGBtn4,SLGBtn5,SLGBtn3
.long SLGBtn0,SLGBtn1,SLGBtn2,SLGBtn6,SLGBtn4,SLGBtn5,SLGBtn3
.long SLGWindow
.long SLGBtn7,SLGBtn7,SLGBtn7
SILayoutGroupIDs:
.long 0
.long 1,1,1,1,1,1,1
.long 2,2,2,2,2,2,2
.long 3,3,3,3,3,3,3
.long 4,4,4,4,4,4,4
.long 5
.long 1,3,4

SILayoutButtonBases:
.long SLButtonBase00,SLButtonBase01,SLButtonBase02,SLButtonBase06
.long SLButtonBase04,SLButtonBase05,SLButtonBase03,SLButtonBase07
SILayoutIconPictures:
.long SLIcon0,SLIcon1,SLIcon2,SLIcon3
.long SLIcon4,SLIcon5,SLIcon6,SLIcon7
SILayoutIconPanes:
.long SLIconPane0,SLIconPane1,SLIconPane2,SLIconPane3
.long SLIconPane4,SLIconPane5,SLIconPane6,SLIconPane7

.text
.global SILayoutGroupParams
SILayoutGroupParams:
	lis r4, SILayoutGroupNames@h
	ori r4, r4, SILayoutGroupNames@l
	lis r5, SILayoutGroupIDs@h
	ori r5, r5, SILayoutGroupIDs@l
	li r6, 0x21
	blr

# - 0x920 : Button Bases
# - 0x940 : Icon Panes
# - 0x960 : Icon Pictures
.extern getPanes__Q23m2d13EmbedLayout_cCFPPCcPPQ34nw4r3lyt4Panei
.extern getPictures__Q23m2d13EmbedLayout_cCFPPCcPPQ34nw4r3lyt7Picturei
.extern ContinueFromSILayoutExtraStuffs
.extern IsWideScreen__Fv
.global SILayoutExtraStuffs
SILayoutExtraStuffs:
	addi r3, r30, 0x74
	lis r4, SILayoutButtonBases@h
	ori r4, r4, SILayoutButtonBases@l
	addi r5, r30, 0x920
	li r6, 8
	bl getPictures__Q23m2d13EmbedLayout_cCFPPCcPPQ34nw4r3lyt7Picturei

	addi r3, r30, 0x74
	lis r4, SILayoutIconPanes@h
	ori r4, r4, SILayoutIconPanes@l
	addi r5, r30, 0x940
	li r6, 8
	bl getPanes__Q23m2d13EmbedLayout_cCFPPCcPPQ34nw4r3lyt4Panei

	addi r3, r30, 0x74
	lis r4, SILayoutIconPictures@h
	ori r4, r4, SILayoutIconPictures@l
	addi r5, r30, 0x960
	li r6, 8
	bl getPictures__Q23m2d13EmbedLayout_cCFPPCcPPQ34nw4r3lyt7Picturei

	bl IsWideScreen__Fv
	cmpwi r3, 1
	beq dontScaleDownSI
	lis r3, float0_85@h
	ori r3, r3, float0_85@l
	lfs f1, 0(r3)
	lwz r3, 0x248(r30)
	stfs f1, 0x44(r3)
	stfs f1, 0x48(r3)
dontScaleDownSI:

	li r3, 1
	b ContinueFromSILayoutExtraStuffs

.global HideItemsByDefault
.extern ContinueFromHideItemsByDefault
HideItemsByDefault:
	li r3, 8
	mtctr r3
	mr r4, r31
meNext:
	lwz r9, 0x960(r4)
	lbz r5, 0xBB(r9)
	rlwinm r5, r5, 0,24,30
	stb r5, 0xBB(r9)
	addi r4, r4, 4
	bdnz meNext

	li r8, 0
	li r7, 1
	li r6, 4
	li r5, 0xE
	li r0, 0xF
	li r3, 1
	b ContinueFromHideItemsByDefault

.global AddHammerPowerupFinalValue
AddHammerPowerupFinalValue:
	li r24, 0
	lis r22, OutputPowerupsByItemID@h
	ori r22, r22, OutputPowerupsByItemID@l
	blr

.data
OutputPowerupsByItemID:
.long 1, 2, 4, 6, 5, 3, -1, 7

.text
.global FixStockItemEffectPlayer
FixStockItemEffectPlayer:
	lwz r0, 0x8A0(r30)
	slwi r0, r0, 2
	add r5, r30, r0
	lwz r28, 0x940(r5)
	blr

.global FixStockItemEffectPlayer2
FixStockItemEffectPlayer2:
	lwz r0, 0x8A0(r30)
	stw r0, 0x8D0(r30)
	blr

.global FixMoreSIEffectRelatedCrap
FixMoreSIEffectRelatedCrap:
	lwz r0, 0x8D0(r31)
	slwi r0, r0, 2
	add r3, r31, r0
	lwz r29, 0x940(r3)
	blr


.global AddHammerSuitAddSound
.extern MakeSFISound
.extern ContinueFromSIAddSound
AddHammerSuitAddSound:
	cmpwi r0, 7
	beq _MakeSFISound
	b ContinueFromSIAddSound
	_MakeSFISound:
	b MakeSFISound

.extern resetAnim__Q23m2d13EmbedLayout_cFib
.extern ContinueFromResetHammerAnimOnOpen
.global ResetHammerAnimOnOpen
ResetHammerAnimOnOpen:
	addi r3, r28, 0x74
	li r4, 0x1E
	li r5, 0
	bl resetAnim__Q23m2d13EmbedLayout_cFib
	addi r3, r28, 0x74
	b ContinueFromResetHammerAnimOnOpen

.extern ChosenStartChoice
.global PickHammerAsPossibleStartChoice
PickHammerAsPossibleStartChoice:
	lwz r0, 0x918(r30)
	li r3, 6
	cmpwi r0, 0
	beq notGoodEnough
	stw r3, 0x8A0(r30)
	b ChosenStartChoice
notGoodEnough:
	lwz r0, 0x91C(r30)
	li r3, 7
	cmpwi r0, 0
	beq _ChosenStartChoice
	stw r3, 0x8A0(r30)
	_ChosenStartChoice:
	b ChosenStartChoice


.global ChooseOldButtonOffAnim
ChooseOldButtonOffAnim:
	cmpwi r4, 7
	beq useHammerOffAnim
	addi r4, r4, 0x16
	blr
useHammerOffAnim:
	li r4, 0x20
	blr

.global ChooseOldButtonOffAnim2
ChooseOldButtonOffAnim2:
	cmpwi r3, 7
	beq useHammerOffAnim2
	addi r0, r3, 0x16
	blr
useHammerOffAnim2:
	li r0, 0x20
	blr

.global ChooseButtonOnAnim
ChooseButtonOnAnim:
	cmpwi r4, 7
	beq useHammerOnAnim
	addi r4, r4, 1
	blr
useHammerOnAnim:
	li r4, 0x1E
	blr

.global DoSISelectCursor
.extern UpdateSelectCursor
DoSISelectCursor:
	slwi r0, r0, 2
	add r3, r3, r0
	lwz r3, 0x920(r3)
	li r4, 0
	li r5, 0
	b UpdateSelectCursor


.global ChooseHitButtonAnim
ChooseHitButtonAnim:
	cmpwi r4, 7
	beq useHammerHitAnim
	addi r4, r4, 0xF
	blr
useHammerHitAnim:
	li r4, 0x1F
	blr



.global UpdateCalcPowerupResult
.extern ContinueFromCalcPowerupResult
UpdateCalcPowerupResult:
	cmpwi r3, 7
	bne cprIsDone
	li r6, 7
	stb r0, 0xF(r1)
cprIsDone:
	b ContinueFromCalcPowerupResult

.global FixCalcPowerupResultForMushroom
.extern ContinueFromFixCalcPowerupResultForMushroom
FixCalcPowerupResultForMushroom:
	stb r0, 0xF(r1)
	li r6, 1
	b ContinueFromFixCalcPowerupResultForMushroom



.global SISLayoutExtraStuffs
.extern ContinueFromSISLayoutExtraStuffs
.extern findTextBoxByName__Q23m2d17EmbedLayoutBase_cCFPCc
.extern findPictureByName__Q23m2d17EmbedLayoutBase_cCFPCc
SISLayoutExtraStuffs:
	addi r3, r30, 0x70
	lis r4, whatever_s@h
	ori r4, r4, whatever_s@l
	bl findTextBoxByName__Q23m2d17EmbedLayoutBase_cCFPCc
	stw r3, 0x288(r30)

	addi r3, r30, 0x70
	lis r4, fuck_brlyts@h
	ori r4, r4, fuck_brlyts@l
	bl findTextBoxByName__Q23m2d17EmbedLayoutBase_cCFPCc
	stw r3, 0x284(r30)

	addi r3, r30, 0x70
	lis r4, im_tired_of_this@h
	ori r4, r4, im_tired_of_this@l
	bl findPictureByName__Q23m2d17EmbedLayoutBase_cCFPCc
	stw r3, 0x28C(r30)
	
	li r3, 1
	b ContinueFromSISLayoutExtraStuffs

.data
whatever_s: .string "whatever"
fuck_brlyts: .string "fuck_brlyts"
im_tired_of_this: .string "im_tired_of_this"

.text
.global UpdateSIS
.extern ContinueFromUpdateSIS
.extern WriteNumberToTextBox__FPiPCiPQ34nw4r3lyt7TextBoxb
UpdateSIS:
	cmpwi r0, 0
	beq DontUpdateSIS
	subi r25, r25, 0x1C

	# replicate this shit
	lwz r0, 0x280(r25)
	cmpwi r0, 0
	lwz r3, 0x28C(r25)
	lbz r0, 0xBB(r3)
	beq theresNone

	li r4, 1
	rlwinm r0, r0, 0,24,30
	b changedBase
theresNone:
	li r4, 0
	ori r0, r0, 1
changedBase:
	stb r0, 0xBB(r3)
	slwi r29, r4, 2
	addi r5, r1, 0x14
	lwzx r27, r26, r29
	li r4, 0
	stw r27, 0x14(r1)
	lwz r3, 0x284(r25)
	lwz r12, 0(r3)
	lwz r12, 0x28(r12)
	mtctr r12
	bctrl

	lwzx r29, r28, r29
	addi r5, r1, 0x10
	stw r29, 0x10(r1)
	li r4, 2
	lwz r3, 0x284(r25)
	lwz r12, 0(r3)
	lwz r12, 0x28(r12)
	mtctr r12
	bctrl

	addi r5, r1, 0x14
	li r4, 0
	lwz r3, 0x288(r25)
	lwz r12, 0(r3)
	lwz r12, 0x28(r12)
	mtctr r12
	bctrl

	addi r5, r1, 0x10
	li r4, 2
	lwz r3, 0x288(r25)
	lwz r12, 0(r3)
	lwz r12, 0x28(r12)
	mtctr r12
	bctrl

	lwz r5, 0x284(r25)
	addi r3, r25, 0x280
	lis r4, ptrToTwo@h
	ori r4, r4, ptrToTwo@l
	li r6, 1
	bl WriteNumberToTextBox__FPiPCiPQ34nw4r3lyt7TextBoxb

DontUpdateSIS:
	addi r11, r1, 0x40
	b ContinueFromUpdateSIS

.data
.align 4
ptrToTwo: .long 2
float0_85: .float 0.85

.text
.global SkawoHatesMe, SkawoStillHatesMe

SkawoHatesMe:
	lwzx r4, r27, r0
	cmpwi r0, 28
	bnelr
	lis r5, FireflowerFileName@h
	ori r5, r5, FireflowerFileName@l
	blr
SkawoStillHatesMe:
	lwzx r4, r4, r0
	cmpwi r0, 28
	bnelr
	lis r4, FireflowerModelName@h
	ori r4, r4, FireflowerModelName@l
	blr
.data
FireflowerFileName: .string "g3d/I_fireflower.brres"
FireflowerModelName: .string "I_fireflower"
.align 4

