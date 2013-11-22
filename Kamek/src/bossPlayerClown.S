.text
#include <asm_setup.S>
.align 4
.extern ContinueFromClownCarDetectAdjacent
.extern calculateAdjacentCollision__14collisionMgr_cFPf
.global ClownCarDetectAdjacents
ClownCarDetectAdjacents:
	addi r3, r30, 0x1EC
	li r4, 0
	bl calculateAdjacentCollision__14collisionMgr_cFPf
	cmpwi r3, 0
	beq notAdjacent

	# what direction are we in?
	lfs f1, 0xE8(r30) # x speed
	lis r4, zeroFloat@h
	ori r4, r4, zeroFloat@l
	lfs f2, 0(r4)

	lbz r4, 0x348(r30)

	cmpwi r4, 1
	beq isLeft

	# RIGHT
	fcmpo cr0, f1, f2 # xspeed > 0
	blt notAdjacent
	stfs f2, 0xE8(r30)
	b notAdjacent

isLeft:
	# LEFT
	fcmpo cr0, f1, f2 # xspeed < 0
	bgt notAdjacent
	stfs f2, 0xE8(r30)

notAdjacent:
	# END HERE

	mr r3, r30
	b ContinueFromClownCarDetectAdjacent


.data
.align 4
zeroFloat: .float 0.0

