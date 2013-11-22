.text

#include <asm_setup.S>

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
