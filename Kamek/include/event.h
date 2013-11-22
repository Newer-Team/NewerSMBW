#ifndef __KAMEK_EVENT_H
#define __KAMEK_EVENT_H

#include <common.h>

#ifdef REGION_PAL
	#define EVENT_TABLE (*((void**)0x8042A358))
#endif

#ifdef REGION_NTSC
	// ...
#endif

void TriggerEventFlag(void *eventstruct, u8 event, u8 delay, u8 method, s32 unk3, u8 expiry, s32 unk5);

#endif
