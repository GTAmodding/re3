#pragma once

#define MAX_STOREDSHADOWS    48
#define MAX_POLYBUNCHES      300
#define MAX_STATICSHADOWS    64
#define MAX_PERMAMENTSHADOWS 48


class CEntity;

enum eShadowType
{
	SHADOWTYPE_NONE = 0,
	SHADOWTYPE_DARK,
	SHADOWTYPE_ADDITIVE,
	SHADOWTYPE_INVCOLOR
};

enum eShadowTextureType
{
    SHADOWTEX_NONE = 0,
    SHADOWTEX_CAR,
    SHADOWTEX_PED,
    SHADOWTEX_EXPLOSION,
    SHADOWTEX_HELI,
    SHADOWTEX_HEADLIGHTS,
    SHADOWTEX_BLOOD
};

class CStoredShadow
{
public:
	CVector m_vecPos;
	CVector2D m_vecFront;
	CVector2D m_vecSide;
	float m_fZDistance;
	float m_fScale;
	int16 m_nIntensity;
	uint8 m_ShadowType;
	uint8 m_nRed;
	uint8 m_nGreen;
	uint8 m_nBlue;
	struct
	{
		uint8 bDrawOnWater      : 1;
		uint8 bRendered         : 1;
		//uint8 bDrawOnBuildings  : 1;
	} m_nFlags;
	RwTexture *m_pTexture;

    CStoredShadow()
	{  }
};

VALIDATE_SIZE(CStoredShadow, 0x30);

class CPolyBunch
{
public:
	int16 m_nNumVerts;
	CVector m_aVerts[7];
	uint8 m_aU[7];
	uint8 m_aV[7];
	CPolyBunch *m_pNext;

    CPolyBunch()
	{  }
};

VALIDATE_SIZE(CPolyBunch, 0x6C);

class CStaticShadow
{
public:
	uint32 m_nId;
	CPolyBunch *m_pPolyBunch;
	uint32 m_nTimeCreated;
	CVector m_vecPosn;
	CVector2D m_vecFront;
	CVector2D m_vecSide;
	float m_fZDistance;
	float m_fScale;
	uint8 m_nType;
	int16 m_nIntensity; // unsigned ?
	uint8 m_nRed;
	uint8 m_nGreen;
	uint8 m_nBlue;
	bool m_bJustCreated;
	bool m_bRendered;
	bool m_bTemp;
	RwTexture *m_pTexture;

	CStaticShadow()
	{  }

	void Free();
};

VALIDATE_SIZE(CStaticShadow, 0x40);

class CPermanentShadow
{
public:
	CVector m_vecPos;
	CVector2D m_vecFront;
	CVector2D m_vecSide;
	float m_fZDistance;
	float m_fScale;
	int16 m_nIntensity;
	uint8 m_nType;	// eShadowType
	uint8 m_nRed;
	uint8 m_nGreen;
	uint8 m_nBlue;
	uint32 m_nTimeCreated;
	uint32 m_nLifeTime;
	RwTexture *m_pTexture;

	CPermanentShadow()
	{  }
};

VALIDATE_SIZE(CPermanentShadow, 0x38);

class CPtrList;
class CAutomobile;
class CPed;

class CShadows
{
public:
	static int16            ShadowsStoredToBeRendered;
	static CStoredShadow    asShadowsStored  [MAX_STOREDSHADOWS];
	static CPolyBunch       aPolyBunches     [MAX_POLYBUNCHES];
	static CStaticShadow    aStaticShadows   [MAX_STATICSHADOWS];
	static CPolyBunch      *pEmptyBunchList;
	static CPermanentShadow aPermanentShadows[MAX_PERMAMENTSHADOWS];

	static void Init                         (void);
	static void Shutdown                     (void);
	static void AddPermanentShadow           (            uint8 ShadowType, RwTexture *pTexture, CVector *pPosn, float fFrontX, float fFrontY, float fSideX, float fSideY, int16 nIntensity, uint8 nRed, uint8 nGreen, uint8 nBlue, float fZDistance, uint32 nTime, float fScale);
	static void StoreStaticShadow            (uint32 nID, uint8 ShadowType, RwTexture *pTexture, Const CVector *pPosn, float fFrontX, float fFrontY, float fSideX, float fSideY, int16 nIntensity, uint8 nRed, uint8 nGreen, uint8 nBlue, float fZDistance,               float fScale, float fDrawDistance, bool bTempShadow, float fUpDistance);
	static void StoreShadowToBeRendered      (            uint8 ShadowType,                      CVector *pPosn, float fFrontX, float fFrontY, float fSideX, float fSideY, int16 nIntensity, uint8 nRed, uint8 nGreen, uint8 nBlue);
	static void StoreShadowToBeRendered      (            uint8 ShadowType, RwTexture *pTexture, CVector *pPosn, float fFrontX, float fFrontY, float fSideX, float fSideY, int16 nIntensity, uint8 nRed, uint8 nGreen, uint8 nBlue, float fZDistance, bool bDrawOnWater, float fScale);
	static void StoreShadowForCar            (CAutomobile *pCar);
	static void StoreCarLightShadow          (CAutomobile *pCar, int32 nID, RwTexture *pTexture, CVector *pPosn, float fFrontX, float fFrontY, float fSideX, float fSideY,                   uint8 nRed, uint8 nGreen, uint8 nBlue, float fMaxViewAngle);
	static void StoreShadowForPed            (CPed        *pPed,     float fDisplacementX, float fDisplacementY, float fFrontX, float fFrontY, float fSideX, float fSideY);
	static void StoreShadowForPedObject      (CEntity *pPedObject,   float fDisplacementX, float fDisplacementY, float fFrontX, float fFrontY, float fSideX, float fSideY);
	static void StoreShadowForTree           (CEntity *pTree);
	static void StoreShadowForPole           (CEntity *pPole, float fOffsetX, float fOffsetY, float fOffsetZ, float fPoleHeight, float fPoleWidth, uint32 nID);
	static void SetRenderModeForShadowType   (uint8 ShadowType);
	static void RenderStoredShadows          (void);
	static void RenderStaticShadows          (void);
	static void GeneratePolysForStaticShadow (int16 nStaticShadowID);
	static void CastShadowSectorList         (CPtrList &PtrList, float fStartX, float fStartY, float fEndX, float fEndY,
																							     CVector *pPosn, float fFrontX, float fFrontY, float fSideX, float fSideY, int16 nIntensity, uint8 nRed, uint8 nGreen, uint8 nBlue, float fZDistance,               float fScale, CPolyBunch **ppPolyBunch);
	static void CastShadowEntity             (CEntity *pEntity,  float fStartX, float fStartY, float fEndX, float fEndY,
																							     CVector *pPosn, float fFrontX, float fFrontY, float fSideX, float fSideY, int16 nIntensity, uint8 nRed, uint8 nGreen, uint8 nBlue, float fZDistance,               float fScale, CPolyBunch **ppPolyBunch);
	static void UpdateStaticShadows          (void);
	static void UpdatePermanentShadows       (void);
	static void CalcPedShadowValues            (CVector vecLightDir, float *pfFrontX, float *pfFrontY, float *pfSideX, float *pfSideY, float *pfDisplacementX, float *pfDisplacementY);
	static void RenderExtraPlayerShadows     (void);
	static void TidyUpShadows                (void);
	static void RenderIndicatorShadow        (uint32 nID, uint8 ShadowType, RwTexture *pTexture,  CVector *pPosn, float fFrontX, float fFrontY, float fSideX, float fSideY, int16 nIntensity);
};

extern RwTexture *gpShadowCarTex;
extern RwTexture *gpShadowPedTex;
extern RwTexture *gpShadowHeliTex;
extern RwTexture *gpShadowExplosionTex;
extern RwTexture *gpShadowHeadLightsTex;
extern RwTexture *gpOutline1Tex;
extern RwTexture *gpOutline2Tex;
extern RwTexture *gpOutline3Tex;
extern RwTexture *gpBloodPoolTex;
extern RwTexture *gpReflectionTex;
extern RwTexture *gpGoalMarkerTex;
extern RwTexture *gpWalkDontTex;
extern RwTexture *gpCrackedGlassTex;
extern RwTexture *gpPostShadowTex;
extern RwTexture *gpGoalTex;
