.text
#include <asm_setup.S>
.extern StaffCreditsAddCoin
.extern CreditsModeActive
.extern continueAddToCoinCount
.global StaffCreditsAddAllCoins
StaffCreditsAddAllCoins:
	lis r4, CreditsModeActive@h
	ori r4, r4, CreditsModeActive@l
	lbz r4, 0(r4)
	cmpwi r4, 1
	beq isCredits
	stwu r1, -0x40(r1)
	b continueAddToCoinCount
isCredits:
	b StaffCreditsAddCoin


.extern continueAddToScore
.global StaffCreditsNoScore
StaffCreditsNoScore:
	lis r9, CreditsModeActive@h
	ori r9, r9, CreditsModeActive@l
	lbz r9, 0(r9)
	cmpwi r9, 1
	beqlr
	cmpwi r5, -1
	b continueAddToScore


.global AutoScrollInjector
.extern BG_GM_ptr
AutoScrollInjector:
	lis r3, CameraLockEnabled@h
	ori r3, r3, CameraLockEnabled@l
	lbz r3, 0(r3)
	cmpwi r3, 1
	bnelr

	lis r3, BG_GM_ptr@h
	ori r3, r3, BG_GM_ptr@l
	lwz r3, 0(r3)

	lis r4, CameraLockPosition@h
	ori r4, r4, CameraLockPosition@l

	addis r3, r3, 9
	lfs f1, 0(r4)
	stfs f1, -0x178(r3)
	lfs f1, 4(r4)
	stfs f1, -0x174(r3)
	blr


.global PlayerRotInjector
PlayerRotInjector:
	lis r9, isLockPlayerRotation@h
	ori r9, r9, isLockPlayerRotation@l
	lbz r9, 0(r9)
	cmpwi r9, 1
	bne dontOverridePlayerRot
	lis r9, lockedPlayerRotation@h
	ori r9, r9, lockedPlayerRotation@l
	lha r7, 0(r9)
dontOverridePlayerRot:
	sth r7, 0xA(r1)
	blr

.global PlayerRotInjector2
PlayerRotInjector2:
	lis r9, isLockPlayerRotation@h
	ori r9, r9, isLockPlayerRotation@l
	lbz r9, 0(r9)
	cmpwi r9, 1
	bne dontOverridePlayerRot2
	lis r9, lockedPlayerRotation@h
	ori r9, r9, lockedPlayerRotation@l
	lha r0, 0(r9)
dontOverridePlayerRot2:
	sth r0, 0xA(r1)
	blr

.global PlayerRotInjector3
PlayerRotInjector3:
	lis r9, isLockPlayerRotation@h
	ori r9, r9, isLockPlayerRotation@l
	lbz r9, 0(r9)
	cmpwi r9, 1
	bne dontOverridePlayerRot3
	lis r9, lockedPlayerRotation@h
	ori r9, r9, lockedPlayerRotation@l
	lha r7, 0(r9)
dontOverridePlayerRot3:
	sth r7, 0x12(r1)
	blr

# FOR SOME FUN
	# li r0, 0x6000
	# sth r0, 0x12(r1)
	# li r0, 0x4000
	# sth r0, 0x10(r1)
	# li r0, 0x8000
	# sth r0, 0x14(r1)


.data
.global CameraLockEnabled
.global isLockPlayerRotation
.global CameraLockPosition
.global lockedPlayerRotation
CameraLockEnabled: .byte 0
isLockPlayerRotation: .byte 0
lockedPlayerRotation: .short 0
.align 4
CameraLockPosition: .float 0.0, 0.0
