#ifndef TPL_H
#define TPL_H 

extern "C" {

typedef struct {
	u16 height, width;
	u32 format;
	void *data;
	u32 wrapS, wrapT, minFilter, magFilter;
	float lodBias;
	u8 edgeLod, minLod, maxLod, unpacked;
} TPLTexHeader;

typedef struct {
	// don't need this yet
	u8 dummy;
} TPLPalHeader;

typedef struct {
	TPLTexHeader *texture;
	TPLPalHeader *palette;
} TPLImage;

typedef struct {
	u32 magic;
	u32 imageCount;
	TPLImage *images;
} TPLPalette;

void TPLBind(TPLPalette *palette);
TPLImage *TPLGet(TPLPalette *palette, int index);

}

#endif /* TPL_H */
