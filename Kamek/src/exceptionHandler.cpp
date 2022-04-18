#include "game.h"
#include "fileload.h"
#include "nsmbwVer.h"

#if defined(ANOTHER_WII)
#define GAME_NAME "Another 2.10"
#elif defined(HOLIDAY_SPECIAL)
#define GAME_NAME "NerXMAS v1.10"
#elif defined(SUMMER_SUN)
#define GAME_NAME "SumSun v1.10"
#elif defined(FALLING_LEAF)
#define GAME_NAME "Newer FL v1.10"
#elif defined(NEWER_WII)
#define GAME_NAME "Newer Wii v1.30"
#else
#define GAME_NAME "The game"
#endif

const bool dsisrFun = false;
const bool gprFun = false;

u32 srr0;
const u32 dlcode = 0x80DFBCC0;

struct OSContext
{
    u32 gpr[32];
    u32 cr;
    u32 lr;
    u32 ctr;
    u32 xer;
    double fpr[32];
    u8 unknown0[8];
    u32 srr[2];
    u8 unknown1[2];
    u16 flags;
    u32 gqr[8];
    u8 unknown2[4];
    float fppr[32];
};

/*inline bool GetBit(u32 num, char m)
{
    char n = 31 - m;
    return ((num >> n) & 1);
}
void ShowMainInfo(void * _osContext, u32 dsisr, u32 dar)
{
    void* osContext = _osContext;
    //osContext += 0x198;
    osContext = (void *)(((char *)osContext) + 0x198);
    srr0 = *(u32 *)(osContext);
    //osContext += 0x4;
    osContext = (void *)(((char *)osContext) + 0x4);
    u32 srr1 = *(u32 *)(osContext);

    nw4r::db::Exception_Printf_("SRR0:    %08X\n\n", srr0);

    if (srr1Fun)
    {
        //SRR1 fun.
        //Read 16-31
        char * enabled = "enabled.";
        char * disabled = "disabled.";
        nw4r::db::Exception_Printf_("External interrupt %s\n", (GetBit(srr1, 16) ? enabled : disabled));
        nw4r::db::Exception_Printf_("Privilege level: %s\n", (GetBit(srr1, 17) ? "user" : "supervisor"));
        nw4r::db::Exception_Printf_("Floating-point %s\n", (GetBit(srr1, 18) ? enabled : disabled));
        nw4r::db::Exception_Printf_("Machine check %s\n", (GetBit(srr1, 19) ? enabled : disabled));
        //
        nw4r::db::Exception_Printf_("Single-step trace %s\n", (GetBit(srr1, 21) ? enabled : disabled));
        nw4r::db::Exception_Printf_("Branch trace %s\n", (GetBit(srr1, 22) ? enabled : disabled));
        //
        //
        nw4r::db::Exception_Printf_("Exception prefix: %s\n", (GetBit(srr1, 25) ? "0xFFF....." : "0x000....."));
        nw4r::db::Exception_Printf_("Instruction address translation %s\n", (GetBit(srr1, 26) ? enabled : disabled));
        nw4r::db::Exception_Printf_("Data address translation %s\n", (GetBit(srr1, 27) ? enabled : disabled));
        //
        nw4r::db::Exception_Printf_("Process is %s (performance)\n", (GetBit(srr1, 29) ? "marked" : "not marked"));
        nw4r::db::Exception_Printf_("Exception is %s\n", (GetBit(srr1, 30) ? "recoverable" : "not recoverable"));
        nw4r::db::Exception_Printf_("%s endian mode\n\n", (GetBit(srr1, 31) ? "Little" : "Big"));
    }

    if (dsisrFun)
    {
        //DSISR fun.
        if (GetBit(dsisr, 0)) nw4r::db::Exception_Printf_("Crashed by load/store instruction\n");
        if (GetBit(dsisr, 1)) nw4r::db::Exception_Printf_("Attempted access translation not found\n");
        if (GetBit(dsisr, 4)) nw4r::db::Exception_Printf_("A memory access in not permitted\n");
        if (GetBit(dsisr, 5)) nw4r::db::Exception_Printf_("Store/load instruction performed at a bad address\n");
        nw4r::db::Exception_Printf_("Crashed by %s instruction\n", (GetBit(dsisr, 6) ? "store" : "load"));
        if (GetBit(dsisr, 9)) nw4r::db::Exception_Printf_("DABR match occurs\n");
        if (GetBit(dsisr, 11)) nw4r::db::Exception_Printf_("Instruction is eciwx or ecowx\n");
        nw4r::db::Exception_Printf_("\n");
    }
}*/
char *GetRegionAndVersion()
{
	NSMBWVer version = getNsmbwVer();
	switch(version)
	{
		case pal:
			return "PALv1";
		case pal2:
			return "PALv2";
		case ntsc:
			return "NTSCv1";
		case ntsc2:
			return "NTSCv2";
		case jpn:
			return "JPNv1";
		case jpn2:
			return "JPNv2";
		case kor:
			return "kor";
		case twn:
			return "twn";
		default:
			return "UNKNOWN";
	}
}
char *GetErrorDescription(u16 OSError)
{
    char *desc[] =
        {
            "SYSTEM RESET",
            "MACHINE CHECK",
            "DSI", "ISI",
            "EXTERNAL INTERRUPT",
            "ALIGNMENT",
            "PROGRAM",
            "FLOATING POINT",
            "DECREMENTER",
            "SYSTEM CALL",
            "TRACE",
            "PERFORMANCE MONITOR",
            "BREAKPOINT",
            "SYSTEM INTERRUPT",
            "THERMAL INTERRUPT",
            "PROTECTION",
            "FPE"};
    return desc[OSError];
}
void PrintContext(u16 OSError, void *_osContext, u32 _dsisr, u32 _dar)
{
    OSContext *osContext = (OSContext *)_osContext;
	
    nw4r::db::Exception_Printf_("Whoops! " GAME_NAME " [%s] has crashed - %s\n\nPlease send the information below to\nnewer@newerteam.com\nYou can scroll through this report using the D-Pad.\n\n", GetRegionAndVersion(), GetErrorDescription(OSError));
    nw4r::db::Exception_Printf_("SRR0: %08X | DSISR: %08X | DAR: %08X\n", osContext->srr[0]);

    if (gprFun)
    {
        int i = 0;
        do
        {
            nw4r::db::Exception_Printf_("R%02d:%08X  R%02d:%08X  R%02d:%08X\n", i, osContext->gpr[i], i + 0xB, osContext->gpr[i + 0xB], i + 0x16, osContext->gpr[i + 0x16]);
            i++;
        } while (i < 10);
        nw4r::db::Exception_Printf_("R%02d:%08X  R%02d:%08X\n\n", 10, osContext->gpr[10], 0x15, osContext->gpr[0x15]);
    }

    //Stack trace
    //nw4r::db::Exception_Printf_("\nException info trace (most recent on top):\n");
    int i = 0;
    u32 *stackPointer = (u32 *)((char *)nw4r::db::sException + 0x33C);
    do
    {
        if (stackPointer == 0 || (u32)stackPointer == 0xFFFFFFFF)
            break;

        nw4r::db::Exception_Printf_("%08X", stackPointer[1]);
        if (stackPointer[1] >= dlcode)
        {
            nw4r::db::Exception_Printf_(" - %08X NewerASM", stackPointer[1] - dlcode);
        }
        nw4r::db::Exception_Printf_("\n");

        i++;
        stackPointer = (u32 *)*stackPointer;
    } while (i < 0x10);
    //while (true);
}