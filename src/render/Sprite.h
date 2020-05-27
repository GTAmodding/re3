#pragma once

class CSprite
{
	static float m_f2DNearScreenZ;
	static float m_f2DFarScreenZ;
	static float m_fRecipNearClipPlane;
	static int32 m_bFlushSpriteBufferSwitchZTest;
public:
	static float GetNearScreenZ(void) { return m_f2DNearScreenZ; }
	static float GetFarScreenZ(void) { return m_f2DFarScreenZ; }

	static float CalcHorizonCoors(void);
	static bool CalcScreenCoors(const RwV3d &in, RwV3d *out, float *outw, float *outh, bool farclip);
	static void InitSpriteBuffer(void);
	static void InitSpriteBuffer2D(void);
	static void FlushSpriteBuffer(void);
	static void RenderOneXLUSprite(float x, float y, float z, float w, float h, uint8 r, uint8 g, uint8 b, int16 intens, float recipz, uint8 a);
	static void RenderOneXLUSprite_Rotate_Aspect(float x, float y, float z, float w, float h, uint8 r, uint8 g, uint8 b, int16 intens, float recipz, float roll, uint8 a);
	static void RenderBufferedOneXLUSprite(float x, float y, float z, float w, float h, uint8 r, uint8 g, uint8 b, int16 intens, float recipz, uint8 a);
	static void RenderBufferedOneXLUSprite_Rotate_Dimension(float x, float y, float z, float w, float h, uint8 r, uint8 g, uint8 b, int16 intens, float recipz, float roll, uint8 a);
	static void RenderBufferedOneXLUSprite_Rotate_Aspect(float x, float y, float z, float w, float h, uint8 r, uint8 g, uint8 b, int16 intens, float recipz, float roll, uint8 a);
	// cx/y is the direction in which the colour changes
	static void RenderBufferedOneXLUSprite_Rotate_2Colours(float x, float y, float z, float w, float h, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, float cx, float cy, float recipz, float rotation, uint8 a);
	static void Set6Vertices2D(RwIm2DVertex *verts, const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3);
	static void Set6Vertices2D(RwIm2DVertex *verts, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
		const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3);
	static void RenderBufferedOneXLUSprite2D(float x, float y, float w, float h, const RwRGBA &colour, int16 intens, uint8 alpha);
	static void RenderBufferedOneXLUSprite2D_Rotate_Dimension(float x, float y, float w, float h, const RwRGBA &colour, int16 intens, float rotation, uint8 alpha);

};
