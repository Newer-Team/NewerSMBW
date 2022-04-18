#ifndef __MWERKS__
.set r0,0;   .set r1,1;   .set r2,2; .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31; .set f0,0; .set f2,2; .set f3,3
.set f1,1;   .set f30,30; .set f31,31
#endif

.set sp,1

#define CODE_BASE 3
#define RELOC 4
#define RELOCS_END 11
#define TARGETS_BASE 11

.global LinkNewerRel
LinkNewerRel:
	# Parameters:
	#  r3: CODE_BASE
	#  r4: RELOC (relocs data)

	lwz TARGETS_BASE, 8(RELOC)
	add TARGETS_BASE, RELOC, TARGETS_BASE

	addi RELOC, RELOC, 0xC

	b startLoop
loop:
#define REL_TYPE 5
#define TARGET 6
#define ADDR 7
#define TEMP 8
#define TEMP2 9

	lwz REL_TYPE, 0(RELOC)
	lwz ADDR, 4(RELOC)
	addi RELOC, RELOC, 8

	clrlslwi TARGET, REL_TYPE, 8, 2
	srwi REL_TYPE, REL_TYPE, 24

	# get the target
	lwzx TARGET, TARGETS_BASE, TARGET

	# check top bit to see if it's a RAM address
	extrwi. TEMP, TARGET, 1, 0
	bne targetIsAbsolute
	add TARGET, CODE_BASE, TARGET
targetIsAbsolute:

	# and do the same for the patch addr
	extrwi. TEMP, ADDR, 1, 0
	bne addrIsAbsolute
	add ADDR, CODE_BASE, ADDR
addrIsAbsolute:

	# jump to the handler for this rel
	cmpwi REL_TYPE, 1
	beq p_ADDR32
	cmpwi REL_TYPE, 4
	beq p_ADDR16_LO
	cmpwi REL_TYPE, 5
	beq p_ADDR16_HI
	cmpwi REL_TYPE, 6
	beq p_ADDR16_HA
	cmpwi REL_TYPE, 10
	beq p_REL24

	b continueLoop

p_ADDR32:
	stw TARGET, 0(ADDR)
	b continueLoop

p_ADDR16_LO:
	sth TARGET, 0(ADDR)
	b continueLoop

p_ADDR16_HI:
	srwi TARGET, TARGET, 16
	sth TARGET, 0(ADDR)
	b continueLoop

p_ADDR16_HA:
	extrwi TEMP, TARGET, 1, 16
	srwi TARGET, TARGET, 16
	add TARGET, TARGET, TEMP
	sth TARGET, 0(ADDR)
	b continueLoop

p_REL24:
#define INSN TEMP
#define DIFF TEMP2
	sub DIFF, TARGET, ADDR
	clrlwi DIFF, DIFF, 6
	lwz INSN, 0(ADDR)
	or INSN, INSN, DIFF
	stw INSN, 0(ADDR)
	# no branch because it's the last one

continueLoop:
	# do stuff with the cache
	dcbst r0, ADDR
	sync
	icbi r0, ADDR

startLoop:
	cmpw RELOC, RELOCS_END
	blt loop

	sync
	isync

	blr

