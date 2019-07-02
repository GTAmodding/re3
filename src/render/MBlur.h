#pragma once

class CMBlur
{
public:
	static RwRaster *&pFrontBuffer;
	static bool &ms_bJustInitialised;
	static bool &BlurOn;

public:
	static void MotionBlurOpen(RwCamera *cam);
	static void MotionBlurClose(void);
	static void CreateImmediateModeData(RwCamera *cam, RwRect *rect);
	static void MotionBlurRender(RwCamera *cam, uint32 red, uint32 green, uint32 blue, uint32 blur, int32 type, uint32 addalpha);
	static void OverlayRender(RwCamera *cam, RwRaster *raster, RwRGBA color, int32 type, uint32 bluralpha);
};
