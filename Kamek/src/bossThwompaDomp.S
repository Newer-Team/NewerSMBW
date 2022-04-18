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
.extern ThwompaDompMoveC__FP5dEn_c

.align 4



.global ThwompaDompMove
ThwompaDompMove:
	
	# Registers I can't clobber - r0, r3, r30, r31
	# r3 is self
	
	subi r0, r4, 1
	
	
	# Load settings and make sure it's thwompy
	lwz r4, 4(r3)
	slwi r4, r4, 28
		
	cmpwi r4, 1
	b ThwompaDompMoveC__FP5dEn_c
	
	blr
	
