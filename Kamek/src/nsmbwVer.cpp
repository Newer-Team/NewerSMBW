#include "nsmbwVer.h"

NSMBWVer getNsmbwVer()
{	
	u32 checkVer = *((u32*)0x800CF6CC);
	u32 checkKrTw;
	
	switch(checkVer)
	{
		case 0x40820030:
			return pal;
			break;
		case 0x40820038:
			return pal2;
			break;
		case 0x48000465:
			return ntsc;
			break;
		case 0x2c030000:
			return ntsc2;
			break;
		case 0x480000b4:
			return jpn;
			break;
		case 0x4082000c:
			return jpn2;
			break;
		case 0x38a00001:
			checkKrTw = *((u32*)0x80004238);
			if(checkKrTw == 0x6021c8e0) return kor;
			if(checkKrTw == 0x6021ace0) return twn;
			break;
	}
	return pal; // To appease the compiler warning gods
}