#define WITHD3D
#include "common.h"

#ifdef EXTENDED_PIPELINES

#include "main.h"
#include "RwHelper.h"
#include "Lights.h"
#include "Timecycle.h"
#include "FileMgr.h"
#include "Clock.h"
#include "Weather.h"
#include "TxdStore.h"
#include "Renderer.h"
#include "World.h"
#include "custompipes.h"

#ifndef LIBRW
#error "Need librw for EXTENDED_PIPELINES"
#endif

namespace CustomPipes {

rw::int32 CustomMatOffset;

void*
CustomMatCtor(void *object, int32, int32)
{
	CustomMatExt *ext = GetCustomMatExt((rw::Material*)object);
	ext->glossTex = nil;
	ext->haveGloss = false;
	return object;
}

void*
CustomMatCopy(void *dst, void *src, int32, int32)
{
	CustomMatExt *srcext = GetCustomMatExt((rw::Material*)src);
	CustomMatExt *dstext = GetCustomMatExt((rw::Material*)dst);
	dstext->glossTex = srcext->glossTex;
	dstext->haveGloss = srcext->haveGloss;
	return dst;
}



rw::TexDictionary *neoTxd;

bool bRenderingEnvMap;
int32 EnvMapSize = 128;
rw::Camera *EnvMapCam;
rw::Texture *EnvMapTex;
rw::Texture *EnvMaskTex;
static rw::RWDEVICE::Im2DVertex EnvScreenQuad[4];
static int16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };

static rw::Camera*
CreateEnvMapCam(rw::World *world)
{
	rw::Raster *fbuf = rw::Raster::create(EnvMapSize, EnvMapSize, 0, rw::Raster::CAMERATEXTURE);
	if(fbuf){
		rw::Raster *zbuf = rw::Raster::create(EnvMapSize, EnvMapSize, 0, rw::Raster::ZBUFFER);
		if(zbuf){
			rw::Frame *frame = rw::Frame::create();
			if(frame){
				rw::Camera *cam = rw::Camera::create();
				if(cam){
					cam->frameBuffer = fbuf;
					cam->zBuffer = zbuf;
					cam->setFrame(frame);
					cam->setNearPlane(0.1f);
					cam->setFarPlane(250.0f);
					rw::V2d vw = { 2.0f, 2.0f };
					cam->setViewWindow(&vw);
					world->addCamera(cam);
					EnvMapTex = rw::Texture::create(fbuf);
					EnvMapTex->setFilter(rw::Texture::LINEAR);

					frame->matrix.right.x = -1.0f;
					frame->matrix.up.y = -1.0f;
					frame->matrix.update();
					return cam;
				}
				frame->destroy();
			}
			zbuf->destroy();
		}
		fbuf->destroy();
	}
	return nil;
}

static void
DestroyCam(rw::Camera *cam)
{
	if(cam == nil)
		return;
	if(cam->frameBuffer){
		cam->frameBuffer->destroy();
		cam->frameBuffer = nil;
	}
	if(cam->zBuffer){
		cam->zBuffer->destroy();
		cam->zBuffer = nil;
	}
	rw::Frame *f = cam->getFrame();
	if(f){
		cam->setFrame(nil);
		f->destroy();
	}
	cam->world->removeCamera(cam);
	cam->destroy();
}

void
RenderEnvMapScene(void)
{
	CRenderer::RenderRoads();
	CRenderer::RenderEverythingBarRoads();
	CRenderer::RenderFadingInEntities();
}

void
EnvMapRender(void)
{
	if(VehiclePipeSwitch != VEHICLEPIPE_NEO)
		return;

	RwCameraEndUpdate(Scene.camera);

	// Neo does this differently, but i'm not quite convinced it's much better
	rw::V3d camPos = FindPlayerCoors();
	EnvMapCam->getFrame()->matrix.pos = camPos;
	EnvMapCam->getFrame()->transform(&EnvMapCam->getFrame()->matrix, rw::COMBINEREPLACE);

	rw::RGBA skycol;
	skycol.red = CTimeCycle::GetSkyBottomRed();
	skycol.green = CTimeCycle::GetSkyBottomGreen();
	skycol.blue = CTimeCycle::GetSkyBottomBlue();
	skycol.alpha = 255;
	EnvMapCam->clear(&skycol, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);
	RwCameraBeginUpdate(EnvMapCam);
	bRenderingEnvMap = true;
	RenderEnvMapScene();
	bRenderingEnvMap = false;

	if(EnvMaskTex){
		rw::SetRenderState(rw::VERTEXALPHA, TRUE);
		rw::SetRenderState(rw::SRCBLEND, rw::BLENDZERO);
		rw::SetRenderState(rw::DESTBLEND, rw::BLENDSRCCOLOR);
		rw::SetRenderStatePtr(rw::TEXTURERASTER, EnvMaskTex->raster);
		rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, EnvScreenQuad, 4, QuadIndices, 6);
		rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
		rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	}
	RwCameraEndUpdate(EnvMapCam);


	RwCameraBeginUpdate(Scene.camera);

	// debug env map
//	rw::SetRenderStatePtr(rw::TEXTURERASTER, EnvMapTex->raster);
//	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, EnvScreenQuad, 4, QuadIndices, 6);
}

static void
EnvMapInit(void)
{
	if(neoTxd)
		EnvMaskTex = neoTxd->find("CarReflectionMask");

	EnvMapCam = CreateEnvMapCam(Scene.world);

	int width = EnvMapCam->frameBuffer->width;
	int height = EnvMapCam->frameBuffer->height;
	float screenZ = RwIm2DGetNearScreenZ();
	float recipZ = 1.0f/EnvMapCam->nearPlane;

	EnvScreenQuad[0].setScreenX(0.0f);
	EnvScreenQuad[0].setScreenY(0.0f);
	EnvScreenQuad[0].setScreenZ(screenZ);
	EnvScreenQuad[0].setCameraZ(EnvMapCam->nearPlane);
	EnvScreenQuad[0].setRecipCameraZ(recipZ);
	EnvScreenQuad[0].setColor(255, 255, 255, 255);
	EnvScreenQuad[0].setU(0.0f, recipZ);
	EnvScreenQuad[0].setV(0.0f, recipZ);

	EnvScreenQuad[1].setScreenX(0.0f);
	EnvScreenQuad[1].setScreenY(height);
	EnvScreenQuad[1].setScreenZ(screenZ);
	EnvScreenQuad[1].setCameraZ(EnvMapCam->nearPlane);
	EnvScreenQuad[1].setRecipCameraZ(recipZ);
	EnvScreenQuad[1].setColor(255, 255, 255, 255);
	EnvScreenQuad[1].setU(0.0f, recipZ);
	EnvScreenQuad[1].setV(1.0f, recipZ);

	EnvScreenQuad[2].setScreenX(width);
	EnvScreenQuad[2].setScreenY(height);
	EnvScreenQuad[2].setScreenZ(screenZ);
	EnvScreenQuad[2].setCameraZ(EnvMapCam->nearPlane);
	EnvScreenQuad[2].setRecipCameraZ(recipZ);
	EnvScreenQuad[2].setColor(255, 255, 255, 255);
	EnvScreenQuad[2].setU(1.0f, recipZ);
	EnvScreenQuad[2].setV(1.0f, recipZ);

	EnvScreenQuad[3].setScreenX(width);
	EnvScreenQuad[3].setScreenY(0.0f);
	EnvScreenQuad[3].setScreenZ(screenZ);
	EnvScreenQuad[3].setCameraZ(EnvMapCam->nearPlane);
	EnvScreenQuad[3].setRecipCameraZ(recipZ);
	EnvScreenQuad[3].setColor(255, 255, 255, 255);
	EnvScreenQuad[3].setU(1.0f, recipZ);
	EnvScreenQuad[3].setV(0.0f, recipZ);
}

static void
EnvMapShutdown(void)
{
	EnvMapTex->raster = nil;
	EnvMapTex->destroy();
	EnvMapTex = nil;
	DestroyCam(EnvMapCam);
	EnvMapCam = nil;
}

/*
 * Tweak values
 */

#define INTERP_SETUP \
		int h1 = CClock::GetHours();								  \
		int h2 = (h1+1)%24;										  \
		int w1 = CWeather::OldWeatherType;								  \
		int w2 = CWeather::NewWeatherType;								  \
		float timeInterp = (CClock::GetSeconds()/60.0f + CClock::GetMinutes())/60.0f;	  \
		float c0 = (1.0f-timeInterp)*(1.0f-CWeather::InterpolationValue);				  \
		float c1 = timeInterp*(1.0f-CWeather::InterpolationValue);					  \
		float c2 = (1.0f-timeInterp)*CWeather::InterpolationValue;					  \
		float c3 = timeInterp*CWeather::InterpolationValue;
#define INTERP(v) v[h1][w1]*c0 + v[h2][w1]*c1 + v[h1][w2]*c2 + v[h2][w2]*c3;
#define INTERPF(v,f) v[h1][w1].f*c0 + v[h2][w1].f*c1 + v[h1][w2].f*c2 + v[h2][w2].f*c3;

InterpolatedFloat::InterpolatedFloat(float init)
{
	curInterpolator = 61;	// compared against second
	for(int h = 0; h < 24; h++)
		for(int w = 0; w < NUMWEATHERS; w++)
			data[h][w] = init;
}

void
InterpolatedFloat::Read(char *s, int line, int field)
{
	sscanf(s, "%f", &data[line][field]);
}

float
InterpolatedFloat::Get(void)
{
	if(curInterpolator != CClock::GetSeconds()){
		INTERP_SETUP
		curVal = INTERP(data);
		curInterpolator = CClock::GetSeconds();
	}
	return curVal;
}

InterpolatedColor::InterpolatedColor(const Color &init)
{
	curInterpolator = 61;	// compared against second
	for(int h = 0; h < 24; h++)
		for(int w = 0; w < NUMWEATHERS; w++)
			data[h][w] = init;
}

void
InterpolatedColor::Read(char *s, int line, int field)
{
	int r, g, b, a;
	sscanf(s, "%i, %i, %i, %i", &r, &g, &b, &a);
	data[line][field] = Color(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
}

Color
InterpolatedColor::Get(void)
{
	if(curInterpolator != CClock::GetSeconds()){
		INTERP_SETUP
		curVal.r = INTERPF(data, r);
		curVal.g = INTERPF(data, g);
		curVal.b = INTERPF(data, b);
		curVal.a = INTERPF(data, a);
		curInterpolator = CClock::GetSeconds();
	}
	return curVal;
}

void
InterpolatedLight::Read(char *s, int line, int field)
{
	int r, g, b, a;
	sscanf(s, "%i, %i, %i, %i", &r, &g, &b, &a);
	data[line][field] = Color(r/255.0f, g/255.0f, b/255.0f, a/100.0f);
}

char*
ReadTweakValueTable(char *fp, InterpolatedValue &interp)
{
	char buf[24], *p;
	int c;
	int line, field;

	line = 0;
	c = *fp++;
	while(c != '\0' && line < 24){
		field = 0;
		if(c != '\0' && c != '#'){
			while(c != '\0' && c != '\n' && field < NUMWEATHERS){
				p = buf;
				while(c != '\0' && c == '\t')
					c = *fp++;
				*p++ = c;
				while(c = *fp++, c != '\0' && c != '\t' && c != '\n')
					*p++ = c;
				*p++ = '\0';
				interp.Read(buf, line, field);
				field++;
			}
			line++;
		}
		while(c != '\0' && c != '\n')
			c = *fp++;
		c = *fp++;
	}
	return fp-1;
}



/*
 * Neo Vehicle pipe
 */

int32 VehiclePipeSwitch = VEHICLEPIPE_MATFX;
float VehicleShininess = 0.7f;	// the default is a bit extreme
float VehicleSpecularity = 1.0f;
InterpolatedFloat Fresnel(0.4f);
InterpolatedFloat Power(18.0f);
InterpolatedLight DiffColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
InterpolatedLight SpecColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
rw::ObjPipeline *vehiclePipe;

void
AttachVehiclePipe(rw::Atomic *atomic)
{
	atomic->pipeline = vehiclePipe;
}

void
AttachVehiclePipe(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		AttachVehiclePipe(rw::Atomic::fromClump(lnk));
}



/*
 * Neo World pipe
 */

bool LightmapEnable;
float LightmapMult = 1.0f;
InterpolatedFloat WorldLightmapBlend(1.0f);
rw::ObjPipeline *worldPipe;

void
AttachWorldPipe(rw::Atomic *atomic)
{
	atomic->pipeline = worldPipe;
}

void
AttachWorldPipe(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		AttachWorldPipe(rw::Atomic::fromClump(lnk));
}




/*
 * Neo Gloss pipe
 */

bool GlossEnable;
float GlossMult = 1.0f;
rw::ObjPipeline *glossPipe;

rw::Texture*
GetGlossTex(rw::Material *mat)
{
	if(neoTxd == nil)
		return nil;
	CustomMatExt *ext = GetCustomMatExt(mat);
	if(!ext->haveGloss){
		char glossname[128];
		strcpy(glossname, mat->texture->name);
		strcat(glossname, "_gloss");
		ext->glossTex = neoTxd->find(glossname);
		ext->haveGloss = true;
	}
	return ext->glossTex;
}

void
AttachGlossPipe(rw::Atomic *atomic)
{
	atomic->pipeline = glossPipe;
}

void
AttachGlossPipe(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		AttachWorldPipe(rw::Atomic::fromClump(lnk));
}



/*
 * Neo Rim pipes
 */

bool RimlightEnable;
float RimlightMult = 1.0f;
InterpolatedColor RampStart(Color(0.0f, 0.0f, 0.0f, 1.0f));
InterpolatedColor RampEnd(Color(1.0f, 1.0f, 1.0f, 1.0f));
InterpolatedFloat Offset(0.5f);
InterpolatedFloat Scale(1.5f);
InterpolatedFloat Scaling(2.0f);
rw::ObjPipeline *rimPipe;
rw::ObjPipeline *rimSkinPipe;

void
AttachRimPipe(rw::Atomic *atomic)
{
	if(rw::Skin::get(atomic->geometry))
		atomic->pipeline = rimSkinPipe;
	else
		atomic->pipeline = rimPipe;
}

void
AttachRimPipe(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		AttachRimPipe(rw::Atomic::fromClump(lnk));
}

/*
 * High level stuff
 */

void
CustomPipeInit(void)
{
	RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, "neo/neo.txd");
	if(stream == nil)
		printf("Error: couldn't open 'neo/neo.txd'\n");
	else{
		if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil))
			neoTxd = RwTexDictionaryGtaStreamRead(stream);
		RwStreamClose(stream, nil);
	}

	EnvMapInit();

	CreateVehiclePipe();
	CreateWorldPipe();
	CreateGlossPipe();
	CreateRimLightPipes();
}

void
CustomPipeShutdown(void)
{
	DestroyVehiclePipe();
	DestroyWorldPipe();
	DestroyGlossPipe();
	DestroyRimLightPipes();

	EnvMapShutdown();

	if(neoTxd){
		neoTxd->destroy();
		neoTxd = nil;
	}
}

void
CustomPipeRegister(void)
{
#ifdef RW_OPENGL
	CustomPipeRegisterGL();
#endif

	CustomMatOffset = rw::Material::registerPlugin(sizeof(CustomMatExt), MAKECHUNKID(rwVENDORID_ROCKSTAR, 0x80),
		CustomMatCtor, nil, CustomMatCopy);
}


// Load textures from generic as fallback

rw::TexDictionary *genericTxd;
rw::Texture *(*defaultFindCB)(const char *name);

static rw::Texture*
customFindCB(const char *name)
{
	rw::Texture *res = defaultFindCB(name);
	if(res == nil)
		res = genericTxd->find(name);
	return res;
}

void
SetTxdFindCallback(void)
{
	int slot = CTxdStore::FindTxdSlot("generic");
	CTxdStore::AddRef(slot);
	// TODO: function for this
	genericTxd = CTxdStore::GetSlot(slot)->texDict;
	assert(genericTxd);
	if(defaultFindCB == nil)
		defaultFindCB = rw::Texture::findCB;
	rw::Texture::findCB = customFindCB;
}

}

#endif
