#define WITHD3D
#include "common.h"
#include <rpskin.h>

#include "RwHelper.h"
#include "Timecycle.h"
#include "skeleton.h"
#include "Debug.h"
#if !defined(FINAL) || defined(DEBUGMENU)
#include "rtcharse.h"
#endif
#ifndef FINAL
RtCharset *debugCharset;
#endif

#ifdef PS2_ALPHA_TEST
bool gPS2alphaTest = true;
#else
bool gPS2alphaTest = false;
#endif
bool gBackfaceCulling = true;

#if !defined(FINAL) || defined(DEBUGMENU)
static bool charsetOpen;
void OpenCharsetSafe()
{
	if(!charsetOpen)
		RtCharsetOpen();
	charsetOpen = true;
}
#endif

void CreateDebugFont()
{
#ifndef FINAL
	RwRGBA color = { 255, 255, 128, 255 };
	RwRGBA colorbg = { 0, 0, 0, 0 };
	OpenCharsetSafe();
	debugCharset = RtCharsetCreate(&color, &colorbg);
#endif
}

void DestroyDebugFont()
{
#ifndef FINAL
	RtCharsetDestroy(debugCharset);
	RtCharsetClose();
	charsetOpen = false;
#endif
}

void ObrsPrintfString(const char *str, short x, short y)
{
#ifndef FINAL
	RtCharsetPrintBuffered(debugCharset, str, x*8, y*16, true);
#endif
}

void FlushObrsPrintfs()
{
#ifndef FINAL
	RtCharsetBufferFlush();
#endif
}

void
DefinedState(void)
{
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSWRAP);
	RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEGOURAUD);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEALPHAPRIMITIVEBUFFER, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEBORDERCOLOR, (void*)RWRGBALONG(0, 0, 0, 255));
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEFOGCOLOR,
		(void*)RWRGBALONG(CTimeCycle::GetFogRed(), CTimeCycle::GetFogGreen(), CTimeCycle::GetFogBlue(), 255));
	RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void*)rwFOGTYPELINEAR);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);

#ifdef LIBRW
	rw::SetRenderState(rw::ALPHATESTFUNC, rw::ALPHAGREATEREQUAL);

	rw::SetRenderState(rw::GSALPHATEST, gPS2alphaTest);
#else
	// D3D stuff
	RwD3D8SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
#endif
	SetAlphaRef(2);
}

void
SetAlphaRef(int ref)
{
#ifdef LIBRW
	rw::SetRenderState(rw::ALPHATESTREF, ref+1);
#else
	RwD3D8SetRenderState(D3DRS_ALPHAREF, ref);
#endif
}

void
SetCullMode(uint32 mode)
{
	if(gBackfaceCulling)
		RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)mode);
	else
		RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
}

RwFrame*
GetFirstFrameCallback(RwFrame *child, void *data)
{
	*(RwFrame**)data = child;
	return nil;
}

RwFrame*
GetFirstChild(RwFrame *frame)
{
	RwFrame *child;

	child = nil;
	RwFrameForAllChildren(frame, GetFirstFrameCallback, &child);
	return child;
}

RwObject*
GetFirstObjectCallback(RwObject *object, void *data)
{
	*(RwObject**)data = object;
	return nil;
}

RwObject*
GetFirstObject(RwFrame *frame)
{
	RwObject *obj;

	obj = nil;
	RwFrameForAllObjects(frame, GetFirstObjectCallback, &obj);
	return obj;
}

RpAtomic*
GetFirstAtomicCallback(RpAtomic *atm, void *data)
{
	*(RpAtomic**)data = atm;
	return nil;
}

RpAtomic*
GetFirstAtomic(RpClump *clump)
{
	RpAtomic *atm;

	atm = nil;
	RpClumpForAllAtomics(clump, GetFirstAtomicCallback, &atm);
	return atm;
}

RwTexture*
GetFirstTextureCallback(RwTexture *tex, void *data)
{
	*(RwTexture**)data = tex;
	return nil;
}

RwTexture*
GetFirstTexture(RwTexDictionary *txd)
{
	RwTexture *tex;

	tex = nil;
	RwTexDictionaryForAllTextures(txd, GetFirstTextureCallback, &tex);
	return tex;
}

#ifdef PED_SKIN
static RpAtomic*
isSkinnedCb(RpAtomic *atomic, void *data)
{
	RpAtomic **pAtomic = (RpAtomic**)data;
	if(*pAtomic)
		return nil;	// already found one
	if(RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic)))
		*pAtomic = atomic;	// we could just return nil here directly...
	return atomic;
}

RpAtomic*
IsClumpSkinned(RpClump *clump)
{
	RpAtomic *atomic = nil;
	RpClumpForAllAtomics(clump, isSkinnedCb, &atomic);
	return atomic;
}

static RpAtomic*
GetAnimHierarchyCallback(RpAtomic *atomic, void *data)
{
	*(RpHAnimHierarchy**)data = RpSkinAtomicGetHAnimHierarchy(atomic);
	return nil;
}

RpHAnimHierarchy*
GetAnimHierarchyFromSkinClump(RpClump *clump)
{
	RpHAnimHierarchy *hier = nil;
	RpClumpForAllAtomics(clump, GetAnimHierarchyCallback, &hier);
	return hier;
}

static RwFrame*
GetAnimHierarchyFromClumpCB(RwFrame *frame, void *data)
{
	RpHAnimHierarchy *hier = RpHAnimFrameGetHierarchy(frame);
	if(hier){
		*(RpHAnimHierarchy**)data = hier;
		return nil;
	}
	RwFrameForAllChildren(frame, GetAnimHierarchyFromClumpCB, data);
	return frame;
}

RpHAnimHierarchy*
GetAnimHierarchyFromClump(RpClump *clump)
{
	RpHAnimHierarchy *hier = nil;
	RwFrameForAllChildren(RpClumpGetFrame(clump), GetAnimHierarchyFromClumpCB, &hier);
	return hier;
}

RwFrame*
GetHierarchyFromChildNodesCB(RwFrame *frame, void *data)
{
	RpHAnimHierarchy **pHier = (RpHAnimHierarchy**)data;
	RpHAnimHierarchy *hier = RpHAnimFrameGetHierarchy(frame);
	if(hier == nil)
		RwFrameForAllChildren(frame, GetHierarchyFromChildNodesCB, &hier);
	*pHier = hier;
	return nil;
}

void
SkinGetBonePositionsToTable(RpClump *clump, RwV3d *boneTable)
{
	int i, parent;
	RpAtomic *atomic;
	RpSkin *skin;
	RpHAnimHierarchy *hier;
	int numBones;
	RwMatrix m, invmat;
	int stack[32];
	int sp;

	if(boneTable == nil)
		return;

//	atomic = GetFirstAtomic(clump);		// mobile, also VC
	atomic = IsClumpSkinned(clump);		// xbox, seems safer
	assert(atomic);
	skin = RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic));
	assert(skin);
	hier = GetAnimHierarchyFromSkinClump(clump);
	assert(hier);
	boneTable[0].x = 0.0f;
	boneTable[0].y = 0.0f;
	boneTable[0].z = 0.0f;
	numBones = RpSkinGetNumBones(skin);
	parent = 0;
	sp = 0;
#ifdef FIX_BUGS
	stack[0] = 0;	// i think this is ok
#endif
	for(i = 1; i < numBones; i++){
		RwMatrixCopy(&m, &RpSkinGetSkinToBoneMatrices(skin)[i]);
		RwMatrixInvert(&invmat, &m);
		const RwMatrix *x = RpSkinGetSkinToBoneMatrices(skin);
		RwV3dTransformPoints(&boneTable[i], &invmat.pos, 1, &x[parent]);
		if(HIERNODEINFO(hier)[i].flags & rpHANIMPUSHPARENTMATRIX)
			stack[++sp] = parent;
		if(HIERNODEINFO(hier)[i].flags & rpHANIMPOPPARENTMATRIX)
			parent = stack[sp--];
		else
			parent = i;

		//assert(parent >= 0 && parent < numBones);
	}
}

RpHAnimAnimation*
HAnimAnimationCreateForHierarchy(RpHAnimHierarchy *hier)
{
	int i;
#if defined FIX_BUGS || defined LIBRW
	int numNodes = hier->numNodes*2;	// you're supposed to have at least two KFs per node
#else
	int numNodes = hier->numNodes;
#endif
	RpHAnimAnimation *anim = RpHAnimAnimationCreate(rpHANIMSTDKEYFRAMETYPEID, numNodes, 0, 0.0f);
	if(anim == nil)
		return nil;
	RpHAnimStdKeyFrame *frame;
	for(i = 0; i < numNodes; i++){
		frame = (RpHAnimStdKeyFrame*)HANIMFRAME(anim, i);	// games uses struct size here, not safe
		frame->q.real = 1.0f;
		frame->q.imag.x = frame->q.imag.y = frame->q.imag.z = 0.0f;
		frame->t.x = frame->t.y = frame->t.z = 0.0f;
#if defined FIX_BUGS || defined LIBRW
		// times are subtracted and divided giving NaNs
		// so they can't both be 0
		frame->time = i/hier->numNodes;
#else
		frame->time = 0.0f;
#endif
		frame->prevFrame = nil;
	}
	return anim;
}

void
RenderSkeleton(RpHAnimHierarchy *hier)
{
	int i;
	int sp;
	int stack[32];
	int par;
	CVector p1, p2;
	int numNodes = hier->numNodes;
	RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
	p1 = mats[0].pos;

	par = 0;
	sp = 0;
	stack[sp++] = par;
	for(i = 1; i < numNodes; i++){
		p1 = mats[par].pos;
		p2 = mats[i].pos;
		CDebug::AddLine(p1, p2, 0xFFFFFFFF, 0xFFFFFFFF);
		if(HIERNODEINFO(hier)[i].flags & rpHANIMPUSHPARENTMATRIX)
			stack[sp++] = par;
		par = i;
		if(HIERNODEINFO(hier)[i].flags & rpHANIMPOPPARENTMATRIX)
			par = stack[--sp];
	}
}
#endif

void
CameraSize(RwCamera * camera, RwRect * rect,
		   RwReal viewWindow, RwReal aspectRatio)
{
	if (camera)
	{
		RwVideoMode         videoMode;
		RwRect              r;
		RwRect              origSize = { 0, 0, 0, 0 };	// FIX just to make the compier happy
		RwV2d               vw;

		RwEngineGetVideoModeInfo(&videoMode,
								 RwEngineGetCurrentVideoMode());

		origSize.w  = RwRasterGetWidth(RwCameraGetRaster(camera));
		origSize.h = RwRasterGetHeight(RwCameraGetRaster(camera));

		if (!rect)
		{
			if (videoMode.flags & rwVIDEOMODEEXCLUSIVE)
			{
				/* For full screen applications, resizing the camera just doesn't
				 * make sense, use the video mode size.
				 */

				r.x = r.y = 0;
				r.w = videoMode.width;
				r.h = videoMode.height;
				rect = &r;
			}
			else
			{
				/*
				rect not specified - reuse current values
				*/
				r.w = RwRasterGetWidth(RwCameraGetRaster(camera));
				r.h = RwRasterGetHeight(RwCameraGetRaster(camera));
				r.x = r.y = 0;
				rect = &r;
			}
		}

		if (( origSize.w != rect->w ) || ( origSize.h != rect->h ))
		{
			RwRaster           *raster;
			RwRaster           *zRaster;

			// BUG: game just changes camera raster's sizes, but this is a hack
#if defined FIX_BUGS || defined LIBRW
			/*
			 * Destroy rasters...
			 */

			raster = RwCameraGetRaster(camera);
			if( raster )
			{
				RwRasterDestroy(raster);
				camera->frameBuffer = nil;
			}

			zRaster = RwCameraGetZRaster(camera);
			if( zRaster )
			{
				RwRasterDestroy(zRaster);
				camera->zBuffer = nil;
			}

			/*
			 * Create new rasters...
			 */

			raster = RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPECAMERA);
			zRaster = RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPEZBUFFER);

			if( raster && zRaster )
			{
				RwCameraSetRaster(camera, raster);
				RwCameraSetZRaster(camera, zRaster);
			}
			else
			{
				if( raster )
				{
					RwRasterDestroy(raster);
				}

				if( zRaster )
				{
					RwRasterDestroy(zRaster);
				}

				rect->x = origSize.x;
				rect->y = origSize.y;
				rect->w = origSize.w;
				rect->h = origSize.h;

				/*
				 * Use default values...
				 */
				raster =
					RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPECAMERA);

				zRaster =
					RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPEZBUFFER);

				RwCameraSetRaster(camera, raster);
				RwCameraSetZRaster(camera, zRaster);
			}
#else
			raster = RwCameraGetRaster(camera);
			zRaster = RwCameraGetZRaster(camera);

			raster->width = zRaster->width = rect->w;
			raster->height = zRaster->height = rect->h;
#endif
		}

		/* Figure out the view window */
		if (videoMode.flags & rwVIDEOMODEEXCLUSIVE)
		{
			/* derive ratio from aspect ratio */
			vw.x = viewWindow;
			vw.y = viewWindow / aspectRatio;
		}
		else
		{
			/* derive from pixel ratios */
			if (rect->w > rect->h)
			{
				vw.x = viewWindow;
				vw.y = (rect->h * viewWindow) / rect->w;
			}
			else
			{
				vw.x = (rect->w * viewWindow) / rect->h;
				vw.y = viewWindow;
			}
		}

		RwCameraSetViewWindow(camera, &vw);

		RsGlobal.width  = rect->w;
		RsGlobal.height = rect->h;
	}

	return;
}

void
CameraDestroy(RwCamera *camera)
{
	RwRaster    *raster, *tmpRaster;
	RwFrame     *frame;

	if (camera)
	{
		frame = RwCameraGetFrame(camera);
		if (frame)
		{
			RwFrameDestroy(frame);
		}

		raster = RwCameraGetRaster(camera);
		if (raster)
		{
			tmpRaster = RwRasterGetParent(raster);

			RwRasterDestroy(raster);

			if ((tmpRaster != nil) && (tmpRaster != raster))
			{
				RwRasterDestroy(tmpRaster);
			}
		}

		raster = RwCameraGetZRaster(camera);
		if (raster)
		{
			tmpRaster = RwRasterGetParent(raster);

			RwRasterDestroy(raster);

			if ((tmpRaster != nil) && (tmpRaster != raster))
			{
				RwRasterDestroy(tmpRaster);
			}
		}

		RwCameraDestroy(camera);
	}

	return;
}

RwCamera           *
CameraCreate(RwInt32 width, RwInt32 height, RwBool zBuffer)
{
	RwCamera           *camera;

	camera = RwCameraCreate();

	if (camera)
	{
		RwCameraSetFrame(camera, RwFrameCreate());
		RwCameraSetRaster(camera,
						  RwRasterCreate(0, 0, 0, rwRASTERTYPECAMERA));

		if (zBuffer)
		{
			RwCameraSetZRaster(camera,
							   RwRasterCreate(0, 0, 0,
											  rwRASTERTYPEZBUFFER));
		}

		/* now check that everything is valid */
		if (RwCameraGetFrame(camera) &&
			RwCameraGetRaster(camera) &&
			RwRasterGetParent(RwCameraGetRaster(camera)) &&
			(!zBuffer || (RwCameraGetZRaster(camera) &&
						  RwRasterGetParent(RwCameraGetZRaster
											(camera)))))
		{
			/* everything OK */
			return (camera);
		}
	}

	/* if we're here then an error must have occurred so clean up */

	CameraDestroy(camera);
	return (nil);
}

#ifdef LIBRW
#include <rpmatfx.h>
#include "VehicleModelInfo.h"

int32
findPlatform(rw::Atomic *a)
{
	rw::Geometry *g = a->geometry;
	if(g->instData)
		return g->instData->platform;
	return 0;
}

// in CVehicleModelInfo in VC
static RpMaterial*
GetMatFXEffectMaterialCB(RpMaterial *material, void *data)
{
	if(RpMatFXMaterialGetEffects(material) == rpMATFXEFFECTNULL)
		return material;
	*(int*)data = RpMatFXMaterialGetEffects(material);
	return nil;
}

// Game doesn't read atomic extensions so we never get any other than the default pipe,
// but we need it for uninstancing
void
attachPipe(rw::Atomic *atomic)
{
	if(RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic)))
		atomic->pipeline = rw::skinGlobals.pipelines[rw::platform];
	else{
		int fx = rpMATFXEFFECTNULL;
		RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic), GetMatFXEffectMaterialCB, &fx);
		if(fx != rpMATFXEFFECTNULL)
			RpMatFXAtomicEnableEffects(atomic);
	}
}

// Attach pipes for the platform we have native data for so we can uninstance
void
switchPipes(rw::Atomic *a, int32 platform)
{
	if(a->pipeline && a->pipeline->platform != platform){
		uint32 plgid = a->pipeline->pluginID;
		switch(plgid){
		// assume default pipe won't be attached explicitly
		case rw::ID_SKIN:
			a->pipeline = rw::skinGlobals.pipelines[platform];
			break;
		case rw::ID_MATFX:
			a->pipeline = rw::matFXGlobals.pipelines[platform];
			break;
		}
	}
}

RpAtomic*
ConvertPlatformAtomic(RpAtomic *atomic, void *data)
{
	int32 driver = rw::platform;
	int32 platform = findPlatform(atomic);
	if(platform != 0 && platform != driver){
		attachPipe(atomic);	// kludge
		rw::ObjPipeline *origPipe = atomic->pipeline;
		rw::platform = platform;
		switchPipes(atomic, rw::platform);
		if(atomic->geometry->flags & rw::Geometry::NATIVE)
			atomic->uninstance();
		// no ADC in this game
		//rw::ps2::unconvertADC(atomic->geometry);
		rw::platform = driver;
		atomic->pipeline = origPipe;
	}
	return atomic;
}
#endif

#if defined(FIX_BUGS) && defined(GTA_PC)
RwUInt32 saved_alphafunc, saved_alpharef;

void
SetAlphaTest(RwUInt32 alpharef)
{
#ifdef LIBRW
	saved_alphafunc = rw::GetRenderState(rw::ALPHATESTFUNC);
	saved_alpharef = rw::GetRenderState(rw::ALPHATESTREF);

	rw::SetRenderState(rw::ALPHATESTFUNC, rw::ALPHAGREATEREQUAL);
	rw::SetRenderState(rw::ALPHATESTREF, 0);
#else
	RwD3D8GetRenderState(D3DRS_ALPHAFUNC, &saved_alphafunc);
	RwD3D8GetRenderState(D3DRS_ALPHAREF, &saved_alpharef);
	
	RwD3D8SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	RwD3D8SetRenderState(D3DRS_ALPHAREF, alpharef);
#endif
}

void
RestoreAlphaTest()
{
#ifdef LIBRW
	rw::SetRenderState(rw::ALPHATESTFUNC, saved_alphafunc);
	rw::SetRenderState(rw::ALPHATESTREF, saved_alpharef);
#else
	RwD3D8SetRenderState(D3DRS_ALPHAFUNC, saved_alphafunc);
	RwD3D8SetRenderState(D3DRS_ALPHAREF, saved_alpharef);
#endif
}
#endif
