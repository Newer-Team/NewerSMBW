#undef DEBUG

.text

.extern _savegpr_21
.extern _restgpr_21
.extern OSReport

.set data, 31
.set inp_top, 30
.set outp, 29
.set inp, 28
.set outp_save, 27
.set flag, 26
.set count8, 25
.set index, 24
.set len, 23
.set t_temp, 22

.set sp, 1

.global UncompressBackward
UncompressBackward:
	stwu sp, -0x40(sp)
	mflr r0
	stw r0, 0x44(sp)
	addi r11, sp, 0x40
	bl _savegpr_21

	cmpwi r3, 0
	beq exit

	# endianness issues are annoying.
	li r9, -4
	lwbrx outp, r3, r9

	li r9, -8
	lwbrx inp_top, r3, r9

	#lbz r6, -4(r3)
	#lbz r7, -3(r3)
	#lbz r8, -2(r3)
	#lbz r9, -1(r3)
	#slwi r9, r9, 24
	#slwi r8, r8, 16
	#slwi r7, r6, 8
	#or r9, r9, r8
	#or r9, r9, r7
	#or inp_top, r9, r6

	#lbz r6, -8(r3)
	#lbz r7, -7(r3)
	#lbz r8, -6(r3)
	#lbz r9, -5(r3)
	#slwi r9, r9, 24
	#slwi r8, r8, 16
	#slwi r7, r6, 8
	#or r9, r9, r8
	#or r9, r9, r7
	#or outp, r9, r6

	add outp, r3, outp
	srwi t_temp, inp_top, 24
	sub inp, r3, t_temp

	clrlwi inp_top, inp_top, 8
	sub inp_top, r3, inp_top
	mr outp_save, outp

#ifdef DEBUG
	crclr 4*cr0+eq
	lis r3, Start@h
	ori r3, r3, Start@l
	mr r4, outp
	mr r5, inp
	bl OSReport
#endif

loop:
	cmpw inp, inp_top
	ble end_loop
	lbzu flag, -1(inp)

#ifdef DEBUG
	crclr 4*cr0+eq
	lis r3, MFlag@h
	ori r3, r3, MFlag@l
	mr r4, outp
	mr r5, inp
	mr r6, flag
	bl OSReport
#endif

	li count8, 8
loop8:
	subi count8, count8, 1
	cmpwi count8, 0
	blt loop

	andi. r12, flag, 0x80
	bne blockcopy
bytecopy:
#ifdef DEBUG
	crclr 4*cr0+eq
	lis r3, MOne@h
	ori r3, r3, MOne@l
	mr r4, outp
	mr r5, inp
	lbz r6, -1(inp)
	lbz r7, -1(inp)
	bl OSReport
#endif

	lbzu data, -1(inp)
	stbu data, -1(outp)
	b joinhere

blockcopy:
	lbz len, -1(inp)
	lbz index, -2(inp)
	slwi t_temp, len, 8
	or index, index, t_temp
	clrlwi index, index, 20
	addi index, index, 2
	srwi len, len, 4
	addi len, len, 2

#ifdef DEBUG
	crclr 4*cr0+eq
	lis r3, MTwo@h
	ori r3, r3, MTwo@l
	mr r4, outp
	mr r5, inp
	mr r6, len
	mr r7, index
	bl OSReport
#endif

	subi inp, inp, 2

patterncopy:
	lbzx data, outp, index
	stbu data, -1(outp)
	subi len, len, 1
	cmpwi len, 0
	bge patterncopy

joinhere:
	cmpw inp, inp_top
	slwi flag, flag, 1
	bgt loop8

end_loop:
exit:
#ifdef DEBUG
	crclr 4*cr0+eq
	lis r3, Done@h
	ori r3, r3, Done@l
	mr r4, outp
	mr r5, inp
	bl OSReport
#endif

	addi r11, sp, 0x40
	bl _restgpr_21
	lwz r0, 0x44(sp)
	mtlr r0
	addi sp, sp, 0x40
	blr




.data
Start:	.string "[O=%p I=%p] START\n"
MFlag:	.string "[O=%p I=%p] Flag: %02x\n"
MOne:	.string "[O=%p I=%p] Copy: %02x [%c]\n"
MTwo:	.string "[O=%p I=%p] Copy: %02x bytes from offset %06x\n"
Done:	.string "[O=%p I=%p] END\n"

.align 4

