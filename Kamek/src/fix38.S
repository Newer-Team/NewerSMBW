#include <asm_setup.S>

.extern continueFromFlagObjCheck
.extern returnFromFlagObjCheck

.global fix38WithJumpCoin

.text

fix38WithJumpCoin:
	# hacky fix because I can't figure out how to do it the right way
	
	# check actor type for EN_COIN_JUMP
	lhz r0, 8(r3)
	cmplwi r0, 403
	beq isExemptedActor
	# and EN_COIN
	cmplwi r0, 401
	beq isExemptedActor
	# and EN_HATENA_BALLOON
	cmplwi r0, 221
	beq isExemptedActor
	
	# it wasn't that, so just return 0
	li r3, 0
	b returnFromFlagObjCheck

isExemptedActor:
	b continueFromFlagObjCheck

.global PreventW5Vine
PreventW5Vine:
	li r3, 1
	blr
