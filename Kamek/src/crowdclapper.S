.extern OneofManySoundPlayers
.extern EventTable

#include <asm_setup.S>

.text
.align 4

.global CrowdClapperEventActivate
CrowdClapperEventActivate:

	mflr r0
	bl OneofManySoundPlayers
	mtlr r0
	
	lis r8, EventTable@h
	ori r8, r8, EventTable@l

	lwz r11, 4(r8)

	lwz r9, 4(r31)
	srwi r9, r9, 24
	
	subi r9, r9, 1
	
	li r10, 1
	slw r10, r10, r9
	
	or r11, r11, r10
	stw r11, 0(r8)
	blr
