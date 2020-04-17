#include "dcommon.h"
#include "common.h"
#include "rwcore.h"
#include "rwplcore.h"
#include "main.h"	// Camera
#include "Bm437_IBM_BIOS.h"
#include "Bm437_IBM_VGA8.h"

Font vga = {{nil, nil, nil}, 8, 16, 256};
Font bios = {{nil, nil, nil}, 8, 8, 256};
Font *curfont = &vga;

#define NUMCHARS 100
RwImVertexIndex indices[NUMCHARS * 6];
RwIm2DVertex vertices[NUMCHARS * 4];
int curVert;
int curIndex;

int fontscale = 1;

void changeColors(RwImage *img, RwRGBA fg, RwRGBA bg)
{
	RwInt32 x, y;
	RwInt32 w = img->width;
	RwInt32 h = img->height;
	RwInt32 stride = img->stride;
	RwUInt8 *pixels = img->pixels;
	RwUInt8 *line, *p;
	assert(img->depth == 32);

	line = pixels;
	for (y = 0; y < h; y++)
	{
		p = line;
		for (x = 0; x < w; x++)
		{
			if (p[3])
			{
				p[0] = fg.red;
				p[1] = fg.green;
				p[2] = fg.blue;
				p[3] = fg.alpha;
			}
			else
			{
				p[0] = bg.red;
				p[1] = bg.green;
				p[2] = bg.blue;
				p[3] = bg.alpha;
			}
			p += 4;
		}
		line += stride;
	}
}

void createDebugFont(uint8* data, Font *font)
{
	uint8 *pixels;
	int size;
	RwInt32 w, h, d, flags;
	int style;

	RwImage *img = readTGA(data);
	size = img->stride * img->height;
	pixels = new uint8[size];
	memcpy(pixels, img->pixels, size);
	RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &flags);

	style = FONT_NORMAL;
	RwRGBA fg_normal = {255, 255, 255, 255};
	RwRGBA bg_normal = {255, 255, 255, 0};
	changeColors(img, fg_normal, bg_normal);
	font->rasters[style] = RwRasterCreate(w, h, d, flags);
	font->rasters[style] = RwRasterSetFromImage(font->rasters[style], img);
	assert(font->rasters[style]);

	memcpy(img->pixels, pixels, size); // reset pixels

	style = FONT_SEL_ACTIVE;
	RwRGBA fg_sel_active = {200, 200, 200, 255};
	RwRGBA bg_sel_active = {132, 132, 132, 255};
	changeColors(img, fg_sel_active, bg_sel_active);
	font->rasters[style] = RwRasterCreate(w, h, d, flags);
	font->rasters[style] = RwRasterSetFromImage(font->rasters[style], img);
	assert(font->rasters[style]);

	memcpy(img->pixels, pixels, size); // reset pixels

	style = FONT_SEL_INACTIVE;
	RwRGBA fg_sel_inactive = {200, 200, 200, 255};
	RwRGBA bg_sel_inactive = {200, 200, 200, 0};
	changeColors(img, fg_sel_inactive, bg_sel_inactive);
	font->rasters[style] = RwRasterCreate(w, h, d, flags);
	font->rasters[style] = RwRasterSetFromImage(font->rasters[style], img);
	assert(font->rasters[style]);

	memcpy(img->pixels, pixels, size); // reset pixels

	style = FONT_MOUSE;
	RwRGBA fg_mouse = {255, 255, 255, 255};
	RwRGBA bg_mouse = {132, 132, 132, 255};
	changeColors(img, fg_mouse, bg_mouse);
	font->rasters[style] = RwRasterCreate(w, h, d, flags);
	font->rasters[style] = RwRasterSetFromImage(font->rasters[style], img);
	assert(font->rasters[style]);

	RwImageDestroy(img);
	delete[] pixels;
}

void createDebugFonts(void)
{
	createDebugFont((uint8*)Bm437_IBM_BIOS_tga, &bios);
	createDebugFont((uint8*)Bm437_IBM_VGA8_tga, &vga);
}

Pt fontPrint(const char *s, float xstart, float ystart, int style)
{
	char c;
	RwCamera *cam;
	RwRaster *raster;
	RwIm2DVertex *vert;
	RwImVertexIndex *ix;
	float u, v, du, dv;
	float uhalf, vhalf;
	float x, y;
	Pt sz;
	int szx;

	sz.y = curfont->glyphheight * fontscale;
	sz.x = 0;
	szx = 0;

	x = xstart;
	y = ystart;
	raster = curfont->rasters[style];
	curVert = 0;
	curIndex = 0;
	cam = Scene.camera;
	vert = &vertices[curVert];
	ix = &indices[curIndex];
	du = curfont->glyphwidth / (float)raster->width;
	dv = curfont->glyphheight / (float)raster->height;
	uhalf = 0.5f / raster->width;
	vhalf = 0.5f / raster->height;
	float recipz = 1.0f / RwCameraGetNearClipPlane(cam);
	while (c = *s++)
	{
		if (c == '\n')
		{
			x = xstart;
			y += curfont->glyphheight * fontscale;
			sz.y = curfont->glyphheight * fontscale;
			if (szx > sz.x)
				sz.x = szx;
			szx = 0;
			continue;
		}

		if (c >= curfont->numglyphs)
			c = 0;
		u = (c % 16) * curfont->glyphwidth / (float)raster->width;
		v = (c / 16) * curfont->glyphheight / (float)raster->height;

		RwIm2DVertexSetScreenX(vert, x);
		RwIm2DVertexSetScreenY(vert, y);
		RwIm2DVertexSetScreenZ(vert, RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(vert, RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(vert, recipz);
		RwIm2DVertexSetIntRGBA(vert, 255, 255, 255, 255);
		RwIm2DVertexSetU(vert, u + uhalf, recipz);
		RwIm2DVertexSetV(vert, v + vhalf, recipz);
		vert++;

		RwIm2DVertexSetScreenX(vert, x + curfont->glyphwidth * fontscale);
		RwIm2DVertexSetScreenY(vert, y);
		RwIm2DVertexSetScreenZ(vert, RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(vert, RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(vert, recipz);
		RwIm2DVertexSetIntRGBA(vert, 255, 255, 255, 255);
		RwIm2DVertexSetU(vert, u + du + uhalf, recipz);
		RwIm2DVertexSetV(vert, v + vhalf, recipz);
		vert++;

		RwIm2DVertexSetScreenX(vert, x);
		RwIm2DVertexSetScreenY(vert, y + curfont->glyphheight * fontscale);
		RwIm2DVertexSetScreenZ(vert, RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(vert, RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(vert, recipz);
		RwIm2DVertexSetIntRGBA(vert, 255, 255, 255, 255);
		RwIm2DVertexSetU(vert, u + uhalf, recipz);
		RwIm2DVertexSetV(vert, v + dv + vhalf, recipz);
		vert++;

		RwIm2DVertexSetScreenX(vert, x + curfont->glyphwidth * fontscale);
		RwIm2DVertexSetScreenY(vert, y + curfont->glyphheight * fontscale);
		RwIm2DVertexSetScreenZ(vert, RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(vert, RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(vert, recipz);
		RwIm2DVertexSetIntRGBA(vert, 255, 255, 255, 255);
		RwIm2DVertexSetU(vert, u + du + uhalf, recipz);
		RwIm2DVertexSetV(vert, v + dv + vhalf, recipz);
		vert++;
		*ix++ = curVert;
		*ix++ = curVert + 1;
		*ix++ = curVert + 2;
		*ix++ = curVert + 2;
		*ix++ = curVert + 1;
		*ix++ = curVert + 3;

		curVert += 4;
		curIndex += 6;
		x += curfont->glyphwidth * fontscale;
		szx += curfont->glyphwidth * fontscale;
	}
	if (szx > sz.x)
		sz.x = szx;
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, vertices, curVert, indices, curIndex);
	return sz;
}

Pt fontGetStringSize(const char *s)
{
	Pt sz = {0, 0};
	int x;
	char c;
	sz.y = curfont->glyphheight * fontscale; // always assume one line;
	x = 0;
	while (c = *s++)
	{
		if (c == '\n')
		{
			sz.y += curfont->glyphheight * fontscale;
			if (x > sz.x)
				sz.x = x;
			x = 0;
		}
		else
			x += curfont->glyphwidth * fontscale;
	}
	if (x > sz.x)
		sz.x = x;
	return sz;
}

int fontGetLen(int len)
{
	return len * curfont->glyphwidth * fontscale;
}
