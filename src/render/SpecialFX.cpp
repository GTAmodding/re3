#include "common.h"
#include "patcher.h"
#include "SpecialFX.h"
#include "Timer.h"
#include "Sprite.h"
#include "Font.h"
#include "Text.h"
#include "TxdStore.h"
#include "FileMgr.h"
#include "FileLoader.h"
#include "Lights.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "Particle.h"
#include "General.h"
#include "Camera.h"

WRAPPER void CSpecialFX::Render(void) { EAXJMP(0x518DC0); }
WRAPPER void CSpecialFX::Update(void) { EAXJMP(0x518D40); }

WRAPPER void CMotionBlurStreaks::RegisterStreak(int32 id, uint8 r, uint8 g, uint8 b, CVector p1, CVector p2) { EAXJMP(0x519460); }


CBulletTrace (&CBulletTraces::aTraces)[16] = *(CBulletTrace(*)[16])*(uintptr*)0x72B1B8;

WRAPPER void CBulletTraces::Init(void) { EAXJMP(0x518DE0); }

WRAPPER void CBrightLights::RegisterOne(CVector pos, CVector up, CVector right, CVector fwd, uint8 type, uint8 unk1, uint8 unk2, uint8 unk3) { EAXJMP(0x51A410); }

RpAtomic *
MarkerAtomicCB(RpAtomic *atomic, void *data)
{
	*(RpAtomic**)data = atomic;
	return atomic;
}

bool
C3dMarker::AddMarker(uint32 identifier, uint16 type, float fSize, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate)
{
	m_nIdentifier = identifier;

	m_Matrix.SetUnity();

	RpAtomic *origAtomic;
	origAtomic = nil;
	RpClumpForAllAtomics(C3dMarkers::m_pRpClumpArray[type], MarkerAtomicCB, &origAtomic);

	RpAtomic *atomic = RpAtomicClone(origAtomic);
	RwFrame *frame = RwFrameCreate();
	RpAtomicSetFrame(atomic, frame);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
	
	RpGeometry *geometry = RpAtomicGetGeometry(atomic);
	RpGeometrySetFlags(geometry, RpGeometryGetFlags(geometry) | rpGEOMETRYMODULATEMATERIALCOLOR);

	m_pAtomic = atomic;
	m_Matrix.Attach(RwFrameGetMatrix(RpAtomicGetFrame(m_pAtomic)));
	m_pMaterial = RpGeometryGetMaterial(geometry, 0);
	m_fSize = fSize;
	m_fStdSize = m_fSize;
	m_Color.red = r;
	m_Color.green = g;
	m_Color.blue = b;
	m_Color.alpha = a;
	m_nPulsePeriod = pulsePeriod;
	m_fPulseFraction = pulseFraction;
	m_nRotateRate = rotateRate;
	m_nStartTime = CTimer::GetTimeInMilliseconds();
	m_nType = type;
	return m_pAtomic != nil;
}

void
C3dMarker::DeleteMarkerObject()
{
	RwFrame *frame;

	m_nIdentifier = 0;
	m_nStartTime = 0;
	m_bIsUsed = false;
	m_nType = MARKERTYPE_INVALID;

	frame = RpAtomicGetFrame(m_pAtomic);
	RpAtomicDestroy(m_pAtomic);
	RwFrameDestroy(frame);
	m_pAtomic = nil;
}

void
C3dMarker::Render()
{
	if (m_pAtomic == nil) return;

	RwRGBA *color = RpMaterialGetColor(m_pMaterial);
	*color = m_Color;

	m_Matrix.UpdateRW();

	CMatrix matrix;
	matrix.Attach(m_Matrix.m_attachment);
	matrix.Scale(m_fSize);
	matrix.UpdateRW();

	RwFrameUpdateObjects(RpAtomicGetFrame(m_pAtomic));
	SetBrightMarkerColours(m_fBrightness);
	if (m_nType != MARKERTYPE_ARROW)
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RpAtomicRender(m_pAtomic);
	if (m_nType != MARKERTYPE_ARROW)
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	ReSetAmbientAndDirectionalColours();
}

C3dMarker(&C3dMarkers::m_aMarkerArray)[NUM3DMARKERS] = *(C3dMarker(*)[NUM3DMARKERS])*(uintptr*)0x72D408;
int32 &C3dMarkers::NumActiveMarkers = *(int32*)0x8F2A08;
RpClump* (&C3dMarkers::m_pRpClumpArray)[NUMMARKERTYPES] = *(RpClump*(*)[NUMMARKERTYPES])*(uintptr*)0x8E2888;

void
C3dMarkers::Init()
{
	for (int i = 0; i < NUM3DMARKERS; i++) {
		m_aMarkerArray[i].m_pAtomic = nil;
		m_aMarkerArray[i].m_nType = MARKERTYPE_INVALID;
		m_aMarkerArray[i].m_bIsUsed = false;
		m_aMarkerArray[i].m_nIdentifier = 0;
		m_aMarkerArray[i].m_Color.red = 255;
		m_aMarkerArray[i].m_Color.green = 255;
		m_aMarkerArray[i].m_Color.blue = 255;
		m_aMarkerArray[i].m_Color.alpha = 255;
		m_aMarkerArray[i].m_nPulsePeriod = 1024;
		m_aMarkerArray[i].m_nRotateRate = 5;
		m_aMarkerArray[i].m_nStartTime = 0;
		m_aMarkerArray[i].m_fPulseFraction = 0.25f;
		m_aMarkerArray[i].m_fStdSize = 1.0f;
		m_aMarkerArray[i].m_fSize = 1.0f;
		m_aMarkerArray[i].m_fBrightness = 1.0f;
		m_aMarkerArray[i].m_fCameraRange = 0.0f;
	}
	NumActiveMarkers = 0;
	int txdSlot = CTxdStore::FindTxdSlot("particle");
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(txdSlot);
	CFileMgr::ChangeDir("\\");
	m_pRpClumpArray[MARKERTYPE_ARROW] = CFileLoader::LoadAtomicFile2Return("models/generic/arrow.dff");
	m_pRpClumpArray[MARKERTYPE_CYLINDER] = CFileLoader::LoadAtomicFile2Return("models/generic/zonecylb.dff");
	CTxdStore::PopCurrentTxd();
}

void
C3dMarkers::Shutdown()
{
	for (int i = 0; i < NUM3DMARKERS; i++) {
		if (m_aMarkerArray[i].m_pAtomic != nil)
			m_aMarkerArray[i].DeleteMarkerObject();
	}

	for (int i = 0; i < NUMMARKERTYPES; i++) {
		if (m_pRpClumpArray[i] != nil)
			RpClumpDestroy(m_pRpClumpArray[i]);
	}
}

void
C3dMarkers::Render()
{
	NumActiveMarkers = 0;
	ActivateDirectional();
	for (int i = 0; i < NUM3DMARKERS; i++) {
		if (m_aMarkerArray[i].m_bIsUsed) {
			if (m_aMarkerArray[i].m_fCameraRange < 120.0f)
				m_aMarkerArray[i].Render();
			NumActiveMarkers++;
			m_aMarkerArray[i].m_bIsUsed = false;
		} else if (m_aMarkerArray[i].m_pAtomic != nil) {
			m_aMarkerArray[i].DeleteMarkerObject();
		}
	}
}

C3dMarker *
C3dMarkers::PlaceMarker(uint32 identifier, uint16 type, CVector &pos, float size, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate)
{
	C3dMarker *pMarker;

	pMarker = nil;
	float dist = Sqrt((pos.x - FindPlayerCentreOfWorld(0).x) * (pos.x - FindPlayerCentreOfWorld(0).x) + (pos.y - FindPlayerCentreOfWorld(0).y) * (pos.y - FindPlayerCentreOfWorld(0).y));

	if (type != MARKERTYPE_ARROW && type != MARKERTYPE_CYLINDER) return nil;

	for (int i = 0; i < NUM3DMARKERS; i++) {
		if (!m_aMarkerArray[i].m_bIsUsed && m_aMarkerArray[i].m_nIdentifier == identifier) {
			pMarker = &m_aMarkerArray[i];
			break;
		}
	}

	if (pMarker == nil) {
		for (int i = 0; i < NUM3DMARKERS; i++) {
			if (m_aMarkerArray[i].m_nType == MARKERTYPE_INVALID) {
				pMarker = &m_aMarkerArray[i];
				break;
			}
		}
	}

	if (pMarker == nil && type == MARKERTYPE_ARROW) {
		for (int i = 0; i < NUM3DMARKERS; i++) {
			if (dist < m_aMarkerArray[i].m_fCameraRange && m_aMarkerArray[i].m_nType == MARKERTYPE_ARROW && (pMarker == nil || m_aMarkerArray[i].m_fCameraRange > pMarker->m_fCameraRange)) {
				pMarker = &m_aMarkerArray[i];
				break;
			}
		}

		if (pMarker != nil)
			pMarker->m_nType = MARKERTYPE_INVALID;
	}

	if (pMarker == nil) return pMarker;

	pMarker->m_fCameraRange = dist;
	if (pMarker->m_nIdentifier == identifier && pMarker->m_nType == type) {
		if (type == MARKERTYPE_ARROW) {
			if (dist < 25.0f) {
				if (dist > 5.0f)
					pMarker->m_fStdSize = size - (25.0f - dist) * (0.3f * size) / 20.0f;
				else
					pMarker->m_fStdSize = size - 0.3f * size;
			} else {
				pMarker->m_fStdSize = size;
			}
		} else if (type == MARKERTYPE_CYLINDER) {
			if (dist < size + 12.0f) {
				if (dist > size + 1.0f)
					pMarker->m_Color.alpha = (1.0f - (size + 12.0f - dist) * 0.7f / 11.0f) * (float)a;
				else
					pMarker->m_Color.alpha = (float)a * 0.3f;
			} else {
				pMarker->m_Color.alpha = a;
			}
		}
		float someSin = Sin(TWOPI * (float)((pMarker->m_nPulsePeriod - 1) & (CTimer::GetTimeInMilliseconds() - pMarker->m_nStartTime)) / (float)pMarker->m_nPulsePeriod);
		pMarker->m_fSize = pMarker->m_fStdSize - pulseFraction * pMarker->m_fStdSize * someSin;

		if (type == MARKERTYPE_ARROW) {
			pos.z += 0.25f * pMarker->m_fStdSize * someSin;
		} else if (type == MARKERTYPE_0) {
			if (someSin > 0.0f)
				pMarker->m_Color.alpha = (float)a * 0.7f * someSin + a;
			else
				pMarker->m_Color.alpha = (float)a * 0.4f * someSin + a;
		}
		if (pMarker->m_nRotateRate) {
			RwV3d pos = pMarker->m_Matrix.m_matrix.pos;
			pMarker->m_Matrix.RotateZ(DEGTORAD(pMarker->m_nRotateRate * CTimer::GetTimeStep()));
			pMarker->m_Matrix.GetPosition() = pos;
		}
		if (type == MARKERTYPE_ARROW)
			pMarker->m_Matrix.GetPosition() = pos;
		pMarker->m_bIsUsed = true;
		return pMarker;
	}

	if (pMarker->m_nIdentifier != 0)
		pMarker->DeleteMarkerObject();

	pMarker->AddMarker(identifier, type, size, r, g, b, a, pulsePeriod, pulseFraction, rotateRate);
	if (type == MARKERTYPE_CYLINDER || type == MARKERTYPE_0 || type == MARKERTYPE_2) {
		float z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 1.0f, nil);
		if (z != 0.0f)
			pos.z = z - 0.05f * size;
	}
	pMarker->m_Matrix.SetTranslate(pos.x, pos.y, pos.z);
	if (type == MARKERTYPE_2) {
		pMarker->m_Matrix.RotateX(PI);
		pMarker->m_Matrix.GetPosition() = pos;
	}
	pMarker->m_Matrix.UpdateRW();
	if (type == MARKERTYPE_ARROW) {
		if (dist < 25.0f) {
			if (dist > 5.0f)
				pMarker->m_fStdSize = size - (25.0f - dist) * (0.3f * size) / 20.0f;
			else
				pMarker->m_fStdSize = size - 0.3f * size;
		} else {
			pMarker->m_fStdSize = size;
		}
	} else if (type == MARKERTYPE_CYLINDER) {
		if (dist < size + 12.0f) {
			if (dist > size + 1.0f)
				pMarker->m_Color.alpha = (1.0f - (size + 12.0f - dist) * 0.7f / 11.0f) * (float)a;
			else
				pMarker->m_Color.alpha = (float)a * 0.3f;
		} else {
			pMarker->m_Color.alpha = a;
		}
	}
	pMarker->m_bIsUsed = true;
	return pMarker;
}

void
C3dMarkers::PlaceMarkerSet(uint32 id, uint16 type, CVector &pos, float size, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate)
{
	PlaceMarker(id, type, pos, size, r, g, b, a, pulsePeriod, pulseFraction, 1);
	PlaceMarker(id, type, pos, size * 0.93f, r, g, b, a, pulsePeriod, pulseFraction, 2);
	PlaceMarker(id, type, pos, size * 0.86f, r, g, b, a, pulsePeriod, pulseFraction, -1);
}


void
C3dMarkers::Update()
{
}

#define MONEY_MESSAGE_LIFETIME_MS 2000

CMoneyMessage CMoneyMessages::aMoneyMessages[NUMMONEYMESSAGES];

void
CMoneyMessage::Render()
{
	const float MAX_SCALE = 4.0f;
	uint32 nLifeTime = CTimer::GetTimeInMilliseconds() - m_nTimeRegistered;
	if (nLifeTime >= MONEY_MESSAGE_LIFETIME_MS) m_nTimeRegistered = 0;
	else {
		float fLifeTime = (float)nLifeTime / MONEY_MESSAGE_LIFETIME_MS;
		RwV3d vecOut;
		float fDistX, fDistY;
		if (CSprite::CalcScreenCoors(m_vecPosition + CVector(0.0f, 0.0f, fLifeTime), &vecOut, &fDistX, &fDistY, true)) {
			fDistX *= (0.7 * fLifeTime + 2.0) * m_fSize;
			fDistY *= (0.7 * fLifeTime + 2.0) * m_fSize;
			CFont::SetPropOn();
			CFont::SetBackgroundOff();

			float fScaleY = fDistY / 100.0f;
			if (fScaleY > MAX_SCALE) fScaleY = MAX_SCALE;

			float fScaleX = fDistX / 100.0f;
			if (fScaleX > MAX_SCALE) fScaleX = MAX_SCALE;

			CFont::SetScale(fScaleX, fScaleY); // maybe use SCREEN_SCALE_X and SCREEN_SCALE_Y here?
			CFont::SetCentreOn();
			CFont::SetCentreSize(SCREEN_WIDTH);
			CFont::SetJustifyOff();
			CFont::SetColor(CRGBA(m_Colour.r, m_Colour.g, m_Colour.b, (255.0f - 255.0f * fLifeTime) * m_fOpacity));
			CFont::SetBackGroundOnlyTextOff();
			CFont::SetFontStyle(FONT_BANK);
			CFont::PrintString(vecOut.x, vecOut.y, m_aText);
		}
	}
}

void
CMoneyMessages::Init()
{
	for (int32 i = 0; i < NUMMONEYMESSAGES; i++)
		aMoneyMessages[i].m_nTimeRegistered = 0;
}

void
CMoneyMessages::Render()
{
	for (int32 i = 0; i < NUMMONEYMESSAGES; i++) {
		if (aMoneyMessages[i].m_nTimeRegistered != 0)
			aMoneyMessages[i].Render();
	}
}

void
CMoneyMessages::RegisterOne(CVector vecPos, const char *pText, uint8 bRed, uint8 bGreen, uint8 bBlue, float fSize, float fOpacity)
{
	uint32 nIndex = 0;
	while (aMoneyMessages[nIndex].m_nTimeRegistered != 0) {
		if (++nIndex >= NUMMONEYMESSAGES) return;
	}

	// Add data of this money message to the array
	AsciiToUnicode(pText, aMoneyMessages[nIndex].m_aText);

	aMoneyMessages[nIndex].m_nTimeRegistered = CTimer::GetTimeInMilliseconds();
	aMoneyMessages[nIndex].m_vecPosition = vecPos;
	aMoneyMessages[nIndex].m_Colour.red = bRed;
	aMoneyMessages[nIndex].m_Colour.green = bGreen;
	aMoneyMessages[nIndex].m_Colour.blue = bBlue;
	aMoneyMessages[nIndex].m_fSize = fSize;
	aMoneyMessages[nIndex].m_fOpacity = fOpacity;
}

CRGBA FoamColour(255, 255, 255, 255);
unsigned int CSpecialParticleStuff::BoatFromStart;

void
CSpecialParticleStuff::CreateFoamAroundObject(CMatrix* pMatrix, float innerFw, float innerRg, float innerUp, int32 particles)
{
	float outerFw = innerFw + 5.0f;
	float outerRg = innerRg + 5.0f;
	float outerUp = innerUp + 5.0f;
	for (int attempts = 0; particles > 0 && attempts < 1000; attempts++) {
		CVector pos;
		int rnd = CGeneral::GetRandomNumber();
		pos.x = (int8)(rnd - 128) * innerFw / 110.0f;
		pos.y = (int8)((rnd >> 8) - 128) * innerFw / 110.0f;
		pos.z = 0.0f;
		if (DotProduct2D(pos, TheCamera.GetForward()) >= 0)
			continue;
		// was there any point in adding it here?
		pos += pMatrix->GetPosition();
		pos.z = 2.0f;
		float fw = Abs(DotProduct(pMatrix->GetForward(), pos - pMatrix->GetPosition()));
		if (fw >= outerFw)
			continue;
		float rg = Abs(DotProduct(pMatrix->GetRight(), pos - pMatrix->GetPosition()));
		if (rg >= outerRg)
			continue;
		float up = Abs(DotProduct(pMatrix->GetUp(), pos - pMatrix->GetPosition()));
		if (up >= outerUp)
			continue;
		if (fw > innerFw || rg > innerRg || up > innerUp) {
			CParticle::AddParticle(PARTICLE_STEAM2, pos, CVector(0.0f, 0.0f, 0.0f), nil, 4.0f, FoamColour, 1, 0, 0, 0);
			particles--;
		}
	}
}

void
CSpecialParticleStuff::StartBoatFoamAnimation()
{
	BoatFromStart = CTimer::GetTimeInMilliseconds();
}

void
CSpecialParticleStuff::UpdateBoatFoamAnimation(CMatrix* pMatrix)
{
	static int32 FrameInAnimation = 0;
	static float X, Y, Z, dX, dY, dZ;
	CreateFoamAroundObject(pMatrix, 107.0f, 24.1f, 30.5f, 2);
	uint32 prev = CTimer::GetPreviousTimeInMilliseconds();
	uint32 cur = CTimer::GetTimeInMilliseconds();
	if (FrameInAnimation != 0) {
		X += dX;
		Y += dY;
		Z += dZ;
		CVector pos = *pMatrix * CVector(X, Y, Z);
		CParticle::AddParticle(PARTICLE_STEAM_NY, pos, CVector(0.0f, 0.0f, 0.0f),
			nil, FrameInAnimation * 0.5f + 2.0f, FoamColour, 1, 0, 0, 0);
		if (++FrameInAnimation > 15)
			FrameInAnimation = 0;
	}
	if ((cur & 0x3FF) < (prev & 0x3FF)) {
		FrameInAnimation = 1;
		int rnd = CGeneral::GetRandomNumber();
		X = (int8)(rnd - 128) * 0.2f;
		Y = (int8)((rnd >> 8) - 128) * 0.2f;
		Z = 10.0f;
		rnd = CGeneral::GetRandomNumber();
		dX = (int8)(rnd - 128) * 0.02f;
		dY = (int8)((rnd >> 8) - 128) * 0.02f;
		dZ = 2.0f;
	}
}

STARTPATCHES
	InjectHook(0x51B070, &C3dMarker::AddMarker, PATCH_JUMP);
	InjectHook(0x51B170, &C3dMarker::DeleteMarkerObject, PATCH_JUMP);
	InjectHook(0x51B1B0, &C3dMarker::Render, PATCH_JUMP);
	InjectHook(0x51B2B0, C3dMarkers::Init, PATCH_JUMP);
	InjectHook(0x51B480, C3dMarkers::PlaceMarker, PATCH_JUMP);
	InjectHook(0x51BB80, C3dMarkers::PlaceMarkerSet, PATCH_JUMP);
	InjectHook(0x51B400, C3dMarkers::Render, PATCH_JUMP);
	InjectHook(0x51B3B0, C3dMarkers::Shutdown, PATCH_JUMP);
	
	InjectHook(0x51AF70, CMoneyMessages::Init, PATCH_JUMP);
	InjectHook(0x51B030, CMoneyMessages::Render, PATCH_JUMP);
ENDPATCHES
