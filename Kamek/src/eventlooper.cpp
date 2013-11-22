#include <common.h>
#include <game.h>

struct EventLooper {
	u32 id;			// 0x00
	u32 settings;	// 0x04
	u16 name;		// 0x08
	u8 _0A[6];		// 0x0A
	u8 _10[0x9C];	// 0x10
	float x;		// 0xAC
	float y;		// 0xB0
	float z;		// 0xB4
	u8 _B8[0x318];	// 0xB8
	// Any variables you add to the class go here; starting at offset 0x3D0
	u64 eventFlag;	// 0x3D0
	u64 eventActive;	// 0x3D0
	u8 delay;		// 0x3D4
	u8 delayCount;	// 0x3D7
};

void EventLooper_Update(EventLooper *self);



bool EventLooper_Create(EventLooper *self) {
	char eventStart	= (self->settings >> 24)	& 0xFF;
	char eventEnd	= (self->settings >> 16)	& 0xFF;

	// Putting all the events into the flag
	int i;
	u64 q = (u64)0;
	for(i=eventStart;i<(eventEnd+1);i++)
	{
		q = q | ((u64)1 << (i - 1));
	}
		
	self->eventFlag = q;
	
	self->delay		= (((self->settings) & 0xFF) + 1) * 10;
	self->delayCount = 0;
	
	char tmpEvent= (self->settings >> 8)	& 0xFF;
	if (tmpEvent == 0)
	{
		self->eventActive = (u64)0xFFFFFFFFFFFFFFFF;
	}
	else
	{
		self->eventActive = (u64)1 << (tmpEvent - 1);
		
	}
	

	if (dFlagMgr_c::instance->flags & self->eventActive)
	{
		u64 evState = (u64)1 << (eventStart - 1);
		dFlagMgr_c::instance->flags |= evState;
	}

	EventLooper_Update(self);
	
	return true;
}

bool EventLooper_Execute(EventLooper *self) {
	EventLooper_Update(self);
	return true;
}


void EventLooper_Update(EventLooper *self) {
	
	if ((dFlagMgr_c::instance->flags & self->eventActive) == 0)
		return;

	// Waiting for the right moment
	if (self->delayCount < self->delay) 
	{

		self->delayCount = self->delayCount + 1;
		return;
	}	
	
	// Reset the delay
	self->delayCount = 0;
	
	// Find which event(s) is/are on
	u64 evState = dFlagMgr_c::instance->flags & self->eventFlag;
	
	// Turn off the old events
	dFlagMgr_c::instance->flags = dFlagMgr_c::instance->flags & (~self->eventFlag);
	
	// Shift them right if they can, if not, reset!
	evState = evState << 1;
	if (evState < self->eventFlag)
	{
		dFlagMgr_c::instance->flags = dFlagMgr_c::instance->flags | evState;
	}
	
	else
	{
		char eventStart	= (self->settings >> 24)	& 0xFF;
		evState = (u64)1 << (eventStart - 1);
		dFlagMgr_c::instance->flags = dFlagMgr_c::instance->flags | evState;
	}
	
	
}
