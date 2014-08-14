.extern currentHeap
.extern MakeScene
.extern DVDClass
.extern GetSceneLightInfo
.extern GetRes
.extern GetAnmScn
.extern BindAnmScn
.extern AssignAnmScnToLightInfo
.extern LoadBlight
.extern LoadBlmap


.text

#include <asm_setup.S>

.global LoadMapScene
LoadMapScene:
	stwu sp, -0x30(sp)
	mflr r0
	stw r0, 0x34(sp)

	stw r31, 0x2C(sp)



	lis r3, currentHeap@h
	ori r3, r3, currentHeap@l
	lwz r3, 0(r3)

	li r4, 36	#Light count
	li r5, 8	#Ambient light count
	li r6, 2	#Dunno
	li r7, 0	#Make fog (bool)
	li r8, 0	#Scene ID

	bl MakeScene


	# Now make the actual scene!
	# Get light info
	li r3, 0
	bl GetSceneLightInfo
	mr r31, r3

	# Get scene/scene.brres
	lis r3, DVDClass@h
	ori r3, r3, DVDClass@l
	lwz r3, 0(r3)
	addi r3, r3, 4

	lis r4, EnvWorld@h
	ori r4, r4, EnvWorld@l

	lis r5, SceneBrres@h
	ori r5, r5, SceneBrres@l

	bl GetRes

	# Got that, now get the AnmScn we want (MainSelect)
	stw r3, 0x0C(sp)	#ResFile

	addi r3, sp, 0xC
	lis r4, MainSelect@h
	ori r4, r4, MainSelect@l
	bl GetAnmScn
	stw r3, 0x10(sp)

	# Bind it
	addi r3, sp, 0x10
	addi r4, sp, 0x10
	bl BindAnmScn

	# Add it to lightinfo
	mr r3, r31			#This
	addi r4, sp, 0x10	#AnmScn pointer
	li r5, -1			#Dunno
	li r6, 3			#Dunno

	lis r9, Zero@h
	ori r9, r9, Zero@l
	lfs f1, 0(r9)		#Dunno

	bl AssignAnmScnToLightInfo


	# Now set up the rest of the scene
	# Get blight
	lis r3, DVDClass@h
	ori r3, r3, DVDClass@l
	lwz r3, 0(r3)
	addi r3, r3, 4

	lis r4, EnvWorld@h
	ori r4, r4, EnvWorld@l

	lis r5, BlightW1@h
	ori r5, r5, BlightW1@l

	bl GetRes

	# Load it into lightinfo
	mr r4, r3
	mr r3, r31
	bl LoadBlight



	# Do the same for blmap
	lis r3, DVDClass@h
	ori r3, r3, DVDClass@l
	lwz r3, 0(r3)
	addi r3, r3, 4

	lis r4, EnvWorld@h
	ori r4, r4, EnvWorld@l

	lis r5, BlmapW1@h
	ori r5, r5, BlmapW1@l

	bl GetRes

	# Load it into its class
	mr r4, r3
	lwz r3, 0x14(r31)
	bl LoadBlmap


	# DONE!!
	lwz r31, 0x2C(sp)

	lwz r0, 0x34(sp)
	mtlr r0
	addi sp, sp, 0x30
	blr






.data
EnvWorld:	.string "Env_world"
SceneBrres:	.string "scene/scene.brres"
MainSelect:	.string "MainSelect"
.align 4
Zero:		.float 0.0
BlightW1:	.string "light/W8.blight"
BlmapW1:	.string "light/W8.blmap"

.align 4

