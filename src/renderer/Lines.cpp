#include "common.h"

#include "main.h"
#include "Lines.h"

// This is super inefficient, why split the line into segments at all?
void
CLines::RenderLineWithClipping(float x1, float y1, float z1, float x2, float y2, float z2, uint32 c1, uint32 c2)
{
	static RwIm3DVertex v[2];
#ifdef THIS_IS_STUPID
	int i;
	float f1, f2;
	float len = sqrt(sq(x1-x2) + sq(y1-y2) + sq(z1-z2));
	int numsegs = len/1.5f + 1.0f;

	RwRGBA col1;
	col1.red = c1>>24;
	col1.green = c1>>16;
	col1.blue = c1>>8;
	col1.alpha = c1;
	RwRGBA col2;
	col2.red = c2>>24;
	col2.green = c2>>16;
	col2.blue = c2>>8;
	col2.alpha = c2;

	float dx = x2 - x1;
	float dy = y2 - y1;
	float dz = z2 - z1;
	for(i = 0; i < numsegs; i++){
		f1 = (float)i/numsegs;
		f2 = (float)(i+1)/numsegs;

		RwIm3DVertexSetRGBA(&v[0], (int)(col1.red + (col2.red-col1.red)*f1),
			(int)(col1.green + (col2.green-col1.green)*f1),
			(int)(col1.blue + (col2.blue-col1.blue)*f1),
			(int)(col1.alpha + (col2.alpha-col1.alpha)*f1));
		RwIm3DVertexSetRGBA(&v[1], (int)(col1.red + (col2.red-col1.red)*f2),
			(int)(col1.green + (col2.green-col1.green)*f2),
			(int)(col1.blue + (col2.blue-col1.blue)*f2),
			(int)(col1.alpha + (col2.alpha-col1.alpha)*f2));
		RwIm3DVertexSetPos(&v[0], x1 + dx*f1, y1 + dy*f1, z1 + dz*f1);
		RwIm3DVertexSetPos(&v[1], x1 + dx*f2, y1 + dy*f2, z1 + dz*f2);

		LittleTest();
		if(RwIm3DTransform(v, 2, nil, 0)){
			RwIm3DRenderLine(0, 1);
			RwIm3DEnd();
		}
	}
#else
	RwRGBA col1;
	col1.red = c1>>24;
	col1.green = c1>>16;
	col1.blue = c1>>8;
	col1.alpha = c1;
	RwRGBA col2;
	col2.red = c2>>24;
	col2.green = c2>>16;
	col2.blue = c2>>8;
	col2.alpha = c2;

	RwIm3DVertexSetRGBA(&v[0], col1.red, col1.green, col1.blue, col1.alpha);
	RwIm3DVertexSetRGBA(&v[1], col2.red, col2.green, col2.blue, col2.alpha);
	RwIm3DVertexSetPos(&v[0], x1, y1, z1);
	RwIm3DVertexSetPos(&v[1], x2, y2, z2);
	LittleTest();
	if(RwIm3DTransform(v, 2, nil, 0)){
		RwIm3DRenderLine(0, 1);
		RwIm3DEnd();
	}
#endif
}
