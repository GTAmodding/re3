#ifndef LIBRW
#define WITHD3D
#endif
#include "common.h"
#ifndef LIBRW
#include <d3d8caps.h>
#endif

#include "main.h"
#include "RwHelper.h"
#include "Camera.h"
#include "MBlur.h"
#include "postfx.h"

// Originally taken from RW example 'mblur'

RwRaster *CMBlur::pFrontBuffer;
bool CMBlur::ms_bJustInitialised;
bool CMBlur::ms_bScaledBlur;
bool CMBlur::BlurOn;

static RwIm2DVertex Vertex[4];
static RwImVertexIndex Index[6] = { 0, 1, 2, 0, 2, 3 };

#ifndef LIBRW
extern "C" D3DCAPS8 _RwD3D8DeviceCaps;
#endif
RwBool
CMBlur::MotionBlurOpen(RwCamera *cam)
{
#ifdef EXTENDED_COLOURFILTER
	CPostFX::Open(cam);
	return TRUE;
#else
#ifdef GTA_PS2
	RwRect rect = {0, 0, 0, 0};
	
	if (pFrontBuffer)
		return TRUE;
	
	BlurOn = true;
	
	rect.w = RwRasterGetWidth(RwCameraGetRaster(cam));
	rect.h = RwRasterGetHeight(RwCameraGetRaster(cam));
	
	pFrontBuffer = RwRasterCreate(0, 0, 0, rwRASTERDONTALLOCATE|rwRASTERTYPECAMERATEXTURE);
	if (!pFrontBuffer)
	{
		printf("Error creating raster\n");
		return FALSE;
	}
	
	RwRaster *raster = RwRasterSubRaster(pFrontBuffer, RwCameraGetRaster(cam), &rect);
	if (!raster)
	{
		RwRasterDestroy(pFrontBuffer);
		pFrontBuffer = NULL;
		printf("Error subrastering\n");
		return FALSE;
	}
	
	CreateImmediateModeData(cam, &rect);
#else
	RwRect rect = { 0, 0, 0, 0 };

	if(pFrontBuffer)
		MotionBlurClose();
	
#ifndef LIBRW
	extern void _GetVideoMemInfo(LPDWORD total, LPDWORD avaible);
	DWORD total, avaible;
	
	_GetVideoMemInfo(&total, &avaible);
	debug("Available video memory %d\n", avaible);
#endif
		
	if(BlurOn)
	{
		uint32 width  = Pow(2.0f, int32(log2(RwRasterGetWidth (RwCameraGetRaster(cam))))+1);
		uint32 height = Pow(2.0f, int32(log2(RwRasterGetHeight(RwCameraGetRaster(cam))))+1);
		uint32 depth  = RwRasterGetDepth(RwCameraGetRaster(cam));
		
#ifndef LIBRW
		extern DWORD _dwMemTotalVideo;
		if ( _RwD3D8DeviceCaps.MaxTextureWidth >= width && _RwD3D8DeviceCaps.MaxTextureHeight >= height )
		{
			total = _dwMemTotalVideo - 3 *
				( RwRasterGetDepth(RwCameraGetRaster(cam))
				* RwRasterGetHeight(RwCameraGetRaster(cam))
				* RwRasterGetWidth(RwCameraGetRaster(cam)) / 8 );
			BlurOn = total >= height*width*(depth/8) + (12*1024*1024) /*12 MB*/;
		}
		else
			BlurOn = false;
#endif
		
		if ( BlurOn )
		{
			ms_bScaledBlur = false;
			rect.w = width;
			rect.h = height;
			
			pFrontBuffer = RwRasterCreate(rect.w, rect.h, depth, rwRASTERTYPECAMERATEXTURE);
			if ( !pFrontBuffer )
			{
				debug("MBlurOpen can't create raster.");
				BlurOn = false;
				rect.w = RwRasterGetWidth(RwCameraGetRaster(cam));
				rect.h = RwRasterGetHeight(RwCameraGetRaster(cam));
			}
			else
				ms_bJustInitialised = true;
		}
		else
		{
			rect.w = RwRasterGetWidth(RwCameraGetRaster(cam));
			rect.h = RwRasterGetHeight(RwCameraGetRaster(cam));
		}
		
#ifndef LIBRW
		_GetVideoMemInfo(&total, &avaible);
		debug("Available video memory %d\n", avaible);
#endif
		CreateImmediateModeData(cam, &rect);
	}
	else
	{
		rect.w = RwRasterGetWidth(RwCameraGetRaster(cam));
		rect.h = RwRasterGetHeight(RwCameraGetRaster(cam));
		CreateImmediateModeData(cam, &rect);
	}
	
	return TRUE;
#endif
#endif
}

RwBool
CMBlur::MotionBlurClose(void)
{
#ifdef EXTENDED_COLOURFILTER
	CPostFX::Close();
#else
	if(pFrontBuffer){
		RwRasterDestroy(pFrontBuffer);
		pFrontBuffer = nil;
		
		return TRUE;
	}
#endif
	return FALSE;
}

void
CMBlur::CreateImmediateModeData(RwCamera *cam, RwRect *rect)
{
	float zero, xmax, ymax;

	if(RwRasterGetDepth(RwCameraGetRaster(cam)) == 16){
		zero = HALFPX;
		xmax = rect->w + HALFPX;
		ymax = rect->h + HALFPX;
	}else{
		zero = -HALFPX;
		xmax = rect->w - HALFPX;
		ymax = rect->h - HALFPX;
	}

	RwIm2DVertexSetScreenX(&Vertex[0], zero);
	RwIm2DVertexSetScreenY(&Vertex[0], zero);
	RwIm2DVertexSetScreenZ(&Vertex[0], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex[0], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex[0], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex[0], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex[0], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex[0], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex[1], zero);
	RwIm2DVertexSetScreenY(&Vertex[1], ymax);
	RwIm2DVertexSetScreenZ(&Vertex[1], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex[1], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex[1], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex[1], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex[1], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex[1], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex[2], xmax);
	RwIm2DVertexSetScreenY(&Vertex[2], ymax);
	RwIm2DVertexSetScreenZ(&Vertex[2], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex[2], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex[2], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex[2], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex[2], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex[2], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex[3], xmax);
	RwIm2DVertexSetScreenY(&Vertex[3], zero);
	RwIm2DVertexSetScreenZ(&Vertex[3], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex[3], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex[3], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex[3], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex[3], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex[3], 255, 255, 255, 255);
}

void
CMBlur::MotionBlurRender(RwCamera *cam, uint32 red, uint32 green, uint32 blue, uint32 blur, int32 type, uint32 bluralpha)
{
#ifdef EXTENDED_COLOURFILTER
	CPostFX::Render(cam, red, green, blue, blur, type, bluralpha);
#else
	PUSH_RENDERGROUP("CMBlur::MotionBlurRender");
	RwRGBA color = { (RwUInt8)red, (RwUInt8)green, (RwUInt8)blue, (RwUInt8)blur };
#ifdef GTA_PS2
	if( pFrontBuffer )
		OverlayRender(cam, pFrontBuffer, color, type, bluralpha);
#else
	if(BlurOn){
		if(pFrontBuffer){
			if(ms_bJustInitialised)
				ms_bJustInitialised = false;
			else
				OverlayRender(cam, pFrontBuffer, color, type, bluralpha);
		}
		RwRasterPushContext(pFrontBuffer);
		RwRasterRenderFast(RwCameraGetRaster(cam), 0, 0);
		RwRasterPopContext();
	}else{
		OverlayRender(cam, nil, color, type, bluralpha);
	}
#endif
	POP_RENDERGROUP();
#endif
}

void
CMBlur::OverlayRender(RwCamera *cam, RwRaster *raster, RwRGBA color, int32 type, int32 bluralpha)
{
	int r, g, b, a;

	r = color.red;
	g = color.green;
	b = color.blue;
	a = color.alpha;

	DefinedState();

	switch(type)
	{
	case MOTION_BLUR_SECURITY_CAM:
		r = 0;
		g = 255;
		b = 0;
		a = 128;
		break;
	case MOTION_BLUR_INTRO:
		r = 100;
		g = 220;
		b = 230;
		a = 158;
		break;
	case MOTION_BLUR_INTRO2:
		r = 80;
		g = 255;
		b = 230;
		a = 138;
		break;
	case MOTION_BLUR_INTRO3:
		r = 255;
		g = 60;
		b = 60;
		a = 200;
		break;
	case MOTION_BLUR_INTRO4:
		r = 255;
		g = 180;
		b = 180;
		a = 128;
		break;
	}

	if(!BlurOn){
		r = Min(r*0.6f, 255.0f);
		g = Min(g*0.6f, 255.0f);
		b = Min(b*0.6f, 255.0f);
		if(type != MOTION_BLUR_SNIPER)
			a = Min(a*0.6f, 255.0f);
		// game clamps to 255 here, but why?
	}
	RwIm2DVertexSetIntRGBA(&Vertex[0], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex[1], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex[2], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex[3], r, g, b, a);

	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, BlurOn ? raster : nil);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, Vertex, 4, Index, 6);

	a = bluralpha/2;
	if(a < 30)
		a = 30;

	if(BlurOn && a != 0){	// the second condition should always be true
		RwIm2DVertexSetIntRGBA(&Vertex[0], 255, 255, 255, a);
		RwIm2DVertexSetIntRGBA(&Vertex[1], 255, 255, 255, a);
		RwIm2DVertexSetIntRGBA(&Vertex[2], 255, 255, 255, a);
		RwIm2DVertexSetIntRGBA(&Vertex[3], 255, 255, 255, a);
		RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, Vertex, 4, Index, 6);
	}

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}
