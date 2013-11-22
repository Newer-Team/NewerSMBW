.text
#include <asm_setup.S>

.global ReturnNone
ReturnNone:
	blr

.global ReturnTrue
ReturnTrue:
	li r3, 1
	blr

.global ReturnFalse
ReturnFalse:
	li r3, 0
	blr

