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

.align 4
.extern ExitFromTileGodHack
.extern TileTable

.global NEW_GetTileFromTileTable
NEW_GetTileFromTileTable:
	andi. r0, r3, 0x8000
	beq UseRegular
	
	li r4, 0x7FFF
	and r3, r3, r4
	blr
	
UseRegular:
	lis r4, TileTable@h
	ori r4, r4, TileTable@l
	
	clrlslwi r0, r3, 16, 1
	lhzx r3, r4, r0
	blr


.global TileGodHack
TileGodHack:
	lwz r4, 4(r3)          # Load settings
	extrwi. r0, r4, 1, 16  # Get and check the "use regular tile" flag
	extrwi r4, r4, 3, 17   # Get the tile number OR tileset number
	bne Special
	
	lis r3, TileGodTiles@h
	ori r3, r3, TileGodTiles@l
	slwi r4, r4, 1
	lhzx r25, r3, r4
	
	b ExitFromTileGodHack
	
Special:
	lwz r3, 4(r3)          # Load settings
	srwi r3, r3, 20        # Get tile number
	andi. r3, r3, 0xFF     # Cut off top nybble
	slwi r4, r4, 8         # Shift tileset number
	or r3, r3, r4         # Add tileset number
	ori r25, r3, 0x8000    # Add "get regular tile" flag
	
	b ExitFromTileGodHack



.data
TileGodTiles:
	.short 0x7C, 0x02, 0x0C, 0x7B, 0x0F, 0x00


