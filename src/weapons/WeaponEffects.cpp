#include "common.h"

#include "WeaponEffects.h"
#include "TxdStore.h"
#include "Sprite.h"

RwTexture *gpCrossHairTex;
RwRaster *gpCrossHairRaster;

CWeaponEffects gCrossHair;

CWeaponEffects::CWeaponEffects()
{
	
}

CWeaponEffects::~CWeaponEffects()
{
	
}

void
CWeaponEffects::Init(void)
{
	gCrossHair.m_bActive = false;
	gCrossHair.m_vecPos = CVector(0.0f, 0.0f, 0.0f);
	gCrossHair.m_nRed = 0;
	gCrossHair.m_nGreen = 0;
	gCrossHair.m_nBlue = 0;
	gCrossHair.m_nAlpha = 255;
	gCrossHair.m_fSize = 1.0f;
	gCrossHair.m_fRotation = 0.0f;
	
	
	CTxdStore::PushCurrentTxd();
	int32 slot = CTxdStore::FindTxdSlot("particle");
	CTxdStore::SetCurrentTxd(slot);
	
	gpCrossHairTex    = RwTextureRead("crosshair", nil);
	gpCrossHairRaster = RwTextureGetRaster(gpCrossHairTex);
	
	CTxdStore::PopCurrentTxd();
}

void
CWeaponEffects::Shutdown(void)
{
	RwTextureDestroy(gpCrossHairTex);
#if GTA_VERSION >= GTA3_PC_11
	gpCrossHairTex = nil;
#endif
}

void
CWeaponEffects::MarkTarget(CVector pos, uint8 red, uint8 green, uint8 blue, uint8 alpha, float size)
{
	gCrossHair.m_bActive = true;
	gCrossHair.m_vecPos = pos;
	gCrossHair.m_nRed = red;
	gCrossHair.m_nGreen = green;
	gCrossHair.m_nBlue = blue;
	gCrossHair.m_nAlpha = alpha;
	gCrossHair.m_fSize = size;
}

void
CWeaponEffects::ClearCrossHair(void)
{
	gCrossHair.m_bActive = false;
}

void
CWeaponEffects::Render(void)
{
	if ( gCrossHair.m_bActive )
	{
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void *)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void *)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void *)gpCrossHairRaster);

		RwV3d pos;
		float w, h;
		if ( CSprite::CalcScreenCoors(gCrossHair.m_vecPos, &pos, &w, &h, true) )
		{
			float recipz = 1.0f / pos.z;
			CSprite::RenderOneXLUSprite(pos.x, pos.y, pos.z,
				gCrossHair.m_fSize * w, gCrossHair.m_fSize * h,
				gCrossHair.m_nRed, gCrossHair.m_nGreen, gCrossHair.m_nBlue, 255,
				recipz, 255);
		}
		
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void *)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void *)rwBLENDINVSRCALPHA);
	}
}