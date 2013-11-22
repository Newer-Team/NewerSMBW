#ifndef TEXMAPCOLOURISER_H
#define TEXMAPCOLOURISER_H 
#include <game.h>

// Colourises an IA8 texture
class dTexMapColouriser_c {
	public:
		dTexMapColouriser_c();
		~dTexMapColouriser_c();

		void resetAndClear();
		void setTexMap(nw4r::lyt::TexMap *tm);
		void applyAlso(nw4r::lyt::TexMap *tm);
		void colourise(int h, int s, int l);

	private:
		nw4r::lyt::TexMap *texmap;
		u16 *original;
		u16 *mine;
};

#endif /* TEXMAPCOLOURISER_H */
