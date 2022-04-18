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

.extern Global5758
.global MessageBoxIsShowing

# -if param & 1 /and/ messagebox is on, 
# otherwise, return Global5758 & param

.global Query5758Replacement
Query5758Replacement:
	mr 5, 3					# save param
	lis 4, Global5758@h
	ori 4, 4, Global5758@l
	lwz 0, 0(4)				# get 5758
	and 3, 0, 3				# AND with param
	andi. 5, 5, 1			# if param & 1 == 0..
	beqlr					# then return
	
	lis 4, MessageBoxIsShowing@h
	ori 4, 4, MessageBoxIsShowing@l
	lwz 4, 0(4)
	or 3, 3, 4				# otherwise, OR with MessageBoxIsShowing
	blr						# then return


.data
MessageBoxIsShowing:
	.long 0
