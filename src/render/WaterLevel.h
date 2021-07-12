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

// 64x64 Large blocks 64x64 each
#define WATER_TO_BLOCK_X(x)             ( WATER_UNSIGN_X(x) / WATER_BLOCK_SECTORS )
#define WATER_TO_BLOCK_Y(x)             ( WATER_UNSIGN_Y(x) / WATER_BLOCK_SECTORS ) 

// 128x128 Small blocks 32x32 each
#define WATER_TO_FINEBLOCK_X(x)         ( WATER_UNSIGN_X(x) / WATER_FINEBLOCK_SECTORS )
#define WATER_TO_FINEBLOCK_Y(x)         ( WATER_UNSIGN_Y(x) / WATER_FINEBLOCK_SECTORS ) 

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

struct ColData
{
	uint8 SurfaceType;
	uint8 PieceType;
};

enum eBeachToy
{
	BEACHTOY_0 = 0,
	BEACHTOY_BALL,
	BEACHTOY_LOUNGE_WOOD_UP,
	BEACHTOY_LOUNGE_TOWEL_UP,
	BEACHTOY_LOUNGE_WOOD_ON,
	BEACHTOY_ANY_LOUNGE,
	BEACHTOY_LOTION,
	BEACHTOY_TOWEL1,
	BEACHTOY_TOWEL2,
	BEACHTOY_TOWEL3,
	BEACHTOY_TOWEL4,
	BEACHTOY_ANY_TOWEL,
};

extern RwRaster* gpWaterRaster;
extern bool gbDontRenderWater;

class CEntity;

class CWaterLevel
{
public:
	static CWaterLevel  *mspInst;

	int32       m_nNoOfWaterLevels;
	float       *m_aWaterZs;
	CRect       *m_aWaterRects;
	int8        m_aWaterBlockList[MAX_LARGE_SECTORS][MAX_LARGE_SECTORS];				// 64x64 Large blocks 64x64 each
	int8        m_aWaterFineBlockList[MAX_SMALL_SECTORS][MAX_SMALL_SECTORS];			// 128x128 Small blocks 32x32 each
	void        *unk1; // TODO
	void        *unk2; // TODO

	static bool        WavesCalculatedThisFrame;

	static int32       mRecentlyRenderedWavySector;
	static bool        RequireWavySector;
	static bool        MaskCalculatedThisFrame;
	static CVector     PreCalculatedMaskPosn;
	static bool        m_bRenderSeaBed;
	static int32       m_nRenderWaterLayers;

	static RpAtomic    *ms_pWavyAtomic;
	static RpAtomic    *ms_pMaskAtomic;

	static void    Initialise(const char *pWaterDat, void *pInst = nil); // out of class in III PC and later because of SecuROM
	static void    Initialise2();
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
	static float   GetWaterWavesOnly(short x, short y);	// unused
	static CVector GetWaterNormal(float fX, float fY);

	static int32   NearWater();

	static void    RenderWater();
	static void    RenderTransparentWater(void);
	// unused 
	static void RenderOneFlatSmallWaterPoly    (float fX, float fY, float fZ, RwRGBA const &color);
	// inlined
	static void RenderOneFlatLargeWaterPoly    (float fX, float fY, float fZ, RwRGBA const &color);	
	static void RenderOneFlatHugeWaterPoly     (float fX, float fY, float fZ, RwRGBA const &color);
	static void RenderOneFlatExtraHugeWaterPoly(float fX, float fY, float fZ, RwRGBA const &color);
	// inlined
	static void RenderOneWavySector            (float fX, float fY, float fZ, RwRGBA const &color, bool bDontRender = false);
	// unused
#ifdef PC_WATER
	static void RenderWavyMask(float fX, float fY, float fZ, float fSectorX, float fSectorY, float fCamPosX, float fCamPosY, float fCamDirX, float fCamDirY, RwRGBA const&color);
#else
	static void RenderWavyMask(float fX, float fY, float fZ, float fSectorX, float fSectorY, int32 nCamDirX, int32 nCamDirY, RwRGBA const&color);
#endif

#ifdef PC_WATER
	static void PreCalcWaterGeometry(void);
	static bool PreCalcWavySector(RwRGBA const &color);	//fucked up
	static bool PreCalcWavyMask(float fX, float fY, float fZ, float fSectorX, float fSectorY, float fCamPosX, float fCamPosY, float fCamDirX, float fCamDirY, RwRGBA const&color);
#endif


	static void RenderBoatWakes(void);
	static void RenderWakeSegment(CVector2D &vecA, CVector2D &vecB, CVector2D &vecC, CVector2D &vecD, float &fSizeA, float &fSizeB, float &fAlphaA, float &fAlphaB, float &fWakeZ);

	// unused
	static void RenderOneSlopedUnderWaterPoly(float fX, float fY, float fZ, RwRGBA const&color); // UNUSED
	static void RenderOneFlatSmallWaterPolyBlended(float fX, float fY, float fZ, float fCamX, float fCamY, RwRGBA const &color, RwRGBA const &colorTrans, float fDrawDist);
	static float CalcDistanceToWater(float fX, float fY);
	static void RenderAndEmptyRenderBuffer();
	
	static bool GetGroundLevel(CVector const &vecPosn, float *pfOutLevel, ColData *pData, float fDistance);

	// unused
	static bool IsLocationOutOfWorldBounds_WS(CVector const &vecPosn, int nOffset);
	// unused
	static bool GetGroundLevel_WS(CVector const & vecPosn, float *pfOutLevel, ColData *pData, float fDistance);
	static bool GetWaterDepth(CVector const &vecPosn, float *pfDepth, float *pfLevelNoWaves, float *pfGroundLevel);

	static void RenderSeaBirds();
	static void RenderShipsOnHorizon();

	static void HandleSeaLifeForms();

	static void HandleBeachToysStuff(void);
	static CEntity *CreateBeachToy(CVector const &vec, eBeachToy beachtoy);
};

VALIDATE_SIZE(CWaterLevel, 0x5014);
