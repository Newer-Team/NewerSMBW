#include <actors.h>
#include "nsmbwVer.h"

// Get the appropriate ID based on game version
Actors translateActorID(Actors id)
{
	NSMBWVer version = getNsmbwVer();
	switch(version)
	{
		case pal:
		case pal2:
		case ntsc:
		case ntsc2:
		case jpn:
		case jpn2:
		default: // unknown
			return adjustID(id);
			break;
		case kor:
		case twn:
			return id;
			break;
	}
}

Actors adjustID(Actors id)
{
	if(id > 703)
		return Actors(id - 2);
	if(id == 702 || id == 703) // wtf
		return Actors(id + 2);
	
	return id; // Actor is below id 702
}
