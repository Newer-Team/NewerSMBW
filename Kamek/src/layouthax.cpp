#include <game.h>

void LoadPregameStyleNameAndNumber(m2d::EmbedLayout_c *layout);
extern "C" void InsertPauseWindowText(void *thing) {
	m2d::EmbedLayout_c *el = (m2d::EmbedLayout_c*)(((u8*)thing)+0x70);
	LoadPregameStyleNameAndNumber(el);
}

