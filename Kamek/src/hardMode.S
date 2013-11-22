.text
#include <asm_setup.S>
.align 4

.global TimerAdjustments
TimerAdjustments:

	lis 	r10, enableHardMode@h
	ori 	r10, r10, enableHardMode@l
	lbz 	r10, 0(r10)
	cmpwi 	r10, 0
	beq 	NoTimerAdjust

	li		r10, 250
	slwi	r10, r10, 0xC
	cmpw	r3, r10
	blt		NoTimerAdjust

	mr		r3, r10

	b 		NoTimerAdjust

NoTimerAdjust:

	subic.	r0, r3, 92	
	blr



.global OneHitKill
OneHitKill:

	lis 	r10, enableHardMode@h
	ori 	r10, r10, enableHardMode@l
	lbz 	r10, 0(r10)
	cmpwi 	r10, 0
	beq 	NoOneHitKill

	li		r0, 0
	cmpwi	r0, 0
	blr

NoOneHitKill:

	cmpwi	r0, 0
	blr



.align 4

.data

.global enableHardMode
enableHardMode:
	.byte 0

.align 4
