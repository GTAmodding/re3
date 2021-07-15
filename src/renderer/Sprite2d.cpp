#include "common.h"

#include "main.h"
#include "Draw.h"
#include "Camera.h"
#include "Sprite2d.h"
#include "Font.h"

RwIm2DVertex CSprite2d::maVertices[8];
float CSprite2d::RecipNearClip;
int32 CSprite2d::mCurrentBank;
RwTexture *CSprite2d::mpBankTextures[10];
int32 CSprite2d::mCurrentSprite[10];
int32 CSprite2d::mBankStart[10];
RwIm2DVertex CSprite2d::maBankVertices[500];

void
CSprite2d::SetRecipNearClip(void)
{
	RecipNearClip = 1.0f / RwCameraGetNearClipPlane(Scene.camera);
}

void
CSprite2d::InitPerFrame(void)
{
	int i;

	mCurrentBank = 0;
	for(i = 0; i < 10; i++)
		mCurrentSprite[i] = 0;
#ifndef SQUEEZE_PERFORMANCE
	for(i = 0; i < 10; i++)
		mpBankTextures[i] = nil;
#endif
}

int32
CSprite2d::GetBank(int32 n, RwTexture *tex)
{
#ifndef SQUEEZE_PERFORMANCE
	mpBankTextures[mCurrentBank] = tex;
#endif
	mCurrentSprite[mCurrentBank] = 0;
	mBankStart[mCurrentBank+1] = mBankStart[mCurrentBank] + n;
	return mCurrentBank++;
}

void
CSprite2d::AddSpriteToBank(int32 bank, const CRect &rect, const CRGBA &col,
	float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
	SetVertices(&maBankVertices[6 * (mCurrentSprite[bank] + mBankStart[bank])],
		rect, col, col, col, col,
		u0, v0, u1, v1, u2, v2, u3, v3);
	mCurrentSprite[bank]++;
	if(mCurrentSprite[bank] + mBankStart[bank] >= mBankStart[bank+1]){
		DrawBank(bank);
		mCurrentSprite[bank] = 0;
	}
}

void
CSprite2d::DrawBank(int32 bank)
{
	if(mCurrentSprite[bank] == 0)
		return;
#ifndef SQUEEZE_PERFORMANCE
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,
		mpBankTextures[bank] ? RwTextureGetRaster(mpBankTextures[bank]) : nil);
#else
	CFont::Sprite[bank].SetRenderState();
#endif
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwIm2DRenderPrimitive(rwPRIMTYPETRILIST, &maBankVertices[6*mBankStart[bank]], 6*mCurrentSprite[bank]);
	mCurrentSprite[bank] = 0;
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
}


void
CSprite2d::Delete(void)
{
	if(m_pTexture){
		RwTextureDestroy(m_pTexture);
		m_pTexture = nil;
	}
}

void
CSprite2d::SetTexture(const char *name)
{
	Delete();
	if(name)
		m_pTexture = RwTextureRead(name, nil);
}

void
CSprite2d::SetTexture(const char *name, const char *mask)
{
	Delete();
	if(name)
		m_pTexture = RwTextureRead(name, mask);
}

void
CSprite2d::SetAddressing(RwTextureAddressMode addr)
{
	if(m_pTexture)
		RwTextureSetAddressing(m_pTexture, addr);
}

void
CSprite2d::SetRenderState(void)
{
	if(m_pTexture)
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(m_pTexture));
	else
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
}

void
CSprite2d::Draw(float x, float y, float w, float h, const CRGBA &col)
{
	SetVertices(CRect(x, y, x + w, y + h), col, col, col, col, 0);
	SetRenderState();
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
}

void
CSprite2d::Draw(const CRect &rect, const CRGBA &col)
{
	SetVertices(rect, col, col, col, col, 0);
	SetRenderState();
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
}

void
CSprite2d::Draw(const CRect &rect, const CRGBA &col,
	float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
	SetVertices(rect, col, col, col, col, u0, v0, u1, v1, u3, v3, u2, v2);
	SetRenderState();
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
}

void
CSprite2d::Draw(const CRect &rect, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3)
{
	SetVertices(rect, c0, c1, c2, c3, 0);
	SetRenderState();
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
}

void
CSprite2d::Draw(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, const CRGBA &col)
{
	SetVertices(x1, y1, x2, y2, x3, y3, x4, y4, col, col, col, col);
	SetRenderState();
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
}


// Arguments:
// 2---3
// |   |
// 0---1
void
CSprite2d::SetVertices(const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3, uint32 far)
{
	float screenz, z, recipz;

	if(far){
		screenz = RwIm2DGetFarScreenZ();
		z = RwCameraGetFarClipPlane(Scene.camera);
	}else{
		screenz = RwIm2DGetNearScreenZ();
		z = 1.0f/RecipNearClip;
	}
	recipz = 1.0f/z;
	float offset = 1.0f/1024.0f;

	// This is what we draw:
	// 0---1
	// | / |
	// 3---2
	RwIm2DVertexSetScreenX(&maVertices[0], r.left);
	RwIm2DVertexSetScreenY(&maVertices[0], r.top);
	RwIm2DVertexSetScreenZ(&maVertices[0], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[0], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[0], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[0], c2.r, c2.g, c2.b, c2.a);
	RwIm2DVertexSetU(&maVertices[0], 0.0f+offset, recipz);
	RwIm2DVertexSetV(&maVertices[0], 0.0f+offset, recipz);

	RwIm2DVertexSetScreenX(&maVertices[1], r.right);
	RwIm2DVertexSetScreenY(&maVertices[1], r.top);
	RwIm2DVertexSetScreenZ(&maVertices[1], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[1], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[1], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[1], c3.r, c3.g, c3.b, c3.a);
	RwIm2DVertexSetU(&maVertices[1], 1.0f+offset, recipz);
	RwIm2DVertexSetV(&maVertices[1], 0.0f+offset, recipz);

	RwIm2DVertexSetScreenX(&maVertices[2], r.right);
	RwIm2DVertexSetScreenY(&maVertices[2], r.bottom);
	RwIm2DVertexSetScreenZ(&maVertices[2], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[2], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[2], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[2], c1.r, c1.g, c1.b, c1.a);
	RwIm2DVertexSetU(&maVertices[2], 1.0f+offset, recipz);
	RwIm2DVertexSetV(&maVertices[2], 1.0f+offset, recipz);

	RwIm2DVertexSetScreenX(&maVertices[3], r.left);
	RwIm2DVertexSetScreenY(&maVertices[3], r.bottom);
	RwIm2DVertexSetScreenZ(&maVertices[3], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[3], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[3], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[3], c0.r, c0.g, c0.b, c0.a);
	RwIm2DVertexSetU(&maVertices[3], 0.0f+offset, recipz);
	RwIm2DVertexSetV(&maVertices[3], 1.0f+offset, recipz);
}

void
CSprite2d::SetVertices(const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3,
	float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
	float screenz, z, recipz;

	screenz = RwIm2DGetNearScreenZ();
	z = 1.0f/RecipNearClip;
	recipz = 1.0f/z;

	// This is what we draw:
	// 0---1
	// | / |
	// 3---2
	RwIm2DVertexSetScreenX(&maVertices[0], r.left);
	RwIm2DVertexSetScreenY(&maVertices[0], r.top);
	RwIm2DVertexSetScreenZ(&maVertices[0], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[0], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[0], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[0], c2.r, c2.g, c2.b, c2.a);
	RwIm2DVertexSetU(&maVertices[0], u0, recipz);
	RwIm2DVertexSetV(&maVertices[0], v0, recipz);

	RwIm2DVertexSetScreenX(&maVertices[1], r.right);
	RwIm2DVertexSetScreenY(&maVertices[1], r.top);
	RwIm2DVertexSetScreenZ(&maVertices[1], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[1], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[1], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[1], c3.r, c3.g, c3.b, c3.a);
	RwIm2DVertexSetU(&maVertices[1], u1, recipz);
	RwIm2DVertexSetV(&maVertices[1], v1, recipz);

	RwIm2DVertexSetScreenX(&maVertices[2], r.right);
	RwIm2DVertexSetScreenY(&maVertices[2], r.bottom);
	RwIm2DVertexSetScreenZ(&maVertices[2], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[2], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[2], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[2], c1.r, c1.g, c1.b, c1.a);
	RwIm2DVertexSetU(&maVertices[2], u2, recipz);
	RwIm2DVertexSetV(&maVertices[2], v2, recipz);

	RwIm2DVertexSetScreenX(&maVertices[3], r.left);
	RwIm2DVertexSetScreenY(&maVertices[3], r.bottom);
	RwIm2DVertexSetScreenZ(&maVertices[3], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[3], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[3], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[3], c0.r, c0.g, c0.b, c0.a);
	RwIm2DVertexSetU(&maVertices[3], u3, recipz);
	RwIm2DVertexSetV(&maVertices[3], v3, recipz);
}

void
CSprite2d::SetVertices(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
		const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3)
{
	float screenz, recipz;
	float z = RwCameraGetNearClipPlane(Scene.camera);	// not done by game

	screenz = RwIm2DGetNearScreenZ();
	recipz = RecipNearClip;

	RwIm2DVertexSetScreenX(&maVertices[0], x3);
	RwIm2DVertexSetScreenY(&maVertices[0], y3);
	RwIm2DVertexSetScreenZ(&maVertices[0], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[0], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[0], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[0], c2.r, c2.g, c2.b, c2.a);
	RwIm2DVertexSetU(&maVertices[0], 0.0f, recipz);
	RwIm2DVertexSetV(&maVertices[0], 0.0f, recipz);

	RwIm2DVertexSetScreenX(&maVertices[1], x4);
	RwIm2DVertexSetScreenY(&maVertices[1], y4);
	RwIm2DVertexSetScreenZ(&maVertices[1], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[1], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[1], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[1], c3.r, c3.g, c3.b, c3.a);
	RwIm2DVertexSetU(&maVertices[1], 1.0f, recipz);
	RwIm2DVertexSetV(&maVertices[1], 0.0f, recipz);

	RwIm2DVertexSetScreenX(&maVertices[2], x2);
	RwIm2DVertexSetScreenY(&maVertices[2], y2);
	RwIm2DVertexSetScreenZ(&maVertices[2], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[2], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[2], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[2], c1.r, c1.g, c1.b, c1.a);
	RwIm2DVertexSetU(&maVertices[2], 1.0f, recipz);
	RwIm2DVertexSetV(&maVertices[2], 1.0f, recipz);

	RwIm2DVertexSetScreenX(&maVertices[3], x1);
	RwIm2DVertexSetScreenY(&maVertices[3], y1);
	RwIm2DVertexSetScreenZ(&maVertices[3], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[3], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[3], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[3], c0.r, c0.g, c0.b, c0.a);
	RwIm2DVertexSetU(&maVertices[3], 0.0f, recipz);
	RwIm2DVertexSetV(&maVertices[3], 1.0f, recipz);
}

void
CSprite2d::SetVertices(int n, float *positions, float *uvs, const CRGBA &col)
{
	int i;
	float screenz, recipz, z;

	screenz = RwIm2DGetNearScreenZ();
	recipz = RecipNearClip;
	z = RwCameraGetNearClipPlane(Scene.camera);	// not done by game


	for(i = 0; i < n; i++){
		RwIm2DVertexSetScreenX(&maVertices[i], positions[i*2 + 0]);
		RwIm2DVertexSetScreenY(&maVertices[i], positions[i*2 + 1]);
		RwIm2DVertexSetScreenZ(&maVertices[i], screenz + 0.0001f);
		RwIm2DVertexSetCameraZ(&maVertices[i], z);
		RwIm2DVertexSetRecipCameraZ(&maVertices[i], recipz);
		RwIm2DVertexSetIntRGBA(&maVertices[i], col.r, col.g, col.b, col.a);
		RwIm2DVertexSetU(&maVertices[i], uvs[i*2 + 0], recipz);
		RwIm2DVertexSetV(&maVertices[i], uvs[i*2 + 1], recipz);
	}
}

void
CSprite2d::SetMaskVertices(int n, float *positions)
{
	int i;
	float screenz, recipz, z;

	screenz = RwIm2DGetNearScreenZ();
	recipz = RecipNearClip;
	z = RwCameraGetNearClipPlane(Scene.camera);	// not done by game

	for(i = 0; i < n; i++){
		RwIm2DVertexSetScreenX(&maVertices[i], positions[i*2 + 0]);
		RwIm2DVertexSetScreenY(&maVertices[i], positions[i*2 + 1]);
		RwIm2DVertexSetScreenZ(&maVertices[i], screenz);
		RwIm2DVertexSetCameraZ(&maVertices[i], z);
		RwIm2DVertexSetRecipCameraZ(&maVertices[i], recipz);
#if !defined(GTA_PS2_STUFF) && defined(RWLIBS)
		RwIm2DVertexSetIntRGBA(&maVertices[i], 0, 0, 0, 0);
#else
		RwIm2DVertexSetIntRGBA(&maVertices[i], 255, 255, 255, 255);
#endif
	}
}

void
CSprite2d::SetVertices(RwIm2DVertex *verts, const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3,
		float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
	float screenz, recipz, z;

	screenz = RwIm2DGetNearScreenZ();
	recipz = RecipNearClip;
	z = RwCameraGetNearClipPlane(Scene.camera);	// not done by game

	RwIm2DVertexSetScreenX(&verts[0], r.left);
	RwIm2DVertexSetScreenY(&verts[0], r.top);
	RwIm2DVertexSetScreenZ(&verts[0], screenz);
	RwIm2DVertexSetCameraZ(&verts[0], z);
	RwIm2DVertexSetRecipCameraZ(&verts[0], recipz);
	RwIm2DVertexSetIntRGBA(&verts[0], c2.r, c2.g, c2.b, c2.a);
	RwIm2DVertexSetU(&verts[0], u0, recipz);
	RwIm2DVertexSetV(&verts[0], v0, recipz);

	RwIm2DVertexSetScreenX(&verts[1], r.left);
	RwIm2DVertexSetScreenY(&verts[1], r.bottom);
	RwIm2DVertexSetScreenZ(&verts[1], screenz);
	RwIm2DVertexSetCameraZ(&verts[1], z);
	RwIm2DVertexSetRecipCameraZ(&verts[1], recipz);
	RwIm2DVertexSetIntRGBA(&verts[1], c0.r, c0.g, c0.b, c0.a);
	RwIm2DVertexSetU(&verts[1], u2, recipz);
	RwIm2DVertexSetV(&verts[1], v2, recipz);

	RwIm2DVertexSetScreenX(&verts[2], r.right);
	RwIm2DVertexSetScreenY(&verts[2], r.bottom);
	RwIm2DVertexSetScreenZ(&verts[2], screenz);
	RwIm2DVertexSetCameraZ(&verts[2], z);
	RwIm2DVertexSetRecipCameraZ(&verts[2], recipz);
	RwIm2DVertexSetIntRGBA(&verts[2], c1.r, c1.g, c1.b, c1.a);
	RwIm2DVertexSetU(&verts[2], u3, recipz);
	RwIm2DVertexSetV(&verts[2], v3, recipz);

	RwIm2DVertexSetScreenX(&verts[3], r.left);
	RwIm2DVertexSetScreenY(&verts[3], r.top);
	RwIm2DVertexSetScreenZ(&verts[3], screenz);
	RwIm2DVertexSetCameraZ(&verts[3], z);
	RwIm2DVertexSetRecipCameraZ(&verts[3], recipz);
	RwIm2DVertexSetIntRGBA(&verts[3], c2.r, c2.g, c2.b, c2.a);
	RwIm2DVertexSetU(&verts[3], u0, recipz);
	RwIm2DVertexSetV(&verts[3], v0, recipz);

	RwIm2DVertexSetScreenX(&verts[4], r.right);
	RwIm2DVertexSetScreenY(&verts[4], r.bottom);
	RwIm2DVertexSetScreenZ(&verts[4], screenz);
	RwIm2DVertexSetCameraZ(&verts[4], z);
	RwIm2DVertexSetRecipCameraZ(&verts[4], recipz);
	RwIm2DVertexSetIntRGBA(&verts[4], c1.r, c1.g, c1.b, c1.a);
	RwIm2DVertexSetU(&verts[4], u3, recipz);
	RwIm2DVertexSetV(&verts[4], v3, recipz);

	RwIm2DVertexSetScreenX(&verts[5], r.right);
	RwIm2DVertexSetScreenY(&verts[5], r.top);
	RwIm2DVertexSetScreenZ(&verts[5], screenz);
	RwIm2DVertexSetCameraZ(&verts[5], z);
	RwIm2DVertexSetRecipCameraZ(&verts[5], recipz);
	RwIm2DVertexSetIntRGBA(&verts[5], c3.r, c3.g, c3.b, c3.a);
	RwIm2DVertexSetU(&verts[5], u1, recipz);
	RwIm2DVertexSetV(&verts[5], v1, recipz);

}

void
CSprite2d::DrawRect(const CRect &r, const CRGBA &col)
{
	SetVertices(r, col, col, col, col, false);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)(col.a != 255));
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, maVertices, 4);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEGOURAUD);
}

void
CSprite2d::DrawRect(const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3)
{
	SetVertices(r, c0, c1, c2, c3, false);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, maVertices, 4);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
}

void
CSprite2d::DrawRectXLU(const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3)
{
	SetVertices(r, c0, c1, c2, c3, false);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
}

void CSprite2d::Draw2DPolygon(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, const CRGBA &color)
{
	SetVertices(x1, y1, x2, y2, x3, y3, x4, y4, color, color, color, color);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)(color.a != 255));
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEGOURAUD);
}
