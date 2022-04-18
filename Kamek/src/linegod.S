.text

#ifndef __MWERKS__
.set r0,0;   .set r1,1;   .set r2,2; .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31; .set f0,0; .set f2,2; .set f3,3
#endif

.align 4
.extern ContinueBgActorSpawn
.global BgActorSpawnFix
BgActorSpawnFix:
	mr 30, 3
	
	#lis 3, TestStr@h
	#ori 3, 3, TestStr@l
	#mr 4, 30
	#crclr 4*cr1+eq
	#bl OSReport
	
	lbz 0, 7(30)
	cmpwi 0, 1
	beq JustRet
	b ContinueBgActorSpawn
JustRet:
	
	lwz 31, 0x3C(1)
	lwz 30, 0x38(1)
	lwz 0, 0x44(1)
	mtlr 0
	addi 1, 1, 0x40
	blr


.global BgActorSetInfoFix
BgActorSetInfoFix:
	sth 4, 0(3)
	sth 5, 2(3)
	sth 6, 4(3)
	stb 7, 6(3)
	li 4, 0
	stb 4, 7(3)
	blr


#.data
#TestStr:
#	.string "Spawning actor %p\n"
#	.align 4
