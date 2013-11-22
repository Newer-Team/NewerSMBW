.extern sprintf
.extern GenerateRandomNumber
.extern BombSleepState
.extern Cloud_death_execute
.extern Cloud_execute

#include <asm_setup.S>
.text
.align 4

.global BigBlockSound
.extern Delete__7fBase_cFv
.extern PlaySoundWithFunctionB4
.extern SoundRelatedClass
.extern DetachSound__Q34nw4r3snd11SoundHandleFv
BigBlockSound:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 8(r1)
	mr r31, r3

	# create a SoundHandle
	li r3, 0
	stw r3, 0xC(r1)

	# call PSWFB4
	lis r3, SoundRelatedClass@h
	ori r3, r3, SoundRelatedClass@l
	lwz r3, 0(r3)
	addi r4, r1, 0xC
	li r5, 510
	li r6, 1
	bl PlaySoundWithFunctionB4

	# destroy SoundHandle
	addi r3, r1, 0xC
	bl DetachSound__Q34nw4r3snd11SoundHandleFv

	mr r3, r31
	lwz r31, 8(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	b Delete__7fBase_cFv

.global BigBlockLeapingItem
BigBlockLeapingItem:
	oris r0, r0, 0xD00
	blr

.global BooZOrder
BooZOrder:
	mr r3, r30
	lwz r4, 4(r30)
	extrwi. r4, r4, 4,4
	beqlr

	lis r4, Float_5000@h
	ori r4, r4, Float_5000@l
	lfs f1, 0xB4(r30)
	lfs f2, 0(r4)
	fadds f1, f1, f2
	stfs f1, 0xB4(r30)
	blr

.global GetTexFilenameForR5, TexFilenameBuffer, TexFormatString

GetTexFilenameForR5:
	# obj number should be put into r5
	# returned filename is in r5
	# this makes it easy to use with dRes_c::GetResource

	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r3, 0x1C(sp)
	stw r4, 0x18(sp)
	stw r6, 0x14(sp)
	stw r7, 0x10(sp)
	stw r8, 0xC(sp)
	stw r9, 0x8(sp)

	lis r3, TexFilenameBuffer@h
	ori r3, r3, TexFilenameBuffer@l

	lis r4, TexFormatString@h
	ori r4, r4, TexFormatString@l

	crclr 4*cr1+eq
	bl sprintf

	lis r5, TexFilenameBuffer@h
	ori r5, r5, TexFilenameBuffer@l

	lwz r0, 0x24(sp)
	lwz r3, 0x1C(sp)
	lwz r4, 0x18(sp)
	lwz r6, 0x14(sp)
	lwz r7, 0x10(sp)
	lwz r8, 0xC(sp)
	lwz r9, 0x8(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr


.global TEX_YoshiEggSetColour
TEX_YoshiEggSetColour:

	stwu sp, -0x30(sp)
	mflr r0
	stw r0, 0x24(sp)
	mr r9, r3 
	stw r4, 0x18(sp)
	stw r5, 0x14(sp)
	stw r6, 0x10(sp)
	stw r7, 0x0C(sp)

	li r3, 10
	bl GenerateRandomNumber
	stw r3, 0x5F8(r9)

	lwz r0, 0x24(sp)
	mr r3, r9
	lwz r4, 0x18(sp)
	lwz r5, 0x14(sp)
	lwz r6, 0x10(sp)
	lwz r7, 0x0C(sp)
	mtlr r0
	addi sp, sp, 0x30
	blr


.global TEX_YoshiEggSetBrres
TEX_YoshiEggSetBrres:

	lwz r5, 0x5F8(r27)
	clrlwi r5, r5, 28
	
	cmpwi r5, 4
	blt YoshiEggOne
	
	cmpwi r5, 8
	blt YoshiEggTwo
	
	li r5, 2
	b GetTexFilenameForR5
	
	
YoshiEggOne:
	li r5, 0
	b GetTexFilenameForR5

YoshiEggTwo:
	li r5, 1
	b GetTexFilenameForR5



.global TEX_YoshiEggSetFrame
TEX_YoshiEggSetFrame:
	
	lwz r5, 0x5F8(r27)
	clrlwi r5, r5, 30
	blr

	
.global TEX_YoshiEggEffect
TEX_YoshiEggEffect:

	lwz r0, 0x5F8(r3)
	clrlwi r0, r0, 30
	blr


.global TEX_YoshiSetColour
TEX_YoshiSetColour:

	lwz r4, 0x5F8(r31)
	oris r4, r4, 1
	blr
	

.global TEX_YoshiHack
TEX_YoshiHack:
	
	stwu sp, -0x30(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r3, 0x1C(sp)
	stw r4, 0x18(sp)
	stw r5, 0x14(sp)
	stw r6, 0x10(sp)
	stw r7, 0x0C(sp)
	stw r8, 0x08(sp)
	stw r9, 0x04(sp)

	lwz r5, 0x24C(r31)
	clrlwi r5, r5, 28
		
	lis r3, TexFilenameBuffer@h
	ori r3, r3, TexFilenameBuffer@l

	lis r4, TexFormatString@h
	ori r4, r4, TexFormatString@l

	crclr 4*cr1+eq
	bl sprintf

	lis r5, TexFilenameBuffer@h
	ori r5, r5, TexFilenameBuffer@l

	lis r4, YoshiArc@h
	ori r4, r4, YoshiArc@l
	
	lwz r0, 0x24(sp)
	lwz r3, 0x1C(sp)
	lwz r6, 0x10(sp)
	lwz r7, 0x0C(sp)
	lwz r8, 0x08(sp)
	lwz r9, 0x04(sp)
	mtlr r0
	addi sp, sp, 0x30
	blr




.global TEX_WoodBox
TEX_WoodBox:
	lwz r5, 4(r30)
	srwi r5, r5, 24
	b GetTexFilenameForR5


.global TEX_Bush
TEX_Bush:
	srwi r5, r27, 28
	b GetTexFilenameForR5


.global TEX_TobiPuku
TEX_TobiPuku:
	lwz r5, 4(r28)
	andi. r5, r5, 0xF
	b GetTexFilenameForR5

.global TobiPukuBypass
TobiPukuBypass:
	slwi r4, r30, 28
	lwz r5, 4(r29)
	clrlwi r5, r5, 28
	or r4, r4, r5
	blr

.global TobiPukuEffectChange
TobiPukuEffectChange:
	lwz r9, 4(r30)
	clrlwi r9, r9, 28
	cmpwi r9, 1
	bne dontChangeEff_
	lis r4, magmawave@h
	ori r4, r4, magmawave@l
dontChangeEff_:
	bctr

.data
magmawave: .string "Wm_en_magmawave"
.text

.global TobiPukuSoundChange
TobiPukuSoundChange:
	lwz r9, 4(r30)
	clrlwi r9, r9, 28
	cmpwi r9, 1
	li r4, 644
	beqlr
	li r4, 476
	blr

.global IgaPukuBypass
IgaPukuBypass:
	lwz r4, 4(r29)
	clrrwi r4, r4, 24
	ori r4, r4, 0x10
	blr

.global IgaPukuNoEffect1
IgaPukuNoEffect1:
	lwz r9, 4(r29)
	srwi r9, r9, 24
	cmpwi r9, 1
	beqlr
	bctr

.global IgaPukuChangeEffect2
IgaPukuChangeEffect2:
	lwz r9, 4(r29)
	srwi r9, r9, 24
	cmpwi r9, 1
	bne dontChangeEff
	lis r4, firepillarsign@h
	ori r4, r4, firepillarsign@l
dontChangeEff:
	bctr

.data
firepillarsign: .string "Wm_ob_firespillarunder"
.text

.global IgaPukuSoundChange
IgaPukuSoundChange:
	lwz r9, 4(r29)
	srwi r9, r9, 24
	cmpwi r9, 1
	li r4, 479
	bnelr
	li r4, 644
	blr

# The below was added by Tempus


.global TEX_Goomba
TEX_Goomba:
.global TEX_BuzzyShell
TEX_BuzzyShell:
.global TEX_SpinyShell
TEX_SpinyShell:
.global TEX_Coaster
TEX_Coaster:
	lwz r5, 4(r3)
	srwi r5, r5, 24
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_BigBoo
TEX_BigBoo:
	lwz r5, 4(r27)
	srwi r5, r5, 24
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_GabonRock
TEX_GabonRock:
.global TEX_BigBlock
TEX_BigBlock:
.global TEX_Manta
TEX_Manta:
.global TEX_BlockSlide
TEX_BlockSlide:
.global TEX_CloudBounce
TEX_CloudBounce:
	lwz r5, 4(r28)
	srwi r5, r5, 24
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_Thwomp
TEX_Thwomp:
.global TEX_GiantBuzzy
TEX_GiantBuzzy:
.global TEX_Buzzy
TEX_Buzzy:
.global TEX_SpikeTop
TEX_SpikeTop:
.global TEX_Koopa
TEX_Koopa:
.global TEX_Gabon
TEX_Gabon:
.global TEX_Bramball
TEX_Bramball:
.global TEX_Monty
TEX_Monty:
.global TEX_ChainChomp
TEX_ChainChomp:
.global TEX_Parabeetle
TEX_Parabeetle:
.global TEX_Bomb
TEX_Bomb:
.global TEX_ParaBomb
TEX_ParaBomb:
.global TEX_CheepChomp
TEX_CheepChomp:
.global TEX_PokeyB
TEX_PokeyB:
.global TEX_Huckit
TEX_Huckit:
.global TEX_IgaPuku
TEX_IgaPuku:
	lwz r5, 4(r29)
	srwi r5, r5, 24
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_BigGoomba
TEX_BigGoomba:
.global TEX_Barrel
TEX_Barrel:
.global TEX_Blooper
TEX_Blooper:
.global TEX_Spiny
TEX_Spiny:
.global TEX_SpringBlock
TEX_SpringBlock:
.global TEX_Boo
TEX_Boo:
.global TEX_Goal
TEX_Goal:
.global TEX_StretchMush
TEX_StretchMush:
.global TEX_Spinner
TEX_Spinner:
.global TEX_AirshipGear
TEX_AirshipGear:
.global TEX_BigShell
TEX_BigShell:
.global TEX_PokeyA
TEX_PokeyA:
.global TEX_SpikeBall
TEX_SpikeBall:
.global TEX_HuckitBalls
TEX_HuckitBalls:
.global TEX_LightBlock
TEX_LightBlock:
.global HammerColour
HammerColour:
	lwz r5, 4(r30)
	srwi r5, r5, 24
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_Podoboo
TEX_Podoboo:
	lwz r5, 4(r31)
	srwi r5, r5, 24
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


# All these vary
.global TEX_BigIcicle
TEX_BigIcicle:
	lwz r5, 4(r28)
	srwi r5, r5, 20
	andi. r5, r5, 0xF
	b GetTexFilenameForR5

.global TEX_MushroomPlatform
TEX_MushroomPlatform:
	lwz r5, 4(r30)
	srwi r5, r5, 20
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_RollingHills
TEX_RollingHills:
	lwz r5, 4(r30)

	# Get style
	extrwi. r9, r5, 4, 8
	beq JustUseZero

	extrwi r5, r5, 8, 16
	b GetTexFilenameForR5

JustUseZero:
	li r5, 0
	b GetTexFilenameForR5

.global TEX_Gabon_Spike
TEX_Gabon_Spike:
	andis. r4, r4, 0xFFF0
	blr

.global TEX_Switch
TEX_Switch:
	lwz r5, 4(r29)
	srwi r5, r5, 16
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_WiggleShroom
TEX_WiggleShroom:
	lwz r5, -0x520(r3)
	# This sprite is pretty complex
	# The register with THIS is r29
	# before the loading function,
	# it passes it into r3, and then mr's
	# it into r30. r3 is then overwritten.
	# Then, it puts r31 into r5, r30 partly
	# into r3 and partly into r4. Why?
	# So, the possibilities for THIS are
	# r30, r3, and r4, no? In the next function,
	# r30 is stored over before our getFile, as
	# are r29/r28.
	
	srwi r5, r5, 24
	andi. r5, r5, 0xF
	b GetTexFilenameForR5



.global TEX_MantaInherit
TEX_MantaInherit:
	lwz r5, 4(r28)
	andis. r5, r5, 0x0F00
	
	or r5, r5, r0
	oris r5, r5, 0x8000
	blr



.global TEX_CheepA
TEX_CheepA:
	lwz r5, 4(r27)
	srwi r5, r5, 20
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_CheepB
TEX_CheepB:
	lwz r5, 4(r28)
	srwi r5, r5, 20
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_CheepC
TEX_CheepC:
	lwz r5, 4(r28)
	srwi r5, r5, 20
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_Midway
TEX_Midway:
	lwz r5, 4(r29)
	srwi r5, r5, 24
	andi. r5, r5, 0xFF
	b GetTexFilenameForR5


.global TEX_ColorBlock
TEX_ColorBlock:

	cmpwi r0, 0xB0
	bnelr

	subi r9, r29, 0x4E0

	lwz r5, 4(r9)
	srwi r5, r5, 28
	andi. r5, r5, 0xF
	b GetTexFilenameForR5



.global TEX_Platforms
TEX_Platforms:

	li r5, 1
	b GetTexFilenameForR5
#	cmpwi r0, 9
#	blt ReplacePlatforms
#
#	lwzx r5, r5, r0
#	blr
	

ReplacePlatforms:

	lwz r5, 4(r29)
	srwi r5, r5, 4
	andi. r5, r5, 0xF
	b GetTexFilenameForR5


.global TEX_Platform_ShifterH
TEX_Platform_ShifterH:

	stw r5, 0x504(r30)
	lwz r5, 4(r30)
	stw r5, 0x3D8(r30)
	blr

.global TEX_Platform_ShifterF
TEX_Platform_ShifterF:

	stw r4, 0x504(r30)
	lwz r4, 4(r30)
	stw r4, 0x3D8(r30)
	blr

.global TEX_Platform_ShifterL
TEX_Platform_ShifterL:

	stw r5, 0x504(r28)
	lwz r5, 4(r28)
	stw r5, 0x3D8(r28)
	blr

.global TEX_Platform_ShifterR
TEX_Platform_ShifterR:

	stw r4, 0x504(r31)
	lwz r4, 4(r31)
	stw r4, 0x3D8(r31)
	blr

.global TEX_Platform_ShifterS
TEX_Platform_ShifterS:

	stw r30, 0x4C4(r27)
	lwz r30, 4(r27)
	stw r30, 0x398(r27)
	blr

.global TEX_Platform_ShifterK
TEX_Platform_ShifterK:

	stw r4, 0x5A4(r27)
	lwz r4, 4(r27)
	stw r4, 0x478(r27)
	blr

.global TEX_Platform_ShifterW
TEX_Platform_ShifterW:

	stw r0, 0x504(r29)
	lwz r0, 4(r29)
	stw r0, 0x3D8(r29)
	blr

.global TEX_Platform_ShifterSL
TEX_Platform_ShifterSL:

	stw r4, 0x504(r29)
	lwz r4, 4(r29)
	stw r4, 0x3D8(r29)
	blr



# The small sand pieces that fall from the snowman
.global TEX_SnowmanSandToSnow
TEX_SnowmanSandToSnow:

	lwz r10, 4(r29)
	srwi r10, r10, 24
	andi. r10, r10, 0xF
	
	cmpwi r10, 0x2
	bne SnowmanSandToSnow

	lis r4, snoweffectC@h
	ori r4, r4, snoweffectC@l
	blr

SnowmanSandToSnow:
	addi r4, r4, 0x792C
	blr


# The small sand puffs that occur when it moves
.global TEX_SnowmanSandToSnowB
TEX_SnowmanSandToSnowB:

	lwz r10, 4(r30)
	srwi r10, r10, 24
	andi. r10, r10, 0xF
	
	cmpwi r10, 0x2
	bne SnowmanSandToSnowB

	lis r4, snoweffectD@h
	ori r4, r4, snoweffectD@l
	blr

SnowmanSandToSnowB:
	addi r4, r4, 0x7944
	blr



SnowmanPokeyRotate:
	li r5, 0x100
	blr

.global TEX_PokeySnowmanRotate
TEX_PokeySnowmanRotate:

	lwz r10, 4(r30)
	srwi r10, r10, 24
	andi. r10, r10, 0xF
	
	cmpwi r10, 0x2
	bne SnowmanPokeyRotate

	li r5, 0
	blr


PokeySnowmanImmuneToIce:
	lwz r31, 4(r5)
	blr

.global TEX_PokeySnowmanImmuneToIce
TEX_PokeySnowmanImmuneToIce:
	lwz r10, 4(r3)
	srwi r10, r10, 24
	andi. r10, r10, 0xF
	
	cmpwi r10, 0x2
	bne PokeySnowmanImmuneToIce

	li r3, 0
	mtlr r0
	blr

         


.global TEX_PokeySnowman
TEX_PokeySnowman:

	lwz r10, 4(r30)
	srwi r10, r10, 24
	andi. r10, r10, 0xF
	
	cmpwi r10, 0x2
	bne SnowmanPokey

	lwz r10, 4(r30)
	# r10 is the current height
	andi. r10, r10, 0xF
	
	# Find it
	lis r11, SnowmanTable@h
	ori r11, r11, SnowmanTable@l
	slwi r10, r10, 2	# size *= 4 (size of float)
	lfsx f0, r11, r10	# get it from the array
	
	# We should probably change ActivePhysics and maybe some other rects here
	addi r10, r30, 0x148

	lfs f10, 0(r10)
	fmuls f10, f0, f10
	stfs f10, 0(r10)

	lfs f10, 4(r10)
	fmuls f10, f0, f10
	stfs f10, 4(r10)

	lfs f10, 8(r10)
	fmuls f10, f0, f10
	stfs f10, 8(r10)

	lfs f10, 0xC(r10)
	fmuls f10, f0, f10
	stfs f10, 0xC(r10)

	blr
	

SnowmanPokey:

	lfs f0, 0x54(r31)
	blr




.global TEX_PokeySnowmanPos
TEX_PokeySnowmanPos:

	mr r3, r30
	
	lwz r10, 4(r30)
	andi. r10, r10, 0xF
	
	lis r11, SnowmanTable@h
	ori r11, r11, SnowmanTable@l
	slwi r10, r10, 2	# size *= 4 (size of float)
	lfsx f0, r11, r10	# get it from the array	
	
	lis r5, AddAmount@h
	ori r5, r5, AddAmount@l
	slwi r6, r6, 2	# size *= 4 (size of float)
	lfsx f3, r5, r6	# get it from the array

	lfs f2, 0xB0(r30)
	fmuls f3, f3, f0
	fmuls f3, f3, f0
	fmuls f3, f3, f0
	fsubs f2, f2, f3
	stfs f2, 0xB0(r30)

	blr
	
SnowmanPokeyPos:

	fsubs f0, f0, f31
	fsubs f0, f0, f31
	fsubs f1, f1, f31
	fsubs f1, f1, f31


	lfs f2, 8(r1)
	fadds f2, f2, f31
	stfs f2, 8(r1)
	blr


.global TEX_PokeyC
TEX_PokeyC:

	lwz r5, 4(r30)
	andis. r5, r5, 0xFFF0
	or r5, r5, r31
	or r5, r5, r0

	lwz r0, 0x524(r30)
	slwi r0, r0, 8
	or r5, r5, r0

	blr


.global TEX_PokeyNoSand
TEX_PokeyNoSand:
	cmpwi r3, 0x20
	blr


.global TEX_HuckitBallsInherit
TEX_HuckitBallsInherit:
	lwz r10, 4(r9)
	andis. r10, r10, 0xF00
	or r4, r4, r0
	or r4, r4, r10
	blr

.global BomheiForceFacingDirection
BomheiForceFacingDirection:
	stb r3, 0x348(r30)

	lwz r6, 4(r30)
	clrlwi r6, r6, 28
	cmpwi r6, 0
	beqlr
	subi r6, r6, 1
	stb r6, 0x348(r30)
	blr

.global BomheiCollisionCallback
.extern collisionCallback__5dEn_cFP13ActivePhysicsP13ActivePhysics
BomheiCollisionCallback:
	lwz r5, 4(r4)
	lhz r5, 8(r5)
	cmpwi r5, 0x70 #hammerbros
	beqlr
	cmpwi r5, 0x72 #lift hammerbros
	beqlr
	b collisionCallback__5dEn_cFP13ActivePhysicsP13ActivePhysics

.global JustOneBomb
.extern ContinueFromJustOneBomb
JustOneBomb:
	lwz r5, 4(r3)
	extrwi r5, r5, 4, 20
	cmpwi r5, 0
	beq dontDoThis
	li r0, 1
dontDoThis:
	stw r0, 0x6F0(r3)
	b ContinueFromJustOneBomb

.global HammerTime
HammerTime:
	lwz r8, 4(r31)

	srwi r8, r8, 8
	andi. r8, r8, 0xF

	cmpwi r8, 1
	beq ThrowBobomb

	cmpwi r8, 2
	beq ThrowGoomba

	cmpwi r8, 3
	beq ThrowSpiny

	cmpwi r8, 4
	beq ThrowCoin

	cmpwi r8, 5
	beq ThrowLemmyBall

	# Use the normal settings
	lhz r8, 8(r31)
	cmpwi r8, 0x70
	beq isHammerBros
	cmpwi r8, 0x72
	beq isLiftHammerBros
	blr
isHammerBros:
	oris r4, r0, 0xF
	blr
isLiftHammerBros:
	ori r4, r0, 0x10
	blr


ThrowBobomb:
	li r3, 133
	lbz r4, 0x348(r31)
	addi r4, r4, 1
	blr

ThrowGoomba:
	li r3, 0x33
	li r4, 1
	blr

ThrowSpiny:
	li r3, 0x39
	li r4, 1
	blr

ThrowCoin:
	li r3, 10
	mflr r8
	mr r9, r5
	bl GenerateRandomNumber
	mr r5, r9
	
	mtlr r8
	cmpwi r3, 1
	beq ThrowItem
	
	li r3, 0x193
	li r4, 0
	li r6, 0
	li r7, 0
	blr

ThrowItem:
	li r3, 6
	mflr r8
	mr r9, r5
	bl GenerateRandomNumber
	mtlr r8
	mr r5, r9

	li r4, 0

	mr r4, r3
	li r3, 0x3C
	li r6, 0
	li r7, 0
	blr
	
	
ThrowLemmyBall:
	li r3, 0x232
	li r4, 0
	blr




.global HammerLaunch
HammerLaunch:
	mr r3, r30
	mr r10, r30
	# r31 is the new actor currently
	# r30 is self, all others are volatile
	
	lwz r5, 4(r30)
	lbz r8, 0x348(r31)
	
	srwi r5, r5, 8
	andi. r5, r5, 0xF

	cmpwi r5, 0
	beqlr

	lis r7, LaunchAngle@h
	ori r7, r7, LaunchAngle@l

#	lfs f0, 0(r7)
#	stfs f0, 0xDC(r31)
#	stfs f0, 0xE0(r31)
#	stfs f0, 0xE4(r31)
	
	# Nybble 11 is for y right now
	li r6, 0
	slwi r6, r6, 2	# size *= 4 (size of float)
	lfsx f0, r7, r6	# get it from the array	
	
	stfs f0, 0xEC(r31)
	
	cmpwi r5, 5
	beq LemmyLaunch

	cmpwi r5, 1
	beq BombLaunch

	li r6, 1
	slwi r6, r6, 2	# size *= 4 (size of float)

	cmpwi r8, 0
	bne FacingLeft	

	# Nybble 12 is for x right now
	lfsx f0, r7, r6	# get it from the array	
	
	stfs f0, 0xE8(r31)
	
	blr


FacingLeft:

	lis r7, LaunchAngleLeft@h
	ori r7, r7, LaunchAngleLeft@l
	
	# Nybble 12 is for x right now
	lfsx f0, r7, r6	# get it from the array	
	
	stfs f0, 0xE8(r31)
	blr


LemmyLaunch:	
	
	li r6, 2
	slwi r6, r6, 2	# size *= 4 (size of float)

	cmpwi r8, 0
	bne FacingLeft	

	# Nybble 12 is for x right now
	lfsx f0, r7, r6	# get it from the array	
	
	stfs f0, 0xE8(r31)
	blr	


BombLaunch:

	stwu sp, -0x40(sp)
	mflr r0
	stw r31, 0x2C(sp)
	stw r30, 0x28(sp)
	stw r0, 0x24(sp)
	stw r3, 0x20(sp)
	stw r4, 0x1C(sp)
	stw r5, 0x18(sp)
	stw r6, 0x10(sp)
	stw r7, 0x8(sp)
	stw r8, 0x34(sp)
	stw r9, 0x30(sp)

	li r0, 180
	stw r0, 0x5EC(r31) # Burning time of the bomb

	li r0, 0
	stw r0, 0x38(sp)
	lfs f1, 0x38(sp)
	addi r3, r31, 0x5C0
	li r4, 0
.extern setFrameForEntry__Q23m3d8anmClr_cFfi
	bl setFrameForEntry__Q23m3d8anmClr_cFfi

	mr r3, r31

	lwz r12, 0x60(r31)
	lwz r12, 0xD4(r12)
	mtctr r12
	lis r4, BombSleepState@h
	ori r4, r4, BombSleepState@l

#What in the FUCK are you doing, Ancient Code
#	lwz r5, 4(r30)
#	srwi r5, r5, 28
#
#	mulli r5, r5, 0x40
#
#	add r4, r4, r5
	bctrl


	lwz r31, 0x2C(sp)
	lwz r30, 0x28(sp)
	lwz r0, 0x24(sp)
	lwz r3, 0x20(sp)
	lwz r4, 0x1C(sp)
	lwz r5, 0x18(sp)
	lwz r6, 0x10(sp)
	lwz r7, 0x8(sp)
	lwz r8, 0x34(sp)
	lwz r9, 0x30(sp)
	mr r3, r30

	lis r7, LaunchAngle@h
	ori r7, r7, LaunchAngle@l

	li r6, 0
	slwi r6, r6, 2	# size *= 4 (size of float)

	lbz r8, 0x348(r31)
	cmpwi r8, 0
	bne BombFacingLeft	

	lfsx f0, r7, r6
	stfs f0, 0xE8(r31)

	mtlr r0
	addi sp, sp, 0x40
	blr

BombFacingLeft:

	lis r7, LaunchAngleLeft@h
	ori r7, r7, LaunchAngleLeft@l
	
	lfsx f0, r7, r6
	stfs f0, 0xE8(r31)
	
	mtlr r0
	addi sp, sp, 0x40
	blr


.global HammerDontTrashEn450
HammerDontTrashEn450:
	lhz r5, 8(r3)
	cmpwi r5, 0x73 #hammer
	bnelr
	#beq FUCK_THE_SHITTY_HAMMER
	stw r4, 0x450(r3)
	blr
FUCK_THE_SHITTY_HAMMER:
	lbz r4, 0x348(r31)
	stb r4, 0x348(r3)
	blr

.global HammerDontTrashEn420
HammerDontTrashEn420:
	lhz r5, 8(r3)
	cmpwi r5, 0x73 #hammer
	bnelr
	stw r0, 0x420(r3)
	blr

.global HammerIgnoreEn420
HammerIgnoreEn420:
	lwz r0, 0x420(r3)
	lhz r5, 8(r3)
	cmpwi r5, 0x73 #hammer
	beqlr
	li r0, 0
	blr

.global HammerSpin
HammerSpin:
	lwz r5, 4(r10)
	extrwi r5, r5, 4,20
	cmpwi r5, 1
	beqlr # Don't set this rotation for bombs
	sth r3, 0x100(r31)
	blr


.global HammerLeft
HammerLeft:

	sth r29, 0x104(r31)

	lbz r8, 0x348(r10)
	lha r7, 0x102(r31)

	cmpwi r8, 0
	bne GoingLeft
	blr


GoingLeft:

	li r8, 1
	stb r8, 0x348(r31)

	li r6, -0x300
	add r7, r6, r7
	sth r7, 0x102(r31)
	sth r7, 0x108(r31)
	sth r7, 0xC(r1)
	blr




.global TEX_BlackFlakes
TEX_BlackFlakes:

	mflr r9

	lwz r6, 4(r23)
	srwi r6, r6, 24
	andi. r6, r6, 0xF

	cmpwi r6, 0
	beql FlakeySnow

	cmpwi r6, 1
	beql FlakeyLeavesRed

	cmpwi r6, 2
	beql FlakeyLeavesScarlet

	cmpwi r6, 3
	beql FlakeyLeavesBrown

	cmpwi r6, 4
	beql FlakeyLeavesGold

	cmpwi r6, 5
	beql FlakeyLeavesChestnut

	cmpwi r6, 6
	beql FlakeySakura

	cmpwi r6, 7
	beql FlakeyStardustWhite

	cmpwi r6, 8
	beql FlakeyStardustGold

	cmpwi r6, 9
	beql FlakeyStardustPurple

	cmpwi r6, 10
	beql FlakeyStardustMauve

	stb r5, 0x5B4(r23)
	stb r5, 0x5B8(r23)

	stb r4, 0x5B5(r23)
	stb r4, 0x5B9(r23)

	stb r3, 0x5B6(r23)
	stb r3, 0x5BA(r23)

	stb r0, 0x5B7(r23)
	stb r0, 0x5BB(r23)

	mtlr r9
	blr

FlakeySnow:
	li r5, 0xFF
	li r4, 0xFF
	li r3, 0xFF
	li r0, 0xA0
	blr

FlakeyLeavesRed:
	li r5, 0xCC
	li r4, 0x0
	li r3, 0x0
	li r0, 0xFF
	blr

FlakeyLeavesScarlet:
	li r5, 0x66
	li r4, 0x0
	li r3, 0x0
	li r0, 0xFF
	blr

FlakeyLeavesBrown:
	li r5, 0x99
	li r4, 0x66
	li r3, 0x0
	li r0, 0xFF
	blr

FlakeyLeavesGold:
	li r5, 0xFF
	li r4, 0x80
	li r3, 0x0
	li r0, 0xFF
	blr

FlakeyLeavesChestnut:
	li r5, 0x66
	li r4, 0x33
	li r3, 0x0
	li r0, 0xFF
	blr

FlakeySakura:
	li r5, 0xFF
	li r4, 0x99
	li r3, 0xCC
	li r0, 0xFF
	blr
	
FlakeyStardustWhite:
	li r5, 0xFF
	li r4, 0xFF
	li r3, 0xD0
	bl GenerateRandomNumber
	mr r0, r3
	li r3, 0xFF
	blr
	
FlakeyStardustGold:
	li r5, 0xFF
	li r4, 0x80
	li r3, 0xD0
	bl GenerateRandomNumber
	mr r0, r3
	li r3, 0x0
	blr
	
FlakeyStardustPurple:
	li r5, 0x66
	li r4, 0x00
	li r3, 0xD0
	bl GenerateRandomNumber
	mr r0, r3
	li r3, 0x99
	blr
	
FlakeyStardustMauve:
	li r5, 0xCC
	li r4, 0x99
	li r3, 0xD0
	bl GenerateRandomNumber
	mr r0, r3
	li r3, 0xCC
	blr
	
	
	
.global TEX_Flakes
TEX_Flakes:
	stwu sp, -0x20(sp)
	mflr r0
	stw r0, 0x24(sp)
	stw r3, 0x1C(sp)
	stw r5, 0x18(sp)
	stw r6, 0x14(sp)
	stw r7, 0x10(sp)
	stw r8, 0xC(sp)
	stw r9, 0x8(sp)

	lwz r5, 4(r30)
	srwi r5, r5, 24
	andi. r5, r5, 0xF

	cmpwi r5, 5
	blel LeafString

	cmpwi r5, 0
	beql SnowString

	cmpwi r5, 6
	beql PetalString

	cmpwi r5, 7
	bgel StardustString

	lwz r0, 0x24(sp)
	lwz r3, 0x1C(sp)
	lwz r5, 0x18(sp)
	lwz r6, 0x14(sp)
	lwz r7, 0x10(sp)
	lwz r8, 0xC(sp)
	lwz r9, 0x8(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr


SnowString:
	lis r4, stringsnow@h
	ori r4, r4, stringsnow@l
	blr

LeafString:
	lis r4, stringleaf@h
	ori r4, r4, stringleaf@l
	blr

PetalString:
	lis r4, stringpetal@h
	ori r4, r4, stringpetal@l
	blr

StardustString:
	lis r4, stringstardust@h
	ori r4, r4, stringstardust@l
	blr


.global TEX_CrabBallBurst
TEX_CrabBallBurst:
	lwz r3, 4(r29)
	srwi r3, r3, 24
	andi. r3, r3, 0xF
	
	cmpwi r3, 0
	beq huckitRockBallEffectsA

	lis r3, snoweffectA@h
	ori r3, r3, snoweffectA@l
	blr


huckitRockBallEffectsA:
	lis r3, rockeffectA@h
	ori r3, r3, rockeffectA@l
	blr


.global TEX_CrabBallSmoke
TEX_CrabBallSmoke:
	lwz r3, 4(r29)
	srwi r3, r3, 24
	andi. r3, r3, 0xF
	
	cmpwi r3, 0
	beq huckitRockBallEffectsB

	lis r3, snoweffectB@h
	ori r3, r3, snoweffectB@l
	blr

huckitRockBallEffectsB:
	lis r3, rockeffectB@h
	ori r3, r3, rockeffectB@l
	blr



.global ChangeWindyColour
ChangeWindyColour:
	lwz r3, 4(r23)
	andi. r3, r3, 0xF0
	stb r0, 0x5BB(r23)
	beqlr
	lis r3, 0xFFFF
	ori r3, r3, 0xFF64
	stw r3, 0x5B4(r23)
	lis r3, 0x87E6
	ori r3, r3, 0xFFFF
	stw r3, 0x5B8(r23)
	blr


.global GLOW_LightBlock
GLOW_LightBlock:
	lwz r4, 4(r3)

	andi. r4, r4, 0x1
	cmpwi r4, 1

	beqlr

	lwzu r12, 0x404(r3)
	lwz r12, 0x10(r12)
	mtctr r12
	bctr


.global CloudLT_Execute
CloudLT_Execute:
	stwu r1, -0x10(r1)
	mflr r0
	stw r0, 0x14(r1)
	stw r31, 0xC(r1)

	mr r31, r3
	bl Cloud_execute

	lwz r8, 4(r31)
	srwi r8, r8, 28
	cmpwi r8, 1
	bne dontChangeMyColour

	li r9, 0xFF
	li r8, 0
	mr r4, r31
	li r5, 75
	mtctr r5
colourLoop:
	stb r9, 0x934(r4)
	stb r8, 0x935(r4)
	stb r8, 0x936(r4)

	stb r9, 0xA60(r4)
	stb r8, 0xA61(r4)
	stb r8, 0xA62(r4)

	addi r4, r4, 4
	bdnz colourLoop

dontChangeMyColour:
	lwz r31, 0xC(r1)
	lwz r0, 0x14(r1)
	mtlr r0
	addi r1, r1, 0x10
	blr


.global TEX_CloudLT_A
TEX_CloudLT_A:
	lwz r8, 4(r31)

	srwi r8, r8, 28
	cmpwi r8, 1

	beq TEX_CloudLT_Aa

	addi r4, r28, 0x2C
	blr

TEX_CloudLT_Aa:
	lis r4, CloudA@h
	ori r4, r4, CloudA@l
	blr


.global TEX_CloudLT_B
TEX_CloudLT_B:
	lwz r8, 4(r31)

	srwi r8, r8, 28
	cmpwi r8, 1

	beq TEX_CloudLT_Ba

	addi r4, r28, 0x38
	blr

TEX_CloudLT_Ba:
	lis r4, CloudB@h
	ori r4, r4, CloudB@l
	blr


.global TEX_CloudLT_C
TEX_CloudLT_C:
	lwz r8, 4(r31)

	srwi r8, r8, 28
	cmpwi r8, 1

	beq TEX_CloudLT_Ca

	addi r4, r28, 0x44
	blr

TEX_CloudLT_Ca:
	lis r4, CloudC@h
	ori r4, r4, CloudC@l
	blr



.global BigMetPowFix
.extern OriginalPowBlockActivated
BigMetPowFix:
	lwz r5, 4(r3)
	extrwi. r5, r5, 4,4
	beqlr
	b OriginalPowBlockActivated

.global TogeMetPowFix
TogeMetPowFix:
	lwz r6, 4(r3)
	extrwi r6, r6, 4,4
	cmpwi r6, 5
	beqlr
	b OriginalPowBlockActivated

.extern collisionCat13_Hammer__5dEn_cFP13ActivePhysicsP13ActivePhysics
.global TogeMetHammerFix
TogeMetHammerFix:
	lwz r6, 4(r3)
	extrwi r6, r6, 4,4
	cmpwi r6, 5
	beq TogeMetHammerSpecial
	b collisionCat13_Hammer__5dEn_cFP13ActivePhysicsP13ActivePhysics
TogeMetHammerSpecial:
	li r3, 1
	blr

.extern collisionCat2_IceBall_15_YoshiIce__5dEn_cFP13ActivePhysicsP13ActivePhysics
.extern TogeTekkyuIceballCollision
.global TogeMetIceFix
TogeMetIceFix:
	lwz r6, 4(r3)
	extrwi r6, r6, 4,4
	cmpwi r6, 5
	beq TogeMetIceSpecial
	b collisionCat2_IceBall_15_YoshiIce__5dEn_cFP13ActivePhysicsP13ActivePhysics
TogeMetIceSpecial:
	li r3, 1
	blr

.data
BomombDir:
	.word 0x0400,0xFC00

TexFilenameBuffer:
	.long 0,0,0,0

TexFormatString:
	.string "g3d/t%02d.brres"


.global TEX_FlakeArcs
TEX_FlakeArcs:
	.long stringsnow
	.long stringleaf
	.long stringpetal
	.long stringstardust
	.long 0

stringsnow:
	.string "env_snow"
.align 4

stringleaf:
	.string "env_leaf"
.align 4

stringpetal:
	.string "env_petal"
.align 4

stringstardust:
	.string "env_stardust"
.align 4


snoweffectA:
	.string "Wm_en_blockcloud"
.align 4

snoweffectB:
	.string "Wm_ob_iceattack"
.align 4

rockeffectA:
	.string "Wm_ob_cmnboxsmoke"
.align 4

rockeffectB:
	.string "Wm_ob_cmnrockpiece"
.align 4

snoweffectD:
	.string "Wm_ob_icebreaksmk"
.align 4

snoweffectC:
	.string "Wm_ob_icewait"
.align 4



ConvertFloat:
	.long  0x43300000,0x0,0x43300000,0x80000000

.align 4


SnowmanTable:
	.float 1.0, 1.333, 1.666, 2.0, 2.333, 2.666, 3.0, 3.333, 3.666, 4.0, 4.333, 4.666
	
AddAmount:
	.float 32.0

LaunchAngle:
	.float 2.4, 3.0, 1.5

LaunchAngleLeft:
	.float -2.4, -3.0, -1.5

.align 4



YoshiArc:
	.string "Y_TexGreen"
.align 4

Y_TexGreen:
	.string "Y_TexGreen"
.align 4
Y_TexRed:
	.string "Y_TexRed"
.align 4
Y_TexYellow:
	.string "Y_TexYellow"
.align 4
Y_TexBlue:
	.string "Y_TexBlue"
.align 4
Y_TexPurple:
	.string "Y_TexPurple"
.align 4
Y_TexOrange:
	.string "Y_TexOrange"
.align 4
Y_TexSkyBlue:
	.string "Y_TexSkyBlue"
.align 4
Y_TexBlack:
	.string "Y_TexBlack"
.align 4
Y_TexWhite:
	.string "Y_TexWhite"
.align 4

YoshiStrings:
	.long Y_TexGreen, Y_TexRed, Y_TexYellow, Y_TexBlue, Y_TexPurple, Y_TexOrange, Y_TexSkyBlue, Y_TexBlack, Y_TexWhite

.align 4

CloudA:
	.string "wny_mist.0"
.align 4
CloudB:
	.string "wny_mist.1"
.align 4
CloudC:
	.string "wny_mist.2"
.align 4

Float_5000: .float 5000.0

