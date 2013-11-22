.text
#include <asm_setup.S>
.extern OSReport


.align 4



LoadFromSettingsS: 
	# we can trash r12 for now, it's not supposed to be guaranteed anyway
	lwz r12, 4(r3)		# get the sprite settings
	srwi r12, r12, 20	# get the highest nybble as r10 (maps to Nyb7 in Reggie)
	andi. r12, r12, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r12, r12, 2	# size *= 4 (size of float)
	lfsx f8, r11, r12	# get it from the array
	blr


LoadFromSettingsSF: 
	# we can trash r12 for now, it's not supposed to be guaranteed anyway
	lwz r12, 4(r3)		# get the sprite settings
	srwi r12, r12, 28	# get the highest nybble as r12 (maps to Nyb5 in Reggie)
	andi. r12, r12, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r12, r12, 2	# size *= 4 (size of float)
	lfsx f8, r11, r12	# get it from the array
	blr


LoadFromSettingsC: 
	# we can trash r12 for now, it's not supposed to be guaranteed anyway
	lwz r12, 4(r4)		# get the sprite settings
	srwi r12, r12, 20	# get the highest nybble as r12 (maps to Nyb7 in Reggie)
	andi. r12, r12, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r12, r12, 2	# size *= 4 (size of float)
	lfsx f8, r11, r12	# get it from the array
	blr


LoadFromSettingsCF: 
	# we can trash r12 for now, it's not supposed to be guaranteed anyway
	lwz r12, 4(r4)		# get the sprite settings
	srwi r12, r12, 28	# get the highest nybble as r12 (maps to Nyb5 in Reggie)
	andi. r12, r12, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r12, r12, 2	# size *= 4 (size of float)
	lfsx f8, r11, r12	# get it from the array
	blr


LoadFromSettingsH: 
	# we can trash r12 for now, it's not supposed to be guaranteed anyway
	lwz r12, 4(r30)		# get the sprite settings
	srwi r12, r12, 20	# get the highest nybble as r12 (maps to Nyb7 in Reggie)
	andi. r12, r12, 0xF

	# first off, get the value from the lookup table
	lis r11, RiderLookupTable@h
	ori r11, r11, RiderLookupTable@l
	slwi r12, r12, 2	# size *= 4 (size of float)
	lfsx f8, r11, r12	# get it from the array
	blr


LoadFromSettingsHF: 
	# we can trash r12 for now, it's not supposed to be guaranteed anyway
	lwz r12, 4(r30)		# get the sprite settings
	srwi r12, r12, 28	# get the highest nybble as r12 (maps to Nyb5 in Reggie)
	andi. r12, r12, 0xF

	# first off, get the value from the lookup table
	lis r11, RiderLookupTable@h
	ori r11, r11, RiderLookupTable@l
	slwi r12, r12, 2	# size *= 4 (size of float)
	lfsx f8, r11, r12	# get it from the array
	blr


LoadFromSettingsL: 
	# we can trash r12 for now, it's not supposed to be guaranteed anyway
	lwz r12, 8(r30)		# get the sprite settings
	srwi r12, r12, 20	# get the highest nybble as r12 (maps to Nyb7 in Reggie)
	andi. r12, r12, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r12, r12, 2	# size *= 4 (size of float)
	lfsx f8, r11, r12	# get it from the array
	blr


LoadFromSettingsLF: 
	# we can trash r12 for now, it's not supposed to be guaranteed anyway
	lwz r12, 8(r30)		# get the sprite settings
	srwi r12, r12, 28	# get the highest nybble as r12 (maps to Nyb5 in Reggie)
	andi. r12, r12, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r12, r12, 2	# size *= 4 (size of float)
	lfsx f8, r11, r12	# get it from the array
	blr



ModelNoChanges:
	lwz r4, 0x47C(r3)
	lfs f0, 0xDC(r4)
	stfs f0, 0x464(r3)	
	lfs f0, 0xE0(r4)
	stfs f0, 0x468(r3)
	lfs f0, 0xE4(r4)
	stfs f0, 0x46C(r3)
	blr


.global ModifyGlobalSizes
ModifyGlobalSizes:

	# Go back if it's 60
	lhz r10, 8(r3)
	cmpwi r10, 60
	beq ModelNoChanges


	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq ModelNoChanges

	# Else, use Global
	lis r11, GlobalSpriteSize@h
	ori r11, r11, GlobalSpriteSize@l
	lfs f8, 0(r11)

	mflr r9

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsSF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsS

	mtlr r9

	# Modify the actor
	lwz r4, 0x47C(r3)
	
	lfs f0, 0xDC(r4)
	fmuls f0, f0, f8
	stfs f0, 0xDC(r3)	
	stfs f0, 0x464(r3)	

	lfs f0, 0xE0(r4)
	fmuls f0, f0, f8
	stfs f0, 0xE0(r3)
	stfs f0, 0x468(r3)

	lfs f0, 0xE4(r4)
	fmuls f0, f0, f8
	stfs f0, 0xE4(r3)
	stfs f0, 0x46C(r3)

	blr


# Another one to try at 80095750
SpeedUpdatesNoChangesB:
	lfs     f3, 0xC(r1)
	lfs     f1, 0x440(r30)
	lfs     f2, 0x10(r1)
	fadds   f6, f3, f1
	lfs     f0, 0x444(r30)
	lfs     f4, 0x14(r1)
	fadds   f5, f2, f0
	lfs     f3, 0x448(r30)
	lfs     f1, 0xAC(r30) #Actor.x_pos
	fadds   f3, f4, f3
	lfs     f0, 0xB0(r30) #Actor.y_pos
	fadds   f2, f1, f6
	fadds   f1, f0, f5
	lfs     f0, 0xB4(r30) #Actor.z_pos
	stfs    f2, 0xAC(r30) #Actor.x_pos
	fadds   f0, f0, f3
	stfs    f1, 0xB0(r30) #Actor.y_pos
	stfs    f0, 0xB4(r30) #Actor.z_pos
	psq_l   f31, 0x38(r1), 1, 0
	lfd     f31, 0x30(r1)
	psq_l   f30, 0x28(r1), 1, 0
	lfd     f30, 0x20(r1)
	lwz     r31, 0x1C(r1)
	lwz     r30, 0x18(r1)
	lwz     r0, 0x44(r1)
	stfs    f6, 0xC(r1)
	stfs    f5, 0x10(r1)
	stfs    f3, 0x14(r1)
	mtlr    r0
	addi    r1, r1, 0x40
	blr
	

.global ModifySpeedUpdatesB
ModifySpeedUpdatesB:

	# Go back if Type is a Player, Yoshi, or World Map Shit
	lhz r10, 8(r30)
	cmpwi r10, 0x12
	blt SpeedUpdatesNoChangesB

	# Go back if it's 60
	cmpwi r10, 60
	beq SpeedUpdatesNoChangesB

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq SpeedUpdatesNoChangesB

	# Else, use Global
	lis r11, GlobalSpriteSpeed@h
	ori r11, r11, GlobalSpriteSpeed@l
	lfs f8, 0(r11)

	mflr r9

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsHF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsH

	mtlr r9


	lfs     f3, 0xC(r1)
	lfs     f1, 0x440(r30)
	lfs     f2, 0x10(r1)
	fadds   f6, f3, f1
	lfs     f0, 0x444(r30)
	lfs     f4, 0x14(r1)
	fadds   f5, f2, f0
	lfs     f3, 0x448(r30)
	lfs     f1, 0xAC(r30) #Actor.x_pos
	fadds   f3, f4, f3
	lfs     f0, 0xB0(r30) #Actor.y_pos
	fmuls   f6, f8, f6
	fadds   f2, f1, f6
	fmuls   f5, f8, f5
	fadds   f1, f0, f5
	lfs     f0, 0xB4(r30) #Actor.z_pos
	stfs    f2, 0xAC(r30) #Actor.x_pos
	fmuls   f3, f8, f3
	fadds   f0, f0, f3
	stfs    f1, 0xB0(r30) #Actor.y_pos
	stfs    f0, 0xB4(r30) #Actor.z_pos
	psq_l   f31, 0x38(r1), 1, 0
	lfd     f31, 0x30(r1)
	psq_l   f30, 0x28(r1), 1, 0
	lfd     f30, 0x20(r1)
	lwz     r31, 0x1C(r1)
	lwz     r30, 0x18(r1)
	lwz     r0, 0x44(r1)
	stfs    f6, 0xC(r1)
	stfs    f5, 0x10(r1)
	stfs    f3, 0x14(r1)
	mtlr    r0
	addi    r1, r1, 0x40
	blr



# dStageActor_positionRelated: 80065620
SpeedUpdatesNoChangesA:
	lfs f3, 0xB0(r4)
	lfs f2, 0xD4(r4)
	lfs f1, 0xAC(r4)
	lfs f0, 0xD0(r4)
	fadds f2, f3, f2
	fadds f0, f1, f0
	stfs f2, 4(r3)
	stfs f0, 0(r3)
	blr
	

.global ModifySpeedUpdatesA
ModifySpeedUpdatesA:

	# Go back if Type is a Player, Yoshi, or World Map Shit
	lhz r10, 8(r4)
	cmpwi r10, 0x12
	blt SpeedUpdatesNoChangesA

	# Go back if it's 60
	cmpwi r10, 60
	beq SpeedUpdatesNoChangesA

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq SpeedUpdatesNoChangesA

	# Else, use Global
	lis r11, GlobalSpriteSize@h
	ori r11, r11, GlobalSpriteSize@l
	lfs f8, 0(r11)

	mflr r9

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsCF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsC

	mtlr r9

	lfs f3, 0xB0(r4)
	lfs f2, 0xD4(r4)
	lfs f1, 0xAC(r4)
	lfs f0, 0xD0(r4)
	fmuls f2, f8, f2
	fadds f2, f3, f2
	fmuls f0, f8, f0
	fadds f0, f1, f0
	stfs f2, 4(r3)
	stfs f0, 0(r3)
	blr


#SpeedUpdatesNoChanges:
#	# Xpos
#	lfs f1, 0xAC(r3)
#	lfs f0, 0(r4)
#	fadds f0, f1, f0
#	stfs f0, 0xAC(r3)
#	# Ypos
#	lfs f1, 0xB0(r3)
#	lfs f0, 4(r4)
#	fadds f0, f1, f0
#	stfs f0, 0xB0(r3)
#	# Zpos
#	lfs f1, 0xB4(r3)
#	lfs f0, 8(r4)
#	fadds f0, f1, f0
#	stfs f0, 0xB4(r3)
#	blr
#	
#
#.global ModifySpeedUpdates
#ModifySpeedUpdates:
#
#	# Go back if Type is a Player, Yoshi, or World Map Shit
#	lhz r10, 8(r3)
#	cmpwi r10, 0x12
#	blt SpeedUpdatesNoChanges
#
#	# Go back if SizerOn is 0 (off)
#	lis r10, SizerOn@h
#	ori r10, r10, SizerOn@l
#	lbz r10, 0(r10)
#	cmpwi r10, 0
#	beq SpeedUpdatesNoChanges
#
#	# Else, use Global
#	lis r11, GlobalSpriteSize@h
#	ori r11, r11, GlobalSpriteSize@l
#	lfs f8, 0(r11)
#
#	mflr r9
#
#	# Load from Nyb5 Settings if SizerOn is 1
#	cmpwi r10, 1
#	beql LoadFromSettingsSF
#
#	# Load from Nyb7 Settings if SizerOn is 2
#	cmpwi r10, 2
#	beql LoadFromSettingsS
#
#	mtlr r9
#
#	# Xpos
#	lfs f1, 0xAC(r3)
#	lfs f0, 0(r4)
#	fmuls f0, f8, f0
#	fadds f0, f1, f0
#	stfs f0, 0xAC(r3)
#	# Ypos
#	lfs f1, 0xB0(r3)
#	lfs f0, 4(r4)
#	fmuls f0, f8, f0
#	fadds f0, f1, f0
#	stfs f0, 0xB0(r3)
#	# Zpos
#	lfs f1, 0xB4(r3)
#	lfs f0, 8(r4)
#	fmuls f0, f8, f0
#	fadds f0, f1, f0
#	stfs f0, 0xB4(r3)
#	blr


NotTooBigToBurn:
    lwz       r4, 4(r5)
    blr

.global TooBigToBurn
TooBigToBurn:

	# Go back if Type is a Player, Yoshi, or World Map Shit
	lhz r10, 8(r31)
	cmpwi r10, 0x12
	blt NotTooBigToBurn

	# Go back if it's 60
	cmpwi r10, 60
	beq NotTooBigToBurn

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq NotTooBigToBurn

	cmpwi r10, 3
	beq B_GlobalCompare

	cmpwi r10, 1
	bne B_UseNyb7
	# use nyb 5
	lwz r6, 4(r31)
	srwi r6, r6, 28
	b B_GotNyb
B_UseNyb7:
	# use nyb 7
	lwz r6, 4(r31)
	extrwi r6, r6, 4, 8
B_GotNyb:
	cmpwi r6, 7
	bge B_Dont
	b NotTooBigToBurn

B_GlobalCompare:
	lis r6, GlobalSpriteSize@h
	ori r6, r6, GlobalSpriteSize@l
	lwz r6, 0(r6)
	lis r7, 0x3F80
	cmpw r6, r7
	beq NotTooBigToBurn

B_Dont:
	lwz r31, 0x4C(r1)
	lwz r30, 0x48(r1)
	lwz r29, 0x44(r1)
	lwz r0, 0x54(r1)
	mtlr r0
	addi r1, r1, 0x50
	blr


NotTooBigToFreeze:
    lwz       r0, 0x490(r3)
    blr

.global TooBigToFreeze
TooBigToFreeze:

	# Go back if Type is a Player, Yoshi, or World Map Shit
	lhz r10, 8(r28)
	cmpwi r10, 0x12
	blt NotTooBigToFreeze

	# Go back if it's 60
	cmpwi r10, 60
	beq NotTooBigToFreeze

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq NotTooBigToFreeze

	cmpwi r10, 3
	beq F_GlobalCompare

	cmpwi r10, 1
	bne F_UseNyb7
	# use nyb 5
	lwz r6, 4(r28)
	srwi r6, r6, 28
	b F_GotNyb
F_UseNyb7:
	# use nyb 7
	lwz r6, 4(r28)
	extrwi r6, r6, 4, 8
F_GotNyb:
	cmpwi r6, 7
	bge F_Dont
	b NotTooBigToFreeze

F_GlobalCompare:
	lis r6, GlobalSpriteSize@h
	ori r6, r6, GlobalSpriteSize@l
	lwz r6, 0(r6)
	lis r7, 0x3F80
	cmpw r6, r7
	beq NotTooBigToFreeze

F_Dont:
	lwz r31, 0x1C(r1)
	li r3, 1
	lwz r30, 0x18(r1)
	lwz r29, 0x14(r1)
	lwz r28, 0x10(r1)
	lwz r0, 0x24(r1)
	mtlr r0
	addi r1, r1, 0x20
	blr



CollisionNoChanges:
	stw r4, 4(r3)
	li r0, 0
	lfs f0, 0(r5)
	stfs f0, 0x1C(r3)
	lfs f0, 4(r5)
	stfs f0, 0x20(r3)
	lfs f0, 8(r5)
	stfs f0, 0x24(r3)
	lfs f0, 0xC(r5)
	stfs f0, 0x28(r3)
	lbz r4, 0x10(r5)
	stb r4, 0x2C(r3)
	lbz r4, 0x11(r5)
	stb r4, 0x2D(r3)
	lwz r4, 0x14(r5)
	stw r4, 0x30(r3)
	lwz r4, 0x18(r5)
	stw r4, 0x34(r3)
	lhz r4, 0x1C(r5)
	sth r4, 0x38(r3)
	lwz r4, 0x20(r5)
	stw r4, 0x3C(r3)
	stb r0, 0xA1(r3)
	blr


.global ModifyCollision
ModifyCollision:

	# Go back if it's 60
	lhz r10, 8(r4)
	cmpwi r10, 60
	beq CollisionNoChanges

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq CollisionNoChanges

	# Else, use Global
	lis r11, GlobalSpriteSize@h
	ori r11, r11, GlobalSpriteSize@l
	lfs f8, 0(r11)

	mflr r9

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsCF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsC

	mtlr r9

	#This is the old ActivePhysics_InitWithStruct function.
	stw r4, 4(r3)
	li r0, 0
	
	# x_dist_to_center
	lfs f0, 0(r5)			#Load it
#	fmuls f0, f0, f8		#Multiply it by the setting
	stfs f0, 0x1C(r3)		#Store it
		
	# y_dist_to_center		 Time to do all the other ones
	lfs f0, 4(r5)
	fmuls f0, f0, f8
	stfs f0, 0x20(r3)
	
	# x_dist_to_edge
	lfs f0, 8(r5)
	fmuls f0, f0, f8
	stfs f0, 0x24(r3)
	
	# y_dist_to_edge
	lfs f0, 0xC(r5)
	fmuls f0, f0, f8
	stfs f0, 0x28(r3)

	# Rest of the function
	lbz r4, 0x10(r5)
	stb r4, 0x2C(r3)
	lbz r4, 0x11(r5)
	stb r4, 0x2D(r3)
	lwz r4, 0x14(r5)
	stw r4, 0x30(r3)
	lwz r4, 0x18(r5)
	stw r4, 0x34(r3)
	lhz r4, 0x1C(r5)
	sth r4, 0x38(r3)
	lwz r4, 0x20(r5)
	stw r4, 0x3C(r3)
	stb r0, 0xA1(r3)

	blr						# Off we go!


.global ModifyStandOnTopInitA
ModifyStandOnTopInitA:

	# we can trash r10 for now, it's not supposed to be guaranteed anyway
	lwz r10, 4(r3)
	lwz r10, 4(r10)		# get the sprite settings
	srwi r10, r10, 20	# get the highest nybble as r10 (maps to Nyb5 in Reggie)
	andi. r10, r10, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r10, r10, 2	# size *= 4 (size of float)
	lfsx f8, r11, r10	# get it from the array

	fmuls f1, f1, f8	
	fmuls f2, f2, f8	
	fmuls f3, f3, f8	
	fmuls f4, f4, f8	
	fmuls f5, f5, f8	

	#This is the old function.
	stfs f1, 0x4C(r3)
	stfs f2, 0x50(r3)
	stfs f3, 0x54(r3)
	stfs f4, 0x58(r3)
	stfs f5, 0x5C(r3)

	blr						# Off we go!


.global ModifyStandOnTopInitB
ModifyStandOnTopInitB:

	# we can trash r10 for now, it's not supposed to be guaranteed anyway
	lwz r10, 4(r3)
	lwz r10, 4(r10)		# get the sprite settings
	srwi r10, r10, 20	# get the highest nybble as r10 (maps to Nyb5 in Reggie)
	andi. r10, r10, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r10, r10, 2	# size *= 4 (size of float)
	lfsx f8, r11, r10	# get it from the array

	fmuls f1, f1, f8	
	fmuls f2, f2, f8	
	fmuls f3, f3, f8	
	fmuls f4, f4, f8	
	fmuls f5, f5, f8	


	#This is the old function. I optimized it a tad. A big tad.
	lfs f0, 0(r7)
	fmuls f1, f1, f0
	fmuls f4, f4, f0	
	fmuls f5, f5, f0	

	lfs f0, 4(r7)
	fmuls f2, f2, f0	
	fmuls f3, f3, f0	


	stfs f1, 0x4C(r3)
	stfs f2, 0x50(r3)
	stfs f3, 0x54(r3)
	stfs f4, 0x58(r3)
	stfs f5, 0x5C(r3)

	blr						# Off we go!



.global ModifyStandOnTopInitC
ModifyStandOnTopInitC:
	
	fmr f5, f3
	fmr f4, f2
	fmr f3, f1
	lfs f1, 0(r5)
	lfs f2, 4(r5)
	
	b ModifyStandOnTopInitA


.global ModifyStandOnTopInitD
ModifyStandOnTopInitD:


	# we can trash r10 for now, it's not supposed to be guaranteed anyway
	lwz r10, 4(r3)
	lwz r10, 4(r10)		# get the sprite settings
	srwi r10, r10, 20	# get the highest nybble as r10 (maps to Nyb5 in Reggie)
	andi. r10, r10, 0xF

	# first off, get the value from the lookup table
	lis r11, LookupTable@h
	ori r11, r11, LookupTable@l
	slwi r10, r10, 2	# size *= 4 (size of float)
	lfsx f8, r11, r10	# get it from the array

	fmr f5, f3
	fmr f4, f2
	fmr f3, f1
	lfs f1, 0(r5)
	lfs f2, 4(r5)

	fmuls f1, f1, f8	
	fmuls f2, f2, f8	
	fmuls f3, f3, f8	
	fmuls f4, f4, f8	
	fmuls f5, f5, f8	

	lfs f0, 0(r8)
	fmuls f1, f1, f0
	fmuls f4, f4, f0	
	fmuls f5, f5, f0	

	lfs f0, 4(r8)
	fmuls f2, f2, f0	
	fmuls f3, f3, f0	

	stfs f1, 0x4C(r3)
	stfs f2, 0x50(r3)
	stfs f3, 0x54(r3)
	stfs f4, 0x58(r3)
	stfs f5, 0x5C(r3)

	blr						# Off we go!


RiderInitANoChanges:
	lfs f0, 0(r4)
	stfs f0, 0x4C(r3)
	lfs f0, 4(r4)
	stfs f0, 0x50(r3)
	lfs f0, 0(r5)
	stfs f0, 0x54(r3)
	lfs f0, 4(r5)
	stfs f0, 0x58(r3)
	blr

.global ModifyRiderInitA
ModifyRiderInitA:

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq RiderInitANoChanges

	# Else, use Global
	lis r11, GlobalSpriteSize@h
	ori r11, r11, GlobalSpriteSize@l
	lfs f8, 0(r11)

	mflr r9

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsSF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsS

	mtlr r9

	lfs f0, 0(r4)
	fmuls f0, f0, f8
	stfs f0, 0x4C(r3)

	lfs f0, 4(r4)
	fmuls f0, f0, f8
	stfs f0, 0x50(r3)

	lfs f0, 0(r5)
	fmuls f0, f0, f8
	stfs f0, 0x54(r3)

	lfs f0, 4(r5)
	fmuls f0, f0, f8
	stfs f0, 0x58(r3)

	blr						# Off we go!


RiderInitBNoChanges:
	lfs f0, 0(r5)
	stfs f0, 0x4C(r3)
	lfs f0, 4(r5)
	stfs f0, 0x50(r3)
	lfs f0, 0(r4)
	stfs f0, 0x54(r3)
	lfs f0, 4(r4)
	stfs f0, 0x58(r3)
	blr
	

.global ModifyRiderInitB
ModifyRiderInitB:

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq RiderInitBNoChanges

	# Else, use Global
	lis r11, GlobalSpriteSize@h
	ori r11, r11, GlobalSpriteSize@l
	lfs f8, 0(r11)

	mflr r9

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsSF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsS

	mtlr r9

	lfs f0, 0(r5)
	fmuls f0, f0, f8
	stfs f0, 0x4C(r3)

	lfs f0, 4(r5)
	fmuls f0, f0, f8
	stfs f0, 0x50(r3)

	lfs f0, 0(r4)
	fmuls f0, f0, f8
	stfs f0, 0x54(r3)

	lfs f0, 4(r4)
	fmuls f0, f0, f8
	stfs f0, 0x58(r3)

	blr						# Off we go!



RiderInitCNoChanges:
	stfs f1, 0x4C(r3)
	stfs f2, 0x50(r3)
	stfs f3, 0x54(r3)
	stfs f4, 0x58(r3)
	blr	

.global ModifyRiderInitC
ModifyRiderInitC:

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq RiderInitCNoChanges

	# Else, use Global
	lis r11, GlobalSpriteSize@h
	ori r11, r11, GlobalSpriteSize@l
	lfs f8, 0(r11)

	mflr r9

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsSF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsS

	mtlr r9

	fmuls f1, f1, f8	
	fmuls f2, f2, f8	
	fmuls f3, f3, f8	
	fmuls f4, f4, f8	

	stfs f1, 0x4C(r3)
	stfs f2, 0x50(r3)
	stfs f3, 0x54(r3)
	stfs f4, 0x58(r3)

	blr						# Off we go!



RiderInitDNoChanges:
	stfs f3, 0x4C(r3)
	stfs f4, 0x50(r3)
	stfs f1, 0x54(r3)
	stfs f2, 0x58(r3)
	blr	


.global ModifyRiderInitD
ModifyRiderInitD:

	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@h
	ori r10, r10, SizerOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq RiderInitDNoChanges

	# Else, use Global
	lis r11, GlobalSpriteSize@h
	ori r11, r11, GlobalSpriteSize@l
	lfs f8, 0(r11)

	mflr r9

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsSF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsS

	mtlr r9

	fmuls f1, f1, f8	
	fmuls f2, f2, f8	
	fmuls f3, f3, f8	
	fmuls f4, f4, f8	

	stfs f3, 0x4C(r3)
	stfs f4, 0x50(r3)
	stfs f1, 0x54(r3)
	stfs f2, 0x58(r3)

	blr						# Off we go!


.global InheritSettings
InheritSettings:

	lwz r4, 4(r3)
	andis. r4, r4, 0xFFF0
	blr
	

.global InheritSettings_B
InheritSettings_B:

	lwz r5, 4(r3)
	andis. r5, r5, 0x0FF0
	blr


.global InheritSettings_C
InheritSettings_C:

	lwz r4, 4(r3)
	andis. r4, r4, 0xFFF0
	blr

.global InheritSettings_D
InheritSettings_D:

	lwz r0, 4(r3)
	andis. r0, r0, 0xFFF0
	or r4, r4, r0
	blr


.global InheritSettings_D_r31
InheritSettings_D_r31:

	lwz r0, 4(r31)
	andis. r0, r0, 0xFFF0
	or r4, r4, r0
	blr



.global AlwaysVisible
AlwaysVisible:

	li r3, 1
	blr




ModifyLoadSizeNoChanges:
	lhz     r0, 0(r30)
	cmpwi   r17, 0
	lhz     r3, 2(r30)
	li      r28, 0
	mulli   r4, r0, 0x28
	lwz     r0, 0x24(r1)
	lhz     r7, 4(r30)
	add     r31, r0, r4
	lwz     r0, 0xC(r31)
	lwz     r6, 0x14(r31)
	li		r6, 12800
	add     r3, r3, r0
	lwz     r8, 0x10(r31)
	subf    r0, r6, r3
	lhz     r4, 0x20(r31)
	li		r4, 12800
	add     r3, r3, r6
	lhz     r5, 0x22(r31)
	li		r5, 12800
	subf    r6, r4, r0
	lwz     r9, 0x18(r31)
	subf    r7, r8, r7
	add     r5, r5, r3
	srawi   r26, r6, 4
	lhz     r4, 0x1C(r31)
	subf    r0, r9, r7
	srawi   r25, r5, 4
	subf    r4, r4, r0
	lhz     r3, 0x1E(r31)
	add     r0, r7, r9
	mr      r27, r25
	srawi   r23, r4, 4
	add     r0, r3, r0
	srawi   r22, r0, 4
	blr

.global ModifyLoadSize
ModifyLoadSize:

	mflr r9
	
	# Go back if SizerOn is 0 (off)
	lis r10, SizerOn@ha
	lbz r10, SizerOn@l(r10)
	cmpwi r10, 0
	b ModifyLoadSizeNoChanges

	# Else, use Global
	lis r11, GlobalSpriteSize@h
	ori r11, r11, GlobalSpriteSize@l
	lfs f8, 0(r11)

	# Load from Nyb5 Settings if SizerOn is 1
	cmpwi r10, 1
	beql LoadFromSettingsLF

	# Load from Nyb7 Settings if SizerOn is 2
	cmpwi r10, 2
	beql LoadFromSettingsL
	
	lhz     r0, 0(r30)
	cmpwi   r17, 0
	lhz     r3, 2(r30)
	li      r28, 0
	mulli   r4, r0, 0x28
	lwz     r0, 0x24(r1)
	lhz     r7, 4(r30)
	add     r31, r0, r4

	lwz     r12, 0xC(r31)
	bl ConverterFunc
	mr		r0, r12
	li		r0, 12800

	lwz     r12, 0x14(r31)
	bl ConverterFunc
	mr		r6, r12
	li		r6, 12800

	add     r3, r3, r0

	lwz     r12, 0x10(r31)
	bl ConverterFunc
	mr		r8, r12
	li		r8, 12800

	subf    r0, r6, r3

	lhz     r12, 0x20(r31)
	bl ConverterFunc
	mr		r4, r12
	li		r4, 12800

	add     r3, r3, r6

	lhz     r12, 0x22(r31)
	bl ConverterFunc
	mr		r5, r12
	li		r5, 12800

	subf    r6, r4, r0

	lwz     r12, 0x18(r31)
	bl ConverterFunc
	mr		r9, r12
	li		r9, 12800
	
	subf    r7, r8, r7
	add     r5, r5, r3
	srawi   r26, r6, 4

	lhz     r12, 0x1C(r31)
	bl ConverterFunc
	mr		r4, r12
	li		r4, 12800

	subf    r0, r9, r7
	srawi   r25, r5, 4
	subf    r4, r4, r0

	lhz     r12, 0x1E(r31)
	bl ConverterFunc
	mr		r3, r12
	li		r3, 12800

	add     r0, r7, r9
	mr      r27, r25
	srawi   r23, r4, 4
	add     r0, r3, r0
	srawi   r22, r0, 4

	mtlr r9
	blr



ConverterFunc:
	# Converts an int to a float
	lis r10, ConvertFloat@h
	ori r10, r10, ConvertFloat@l
	xoris r12,r12,0x8000
	stw r12, 4(r10)
	lfd f0, 0(r10)
	lfd f1, 8(r10)
	fsub f0, f0, f1

	fmuls f0, f0, f8

	# Converts a float to an int
	fctiw f0, f1
	stfd f0, 0(r12)
	lwz r12, 4(r12)
	blr




	


.global PiranhaFireType
PiranhaFireType:

	li r3, 0x76
	blr


.global PiranhaFirePos
PiranhaFirePos:
	lis r11, PiranhaPosF@h
	ori r11, r11, PiranhaPosF@l
	lfs f8, 0(r11)
	
	fadds f1, f1, f8
	fadds f2, f2, f8
	fadds f0, f0, f8

	stfs f0, 0x8(r1)
	stfs f1, 0xC(r1)
	stfs f2, 0x10(r1)

	blr




.global PipeGeneratorSpawn
PipeGeneratorSpawn:

	li r3, 0x87
	li r4, 0
	blr


.global LakituBomb
LakituBomb:

	li r3, 0x33
	blr





.global GlobalZOrderDeath
GlobalZOrderDeath:

	lis r10, ZOrderOn@h
	ori r10, r10, ZOrderOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq NormalZorder

	lis r11, InFront@h
	ori r11, r11, InFront@l
	lfs f8, 0(r11)
	stfs f8, 0xB4(r28)
	stfs f8, 0xC(r1)
	
	mtlr r0
	addi r1, r1, 0x60
	blr

NormalZorder:
	mtlr r0
	addi r1, r1, 0x60
	blr


.global PokeyZOrderDeath
PokeyZOrderDeath:

	lis r10, ZOrderOn@h
	ori r10, r10, ZOrderOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq NormalPokeyZorder

	lis r11, InFront@h
	ori r11, r11, InFront@l
	lfs f0, 0(r11)
	stfs f0, 0xB4(r31)

	lwz r0, 0x5D0(r31)
	blr

NormalPokeyZorder:
	lwz r0, 0x5D0(r31)
	blr


.global PokeyZOrderDeathFreeze
PokeyZOrderDeathFreeze:

	lis r10, ZOrderOn@h
	ori r10, r10, ZOrderOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq NormalPokeyZorderFreeze

	lis r11, InFront@h
	ori r11, r11, InFront@l
	lfs f0, 0(r11)
	stfs f0, 0xB4(r3)

	lwz r0, 0x604(r3)
	blr

NormalPokeyZorderFreeze:
	lwz r0, 0x604(r3)
	blr


.global PokeyZOrderDamage
PokeyZOrderDamage:

	lis r10, ZOrderOn@h
	ori r10, r10, ZOrderOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq NormalPokeyZorderDamage

	lis r11, InFront@h
	ori r11, r11, InFront@l
	lfs f0, 0(r11)
	stfs f0, 0xB4(r31)

	mr r3, r31
	blr

NormalPokeyZorderDamage:
	mr r3, r31
	blr


.global GabonRockZOrderDeath
GabonRockZOrderDeath:

	lis r10, ZOrderOn@h
	ori r10, r10, ZOrderOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq NormalGabonRockZorder

	stfs f0, 0xE8(r3)
	lis r11, InFront@h
	ori r11, r11, InFront@l
	lfs f0, 0(r11)
	stfs f0, 0xB4(r3)
	blr

NormalGabonRockZorder:
	stfs f0, 0xE8(r3)
	blr


.global GabonRockZOrderDeathDrop
GabonRockZOrderDeathDrop:

	lis r10, ZOrderOn@h
	ori r10, r10, ZOrderOn@l
	lbz r10, 0(r10)
	cmpwi r10, 0
	beq NormalGabonRockZorderDrop

	mr r31, r3
	lis r11, InFront@h
	ori r11, r11, InFront@l
	lfs f0, 0(r11)
	stfs f0, 0xB4(r31)
	blr

NormalGabonRockZorderDrop:
	mr r31, r3
	blr


notRegularTeresa:
	li r5, 1
	blr

.global BooZOrderHack
BooZOrderHack:

	lhz r0, 8(r3)
	cmplwi r0, 0xB0
	bne notRegularTeresa

	lfs f8, 0xB4(r30)

	lis r11, booZorderPlus@h
	ori r11, r11, booZorderPlus@l
	lfs f7, 0(r11)

	fadds f8, f8, f7

	stfs f7, 0xB4(r30)
	stfs f7, 0x990(r30)
	stfs f7, 0x99C(r30)

	li r5, 1
	blr


.data

.global GlobalSpriteSize
GlobalSpriteSize:
	.float 1.0

.global GlobalSpriteSpeed
GlobalSpriteSpeed:
	.float 1.0

.global GlobalRiderSize
GlobalRiderSize:
	.float 1.0

.global GlobalStarsCollected
GlobalStarsCollected:
	.int 0

.global SizerOn
SizerOn:
	.byte 0

.align 4

.global ZOrderOn
ZOrderOn:
	.byte 0

.align 4

LookupTable:
	.float 1, 1, 1, 1, 0.25, 0.5, 0.75, 1.25, 1.5, 2, 2.5, 3, 4, 5, 8, 10

RiderLookupTable:
	.float 1, 1, 1, 1, 0.6, 0.7, 0.85, 1, 1, 1.1, 1.25, 1.5, 2, 2.5, 4, 5

PiranhaPosF:
	.float 4.0

InFront:
	.float 5500.0

ConvertFloat:
	.long  0x43300000,0x0,0x43300000,0x80000000
	
	
Stupid:	.string "Fucking Sprite was enlarged by %f times\n"
.align 4

booZorderPlus:
	.float 5500.0

