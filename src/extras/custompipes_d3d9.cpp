#define WITHD3D
#include "common.h"

#ifdef RW_D3D9
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

#ifdef EXTENDED_PIPELINES

#ifndef LIBRW
#error "Need librw for EXTENDED_PIPELINES"
#endif

extern RwTexture *gpWhiteTexture;	// from vehicle model info

namespace CustomPipes {

enum {
	// rim pipe
	VSLOC_boneMatrices = rw::d3d::VSLOC_afterLights,
	VSLOC_viewVec =	VSLOC_boneMatrices + 64*3,
	VSLOC_rampStart,
	VSLOC_rampEnd,
	VSLOC_rimData,

	// gloss pipe
	VSLOC_eye = rw::d3d::VSLOC_afterLights,

	VSLOC_reflProps,
	VSLOC_specLights
};

/*
 * Neo Vehicle pipe
 */

static void *neoVehicle_VS;
static void *neoVehicle_PS;

void
uploadSpecLights(void)
{
	struct VsLight {
		rw::RGBAf color;
		float pos[4];	// unused
		rw::V3d dir;
		float power;
	} specLights[1 + NUMEXTRADIRECTIONALS];
	memset(specLights, 0, sizeof(specLights));
	for(int i = 0; i < 1+NUMEXTRADIRECTIONALS; i++)
		specLights[i].power = 1.0f;
	float power = Power.Get();
	Color speccol = SpecColor.Get();
	specLights[0].color.red = speccol.r;
	specLights[0].color.green = speccol.g;
	specLights[0].color.blue = speccol.b;
	specLights[0].dir = pDirect->getFrame()->getLTM()->at;
	specLights[0].power = power;
	for(int i = 0; i < NUMEXTRADIRECTIONALS; i++){
		if(pExtraDirectionals[i]->getFlags() & rw::Light::LIGHTATOMICS){
			specLights[1+i].color = pExtraDirectionals[i]->color;
			specLights[1+i].dir = pExtraDirectionals[i]->getFrame()->getLTM()->at;
			specLights[1+i].power = power*2.0f;
		}
	}
	rw::d3d::d3ddevice->SetVertexShaderConstantF(VSLOC_specLights, (float*)&specLights, 3*(1 + NUMEXTRADIRECTIONALS));
}

void
vehicleRenderCB(rw::Atomic *atomic, rw::d3d9::InstanceDataHeader *header)
{
	using namespace rw;
	using namespace rw::d3d;
	using namespace rw::d3d9;

	// TODO: make this less of a kludge
	if(VehiclePipeSwitch == VEHICLEPIPE_MATFX){
		matFXGlobals.pipelines[rw::platform]->render(atomic);
		return;
	}

	int vsBits;
	rw::uint32 flags = atomic->geometry->flags;
	setStreamSource(0, header->vertexStream[0].vertexBuffer, 0, header->vertexStream[0].stride);
	setIndices(header->indexBuffer);
	setVertexDeclaration(header->vertexDeclaration);

	vsBits = lightingCB_Shader(atomic);
	uploadSpecLights();
	uploadMatrices(atomic->getFrame()->getLTM());

	setVertexShader(neoVehicle_VS);

	V3d eyePos = rw::engine->currentCamera->getFrame()->getLTM()->pos;
	d3ddevice->SetVertexShaderConstantF(VSLOC_eye, (float*)&eyePos, 1);

	float reflProps[4];
	reflProps[0] = Fresnel.Get();
	reflProps[1] = SpecColor.Get().a;

	d3d::setTexture(1, EnvMapTex);

	SetRenderState(SRCBLEND, BLENDONE);

	InstanceData *inst = header->inst;
	for(rw::uint32 i = 0; i < header->numMeshes; i++){
		Material *m = inst->material;

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || m->color.alpha != 255);

		reflProps[2] = m->surfaceProps.specular * VehicleShininess;
		reflProps[3] = m->surfaceProps.specular == 0.0f ? 0.0f : VehicleSpecularity;
		d3ddevice->SetVertexShaderConstantF(VSLOC_reflProps, reflProps, 1);

		setMaterial(flags, m->color, m->surfaceProps);

		if(m->texture)
			d3d::setTexture(0, m->texture);
		else
			d3d::setTexture(0, gpWhiteTexture);
		setPixelShader(neoVehicle_PS);

		drawInst(header, inst);
		inst++;
	}
	d3d::setTexture(1, nil);

	SetRenderState(SRCBLEND, BLENDSRCALPHA);
}

void
CreateVehiclePipe(void)
{
	if(CFileMgr::LoadFile("neo/carTweakingTable.dat", work_buff, sizeof(work_buff), "r") <= 0)
		printf("Error: couldn't open 'neo/carTweakingTable.dat'\n");
	else{
		char *fp = (char*)work_buff;
		fp = ReadTweakValueTable(fp, Fresnel);
		fp = ReadTweakValueTable(fp, Power);
		fp = ReadTweakValueTable(fp, DiffColor);
		fp = ReadTweakValueTable(fp, SpecColor);
	}

#include "shaders/obj/neoVehicle_VS.inc"
	neoVehicle_VS = rw::d3d::createVertexShader(neoVehicle_VS_cso);
	assert(neoVehicle_VS);

#include "shaders/obj/neoVehicle_PS.inc"
	neoVehicle_PS = rw::d3d::createPixelShader(neoVehicle_PS_cso);
	assert(neoVehicle_PS);


	rw::d3d9::ObjPipeline *pipe = rw::d3d9::ObjPipeline::create();
	pipe->instanceCB = rw::d3d9::defaultInstanceCB;
	pipe->uninstanceCB = rw::d3d9::defaultUninstanceCB;
	pipe->renderCB = vehicleRenderCB;
	vehiclePipe = pipe;
}

void
DestroyVehiclePipe(void)
{
	rw::d3d::destroyVertexShader(neoVehicle_VS);
	neoVehicle_VS = nil;

	rw::d3d::destroyPixelShader(neoVehicle_PS);
	neoVehicle_PS = nil;

	((rw::d3d9::ObjPipeline*)vehiclePipe)->destroy();
	vehiclePipe = nil;
}



/*
 * Neo World pipe
 */

static void *neoWorld_VS;
static void *neoWorldIII_PS;

static void
worldRenderCB(rw::Atomic *atomic, rw::d3d9::InstanceDataHeader *header)
{
	using namespace rw;
	using namespace rw::d3d;
	using namespace rw::d3d9;

	if(!LightmapEnable){
		defaultRenderCB_Shader(atomic, header);
		return;
	}

	int vsBits;
	setStreamSource(0, header->vertexStream[0].vertexBuffer, 0, header->vertexStream[0].stride);
	setIndices(header->indexBuffer);
	setVertexDeclaration(header->vertexDeclaration);

	vsBits = lightingCB_Shader(atomic);
	uploadMatrices(atomic->getFrame()->getLTM());


	float lightfactor[4];

	InstanceData *inst = header->inst;
	for(rw::uint32 i = 0; i < header->numMeshes; i++){
		Material *m = inst->material;

		if(MatFX::getEffects(m) == MatFX::DUAL){
			setVertexShader(neoWorld_VS);

			MatFX *matfx = MatFX::get(m);
			Texture *dualtex = matfx->getDualTexture();
			if(dualtex == nil)
				goto notex;
			d3d::setTexture(1, dualtex);
			lightfactor[0] = lightfactor[1] = lightfactor[2] = WorldLightmapBlend.Get()*LightmapMult;
		}else{
		notex:
			setVertexShader(default_amb_VS);

			d3d::setTexture(1, nil);
			lightfactor[0] = lightfactor[1] = lightfactor[2] = 0.0f;
		}
		lightfactor[3] = m->color.alpha/255.0f;
		d3d::setTexture(0, m->texture);
		d3ddevice->SetPixelShaderConstantF(1, lightfactor, 1);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || m->color.alpha != 255);

		RGBA color = { 255, 255, 255, m->color.alpha };
		setMaterial(color, m->surfaceProps);

		if(m->texture)
			d3d::setTexture(0, m->texture);
		else
			d3d::setTexture(0, gpWhiteTexture);
		setPixelShader(neoWorldIII_PS);

		drawInst(header, inst);
		inst++;
	}
	d3d::setTexture(1, nil);
}

void
CreateWorldPipe(void)
{
	if(CFileMgr::LoadFile("neo/worldTweakingTable.dat", work_buff, sizeof(work_buff), "r") <= 0)
		printf("Error: couldn't open 'neo/worldTweakingTable.dat'\n");
	else
		ReadTweakValueTable((char*)work_buff, WorldLightmapBlend);

#include "shaders/obj/default_UV2_VS.inc"
	neoWorld_VS = rw::d3d::createVertexShader(default_UV2_VS_cso);
	assert(neoWorld_VS);

#include "shaders/obj/neoWorldIII_PS.inc"
	neoWorldIII_PS = rw::d3d::createPixelShader(neoWorldIII_PS_cso);
	assert(neoWorldIII_PS);


	rw::d3d9::ObjPipeline *pipe = rw::d3d9::ObjPipeline::create();
	pipe->instanceCB = rw::d3d9::defaultInstanceCB;
	pipe->uninstanceCB = rw::d3d9::defaultUninstanceCB;
	pipe->renderCB = worldRenderCB;
	worldPipe = pipe;
}

void
DestroyWorldPipe(void)
{
	rw::d3d::destroyVertexShader(neoWorld_VS);
	neoWorld_VS = nil;
	rw::d3d::destroyPixelShader(neoWorldIII_PS);
	neoWorldIII_PS = nil;


	((rw::d3d9::ObjPipeline*)worldPipe)->destroy();
	worldPipe = nil;
}




/*
 * Neo Gloss pipe
 */

static void *neoGloss_VS;
static void *neoGloss_PS;

static void
glossRenderCB(rw::Atomic *atomic, rw::d3d9::InstanceDataHeader *header)
{
	worldRenderCB(atomic, header);

	using namespace rw;
	using namespace rw::d3d;
	using namespace rw::d3d9;

	if(!GlossEnable)
		return;

	setVertexShader(neoGloss_VS);
	setPixelShader(neoGloss_PS);

	V3d eyePos = rw::engine->currentCamera->getFrame()->getLTM()->pos;
	d3ddevice->SetVertexShaderConstantF(VSLOC_eye, (float*)&eyePos, 1);
	d3ddevice->SetPixelShaderConstantF(1, (float*)&GlossMult, 1);

	SetRenderState(VERTEXALPHA, TRUE);
	SetRenderState(SRCBLEND, BLENDONE);
	SetRenderState(DESTBLEND, BLENDONE);
	SetRenderState(ZWRITEENABLE, FALSE);
	SetRenderState(ALPHATESTFUNC, ALPHAALWAYS);

	InstanceData *inst = header->inst;
	for(rw::uint32 i = 0; i < header->numMeshes; i++){
		Material *m = inst->material;

		if(m->texture){
			Texture *tex = GetGlossTex(m);
			if(tex){
				d3d::setTexture(0, tex);
				drawInst(header, inst);
			}
		}
		inst++;
	}

	SetRenderState(ZWRITEENABLE, TRUE);
	SetRenderState(ALPHATESTFUNC, ALPHAGREATEREQUAL);
	SetRenderState(SRCBLEND, BLENDSRCALPHA);
	SetRenderState(DESTBLEND, BLENDINVSRCALPHA);
}

void
CreateGlossPipe(void)
{
#include "shaders/obj/neoGloss_VS.inc"
	neoGloss_VS = rw::d3d::createVertexShader(neoGloss_VS_cso);
	assert(neoGloss_VS);

#include "shaders/obj/neoGloss_PS.inc"
	neoGloss_PS = rw::d3d::createPixelShader(neoGloss_PS_cso);
	assert(neoGloss_PS);


	rw::d3d9::ObjPipeline *pipe = rw::d3d9::ObjPipeline::create();
	pipe->instanceCB = rw::d3d9::defaultInstanceCB;
	pipe->uninstanceCB = rw::d3d9::defaultUninstanceCB;
	pipe->renderCB = glossRenderCB;
	glossPipe = pipe;
}

void
DestroyGlossPipe(void)
{
	rw::d3d::destroyVertexShader(neoGloss_VS);
	neoGloss_VS = nil;

	rw::d3d::destroyPixelShader(neoGloss_PS);
	neoGloss_PS = nil;

	((rw::d3d9::ObjPipeline*)glossPipe)->destroy();
	glossPipe = nil;
}



/*
 * Neo Rim pipes
 */

static void *neoRim_VS;
static void *neoRimSkin_VS;

static void
uploadRimData(bool enable)
{
	using namespace rw;
	using namespace rw::d3d;

	V3d viewVec = rw::engine->currentCamera->getFrame()->getLTM()->at;
	d3ddevice->SetVertexShaderConstantF(VSLOC_viewVec, (float*)&viewVec, 1);
	float rimData[4];
	rimData[0] = Offset.Get();
	rimData[1] = Scale.Get();
	if(enable)
		rimData[2] = Scaling.Get()*RimlightMult;
	else
		rimData[2] = 0.0f;
	rimData[3] = 0.0f;
	d3ddevice->SetVertexShaderConstantF(VSLOC_rimData, rimData, 1);
	Color col = RampStart.Get();
	d3ddevice->SetVertexShaderConstantF(VSLOC_rampStart, (float*)&col, 1);
	col = RampEnd.Get();
	d3ddevice->SetVertexShaderConstantF(VSLOC_rampEnd, (float*)&col, 1);
}

static void
rimRenderCB(rw::Atomic *atomic, rw::d3d9::InstanceDataHeader *header)
{
	using namespace rw;
	using namespace rw::d3d;
	using namespace rw::d3d9;

	if(!RimlightEnable){
		defaultRenderCB_Shader(atomic, header);
		return;
	}

	int vsBits;
	rw::uint32 flags = atomic->geometry->flags;
	setStreamSource(0, header->vertexStream[0].vertexBuffer, 0, header->vertexStream[0].stride);
	setIndices(header->indexBuffer);
	setVertexDeclaration(header->vertexDeclaration);

	vsBits = lightingCB_Shader(atomic);
	uploadMatrices(atomic->getFrame()->getLTM());

	setVertexShader(neoRim_VS);

	uploadRimData(atomic->geometry->flags & Geometry::LIGHT);

	InstanceData *inst = header->inst;
	for(rw::uint32 i = 0; i < header->numMeshes; i++){
		Material *m = inst->material;

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || m->color.alpha != 255);

		setMaterial(flags, m->color, m->surfaceProps);

		if(m->texture){
			d3d::setTexture(0, m->texture);
			setPixelShader(default_tex_PS);
		}else
			setPixelShader(default_PS);

		drawInst(header, inst);
		inst++;
	}
}

static void
rimSkinRenderCB(rw::Atomic *atomic, rw::d3d9::InstanceDataHeader *header)
{
	using namespace rw;
	using namespace rw::d3d;
	using namespace rw::d3d9;

	if(!RimlightEnable){
		skinRenderCB(atomic, header);
		return;
	}

	int vsBits;
	rw::uint32 flags = atomic->geometry->flags;
	setStreamSource(0, (IDirect3DVertexBuffer9*)header->vertexStream[0].vertexBuffer,
	                           0, header->vertexStream[0].stride);
	setIndices((IDirect3DIndexBuffer9*)header->indexBuffer);
	setVertexDeclaration((IDirect3DVertexDeclaration9*)header->vertexDeclaration);

	vsBits = lightingCB_Shader(atomic);
	uploadMatrices(atomic->getFrame()->getLTM());

	uploadSkinMatrices(atomic);

	setVertexShader(neoRimSkin_VS);

	uploadRimData(atomic->geometry->flags & Geometry::LIGHT);

	InstanceData *inst = header->inst;
	for(rw::uint32 i = 0; i < header->numMeshes; i++){
		Material *m = inst->material;

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || m->color.alpha != 255);

		setMaterial(flags, m->color, m->surfaceProps);

		if(inst->material->texture){
			d3d::setTexture(0, m->texture);
			setPixelShader(default_tex_PS);
		}else
			setPixelShader(default_PS);

		drawInst(header, inst);
		inst++;
	}
}

void
CreateRimLightPipes(void)
{
	if(CFileMgr::LoadFile("neo/rimTweakingTable.dat", work_buff, sizeof(work_buff), "r") <= 0)
		printf("Error: couldn't open 'neo/rimTweakingTable.dat'\n");
	else{
		char *fp = (char*)work_buff;
		fp = ReadTweakValueTable(fp, RampStart);
		fp = ReadTweakValueTable(fp, RampEnd);
		fp = ReadTweakValueTable(fp, Offset);
		fp = ReadTweakValueTable(fp, Scale);
		fp = ReadTweakValueTable(fp, Scaling);
	}


#include "shaders/obj/neoRim_VS.inc"
	neoRim_VS = rw::d3d::createVertexShader(neoRim_VS_cso);
	assert(neoRim_VS);

#include "shaders/obj/neoRimSkin_VS.inc"
	neoRimSkin_VS = rw::d3d::createVertexShader(neoRimSkin_VS_cso);
	assert(neoRimSkin_VS);


	rw::d3d9::ObjPipeline *pipe = rw::d3d9::ObjPipeline::create();
	pipe->instanceCB = rw::d3d9::defaultInstanceCB;
	pipe->uninstanceCB = rw::d3d9::defaultUninstanceCB;
	pipe->renderCB = rimRenderCB;
	rimPipe = pipe;

	pipe = rw::d3d9::ObjPipeline::create();
	pipe->instanceCB = rw::d3d9::skinInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = rimSkinRenderCB;
	rimSkinPipe = pipe;
}

void
DestroyRimLightPipes(void)
{
	rw::d3d::destroyVertexShader(neoRim_VS);
	neoRim_VS = nil;

	rw::d3d::destroyVertexShader(neoRimSkin_VS);
	neoRimSkin_VS = nil;

	((rw::d3d9::ObjPipeline*)rimPipe)->destroy();
	rimPipe = nil;

	((rw::d3d9::ObjPipeline*)rimSkinPipe)->destroy();
	rimSkinPipe = nil;
}

}

#endif

#ifdef NEW_RENDERER
#ifndef LIBRW
#error "Need librw for NEW_PIPELINES"
#endif

namespace WorldRender
{

struct BuildingInst
{
	rw::RawMatrix combinedMat;
	rw::d3d9::InstanceDataHeader *instHeader;
	uint8 fadeAlpha;
	bool lighting;
};
BuildingInst blendInsts[3][2000];
int numBlendInsts[3];

static RwRGBAReal black;

static void
SetMatrix(BuildingInst *building, rw::Matrix *worldMat)
{
	using namespace rw;
	RawMatrix world, worldview;
	Camera *cam = engine->currentCamera;
	convMatrix(&world, worldMat);
	RawMatrix::mult(&worldview, &world, &cam->devView);
	RawMatrix::mult(&building->combinedMat, &worldview, &cam->devProj);
}

static bool
IsTextureTransparent(RwTexture *tex)
{
	if(tex == nil || tex->raster == nil)
		return false;
	return PLUGINOFFSET(rw::d3d::D3dRaster, tex->raster, rw::d3d::nativeRasterOffset)->hasAlpha;
}

// Render all opaque meshes and put atomics that needs blending
// into the deferred list.
void
AtomicFirstPass(RpAtomic *atomic, int pass)
{
	using namespace rw;
	using namespace rw::d3d;
	using namespace rw::d3d9;

	BuildingInst *building = &blendInsts[pass][numBlendInsts[pass]];

	atomic->getPipeline()->instance(atomic);
	building->instHeader = (d3d9::InstanceDataHeader*)atomic->geometry->instData;
	assert(building->instHeader != nil);
	assert(building->instHeader->platform == PLATFORM_D3D9);
	building->fadeAlpha = 255;
	building->lighting = !!(atomic->geometry->flags & rw::Geometry::LIGHT);
	rw::uint32 flags = atomic->geometry->flags;

	bool setupDone = false;
	bool defer = false;
	SetMatrix(building, atomic->getFrame()->getLTM());

	InstanceData *inst = building->instHeader->inst;
	for(rw::uint32 i = 0; i < building->instHeader->numMeshes; i++, inst++){
		Material *m = inst->material;

		if(inst->vertexAlpha || m->color.alpha != 255 ||
		   IsTextureTransparent(m->texture)){
			defer = true;
			continue;
		}

		// alright we're rendering this atomic
		if(!setupDone){
			setStreamSource(0, building->instHeader->vertexStream[0].vertexBuffer, 0, building->instHeader->vertexStream[0].stride);
			setIndices(building->instHeader->indexBuffer);
			setVertexDeclaration(building->instHeader->vertexDeclaration);
			setVertexShader(default_amb_VS);
			d3ddevice->SetVertexShaderConstantF(VSLOC_combined, (float*)&building->combinedMat, 4);
			if(building->lighting)
				setAmbient(pAmbient->color);
			else
				setAmbient(black);
			setupDone = true;
		}

		setMaterial(flags, m->color, m->surfaceProps);

		if(m->texture){
			d3d::setTexture(0, m->texture);
			setPixelShader(default_tex_PS);
		}else
			setPixelShader(default_PS);

		drawInst(building->instHeader, inst);
	}
	if(defer)
		numBlendInsts[pass]++;
}

void
AtomicFullyTransparent(RpAtomic *atomic, int pass, int fadeAlpha)
{
	using namespace rw;
	using namespace rw::d3d;
	using namespace rw::d3d9;

	BuildingInst *building = &blendInsts[pass][numBlendInsts[pass]];

	atomic->getPipeline()->instance(atomic);
	building->instHeader = (d3d9::InstanceDataHeader*)atomic->geometry->instData;
	assert(building->instHeader != nil);
	assert(building->instHeader->platform == PLATFORM_D3D9);
	building->fadeAlpha = fadeAlpha;
	building->lighting = !!(atomic->geometry->flags & rw::Geometry::LIGHT);
	SetMatrix(building, atomic->getFrame()->getLTM());
	numBlendInsts[pass]++;
}

void
RenderBlendPass(int pass)
{
	using namespace rw;
	using namespace rw::d3d;
	using namespace rw::d3d9;

	setVertexShader(default_amb_VS);

	int i;
	for(i = 0; i < numBlendInsts[pass]; i++){
		BuildingInst *building = &blendInsts[pass][i];

		setStreamSource(0, building->instHeader->vertexStream[0].vertexBuffer, 0, building->instHeader->vertexStream[0].stride);
		setIndices(building->instHeader->indexBuffer);
		setVertexDeclaration(building->instHeader->vertexDeclaration);
		d3ddevice->SetVertexShaderConstantF(VSLOC_combined, (float*)&building->combinedMat, 4);
		if(building->lighting)
			setAmbient(pAmbient->color);
		else
			setAmbient(black);

		InstanceData *inst = building->instHeader->inst;
		for(rw::uint32 j = 0; j < building->instHeader->numMeshes; j++, inst++){
			Material *m = inst->material;
			if(!inst->vertexAlpha && m->color.alpha == 255 && !IsTextureTransparent(m->texture) && building->fadeAlpha == 255)
				continue;	// already done this one

			rw::RGBA color = m->color;
			color.alpha = (color.alpha * building->fadeAlpha)/255;
			setMaterial(color, m->surfaceProps);	// always modulate here

			if(m->texture){
				d3d::setTexture(0, m->texture);
				setPixelShader(default_tex_PS);
			}else
				setPixelShader(default_PS);

			drawInst(building->instHeader, inst);
		}
	}
}
}
#endif

#endif
