#include "common.h"

#include "SpecialFX.h"
#include "RenderBuffer.h"
#include "Timer.h"
#include "Sprite.h"
#include "Font.h"
#include "Text.h"
#include "TxdStore.h"
#include "FileMgr.h"
#include "FileLoader.h"
#include "Timecycle.h"
#include "Lights.h"
#include "ModelIndices.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "PlayerPed.h"
#include "Particle.h"
#include "Shadows.h"
#include "General.h"
#include "Camera.h"
#include "Shadows.h"
#include "main.h"

RwIm3DVertex StreakVertices[4];
RwImVertexIndex StreakIndexList[12];

RwIm3DVertex TraceVertices[6];
RwImVertexIndex TraceIndexList[12];


void
CSpecialFX::Init(void)
{
	CBulletTraces::Init();

	RwIm3DVertexSetU(&StreakVertices[0], 0.0f);
	RwIm3DVertexSetV(&StreakVertices[0], 0.0f);
	RwIm3DVertexSetU(&StreakVertices[1], 1.0f);
	RwIm3DVertexSetV(&StreakVertices[1], 0.0f);
	RwIm3DVertexSetU(&StreakVertices[2], 0.0f);
	RwIm3DVertexSetV(&StreakVertices[2], 0.0f);
	RwIm3DVertexSetU(&StreakVertices[3], 1.0f);
	RwIm3DVertexSetV(&StreakVertices[3], 0.0f);

	StreakIndexList[0] = 0;
	StreakIndexList[1] = 1;
	StreakIndexList[2] = 2;
	StreakIndexList[3] = 1;
	StreakIndexList[4] = 3;
	StreakIndexList[5] = 2;
	StreakIndexList[6] = 0;
	StreakIndexList[7] = 2;
	StreakIndexList[8] = 1;
	StreakIndexList[9] = 1;
	StreakIndexList[10] = 2;
	StreakIndexList[11] = 3;

	RwIm3DVertexSetRGBA(&TraceVertices[0], 20, 20, 20, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[1], 20, 20, 20, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[2], 70, 70, 70, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[3], 70, 70, 70, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[4], 10, 10, 10, 255);
	RwIm3DVertexSetRGBA(&TraceVertices[5], 10, 10, 10, 255);
	RwIm3DVertexSetU(&TraceVertices[0], 0.0);
	RwIm3DVertexSetV(&TraceVertices[0], 0.0);
	RwIm3DVertexSetU(&TraceVertices[1], 1.0);
	RwIm3DVertexSetV(&TraceVertices[1], 0.0);
	RwIm3DVertexSetU(&TraceVertices[2], 0.0);
	RwIm3DVertexSetV(&TraceVertices[2], 0.5);
	RwIm3DVertexSetU(&TraceVertices[3], 1.0);
	RwIm3DVertexSetV(&TraceVertices[3], 0.5);
	RwIm3DVertexSetU(&TraceVertices[4], 0.0);
	RwIm3DVertexSetV(&TraceVertices[4], 1.0);
	RwIm3DVertexSetU(&TraceVertices[5], 1.0);
	RwIm3DVertexSetV(&TraceVertices[5], 1.0);

	TraceIndexList[0] = 0;
	TraceIndexList[1] = 2;
	TraceIndexList[2] = 1;
	TraceIndexList[3] = 1;
	TraceIndexList[4] = 2;
	TraceIndexList[5] = 3;
	TraceIndexList[6] = 2;
	TraceIndexList[7] = 4;
	TraceIndexList[8] = 3;
	TraceIndexList[9] = 3;
	TraceIndexList[10] = 4;
	TraceIndexList[11] = 5;

	CMotionBlurStreaks::Init();
	CBrightLights::Init();
	CShinyTexts::Init();
	CMoneyMessages::Init();
	C3dMarkers::Init();
}

RwObject*
LookForBatCB(RwObject *object, void *data)
{
	static CMatrix MatLTM;

	if(CVisibilityPlugins::GetAtomicModelInfo((RpAtomic*)object) == (CSimpleModelInfo*)data){
		MatLTM = CMatrix(RwFrameGetLTM(RpAtomicGetFrame((RpAtomic*)object)));
		CVector p1 = MatLTM * CVector(0.02f, 0.05f, 0.07f);
		CVector p2 = MatLTM * CVector(0.246f, 0.0325f, 0.796f);
		CMotionBlurStreaks::RegisterStreak((uintptr)object, 100, 100, 100, p1, p2);
	}
	return nil;
}

void
CSpecialFX::Update(void)
{
	CMotionBlurStreaks::Update();
	CBulletTraces::Update();

	if(FindPlayerPed() &&
	   FindPlayerPed()->GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT &&
	   FindPlayerPed()->GetWeapon()->m_eWeaponState == WEAPONSTATE_FIRING){
#ifdef PED_SKIN
		if(IsClumpSkinned(FindPlayerPed()->GetClump())){
			LookForBatCB((RwObject*)FindPlayerPed()->m_pWeaponModel, CModelInfo::GetModelInfo(MI_BASEBALL_BAT));
		}else
#endif
			RwFrameForAllObjects(FindPlayerPed()->m_pFrames[PED_HANDR]->frame, LookForBatCB, CModelInfo::GetModelInfo(MI_BASEBALL_BAT));
	}
}

void
CSpecialFX::Shutdown(void)
{
	C3dMarkers::Shutdown();
}

void
CSpecialFX::Render(void)
{
	PUSH_RENDERGROUP("CSpecialFX::Render");
	CMotionBlurStreaks::Render();
	CBulletTraces::Render();
	CBrightLights::Render();
	CShinyTexts::Render();
	CMoneyMessages::Render();
#ifdef NEW_RENDERER
	if(!(gbNewRenderer && FredIsInFirstPersonCam()))
#endif
	C3dMarkers::Render();
	POP_RENDERGROUP();
}

CRegisteredMotionBlurStreak CMotionBlurStreaks::aStreaks[NUMMBLURSTREAKS];

void
CRegisteredMotionBlurStreak::Update(void)
{
	int i;
	bool wasUpdated;
	bool lastWasUpdated = false;
	for(i = 2; i > 0; i--){
		m_pos1[i] = m_pos1[i-1];
		m_pos2[i] = m_pos2[i-1];
		m_isValid[i] = m_isValid[i-1];
		wasUpdated = true;
		if(!lastWasUpdated && !m_isValid[i])
			wasUpdated = false;
		lastWasUpdated = wasUpdated;
	}
	m_isValid[0] = false;
	if(!wasUpdated)
		m_id = 0;
}

void
CRegisteredMotionBlurStreak::Render(void)
{
	int i;
	int a1, a2;
	for(i = 0; i < 2; i++)
		if(m_isValid[i] && m_isValid[i+1]){
			a1 = (255/3)*(3-i)/3;
			RwIm3DVertexSetRGBA(&StreakVertices[0], m_red, m_green, m_blue, a1);
			RwIm3DVertexSetRGBA(&StreakVertices[1], m_red, m_green, m_blue, a1);
			a2 = (255/3)*(3-(i+1))/3;
			RwIm3DVertexSetRGBA(&StreakVertices[2], m_red, m_green, m_blue, a2);
			RwIm3DVertexSetRGBA(&StreakVertices[3], m_red, m_green, m_blue, a2);
			RwIm3DVertexSetPos(&StreakVertices[0], m_pos1[i].x, m_pos1[i].y, m_pos1[i].z);
			RwIm3DVertexSetPos(&StreakVertices[1], m_pos2[i].x, m_pos2[i].y, m_pos2[i].z);
			RwIm3DVertexSetPos(&StreakVertices[2], m_pos1[i+1].x, m_pos1[i+1].y, m_pos1[i+1].z);
			RwIm3DVertexSetPos(&StreakVertices[3], m_pos2[i+1].x, m_pos2[i+1].y, m_pos2[i+1].z);
			LittleTest();
			if(RwIm3DTransform(StreakVertices, 4, nil, rwIM3D_VERTEXUV)){
				RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, StreakIndexList, 12);
				RwIm3DEnd();
			}
		}
}

void
CMotionBlurStreaks::Init(void)
{
	int i;
	for(i = 0; i < NUMMBLURSTREAKS; i++)
		aStreaks[i].m_id = 0;
}

void
CMotionBlurStreaks::Update(void)
{
	int i;
	for(i = 0; i < NUMMBLURSTREAKS; i++)
		if(aStreaks[i].m_id != 0)
			aStreaks[i].Update();
}

void
CMotionBlurStreaks::RegisterStreak(uintptr id, uint8 r, uint8 g, uint8 b, CVector p1, CVector p2)
{
	int i;
	for(i = 0; i < NUMMBLURSTREAKS; i++){
		if(aStreaks[i].m_id == id){
			// Found a streak from last frame, update
			aStreaks[i].m_red = r;
			aStreaks[i].m_green = g;
			aStreaks[i].m_blue = b;
			aStreaks[i].m_pos1[0] = p1;
			aStreaks[i].m_pos2[0] = p2;
			aStreaks[i].m_isValid[0] = true;
			return;
		}
	}
	// Find free slot
	for(i = 0; aStreaks[i].m_id != 0; i++)
		if(i == NUMMBLURSTREAKS-1)
			return;
	// Create a new streak
	aStreaks[i].m_id = id;
	aStreaks[i].m_red = r;
	aStreaks[i].m_green = g;
	aStreaks[i].m_blue = b;
	aStreaks[i].m_pos1[0] = p1;
	aStreaks[i].m_pos2[0] = p2;
	aStreaks[i].m_isValid[0] = true;
	aStreaks[i].m_isValid[1] = false;
	aStreaks[i].m_isValid[2] = false;
}

void
CMotionBlurStreaks::Render(void)
{
	bool setRenderStates = false;
	int i;
	for(i = 0; i < NUMMBLURSTREAKS; i++)
		if(aStreaks[i].m_id != 0){
			if(!setRenderStates){
				RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
				RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
				RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);
				RwRenderStateSet(rwRENDERSTATEFOGCOLOR,
					(void*)RWRGBALONG(CTimeCycle::GetFogRed(), CTimeCycle::GetFogGreen(), CTimeCycle::GetFogBlue(), 255));
				RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
				RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)FALSE);
 				setRenderStates = true;
			}
			aStreaks[i].Render();
		}
	if(setRenderStates){
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
	}
}


CBulletTrace CBulletTraces::aTraces[NUMBULLETTRACES];

void CBulletTraces::Init(void)
{
	for (int i = 0; i < NUMBULLETTRACES; i++)
		aTraces[i].m_bInUse = false;
}

void CBulletTraces::AddTrace(CVector* vecStart, CVector* vecTarget)
{
	int index;
	for (index = 0; index < NUMBULLETTRACES; index++) {
		if (!aTraces[index].m_bInUse)
			break;
	}
	if (index == NUMBULLETTRACES)
		return;
	aTraces[index].m_vecCurrentPos = *vecStart;
	aTraces[index].m_vecTargetPos = *vecTarget;
	aTraces[index].m_bInUse = true;
	aTraces[index].m_framesInUse = 0;
	aTraces[index].m_lifeTime = 25 + CGeneral::GetRandomNumber() % 32;
}

void CBulletTraces::Render(void)
{
	for (int i = 0; i < NUMBULLETTRACES; i++) {
		if (!aTraces[i].m_bInUse)
			continue;
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
#ifdef FIX_BUGS
		// Raster has no transparent pixels so it relies on the raster format having alpha
		// to turn on blending. librw image conversion might get rid of it right now so let's
		// just force it on.
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
#endif
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpShadowExplosionTex));
		CVector inf = aTraces[i].m_vecCurrentPos;
		CVector sup = aTraces[i].m_vecTargetPos;
		CVector center = (inf + sup) / 2;
		CVector width = CrossProduct(TheCamera.GetForward(), (sup - inf));
		width.Normalise();
		width /= 20;
		uint8 intensity = aTraces[i].m_lifeTime;
		for (int i = 0; i < ARRAY_SIZE(TraceVertices); i++)
			RwIm3DVertexSetRGBA(&TraceVertices[i], intensity, intensity, intensity, 0xFF);
		RwIm3DVertexSetPos(&TraceVertices[0], inf.x + width.x, inf.y + width.y, inf.z + width.z);
		RwIm3DVertexSetPos(&TraceVertices[1], inf.x - width.x, inf.y - width.y, inf.z - width.z);
		RwIm3DVertexSetPos(&TraceVertices[2], center.x + width.x, center.y + width.y, center.z + width.z);
		RwIm3DVertexSetPos(&TraceVertices[3], center.x - width.x, center.y - width.y, center.z - width.z);
		RwIm3DVertexSetPos(&TraceVertices[4], sup.x + width.x, sup.y + width.y, sup.z + width.z);
		RwIm3DVertexSetPos(&TraceVertices[5], sup.x - width.x, sup.y - width.y, sup.z - width.z);
		LittleTest();
		if (RwIm3DTransform(TraceVertices, ARRAY_SIZE(TraceVertices), nil, rwIM3D_VERTEXUV)) {
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TraceIndexList, ARRAY_SIZE(TraceIndexList));
			RwIm3DEnd();
		}
	}
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}

void CBulletTraces::Update(void)
{
	for (int i = 0; i < NUMBULLETTRACES; i++) {
		if (aTraces[i].m_bInUse)
			aTraces[i].Update();
	}
}

void CBulletTrace::Update(void)
{
	if (m_framesInUse == 0) {
		m_framesInUse++;
		return;
	}
	if (m_framesInUse > 60) {
		m_bInUse = false;
		return;
	}
	CVector diff = m_vecCurrentPos - m_vecTargetPos;
	float remaining = diff.Magnitude();
	if (remaining > 0.8f)
		m_vecCurrentPos = m_vecTargetPos + (remaining - 0.8f) / remaining * diff;
	else
		m_bInUse = false;
	if (--m_lifeTime == 0)
		m_bInUse = false;
	m_framesInUse++;
}

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

	RpMaterialSetColor(m_pMaterial, &m_Color);

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

C3dMarker C3dMarkers::m_aMarkerArray[NUM3DMARKERS];
int32 C3dMarkers::NumActiveMarkers;
RpClump* C3dMarkers::m_pRpClumpArray[NUMMARKERTYPES];

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
			CVector pos = pMarker->m_Matrix.GetPosition();
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


#define BRIGHTLIGHTS_MAX_DIST (60.0f)	// invisible beyond this
#define BRIGHTLIGHTS_FADE_DIST (45.0f)	// strongest between these two
#define CARLIGHTS_MAX_DIST (30.0f)
#define CARLIGHTS_FADE_DIST (15.0f)	// 31 for close lights

int CBrightLights::NumBrightLights;
CBrightLight CBrightLights::aBrightLights[NUMBRIGHTLIGHTS];

void
CBrightLights::Init(void)
{
	NumBrightLights = 0;
}

void
CBrightLights::RegisterOne(CVector pos, CVector up, CVector side, CVector front,
	uint8 type, uint8 red, uint8 green, uint8 blue)
{
	if(NumBrightLights >= NUMBRIGHTLIGHTS)
		return;

	aBrightLights[NumBrightLights].m_camDist = (pos - TheCamera.GetPosition()).Magnitude();
	if(aBrightLights[NumBrightLights].m_camDist > BRIGHTLIGHTS_MAX_DIST)
		return;

	aBrightLights[NumBrightLights].m_pos = pos;
	aBrightLights[NumBrightLights].m_up = up;
	aBrightLights[NumBrightLights].m_side = side;
	aBrightLights[NumBrightLights].m_front = front;
	aBrightLights[NumBrightLights].m_type = type;
	aBrightLights[NumBrightLights].m_red = red;
	aBrightLights[NumBrightLights].m_green = green;
	aBrightLights[NumBrightLights].m_blue = blue;

	NumBrightLights++;
}

static float TrafficLightsSide[6] = { -0.09f, 0.09f, 0.162f, 0.09f, -0.09f, -0.162f };
static float TrafficLightsUp[6] = { 0.162f, 0.162f, 0.0f, -0.162f, -0.162f, 0.0f };
static float LongCarHeadLightsSide[8] = { -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f };
static float LongCarHeadLightsFront[8] = { 0.1f, 0.1f, -0.1f, -0.1f, 0.1f, 0.1f, -0.1f, -0.1f };
static float LongCarHeadLightsUp[8] = { 0.1f, 0.1f, 0.1f, 0.1f, -0.1f, -0.1f, -0.1f, -0.1f };
static float SmallCarHeadLightsSide[8] = { -0.08f, 0.08f, -0.08f, 0.08f, -0.08f, 0.08f, -0.08f, 0.08f };
static float SmallCarHeadLightsFront[8] = { 0.08f, 0.08f, -0.08f, -0.08f, 0.08f, 0.08f, -0.08f, -0.08f };
static float SmallCarHeadLightsUp[8] = { 0.08f, 0.08f, 0.08f, 0.08f, -0.08f, -0.08f, -0.08f, -0.08f };
static float BigCarHeadLightsSide[8] = { -0.15f, 0.15f, -0.15f, 0.15f, -0.15f, 0.15f, -0.15f, 0.15f };
static float BigCarHeadLightsFront[8] = { 0.15f, 0.15f, -0.15f, -0.15f, 0.15f, 0.15f, -0.15f, -0.15f };
static float BigCarHeadLightsUp[8] = { 0.15f, 0.15f, 0.15f, 0.15f, -0.15f, -0.15f, -0.15f, -0.15f };
static float TallCarHeadLightsSide[8] = { -0.08f, 0.08f, -0.08f, 0.08f, -0.08f, 0.08f, -0.08f, 0.08f };
static float TallCarHeadLightsFront[8] = { 0.08f, 0.08f, -0.08f, -0.08f, 0.08f, 0.08f, -0.08f, -0.08f };
static float TallCarHeadLightsUp[8] = { 0.2f, 0.2f, 0.2f, 0.2f, -0.2f, -0.2f, -0.2f, -0.2f };
static float SirenLightsSide[6] = { -0.04f, 0.04f, 0.06f, 0.04f, -0.04f, -0.06f };
static float SirenLightsUp[6] = { 0.06f, 0.06f, 0.0f, -0.06f, -0.06f, 0.0f };
static RwImVertexIndex TrafficLightIndices[4*3] = { 0, 1, 5,  1, 2, 3,  1, 3, 4,  1, 4, 5 };
static RwImVertexIndex CubeIndices[12*3] = {
	0, 2, 1,  1, 2, 3,  3, 5, 1,  3, 7, 5,
	2, 7, 3,  2, 6, 7,  4, 0, 1,  4, 1, 5,
	6, 0, 4,  6, 2, 0,  6, 5, 7,  6, 4, 5
};

void
CBrightLights::Render(void)
{
	int i, j;
	CVector pos;

	if(NumBrightLights == 0)
		return;

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;

	for(i = 0; i < NumBrightLights; i++){
		if(TempBufferIndicesStored > TEMPBUFFERINDEXSIZE-40 || TempBufferVerticesStored > TEMPBUFFERVERTSIZE-40)
			RenderOutGeometryBuffer();

		int r, g, b, a;
		float flicker = (CGeneral::GetRandomNumber()&0xFF) * 0.2f;
		switch(aBrightLights[i].m_type){
		case BRIGHTLIGHT_TRAFFIC_GREEN:
			r = flicker; g = 255; b = flicker;
			break;
		case BRIGHTLIGHT_TRAFFIC_YELLOW:
			r = 255; g = 128; b = flicker;
			break;
		case BRIGHTLIGHT_TRAFFIC_RED:
			r = 255; g = flicker; b = flicker;
			break;

		case BRIGHTLIGHT_FRONT_LONG:
		case BRIGHTLIGHT_FRONT_SMALL:
		case BRIGHTLIGHT_FRONT_BIG:
		case BRIGHTLIGHT_FRONT_TALL:
			r = 255; g = 255; b = 255;
			break;

		case BRIGHTLIGHT_REAR_LONG:
		case BRIGHTLIGHT_REAR_SMALL:
		case BRIGHTLIGHT_REAR_BIG:
		case BRIGHTLIGHT_REAR_TALL:
			r = 255; g = flicker; b = flicker;
			break;

		case BRIGHTLIGHT_SIREN:
			r = aBrightLights[i].m_red;
			g = aBrightLights[i].m_green;
			b = aBrightLights[i].m_blue;
			break;
		}

		if(aBrightLights[i].m_camDist < BRIGHTLIGHTS_FADE_DIST)
			a = 255;
		else
			a = 255*(1.0f - (aBrightLights[i].m_camDist-BRIGHTLIGHTS_FADE_DIST)/(BRIGHTLIGHTS_MAX_DIST-BRIGHTLIGHTS_FADE_DIST));
		// fade car lights down to 31 as they come near
		if(aBrightLights[i].m_type >= BRIGHTLIGHT_FRONT_LONG && aBrightLights[i].m_type <= BRIGHTLIGHT_REAR_TALL){
			if(aBrightLights[i].m_camDist < CARLIGHTS_FADE_DIST)
				a = 31;
			else if(aBrightLights[i].m_camDist < CARLIGHTS_MAX_DIST)
				a = 31 + (255-31)*((aBrightLights[i].m_camDist-CARLIGHTS_FADE_DIST)/(CARLIGHTS_MAX_DIST-CARLIGHTS_FADE_DIST));
		}

		switch(aBrightLights[i].m_type){
		case BRIGHTLIGHT_TRAFFIC_GREEN:
		case BRIGHTLIGHT_TRAFFIC_YELLOW:
		case BRIGHTLIGHT_TRAFFIC_RED:
			for(j = 0; j < 6; j++){
				pos = TrafficLightsSide[j]*aBrightLights[i].m_side +
					TrafficLightsUp[j]*aBrightLights[i].m_up +
					aBrightLights[i].m_pos;
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored+j], r, g, b, a);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored+j], pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 4*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = TrafficLightIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 6;
			TempBufferIndicesStored += 4*3;
			break;

		case BRIGHTLIGHT_FRONT_LONG:
		case BRIGHTLIGHT_REAR_LONG:
			for(j = 0; j < 8; j++){
				pos = LongCarHeadLightsSide[j]*aBrightLights[i].m_side +
					LongCarHeadLightsUp[j]*aBrightLights[i].m_up +
					LongCarHeadLightsFront[j]*aBrightLights[i].m_front +
					aBrightLights[i].m_pos;
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored+j], r, g, b, a);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored+j], pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 12*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = CubeIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 8;
			TempBufferIndicesStored += 12*3;
			break;

		case BRIGHTLIGHT_FRONT_SMALL:
		case BRIGHTLIGHT_REAR_SMALL:
			for(j = 0; j < 8; j++){
				pos = SmallCarHeadLightsSide[j]*aBrightLights[i].m_side +
					SmallCarHeadLightsUp[j]*aBrightLights[i].m_up +
					SmallCarHeadLightsFront[j]*aBrightLights[i].m_front +
					aBrightLights[i].m_pos;
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored+j], r, g, b, a);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored+j], pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 12*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = CubeIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 8;
			TempBufferIndicesStored += 12*3;
			break;

		case BRIGHTLIGHT_FRONT_BIG:
		case BRIGHTLIGHT_REAR_BIG:
			for (j = 0; j < 8; j++) {
			pos = BigCarHeadLightsSide[j] * aBrightLights[i].m_side +
				BigCarHeadLightsUp[j] * aBrightLights[i].m_up +
				BigCarHeadLightsFront[j] * aBrightLights[i].m_front +
				aBrightLights[i].m_pos;
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored + j], r, g, b, a);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored + j], pos.x, pos.y, pos.z);
		}
		for (j = 0; j < 12 * 3; j++)
			TempBufferRenderIndexList[TempBufferIndicesStored + j] = CubeIndices[j] + TempBufferVerticesStored;
		TempBufferVerticesStored += 8;
		TempBufferIndicesStored += 12 * 3;
		break;

		case BRIGHTLIGHT_FRONT_TALL:
		case BRIGHTLIGHT_REAR_TALL:
			for(j = 0; j < 8; j++){
				pos = TallCarHeadLightsSide[j]*aBrightLights[i].m_side +
					TallCarHeadLightsUp[j]*aBrightLights[i].m_up +
					TallCarHeadLightsFront[j]*aBrightLights[i].m_front +
					aBrightLights[i].m_pos;
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored+j], r, g, b, a);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored+j], pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 12*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = CubeIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 8;
			TempBufferIndicesStored += 12*3;
			break;

		case BRIGHTLIGHT_SIREN:
			for(j = 0; j < 6; j++){
				pos = SirenLightsSide[j] * TheCamera.GetRight() +
					SirenLightsUp[j] * TheCamera.GetUp() +
					aBrightLights[i].m_pos;
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored+j], r, g, b, a);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored+j], pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 4*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = TrafficLightIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 6;
			TempBufferIndicesStored += 4*3;
			break;

		}
	}

	RenderOutGeometryBuffer();
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	NumBrightLights = 0;
}

void
CBrightLights::RenderOutGeometryBuffer(void)
{
	if(TempBufferIndicesStored != 0){
		LittleTest();
		if(RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, rwIM3D_VERTEXUV)){
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
			RwIm3DEnd();
		}
		TempBufferVerticesStored = 0;
		TempBufferIndicesStored = 0;
	}
}

int CShinyTexts::NumShinyTexts;
CShinyText CShinyTexts::aShinyTexts[NUMSHINYTEXTS];

void
CShinyTexts::Init(void)
{
	NumShinyTexts = 0;
}

void
CShinyTexts::RegisterOne(CVector p0, CVector p1, CVector p2, CVector p3,
	float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3,
	uint8 type, uint8 red, uint8 green, uint8 blue, float maxDist)
{
	if(NumShinyTexts >= NUMSHINYTEXTS)
		return;

	aShinyTexts[NumShinyTexts].m_camDist = (p0 - TheCamera.GetPosition()).Magnitude();
	if(aShinyTexts[NumShinyTexts].m_camDist > maxDist)
		return;
	aShinyTexts[NumShinyTexts].m_verts[0] = p0;
	aShinyTexts[NumShinyTexts].m_verts[1] = p1;
	aShinyTexts[NumShinyTexts].m_verts[2] = p2;
	aShinyTexts[NumShinyTexts].m_verts[3] = p3;
	aShinyTexts[NumShinyTexts].m_texCoords[0].x = u0;
	aShinyTexts[NumShinyTexts].m_texCoords[0].y = v0;
	aShinyTexts[NumShinyTexts].m_texCoords[1].x = u1;
	aShinyTexts[NumShinyTexts].m_texCoords[1].y = v1;
	aShinyTexts[NumShinyTexts].m_texCoords[2].x = u2;
	aShinyTexts[NumShinyTexts].m_texCoords[2].y = v2;
	aShinyTexts[NumShinyTexts].m_texCoords[3].x = u3;
	aShinyTexts[NumShinyTexts].m_texCoords[3].y = v3;
	aShinyTexts[NumShinyTexts].m_type = type;
	aShinyTexts[NumShinyTexts].m_red = red;
	aShinyTexts[NumShinyTexts].m_green = green;
	aShinyTexts[NumShinyTexts].m_blue = blue;
	// Fade out at half the max dist
	float halfDist = maxDist*0.5f;
	if(aShinyTexts[NumShinyTexts].m_camDist > halfDist){
		float f = 1.0f - (aShinyTexts[NumShinyTexts].m_camDist - halfDist)/halfDist;
		aShinyTexts[NumShinyTexts].m_red *= f;
		aShinyTexts[NumShinyTexts].m_green *= f;
		aShinyTexts[NumShinyTexts].m_blue *= f;
	}

	NumShinyTexts++;
}

void
CShinyTexts::Render(void)
{
	int i, ix, v;
	RwTexture *lastTex = nil;

	if(NumShinyTexts == 0)
		return;

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);

	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;

	for(i = 0; i < NumShinyTexts; i++){
		if(TempBufferIndicesStored > TEMPBUFFERINDEXSIZE-64 || TempBufferVerticesStored > TEMPBUFFERVERTSIZE-62)
			RenderOutGeometryBuffer();

		uint8 r = aShinyTexts[i].m_red;
		uint8 g = aShinyTexts[i].m_green;
		uint8 b = aShinyTexts[i].m_blue;

		switch(aShinyTexts[i].m_type){
		case SHINYTEXT_WALK:
			if(lastTex != gpWalkDontTex){
				RenderOutGeometryBuffer();
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpWalkDontTex));
				lastTex = gpWalkDontTex;
			}
	quad:
			v = TempBufferVerticesStored;
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v+0], r, g, b, 255);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[v+0], aShinyTexts[i].m_verts[0].x, aShinyTexts[i].m_verts[0].y, aShinyTexts[i].m_verts[0].z);
			RwIm3DVertexSetU(&TempBufferRenderVertices[v+0], aShinyTexts[i].m_texCoords[0].x);
			RwIm3DVertexSetV(&TempBufferRenderVertices[v+0], aShinyTexts[i].m_texCoords[0].y);
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v+1], r, g, b, 255);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[v+1], aShinyTexts[i].m_verts[1].x, aShinyTexts[i].m_verts[1].y, aShinyTexts[i].m_verts[1].z);
			RwIm3DVertexSetU(&TempBufferRenderVertices[v+1], aShinyTexts[i].m_texCoords[1].x);
			RwIm3DVertexSetV(&TempBufferRenderVertices[v+1], aShinyTexts[i].m_texCoords[1].y);
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v+2], r, g, b, 255);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[v+2], aShinyTexts[i].m_verts[2].x, aShinyTexts[i].m_verts[2].y, aShinyTexts[i].m_verts[2].z);
			RwIm3DVertexSetU(&TempBufferRenderVertices[v+2], aShinyTexts[i].m_texCoords[2].x);
			RwIm3DVertexSetV(&TempBufferRenderVertices[v+2], aShinyTexts[i].m_texCoords[2].y);
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v+3], r, g, b, 255);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[v+3], aShinyTexts[i].m_verts[3].x, aShinyTexts[i].m_verts[3].y, aShinyTexts[i].m_verts[3].z);
			RwIm3DVertexSetU(&TempBufferRenderVertices[v+3], aShinyTexts[i].m_texCoords[3].x);
			RwIm3DVertexSetV(&TempBufferRenderVertices[v+3], aShinyTexts[i].m_texCoords[3].y);
			ix = TempBufferIndicesStored;
			TempBufferRenderIndexList[ix+0] = 0 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+1] = 1 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+2] = 2 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+3] = 2 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+4] = 1 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+5] = 3 + TempBufferVerticesStored;
			TempBufferVerticesStored += 4;
			TempBufferIndicesStored += 6;
			break;

		case SHINYTEXT_FLAT:
			if(lastTex != nil){
				RenderOutGeometryBuffer();
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
				lastTex = nil;
			}
			goto quad;
		}
	}

	RenderOutGeometryBuffer();
	NumShinyTexts = 0;

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}

void
CShinyTexts::RenderOutGeometryBuffer(void)
{
	if(TempBufferIndicesStored != 0){
		LittleTest();
		if(RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, rwIM3D_VERTEXUV)){
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
			RwIm3DEnd();
		}
		TempBufferVerticesStored = 0;
		TempBufferIndicesStored = 0;
	}
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
			fDistX *= (0.7f * fLifeTime + 2.0f) * m_fSize;
			fDistY *= (0.7f * fLifeTime + 2.0f) * m_fSize;
			CFont::SetPropOn();
			CFont::SetBackgroundOff();

			float fScaleY = Min(fDistY / 100.0f, MAX_SCALE);
			float fScaleX = Min(fDistX / 100.0f, MAX_SCALE);

#ifdef FIX_BUGS
			CFont::SetScale(SCREEN_SCALE_X(fScaleX), SCREEN_SCALE_Y(fScaleY));
#else
			CFont::SetScale(fScaleX, fScaleY);
#endif
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
	uint32 i;
#ifdef FIX_BUGS
	for(i = 0; i < NUMMONEYMESSAGES && aMoneyMessages[i].m_nTimeRegistered != 0; i++);
#else
	for(i = 0; aMoneyMessages[i].m_nTimeRegistered != 0 && i < NUMMONEYMESSAGES; i++);
#endif

	if(i < NUMMONEYMESSAGES) {
		// Add data of this money message to the array
		AsciiToUnicode(pText, aMoneyMessages[i].m_aText);

		aMoneyMessages[i].m_nTimeRegistered = CTimer::GetTimeInMilliseconds();
		aMoneyMessages[i].m_vecPosition = vecPos;
		aMoneyMessages[i].m_Colour.red = bRed;
		aMoneyMessages[i].m_Colour.green = bGreen;
		aMoneyMessages[i].m_Colour.blue = bBlue;
		aMoneyMessages[i].m_fSize = fSize;
		aMoneyMessages[i].m_fOpacity = fOpacity;
	}
}

CRGBA FoamColour(255, 255, 255, 255);
uint32 CSpecialParticleStuff::BoatFromStart;

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
