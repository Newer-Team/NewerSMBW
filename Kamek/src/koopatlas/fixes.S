.text
#include <asm_setup.S>
.global ClearLastPowerupStoreTypeAfterGameOver
.extern LastPowerupStoreType
ClearLastPowerupStoreTypeAfterGameOver:
	lis r4, LastPowerupStoreType@h
	ori r4, r4, LastPowerupStoreType@l
	li r5, 0
	stw r5, 0(r4)
	blr

