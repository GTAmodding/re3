#pragma once

#define WATER_Z_OFFSET (1.5f)

#define NO_WATER -128

#define MAX_SMALL_SECTORS      128
#define MAX_LARGE_SECTORS      64
#define MAX_HUGE_SECTORS       32
#define MAX_EXTRAHUGE_SECTORS  16

#define SMALL_SECTOR_SIZE      32
#define LARGE_SECTOR_SIZE      64
#define HUGE_SECTOR_SIZE       128
#define EXTRAHUGE_SECTOR_SIZE  256

#define WATER_START_X -2048.0f
#define WATER_END_X    2048.0f

#define WATER_START_Y -2048.0f
#define WATER_END_Y    2048.0f

#define WATER_WIDTH		((WATER_END_X - WATER_START_X))
#define WATER_HEIGHT	((WATER_END_Y - WATER_START_Y))

#define WATER_UNSIGN_X(x)                   ( (x) + (WATER_WIDTH /2) )
#define WATER_UNSIGN_Y(y)                   ( (y) + (WATER_HEIGHT/2) )
#define WATER_SIGN_X(x)                     ( (x) - (WATER_WIDTH /2) )
#define WATER_SIGN_Y(y)                     ( (y) - (WATER_HEIGHT/2) )

// 32
#define WATER_SMALL_X(x)                    ( WATER_UNSIGN_X(x)					/ MAX_SMALL_SECTORS     )
#define WATER_SMALL_Y(y)                    ( WATER_UNSIGN_Y(y)					/ MAX_SMALL_SECTORS     )
#define WATER_FROM_SMALL_SECTOR_X(x)        ( ((x) - (MAX_SMALL_SECTORS/2)    ) * SMALL_SECTOR_SIZE     )
#define WATER_FROM_SMALL_SECTOR_Y(y)        ( ((y) - (MAX_SMALL_SECTORS/2)    ) * SMALL_SECTOR_SIZE     )
#define WATER_TO_SMALL_SECTOR_X(x)          ( WATER_UNSIGN_X(x)					/ SMALL_SECTOR_SIZE     )
#define WATER_TO_SMALL_SECTOR_Y(y)          ( WATER_UNSIGN_Y(y)					/ SMALL_SECTOR_SIZE     )
			
// 64			
#define WATER_LARGE_X(x)                    ( WATER_UNSIGN_X(x)					/ MAX_LARGE_SECTORS     )
#define WATER_LARGE_Y(y)                    ( WATER_UNSIGN_Y(y)					/ MAX_LARGE_SECTORS     )
#define WATER_FROM_LARGE_SECTOR_X(x)        ( ((x) - (MAX_LARGE_SECTORS/2)    ) * LARGE_SECTOR_SIZE     )
#define WATER_FROM_LARGE_SECTOR_Y(y)        ( ((y) - (MAX_LARGE_SECTORS/2)    ) * LARGE_SECTOR_SIZE     )
#define WATER_TO_LARGE_SECTOR_X(x)          ( WATER_UNSIGN_X(x)					/ LARGE_SECTOR_SIZE     )
#define WATER_TO_LARGE_SECTOR_Y(y)          ( WATER_UNSIGN_Y(y)					/ LARGE_SECTOR_SIZE     )
				
// 128				
#define WATER_HUGE_X(x)                     ( WATER_UNSIGN_X(x)					/ MAX_HUGE_SECTORS      )
#define WATER_HUGE_Y(y)                     ( WATER_UNSIGN_Y(y)					/ MAX_HUGE_SECTORS      )
#define WATER_FROM_HUGE_SECTOR_X(x)         ( ((x) - (MAX_HUGE_SECTORS/2)     ) * HUGE_SECTOR_SIZE      )
#define WATER_FROM_HUGE_SECTOR_Y(y)         ( ((y) - (MAX_HUGE_SECTORS/2)     ) * HUGE_SECTOR_SIZE      )
#define WATER_TO_HUGE_SECTOR_X(x)           ( WATER_UNSIGN_X(x)					/ HUGE_SECTOR_SIZE      )
#define WATER_TO_HUGE_SECTOR_Y(y)           ( WATER_UNSIGN_Y(y)					/ HUGE_SECTOR_SIZE      )

// 256	
#define WATER_EXTRAHUGE_X(x)                ( WATER_UNSIGN_X(x)					/ MAX_EXTRAHUGE_SECTORS )
#define WATER_EXTRAHUGE_Y(y)                ( WATER_UNSIGN_Y(y)					/ MAX_EXTRAHUGE_SECTORS )
#define WATER_FROM_EXTRAHUGE_SECTOR_X(x)    ( ((x) - (MAX_EXTRAHUGE_SECTORS/2)) * EXTRAHUGE_SECTOR_SIZE )
#define WATER_FROM_EXTRAHUGE_SECTOR_Y(y)    ( ((y) - (MAX_EXTRAHUGE_SECTORS/2)) * EXTRAHUGE_SECTOR_SIZE )
#define WATER_TO_EXTRAHUGE_SECTOR_X(x)      ( WATER_UNSIGN_X(x)					/ EXTRAHUGE_SECTOR_SIZE )
#define WATER_TO_EXTRAHUGE_SECTOR_Y(y)      ( WATER_UNSIGN_Y(y)					/ EXTRAHUGE_SECTOR_SIZE )


#define MAX_BOAT_WAKES 8

extern RwRaster* gpWaterRaster;
extern bool gbDontRenderWater;

class CWaterLevel
{
	static int32       ms_nNoOfWaterLevels;
	static float       ms_aWaterZs[48];
	static CRect       ms_aWaterRects[48];
	static int8        aWaterBlockList[MAX_LARGE_SECTORS][MAX_LARGE_SECTORS];
	static int8        aWaterFineBlockList[MAX_SMALL_SECTORS][MAX_SMALL_SECTORS];
	static bool        WavesCalculatedThisFrame;
	static RpAtomic   *ms_pWavyAtomic;
	static RpGeometry *apGeomArray[MAX_BOAT_WAKES];
	static int16       nGeomUsed;

public:
	static void    Initialise(Const char *pWaterDat); // out of class in III PC and later because of SecuROM
	static void    Shutdown();
	static void    CreateWavyAtomic();
	static void    DestroyWavyAtomic();
	static void    AddWaterLevel(float fXLeft, float fYBottom, float fXRight, float fYTop, float fLevel);
	static bool    WaterLevelAccordingToRectangles(float fX, float fY, float *pfOutLevel = nil);
	static bool    TestVisibilityForFineWaterBlocks(const CVector &worldPos);
	static void    RemoveIsolatedWater();
	static bool    GetWaterLevel(float fX, float fY, float fZ, float *pfOutLevel, bool bDontCheckZ);
	static bool    GetWaterLevel(CVector coors, float *pfOutLevel, bool bDontCheckZ) { return GetWaterLevel(coors.x, coors.y, coors.z, pfOutLevel, bDontCheckZ); }
	static bool    GetWaterLevelNoWaves(float fX, float fY, float fZ, float *pfOutLevel);
	static void    RenderWater();
	static void    RenderOneFlatSmallWaterPoly    (float fX, float fY, float fZ, RwRGBA const &color);
	static void    RenderOneFlatLargeWaterPoly    (float fX, float fY, float fZ, RwRGBA const &color);	
	static void    RenderOneFlatHugeWaterPoly     (float fX, float fY, float fZ, RwRGBA const &color);
	static void    RenderOneFlatExtraHugeWaterPoly(float fX, float fY, float fZ, RwRGBA const &color);
	static void    RenderOneWavySector            (float fX, float fY, float fZ, RwRGBA const &color, bool bUnk = false);
	static float   CalcDistanceToWater(float fX, float fY);
	static void    RenderAndEmptyRenderBuffer();	
	static void    AllocateBoatWakeArray();
	static void    FreeBoatWakeArray();
};
