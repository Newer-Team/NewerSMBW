# initial setup
# * add exception handling for all cases
# many breakpoints handling
# * add breakpoint list with addresses
# * add breakpoint list with original values
# allow PPC side searches
# * u32 with number of matches
# * list of addresses that match
# breakpoint conditions
# * address == foo
# *		&&
# * r* == bar
# * r* != bar
# * r* > bar
# * r* < bar

.text

.set r0,0;   .set r1,1;   .set r2,2;   .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31; .set f0,0;   .set f2,2;   .set f3,3

.set FROZENVAL, 18;
.set CCOO, 20;
.set ZEROVAL, 21;
.set CRCHECK, 22;
.set EXIPORT, 23;
.set CDOO, 24;
.set MSR, 25;
.set NEWMSR, 26;
.set REMAINDER, 27;
.set MP3PROT, 28;
.set RECVBYTE, 29;
.set TEMPLR, 30;
.set STATE, 31;

.globl _codehandler_start

gameid:
.long	0,0
cheatdata:
.long	frozenvalue
.space 39*4

.extern OSReport
_codehandler_start:
#if 0
	lis r3, debug_string@h
	ori r3, r3, debug_string@l
	bl OSReport
debug_string:
.string "Codehandler called\n"
#endif
	stwu	r1,-168(r1)		# stores sp
	stw		r0,8(r1)		# stores r0

	mflr	r0
	stw		r0,172(r1)		# stores lr

	mfcr	r0
	stw		r0,12(r1)		# stores cr

	mfctr	r0
	stw		r0,16(r1)		# stores ctr

	mfxer	r0
	stw		r0,20(r1)		# stores xer

	stmw	r3,24(r1)		# saves r3-r31

	mfmsr	MSR
	ori		NEWMSR,MSR,0x2000	#enable floating point ?
	andi.	NEWMSR,NEWMSR,0xF9FF
	mtmsr	NEWMSR

	stfd	f2,152(r1)		# stores f2
	stfd	f3,160(r1)		# stores f3

	lis		CCOO, 0xCC00
	lhz		MP3PROT, 0x4010(CCOO)
	ori		r21, MP3PROT, 0xFF
	sth		r21, 0x4010(CCOO) # disable MP3 memory protection

_check_state:
	lis		STATE, state@h
	ori		STATE, STATE, state@l
	lwz		r5, 0(STATE)
	cmpwi	r5, 0
	bne- _setvalues

_init_exceptions:
	# 4,3,5,17 are trampled
	lis		r4,bphandler@h
	ori		r4,r4,bphandler@l
	lis		r3,0x8000
	ori		r3,r3,0x300			# bprw  0x80000300
	bl  writebranch

	addi	r3,r3,0xA00			# trace 0x80000D00
	bl	writebranch

	addi	r3,r3,0x600			# bpx   0x80001300
	bl	writebranch

	li		r5, 1
	stw		r5, 0(STATE)		# set state to 1 (exception handling linked)

_setvalues:
	li		ZEROVAL,0
	li		CRCHECK,0x19
	li		EXIPORT,0xD0
	lis		CDOO,0xCD00

	lis		FROZENVAL, frozenvalue@h
	ori		FROZENVAL, FROZENVAL, frozenvalue@l	# read buffer just store in lowmem
	lwz		r0,172(r1)				# loads lr
	#stw	r0,4(r18)				# stores lr
	stw		r0,4(FROZENVAL)			# stores lr
	stw		ZEROVAL, 0x643C(CDOO)	# exi speed up

frozen:
	bl	exireceivebyte
	beq	finish					# r3 returns 1 or 0, one for byte received ok

checkcommand:
	cmpwi	RECVBYTE, 0x04		# checks if 8/16/32 bits write command
	bge _nextcommand
	cmpwi	RECVBYTE, 0x01
	blt finish
	b   writedword				#write value to address
_nextcommand:
	beq	readmem
	cmpwi	RECVBYTE, 0x06
	beq	freezegame
	cmpwi	RECVBYTE, 0x07
	beq	unfreezegame
	cmpwi	RECVBYTE, 0x08
	beq	resumegame
	cmpwi	RECVBYTE, 0x09
	beq	breakpoints		#ibp
	cmpwi	RECVBYTE, 0x10
	beq	breakpoints		#dbp
	cmpwi	RECVBYTE, 0x2F
	beq	upbpdata
	cmpwi	RECVBYTE, 0x30
	beq	getbpdata
	cmpwi	RECVBYTE, 0x38
	beq	cancelbreakpoints
	cmpwi	RECVBYTE, 0x41
	beq	uploadcode
	cmpwi	RECVBYTE, 0x44
	beq	breakpoints #step
	cmpwi	RECVBYTE, 0x50
	beq	pausestatus
	cmpwi	RECVBYTE, 0x89
	beq	breakpoints #aligned dbp
	cmpwi	RECVBYTE, 0x99
	beq	versionnumber
	b	finish

#******************************************************************
#						subroutine: getpausestatus:
#				zero  running, one  paused, two  on a breakpoint
#******************************************************************

pausestatus:
	lwz		r3,0(FROZENVAL)
	bl  exisendbyte
	b   finish


#******************************************************************
#						subroutine: freezegame:
#				Write a 1 to a memory location to freeze
#******************************************************************

freezegame:
	li		r4, 1
	stw		r4, 0(FROZENVAL)
	b   finish

#******************************************************************
#						subroutine: unfreezegame:
#				Write a 0 to a memory location to unfreeze
#******************************************************************

unfreezegame:
	stw		ZEROVAL, 0(FROZENVAL)
	b   resumegame

#******************************************************************
#						subroutine: upload bp data:
#				receive the dp databuffer from the PC
#******************************************************************

upbpdata:
	bl  exisendbyteAA

	li		r16, 40*4				# 40 "registers rN" * 4
	lis		r12, regbuffer@h
	ori		r12, r12, regbuffer@l
	b   upload

#******************************************************************
#						subroutine: getbp data:
#				send the dp databuffer to the PC
#******************************************************************

getbpdata:
	li		r3, 72*4				# register buffer
	lis		r12, regbuffer@h
	ori		r12, r12, regbuffer@l
	bl  exisendbuffer			# send it, less than a packet
	b   finish


#******************************************************************
#						subroutine: breakpoints:
#				handles data/instruction address breakpoints
#******************************************************************

breakpoints:

	cmpwi	cr6,RECVBYTE,0x10		# 0x10=instruction, 0x09/0x89=data
	cmpwi	cr5,RECVBYTE,0x44		# 0x44=step

	lis r4, bphandler@h
	ori	r4,r4,bphandler@l	#used for writebranch (=source address)

	lis	r3,0x8000
	ori	r3,r3,0x300			# bprw  0x80000300
	bl	writebranch

	addi r3,r3,0xA00		# trace 0x80000D00
	bl	writebranch

	addi r3,r3,0x600		# bpx   0x80001300
	bl	writebranch

	lis r12, bpbuffer@h
	ori	r12, r12, bpbuffer@l	# read buffer just store in lowmem

	stw	ZEROVAL,0(r12)		# clears inst bp
	stw	ZEROVAL,4(r12)		# clears data bp
	stw	ZEROVAL,8(r12)		# clears alignement

	lis r4, regbuffer@h
	ori	r4, r4, regbuffer@l
	lwz	r9,0x18(r4)			# srr1

	lwz	r3,0(FROZENVAL)
	cmpwi	r3,2		# checks if the gecko is on a breakpoint
	;bne	+12
	;beq	cr5,+12
	bne p12
	beq cr5, p12_2
	b	bp
p12:
	li	r3,0
p12_2:
	stw	r3,12(r12)			# if not, clears the previous "broken on" address
	bne	bp

	bne	cr5,bp
	ori	r9,r9,0x400
	stw	r9,0x18(r4)			# srr1 |= 0x400			# set SE (single step)
	b	unfreezegame

bp:
	rlwinm	r9,r9,0,22,20
	stw	r9,0x18(r4)			# srr1 &= 0xfffffbff	# clear SE (single step)
	beq	cr5,finish
	;beq	cr6,+8
	beq cr6, p8
	addi r12,r12,4			# if databp r12=r12+4

p8:
	li	r3, 4				# 4 bytes
	bl	exireceivebuffer

	ble	cr6,noalignement	# if not aligned data bp (0x89)

	addi	r12,r12,4

	li	r3, 4				# 4 bytes
	bl	exireceivebuffer

noalignement:
	lis r4, bpbuffer@h
	ori	r4, r4, bpbuffer@l	# read buffer just store in lowmem

	lwz	r3, 0(r4)			# inst bp
	lwz	r4, 4(r4)			# data bp
	mtspr	1010, r3		# set IABR
	mtspr	1013, r4		# set DABR
	b	finish

#******************************************************************
#						subroutine: bphandler
#	Data/Instruction address breakpoint handler, save context and return
#******************************************************************

#FIXME
bphandler:
	mtsprg	2,r1			# sprg2 = r1
	lis r1, regbuffer@h
	ori r1, r1, regbuffer@l
	mtsprg	3,r3			# sprg3 = r3
	mfsrr1	r3				# r3=srr1
	rlwinm	r3,r3,0,22,20		# clear trace
	xoris	r1, r1, 0x8000		# ensure physical address
	stw r3, 0x18(r1)
	rlwinm	r3,r3,0,24,15
	ori	r3,r3,0x2000
	mfsrr0	r1				# r1=srr0

#	rlwinm	r3,r3,0,17,15		# clear hw interrupt
	mtsrr1	r3			# restore srr1 with hw interrupt & trace cleared

	lis	r3,break@h
	ori	r3,r3,break@l
	mtsrr0	r3
	rfi

break:
	lis	r3, regbuffer@h
	ori	r3, r3, regbuffer@l
	stw	r1,0x14(r3)		#stores srr0

	mr	r1,r3
	mfsprg	r3,3
	stmw	r2,0x24(r1)		#saves r2-r31
	mr	r4,r1
	mfsprg	r1,2

	stw	r0,0x1C(r4)
	stw	r1,0x20(r4)

	mflr	r3
	stw	r3, 0x9C(r4)		# Store LR

	mfcr	r3
	stw	r3, 0x0(r4)		# Store CR

	mfxer	r3
	stw	r3, 0x4(r4)		# Store XER

	mfctr	r3
	stw	r3, 0x8(r4)		# Store CTR

	mfdsisr	r3
	stw	r3, 0xC(r4)		# Store DSISR

	mfdar	r3
	stw	r3, 0x10(r4)	# Store DAR

	li	r9,0
	mtspr	1010,r9		# Clear IABR
	mtspr	1013,r9		# Clear DABR

	lis r5,floatstore@h
	ori r5,r5,floatstore@l # Set r5 to instruction address
	lis r31,0xD004
	ori r31,r31,0x00A0 # Set r31 to 'stfs f0,0xA0(r4)' (==0xD00400A0)
floatloop:
	stw r31,0(r5)
	dcbst r0,r5
	sync
	icbi r0,r5
	isync
floatstore:
	stfs	f0,0xA0(r4)
	addi r31,r31,4			# Advance instruction offset
	addis r31,r31,0x20		# Advance instruction register
	rlwinm. r16,r31,0,5,5	# Check for register > 31
	beq floatloop

skip_floating_point:

	lis r5, bpbuffer@h
	ori	r5,r5,bpbuffer@l

	lwz	r16,0(r5)			# inst bp
	lwz	r17,4(r5)			# data bp
	lwz	r19,12(r5)			# counter for alignment

	cmpwi	r19,0
	beq	_redobp

	cmpwi	r19,2
	;bne	+24
	bne p24
	lwz	r9,0x14(r4)
	addi	r9,r19,3
	stw	r9,0(r5)			# inst bp
	stw	r9,12(r5)			# counter for alignment
	b	_executebp

p24:
	cmpw	r16,r19
	beq	_step

	cmpw	r17,r19
	beq	_step

	add	r9,r16,r17
	stw	r9,12(r5)		# counter for alignment


_alignementcheck:
	lwz	r16,8(r5)		# bp alignment check
	cmpwi	r16,0
	beq	_executebp		# no alignement = normal break

	lwz	r3,0x10(r4)
	cmpw	r16,r3		# we check if address = aligned address
	bne	_step			# if no, we need to set a bp on the next instruction

	li	r16,0
	stw	r16,8(r5)		# if we are on the good address we clear the aligned bp check
	b	_executebp		# and we break

_step:
	li	r17,0
	stw	r17,12(r5)		# counter for alignment
	lwz	r9,0x18(r4)
	ori	r9,r9,0x400
	stw	r9,0x18(r4)
	b	_skipbp			#and we don't break right now

_redobp:
	mtspr	1010,r16		# we set back the instbp with the original value
	mtspr	1013,r17		# we set back the databp with the original value
	li	r9,1
	stw	r9,12(r5)			# counter for alignment
	b	_skipbp				# and we don't break

_executebp:
	li	r5, 2
	#;TODO check FROZENVAL vs r4 in other
	lis FROZENVAL, frozenvalue@h
	ori	FROZENVAL, FROZENVAL, frozenvalue@l	# Freeze once returned to let user know there is a breakpoint hit
	stw	r5, 0(FROZENVAL)

# NOT NEEDED BY MEGAZIG
#	li	r3, 0x11
#	bl	exisendbyte		# tell the PC a bp has happened (send 0x11)
#
	nop
	nop

	bl	_codehandler_start			# bl mainloop, so you can set up a new breakpoint.

_skipbp:
	mfmsr	r1
	rlwinm	r1,r1,0,31,29
	rlwinm	r1,r1,0,17,15
	mtmsr	r1			# we disable the interrupt so nothing interfers with the restore

	lis r1, regbuffer@h
	ori	r1, r1, regbuffer@l

	lwz	r3,0x0(r1)
	mtcr	r3			# restores CR
	lwz	r3,0x14(r1)
	mtsrr0	r3			# restores SRR0
	lwz	r3,0x18(r1)
	mtsrr1	r3			# restores SRR1
	lwz	r3,0x9C(r1)
	mtlr	r3			# restores LR

	lmw	r2,0x24(r1)		# restores r2-r31

	lwz	r0,0x1C(r1)		# restores r0
	lwz	r1,0x20(r1)		# restores r1

	rfi				# back to the game

#******************************************************************
#						subroutine: write dword:
#				Write a long word value to memory sent from PC
#******************************************************************

writedword:
	cmpwi	cr5,RECVBYTE,2
	li	r3, 8			# 8 bytes (location 4 / Value 4)

	lis r12, dwordbuffer@h
	ori	r12, r12, dwordbuffer@l	# buffer
	bl	exireceivebuffer

	lwz	r5, 0(r12)
	lwz	r3, 4(r12)		# read the value
	stb	r3, 0(r5)		# write to location
	blt	cr5,skipp
	sth	r3, 0(r5)		# write to location
	beq	cr5,skipp
	stw	r3, 0(r5)		# write to location
skipp:
	dcbf	r0, r5		# data cache block flush
	sync
	icbi	r0, r5
	isync
	b	finish

#******************************************************************
#						subroutine: Upload :
#						Fill memory with value
#******************************************************************

uploadcode:
	bl	exisendbyteAA

	li	r3, 8			# 8 bytes
	lis r12, dwordbuffer@h
	ori	r12, r12, dwordbuffer@l	# buffer

	bl	exireceivebuffer

	lwz	r16, 4(r12)
	lwz	r12, 0(r12)

upload:
	lis REMAINDER, rem@h
	ori	REMAINDER, REMAINDER, rem@l		# Remainder of bytes upload code
	li	r17,0xF80
	bl	_packetdivide
	beq	douploadremainder

nextrecpacket:
	mr	r3,r17
	bl	exireceivebuffer	# r12 start = start of buffer

uploadwait:
	bl	exisendbyteAA
	beq	uploadwait
	add	r12,r12,r14

	subic.	r11, r11, 1		# decrease loop counter
	bgt	nextrecpacket

douploadremainder:			# send the remainder bytes
	lwz	r3, 0(REMAINDER)	# remainder
	cmpwi	r3,0
	beq	finishupload
	bl	exireceivebuffer
finishupload:
	dcbf	r0, r12		# data cache block flush
	sync
	icbi	r0, r12		# instruction cache block flush
	isync
	b	finish

#******************************************************************
#			subroutine: exireceivebyte:
#		 Return 1(r3) if byte receive, 0(r3) if no byte
#		 Command byte is stored at 0x800027ff
#******************************************************************

exireceivebyte:
	mflr	TEMPLR
	lis	r3, 0xA000		# EXI read command

	bl checkexisend

	andis.	r3, r16, 0x800
	rlwinm	RECVBYTE,r16,16,24,31
	mtlr	TEMPLR
	blr

#******************************************************************
#						subroutine: checkexisend:
#
#******************************************************************

checkexisend:

	stw	EXIPORT, 0x6814(CDOO)		# 32mhz Port B
	stw	r3, 0x6824(CDOO)
	stw	CRCHECK, 0x6820(CDOO)	  # 0xCC006820 (Channel 1 Control Register)

exicheckreceivewait:
 	lwz	r5, 0x6820(CDOO)
	andi.	r5, r5, 1
	bne	exicheckreceivewait	# while((exi_chan1cr)&1);

	lwz	r16, 0x6824(CDOO)
	stw	r5, 0x6814(CDOO)

	blr

#******************************************************************
#						subroutine: exireceivebuffer:
#  r3 byte counter, r12 points to buffer, r3 gets copied as gets destroyed
#******************************************************************

exireceivebuffer:
	mflr	r10			# save link register
	mtctr	r3			# counter
	li	r14,0

bufferloop:
	bl	exicheckreceive
	bl	exicheckreceive
	bl	exireceivebyte
	beq	bufferloop		# r3 returns 1 or 0, one for byte received ok

	stbx	RECVBYTE, r14,r12		# store byte into buffer

	addi	r14, r14, 1		# increase buffer by 1
	bdnz	bufferloop

	mtlr	r10			# retore link register
	blr					# return to command check

#******************************************************************
#						exisendbuffer:
#  r3 byte counter, r12 points to buffer, r3 gets copied as gets destroyed
#******************************************************************

exisendbuffer:
	mflr	r10			# save link register
	mtctr	r3			# r3->counter
	li	r14,0

sendloop:
	bl  exichecktx
	beq sendloop
	lbzx	r3, r12,r14
	bl	exisendbyte
	beq	sendloop

	addi	r14, r14, 1		# increase buffer
	bdnz	sendloop
	mtlr	r10			# restore link register
	blr

#******************************************************************
#						exisendbyte:12345678
#  r3 byte to send, returns 1 if sent, 0 if fail (!!! called by breakpoint)
#******************************************************************
exisendbyteAA:
	li	r3,0xAA

exisendbyte:					# r3, send value
	mflr	TEMPLR
	slwi	r3, r3, 20			# (sendbyte<<20);
	oris	r3, r3, 0xB000		# 0xB0000000 | (OR)
	#;TODO why am I missing?
	li	r22,0x19			# already done in _setvals
	li	r23,0xD0			# already done in _setvals
	lis	r24,0xCD00			# already done in _setvals

	bl checkexisend

	extrwi.  r3, r16, 1,5		# returns either 0 or 1, one for byte received ok
	mtlr	TEMPLR
	blr

#******************************************************************
#						subroutine: exicheckrecieve:
#		 Return 1(r3) if byte receive, 0(r3) if no byte
#******************************************************************

exicheckreceive:
	mflr	TEMPLR
exicheckreceive2:
	lis	r3, 0xD000		# EXI check status command

	bl checkexisend

	rlwinm.	r3,r16,6,31,31		# returns either 0 or 1 for r3

	beq	 exicheckreceive2
	mtlr	TEMPLR
	blr

#******************************************************************
#				subroutine: exichecktx:
#		Return 1 if ok to send, 0 if not
#******************************************************************
exichecktx:
	mflr	r30
	lis r3, 0xC000
	#; TODO yet I'm here
	li  r22, 0x09
	li  r23, 0xD0
	lis r24, 0xCD00

	bl checkexisend

	extrwi. r3, r16, 1,5		# returns either 0 or 1, one for byte received ok
	mtlr	r30
	blr

#******************************************************************
#				Readmem: (reads a memory range back to PC)
#				r3 byte to send, returns 1 if sent, 0 if fail
#******************************************************************

readmem:
	bl	exisendbyteAA

	li	r3, 8						# 8 bytes
	lis r12, dwordbuffer@h
	ori	r12, r12, dwordbuffer@l		# buffer

	bl	exireceivebuffer

	lwz	r5, 4(r12)
	lwz	r12, 0(r12)

	lis REMAINDER, rem@h
	ori	REMAINDER, REMAINDER, rem@l	# place holder for remainder bytes

	ori	r17, ZEROVAL, 0xF800		# 64K packet
	sub	r16, r5, r12				# memrange = (*endlocation - *startlocation)

	bl	_packetdivide

nextsendpacket:
	bgt	transfer			#compares r11 and 0
	lwz	r17, 0(REMAINDER)	# remainder
	cmpwi	r17,0
	beq	finish
transfer:
	mr	r3,r17				# number of bytes
	bl	exisendbuffer

bytewait1:
	bl	exicheckreceive
	bl	exicheckreceive
	bl	exireceivebyte
	beq	bytewait1			# r3 returns 1 or 0, one for byte received ok
	cmpwi	RECVBYTE, 0xCC	# cancel code
	beq	finish
	cmpwi	RECVBYTE, 0xBB	# retry code
	beq	transfer
	cmpwi	RECVBYTE, 0xAA
	bne	bytewait1
	add	r12,r12,r14
	subic.	r11, r11, 1		# decrease loop counter
	blt	finish
	b	nextsendpacket

#******************************************************************
#				Cancel Breakpoints
#				Clears instruction and data and step breakpoints
#******************************************************************

cancelbreakpoints:
	mtspr	1013, ZEROVAL		# clear the DABR
	mtspr	1010, ZEROVAL		# clear the IABR
	lis		r4, regbuffer@h
	ori		r4, r4, regbuffer@l
	lwz		r9,0x18(r4)
	rlwinm	r9,r9,0,22,20
	stw		r9,0x18(r4)			# srr1 &= ~(1<<12)	# clear SE bit
	b finish

#******************************************************************
#					subroutine: version number
#				Sends back the current gecko version number.
#******************************************************************

versionnumber:
	li	r3, 0x80		# 0x80 = Wii, 0x81 = NGC
	bl	exisendbyte
	#b	finish

#******************************************************************
#				Finish
#		Check if the gecko has been paused. if no return to game
#******************************************************************

finish:
	lwz	r4, 0(FROZENVAL)
	cmpwi	r4, 0		# check to see if we have frozen the game
	bne	frozen			# loop around if we have
						# (changed to return for the bp)

resumegame:

	sth MP3PROT,0x4010(CCOO) # restore memory protection value

	lfd	f2,152(r1)		# loads f2
	lfd	f3,160(r1)		# loads f3

	mtmsr	MSR

	lwz	r0,172(r1)
	mtlr	r0			# restores lr

	lwz	r0,12(r1)
	mtcr	r0			# restores cr

	lwz	r0,16(r1)
	mtctr	r0			# restores ctr

	lwz	r0,20(r1)
	mtxer	r0			# restores xer

	lmw	r3,24(r1)		# restores r3-r31

	lwz	r0,8(r1)		# loads r0

	addi	r1,r1,168

	isync

	blr					# return back to game

#******************************************************************
#						Write branch
#	r3 - source (our mastercode location)
#	r4 - destination (lowmem area 0x80001800 address which will branch to
#	volatile: r17, r5
#******************************************************************
writebranch:

	subf	r17, r3, r4		# subtract r3 from r4 and place in r17
	lis	r5, 0x4800			# 0x48000000
	rlwimi	r5,r17,0,6,29
	stw	r5, 0(r3)			# result in r3

	dcbf	r0, r3			# data cache block flush
	sync
	icbi	r0, r3
	isync

	blr						# return



#******************************************************************
#						Packet Divide
#  Used by the down/up routines to calculate the number of packet to send
#******************************************************************

_packetdivide:
	divw.	r11, r16, r17		# fullpackets = memrange / 0xF80 (r11 is full packets)
	mullw	r10, r11, r17
	subf	r10, r10, r16		# r10 holds remainder byte counter
	stw	r10, 0(REMAINDER)		# store remainder
	blr

#==================================================================

frozenvalue:	#frozen value, then LR
.long	0,0
dwordbuffer:
.long	0,0
rem:
.long	0
bpbuffer:
.long 0		#inst address to bp on
.long 0		#data address to bp on
.long 0		#alignement check
.long 0		#counter for alignement / previous broken on address

regbuffer:
.space 72*4

state:
.long 0			# 0=none 1=linked 2=???

.align 4

codelist:
.space 2*4
.end

