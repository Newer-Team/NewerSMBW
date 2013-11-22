.text

#include <asm_setup.S>

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
