#include "common.h"
#include "main.h"
#include "FileMgr.h"
#include "TxdStore.h"
#include "Timer.h"
#include "Weather.h"
#include "Camera.h"
#include "Vehicle.h"
#include "Boat.h"
#include "World.h"
#include "General.h"
#include "Timecycle.h"
#include "ZoneCull.h"
#include "Clock.h"
#include "Particle.h"
#include "ParticleMgr.h"
#include "RwHelper.h"
#include "Streaming.h"
#include "CdStream.h"
#include "Pad.h"
#include "RenderBuffer.h"
#include <rpworld.h>
#include "WaterLevel.h"


float TEXTURE_ADDU;
float TEXTURE_ADDV;

int32 CWaterLevel::ms_nNoOfWaterLevels;
float CWaterLevel::ms_aWaterZs[48];
CRect CWaterLevel::ms_aWaterRects[48];
uint8 CWaterLevel::aWaterBlockList[WATER_BLOCK_SIZE][WATER_BLOCK_SIZE];
uint8 CWaterLevel::aWaterFineBlockList[WATER_FINEBLOCK_SIZE][WATER_FINEBLOCK_SIZE];
bool CWaterLevel::WavesCalculatedThisFrame;
RpAtomic *CWaterLevel::ms_pWavyAtomic;
RpGeometry *CWaterLevel::apGeomArray[8];
int16 CWaterLevel::nGeomUsed;

bool gbDontRenderWater;

//RwTexture *gpWaterTex;
//RwRaster *gpWaterRaster;

RwTexture *gpWaterTex;
RwRaster *gpWaterRaster;


const float fAdd1 = 180.0f;
const float fAdd2 = 80.0f;
const float fRedMult = 0.6f;
const float fGreenMult = 1.0f;
const float fBlueMult = 1.4f;



void
CWaterLevel::Initialise(Const char *pWaterDat)
{
	ms_nNoOfWaterLevels = 0;
	
	int32 hFile = -1;
	
	do
	{
		hFile = CFileMgr::OpenFile("DATA\\waterpro.dat", "rb");
	}
	while ( hFile < 0 );
	
	if ( hFile > 0 )
	{
		if ( hFile >= 0 )
		{
			CFileMgr::Read(hFile, (char *)&ms_nNoOfWaterLevels, sizeof(ms_nNoOfWaterLevels));
			CFileMgr::Read(hFile, (char *)ms_aWaterZs,	sizeof(ms_aWaterZs));
			CFileMgr::Read(hFile, (char *)ms_aWaterRects, sizeof(ms_aWaterRects));
			CFileMgr::Read(hFile, (char *)aWaterBlockList, sizeof(aWaterBlockList));
			CFileMgr::Read(hFile, (char *)aWaterFineBlockList, sizeof(aWaterFineBlockList));
		}
		
		CFileMgr::CloseFile(hFile);
	}
	
	CTxdStore::PushCurrentTxd();

	int32 slot = CTxdStore::FindTxdSlot("particle");
	CTxdStore::SetCurrentTxd(slot);
	
	if ( gpWaterTex == NULL )
		gpWaterTex = RwTextureRead("water_old", NULL);
	gpWaterRaster = RwTextureGetRaster(gpWaterTex);
	
	CTxdStore::PopCurrentTxd();

	CreateWavyAtomic();
	FreeBoatWakeArray();
	
	printf("Done Initing waterlevels\n");
}

void
CWaterLevel::Shutdown()
{
	FreeBoatWakeArray();
	DestroyWavyAtomic();

	if ( gpWaterTex != NULL )
	{
		RwTextureDestroy(gpWaterTex);
		gpWaterTex = NULL;
	}
}

void
CWaterLevel::CreateWavyAtomic()
{
	RpGeometry *wavyGeometry;
	RpMaterial *wavyMaterial;
	RpTriangle *wavyTriangles;
	RpMorphTarget *wavyMorphTarget;
	RwSphere boundingSphere;	
	RwV3d *wavyVert;

	RwFrame *wavyFrame;
	
	{
		wavyGeometry = RpGeometryCreate(9*9, 8*8*2, rpGEOMETRYTRISTRIP
													|rpGEOMETRYTEXTURED
													|rpGEOMETRYPRELIT
													|rpGEOMETRYMODULATEMATERIALCOLOR);
													
		ASSERT(wavyGeometry != NULL);
	
	}
	
	{
		wavyMaterial = RpMaterialCreate();
		
		ASSERT(wavyMaterial != NULL);
		ASSERT(gpWaterTex   != NULL);
		
		RpMaterialSetTexture(wavyMaterial, gpWaterTex);
	}
	
	{
		wavyTriangles = RpGeometryGetTriangles(wavyGeometry);
		
		ASSERT(wavyTriangles != NULL);
		/*  
			[B]       [C]
			 ***********
			 *       * *
			 *     *   *
			 *   *     *
			 * *       *
			 ***********
			[A]       [D]
		*/
		
		for ( int32 i = 0; i < 8; i++ )
		{
			for ( int32 j = 0; j < 8; j++ )
			{	
				RpGeometryTriangleSetVertexIndices(wavyGeometry, 
						&wavyTriangles[2 * 8*i + 2*j + 0], /*A*/9*i+j+0, /*B*/9*i+j+1,   /*C*/9*i+j+9+1);
                                                                  
				RpGeometryTriangleSetVertexIndices(wavyGeometry,  
						&wavyTriangles[2 * 8*i + 2*j + 1], /*A*/9*i+j+0, /*C*/9*i+j+9+1, /*D*/9*i+j+9  );
	
				RpGeometryTriangleSetMaterial(wavyGeometry, &wavyTriangles[2 * 8*i + 2*j + 0], wavyMaterial);
				RpGeometryTriangleSetMaterial(wavyGeometry, &wavyTriangles[2 * 8*i + 2*j + 1], wavyMaterial);
			}
		}
	}
	

	{
		wavyMorphTarget = RpGeometryGetMorphTarget(wavyGeometry, 0);
		ASSERT(wavyMorphTarget != NULL);
		wavyVert = RpMorphTargetGetVertices(wavyMorphTarget);	
		ASSERT(wavyVert != NULL);
		
		for ( int32 i = 0; i < 9; i++ )
		{
			for ( int32 j = 0; j < 9; j++ )
			{
				wavyVert[9*i+j].x = (float)i * 4.0f;
				wavyVert[9*i+j].y = (float)j * 4.0f;
				wavyVert[9*i+j].z = 0.0f;
			}
		}
		
		RpMorphTargetCalcBoundingSphere(wavyMorphTarget, &boundingSphere);
		RpMorphTargetSetBoundingSphere(wavyMorphTarget, &boundingSphere);
		RpGeometryUnlock(wavyGeometry);
	}
	
	
	{
		wavyFrame = RwFrameCreate();
		ASSERT( wavyFrame != NULL );
		
		ms_pWavyAtomic = RpAtomicCreate();		
		ASSERT( ms_pWavyAtomic != NULL );
		
		RpAtomicSetGeometry(ms_pWavyAtomic, wavyGeometry, 0);
		RpAtomicSetFrame(ms_pWavyAtomic, wavyFrame);
		RpMaterialDestroy(wavyMaterial);
		RpGeometryDestroy(wavyGeometry);
	}
}

void
CWaterLevel::DestroyWavyAtomic()
{
	RwFrame *frame;
	
	frame = RpAtomicGetFrame(ms_pWavyAtomic);
	
	RpAtomicDestroy(ms_pWavyAtomic);
	
	RwFrameDestroy(frame);	
}

bool
CWaterLevel::GetWaterLevel(float fX, float fY, float fZ, float *pfOutLevel, bool bDontCheckZ)
{
	int32 x = WATER_HUGE_X(fX);
	int32 y = WATER_HUGE_Y(fY);
	
	ASSERT( x >= 0 && x < HUGE_SECTOR_SIZE );
	ASSERT( y >= 0 && y < HUGE_SECTOR_SIZE );

	uint8 nBlock = aWaterFineBlockList[x][y];

	if ( nBlock == 128 )
		return false;

	ASSERT( pfOutLevel != NULL );
	*pfOutLevel = ms_aWaterZs[nBlock];

	float fAngle = (CTimer::GetTimeInMilliseconds() & 4095) * (TWOPI / 4096.0f);
	
	float fWave = Sin
	(
		/*( WATER_UNSIGN_Y(fY) - float(y) * MAX_HUGE_SECTORS + WATER_UNSIGN_X(fX) - float(x) * MAX_HUGE_SECTORS )*/ // VC
		(float)( ((int32)fX & (MAX_HUGE_SECTORS-1)) + ((int32)fY & (MAX_HUGE_SECTORS-1)) )
		* (TWOPI / MAX_HUGE_SECTORS ) + fAngle
	);

	float fWindFactor = CWeather::Wind * 0.7f + 0.3f;
	
	*pfOutLevel += fWave * fWindFactor;

	if ( bDontCheckZ == false && (*pfOutLevel - fZ) > 3.0f )
	{
		*pfOutLevel = 0.0f;
		return false;
	}

	return true;
}

bool
CWaterLevel::GetWaterLevelNoWaves(float fX, float fY, float fZ, float *pfOutLevel)
{
	int32 x = WATER_HUGE_X(fX);
	int32 y = WATER_HUGE_Y(fY);
	
	ASSERT( x >= 0 && x < HUGE_SECTOR_SIZE );
	ASSERT( y >= 0 && y < HUGE_SECTOR_SIZE );
	
	uint8 nBlock = aWaterFineBlockList[x][y];
		
	if ( nBlock == 128 )
		return false;
	
	ASSERT( pfOutLevel != NULL );
	*pfOutLevel = ms_aWaterZs[nBlock];

	return true;
}

inline float
_GetWaterDrawDist()
{
	// if z less then 15.0f return 1200.0f 
	if ( TheCamera.GetPosition().z < 15.0f )
		return 1200.0f;

	// if z greater then 60.0f return 2000.0f;
	if ( TheCamera.GetPosition().z > 60.0f )
		return 2000.0f;

	return (TheCamera.GetPosition().z + -15.0f) * 800.0f / 45.0f + 1200.0f;
}

inline float
_GetWavyDrawDist()
{
	if ( FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() )
		return 120.0f;
	else
		return 70.0f;
}

inline void
_GetCamBounds(bool *bUseCamStartY, bool *bUseCamEndY, bool *bUseCamStartX, bool *bUseCamEndX)
{
	if ( TheCamera.GetForward().z > -0.8f )
	{
		if ( Abs(TheCamera.GetForward().x) > Abs(TheCamera.GetForward().y) )
		{
			if ( TheCamera.GetForward().x > 0.0f )
				*bUseCamStartX = true;
			else
				*bUseCamEndX = true;
		}
		else
		{
			if ( TheCamera.GetForward().y > 0.0f )
				*bUseCamStartY = true;
			else
				*bUseCamEndY = true;
		}
	}
}

inline float
SectorRadius(float fSize)
{
	return Sqrt(Pow(fSize, 2) + Pow(fSize, 2));
}

void
CWaterLevel::RenderWater()
{

#ifndef MASTER
	if (gbDontRenderWater)
		return;
#endif

	bool bUseCamEndX   = false;
	bool bUseCamStartY = false;
	
	bool bUseCamStartX = false;
	bool bUseCamEndY   = false;
	
	float fWavySectorMaxRenderDist      = _GetWavyDrawDist();
	float fWavySectorMaxRenderDistSqr   = SQR(fWavySectorMaxRenderDist);
	
	_GetCamBounds(&bUseCamStartY, &bUseCamEndY, &bUseCamStartX, &bUseCamEndX);

	float fHugeSectorMaxRenderDist    = _GetWaterDrawDist();
	float fHugeSectorMaxRenderDistSqr = SQR(fHugeSectorMaxRenderDist);
	
	float windAddUV = CWeather::Wind * 0.0015f + 0.0005f;
	
	
	if ( !CTimer::GetIsPaused() )
	{
#ifdef FIX_BUGS
		TEXTURE_ADDU += (CGeneral::GetRandomNumberInRange(-0.0005f, 0.0005f) + windAddUV) * CTimer::GetTimeStepFix();
		TEXTURE_ADDV += (CGeneral::GetRandomNumberInRange(-0.0005f, 0.0005f) + windAddUV) * CTimer::GetTimeStepFix();
#else
		TEXTURE_ADDU += CGeneral::GetRandomNumberInRange(-0.0005f, 0.0005f) + windAddUV;
		TEXTURE_ADDV += CGeneral::GetRandomNumberInRange(-0.0005f, 0.0005f) + windAddUV;
#endif
	}
	
	if ( TEXTURE_ADDU >= 1.0f )
		TEXTURE_ADDU = 0.0f;
	if ( TEXTURE_ADDV >= 1.0f )
		TEXTURE_ADDV = 0.0f;
	
	WavesCalculatedThisFrame = false;

	RwRGBA color = { 0, 0, 0, 255 };
	
	color.red   = uint32((CTimeCycle::GetDirectionalRed()   * 0.5f + CTimeCycle::GetAmbientRed()  ) * 255.0f);
	color.green = uint32((CTimeCycle::GetDirectionalGreen() * 0.5f + CTimeCycle::GetAmbientGreen()) * 255.0f);
	color.blue  = uint32((CTimeCycle::GetDirectionalBlue()  * 0.5f + CTimeCycle::GetAmbientBlue() ) * 255.0f);

	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
	
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)gpWaterRaster);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE,     (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDZERO);
	
	CVector2D camPos
	(
		TheCamera.GetPosition().x,
		TheCamera.GetPosition().y
	);

	int32 nStartX = WATER_TO_HUGE_SECTOR_X(camPos.x - fHugeSectorMaxRenderDist);
	int32 nEndX   = WATER_TO_HUGE_SECTOR_X(camPos.x + fHugeSectorMaxRenderDist) + 1;
	int32 nStartY = WATER_TO_HUGE_SECTOR_Y(camPos.y - fHugeSectorMaxRenderDist);
	int32 nEndY   = WATER_TO_HUGE_SECTOR_Y(camPos.y + fHugeSectorMaxRenderDist) + 1;

	if ( bUseCamStartX )
		nStartX = WATER_TO_HUGE_SECTOR_X(camPos.x);
	if ( bUseCamEndX )
		nEndX   = WATER_TO_HUGE_SECTOR_X(camPos.x);
	if ( bUseCamStartY )
		nStartY = WATER_TO_HUGE_SECTOR_Y(camPos.y);
	if ( bUseCamEndY )                             
		nEndY   = WATER_TO_HUGE_SECTOR_Y(camPos.y);

	nStartX = clamp(nStartX, 0, MAX_HUGE_SECTORS - 1);
	nEndX   = clamp(nEndX,   0, MAX_HUGE_SECTORS - 1);
	nStartY = clamp(nStartY, 0, MAX_HUGE_SECTORS - 1);
	nEndY   = clamp(nEndY,   0, MAX_HUGE_SECTORS - 1);
	
	for ( int32 x = nStartX; x <= nEndX; x++ )
	{
		for ( int32 y = nStartY; y <= nEndY; y++ )
		{
			if (   !(aWaterBlockList[2*x+0][2*y+0] & 128)
				|| !(aWaterBlockList[2*x+1][2*y+0] & 128)
				|| !(aWaterBlockList[2*x+0][2*y+1] & 128)
				|| !(aWaterBlockList[2*x+1][2*y+1] & 128) )
			{
				float fX = WATER_FROM_HUGE_SECTOR_X(x);
				float fY = WATER_FROM_HUGE_SECTOR_Y(y);

				CVector2D vecHugeSectorCentre
				(
					fX + HUGE_SECTOR_SIZE/2,
					fY + HUGE_SECTOR_SIZE/2
				);

				float fHugeSectorDistToCamSqr = (camPos - vecHugeSectorCentre).MagnitudeSqr();

				if ( fHugeSectorMaxRenderDistSqr > fHugeSectorDistToCamSqr )
				{
					if ( TheCamera.IsSphereVisible(CVector(vecHugeSectorCentre.x, vecHugeSectorCentre.y, 0.0f), SectorRadius(HUGE_SECTOR_SIZE),
							&TheCamera.GetCameraMatrix()) )
					{
						if ( fHugeSectorDistToCamSqr >= SQR(500.0f) /*fHugeSectorNearDist*/ )
						{
							float fZ;
	
							if ( !(aWaterBlockList[2*x+0][2*y+0] & 128) )
								fZ = ms_aWaterZs[ aWaterBlockList[2*x+0][2*y+0] ];
	
							if ( !(aWaterBlockList[2*x+1][2*y+0] & 128) )
								fZ = ms_aWaterZs[ aWaterBlockList[2*x+1][2*y+0] ];
	
							if ( !(aWaterBlockList[2*x+0][2*y+1] & 128) )
								fZ = ms_aWaterZs[ aWaterBlockList[2*x+0][2*y+1] ];
	
							if ( !(aWaterBlockList[2*x+1][2*y+1] & 128) )
								fZ = ms_aWaterZs[ aWaterBlockList[2*x+1][2*y+1] ];
							
							RenderOneFlatHugeWaterPoly(fX, fY, fZ, color);
						}
						else
						{
							for ( int32 x2 = 2*x; x2 <= 2*x+1; x2++ )
							{
								for ( int32 y2 = 2*y; y2 <= 2*y+1; y2++ )
								{
									if ( !(aWaterBlockList[x2][y2] & 128) )
									{
										float fLargeX = WATER_FROM_LARGE_SECTOR_X(x2);
										float fLargeY = WATER_FROM_LARGE_SECTOR_Y(y2);
						
										CVector2D vecLargeSectorCentre
										(
											fLargeX + LARGE_SECTOR_SIZE/2,
											fLargeY + LARGE_SECTOR_SIZE/2
										);
										
										float fLargeSectorDistToCamSqr = (camPos - vecLargeSectorCentre).MagnitudeSqr();
										
										if ( fLargeSectorDistToCamSqr < fHugeSectorMaxRenderDistSqr )
										{
											if ( TheCamera.IsSphereVisible(CVector(vecLargeSectorCentre.x, vecLargeSectorCentre.y, 0.0f), SectorRadius(LARGE_SECTOR_SIZE), //90.879997f,
												&TheCamera.GetCameraMatrix()) )
											{
												// Render four small(32x32) sectors, or one large(64x64).

												//														
												//	       [N]
												//	    ---------
												//	    |0x1|1x1|
												//	[W] --------- [E]
												//	    |0x0|1x0|
												//	    ---------
												//	       [S]
												//	

												if ( fLargeSectorDistToCamSqr < SQR(176.0f) )
												{						
													float fZ;
													
													// WS
													if ( !(aWaterFineBlockList[2*x2+0][2*y2+0] & 128) )
													{
														float fSmallX = fLargeX;
														float fSmallY = fLargeY;
														
														CVector2D vecSmallSectorCentre
														(
															fSmallX + SMALL_SECTOR_SIZE/2,
															fSmallY + SMALL_SECTOR_SIZE/2
														);
														
														float fSmallSectorDistToCamSqr = (camPos - vecSmallSectorCentre).MagnitudeSqr();
														fZ = ms_aWaterZs[ aWaterFineBlockList[2*x2+0][2*y2+0] ];
														
														if ( fSmallSectorDistToCamSqr < fWavySectorMaxRenderDistSqr )
															RenderOneWavySector(fSmallX, fSmallY, fZ, color);
														else
															RenderOneFlatSmallWaterPoly(fSmallX, fSmallY, fZ, color);
													}
													
													// SE
													if ( !(aWaterFineBlockList[2*x2+1][2*y2+0] & 128) )
													{
														float fSmallX = fLargeX + (LARGE_SECTOR_SIZE/2);
														float fSmallY = fLargeY;
														
														CVector2D vecSmallSectorCentre
														(
															fSmallX + SMALL_SECTOR_SIZE/2,
															fSmallY + SMALL_SECTOR_SIZE/2
														);
														
														float fSmallSectorDistToCamSqr = (camPos - vecSmallSectorCentre).MagnitudeSqr();
														fZ = ms_aWaterZs[ aWaterFineBlockList[2*x2+1][2*y2+0] ];
														
														if ( fSmallSectorDistToCamSqr < fWavySectorMaxRenderDistSqr )
															RenderOneWavySector(fSmallX, fSmallY, fZ, color);
														else
															RenderOneFlatSmallWaterPoly(fSmallX, fSmallY, fZ, color);
													}
													
													// WN
													if ( !(aWaterFineBlockList[2*x2+0][2*y2+1] & 128) )
													{
														float fSmallX = fLargeX;
														float fSmallY = fLargeY + (LARGE_SECTOR_SIZE/2);
														
														CVector2D vecSmallSectorCentre
														(
															fSmallX + SMALL_SECTOR_SIZE/2,
															fSmallY + SMALL_SECTOR_SIZE/2
														);
														
														float fSmallSectorDistToCamSqr = (camPos - vecSmallSectorCentre).MagnitudeSqr();
														fZ = ms_aWaterZs[ aWaterFineBlockList[2*x2+0][2*y2+1] ];
														
														if ( fSmallSectorDistToCamSqr < fWavySectorMaxRenderDistSqr )
															RenderOneWavySector(fSmallX, fSmallY, fZ, color);
														else
															RenderOneFlatSmallWaterPoly(fSmallX, fSmallY, fZ, color);
													}
													
													//NE
													if ( !(aWaterFineBlockList[2*x2+1][2*y2+1] & 128) )
													{
														float fSmallX = fLargeX + (LARGE_SECTOR_SIZE/2);
														float fSmallY = fLargeY + (LARGE_SECTOR_SIZE/2);
														
														CVector2D vecSmallSectorCentre
														(
															fSmallX + SMALL_SECTOR_SIZE/2,
															fSmallY + SMALL_SECTOR_SIZE/2
														);
														
														float fSmallSectorDistToCamSqr = (camPos - vecSmallSectorCentre).MagnitudeSqr();
														fZ = ms_aWaterZs[ aWaterFineBlockList[2*x2+1][2*y2+1] ];
														
														if ( fSmallSectorDistToCamSqr < fWavySectorMaxRenderDistSqr )
															RenderOneWavySector(fSmallX, fSmallY, fZ, color);
														else
															RenderOneFlatSmallWaterPoly(fSmallX, fSmallY, fZ, color);
													}
												}
												else
												{
													float fZ;
                                                
													fZ = ms_aWaterZs[ aWaterBlockList[x2][y2] ];
                                                
													RenderOneFlatLargeWaterPoly(fLargeX, fLargeY, fZ, color);
												}
											}	//	if ( TheCamera.IsSphereVisible
										}	//	if ( fLargeSectorDistToCamSqr < fHugeSectorMaxRenderDistSqr )
									}	//	if ( !(aWaterBlockList[x2][y2] & 128) )
								}	//	for ( int32 y2 = 2*y; y2 <= 2*y+1; y2++ )
							}	//	for ( int32 x2 = 2*x; x2 <= 2*x+1; x2++ )
							//
							
						}
					}
				}
			}
		}
	}
	
	/*
	 -----------      ----------------------       ----------------------
	|    [N]    |    |       [ EndY ]       |     |       [  top ]       |
	|           |    |                      |     |                      |
	|[W] [0] [E]|    |[StartX]  []  [ EndX ]|     |[ left ]  []  [ right]|
	|           |    |                      |     |                      |
	|    [S]    |    |       [StartY]       |     |       [bottom]       |
	 -----------      ----------------------       ----------------------
	
	
	[S] [StartY]  [bottom]
	[N] [EndY]    [top]
	[W] [StartX]  [left]
	[E] [EndX]    [right]
	
	[S]    -> [N] && [W]  -> [E]
	bottom -> top && left -> right
	*/

	if ( !bUseCamStartY )
	{
		for ( int32 x = 0; x < 26; x++ )
		{
			for ( int32 y = 0; y < 5; y++ )
			{
				float fX = WATER_SIGN_X(float(x) * EXTRAHUGE_SECTOR_SIZE) - 1280.0f;
				float fY = WATER_SIGN_Y(float(y) * EXTRAHUGE_SECTOR_SIZE) - 1280.0f;
				
				CVector2D vecExtraHugeSectorCentre
				(
					fX + EXTRAHUGE_SECTOR_SIZE/2,
					fY + EXTRAHUGE_SECTOR_SIZE/2
				);
				
				float fCamDistToSector = (vecExtraHugeSectorCentre - camPos).Magnitude();
				
				if ( fCamDistToSector < fHugeSectorMaxRenderDistSqr )
				{
					if ( TheCamera.IsSphereVisible(CVector(vecExtraHugeSectorCentre.x, vecExtraHugeSectorCentre.y, 0.0f), SectorRadius(EXTRAHUGE_SECTOR_SIZE),
							&TheCamera.GetCameraMatrix()) )
					{
						RenderOneFlatExtraHugeWaterPoly(
							vecExtraHugeSectorCentre.x - EXTRAHUGE_SECTOR_SIZE/2,
							vecExtraHugeSectorCentre.y - EXTRAHUGE_SECTOR_SIZE/2,
							0.0f,
							color);
					}
				}
			}
		}
	}
	
	for ( int32 y = 5; y < 21; y++ )
	{
		for ( int32 x = 0; x < 5; x++ )
		{
			float fX = WATER_SIGN_X(float(x) * EXTRAHUGE_SECTOR_SIZE) - 1280.0f;
			float fX2 = WATER_SIGN_X(float(x) * EXTRAHUGE_SECTOR_SIZE) - 1280.0f;
			float fY = WATER_SIGN_Y(float(y) * EXTRAHUGE_SECTOR_SIZE) - 1280.0f;
			
			if ( !bUseCamStartX )
			{
				CVector2D vecExtraHugeSectorCentre
				(
					fX + EXTRAHUGE_SECTOR_SIZE/2,
					fY + EXTRAHUGE_SECTOR_SIZE/2
				);
				
				float fCamDistToSector = (vecExtraHugeSectorCentre - camPos).Magnitude();
				
				if ( fCamDistToSector < fHugeSectorMaxRenderDistSqr )
				{
					if ( TheCamera.IsSphereVisible(CVector(vecExtraHugeSectorCentre.x, vecExtraHugeSectorCentre.y, 0.0f), SectorRadius(EXTRAHUGE_SECTOR_SIZE),
							&TheCamera.GetCameraMatrix()) )
					{
						RenderOneFlatExtraHugeWaterPoly(
							vecExtraHugeSectorCentre.x - EXTRAHUGE_SECTOR_SIZE/2,
							vecExtraHugeSectorCentre.y - EXTRAHUGE_SECTOR_SIZE/2,
							0.0f,
							color);
					}
				}
			}
			
			if ( !bUseCamEndX )
			{
				CVector2D vecExtraHugeSectorCentre
				(
					-(fX2 + EXTRAHUGE_SECTOR_SIZE/2),
					fY + EXTRAHUGE_SECTOR_SIZE/2
				);
				
				float fCamDistToSector = (vecExtraHugeSectorCentre - camPos).Magnitude();
				
				if ( fCamDistToSector < fHugeSectorMaxRenderDistSqr )
				{
					if ( TheCamera.IsSphereVisible(CVector(vecExtraHugeSectorCentre.x, vecExtraHugeSectorCentre.y, 0.0f), SectorRadius(EXTRAHUGE_SECTOR_SIZE),
							&TheCamera.GetCameraMatrix()) )
					{
						RenderOneFlatExtraHugeWaterPoly(
							vecExtraHugeSectorCentre.x - EXTRAHUGE_SECTOR_SIZE/2,
							vecExtraHugeSectorCentre.y - EXTRAHUGE_SECTOR_SIZE/2,
							0.0f,
							color);
					}
				}
			}
		}
	}

	RenderAndEmptyRenderBuffer();
	
	CVector cur_pos = TheCamera.GetPosition();
	
	if ( !CCullZones::CamNoRain()
		&& !CCullZones::PlayerNoRain()
		&& CWeather::NewWeatherType == WEATHER_SUNNY
		&& CClock::GetHours() > 6 && CClock::GetHours() < 20
		&& WavesCalculatedThisFrame)
	{
		static CVector prev_pos(0.0f, 0.0f, 0.0f);
		static CVector prev_front(0.0f, 0.0f, 0.0f);
		static int32 timecounter;

		if ( Abs(prev_pos.x - cur_pos.x) + Abs(prev_pos.y - cur_pos.y) + Abs(prev_pos.z - cur_pos.z) > 1.5f )
		{
			prev_pos = cur_pos;
			timecounter = CTimer::GetTimeInMilliseconds();
		}
		else if ( CTimer::GetTimeInMilliseconds() - timecounter > 5000 )
		{
			static int32 birdgenTime = 0;
			
			if ( CTimer::GetTimeInMilliseconds() - birdgenTime > 1000 )
			{
				birdgenTime = CTimer::GetTimeInMilliseconds();
				
				CVector vecPos = cur_pos;
				
				float fAngle = CGeneral::GetRandomNumberInRange(90.0f, 150.0f);
				
				int32 nRot = CGeneral::GetRandomNumber() % CParticle::SIN_COS_TABLE_SIZE-1;
				
				float fCos = CParticle::Cos(nRot);
				float fSin = CParticle::Sin(nRot);

				vecPos.x += (fCos - fSin) * fAngle;
				vecPos.y += (fSin + fCos) * fAngle;
				vecPos.z += CGeneral::GetRandomNumberInRange(10.0f, 30.0f);
				
				CVector vecDir(CGeneral::GetRandomNumberInRange(-1.0f, 1.0f),
								CGeneral::GetRandomNumberInRange(-1.0f, 1.0f),
								0.0f);
				
				CParticle::AddParticle(PARTICLE_BIRD_FRONT, vecPos, vecDir);
			}
		}
	}
	
	DefinedState();
}

void
CWaterLevel::RenderOneFlatSmallWaterPoly(float fX, float fY, float fZ, RwRGBA const &color)
{
	if ( TempBufferIndicesStored >= TEMPBUFFERINDEXSIZE-6 || TempBufferVerticesStored >= TEMPBUFFERVERTSIZE-4 )
		RenderAndEmptyRenderBuffer();
	
	int32 vidx = TempBufferVerticesStored;
	
	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 0], fX, fY, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 0], TEXTURE_ADDU);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 0], TEXTURE_ADDV);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 0], color.red, color.green, color.blue, color.alpha);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 1], fX, fY + SMALL_SECTOR_SIZE, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 1], TEXTURE_ADDU);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 1], TEXTURE_ADDV + 1.0f);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 1], color.red, color.green, color.blue, color.alpha);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 2], fX + SMALL_SECTOR_SIZE, fY + SMALL_SECTOR_SIZE, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 2], TEXTURE_ADDU + 1.0f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 2], TEXTURE_ADDV + 1.0f);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 2], color.red, color.green, color.blue, color.alpha);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 3], fX + SMALL_SECTOR_SIZE, fY, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 3], TEXTURE_ADDU + 1.0f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 3], TEXTURE_ADDV);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 3], color.red, color.green, color.blue, color.alpha);
	
	
	int32 iidx = TempBufferIndicesStored;

	TempBufferRenderIndexList[iidx + 0] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[iidx + 1] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[iidx + 2] = TempBufferVerticesStored + 1;
	TempBufferRenderIndexList[iidx + 3] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[iidx + 4] = TempBufferVerticesStored + 3;
	TempBufferRenderIndexList[iidx + 5] = TempBufferVerticesStored + 2;
	
	TempBufferVerticesStored += 4;
	TempBufferIndicesStored += 6;
}

void
CWaterLevel::RenderOneFlatLargeWaterPoly(float fX, float fY, float fZ, RwRGBA const &color)
{
	if ( TempBufferIndicesStored >= TEMPBUFFERINDEXSIZE-6 || TempBufferVerticesStored >= TEMPBUFFERVERTSIZE-4 )
		RenderAndEmptyRenderBuffer();
	
	int32 vidx = TempBufferVerticesStored;
	
	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 0], fX, fY, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 0], TEXTURE_ADDU);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 0], TEXTURE_ADDV);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 0], color.red, color.green, color.blue, color.alpha);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 1], fX, fY + LARGE_SECTOR_SIZE, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 1], TEXTURE_ADDU);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 1], TEXTURE_ADDV + 2.0f);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 1], color.red, color.green, color.blue, color.alpha);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 2], fX + LARGE_SECTOR_SIZE, fY + LARGE_SECTOR_SIZE, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 2], TEXTURE_ADDU + 2.0f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 2], TEXTURE_ADDV + 2.0f);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 2], color.red, color.green, color.blue, color.alpha);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 3], fX + LARGE_SECTOR_SIZE, fY, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 3], TEXTURE_ADDU + 2.0f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 3], TEXTURE_ADDV);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 3], color.red, color.green, color.blue, color.alpha);


	int32 iidx = TempBufferIndicesStored;

	TempBufferRenderIndexList[iidx + 0] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[iidx + 1] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[iidx + 2] = TempBufferVerticesStored + 1;
	TempBufferRenderIndexList[iidx + 3] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[iidx + 4] = TempBufferVerticesStored + 3;
	TempBufferRenderIndexList[iidx + 5] = TempBufferVerticesStored + 2;
	
	TempBufferVerticesStored += 4;
	TempBufferIndicesStored += 6;
}

void
CWaterLevel::RenderOneFlatHugeWaterPoly(float fX, float fY, float fZ, RwRGBA const &color)
{
	if ( TempBufferIndicesStored >= TEMPBUFFERINDEXSIZE-6 || TempBufferVerticesStored >= TEMPBUFFERVERTSIZE-4 )
		RenderAndEmptyRenderBuffer();
	
	int32 vidx = TempBufferVerticesStored;

	
	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 0], fX, fY, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 0], TEXTURE_ADDU);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 0], TEXTURE_ADDV);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 0], color.red, color.green, color.blue, 255);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 1], fX, fY + HUGE_SECTOR_SIZE, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 1], TEXTURE_ADDU);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 1], TEXTURE_ADDV + 4.0f);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 1], color.red, color.green, color.blue, 255);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 2], fX + HUGE_SECTOR_SIZE, fY + HUGE_SECTOR_SIZE, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 2], TEXTURE_ADDU + 4.0f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 2], TEXTURE_ADDV + 4.0f);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 2], color.red, color.green, color.blue, 255);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 3], fX + HUGE_SECTOR_SIZE, fY, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 3], TEXTURE_ADDU + 4.0f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 3], TEXTURE_ADDV);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 3], color.red, color.green, color.blue, 255);


	int32 iidx = TempBufferIndicesStored;

	TempBufferRenderIndexList[iidx + 0] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[iidx + 1] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[iidx + 2] = TempBufferVerticesStored + 1;
	TempBufferRenderIndexList[iidx + 3] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[iidx + 4] = TempBufferVerticesStored + 3;
	TempBufferRenderIndexList[iidx + 5] = TempBufferVerticesStored + 2;
	
	TempBufferVerticesStored += 4;
	TempBufferIndicesStored += 6;
}

void
CWaterLevel::RenderOneFlatExtraHugeWaterPoly(float fX, float fY, float fZ, RwRGBA const &color)
{
	if ( TempBufferIndicesStored >= TEMPBUFFERINDEXSIZE-6 || TempBufferVerticesStored >= TEMPBUFFERVERTSIZE-4 )
		RenderAndEmptyRenderBuffer();
	
	int32 vidx = TempBufferVerticesStored;

	
	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 0], fX, fY, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 0], TEXTURE_ADDU);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 0], TEXTURE_ADDV);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 0], color.red, color.green, color.blue, 255);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 1], fX, fY + EXTRAHUGE_SECTOR_SIZE, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 1], TEXTURE_ADDU);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 1], TEXTURE_ADDV + 8.0f);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 1], color.red, color.green, color.blue, 255);

	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 2], fX + EXTRAHUGE_SECTOR_SIZE, fY + EXTRAHUGE_SECTOR_SIZE, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 2], TEXTURE_ADDU + 8.0f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 2], TEXTURE_ADDV + 8.0f);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 2], color.red, color.green, color.blue, 255);
	
	RwIm3DVertexSetPos  (&TempBufferRenderVertices[vidx + 3], fX + EXTRAHUGE_SECTOR_SIZE, fY, fZ - WATER_Z_OFFSET);
	RwIm3DVertexSetU    (&TempBufferRenderVertices[vidx + 3], TEXTURE_ADDU + 8.0f);
	RwIm3DVertexSetV    (&TempBufferRenderVertices[vidx + 3], TEXTURE_ADDV);
	RwIm3DVertexSetRGBA (&TempBufferRenderVertices[vidx + 3], color.red, color.green, color.blue, 255);


	int32 iidx = TempBufferIndicesStored;

	TempBufferRenderIndexList[iidx + 0] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[iidx + 1] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[iidx + 2] = TempBufferVerticesStored + 1;
	TempBufferRenderIndexList[iidx + 3] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[iidx + 4] = TempBufferVerticesStored + 3;
	TempBufferRenderIndexList[iidx + 5] = TempBufferVerticesStored + 2;
	
	TempBufferVerticesStored += 4;
	TempBufferIndicesStored += 6;
}

void
CWaterLevel::RenderOneWavySector(float fX, float fY, float fZ, RwRGBA const &color, bool bUnk)
{
	float fAngle = (CTimer::GetTimeInMilliseconds() & 4095) * (TWOPI / 4096.0f);	
	
	if ( !WavesCalculatedThisFrame )
	{
		nGeomUsed = 0;
		
		WavesCalculatedThisFrame = true;
		
		CBoat::FillBoatList();
		
		ASSERT( ms_pWavyAtomic != NULL );

		RpGeometry *geometry = RpAtomicGetGeometry(ms_pWavyAtomic);
		
		ASSERT( geometry != NULL );

		RwRGBA      *wavyPreLights = RpGeometryGetPreLightColors(geometry);
		RwTexCoords *wavyTexCoords = RpGeometryGetVertexTexCoords(geometry, rwTEXTURECOORDINATEINDEX0);
		RwV3d       *wavyVertices  = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry, 0));
		
		ASSERT( wavyPreLights != NULL );
		ASSERT( wavyTexCoords != NULL );
		ASSERT( wavyVertices  != NULL );

		RpGeometryLock(geometry, rpGEOMETRYLOCKVERTICES
								| rpGEOMETRYLOCKPRELIGHT
								| rpGEOMETRYLOCKTEXCOORDS);
		
		for ( int32 i = 0; i < 9; i++ )
		{
			for ( int32 j = 0; j < 9; j++ )
			{
				wavyTexCoords[9*i+j].u = float(i) / 8 + TEXTURE_ADDV;
				wavyTexCoords[9*i+j].v = float(j) / 8 + TEXTURE_ADDU;
				RwRGBAAssign(&wavyPreLights[9*i+j], &color);

				wavyVertices[9*i+j].z = ( CWeather::Wind * 0.7f + 0.3f )
										* ( Sin(float(i + j) * DEGTORAD(45.0f) + fAngle) )
										+ ( CWeather::Wind * 0.2f * Sin(float(j - i) * PI + (2.0f * fAngle)) );
			}
		}
		
		RpGeometryUnlock(geometry);
	}
	
	static CBoat *apBoatList[4] = { NULL };
	
	if ( apGeomArray[0]
		&& nGeomUsed < MAX_BOAT_WAKES
		&& CBoat::IsSectorAffectedByWake(
			CVector2D(fX + (SMALL_SECTOR_SIZE / 2), fY + (SMALL_SECTOR_SIZE / 2)),
			SMALL_SECTOR_SIZE / 2,
			apBoatList) )
	{
		float fWakeColor = fAdd1 - Max(255.0f - float(color.blue + color.red + color.green) / 3, fAdd2);
		
		RpGeometry *wavyGeometry = RpAtomicGetGeometry(ms_pWavyAtomic);	
		RpGeometry *geom  = apGeomArray[nGeomUsed++];
		
		ASSERT( wavyGeometry != NULL );
		ASSERT( geom != NULL );
		
		RpAtomic *atomic = RpAtomicCreate();
		ASSERT( atomic != NULL );
		
		RpAtomicSetGeometry(atomic, geom, 0);
		
		RwFrame *frame = RwFrameCreate();
		ASSERT( frame != NULL );
		
		RwMatrixCopy(RwFrameGetMatrix(frame), RwFrameGetMatrix(RpAtomicGetFrame(ms_pWavyAtomic)));
		RpAtomicSetFrame(atomic, frame);
		
		RwTexCoords *geomTexCoords  = RpGeometryGetVertexTexCoords(geom, rwTEXTURECOORDINATEINDEX0);
		RwTexCoords *wavyTexCoord   = RpGeometryGetVertexTexCoords(wavyGeometry, rwTEXTURECOORDINATEINDEX0);
		RwRGBA *geomPreLights       = RpGeometryGetPreLightColors(geom);
		RwV3d  *geomVertices        = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geom, 0));
		RwV3d  *wavyVertices        = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(wavyGeometry, 0));
		
		ASSERT( geomTexCoords != NULL );
		ASSERT( wavyTexCoord  != NULL );
		ASSERT( geomPreLights != NULL );
		ASSERT( geomVertices  != NULL );
		ASSERT( wavyVertices  != NULL );

		RpGeometryLock(geom, rpGEOMETRYLOCKVERTICES | rpGEOMETRYLOCKPRELIGHT | rpGEOMETRYLOCKTEXCOORDS);
		
		for ( int32 i = 0; i < 9; i++ )
		{
			for ( int32 j = 0; j < 9; j++ )
			{
				geomTexCoords[9*i+j] = wavyTexCoord[9*i+j];

				float fVertexX = (float)i * 4.0f + fX;
				float fVertexY = (float)j * 4.0f + fY;

				float fDistMult = 0.0f;
		
				for ( int32 k = 0; k < 4; k++ )
				{
					if ( apBoatList[k] != NULL )
						fDistMult += CBoat::IsVertexAffectedByWake(CVector(fVertexX, fVertexY, 0.0f), apBoatList[k]);
				}
				
				if ( fDistMult > 0.0f )
				{
					RwRGBA wakeColor;
					
					RwRGBAAssign(&wakeColor, &color);

					wakeColor.red   = Min(color.red   + int32(fWakeColor * fRedMult   * fDistMult), 255);
					wakeColor.green = Min(color.green + int32(fWakeColor * fGreenMult * fDistMult), 255);
					wakeColor.blue  = Min(color.blue  + int32(fWakeColor * fBlueMult  * fDistMult), 255);
					
					RwRGBAAssign(&geomPreLights[9*i+j], &wakeColor);

				}
				else
					RwRGBAAssign(&geomPreLights[9*i+j], &color);
				
				
				geomVertices[9*i+j].z = wavyVertices[9*i+j].z;
			}
		}
		
		RpGeometryUnlock(geom);
		
		
		RwV3d pos = {0.0f, 0.0f, 0.0f};

		pos.x = fX;
		pos.z = fZ;
		pos.y = fY;

		RwFrameTranslate(RpAtomicGetFrame(atomic), &pos, rwCOMBINEREPLACE);

		RpAtomicRender(atomic);
		
		RpAtomicDestroy(atomic);
		RwFrameDestroy(frame);
	}
	else
	{
		RwV3d pos = { 0.0f, 0.0f, 0.0f };

		pos.x = fX;
		pos.y = fY;
		pos.z = fZ;
		
		ASSERT( ms_pWavyAtomic != NULL );
		
		RwFrameTranslate(RpAtomicGetFrame(ms_pWavyAtomic), &pos, rwCOMBINEREPLACE);
		
		RpAtomicRender(ms_pWavyAtomic);
	}
}

float
CWaterLevel::CalcDistanceToWater(float fX, float fY)
{
	const float fSectorMaxRenderDist = 75.0f;
	
	int32 nStartX = WATER_TO_SMALL_SECTOR_X(fX - fSectorMaxRenderDist) - 1;
	int32 nEndX   = WATER_TO_SMALL_SECTOR_X(fX + fSectorMaxRenderDist) + 1;
	int32 nStartY = WATER_TO_SMALL_SECTOR_Y(fY - fSectorMaxRenderDist) - 1;
	int32 nEndY   = WATER_TO_SMALL_SECTOR_Y(fY + fSectorMaxRenderDist) + 1;
	
	nStartX = clamp(nStartX, 0, MAX_SMALL_SECTORS - 1);
	nEndX   = clamp(nEndX,   0, MAX_SMALL_SECTORS - 1);
	nStartY = clamp(nStartY, 0, MAX_SMALL_SECTORS - 1);
	nEndY   = clamp(nEndY,   0, MAX_SMALL_SECTORS - 1);
	
	float fDistSqr = 1.0e10f;
	
	for ( int32 x = nStartX; x <= nEndX; x++ )
	{
		for ( int32 y = nStartY; y <= nEndY; y++ )
		{
			if ( !(aWaterFineBlockList[x][y] & 128) )
			{				
				float fSectorX = WATER_FROM_SMALL_SECTOR_X(x);
				float fSectorY = WATER_FROM_SMALL_SECTOR_Y(y);
				
				CVector2D vecDist
				(
					fSectorX + SMALL_SECTOR_SIZE - fX,
					fSectorY + SMALL_SECTOR_SIZE - fY
				);
				
				fDistSqr = Min(vecDist.MagnitudeSqr(), fDistSqr);
			}
		}
	}

	return clamp(Sqrt(fDistSqr) - 23.0f, 0.0f, fSectorMaxRenderDist);
}

void
CWaterLevel::RenderAndEmptyRenderBuffer()
{
	if ( TempBufferVerticesStored )
	{
		LittleTest();

		if ( RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, NULL, rwIM3D_VERTEXUV) )
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
			RwIm3DEnd();
		}
	}
	
	TempBufferIndicesStored = 0;
	TempBufferVerticesStored = 0;
}

void
CWaterLevel::AllocateBoatWakeArray()
{
	CStreaming::MakeSpaceFor(14 * CDSTREAM_SECTOR_SIZE); 

	ASSERT(ms_pWavyAtomic != NULL );
	
	RpGeometry    *wavyGeometry    = RpAtomicGetGeometry(ms_pWavyAtomic);
	ASSERT(wavyGeometry    != NULL );
	RpMorphTarget *wavyMorphTarget = RpGeometryGetMorphTarget(wavyGeometry, 0);
	RpMaterial    *wavyMaterial    = RpGeometryGetMaterial(wavyGeometry, 0);

	ASSERT(wavyMorphTarget != NULL );
	ASSERT(wavyMaterial    != NULL );

	for ( int32 geom = 0; geom < MAX_BOAT_WAKES; geom++ )
	{
		if ( apGeomArray[geom] == NULL )
		{
			apGeomArray[geom] = RpGeometryCreate(9*9, 8*8*2, rpGEOMETRYTRISTRIP
															| rpGEOMETRYPRELIT
															| rpGEOMETRYMODULATEMATERIALCOLOR
															| rpGEOMETRYTEXTURED);
			ASSERT(apGeomArray[geom] != NULL);

			RpTriangle *geomTriangles = RpGeometryGetTriangles(apGeomArray[geom]);
			
			ASSERT( geomTriangles != NULL );

			for ( int32 i = 0; i < 8; i++ )
			{
				for ( int32 j = 0; j < 8; j++ )
				{
					
					/*  
						[B]       [C]
						 ***********
						 *       * *
						 *     *   *
						 *   *     *
						 * *       *
						 ***********
						[A]       [D]
					*/
					
					
					RpGeometryTriangleSetVertexIndices(apGeomArray[geom],
						&geomTriangles[2 * 8*i + 2*j + 0], /*A*/i*9+j+0, /*B*/i*9+j+1,   /*C*/i*9+j+9+1);
					
					RpGeometryTriangleSetVertexIndices(apGeomArray[geom],
						&geomTriangles[2 * 8*i + 2*j + 1], /*A*/i*9+j+0, /*C*/i*9+j+9+1, /*D*/i*9+j+9  );

					RpGeometryTriangleSetMaterial(apGeomArray[geom], &geomTriangles[2 * 8*i + 2*j + 0], wavyMaterial);
					
					RpGeometryTriangleSetMaterial(apGeomArray[geom], &geomTriangles[2 * 8*i + 2*j + 1], wavyMaterial);
				}
			}

			RpMorphTarget *geomMorphTarget = RpGeometryGetMorphTarget(apGeomArray[geom], 0);
			RwV3d         *geomVertices    = RpMorphTargetGetVertices(geomMorphTarget);
			
			ASSERT( geomMorphTarget != NULL );
			ASSERT( geomVertices != NULL );

			for ( int32 i = 0; i < 9; i++ )
			{
				for ( int32 j = 0; j < 9; j++ )
				{
					geomVertices[9*i+j].x = (float)i * 4.0f;
					geomVertices[9*i+j].y = (float)j * 4.0f;
					geomVertices[9*i+j].z = 0.0f;
				}
			}

			RpMorphTargetSetBoundingSphere(geomMorphTarget, RpMorphTargetGetBoundingSphere(wavyMorphTarget));			
			RpGeometryUnlock(apGeomArray[geom]);
		}
	}
}

void
CWaterLevel::FreeBoatWakeArray()
{
	for ( int32 i = 0; i < MAX_BOAT_WAKES; i++ )
	{
		if ( apGeomArray[i] != NULL )
		{
			RpGeometryDestroy(apGeomArray[i]);
			apGeomArray[i] = NULL;
		}
	}
	
	nGeomUsed = 0;  
}
