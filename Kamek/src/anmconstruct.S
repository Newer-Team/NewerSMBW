.text
.align 4
.set var_38, -0x38
.set var_10, -0x10
.set arg_4,  4
#define pHeap r29
#define pSize r25
#define resAnm r30
#define bHasCache r27
.global AnmChrConstructWrapper
.extern Exception_Printf___Q24nw4r2dbFPCce
AnmChrConstructWrapper:
    stwu      r1, -0x40(r1)
    mflr      r0
    stw       r0, 0x40+arg_4(r1)
    addi      r11, r1, 0x40+var_10
    stfd      f31, 0x40+var_10(r1)
    psq_st  f31, 0x38(r1), 1, 0
    .extern _savegpr_25
    bl        _savegpr_25
    lwz       r8, 0(r5)
    mr        pHeap, r3
    mr        pSize, r4
    mr        resAnm, r5
    cmpwi     r8, 0
    mr        bHasCache, r7
    beq       resAnmInvalid

    lwz       r0, 0(r6)
    cmpwi     r0, 0
    bne       resMdlValid


resAnmInvalid:                                    # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+34j
	mr r7, r6  # resMdl
	mr r8, resAnm
	lwz r9, 0(r7)
	lwz r10, 0(r8)
	mr r6, bHasCache
	mr r5, r4
	mr r4, r3
	lis r3, resInvalid@h
	ori r3, r3, resInvalid@l
	crclr 4*cr1+eq
	bl Exception_Printf___Q24nw4r2dbFPCce
    li        r3, 0
    b         return

# ---------------------------------------------------------------------------

resMdlValid:                                      # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+40j
    lhz       r31, 0x22(r8)
    mr        r3, r6
    .extern GetResNodeNumEntries__Q34nw4r3g3d6ResMdlCFv
    bl        GetResNodeNumEntries__Q34nw4r3g3d6ResMdlCFv

    neg       r0, bHasCache
    mr        r28, r3
    or        r4, r0, bHasCache
    cmpwi     pSize, 0
    srawi     r4, r4, 0x1F
    slwi      r0, r3, 1
    and       r3, r31, r4
    mulli     r26, r3, 0x4C
    add       r3, r26, r0
    addi      r4, r3, 0x34
    beq       loc_80248B58

    stw       r4, 0(pSize)


loc_80248B58:                                     # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+80j
    cmpwi     pHeap, 0
    bne       loc_80248B68

	mr r7, resAnm
	lwz r8, 0(r7)
	mr r6, bHasCache
	mr r5, pSize
	mr r4, pHeap
	lis r3, noHeap@h
	ori r3, r3, noHeap@l
	crclr 4*cr1+eq
	bl Exception_Printf___Q24nw4r2dbFPCce

    li        r3, 0
    b         return

# ---------------------------------------------------------------------------

loc_80248B68:                                     # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+8Cj
    mr        r3, pHeap
    .extern MEMAllocFromAllocator
    bl        MEMAllocFromAllocator

    cmpwi     r3, 0
    mr        r31, r3
    bne       loc_80248B84

	mr r7, resAnm
	lwz r8, 0(r7)
	mr r6, bHasCache
	mr r5, pSize
	mr r4, pHeap
	lis r3, allocFail@h
	ori r3, r3, allocFail@l
	crclr 4*cr1+eq
	bl Exception_Printf___Q24nw4r2dbFPCce
    li        r3, 0
    b         return

# ---------------------------------------------------------------------------

loc_80248B84:                                     # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+A8j
    cmpwi     bHasCache, 0
    beq       loc_80248B94

    addi      bHasCache, r3, 0x34
    b         loc_80248B98

# ---------------------------------------------------------------------------

loc_80248B94:                                     # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+B8j
    li        bHasCache, 0


loc_80248B98:                                     # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+C0j
    cmpwi     r3, 0
    add       r4, r3, r26
    addi      r5, r4, 0x34
    beq       return_r31

    lwz       resAnm, 0(resAnm)
    li        r0, 0
    .extern __vt__Q34nw4r3g3d9AnmObjChr
    lis       r4, __vt__Q34nw4r3g3d9AnmObjChr@h
    ori       r4, r4, __vt__Q34nw4r3g3d9AnmObjChr@l
    stw       r0, 4(r3)
    stw       pHeap, 8(r3)
    stw       r0, 0xC(r3)
    stw       r4, 0(r3)
    stw       r28, 0x10(r3)
    stw       r5, 0x14(r3)
    mr        r3, r31
    lwz       r12, 0(r31)
    lwz       r12, 0x34(r12)
    mtctr     r12
    bctrl

    lis r9, BLAH@h
    ori r9, r9, BLAH@l
    lwz       r5, 0x24(resAnm)
    lis       r0, 0x4330
    lhz       r4, 0x20(resAnm)
    .extern __vt__Q34nw4r3g3d12AnmObjChrRes
    lis       r3, __vt__Q34nw4r3g3d12AnmObjChrRes@h
    slwi      r6, r5, 2
    subi      r5, r13, 0x5C88
    lwzx      r5, r5, r6
    xoris     r4, r4, 0x8000
    lfs       f1, 0(r9)
    ori       r3, r3, __vt__Q34nw4r3g3d12AnmObjChrRes@l
    stw       r4, 0x40+var_38+4(r1)
    cmpwi     bHasCache, 0
    lfs       f0, 4(r9)
    stw       r0, 0x40+var_38(r1)
    lfd       f3, 8(r9)
    stfs      f1, 0x18(r31)
    lfd       f2, 0x40+var_38(r1)
    stfs      f0, 0x1C(r31)
    fsubs     f0, f2, f3
    stfs      f1, 0x20(r31)
    stfs      f0, 0x24(r31)
    stw       r5, 0x28(r31)
    stw       r3, 0(r31)
    stw       resAnm, 0x2C(r31)
    stw       bHasCache, 0x30(r31)
    beq       return_r31

    lfs       f31, 0x18(r31)
    li        r28, 0
    li        bHasCache, 0
    b         loc_80248C90

# ---------------------------------------------------------------------------

loc_80248C5C:                                     # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+1C8j
    lwz       r3, 0x14(r31)
    lhzx      r3, r3, bHasCache
    rlwinm.   r0, r3, 0,16,16
    bne       loc_80248C88

    clrlwi    r5, r3, 18
    fmr       f1, f31
    mulli     r0, r5, 0x4C
    lwz       r4, 0x30(r31)
    addi      r3, r31, 0x2C
    add       r4, r4, r0
    .extern GetAnmResult__Q34nw4r3g3d9ResAnmChrCFPQ34nw4r3g3d12ChrAnmResultUlf
    bl        GetAnmResult__Q34nw4r3g3d9ResAnmChrCFPQ34nw4r3g3d12ChrAnmResultUlf


loc_80248C88:                                     # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+198j
    addi      bHasCache, bHasCache, 2
    addi      r28, r28, 1


loc_80248C90:                                     # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+188j
    lwz       r0, 0x10(r31)
    cmplw     r28, r0
    blt       loc_80248C5C


return_r31:                                       # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+D4j
                                                  # nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+178j
    mr r9, r31
	mr r7, resAnm
	lwz r8, 0(r7)
	mr r6, bHasCache
	mr r5, pSize
	mr r4, pHeap
	lis r3, final@h
	ori r3, r3, final@l
	crclr 4*cr1+eq
	bl Exception_Printf___Q24nw4r2dbFPCce
    mr        r3, r31


return:                                           # CODE XREF: nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+48j
                                                  # nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+94j
                                                  # nw4r::g3d::AnmObjChrRes::Construct((MEMAllocator *,ulong *,nw4r::g3d::ResAnmChr,nw4r::g3d::ResMdl,bool))+B0j
    addi      r11, r1, 0x40+var_10
    psq_l   f31, 0x38(r1), 1, 0
    lfd       f31, 0x40+var_10(r1)
    .extern _restgpr_25
    bl        _restgpr_25

    lwz       r0, 0x40+arg_4(r1)
    mtlr      r0
    addi      r1, r1, 0x40
    blr


.data
resInvalid: .string "RES INVALID:h=%p s=%p c=%d m=%p\na=%p mV=%p aV=%p\n"
noHeap: .string "NO HEAP:pHeap=%p pSize=%p bHasCache=%d\nresAnm=%p anmVal=%p\n"
allocFail: .string "NO HEAP:pHeap=%p pSize=%p bHasCache=%d\nresAnm=%p anmVal=%p\n"
final: .string "DONE:pHeap=%p pSize=%p bHasCache=%d resAnm=%p anmVal=%p output=%p\n"
.align 8
BLAH:
.float 0.0, 1.0
.long 0x43300000
.long 0x80000000
