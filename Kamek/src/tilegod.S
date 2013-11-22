.text

#include <asm_setup.S>

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


