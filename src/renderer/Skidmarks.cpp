#include "common.h"

#include "main.h"
#include "TxdStore.h"
#include "Timer.h"
#include "Replay.h"
#include "Skidmarks.h"

CSkidmark CSkidmarks::aSkidmarks[NUMSKIDMARKS];

RwImVertexIndex SkidmarkIndexList[SKIDMARK_LENGTH * 6];
RwIm3DVertex SkidmarkVertices[SKIDMARK_LENGTH * 2];
RwTexture *gpSkidTex;
RwTexture *gpSkidBloodTex;
RwTexture *gpSkidMudTex;

void
CSkidmarks::Init(void)
{
	int i, ix, slot;
	CTxdStore::PushCurrentTxd();
	slot = CTxdStore::FindTxdSlot("particle");
	CTxdStore::SetCurrentTxd(slot);
	gpSkidTex = RwTextureRead("particleskid", nil);
	gpSkidBloodTex = RwTextureRead("particleskidblood", nil);
	gpSkidMudTex = RwTextureRead("particleskidmud", nil);
	CTxdStore::PopCurrentTxd();

	for(i = 0; i < NUMSKIDMARKS; i++){
		aSkidmarks[i].m_state = 0;
		aSkidmarks[i].m_wasUpdated = false;
	}

	ix = 0;
	for(i = 0; i < SKIDMARK_LENGTH; i++){
		SkidmarkIndexList[i*6+0] = ix+0;
		SkidmarkIndexList[i*6+1] = ix+2;
		SkidmarkIndexList[i*6+2] = ix+1;
		SkidmarkIndexList[i*6+3] = ix+1;
		SkidmarkIndexList[i*6+4] = ix+2;
		SkidmarkIndexList[i*6+5] = ix+3;
		ix += 2;
	}

	for(i = 0; i < SKIDMARK_LENGTH; i++){
		RwIm3DVertexSetU(&SkidmarkVertices[i*2 + 0], 0.0f);
		RwIm3DVertexSetV(&SkidmarkVertices[i*2 + 0], i*5.01f);
		RwIm3DVertexSetU(&SkidmarkVertices[i*2 + 1], 1.0f);
		RwIm3DVertexSetV(&SkidmarkVertices[i*2 + 1], i*5.01f);
	}
}

void
CSkidmarks::Shutdown(void)
{
	RwTextureDestroy(gpSkidTex);
#if GTA_VERSION >= GTA3_PC_11
	gpSkidTex = nil;
#endif
	RwTextureDestroy(gpSkidBloodTex);
#if GTA_VERSION >= GTA3_PC_11
	gpSkidBloodTex = nil;
#endif
	RwTextureDestroy(gpSkidMudTex);
#if GTA_VERSION >= GTA3_PC_11
	gpSkidMudTex = nil;
#endif
}

void
CSkidmarks::Clear(void)
{
	int i;
	for(i = 0; i < NUMSKIDMARKS; i++){
		aSkidmarks[i].m_state = 0;
		aSkidmarks[i].m_wasUpdated = false;
	}
}

void
CSkidmarks::Update(void)
{
	int i;
	uint32 t1 = CTimer::GetTimeInMilliseconds() + 2500;
	uint32 t2 = CTimer::GetTimeInMilliseconds() + 5000;
	uint32 t3 = CTimer::GetTimeInMilliseconds() + 10000;
	uint32 t4 = CTimer::GetTimeInMilliseconds() + 20000;
	for(i = 0; i < NUMSKIDMARKS; i++){
		switch(aSkidmarks[i].m_state){
		case 1:
			if(!aSkidmarks[i].m_wasUpdated){
				// Didn't continue this one last time, so finish it and set fade times
				aSkidmarks[i].m_state = 2;
				if(aSkidmarks[i].m_last < 4){
					aSkidmarks[i].m_fadeStart = t1;
					aSkidmarks[i].m_fadeEnd = t2;
				}else if(aSkidmarks[i].m_last < 9){
					aSkidmarks[i].m_fadeStart = t2;
					aSkidmarks[i].m_fadeEnd = t3;
				}else{
					aSkidmarks[i].m_fadeStart = t3;
					aSkidmarks[i].m_fadeEnd = t4;
				}
			}
			break;
		case 2:
			if(CTimer::GetTimeInMilliseconds() > aSkidmarks[i].m_fadeEnd)
				aSkidmarks[i].m_state = 0;
			break;
		}
		aSkidmarks[i].m_wasUpdated = false;
	}
}

void
CSkidmarks::Render(void)
{
	int i, j;
	RwTexture *lastTex = nil;

	PUSH_RENDERGROUP("CSkidmarks::Render");

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

	for(i = 0; i < NUMSKIDMARKS; i++){
		if(aSkidmarks[i].m_state == 0 || aSkidmarks[i].m_last < 1)
			continue;

		if(aSkidmarks[i].m_isBloody){
			if(lastTex != gpSkidBloodTex){
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpSkidBloodTex));
				lastTex = gpSkidBloodTex;
			}
		}else if(aSkidmarks[i].m_isMuddy){
			if(lastTex != gpSkidMudTex){
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpSkidMudTex));
				lastTex = gpSkidMudTex;
			}
		}else{
			if(lastTex != gpSkidTex){
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpSkidTex));
				lastTex = gpSkidTex;
			}
		}

		uint32 fade, alpha;
		if(aSkidmarks[i].m_state == 1 || CTimer::GetTimeInMilliseconds() < aSkidmarks[i].m_fadeStart)
			fade = 255;
		else
			fade = 255*(aSkidmarks[i].m_fadeEnd - CTimer::GetTimeInMilliseconds()) / (aSkidmarks[i].m_fadeEnd - aSkidmarks[i].m_fadeStart);

		for(j = 0; j <= aSkidmarks[i].m_last; j++){
			alpha = 128;
			if(j == 0 || j == aSkidmarks[i].m_last && aSkidmarks[i].m_state == 2)
				alpha = 0;
			alpha = alpha*fade/256;

			CVector p1 = aSkidmarks[i].m_pos[j] + aSkidmarks[i].m_side[j];
			CVector p2 = aSkidmarks[i].m_pos[j] - aSkidmarks[i].m_side[j];
			RwIm3DVertexSetRGBA(&SkidmarkVertices[j*2+0], 255, 255, 255, alpha);
			RwIm3DVertexSetPos(&SkidmarkVertices[j*2+0], p1.x, p1.y, p1.z+0.1f);
			RwIm3DVertexSetRGBA(&SkidmarkVertices[j*2+1], 255, 255, 255, alpha);
			RwIm3DVertexSetPos(&SkidmarkVertices[j*2+1], p2.x, p2.y, p2.z+0.1f);
		}

		LittleTest();
		if(RwIm3DTransform(SkidmarkVertices, 2*(aSkidmarks[i].m_last+1), nil, rwIM3D_VERTEXUV)){
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, SkidmarkIndexList, 6*aSkidmarks[i].m_last);
			RwIm3DEnd();
		}
	}

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);

	POP_RENDERGROUP();
}

void
CSkidmarks::RegisterOne(uintptr id, CVector pos, float fwdX, float fwdY, bool *isMuddy, bool *isBloody)
{
	int i;
	CVector2D fwd(fwdX, fwdY);

	if(CReplay::IsPlayingBack())
		return;

	// Find a skidmark to continue
	for(i = 0; i < NUMSKIDMARKS; i++)
		if(aSkidmarks[i].m_state == 1 && aSkidmarks[i].m_id == id)
			break;

	if(i < NUMSKIDMARKS){
		// Continue this one

		if(aSkidmarks[i].m_isBloody != *isBloody){
			// Blood-status changed, end this one
			aSkidmarks[i].m_state = 2;
			aSkidmarks[i].m_fadeStart = CTimer::GetTimeInMilliseconds() + 10000;
			aSkidmarks[i].m_fadeEnd = CTimer::GetTimeInMilliseconds() + 20000;
			return;
		}

		aSkidmarks[i].m_wasUpdated = true;

		if(CTimer::GetTimeInMilliseconds() - aSkidmarks[i].m_lastUpdate <= 100){
			// Last update was recently, just change last coords
			aSkidmarks[i].m_pos[aSkidmarks[i].m_last] = pos;
			return;
		}
		aSkidmarks[i].m_lastUpdate = CTimer::GetTimeInMilliseconds();

		if(aSkidmarks[i].m_last >= SKIDMARK_LENGTH-1){
			// No space to continue, end it
			aSkidmarks[i].m_state = 2;
			aSkidmarks[i].m_fadeStart = CTimer::GetTimeInMilliseconds() + 10000;
			aSkidmarks[i].m_fadeEnd = CTimer::GetTimeInMilliseconds() + 20000;
			*isBloody = false;	// stpo blood marks at end
			return;
		}
		aSkidmarks[i].m_last++;

		aSkidmarks[i].m_pos[aSkidmarks[i].m_last] = pos;

		CVector2D right(aSkidmarks[i].m_pos[aSkidmarks[i].m_last].y - aSkidmarks[i].m_pos[aSkidmarks[i].m_last - 1].y,
		                aSkidmarks[i].m_pos[aSkidmarks[i].m_last - 1].x - aSkidmarks[i].m_pos[aSkidmarks[i].m_last].x);

		right.NormaliseSafe();
		fwd.NormaliseSafe();
		float turn = DotProduct2D(fwd, right);
		turn = Abs(turn) + 1.0f;
		aSkidmarks[i].m_side[aSkidmarks[i].m_last] = CVector(right.x, right.y, 0.0f) * turn * 0.125f;
		if(aSkidmarks[i].m_last == 1)
			aSkidmarks[i].m_side[0] = aSkidmarks[i].m_side[1];

		if(aSkidmarks[i].m_last > 8)
			*isBloody = false;	// stop blood marks after 8
		return;
	}

	// Start a new one
	for(i = 0; i < NUMSKIDMARKS; i++)
		if(aSkidmarks[i].m_state == 0)
			break;
	if(i < NUMSKIDMARKS){
		// Found a free slot
		aSkidmarks[i].m_state = 1;
		aSkidmarks[i].m_id = id;
		aSkidmarks[i].m_pos[0] = pos;
		aSkidmarks[i].m_side[0] = CVector(0.0f, 0.0f, 0.0f);
		aSkidmarks[i].m_wasUpdated = true;
		aSkidmarks[i].m_last = 0;
		aSkidmarks[i].m_lastUpdate = CTimer::GetTimeInMilliseconds() - 1000;
		aSkidmarks[i].m_isBloody = *isBloody;
		aSkidmarks[i].m_isMuddy = *isMuddy;
	}else
		*isBloody = false;	// stop blood marks if no space
}
