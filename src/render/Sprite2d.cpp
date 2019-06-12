#include "common.h"
#include "patcher.h"
#include "Draw.h"
#include "Camera.h"
#include "Sprite2d.h"

RwIm2DVertex *CSprite2d::maVertices = (RwIm2DVertex*)0x6E9168;
float &CSprite2d::RecipNearClip = *(float*)0x880DB4;
int32 &CSprite2d::mCurrentBank = *(int32*)0x8F1AF4;
RwTexture **CSprite2d::mpBankTextures = (RwTexture**)0x774DC0;
int32 *CSprite2d::mCurrentSprite = (int32*)0x6F4500;
int32 *CSprite2d::mBankStart = (int32*)0x774BE8;
RwIm2DVertex *CSprite2d::maBankVertices = (RwIm2DVertex*)0x8429F8;

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
	for(i = 0; i < 10; i++)
		mpBankTextures[i] = nil;
}

int32
CSprite2d::GetBank(int32 n, RwTexture *tex)
{
	mpBankTextures[mCurrentBank] = tex;
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
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,
		mpBankTextures[bank] ? RwTextureGetRaster(mpBankTextures[bank]) : nil);
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
	RwIm2DVertexSetU(&maVertices[0], 0.0f, recipz);
	RwIm2DVertexSetV(&maVertices[0], 0.0f, recipz);

	RwIm2DVertexSetScreenX(&maVertices[1], r.right);
	RwIm2DVertexSetScreenY(&maVertices[1], r.top);
	RwIm2DVertexSetScreenZ(&maVertices[1], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[1], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[1], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[1], c3.r, c3.g, c3.b, c3.a);
	RwIm2DVertexSetU(&maVertices[1], 1.0f, recipz);
	RwIm2DVertexSetV(&maVertices[1], 0.0f, recipz);

	RwIm2DVertexSetScreenX(&maVertices[2], r.right);
	RwIm2DVertexSetScreenY(&maVertices[2], r.bottom);
	RwIm2DVertexSetScreenZ(&maVertices[2], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[2], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[2], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[2], c1.r, c1.g, c1.b, c1.a);
	RwIm2DVertexSetU(&maVertices[2], 1.0f, recipz);
	RwIm2DVertexSetV(&maVertices[2], 1.0f, recipz);

	RwIm2DVertexSetScreenX(&maVertices[3], r.left);
	RwIm2DVertexSetScreenY(&maVertices[3], r.bottom);
	RwIm2DVertexSetScreenZ(&maVertices[3], screenz);
	RwIm2DVertexSetCameraZ(&maVertices[3], z);
	RwIm2DVertexSetRecipCameraZ(&maVertices[3], recipz);
	RwIm2DVertexSetIntRGBA(&maVertices[3], c0.r, c0.g, c0.b, c0.a);
	RwIm2DVertexSetU(&maVertices[3], 0.0f, recipz);
	RwIm2DVertexSetV(&maVertices[3], 1.0f, recipz);
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
	float screenz, recipz;

	screenz = RwIm2DGetNearScreenZ();
	recipz = RecipNearClip;


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
	float screenz, recipz;

	screenz = RwIm2DGetNearScreenZ();
	recipz = RecipNearClip;

	for(i = 0; i < n; i++){
		RwIm2DVertexSetScreenX(&maVertices[i], positions[i*2 + 0]);
		RwIm2DVertexSetScreenY(&maVertices[i], positions[i*2 + 1]);
		RwIm2DVertexSetScreenZ(&maVertices[i], screenz);
		RwIm2DVertexSetCameraZ(&maVertices[i], z);
		RwIm2DVertexSetRecipCameraZ(&maVertices[i], recipz);
		RwIm2DVertexSetIntRGBA(&maVertices[i], 0, 0, 0, 0);
	}
}

void
CSprite2d::SetVertices(RwIm2DVertex *verts, const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3,
		float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
	float screenz, recipz;

	screenz = RwIm2DGetNearScreenZ();
	recipz = RecipNearClip;

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

STARTPATCHES
#define C4 const CRGBA&, const CRGBA&, const CRGBA&, const CRGBA&
#define F8 float, float, float, float, float, float, float, float

	InjectHook(0x51EA20, CSprite2d::SetRecipNearClip, PATCH_JUMP);
	InjectHook(0x51EAE0, CSprite2d::InitPerFrame, PATCH_JUMP);
	InjectHook(0x51EB70, CSprite2d::GetBank, PATCH_JUMP);
	InjectHook(0x51EBC0, CSprite2d::AddSpriteToBank, PATCH_JUMP);
	InjectHook(0x51EC50, CSprite2d::DrawBank, PATCH_JUMP);

	InjectHook(0x51EA00, &CSprite2d::Delete, PATCH_JUMP);
	InjectHook(0x51F950, &CSprite2d::SetRenderState, PATCH_JUMP);
	InjectHook(0x51EA40, (void (CSprite2d::*)(const char*))&CSprite2d::SetTexture, PATCH_JUMP);
	InjectHook(0x51EA70, (void (CSprite2d::*)(const char*,const char*))&CSprite2d::SetTexture, PATCH_JUMP);
	InjectHook(0x51EAA0, &CSprite2d::SetAddressing, PATCH_JUMP);

	InjectHook(0x51EE90, (void (*)(const CRect&, C4, uint32))CSprite2d::SetVertices, PATCH_JUMP);
	InjectHook(0x51F220, (void (*)(const CRect&, C4, F8))CSprite2d::SetVertices, PATCH_JUMP);
	InjectHook(0x51F070, (void (*)(F8, C4))CSprite2d::SetVertices, PATCH_JUMP);
	InjectHook(0x51F3E0, (void (*)(int, float*, float*, const CRGBA&))CSprite2d::SetVertices, PATCH_JUMP);
	InjectHook(0x51F490, CSprite2d::SetMaskVertices, PATCH_JUMP);
	InjectHook(0x51F720, (void (*)(RwIm2DVertex*, const CRect&, C4, F8))CSprite2d::SetVertices, PATCH_JUMP);

	InjectHook(0x51ECE0, (void (CSprite2d::*)(float, float, float, float, const CRGBA &))&CSprite2d::Draw, PATCH_JUMP);
	InjectHook(0x51ED50, (void (CSprite2d::*)(const CRect &, const CRGBA &))&CSprite2d::Draw, PATCH_JUMP);
	InjectHook(0x51ED90, (void (CSprite2d::*)(const CRect &, const CRGBA &, F8))&CSprite2d::Draw, PATCH_JUMP);
	InjectHook(0x51EDF0, (void (CSprite2d::*)(const CRect &, C4))&CSprite2d::Draw, PATCH_JUMP);
	InjectHook(0x51EE40, (void (CSprite2d::*)(F8, const CRGBA &))&CSprite2d::Draw, PATCH_JUMP);

	InjectHook(0x51F970, (void (*)(const CRect&, const CRGBA&))CSprite2d::DrawRect, PATCH_JUMP);
	InjectHook(0x51FA00, (void (*)(const CRect&, C4))CSprite2d::DrawRect, PATCH_JUMP);
	InjectHook(0x51FA80, CSprite2d::DrawRectXLU, PATCH_JUMP);
ENDPATCHES
