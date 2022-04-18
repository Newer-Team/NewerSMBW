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
	cmp		r3, r10
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
