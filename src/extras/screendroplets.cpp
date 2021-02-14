#define WITHD3D
#include "common.h"

#ifdef SCREEN_DROPLETS

#ifndef LIBRW
#error "Need librw for SCREEN_DROPLETS"
#endif

#include "General.h"
#include "main.h"
#include "RwHelper.h"
#include "Timer.h"
#include "Camera.h"
#include "World.h"
#include "ZoneCull.h"
#include "Weather.h"
#include "ParticleObject.h"
	#include "Pad.h"
#include "RenderBuffer.h"
#include "custompipes.h"
#include "postfx.h"
#include "screendroplets.h"

// for 640
#define MAXSIZE 15
#define MINSIZE 4

int ScreenDroplets::ms_initialised;
RwTexture *ScreenDroplets::ms_maskTex;
RwTexture *ScreenDroplets::ms_screenTex;

bool ScreenDroplets::ms_enabled = true;
bool ScreenDroplets::ms_movingEnabled = true;

ScreenDroplets::ScreenDrop ScreenDroplets::ms_drops[ScreenDroplets::MAXDROPS];
int ScreenDroplets::ms_numDrops;
ScreenDroplets::ScreenDropMoving ScreenDroplets::ms_dropsMoving[ScreenDroplets::MAXDROPSMOVING];
int ScreenDroplets::ms_numDropsMoving;

CVector ScreenDroplets::ms_prevCamUp;
CVector ScreenDroplets::ms_prevCamPos;
CVector ScreenDroplets::ms_camMoveDelta;
float ScreenDroplets::ms_camMoveDist;
CVector ScreenDroplets::ms_screenMoveDelta;
float ScreenDroplets::ms_screenMoveDist;
float ScreenDroplets::ms_camUpAngle;

int ScreenDroplets::ms_splashDuration;
CParticleObject *ScreenDroplets::ms_splashObject;

struct Im2DVertexUV2 : rw::RWDEVICE::Im2DVertex
{
	rw::float32 u2, v2;
};

#ifdef RW_D3D9
static void *screenDroplet_PS;
#endif
#ifdef RW_GL3
static rw::gl3::Shader *screenDroplet;
#endif

// platform specific
static void openim2d_uv2(void);
static void closeim2d_uv2(void);
static void RenderIndexedPrimitive_UV2(RwPrimitiveType primType, Im2DVertexUV2 *vertices, RwInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices);

static Im2DVertexUV2 VertexBuffer[TEMPBUFFERVERTSIZE];

void
ScreenDroplets::Initialise(void)
{
	Clear();
	ms_splashDuration = -1;
	ms_splashObject = nil;
}

// Create white circle mask for rain drops
static RwTexture*
CreateDropMask(int32 size)
{
	RwImage *img = RwImageCreate(size, size, 32);
	RwImageAllocatePixels(img);

	uint8 *pixels = RwImageGetPixels(img);
	int32 stride = RwImageGetStride(img);

	for(int y = 0; y < size; y++){
		float yf = ((y + 0.5f)/size - 0.5f)*2.0f;
		for(int x = 0; x < size; x++){
			float xf = ((x + 0.5f)/size - 0.5f)*2.0f;
			memset(&pixels[y*stride + x*4], xf*xf + yf*yf < 1.0f ? 0xFF : 0x00, 4);
		}
	}

	int32 width, height, depth, format;
	RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &width, &height, &depth, &format);
	RwRaster *ras = RwRasterCreate(width, height, depth, format);
	RwRasterSetFromImage(ras, img);
	RwImageDestroy(img);
	return RwTextureCreate(ras);
}

void
ScreenDroplets::InitDraw(void)
{
	ms_maskTex = CreateDropMask(64);

	ms_screenTex = RwTextureCreate(nil);
	RwTextureSetFilterMode(ms_screenTex, rwFILTERLINEAR);

	openim2d_uv2();
#ifdef RW_D3D9
#include "shaders/obj/screenDroplet_PS.inc"
	screenDroplet_PS = rw::d3d::createPixelShader(screenDroplet_PS_cso);
#endif
#ifdef RW_GL3
	using namespace rw::gl3;
	{
#include "shaders/obj/im2d_UV2_vert.inc"
#include "shaders/obj/screenDroplet_frag.inc"
	const char *vs[] = { shaderDecl, header_vert_src, im2d_UV2_vert_src, nil };
	const char *fs[] = { shaderDecl, header_frag_src, screenDroplet_frag_src, nil };
	screenDroplet = Shader::create(vs, fs);
	assert(screenDroplet);
	}
#endif

	ms_initialised = 1;
}

void
ScreenDroplets::Shutdown(void)
{
	if(ms_maskTex){
		RwTextureDestroy(ms_maskTex);
		ms_maskTex = nil;
	}
	if(ms_screenTex){
		RwTextureSetRaster(ms_screenTex, nil);
		RwTextureDestroy(ms_screenTex);
		ms_screenTex = nil;
	}
#ifdef RW_D3D9
	if(screenDroplet_PS){
		rw::d3d::destroyPixelShader(screenDroplet_PS);
		screenDroplet_PS = nil;
	}
#endif
#ifdef RW_GL3
	if(screenDroplet){
		screenDroplet->destroy();
		screenDroplet = nil;
	}
#endif

	closeim2d_uv2();
}

void
ScreenDroplets::Process(void)
{
	ProcessCameraMovement();
	SprayDrops();
	ProcessMoving();
	Fade();
}

static void
FlushBuffer(void)
{
	if(TempBufferIndicesStored){
		RenderIndexedPrimitive_UV2(rwPRIMTYPETRILIST,
			VertexBuffer, TempBufferVerticesStored,
			TempBufferRenderIndexList, TempBufferIndicesStored);
		TempBufferVerticesStored = 0;
		TempBufferIndicesStored = 0;
	}
}

static int
StartStoring(int numIndices, int numVertices, RwImVertexIndex **indexStart, Im2DVertexUV2 **vertexStart)
{
	if(TempBufferIndicesStored + numIndices >= TEMPBUFFERINDEXSIZE ||
	   TempBufferVerticesStored + numVertices >= TEMPBUFFERVERTSIZE)
		FlushBuffer();
        *indexStart = &TempBufferRenderIndexList[TempBufferIndicesStored];
        *vertexStart = &VertexBuffer[TempBufferVerticesStored];
	int vertOffset = TempBufferVerticesStored;
        TempBufferIndicesStored += numIndices;
        TempBufferVerticesStored += numVertices;
	return vertOffset;
}

void
ScreenDroplets::Render(void)
{
	ScreenDrop *drop;

	DefinedState();
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(ms_maskTex));
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

	RwTextureSetRaster(ms_screenTex, CPostFX::pBackBuffer);
#ifdef RW_D3D9
	rw::d3d::im2dOverridePS = screenDroplet_PS;
	rw::d3d::setTexture(1, ms_screenTex);
#endif
#ifdef RW_GL3
	rw::gl3::im2dOverrideShader = screenDroplet;
	rw::gl3::setTexture(1, ms_screenTex);
#endif

	RenderBuffer::ClearRenderBuffer();
	for(drop = &ms_drops[0]; drop < &ms_drops[MAXDROPS]; drop++)
		if(drop->active)
			AddToRenderList(drop);
	FlushBuffer();

#ifdef RW_D3D9
	rw::d3d::im2dOverridePS = nil;
	rw::d3d::setTexture(1, nil);
#endif
#ifdef RW_GL3
	rw::gl3::im2dOverrideShader = nil;
	rw::gl3::setTexture(1, nil);
#endif

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, FALSE);
}

void
ScreenDroplets::AddToRenderList(ScreenDroplets::ScreenDrop *drop)
{
	static float xy[] = {
		-1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f,
		1.0f, -1.0f
	};
	static float uv[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f
	};

	int i;
	RwImVertexIndex *indices;
	Im2DVertexUV2 *verts;
	int first = StartStoring(6, 4, &indices, &verts);

	float scale = 0.5f*SCREEN_SCALE_X(drop->size);

	float screenz = RwIm2DGetNearScreenZ();
	float z = RwCameraGetNearClipPlane(Scene.camera);
	float recipz = 1.0f/z;

	float magSize = SCREEN_SCALE_Y(drop->magnification*(300.0f-40.0f) + 40.0f);
	float ul = drop->x - magSize;
	float vt = drop->y - magSize;
	float ur = drop->x + magSize;
	float vb = drop->y + magSize;
	ul = Max(ul, 0.0f)/RwRasterGetWidth(CPostFX::pBackBuffer);
	vt = Max(vt, 0.0f)/RwRasterGetHeight(CPostFX::pBackBuffer);
	ur = Min(ur, SCREEN_WIDTH)/RwRasterGetWidth(CPostFX::pBackBuffer);
	vb = Min(vb, SCREEN_HEIGHT)/RwRasterGetHeight(CPostFX::pBackBuffer);

	for(i = 0; i < 4; i++){
		RwIm2DVertexSetScreenX(&verts[i], drop->x + xy[i*2]*scale);
		RwIm2DVertexSetScreenY(&verts[i], drop->y + xy[i*2+1]*scale);
		RwIm2DVertexSetScreenZ(&verts[i], screenz);
		RwIm2DVertexSetCameraZ(&verts[i], z);
		RwIm2DVertexSetRecipCameraZ(&verts[i], recipz);
		RwIm2DVertexSetIntRGBA(&verts[i], drop->color.r, drop->color.g, drop->color.b, drop->color.a);
		RwIm2DVertexSetU(&verts[i], uv[i*2], recipz);
		RwIm2DVertexSetV(&verts[i], uv[i*2+1], recipz);

		verts[i].u2 = i < 2 ? ul : ur;
		verts[i].v2 = i % 3 ? vt : vb;
	}
	indices[0] = first + 0;
	indices[1] = first + 1;
	indices[2] = first + 2;
	indices[3] = first + 2;
	indices[4] = first + 3;
	indices[5] = first + 0;
}

void
ScreenDroplets::Clear(void)
{
	ScreenDrop *drop;
	for(drop = &ms_drops[0]; drop < &ms_drops[MAXDROPS]; drop++)
		drop->active = false;
	ms_numDrops = 0;
}

ScreenDroplets::ScreenDrop*
ScreenDroplets::NewDrop(float x, float y, float size, float lifetime, bool fades, int r, int g, int b)
{
	ScreenDrop *drop;
	int i;

	for(i = 0, drop = ms_drops; i < MAXDROPS; i++, drop++)
		if(!ms_drops[i].active)
			goto found;
	return nil;
found:
	ms_numDrops++;
	drop->x = x;
	drop->y = y;
	drop->size = size;
	drop->magnification = (MAXSIZE - size + 1.0f) / (MAXSIZE - MINSIZE + 1.0f);
	drop->fades = fades;
	drop->active = true;
	drop->color.r = r;
	drop->color.g = g;
	drop->color.b = b;
	drop->color.a = 255;
	drop->time = 0.0f;
	drop->lifetime = lifetime;
	return drop;
}

void
ScreenDroplets::SetMoving(ScreenDroplets::ScreenDrop *drop)
{
	ScreenDropMoving *moving;
	for(moving = ms_dropsMoving; moving < &ms_dropsMoving[MAXDROPSMOVING]; moving++)
		if(moving->drop == nil)
			goto found;
	return;
found:
	ms_numDropsMoving++;
	moving->drop = drop;
	moving->dist = 0.0f;
}

void
ScreenDroplets::FillScreen(int n)
{
	float x, y, size;
	ScreenDrop *drop;

	if(!ms_initialised)
		return;
	ms_numDrops = 0;
	for(drop = &ms_drops[0]; drop < &ms_drops[MAXDROPS]; drop++){
		drop->active = false;
		if(drop < &ms_drops[n]){
			x = CGeneral::GetRandomNumber() % (int)SCREEN_WIDTH;
			y = CGeneral::GetRandomNumber() % (int)SCREEN_HEIGHT;
			size = CGeneral::GetRandomNumberInRange(MINSIZE, MAXSIZE);
			NewDrop(x, y, size, 2000.0f, true);
		}
	}
}

void
ScreenDroplets::FillScreenMoving(float amount, bool isBlood)
{
	int n = (ms_screenMoveDelta.z > 5.0f ? 1.5f : 1.0f)*amount*20.0f;
	float x, y, size;
	ScreenDrop *drop;

	while(n--)
		if(ms_numDrops < MAXDROPS && ms_numDropsMoving < MAXDROPSMOVING){
			x = CGeneral::GetRandomNumber() % (int)SCREEN_WIDTH;
			y = CGeneral::GetRandomNumber() % (int)SCREEN_HEIGHT;
			size = CGeneral::GetRandomNumberInRange(MINSIZE, MAXSIZE);
			drop = nil;
			if(isBlood)
				drop = NewDrop(x, y, size, 2000.0f, true, 255, 0, 0);
			else
				drop = NewDrop(x, y, size, 2000.0f, true);
			if(drop)
				SetMoving(drop);
		}
}

void
ScreenDroplets::RegisterSplash(CParticleObject *pobj)
{
	CVector dist = pobj->GetPosition() - ms_prevCamPos;
	if(dist.MagnitudeSqr() < 50.0f){	// 20 originally
		ms_splashDuration = 14;
		ms_splashObject = pobj;
	}
}

void
ScreenDroplets::ProcessCameraMovement(void)
{
	RwMatrix *camMat = RwFrameGetMatrix(RwCameraGetFrame(Scene.camera));
	CVector camPos = camMat->pos;
	CVector camUp = camMat->at;
	ms_camMoveDelta = camPos - ms_prevCamPos;
	ms_camMoveDist = ms_camMoveDelta.Magnitude();

	ms_prevCamUp = camUp;
	ms_prevCamPos = camPos;

	ms_screenMoveDelta.x = -RwV3dDotProduct(&camMat->right, &ms_camMoveDelta);
	ms_screenMoveDelta.y = RwV3dDotProduct(&camMat->up, &ms_camMoveDelta);
	ms_screenMoveDelta.z = RwV3dDotProduct(&camMat->at, &ms_camMoveDelta);
	ms_screenMoveDelta *= 10.0f;
	ms_screenMoveDist = ms_screenMoveDelta.Magnitude2D();

	uint16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
	bool isTopDown = mode == CCam::MODE_TOPDOWN || mode == CCam::MODE_GTACLASSIC || mode == CCam::MODE_TOP_DOWN_PED;
	bool isLookingInDirection = FindPlayerVehicle() && mode == CCam::MODE_1STPERSON &&
		(CPad::GetPad(0)->GetLookBehindForCar() || CPad::GetPad(0)->GetLookLeft() || CPad::GetPad(0)->GetLookRight());
	ms_enabled = !isTopDown && !isLookingInDirection;
	ms_movingEnabled = !isTopDown && !isLookingInDirection;

	// 0 when looking stright up, 180 when looking up or down
	ms_camUpAngle = RADTODEG(Acos(clamp(camUp.z, -1.0f, 1.0f)));
}

void
ScreenDroplets::SprayDrops(void)
{
	bool noRain = CCullZones::PlayerNoRain() || CCullZones::CamNoRain();
	if(!noRain && CWeather::Rain > 0.0f && ms_enabled){
		// 180 when looking stright up, 0 when looking up or down
		float angle = 180.0f - ms_camUpAngle;
		angle = Max(angle, 40.0f);	// want at least some rain
		FillScreenMoving((angle - 40.0f) / 150.0f * CWeather::Rain * 0.5f);
	}

	int i;
	for(i = 0; i < MAX_AUDIOHYDRANTS; i++){
		CAudioHydrant *hyd = CAudioHydrant::Get(i);
		if (hyd->pParticleObject){
			CVector dist = hyd->pParticleObject->GetPosition() - ms_prevCamPos;
			if(dist.MagnitudeSqr() > 40.0f ||
			   DotProduct(dist, ms_prevCamUp) < 0.0f) continue;

			FillScreenMoving(1.0f);
		}
	}

	static int ndrops[] = {
		125, 250, 500, 1000, 1000,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	if(ms_splashDuration >= 0){
		if(ms_numDrops < MAXDROPS) {
			float numDropMult = 1.0f;
			if(ms_splashObject){
				float dist = (ms_splashObject->GetPosition() - ms_prevCamPos).Magnitude();
				numDropMult = 1.0f - (dist - 5.0f)/15.0f;
				if(numDropMult < 0) numDropMult = 0.0f;	// fix
			}
			int n = ndrops[ms_splashDuration] * numDropMult;
			while(n--)
				if(ms_numDrops < MAXDROPS){
					float x = CGeneral::GetRandomNumber() % (int)SCREEN_WIDTH;
					float y = CGeneral::GetRandomNumber() % (int)SCREEN_HEIGHT;
					float size = CGeneral::GetRandomNumberInRange(MINSIZE, MAXSIZE);
					NewDrop(x, y, size, 10000.0f, false);
				}
		}
		ms_splashDuration--;
	}
}

void
ScreenDroplets::NewTrace(ScreenDroplets::ScreenDropMoving *moving)
{
	if(ms_numDrops < MAXDROPS){
		moving->dist = 0.0f;
		NewDrop(moving->drop->x, moving->drop->y, MINSIZE, 500.0f, true,
			moving->drop->color.r, moving->drop->color.g, moving->drop->color.b);
	}
}

void
ScreenDroplets::MoveDrop(ScreenDroplets::ScreenDropMoving *moving)
{
	ScreenDrop *drop = moving->drop;
	if(!ms_movingEnabled)
		return;
	if(!drop->active){
		moving->drop = nil;
		ms_numDropsMoving--;
		return;
	}
	if(ms_screenMoveDelta.z > 0.0f && ms_camMoveDist > 0.3f){
		if(ms_screenMoveDist > 0.5f && TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON){
			// movement when camera turns
			moving->dist += ms_screenMoveDist;
			if(moving->dist > 20.0f && drop->color.a > 100)
				NewTrace(moving);

			drop->x -= ms_screenMoveDelta.x;
			drop->y += ms_screenMoveDelta.y;
		}else{
			// movement out of center 
			float d = ms_screenMoveDelta.z*0.2f;
			float dx, dy, sum;
			dx = drop->x - SCREEN_WIDTH*0.5f + ms_screenMoveDelta.x;
			if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON)
				dy = drop->y - SCREEN_HEIGHT*1.2f - ms_screenMoveDelta.y;
			else
				dy = drop->y - SCREEN_HEIGHT*0.5f - ms_screenMoveDelta.y;
			sum = fabs(dx) + fabs(dy);
			if(sum > 0.001f){
				dx /= sum;
				dy /= sum;
			}
			moving->dist += d;
			if(moving->dist > 20.0f && drop->color.a > 100)
				NewTrace(moving);
			drop->x += dx * d;
			drop->y += dy * d;
		}

		if(drop->x < 0.0f || drop->y < 0.0f ||
		   drop->x > SCREEN_WIDTH || drop->y > SCREEN_HEIGHT){
			moving->drop = nil;
			ms_numDropsMoving--;
		}
	}
}

void
ScreenDroplets::ProcessMoving(void)
{
	ScreenDropMoving *moving;
	if(!ms_movingEnabled)
		return;
	for(moving = ms_dropsMoving; moving < &ms_dropsMoving[MAXDROPSMOVING]; moving++)
		if(moving->drop)
			MoveDrop(moving);
}

void
ScreenDroplets::Fade(void)
{
	ScreenDrop *drop;
	for(drop = &ms_drops[0]; drop < &ms_drops[MAXDROPS]; drop++)
		if(drop->active)
			drop->Fade();
}

void
ScreenDroplets::ScreenDrop::Fade(void)
{
	int delta = CTimer::GetTimeStepInMilliseconds();
	time += delta;
	if(time < lifetime){
		color.a = 255 - time/lifetime*255;
	}else if(fades){
		ScreenDroplets::ms_numDrops--;
		active = false;
	}
}


/*
 * Im2D with two uv coors
 */

#ifdef RW_D3D9
// stolen from RW, not in a public header
namespace rw {
namespace d3d {
void addDynamicVB(uint32 length, uint32 fvf, IDirect3DVertexBuffer9 **buf);	// NB: don't share this pointer
void removeDynamicVB(IDirect3DVertexBuffer9 **buf);
void addDynamicIB(uint32 length, IDirect3DIndexBuffer9 **buf);	// NB: don't share this pointer
void removeDynamicIB(IDirect3DIndexBuffer9 **buf);
}
}
// different than im2d
#define NUMINDICES 1024
#define NUMVERTICES 1024

static int primTypeMap[] = {
	D3DPT_POINTLIST,	// invalid
	D3DPT_LINELIST,
	D3DPT_LINESTRIP,
	D3DPT_TRIANGLELIST,
	D3DPT_TRIANGLESTRIP,
	D3DPT_TRIANGLEFAN,
	D3DPT_POINTLIST,	// actually not supported!
};
// end of stolen stuff


static IDirect3DVertexDeclaration9 *im2ddecl_uv2;
static IDirect3DVertexBuffer9 *im2dvertbuf_uv2;
static IDirect3DIndexBuffer9 *im2dindbuf_uv2;

void
openim2d_uv2(void)
{
	using namespace rw;
	using namespace d3d;
	D3DVERTEXELEMENT9 elements[5] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
		{ 0, offsetof(Im2DVertexUV2, color), D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, offsetof(Im2DVertexUV2, u), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, offsetof(Im2DVertexUV2, u2), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};
	assert(im2ddecl_uv2 == nil);
	im2ddecl_uv2 = (IDirect3DVertexDeclaration9*)d3d9::createVertexDeclaration((d3d9::VertexElement*)elements);
	assert(im2ddecl_uv2);

	assert(im2dvertbuf_uv2 == nil);
	im2dvertbuf_uv2 = (IDirect3DVertexBuffer9*)createVertexBuffer(NUMVERTICES*sizeof(Im2DVertexUV2), 0, true);
	assert(im2dvertbuf_uv2);
	addDynamicVB(NUMVERTICES*sizeof(Im2DVertexUV2), 0, &im2dvertbuf_uv2);

	assert(im2dindbuf_uv2 == nil);
	im2dindbuf_uv2 = (IDirect3DIndexBuffer9*)createIndexBuffer(NUMINDICES*sizeof(rw::uint16), true);
	assert(im2dindbuf_uv2);
	addDynamicIB(NUMINDICES*sizeof(rw::uint16), &im2dindbuf_uv2);
}

void
closeim2d_uv2(void)
{
	using namespace rw;
	using namespace d3d;

	d3d9::destroyVertexDeclaration(im2ddecl_uv2);
	im2ddecl_uv2 = nil;

	removeDynamicVB(&im2dvertbuf_uv2);
	destroyVertexBuffer(im2dvertbuf_uv2);
	im2dvertbuf_uv2 = nil;

	removeDynamicIB(&im2dindbuf_uv2);
	destroyIndexBuffer(im2dindbuf_uv2);
	im2dindbuf_uv2 = nil;
}

void
RenderIndexedPrimitive_UV2(RwPrimitiveType primType, Im2DVertexUV2 *vertices, RwInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices)
{
	using namespace rw;
	using namespace d3d;

	if(numVertices > NUMVERTICES ||
	   numIndices > NUMINDICES){
		// TODO: error
		return;
	}
	rw::uint16 *lockedindices = lockIndices(im2dindbuf_uv2, 0, numIndices*sizeof(rw::uint16), D3DLOCK_DISCARD);
	memcpy(lockedindices, indices, numIndices*sizeof(rw::uint16));
	unlockIndices(im2dindbuf_uv2);

	rw::uint8 *lockedvertices = lockVertices(im2dvertbuf_uv2, 0, numVertices*sizeof(Im2DVertexUV2), D3DLOCK_DISCARD);
	memcpy(lockedvertices, vertices, numVertices*sizeof(Im2DVertexUV2));
	unlockVertices(im2dvertbuf_uv2);

	setStreamSource(0, im2dvertbuf_uv2, 0, sizeof(Im2DVertexUV2));
	setIndices(im2dindbuf_uv2);
	setVertexDeclaration(im2ddecl_uv2);

	if(im2dOverridePS)
		setPixelShader(im2dOverridePS);
	else if(engine->device.getRenderState(TEXTURERASTER))
		setPixelShader(im2d_tex_PS);
	else
		setPixelShader(im2d_PS);

	d3d::flushCache();

	rw::uint32 primCount = 0;
	switch(primType){
	case PRIMTYPELINELIST:
		primCount = numIndices/2;
		break;
	case PRIMTYPEPOLYLINE:
		primCount = numIndices-1;
		break;
	case PRIMTYPETRILIST:
		primCount = numIndices/3;
		break;
	case PRIMTYPETRISTRIP:
		primCount = numIndices-2;
		break;
	case PRIMTYPETRIFAN:
		primCount = numIndices-2;
		break;
	case PRIMTYPEPOINTLIST:
		primCount = numIndices;
		break;
	}
	d3ddevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)primTypeMap[primType], 0,
	                                0, numVertices,
	                                0, primCount);
}
#endif

#ifdef RW_GL3
// different than im2d
#define NUMINDICES 1024
#define NUMVERTICES 1024

static rw::gl3::AttribDesc im2d_UV2_attribDesc[4] = {
	{ rw::gl3::ATTRIB_POS,        GL_FLOAT,         GL_FALSE, 4,
		sizeof(Im2DVertexUV2), 0 },
	{ rw::gl3::ATTRIB_COLOR,      GL_UNSIGNED_BYTE, GL_TRUE,  4,
		sizeof(Im2DVertexUV2), offsetof(Im2DVertexUV2, r) },
	{ rw::gl3::ATTRIB_TEXCOORDS0, GL_FLOAT,         GL_FALSE, 2,
		sizeof(Im2DVertexUV2), offsetof(Im2DVertexUV2, u) },
	{ rw::gl3::ATTRIB_TEXCOORDS1, GL_FLOAT,         GL_FALSE, 2,
		sizeof(Im2DVertexUV2), offsetof(Im2DVertexUV2, u2) }
};

static int primTypeMap[] = {
	GL_POINTS,	// invalid
	GL_LINES,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN,
	GL_POINTS
};

static int32 u_xform;

uint32 im2D_UV2_Vbo, im2D_UV2_Ibo;
#ifdef RW_GL_USE_VAOS
uint32 im2D_UV2_Vao;
#endif

void
openim2d_uv2(void)
{
	u_xform = rw::gl3::registerUniform("u_xform");	// this doesn't add a new one, so it's safe

	glGenBuffers(1, &im2D_UV2_Ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, im2D_UV2_Ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, NUMINDICES*2, nil, GL_STREAM_DRAW);

	glGenBuffers(1, &im2D_UV2_Vbo);
	glBindBuffer(GL_ARRAY_BUFFER, im2D_UV2_Vbo);
	glBufferData(GL_ARRAY_BUFFER, NUMVERTICES*sizeof(Im2DVertexUV2), nil, GL_STREAM_DRAW);

#ifdef RW_GL_USE_VAOS
	glGenVertexArrays(1, &im2D_UV2_Vao);
	glBindVertexArray(im2D_UV2_Vao);
	setAttribPointers(im2d_UV2_attribDesc, 4);
#endif
}

void
closeim2d_uv2(void)
{
	glDeleteBuffers(1, &im2D_UV2_Ibo);
	glDeleteBuffers(1, &im2D_UV2_Vbo);
#ifdef RW_GL_USE_VAOS
	glDeleteVertexArrays(1, &im2D_UV2_Vao);
#endif
}

void
RenderIndexedPrimitive_UV2(RwPrimitiveType primType, Im2DVertexUV2 *vertices, RwInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices)
{
	using namespace rw;
	using namespace gl3;

	GLfloat xform[4];
	Camera *cam;
	cam = (Camera*)engine->currentCamera;

#ifdef RW_GL_USE_VAOS
	glBindVertexArray(im2D_UV2_Vao);
#endif

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, im2D_UV2_Ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, NUMINDICES*2, nil, GL_STREAM_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numIndices*2, indices);

	glBindBuffer(GL_ARRAY_BUFFER, im2D_UV2_Vbo);
	glBufferData(GL_ARRAY_BUFFER, NUMVERTICES*sizeof(Im2DVertexUV2), nil, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Im2DVertexUV2), vertices);

	xform[0] = 2.0f/cam->frameBuffer->width;
	xform[1] = -2.0f/cam->frameBuffer->height;
	xform[2] = -1.0f;
	xform[3] = 1.0f;

	if(im2dOverrideShader)
		im2dOverrideShader->use();
	else
		assert(0);//im2dShader->use();
#ifndef RW_GL_USE_VAOS
	setAttribPointers(im2d_UV2_attribDesc, 4);
#endif

	glUniform4fv(currentShader->uniformLocations[u_xform], 1, xform);

	flushCache();
	glDrawElements(primTypeMap[primType], numIndices,
	               GL_UNSIGNED_SHORT, nil);
#ifndef RW_GL_USE_VAOS
	disableAttribPointers(im2d_UV2_attribDesc, 4);
#endif
}
#endif

#endif
