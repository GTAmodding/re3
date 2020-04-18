#include "dcommon.h"

#pragma pack(push, 1)
struct TGAHeader
{
	RwInt8 IDlen;
	RwInt8 colorMapType;
	RwInt8 imageType;
	RwInt16 colorMapOrigin;
	RwInt16 colorMapLength;
	RwInt8 colorMapDepth;
	RwInt16 xOrigin, yOrigin;
	RwInt16 width, height;
	RwUInt8 depth;
	RwUInt8 descriptor;
};
#pragma pack(pop)

// Should be done better
RwImage *
readTGA(uint8 *p)
{
	RwImage *image;

	TGAHeader header;
	int depth = 0, palDepth = 0;
	memcpy(&header, p, sizeof(header));
	p += sizeof(header);

	assert(header.imageType == 1 || header.imageType == 2);
	p += header.IDlen;
	if (header.colorMapType)
	{
		assert(header.colorMapOrigin == 0);
		depth = (header.colorMapLength <= 16) ? 4 : 8;
		palDepth = header.colorMapDepth;
		assert(palDepth == 24 || palDepth == 32);
	}
	else
	{
		depth = header.depth;
		assert(depth == 24 || depth == 32);
	}

	image = RwImageCreate(header.width, header.height, depth == 24 ? 32 : depth);
	image->allocate();
	RwRGBA *palette = header.colorMapType ? (RwRGBA *)image->palette : NULL;
	if (palette)
	{
		int maxlen = depth == 4 ? 16 : 256;
		int i;
		for (i = 0; i < header.colorMapLength; i++)
		{
			palette[i].blue = *p++;
			palette[i].green = *p++;
			palette[i].red = *p++;
			palette[i].alpha = 0xFF;
			if (palDepth == 32)
				palette[i].alpha = *p++;
		}
		for (; i < maxlen; i++)
		{
			palette[i].red = palette[i].green = palette[i].blue = 0;
			palette[i].alpha = 0xFF;
		}
	}

	RwUInt8 *pixels = image->pixels;
	if (!(header.descriptor & 0x20))
		pixels += (image->height - 1) * image->stride;
	for (int y = 0; y < image->height; y++)
	{
		RwUInt8 *line = pixels;
		for (int x = 0; x < image->width; x++)
		{
			if (palette)
				*line++ = *p++;
			else
			{
				line[2] = *p++;
				line[1] = *p++;
				line[0] = *p++;
				line += 3;
				if (depth == 32)
					*line++ = *p++;
				if (depth == 24)
					*line++ = 0xFF;
			}
		}
		pixels += (header.descriptor & 0x20) ? image->stride : -image->stride;
	}
	return image;
}