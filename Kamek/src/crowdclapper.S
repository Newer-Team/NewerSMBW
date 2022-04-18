.extern OneofManySoundPlayers
.extern EventTable

#ifndef __MWERKS__
.set r0,0;   .set r1,1;   .set r2,2; .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31; .set f0,0; .set f2,2; .set f3,3
#endif

.text
.align 4

.global CrowdClapperEventActivate
CrowdClapperEventActivate:

	mflr r0
	bl OneofManySoundPlayers
	mtlr r0
	
	lis r8, EventTable@h
	ori r8, r8, EventTable@l

	lwz r11, 4(r8)

	lwz r9, 4(r31)
	srwi r9, r9, 24
	
	subi r9, r9, 1
	
	li r10, 1
	slw r10, r10, r9
	
	or r11, r11, r10
	stw r11, 0(r8)
	blr