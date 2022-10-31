.extern ts1table
.extern ts2table
.extern ts3table
.extern Pa1RandomTiles
.extern Pa2RandomTiles
.extern Pa3RandomTiles

.text
.align 4

# 0x80083C1C
.global ReplacePa1Table
ReplacePa1Table:
	lis r29, ts1table@ha
	addi r29, r29, ts1table@l
	blr

# 0x80083C64
.global ReplacePa2Table
ReplacePa2Table:
	lis r29, ts2table@ha
	addi r29, r29, ts2table@l
	blr

# 0x80083CAC
.global ReplacePa3Table
ReplacePa3Table:
	lis r30, ts3table@ha
	addi r30, r30, ts3table@l
	blr

# 0x80086B90
.global ReplacePa1PTMFs
ReplacePa1PTMFs:
	lis r3, Pa1RandomTiles@ha
	li r31, 1	# tileset slot = 1
	addi r3, r3, Pa1RandomTiles@l
	blr

# 0x80086BD4
.global ReplacePa2PTMFs
ReplacePa2PTMFs:
	lis r3, Pa2RandomTiles@ha
	li r31, 2	# tileset slot = 2
	addi r3, r3, Pa2RandomTiles@l
	blr

# 0x80086C18
.global ReplacePa3PTMFs
ReplacePa3PTMFs:
	lis r3, Pa3RandomTiles@ha
	li r31, 3	# tileset slot = 3
	addi r3, r3, Pa3RandomTiles@l
	blr