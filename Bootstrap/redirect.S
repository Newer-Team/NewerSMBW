#ifndef __MWERKS__
.set r0,0;   .set r1,1;   .set r2,2; .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31; .set f0,0; .set f2,2; .set f3,3
.set f1,1;   .set f30,30; .set f31,31
#endif

.set sp,1


.global JumpToBootstrap
JumpToBootstrap:
	b start

################################################################################
# Region Addresses
################################################################################

# Using offset 0x80768D50 for detection (0x807689BE for KR and TW versions).

# PAL v1: 4B9F6DE1
# NTSC v1: 4B9F6CA1
# NTSC v2: 80010014

#define ADDR_OSReport					0x00
#define ADDR_OSFatal					0x04
#define ADDR_DVDConvertPathToEntrynum	0x08
#define ADDR_DVDFastOpen				0x0C
#define ADDR_DVDReadPrio				0x10
#define ADDR_DVDClose					0x14
#define ADDR_EGGHeapAlloc				0x18
#define ADDR_EGGHeapFree				0x1C
#define ADDR_GameHeapPtr				0x20
#define ADDR_ArchiveHeapPtr				0x24
#define ADDR_sprintf					0x28
#define ADDR_rch1						0x2C
#define ADDR_rch2						0x2D

#define ADDR r31
#define FILE r30
#define DLCODE r29


Addresses_PAL:
	.long 0x8015F870		# OSReport
	.long 0x801AF710		# OSFatal
	.long 0x801CA7C0		# DVDConvertPathToEntrynum
	.long 0x801CAAD0		# DVDFastOpen
	.long 0x801CAC60		# DVDReadPrio
	.long 0x801CAB40		# DVDClose
	.long 0x802B8E00		# EGGHeapAlloc
	.long 0x802B90B0		# EGGHeapFree
	.long 0x80377F48		# GameHeapPtr
	.long 0x8042A72C		# ArchiveHeapPtr
	.long 0x802E1ACC		# sprintf
	.byte 'E','U',0,0

Addresses_NTSC:
	.long 0x8015F730		# OSReport
	.long 0x801AF5D0		# OSFatal
	.long 0x801CA680		# DVDConvertPathToEntrynum
	.long 0x801CA990		# DVDFastOpen
	.long 0x801CAB20		# DVDReadPrio
	.long 0x801CAA00		# DVDClose
	.long 0x802B8CC0		# EGGHeapAlloc
	.long 0x802B8F70		# EGGHeapFree
	.long 0x80377C48		# GameHeapPtr
	.long 0x8042A44C		# ArchiveHeapPtr
	.long 0x802E17DC		# sprintf
	.byte 'U','S',0,0

Addresses_JP:
	.long 0x8015F540		# OSReport
	.long 0x801AF3E0		# OSFatal
	.long 0x801CA490		# DVDConvertPathToEntrynum
	.long 0x801CA7A0		# DVDFastOpen
	.long 0x801CA930		# DVDReadPrio
	.long 0x801CA810		# DVDClose
	.long 0x802B8AD0		# EGGHeapAlloc
	.long 0x802B8D80		# EGGHeapFree
	.long 0x803779C8		# GameHeapPtr
	.long 0x8042A16C		# ArchiveHeapPtr
	.long 0x802E15EC		# sprintf
	.byte 'J','P',0,0

Addresses_KR:
	.long 0x8015FC70		# OSReport
	.long 0x801AFB10		# OSFatal
	.long 0x801CABC0		# DVDConvertPathToEntrynum
	.long 0x801CAED0		# DVDFastOpen
	.long 0x801CB060		# DVDReadPrio
	.long 0x801CAF40		# DVDClose
	.long 0x802B9200		# EGGHeapAlloc
	.long 0x802B94B0		# EGGHeapFree
	.long 0x80384948		# GameHeapPtr
	.long 0x804370EC		# ArchiveHeapPtr
	.long 0x802E1D1C		# sprintf
	.byte 'K','R',0,0

Addresses_TW:
	.long 0x8015FC70		# OSReport
	.long 0x801AFB10		# OSFatal
	.long 0x801CABC0		# DVDConvertPathToEntrynum
	.long 0x801CAED0		# DVDFastOpen
	.long 0x801CB060		# DVDReadPrio
	.long 0x801CAF40		# DVDClose
	.long 0x802B9200		# EGGHeapAlloc
	.long 0x802B94B0		# EGGHeapFree
	.long 0x80382D48		# GameHeapPtr
	.long 0x804354EC		# ArchiveHeapPtr
	.long 0x802E1D1C		# sprintf
	.byte 'T','W',0,0

PatchFile:	.string "/NewerRes/Systemxx_x.bin"
DLCode:		.string "/NewerRes/DLCodexx_x.bin"
DLRelocs:	.string "/NewerRes/DLRelocsxx_x.bin"

.align 4

################################################################################
# Code Start
################################################################################

start:
	# Make a stack frame
	stwu sp, -0x60(sp)
	mflr r0
	stw r0, 0x64(sp)
	stw ADDR, 0x5C(sp)
	stw FILE, 0x58(sp)
	stw DLCODE, 0x54(sp)

	# Detect the region and version
	li r6, '1' # version

	lis r5, 0x8076
	ori r5, r5, 0x8D50
	lwz r3, 0(r5)
	clrlwi r3, r3, 16

	cmpwi r3, 0x6DE1
	beq is_PAL

	cmpwi r3, 0x6CA1
	beq is_NTSC

	cmpwi r3, 0x6AB1
	beq is_JPN

	#cmpwi r3, 0x0014 # i don't understand this check; TODO ask ninji
	#bne ERROR_UnknownRegion
    
    # v2
	li r6, '2' # version

	lwz r3, 0x40(r5)
	clrlwi r3, r3, 16
	
	cmpwi r3, 0x6DA1
	beq is_PAL

	cmpwi r3, 0x6C61
	beq is_NTSC

	cmpwi r3, 0x6A71
	beq is_JPN

	# v3
	li r6, '3' # version (Just Korea)
	
	cmpwi r3, 0x71E1 # This value is the same in both Korean and Taiwanese regions
	lwz r3, -0x394(r5) # so we gotta check another address (0x807689BE)
	clrlwi r3, r3, 16
	cmpwi r3, 0x7128
	beq is_KOR
	
	# v4
	li r6, '4' # version (Just Taiwan/Hong Kong)
	
	cmpwi r3, 0x5528
	beq is_TWN

	b ERROR_UnknownRegion

is_PAL:
	lis ADDR, Addresses_PAL@h
	ori ADDR, ADDR, Addresses_PAL@l
	b got_address

is_NTSC:
	lis ADDR, Addresses_NTSC@h
	ori ADDR, ADDR, Addresses_NTSC@l
	b got_address

is_JPN:
	lis ADDR, Addresses_JP@h
	ori ADDR, ADDR, Addresses_JP@l
	b got_address

is_KOR:
	lis ADDR, Addresses_KR@h
	ori ADDR, ADDR, Addresses_KR@l
	b got_address

is_TWN:
	lis ADDR, Addresses_TW@h
	ori ADDR, ADDR, Addresses_TW@l


got_address:
	# fix up the filenames
	lis r3, PatchFile@h
	ori r3, r3, PatchFile@l
	li r4, 16
	bl FixAddr

	lis r3, DLCode@h
	ori r3, r3, DLCode@l
	bl FixAddr

	lis r3, DLRelocs@h
	ori r3, r3, DLRelocs@l
	li r4, 18
	bl FixAddr

	# Now that we have the region, set up stuff we'll need!
	lis r3, MSG_LoadStart@h
	ori r3, r3, MSG_LoadStart@l
	lis r4, PatchFile@h
	ori r4, r4, PatchFile@l
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl

	lis r3, PatchFile@h
	ori r3, r3, PatchFile@l
	lwz r12, ADDR_DVDConvertPathToEntrynum(ADDR)
	mtctr r12
	bctrl

	li r4, -1
	cmpw r3, r4
	beq ERROR_MissingSysFile

	# Cool, got the entry num
	# r3 is already the entrynum so no issue there
	addi r4, sp, 8
	lwz r12, ADDR_DVDFastOpen(ADDR)
	mtctr r12
	bctrl

	li r4, 1
	cmpw r3, r4
	bne ERROR_OpenFailed

	# More info
	lis r3, MSG_FileOpened@h
	ori r3, r3, MSG_FileOpened@l
	lwz r4, 0x3C(sp)
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl

	# Now align the length and allocate some memory
	lwz r3, 0x3C(sp)
	addi r3, r3, 0x1F
	clrrwi r3, r3, 5
	li r4, 0x20
	lwz r5, ADDR_ArchiveHeapPtr(ADDR)
	lwz r5, 0(r5)
	lwz r12, ADDR_EGGHeapAlloc(ADDR)
	mtctr r12
	bctrl

	mr FILE, r3

	cmpwi FILE, 0
	beq ERROR_AllocFailed

	# Read the file
	addi r3, sp, 8
	mr r4, FILE
	lwz r5, 0x3C(sp)
	addi r5, r5, 0x1F
	clrrwi r5, r5, 5
	li r6, 0
	li r7, 2
	lwz r12, ADDR_DVDReadPrio(ADDR)
	mtctr r12
	bctrl

	lis r3, MSG_ReadDone@h
	ori r3, r3, MSG_ReadDone@l
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl

	mr r3, FILE
	bl ApplyKamekPatch

	lis r3, MSG_PatchApplied@h
	ori r3, r3, MSG_PatchApplied@l
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl

	# Free the memory and close the file handle
	mr r3, FILE
	lwz r4, ADDR_ArchiveHeapPtr(ADDR)
	lwz r4, 0(r4)
	lwz r12, ADDR_EGGHeapFree(ADDR)
	mtctr r12
	bctrl

	addi r3, sp, 8
	lwz r12, ADDR_DVDClose(ADDR)
	mtctr r12
	bctrl



	# Next, load dynamic code
	lis r3, MSG_LoadingDLCode@h
	ori r3, r3, MSG_LoadingDLCode@l
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl

	lis r3, DLCode@h
	ori r3, r3, DLCode@l
	lwz r12, ADDR_DVDConvertPathToEntrynum(ADDR)
	mtctr r12
	bctrl

	addi r4, sp, 8
	lwz r12, ADDR_DVDFastOpen(ADDR)
	mtctr r12
	bctrl

	# allocate a buffer
	lwz r3, 0x3C(sp)
	addi r3, r3, 0x1F
	clrrwi r3, r3, 5
	li r4, 0x20
	lwz r5, ADDR_GameHeapPtr(ADDR)
	lwz r5, 0(r5)
	lwz r12, ADDR_EGGHeapAlloc(ADDR)
	mtctr r12
	bctrl

	mr DLCODE, r3

	# read the thing in
	addi r3, sp, 8
	mr r4, DLCODE
	lwz r5, 0x3C(sp)
	addi r5, r5, 0x1F
	clrrwi r5, r5, 5
	li r6, 0
	li r7, 2
	lwz r12, ADDR_DVDReadPrio(ADDR)
	mtctr r12
	bctrl

	# close the handle
	addi r3, sp, 8
	lwz r12, ADDR_DVDClose(ADDR)
	mtctr r12
	bctrl

	# inform us
	lis r3, MSG_DLCodeInfo@h
	ori r3, r3, MSG_DLCodeInfo@l
	mr r4, DLCODE
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl

	# and finally, load + link relocs
	lis r3, DLRelocs@h
	ori r3, r3, DLRelocs@l
	lwz r12, ADDR_DVDConvertPathToEntrynum(ADDR)
	mtctr r12
	bctrl

	addi r4, sp, 8
	lwz r12, ADDR_DVDFastOpen(ADDR)
	mtctr r12
	bctrl

	lwz r3, 0x3C(sp)
	addi r3, r3, 0x1F
	clrrwi r3, r3, 5
	li r4, 0x20
	lwz r5, ADDR_ArchiveHeapPtr(ADDR)
	lwz r5, 0(r5)
	lwz r12, ADDR_EGGHeapAlloc(ADDR)
	mtctr r12
	bctrl

	mr FILE, r3

	lis r3, MSG_LoadingDLCode@h
	ori r3, r3, MSG_LoadingDLCode@l
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl

	addi r3, sp, 8
	mr r4, FILE
	lwz r5, 0x3C(sp)
	addi r5, r5, 0x1F
	clrrwi r5, r5, 5
	li r6, 0
	li r7, 2
	lwz r12, ADDR_DVDReadPrio(ADDR)
	mtctr r12
	bctrl

	lis r3, MSG_Linking@h
	ori r3, r3, MSG_Linking@l
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl

	mr r3, DLCODE
	mr r4, FILE
	bl LinkNewerRel

	mr r3, FILE
	lwz r4, ADDR_ArchiveHeapPtr(ADDR)
	lwz r4, 0(r4)
	lwz r12, ADDR_EGGHeapFree(ADDR)
	mtctr r12
	bctrl

	addi r3, sp, 8
	lwz r12, ADDR_DVDClose(ADDR)
	mtctr r12
	bctrl

	lis r3, MSG_Done@h
	ori r3, r3, MSG_Done@l
	lwz r12, ADDR_OSReport(ADDR)
	mtctr r12
	bctrl


	b return



ERROR_UnknownRegion:
	# can't do much, I guess
	b ERROR_UnknownRegion


ERROR_MissingSysFile:
	lis r3, ErrorBuffer@h
	ori r3, r3, ErrorBuffer@l
	lis r4, MSG_MissingSysFile@h
	ori r4, r4, MSG_MissingSysFile@l
	lis r5, PatchFile@h
	ori r5, r5, PatchFile@l
	lis r6, 0x8000
	lbz r7, 3(r6)
	lbz r6, 7(r6)

	lwz r12, ADDR_sprintf(ADDR)
	mtctr r12
	bctrl

	lis r3, ErrorBuffer@h
	ori r3, r3, ErrorBuffer@l
	b show_error
ERROR_OpenFailed:
	lis r3, MSG_OpenFailed@h
	ori r3, r3, MSG_OpenFailed@l
	b show_error
ERROR_AllocFailed:
	lis r3, MSG_AllocFailed@h
	ori r3, r3, MSG_AllocFailed@l


show_error:
	mr r5, r3
	lis r3, ErrorFG@h
	ori r3, r3, ErrorFG@l
	lis r4, ErrorBG@h
	ori r4, r4, ErrorBG@l
	lwz r12, ADDR_OSFatal(ADDR)
	mtctr r12
	bctrl

return:
	li r3, 1

	lwz ADDR, 0x5C(sp)
	lwz FILE, 0x58(sp)
	lwz DLCODE, 0x54(sp)
	lwz r0, 0x64(sp)
	mtlr r0
	addi sp, sp, 0x60
	blr


FixAddr:
	add r3, r3, r4
	lbz r5, ADDR_rch1(ADDR)
	stb r5, 0(r3)
	lbz r5, ADDR_rch2(ADDR)
	stb r5, 1(r3)
	stb r6, 3(r3)
	blr


ErrorFG: .long 0xFFFFFFFF
ErrorBG: .long 0x00000000

MSG_LoadStart:		.string "1: Loading system file: %s\n"
MSG_FileOpened:		.string "2: File successfully opened. %d bytes. Reading...\n"
MSG_ReadDone:		.string "3: File read. Now patching\n"
MSG_PatchApplied:	.string "4: Patch applied\n"
MSG_LoadingDLCode:	.string "5: Loading dylink code\n"
MSG_DLCodeInfo:		.string "DLCode @ %p\n"
MSG_LoadingRelocs:	.string "6: Loading relocs\n"
MSG_Linking:		.string "7: Linking\n"
MSG_Done:			.string "8: Hacks loaded!\n"

#MSG_MissingSysFile:	.string "FATAL ERROR: System file not found on the disc.\nSomething's gone wrong.\nMissing: %s\nDisc version: %d\nRegion: %c"
MSG_OpenFailed:		.string "FATAL ERROR: System file could not be opened."
MSG_AllocFailed:	.string "FATAL ERROR: Failed to allocate memory (%d)."
MSG_MissingSysFile:	.string "NEWER Super Mario Bros. Wii\n\nSomething went wrong while installing.\nThe following file is required, but can't be found on the SD card or USB drive:\n/NewerSMBW%s\n\nFor troubleshooting and installation\ninstructions, see:\n    http://newerteam.com/help.\n\nDisc version: %d\nGame region: %c"


ErrorBuffer: # 128 bytes
	.long 0, 0, 0, 0, 0, 0, 0, 0
	.long 0, 0, 0, 0, 0, 0, 0, 0
	.long 0, 0, 0, 0, 0, 0, 0, 0
	.long 0, 0, 0, 0, 0, 0, 0, 0

.align 4
