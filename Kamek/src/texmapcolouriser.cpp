#include "texmapcolouriser.h"

dTexMapColouriser_c::dTexMapColouriser_c() {
	texmap = 0;
	original = 0;
	mine = 0;
}

void *EGG__Heap__alloc(unsigned long size, int unk, void *heap);
void EGG__Heap__free(void *ptr, void *heap);

dTexMapColouriser_c::~dTexMapColouriser_c() {
	resetAndClear();
}

void dTexMapColouriser_c::resetAndClear() {
	texmap = 0;
	if (mine) {
		EGG__Heap__free(mine, 0);
		mine = 0;
	}
}

void dTexMapColouriser_c::setTexMap(nw4r::lyt::TexMap *tm) {
	OSReport("Colourising TexMap: %p (w:%d h:%d)\n", tm, tm->width, tm->height);
	if (texmap)
		resetAndClear();

	if (tm->getFormat() != GX_TF_IA8) {
		OSReport("Warning: Trying to colourise image whose format is %d not GX_TF_IA8\n", tm->getFormat());
	}

	texmap = tm;
	original = (u16*)tm->image;
	mine = (u16*)EGG__Heap__alloc(tm->width * tm->height * 4, 0x20, mHeap::gameHeaps[2]);
	tm->image = mine;
	tm->setFormat(GX_TF_RGBA8);
}

void dTexMapColouriser_c::applyAlso(nw4r::lyt::TexMap *tm) {
	if (!texmap) {
		setTexMap(tm);
	} else {
		tm->image = mine;
		tm->setFormat(GX_TF_RGBA8);
	}
}

inline static float hslValue(float n1, float n2, float hue) {
	if (hue > 6.0f)
		hue -= 6.0f;
	else if (hue < 0.0f)
		hue += 6.0f;

	if (hue < 1.0f)
		return n1 + (n2 - n1) * hue;
	else if (hue < 3.0f)
		return n2;
	else if (hue < 4.0f)
		return n1 + (n2 - n1) * (4.0f - hue);
	else
		return n1;
}

void dTexMapColouriser_c::colourise(int h, int s, int l) {
	if (!mine)
		return;

	int width = texmap->width, height = texmap->height;
	int texelW = width / 4, texelH = height / 4;

	u16 *source = original, *dest = mine;

	float hueParam = h / 360.0f;
	float satParam = s / 100.0f;
	float lumParam = l / 100.0f;

	for (int texelY = 0; texelY < texelH; texelY++) {
		for (int texelX = 0; texelX < texelW; texelX++) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					u8 intensity = *source & 0xFF;
					u8 alpha = *source >> 8;

					u8 r, g, b;

					// This is a hack
					if (alpha < 250) {
						r = g = b = intensity;
					} else {
						// converting from GIMP's colourise code...
						// h and s are always the same
						// l is the only thing we need to touch:
						// we get the luminance from the source pixel
						// (which, conveniently, is the intensity)
						// manipulate it using the passed l and then
						// convert the whole thing to RGB

						float lum = intensity / 255.0f;

						// manipulate it
						if (l > 0) {
							lum = lum * (1.0f - lumParam);
							lum += (1.0f - (1.0f - lumParam));
						} else if (l < 0) {
							lum = lum * (lumParam + 1.0f);
						}

						// make it RGB

						if (s == 0) {
							r = g = b = lum*255.0f;
						} else {
							float m1, m2;
							if (lum <= 0.5f)
								m2 = lum * (1.0f + satParam);
							else
								m2 = lum + satParam - lum * satParam;

							m1 = 2.0f * lum - m2;

							r = hslValue(m1, m2, hueParam * 6.0f + 2.0) * 255.0f;
							g = hslValue(m1, m2, hueParam * 6.0f) * 255.0f;
							b = hslValue(m1, m2, hueParam * 6.0f - 2.0) * 255.0f;
						}
					}

					// now write it
					dest[0] = (alpha<<8)|r;
					dest[16] = (g<<8)|b;

					source++;
					dest++;
				}
			}

			dest += 16;
		}
	}
}


