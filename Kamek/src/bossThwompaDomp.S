.text
#include <asm_setup.S>

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
	
