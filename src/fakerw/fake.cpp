#define _CRT_SECURE_NO_WARNINGS
#define WITH_D3D // not WITHD3D, so it's librw define
#include <rwcore.h>
#include <rpworld.h>
#include <rpmatfx.h>
#include <rphanim.h>
#include <rpskin.h>
#include <assert.h>
#include <string.h>
#ifndef _WIN32
#include "crossplatform.h"
#endif

using namespace rw;

RwUInt8 RwObjectGetType(const RwObject *obj) { return obj->type; }
RwFrame* rwObjectGetParent(const RwObject *obj) { return (RwFrame*)obj->parent; }

void *RwMalloc(size_t size) { return engine->memfuncs.rwmalloc(size, 0); }
void *RwCalloc(size_t numObj, size_t sizeObj) {
	void *mem = RwMalloc(numObj*sizeObj);
	if(mem)
		memset(mem, 0, numObj*sizeObj);
	return mem;
}
void  RwFree(void *mem) { engine->memfuncs.rwfree(mem); }


//RwReal RwV3dNormalize(RwV3d * out, const RwV3d * in);
RwReal RwV3dLength(const RwV3d * in) { return length(*in); }
//RwReal RwV2dLength(const RwV2d * in);
//RwReal RwV2dNormalize(RwV2d * out, const RwV2d * in);
//void RwV2dAssign(RwV2d * out, const RwV2d * ina);
//void RwV2dAdd(RwV2d * out, const RwV2d * ina, const RwV2d * inb);
//void RwV2dLineNormal(RwV2d * out, const RwV2d * ina, const RwV2d * inb);
//void RwV2dSub(RwV2d * out, const RwV2d * ina, const RwV2d * inb);
//void RwV2dPerp(RwV2d * out, const RwV2d * in);
//void RwV2dScale(RwV2d * out, const RwV2d * in, RwReal scalar);
//RwReal RwV2dDotProduct(const RwV2d * ina, const RwV2d * inb);
//void RwV3dAssign(RwV3d * out, const RwV3d * ina);
void RwV3dAdd(RwV3d * out, const RwV3d * ina, const RwV3d * inb) { *out = add(*ina, *inb); }
void RwV3dSub(RwV3d * out, const RwV3d * ina, const RwV3d * inb) { *out = sub(*ina, *inb); }
void RwV3dScale(RwV3d * out, const RwV3d * in, RwReal scalar) { *out = scale(*in, scalar); }
void RwV3dIncrementScaled(RwV3d * out,  const RwV3d * in, RwReal scalar) { *out = add(*out, scale(*in, scalar)); }
void RwV3dNegate(RwV3d * out, const RwV3d * in) { *out = neg(*in); }
RwReal RwV3dDotProduct(const RwV3d * ina, const RwV3d * inb) { return dot(*ina, *inb); }
//void RwV3dCrossProduct(RwV3d * out, const RwV3d * ina, const RwV3d * inb);
RwV3d *RwV3dTransformPoints(RwV3d * pointsOut, const RwV3d * pointsIn, RwInt32 numPoints, const RwMatrix * matrix)
	{ V3d::transformPoints(pointsOut, pointsIn, numPoints, matrix); return pointsOut; }
//RwV3d *RwV3dTransformVectors(RwV3d * vectorsOut, const RwV3d * vectorsIn, RwInt32 numPoints, const RwMatrix * matrix);



RwBool RwMatrixDestroy(RwMatrix *mpMat) { mpMat->destroy(); return true; }
RwMatrix *RwMatrixCreate(void) { return Matrix::create(); }
void RwMatrixCopy(RwMatrix * dstMatrix, const RwMatrix * srcMatrix) { *dstMatrix = *srcMatrix; }
void RwMatrixSetIdentity(RwMatrix * matrix) { matrix->setIdentity(); }
RwMatrix *RwMatrixMultiply(RwMatrix * matrixOut, const RwMatrix * MatrixIn1, const RwMatrix * matrixIn2);
RwMatrix *RwMatrixTransform(RwMatrix * matrix, const RwMatrix * transform, RwOpCombineType combineOp)
	{ matrix->transform(transform, (rw::CombineOp)combineOp); return matrix; }
//RwMatrix *RwMatrixOrthoNormalize(RwMatrix * matrixOut, const RwMatrix * matrixIn);
RwMatrix *RwMatrixInvert(RwMatrix * matrixOut, const RwMatrix * matrixIn) { Matrix::invert(matrixOut, matrixIn); return matrixOut; }
RwMatrix *RwMatrixScale(RwMatrix * matrix, const RwV3d * scale, RwOpCombineType combineOp)
	{ matrix->scale(scale, (rw::CombineOp)combineOp); return matrix; }
RwMatrix *RwMatrixTranslate(RwMatrix * matrix, const RwV3d * translation, RwOpCombineType combineOp)
	{ matrix->translate(translation, (rw::CombineOp)combineOp); return matrix; }
RwMatrix *RwMatrixRotate(RwMatrix * matrix, const RwV3d * axis, RwReal angle, RwOpCombineType combineOp)
	{ matrix->rotate(axis, angle, (rw::CombineOp)combineOp); return matrix; }
//RwMatrix *RwMatrixRotateOneMinusCosineSine(RwMatrix * matrix, const RwV3d * unitAxis, RwReal oneMinusCosine, RwReal sine, RwOpCombineType combineOp);
//const RwMatrix *RwMatrixQueryRotate(const RwMatrix * matrix, RwV3d * unitAxis, RwReal * angle, RwV3d * center);
RwV3d *RwMatrixGetRight(RwMatrix * matrix) { return &matrix->right; }
RwV3d *RwMatrixGetUp(RwMatrix * matrix) { return &matrix->up; }
RwV3d *RwMatrixGetAt(RwMatrix * matrix) { return &matrix->at; }
RwV3d *RwMatrixGetPos(RwMatrix * matrix) { return &matrix->pos; }
RwMatrix *RwMatrixUpdate(RwMatrix * matrix) { matrix->update(); return matrix; }
//RwMatrix *RwMatrixOptimize(RwMatrix * matrix, const RwMatrixTolerance *tolerance);




RwFrame *RwFrameForAllObjects(RwFrame * frame, RwObjectCallBack callBack, void *data) {
	FORLIST(lnk, frame->objectList)
		if(callBack(&ObjectWithFrame::fromFrame(lnk)->object, data) == nil)
			break;
	return frame;
}
RwFrame *RwFrameTranslate(RwFrame * frame, const RwV3d * v, RwOpCombineType combine) { frame->translate(v, (CombineOp)combine); return frame; }
RwFrame *RwFrameRotate(RwFrame * frame, const RwV3d * axis, RwReal angle, RwOpCombineType combine) { frame->rotate(axis, angle, (CombineOp)combine); return frame; }
RwFrame *RwFrameScale(RwFrame * frame, const RwV3d * v, RwOpCombineType combine) { frame->scale(v, (CombineOp)combine); return frame; }
RwFrame *RwFrameTransform(RwFrame * frame, const RwMatrix * m, RwOpCombineType combine) { frame->transform(m, (CombineOp)combine); return frame; }
//TODO: actually implement this!
RwFrame *RwFrameOrthoNormalize(RwFrame * frame) { return frame; }
RwFrame *RwFrameSetIdentity(RwFrame * frame) { frame->matrix.setIdentity(); frame->updateObjects(); return frame; }
//RwFrame *RwFrameCloneHierarchy(RwFrame * root);
//RwBool RwFrameDestroyHierarchy(RwFrame * frame);
RwFrame *RwFrameForAllChildren(RwFrame * frame, RwFrameCallBack callBack, void *data)
	{ return frame->forAllChildren(callBack, data); }
RwFrame *RwFrameRemoveChild(RwFrame * child) { child->removeChild(); return child; }
RwFrame *RwFrameAddChild(RwFrame * parent, RwFrame * child) { parent->addChild(child); return parent; }
RwFrame *RwFrameGetParent(const RwFrame * frame) { return frame->getParent(); }
//RwFrame *RwFrameGetRoot(const RwFrame * frame);
RwMatrix *RwFrameGetLTM(RwFrame * frame) { return frame->getLTM(); }
RwMatrix *RwFrameGetMatrix(RwFrame * frame) { return &frame->matrix; }
RwFrame *RwFrameUpdateObjects(RwFrame * frame) { frame->updateObjects(); return frame; }
RwFrame *RwFrameCreate(void) { return rw::Frame::create(); }
//RwBool RwFrameInit(RwFrame *frame);
//RwBool RwFrameDeInit(RwFrame *frame);
RwBool RwFrameDestroy(RwFrame * frame) { frame->destroy(); return true; }
//void _rwFrameInit(RwFrame *frame);
//void _rwFrameDeInit(RwFrame *frame);
//RwBool RwFrameDirty(const RwFrame * frame);
//RwInt32 RwFrameCount(RwFrame * frame);
//RwBool RwFrameSetStaticPluginsSize(RwInt32 size);
RwInt32 RwFrameRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB)
	{ return Frame::registerPlugin(size, pluginID, constructCB, destructCB, (CopyConstructor)copyCB); }
//RwInt32 RwFrameGetPluginOffset(RwUInt32 pluginID);
//RwBool RwFrameValidatePlugins(const RwFrame * frame);
//RwFrame *_rwFrameCloneAndLinkClones(RwFrame * root);
//RwFrame *_rwFramePurgeClone(RwFrame *root);

RwInt32 RwFrameRegisterPluginStream(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB, RwPluginDataChunkWriteCallBack writeCB, RwPluginDataChunkGetSizeCallBack getSizeCB)
	{ return Frame::registerPluginStream(pluginID, readCB, (StreamWrite)writeCB, (StreamGetSize)getSizeCB); }


rwFrameList *rwFrameListDeinitialize(rwFrameList *frameList) {
	rwFree(frameList->frames);
	frameList->frames = nil;
	return frameList;
}
rwFrameList *rwFrameListStreamRead(RwStream *stream, rwFrameList *fl) { return fl->streamRead(stream); }




RwCamera    *RwCameraBeginUpdate(RwCamera * camera) { camera->beginUpdate(); return camera; }
RwCamera    *RwCameraEndUpdate(RwCamera * camera) { camera->endUpdate(); return camera; }
RwCamera    *RwCameraClear(RwCamera * camera, RwRGBA * colour, RwInt32 clearMode) { camera->clear(colour, clearMode); return camera; }
// WARNING: ignored argument
RwCamera    *RwCameraShowRaster(RwCamera * camera, void *pDev, RwUInt32 flags) { camera->showRaster(flags); return camera; }
RwBool       RwCameraDestroy(RwCamera * camera) { camera->destroy(); return true; }
RwCamera    *RwCameraCreate(void) { return rw::Camera::create(); }
RwCamera    *RwCameraClone(RwCamera * camera) { return camera->clone(); }
RwCamera    *RwCameraSetViewOffset(RwCamera *camera, const RwV2d *offset) { camera->setViewOffset(offset); return camera; }
RwCamera    *RwCameraSetViewWindow(RwCamera *camera, const RwV2d *viewWindow) { camera->setViewWindow(viewWindow); return camera; }
RwCamera    *RwCameraSetProjection(RwCamera *camera, RwCameraProjection projection) { camera->projection = projection; return camera; }
RwCamera    *RwCameraSetNearClipPlane(RwCamera *camera, RwReal nearClip) { camera->setNearPlane(nearClip); return camera; }
RwCamera    *RwCameraSetFarClipPlane(RwCamera *camera, RwReal farClip) { camera->setFarPlane(farClip); return camera; }
RwInt32      RwCameraRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwInt32      RwCameraGetPluginOffset(RwUInt32 pluginID);
RwBool       RwCameraValidatePlugins(const RwCamera * camera);
RwFrustumTestResult RwCameraFrustumTestSphere(const RwCamera * camera, const RwSphere * sphere) { return (RwFrustumTestResult)camera->frustumTestSphere(sphere); }
const RwV2d *RwCameraGetViewOffset(const RwCamera *camera) { return &camera->viewOffset; }
RwCamera    *RwCameraSetRaster(RwCamera *camera, RwRaster *raster) { camera->frameBuffer = raster; return camera; }
RwRaster    *RwCameraGetRaster(const RwCamera *camera) { return camera->frameBuffer; }
RwCamera    *RwCameraSetZRaster(RwCamera *camera, RwRaster *zRaster) { camera->zBuffer = zRaster; return camera; }
RwRaster    *RwCameraGetZRaster(const RwCamera *camera) { return camera->zBuffer; }
RwReal       RwCameraGetNearClipPlane(const RwCamera *camera) { return camera->nearPlane; }
RwReal       RwCameraGetFarClipPlane(const RwCamera *camera) { return camera->farPlane; }
RwCamera    *RwCameraSetFogDistance(RwCamera *camera, RwReal fogDistance) { camera->fogPlane = fogDistance; return camera; }
RwReal       RwCameraGetFogDistance(const RwCamera *camera) { return camera->fogPlane; }
RwCamera    *RwCameraGetCurrentCamera(void) { return rw::engine->currentCamera; }
RwCameraProjection RwCameraGetProjection(const RwCamera *camera);
const RwV2d *RwCameraGetViewWindow(const RwCamera *camera) { return &camera->viewWindow; }
RwMatrix    *RwCameraGetViewMatrix(RwCamera *camera) { return &camera->viewMatrix; }
RwCamera    *RwCameraSetFrame(RwCamera *camera, RwFrame *frame) { camera->setFrame(frame); return camera; }
RwFrame     *RwCameraGetFrame(const RwCamera *camera) { return camera->getFrame(); }





RwImage     *RwImageCreate(RwInt32 width, RwInt32 height, RwInt32 depth) { return Image::create(width, height, depth); }
RwBool       RwImageDestroy(RwImage * image) { image->destroy(); return true; }
RwImage     *RwImageAllocatePixels(RwImage * image) { image->allocate(); return image; }
RwImage     *RwImageFreePixels(RwImage * image) { image->free(); return image; }
RwImage     *RwImageCopy(RwImage * destImage, const RwImage * sourceImage);
RwImage     *RwImageResize(RwImage * image, RwInt32 width, RwInt32 height);
RwImage     *RwImageApplyMask(RwImage * image, const RwImage * mask);
RwImage     *RwImageMakeMask(RwImage * image);
RwImage     *RwImageReadMaskedImage(const RwChar * imageName, const RwChar * maskname);
RwImage     *RwImageRead(const RwChar * imageName);
RwImage     *RwImageWrite(RwImage * image, const RwChar * imageName);
RwChar      *RwImageGetPath(void);
const RwChar *RwImageSetPath(const RwChar * path) { Image::setSearchPath(path); return path; }
RwImage     *RwImageSetStride(RwImage * image, RwInt32 stride) { image->stride = stride; return image; }
RwImage     *RwImageSetPixels(RwImage * image, RwUInt8 * pixels) { image->pixels = pixels; return image; }
RwImage     *RwImageSetPalette(RwImage * image, RwRGBA * palette) { image->palette = (uint8*)palette; return image; }
RwInt32      RwImageGetWidth(const RwImage * image) { return image->width; }
RwInt32      RwImageGetHeight(const RwImage * image) { return image->height; }
RwInt32      RwImageGetDepth(const RwImage * image) { return image->depth; }
RwInt32      RwImageGetStride(const RwImage * image) { return image->stride; }
RwUInt8     *RwImageGetPixels(const RwImage * image) { return image->pixels; }
RwRGBA      *RwImageGetPalette(const RwImage * image) { return (RwRGBA*)image->palette; }
RwUInt32     RwRGBAToPixel(RwRGBA * rgbIn, RwInt32 rasterFormat);
RwRGBA      *RwRGBASetFromPixel(RwRGBA * rgbOut, RwUInt32 pixelValue, RwInt32 rasterFormat);
RwBool       RwImageSetGamma(RwReal gammaValue);
RwReal       RwImageGetGamma(void);
RwImage     *RwImageGammaCorrect(RwImage * image);
RwRGBA      *RwRGBAGammaCorrect(RwRGBA * rgb);
RwInt32      RwImageRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwInt32      RwImageGetPluginOffset(RwUInt32 pluginID);
RwBool       RwImageValidatePlugins(const RwImage * image);
//RwBool       RwImageRegisterImageFormat(const RwChar * extension, RwImageCallBackRead imageRead, RwImageCallBackWrite imageWrite);
const RwChar *RwImageFindFileType(const RwChar * imageName);
RwInt32      RwImageStreamGetSize(const RwImage * image);
RwImage     *RwImageStreamRead(RwStream * stream);
const RwImage *RwImageStreamWrite(const RwImage * image, RwStream * stream);

RwImage *RwImageFindRasterFormat(RwImage *ipImage,RwInt32 nRasterType, RwInt32 *npWidth,RwInt32 *npHeight, RwInt32 *npDepth,RwInt32 *npFormat)
{
	return Raster::imageFindRasterFormat(ipImage, nRasterType, npWidth, npHeight, npDepth, npFormat) ? ipImage : nil;
}




RwRaster    *RwRasterCreate(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags) { return Raster::create(width, height, depth, flags); }
RwBool       RwRasterDestroy(RwRaster * raster) { raster->destroy(); return true; }
RwInt32      RwRasterGetWidth(const RwRaster *raster) { return raster->width; }
RwInt32      RwRasterGetHeight(const RwRaster *raster) { return raster->height; }
RwInt32      RwRasterGetStride(const RwRaster *raster);
RwInt32      RwRasterGetDepth(const RwRaster *raster) { return raster->depth; }
RwInt32      RwRasterGetFormat(const RwRaster *raster);
RwInt32      RwRasterGetType(const RwRaster *raster);
RwRaster    *RwRasterGetParent(const RwRaster *raster) { return raster->parent; }
RwRaster    *RwRasterGetOffset(RwRaster *raster,  RwInt16 *xOffset, RwInt16 *yOffset);
RwInt32      RwRasterGetNumLevels(RwRaster * raster);
RwRaster    *RwRasterSubRaster(RwRaster * subRaster, RwRaster * raster, RwRect * rect);
RwRaster    *RwRasterRenderFast(RwRaster * raster, RwInt32 x, RwInt32 y) { return raster->renderFast(x, y) ? raster : nil; }
RwRaster    *RwRasterRender(RwRaster * raster, RwInt32 x, RwInt32 y);
RwRaster    *RwRasterRenderScaled(RwRaster * raster, RwRect * rect);
RwRaster    *RwRasterPushContext(RwRaster * raster) { return Raster::pushContext(raster); }
RwRaster    *RwRasterPopContext(void) { return Raster::popContext(); }
RwRaster    *RwRasterGetCurrentContext(void) { return Raster::getCurrentContext(); }
RwBool       RwRasterClear(RwInt32 pixelValue);
RwBool       RwRasterClearRect(RwRect * rpRect, RwInt32 pixelValue);
RwRaster    *RwRasterShowRaster(RwRaster * raster, void *dev, RwUInt32 flags);
RwUInt8     *RwRasterLock(RwRaster * raster, RwUInt8 level, RwInt32 lockMode);
RwRaster    *RwRasterUnlock(RwRaster * raster);
RwUInt8     *RwRasterLockPalette(RwRaster * raster, RwInt32 lockMode);
RwRaster    *RwRasterUnlockPalette(RwRaster * raster);
RwInt32      RwRasterRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwInt32      RwRasterGetPluginOffset(RwUInt32 pluginID);
RwBool       RwRasterValidatePlugins(const RwRaster * raster);

RwRaster *RwRasterSetFromImage(RwRaster *raster, RwImage *image) { return raster->setFromImage(image); }




RwTexture *RwTextureCreate(RwRaster * raster) { return Texture::create(raster); }
RwBool RwTextureDestroy(RwTexture * texture) { texture->destroy(); return true; }
RwTexture *RwTextureAddRef(RwTexture *texture) { texture->addRef(); return texture; }
// TODO
RwBool RwTextureSetMipmapping(RwBool enable) { return true; }
RwBool RwTextureGetMipmapping(void);
// TODO
RwBool RwTextureSetAutoMipmapping(RwBool enable) { return true; }
RwBool RwTextureGetAutoMipmapping(void);
RwBool RwTextureSetMipmapGenerationCallBack(RwTextureCallBackMipmapGeneration callback);
RwTextureCallBackMipmapGeneration RwTextureGetMipmapGenerationCallBack(void);
RwBool RwTextureSetMipmapNameCallBack(RwTextureCallBackMipmapName callback);
RwTextureCallBackMipmapName RwTextureGetMipmapNameCallBack(void);
RwBool RwTextureGenerateMipmapName(RwChar * name, RwChar * maskName, RwUInt8 mipLevel, RwInt32 format);
RwBool RwTextureRasterGenerateMipmaps(RwRaster * raster, RwImage * image);
RwTextureCallBackRead RwTextureGetReadCallBack(void);
RwBool RwTextureSetReadCallBack(RwTextureCallBackRead fpCallBack);
RwTexture *RwTextureSetName(RwTexture * texture, const RwChar * name) { strncpy(texture->name, name, 32); return texture; }
RwTexture *RwTextureSetMaskName(RwTexture * texture, const RwChar * maskName);
RwChar *RwTextureGetName(RwTexture *texture) { return texture->name; }
RwChar *RwTextureGetMaskName(RwTexture *texture);
RwTexture *RwTextureSetRaster(RwTexture * texture, RwRaster * raster) { texture->raster = raster; return texture; }
RwTexture   *RwTextureRead(const RwChar * name, const RwChar * maskName) { return Texture::read(name, maskName); }
RwRaster *RwTextureGetRaster(const RwTexture *texture) { return texture->raster; }
RwInt32 RwTextureRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwInt32 RwTextureGetPluginOffset(RwUInt32 pluginID);
RwBool RwTextureValidatePlugins(const RwTexture * texture);

RwTexDictionary *RwTextureGetDictionary(RwTexture *texture);
RwTexture *RwTextureSetFilterMode(RwTexture *texture, RwTextureFilterMode filtering) { texture->setFilter((Texture::FilterMode)filtering); return texture; }
RwTextureFilterMode RwTextureGetFilterMode(const RwTexture *texture);
RwTexture *RwTextureSetAddressing(RwTexture *texture, RwTextureAddressMode addressing) {
	texture->setAddressU((Texture::Addressing)addressing);
	texture->setAddressV((Texture::Addressing)addressing);
	return texture;
}
RwTexture *RwTextureSetAddressingU(RwTexture *texture, RwTextureAddressMode addressing) {
	texture->setAddressU((Texture::Addressing)addressing);
	return texture;
}
RwTexture *RwTextureSetAddressingV(RwTexture *texture, RwTextureAddressMode addressing) {
	texture->setAddressV((Texture::Addressing)addressing);
	return texture;
}
RwTextureAddressMode RwTextureGetAddressing(const RwTexture *texture);
RwTextureAddressMode RwTextureGetAddressingU(const RwTexture *texture);
RwTextureAddressMode RwTextureGetAddressingV(const RwTexture *texture);

// TODO
void _rwD3D8TexDictionaryEnableRasterFormatConversion(bool enable) { }

// hack for reading native textures
RwBool rwNativeTextureHackRead(RwStream *stream, RwTexture **tex, RwInt32 size)
{
	*tex = Texture::streamReadNative(stream);
#ifdef LIBRW
	(*tex)->raster = rw::Raster::convertTexToCurrentPlatform((*tex)->raster);
#endif
	return *tex != nil;
}





RwTexDictionary *RwTexDictionaryCreate(void) { return TexDictionary::create(); }
RwBool RwTexDictionaryDestroy(RwTexDictionary * dict) { dict->destroy(); return true; }
RwTexture *RwTexDictionaryAddTexture(RwTexDictionary * dict, RwTexture * texture) { dict->addFront(texture); return texture; }
//RwTexture *RwTexDictionaryRemoveTexture(RwTexture * texture);
RwTexture *RwTexDictionaryFindNamedTexture(RwTexDictionary * dict, const RwChar * name) { return dict->find(name); }
RwTexDictionary *RwTexDictionaryGetCurrent(void) { return TexDictionary::getCurrent(); }
RwTexDictionary *RwTexDictionarySetCurrent(RwTexDictionary * dict) { TexDictionary::setCurrent(dict); return dict; }
const RwTexDictionary *RwTexDictionaryForAllTextures(const RwTexDictionary * dict, RwTextureCallBack fpCallBack, void *pData) {
	FORLIST(lnk, ((RwTexDictionary*)dict)->textures)
		if(fpCallBack(Texture::fromDict(lnk), pData) == nil)
			break;
	return dict;
}
RwBool RwTexDictionaryForAllTexDictionaries(RwTexDictionaryCallBack fpCallBack, void *pData);
RwInt32 RwTexDictionaryRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwInt32 RwTexDictionaryGetPluginOffset(RwUInt32 pluginID);
RwBool RwTexDictionaryValidatePlugins(const RwTexDictionary * dict);
RwUInt32 RwTexDictionaryStreamGetSize(const RwTexDictionary *texDict);
RwTexDictionary *RwTexDictionaryStreamRead(RwStream *stream);
const RwTexDictionary *RwTexDictionaryStreamWrite(const RwTexDictionary *texDict, RwStream *stream) {
	((RwTexDictionary*)texDict)->streamWrite(stream);
	return texDict;
}





RwStream *RwStreamOpen(RwStreamType type, RwStreamAccessType accessType, const void *pData) {
	StreamFile *file;
	StreamMemory *mem;
	RwMemory *memargs;
	const char *mode;

	switch(accessType){
	case rwSTREAMREAD: mode = "rb"; break;
	case rwSTREAMWRITE: mode = "wb"; break;
	case rwSTREAMAPPEND: mode = "ab"; break;
	default: return nil;
	}

	// oh god this is horrible. librw streams really need fixing
	switch(type){
	case rwSTREAMFILENAME:{
		StreamFile fakefile;
		file = rwNewT(StreamFile, 1, 0);
		memcpy(file, &fakefile, sizeof(StreamFile));
#ifndef _WIN32
		char *r = casepath((char*)pData);
		if (r) {
			if (file->open((char*)r, mode)) {
				free(r);
				return file;
			}
			free(r);
		} else
#endif
		{
			if (file->open((char*)pData, mode))
				return file;
		}
		rwFree(file);
		return nil;
	}
	case rwSTREAMMEMORY:{
		StreamMemory fakemem;
		memargs = (RwMemory*)pData;
		mem = rwNewT(StreamMemory, 1, 0);
		memcpy(mem, &fakemem, sizeof(StreamMemory));
		if(mem->open(memargs->start, memargs->length))
			return mem;
		rwFree(mem);
		return nil;
	}
	default:
		assert(0 && "unknown type");
		return nil;
	}
}
RwBool RwStreamClose(RwStream * stream, void *pData) { stream->close(); rwFree(stream); return true; }
RwUInt32 RwStreamRead(RwStream * stream, void *buffer, RwUInt32 length) { return stream->read8(buffer, length); }
RwStream *RwStreamWrite(RwStream * stream, const void *buffer, RwUInt32 length) { stream->write8(buffer, length); return stream; }
RwStream *RwStreamSkip(RwStream * stream, RwUInt32 offset) { stream->seek(offset); return stream; }

RwBool RwStreamFindChunk(RwStream *stream, RwUInt32 type, RwUInt32 *lengthOut, RwUInt32 *versionOut)
	{ return findChunk(stream, type, lengthOut, versionOut); }



void RwIm2DVertexSetCameraX(RwIm2DVertex *vert, RwReal camx) { }
void RwIm2DVertexSetCameraY(RwIm2DVertex *vert, RwReal camy) { }
void RwIm2DVertexSetCameraZ(RwIm2DVertex *vert, RwReal camz) { vert->setCameraZ(camz); }
void RwIm2DVertexSetRecipCameraZ(RwIm2DVertex *vert, RwReal recipz) { vert->setRecipCameraZ(recipz); }
void RwIm2DVertexSetScreenX(RwIm2DVertex *vert, RwReal scrnx) { vert->setScreenX(scrnx); }
void RwIm2DVertexSetScreenY(RwIm2DVertex *vert, RwReal scrny) { vert->setScreenY(scrny); }
void RwIm2DVertexSetScreenZ(RwIm2DVertex *vert, RwReal scrnz) { vert->setScreenZ(scrnz); }
void RwIm2DVertexSetU(RwIm2DVertex *vert, RwReal texU, RwReal recipz) { vert->setU(texU, recipz); }
void RwIm2DVertexSetV(RwIm2DVertex *vert, RwReal texV, RwReal recipz) { vert->setV(texV, recipz); }
void RwIm2DVertexSetIntRGBA(RwIm2DVertex *vert, RwUInt8 red, RwUInt8 green, RwUInt8 blue, RwUInt8 alpha) { vert->setColor(red, green, blue, alpha); }

RwReal RwIm2DGetNearScreenZ(void) { return im2d::GetNearZ(); }
RwReal RwIm2DGetFarScreenZ(void) { return im2d::GetFarZ(); }
RwBool RwIm2DRenderLine(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2)
	{ im2d::RenderLine(vertices, numVertices, vert1, vert2); return true; }
RwBool RwIm2DRenderTriangle(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2, RwInt32 vert3 )
	{ im2d::RenderTriangle(vertices, numVertices, vert1, vert2, vert3); return true; }
RwBool RwIm2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices)
	{ im2d::RenderPrimitive((PrimitiveType)primType, vertices, numVertices); return true; }
RwBool RwIm2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices)
	{ im2d::RenderIndexedPrimitive((PrimitiveType)primType, vertices, numVertices, indices, numIndices); return true; }


void RwIm3DVertexSetPos(RwIm3DVertex *vert, RwReal x, RwReal y, RwReal z) { vert->setX(x); vert->setY(y); vert->setZ(z); }
void RwIm3DVertexSetU(RwIm3DVertex *vert, RwReal u) { vert->setU(u); }
void RwIm3DVertexSetV(RwIm3DVertex *vert, RwReal v) { vert->setV(v); }
void RwIm3DVertexSetRGBA(RwIm3DVertex *vert, RwUInt8 r, RwUInt8 g, RwUInt8 b, RwUInt8 a) { vert->setColor(r, g, b, a); }

void  *RwIm3DTransform(RwIm3DVertex *pVerts, RwUInt32 numVerts, RwMatrix *ltm, RwUInt32 flags) { im3d::Transform(pVerts, numVerts, ltm, flags); return pVerts; }
RwBool RwIm3DEnd(void) { im3d::End(); return true; }
RwBool RwIm3DRenderLine(RwInt32 vert1, RwInt32 vert2) {
	RwImVertexIndex indices[2];
	indices[0] = vert1;
	indices[1] = vert2;
	im3d::RenderIndexedPrimitive((PrimitiveType)PRIMTYPELINELIST, indices, 2);
	return true;
}
RwBool RwIm3DRenderTriangle(RwInt32 vert1, RwInt32 vert2, RwInt32 vert3);
RwBool RwIm3DRenderIndexedPrimitive(RwPrimitiveType primType, RwImVertexIndex *indices, RwInt32 numIndices) { im3d::RenderIndexedPrimitive((PrimitiveType)primType, indices, numIndices); return true; }
RwBool RwIm3DRenderPrimitive(RwPrimitiveType primType);





RwBool RwRenderStateGet(RwRenderState state, void *value)
{
	uint32 *uival = (uint32*)value;
	uint32 fog;
	switch(state){
	case rwRENDERSTATETEXTURERASTER: *(void**)value = GetRenderStatePtr(TEXTURERASTER); return true;
	case rwRENDERSTATETEXTUREADDRESS: *uival = GetRenderState(TEXTUREADDRESS); return true;
	case rwRENDERSTATETEXTUREADDRESSU: *uival = GetRenderState(TEXTUREADDRESSU); return true;
	case rwRENDERSTATETEXTUREADDRESSV: *uival = GetRenderState(TEXTUREADDRESSV); return true;
	case rwRENDERSTATETEXTUREPERSPECTIVE: *uival = 1; return true;
	case rwRENDERSTATEZTESTENABLE: *uival = GetRenderState(ZTESTENABLE); return true;
	case rwRENDERSTATESHADEMODE: *uival = rwSHADEMODEGOURAUD; return true;
	case rwRENDERSTATEZWRITEENABLE: *uival = GetRenderState(ZWRITEENABLE); return true;
	case rwRENDERSTATETEXTUREFILTER: *uival = GetRenderState(TEXTUREFILTER); return true;
	case rwRENDERSTATESRCBLEND: *uival = GetRenderState(SRCBLEND); return true;
	case rwRENDERSTATEDESTBLEND: *uival = GetRenderState(DESTBLEND); return true;
	case rwRENDERSTATEVERTEXALPHAENABLE: *uival = GetRenderState(VERTEXALPHA); return true;
	case rwRENDERSTATEBORDERCOLOR: *uival = 0; return true;
	case rwRENDERSTATEFOGENABLE: *uival = GetRenderState(FOGENABLE); return true;
	case rwRENDERSTATEFOGCOLOR:
		// have to swap R and B here
		fog = GetRenderState(FOGCOLOR);
		*uival = (fog>>16)&0xFF;
		*uival |= (fog&0xFF)<<16;
		*uival |= fog&0xFF00;
		*uival |= fog&0xFF000000;
		return true;
	case rwRENDERSTATEFOGTYPE: *uival = rwFOGTYPELINEAR; return true;
	case rwRENDERSTATEFOGDENSITY: *(float*)value = 1.0f; return true;
	case rwRENDERSTATECULLMODE: *uival = GetRenderState(CULLMODE); return true;

	// all unsupported
	case rwRENDERSTATEFOGTABLE:
	case rwRENDERSTATEALPHAPRIMITIVEBUFFER:

	case rwRENDERSTATESTENCILENABLE:
	case rwRENDERSTATESTENCILFAIL:
	case rwRENDERSTATESTENCILZFAIL:
	case rwRENDERSTATESTENCILPASS:
	case rwRENDERSTATESTENCILFUNCTION:
	case rwRENDERSTATESTENCILFUNCTIONREF:
	case rwRENDERSTATESTENCILFUNCTIONMASK:
	case rwRENDERSTATESTENCILFUNCTIONWRITEMASK:
	default:
		return false;
	}
}
RwBool RwRenderStateSet(RwRenderState state, void *value)
{
	uint32 uival = (uintptr)value;
	uint32 fog;
	switch(state){
	case rwRENDERSTATETEXTURERASTER: SetRenderStatePtr(TEXTURERASTER, value); return true;
	case rwRENDERSTATETEXTUREADDRESS: SetRenderState(TEXTUREADDRESS, uival); return true;
	case rwRENDERSTATETEXTUREADDRESSU: SetRenderState(TEXTUREADDRESSU, uival); return true;
	case rwRENDERSTATETEXTUREADDRESSV: SetRenderState(TEXTUREADDRESSV, uival); return true;
	case rwRENDERSTATETEXTUREPERSPECTIVE: return true;
	case rwRENDERSTATEZTESTENABLE: SetRenderState(ZTESTENABLE, uival); return true;
	case rwRENDERSTATESHADEMODE: return true;
	case rwRENDERSTATEZWRITEENABLE: SetRenderState(ZWRITEENABLE, uival); return true;
	case rwRENDERSTATETEXTUREFILTER: SetRenderState(TEXTUREFILTER, uival); return true;
	case rwRENDERSTATESRCBLEND: SetRenderState(SRCBLEND, uival); return true;
	case rwRENDERSTATEDESTBLEND: SetRenderState(DESTBLEND, uival); return true;
	case rwRENDERSTATEVERTEXALPHAENABLE: SetRenderState(VERTEXALPHA, uival); return true;
	case rwRENDERSTATEBORDERCOLOR: return true;
	case rwRENDERSTATEFOGENABLE: SetRenderState(FOGENABLE, uival); return true;
	case rwRENDERSTATEFOGCOLOR:
		// have to swap R and B here
		fog = (uival>>16)&0xFF;
		fog |= (uival&0xFF)<<16;
		fog |= uival&0xFF00;
		fog |= uival&0xFF000000;
		SetRenderState(FOGCOLOR, fog);
		return true;
	case rwRENDERSTATEFOGTYPE: return true;
	case rwRENDERSTATEFOGDENSITY: return true;
	case rwRENDERSTATEFOGTABLE: return true;
	case rwRENDERSTATEALPHAPRIMITIVEBUFFER: return true;
	case rwRENDERSTATECULLMODE: SetRenderState(CULLMODE, uival); return true;

	// all unsupported
	case rwRENDERSTATESTENCILENABLE:
	case rwRENDERSTATESTENCILFAIL:
	case rwRENDERSTATESTENCILZFAIL:
	case rwRENDERSTATESTENCILPASS:
	case rwRENDERSTATESTENCILFUNCTION:
	case rwRENDERSTATESTENCILFUNCTIONREF:
	case rwRENDERSTATESTENCILFUNCTIONMASK:
	case rwRENDERSTATESTENCILFUNCTIONWRITEMASK:
	default:
		return true;
	}
}

static rw::MemoryFunctions gMemfuncs;
static void *(*real_malloc)(size_t size);
static void *(*real_realloc)(void *mem, size_t newSize);
static void *mallocWrap(size_t sz, uint32 hint) { if(sz == 0) return nil; return real_malloc(sz); }
static void *reallocWrap(void *p, size_t sz, uint32 hint) { return real_realloc(p, sz); }


// WARNING: unused parameters
RwBool RwEngineInit(RwMemoryFunctions *memFuncs, RwUInt32 initFlags, RwUInt32 resArenaSize) {
	if(memFuncs){
		real_malloc = memFuncs->rwmalloc;
		real_realloc = memFuncs->rwrealloc;
		gMemfuncs.rwmalloc = mallocWrap;
		gMemfuncs.rwrealloc = reallocWrap;
		gMemfuncs.rwfree = memFuncs->rwfree;
		Engine::init(&gMemfuncs);
	}else{
		Engine::init(nil);
	}
	return true;
}
// TODO: this is platform dependent
RwBool RwEngineOpen(RwEngineOpenParams *initParams) {
	static EngineOpenParams openParams;
#ifdef RW_D3D9
	openParams.window = (HWND)initParams->displayID;
#else
	openParams = *(EngineOpenParams*)initParams->displayID;
#endif
	return Engine::open(&openParams);
}
RwBool RwEngineStart(void) {
	rw::d3d::isP8supported = false;
	return Engine::start();
}
RwBool RwEngineStop(void) { Engine::stop(); return true; }
RwBool RwEngineClose(void) { Engine::close(); return true; }
RwBool RwEngineTerm(void) { Engine::term(); return true; }
RwInt32 RwEngineRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor initCB, RwPluginObjectDestructor termCB);
RwInt32 RwEngineGetPluginOffset(RwUInt32 pluginID);
RwInt32 RwEngineGetNumSubSystems(void) { return Engine::getNumSubSystems(); }
RwSubSystemInfo *RwEngineGetSubSystemInfo(RwSubSystemInfo *subSystemInfo, RwInt32 subSystemIndex)
	{ return Engine::getSubSystemInfo(subSystemInfo, subSystemIndex); }
RwInt32 RwEngineGetCurrentSubSystem(void) { return Engine::getCurrentSubSystem(); }
RwBool RwEngineSetSubSystem(RwInt32 subSystemIndex) { return Engine::setSubSystem(subSystemIndex); }
RwInt32 RwEngineGetNumVideoModes(void) { return Engine::getNumVideoModes(); }
RwVideoMode *RwEngineGetVideoModeInfo(RwVideoMode *modeinfo, RwInt32 modeIndex)
	{ return Engine::getVideoModeInfo(modeinfo, modeIndex); }
RwInt32 RwEngineGetCurrentVideoMode(void) { return Engine::getCurrentVideoMode(); }
RwBool RwEngineSetVideoMode(RwInt32 modeIndex) { return Engine::setVideoMode(modeIndex); }
RwInt32 RwEngineGetTextureMemorySize(void);
RwInt32 RwEngineGetMaxTextureSize(void);



// TODO
void RwD3D8EngineSetRefreshRate(RwUInt32 refreshRate) {}
RwBool RwD3D8DeviceSupportsDXTTexture(void) { return true; }


void RwD3D8EngineSetMultiSamplingLevels(RwUInt32 level) { Engine::setMultiSamplingLevels(level); }
RwUInt32 RwD3D8EngineGetMaxMultiSamplingLevels(void) { return Engine::getMaxMultiSamplingLevels(); }


RpMaterial *RpMaterialCreate(void) { return Material::create(); }
RwBool RpMaterialDestroy(RpMaterial *material) { material->destroy(); return true; }
//RpMaterial *RpMaterialClone(RpMaterial *material);
RpMaterial *RpMaterialSetTexture(RpMaterial *material, RwTexture *texture) { material->setTexture(texture); return material; }
//RpMaterial *RpMaterialAddRef(RpMaterial *material);
RwTexture *RpMaterialGetTexture(const RpMaterial *material) { return material->texture; }
RpMaterial *RpMaterialSetColor(RpMaterial *material, const RwRGBA *color) { material->color = *color; return material; }
const RwRGBA *RpMaterialGetColor(const RpMaterial *material) { return &material->color; }
RpMaterial *RpMaterialSetSurfaceProperties(RpMaterial *material, const RwSurfaceProperties *surfaceProperties);
const RwSurfaceProperties *RpMaterialGetSurfaceProperties(const RpMaterial *material) { return &material->surfaceProps; }
//RwInt32 RpMaterialRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
//RwInt32 RpMaterialRegisterPluginStream(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB, RwPluginDataChunkWriteCallBack writeCB, RwPluginDataChunkGetSizeCallBack getSizeCB);
//RwInt32 RpMaterialSetStreamAlwaysCallBack(RwUInt32 pluginID, RwPluginDataChunkAlwaysCallBack alwaysCB);
//RwInt32 RpMaterialGetPluginOffset(RwUInt32 pluginID);
//RwBool RpMaterialValidatePlugins(const RpMaterial *material);
//RwUInt32 RpMaterialStreamGetSize(const RpMaterial *material);
//RpMaterial *RpMaterialStreamRead(RwStream *stream);
//const RpMaterial *RpMaterialStreamWrite(const RpMaterial *material, RwStream *stream);
//RpMaterialChunkInfo *_rpMaterialChunkInfoRead(RwStream *stream,  RpMaterialChunkInfo *materialChunkInfo, RwInt32 *bytesRead);





RwReal RpLightGetRadius(const RpLight *light) { return light->radius; }
//const RwRGBAReal *RpLightGetColor(const RpLight *light);
RpLight *RpLightSetFrame(RpLight *light, RwFrame *frame) { light->setFrame(frame); return light; }
RwFrame *RpLightGetFrame(const RpLight *light) { return light->getFrame(); }
//RpLightType RpLightGetType(const RpLight *light);
RpLight *RpLightSetFlags(RpLight *light, RwUInt32 flags) { light->setFlags(flags); return light; }
//RwUInt32 RpLightGetFlags(const RpLight *light);
RpLight *RpLightCreate(RwInt32 type) { return rw::Light::create(type); }
RwBool RpLightDestroy(RpLight *light) { light->destroy(); return true; }
RpLight *RpLightSetRadius(RpLight *light, RwReal radius) { light->radius = radius; return light; }
RpLight *RpLightSetColor(RpLight *light, const RwRGBAReal *color) { light->setColor(color->red, color->green, color->blue); return light; }
//RwReal RpLightGetConeAngle(const RpLight *light);
//RpLight *RpLightSetConeAngle(RpLight * ight, RwReal angle);
//RwUInt32 RpLightStreamGetSize(const RpLight *light);
//RpLight *RpLightStreamRead(RwStream *stream);
//const RpLight *RpLightStreamWrite(const RpLight *light, RwStream *stream);
//RpLightChunkInfo *_rpLightChunkInfoRead(RwStream *stream, RpLightChunkInfo *lightChunkInfo, RwInt32 *bytesRead);
//RwInt32 RpLightRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
//RwInt32 RpLightRegisterPluginStream(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB, RwPluginDataChunkWriteCallBack writeCB, RwPluginDataChunkGetSizeCallBack getSizeCB);
//RwInt32 RpLightSetStreamAlwaysCallBack(RwUInt32 pluginID, RwPluginDataChunkAlwaysCallBack alwaysCB);
//RwInt32 RpLightGetPluginOffset(RwUInt32 pluginID);
//RwBool RpLightValidatePlugins(const RpLight * light);





RpGeometry  *RpGeometryCreate(RwInt32 numVert, RwInt32 numTriangles, RwUInt32 format) { return Geometry::create(numVert, numTriangles, format); }
RwBool RpGeometryDestroy(RpGeometry *geometry) { geometry->destroy(); return true; }
RpGeometry *_rpGeometryAddRef(RpGeometry *geometry);
RpGeometry  *RpGeometryLock(RpGeometry *geometry, RwInt32 lockMode) { geometry->lock(lockMode); return geometry; }
RpGeometry  *RpGeometryUnlock(RpGeometry *geometry) { geometry->unlock(); return geometry; }
RpGeometry  *RpGeometryTransform(RpGeometry *geometry, const RwMatrix *matrix);
RpGeometry  *RpGeometryCreateSpace(RwReal radius);
RpMorphTarget  *RpMorphTargetSetBoundingSphere(RpMorphTarget *morphTarget, const RwSphere *boundingSphere) { morphTarget->boundingSphere = *boundingSphere; return morphTarget; }
RwSphere  *RpMorphTargetGetBoundingSphere(RpMorphTarget *morphTarget) { return &morphTarget->boundingSphere; }
const RpMorphTarget  *RpMorphTargetCalcBoundingSphere(const RpMorphTarget *morphTarget, RwSphere *boundingSphere) { *boundingSphere = morphTarget->calculateBoundingSphere(); return morphTarget; }
RwInt32 RpGeometryAddMorphTargets(RpGeometry *geometry, RwInt32 mtcount) { RwInt32 n = geometry->numMorphTargets; geometry->addMorphTargets(mtcount); return n; }
RwInt32 RpGeometryAddMorphTarget(RpGeometry *geometry) { return RpGeometryAddMorphTargets(geometry, 1); }
RpGeometry  *RpGeometryRemoveMorphTarget(RpGeometry *geometry, RwInt32 morphTarget);
RwInt32 RpGeometryGetNumMorphTargets(const RpGeometry *geometry);
RpMorphTarget  *RpGeometryGetMorphTarget(const RpGeometry *geometry, RwInt32 morphTarget) { return &geometry->morphTargets[morphTarget]; }
RwRGBA  *RpGeometryGetPreLightColors(const RpGeometry *geometry) { return geometry->colors; }
RwTexCoords  *RpGeometryGetVertexTexCoords(const RpGeometry *geometry, RwTextureCoordinateIndex uvIndex) {
	if(uvIndex == rwNARWTEXTURECOORDINATEINDEX)
		return nil;
	return geometry->texCoords[uvIndex-rwTEXTURECOORDINATEINDEX0];
}
RwInt32 RpGeometryGetNumTexCoordSets(const RpGeometry *geometry) { return geometry->numTexCoordSets; }
RwInt32 RpGeometryGetNumVertices (const RpGeometry *geometry) { return geometry->numVertices; }
RwV3d  *RpMorphTargetGetVertices(const RpMorphTarget *morphTarget) { return morphTarget->vertices; }
RwV3d  *RpMorphTargetGetVertexNormals(const RpMorphTarget *morphTarget) { return morphTarget->normals; }
RpTriangle  *RpGeometryGetTriangles(const RpGeometry *geometry) { return geometry->triangles; }
RwInt32 RpGeometryGetNumTriangles(const RpGeometry *geometry) { return geometry->numTriangles; }
RpMaterial  *RpGeometryGetMaterial(const RpGeometry *geometry, RwInt32 matNum) { return geometry->matList.materials[matNum]; }
const RpGeometry  *RpGeometryTriangleSetVertexIndices(const RpGeometry *geometry, RpTriangle *triangle, RwUInt16 vert1, RwUInt16 vert2, RwUInt16 vert3)
	{ triangle->v[0] = vert1; triangle->v[1] = vert2; triangle->v[2] = vert3; return geometry; }
RpGeometry  *RpGeometryTriangleSetMaterial(RpGeometry *geometry, RpTriangle *triangle, RpMaterial *material) {
	int id = geometry->matList.findIndex(material);
	if(id < 0)
		id = geometry->matList.appendMaterial(material);
	if(id < 0)
		return nil;
	triangle->matId = id;
	return geometry;
}
const RpGeometry  *RpGeometryTriangleGetVertexIndices(const RpGeometry *geometry, const RpTriangle *triangle, RwUInt16 *vert1, RwUInt16 *vert2, RwUInt16 *vert3);
RpMaterial   *RpGeometryTriangleGetMaterial(const RpGeometry *geometry, const RpTriangle *triangle);
RwInt32 RpGeometryGetNumMaterials(const RpGeometry *geometry);
RpGeometry  *RpGeometryForAllMaterials(RpGeometry *geometry, RpMaterialCallBack fpCallBack, void *pData) {
	int i;
	for(i = 0; i < geometry->matList.numMaterials; i++)
		if(fpCallBack(geometry->matList.materials[i], pData) == nil)
			break;
	return geometry;
}
//const RpGeometry  *RpGeometryForAllMeshes(const RpGeometry *geometry, RpMeshCallBack fpCallBack, void *pData);
RwInt32 RpGeometryRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwInt32 RpGeometryRegisterPluginStream(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB, RwPluginDataChunkWriteCallBack writeCB, RwPluginDataChunkGetSizeCallBack getSizeCB);
RwInt32 RpGeometrySetStreamAlwaysCallBack(RwUInt32 pluginID, RwPluginDataChunkAlwaysCallBack alwaysCB);
RwInt32 RpGeometryGetPluginOffset(RwUInt32 pluginID);
RwBool RpGeometryValidatePlugins(const RpGeometry *geometry);
RwUInt32 RpGeometryStreamGetSize(const RpGeometry *geometry);
const RpGeometry  *RpGeometryStreamWrite(const RpGeometry *geometry, RwStream *stream);
RpGeometry  *RpGeometryStreamRead(RwStream *stream) { return Geometry::streamRead(stream); }
//RpGeometryChunkInfo *_rpGeometryChunkInfoRead(RwStream *stream, RpGeometryChunkInfo *geometryChunkInfo, RwInt32 *bytesRead);
RwUInt32 RpGeometryGetFlags(const RpGeometry *geometry) { return geometry->flags; }
RpGeometry  *RpGeometrySetFlags(RpGeometry *geometry, RwUInt32 flags) { geometry->flags = flags; return geometry; }
const RwSurfaceProperties *_rpGeometryGetSurfaceProperties(const RpGeometry *geometry);
RpGeometry *_rpGeometrySetSurfaceProperties(RpGeometry *geometry, const RwSurfaceProperties *surfaceProperties);





RwFrame *RpClumpGetFrame(const RpClump * clump) { return clump->getFrame(); }
RpClump *RpClumpSetFrame(RpClump * clump, RwFrame * frame) { clump->setFrame(frame); return clump; }
RpClump *RpClumpForAllAtomics(RpClump * clump, RpAtomicCallBack callback, void *pData) {
	FORLIST(lnk, clump->atomics)
		if(callback(Atomic::fromClump(lnk), pData) == nil)
			break;
	return clump;
}
RpClump *RpClumpForAllLights(RpClump * clump, RpLightCallBack callback, void *pData);
RpClump *RpClumpForAllCameras(RpClump * clump, RwCameraCallBack callback, void *pData);
//RpClump *RpClumpCreateSpace(const RwV3d * position, RwReal radius);
RpClump *RpClumpRender(RpClump * clump) { clump->render(); return clump; }
RpClump *RpClumpRemoveAtomic(RpClump * clump, RpAtomic * atomic) { clump->removeAtomic(atomic); return clump; }
RpClump *RpClumpAddAtomic(RpClump * clump, RpAtomic * atomic) { clump->addAtomic(atomic); return clump; }
//RpClump *RpClumpRemoveLight(RpClump * clump, RpLight * light);
//RpClump *RpClumpAddLight(RpClump * clump, RpLight * light);
//RpClump *RpClumpRemoveCamera(RpClump * clump, RwCamera * camera);
//RpClump *RpClumpAddCamera(RpClump * clump, RwCamera * camera);
RwBool RpClumpDestroy(RpClump * clump) { clump->destroy(); return true; }
RpClump *RpClumpCreate(void) { return rw::Clump::create(); }
RpClump *RpClumpClone(RpClump * clump) { return clump->clone(); }
//RpClump *RpClumpSetCallBack(RpClump * clump, RpClumpCallBack callback);
//RpClumpCallBack RpClumpGetCallBack(const RpClump * clump);
RwInt32 RpClumpGetNumAtomics(RpClump * clump) { return clump->countAtomics(); }
//RwInt32 RpClumpGetNumLights(RpClump * clump);
//RwInt32 RpClumpGetNumCameras(RpClump * clump);
RpClump *RpClumpStreamRead(RwStream * stream) { return rw::Clump::streamRead(stream); }
//RpClump *RpClumpStreamWrite(RpClump * clump, RwStream * stream);
RwInt32 RpClumpRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB)
	{ return Clump::registerPlugin(size, pluginID, constructCB, destructCB, (CopyConstructor)copyCB); }
RwInt32 RpClumpRegisterPluginStream(RwUInt32 pluginID, RwPluginDataChunkReadCallBack  readCB, RwPluginDataChunkWriteCallBack writeCB, RwPluginDataChunkGetSizeCallBack getSizeCB)
	{ return Clump::registerPluginStream(pluginID, readCB, (StreamWrite)writeCB, (StreamGetSize)getSizeCB); }
//RwInt32 RpClumpSetStreamAlwaysCallBack(RwUInt32 pluginID, RwPluginDataChunkAlwaysCallBack alwaysCB);
//RwInt32 RpClumpGetPluginOffset(RwUInt32 pluginID);
//RwBool RpClumpValidatePlugins(const RpClump * clump);



RpAtomic *RpAtomicCreate(void) { return rw::Atomic::create(); }
RwBool RpAtomicDestroy(RpAtomic * atomic) { atomic->destroy(); return true; }
RpAtomic *RpAtomicClone(RpAtomic * atomic) { return atomic->clone(); }
RpAtomic *RpAtomicSetFrame(RpAtomic * atomic, RwFrame * frame) { atomic->setFrame(frame); return atomic; }
RpAtomic *RpAtomicSetGeometry(RpAtomic * atomic, RpGeometry * geometry, RwUInt32 flags) { atomic->setGeometry(geometry, flags); return atomic; }

RwFrame *RpAtomicGetFrame(const RpAtomic * atomic) { return atomic->getFrame(); }
RpAtomic *RpAtomicSetFlags(RpAtomic * atomic, RwUInt32 flags) { atomic->setFlags(flags); return atomic; }
RwUInt32 RpAtomicGetFlags(const RpAtomic * atomic) { return atomic->getFlags(); }
RwSphere *RpAtomicGetBoundingSphere(RpAtomic * atomic) { return &atomic->boundingSphere; }
RpAtomic *RpAtomicRender(RpAtomic * atomic) { atomic->render(); return atomic; }
RpClump *RpAtomicGetClump(const RpAtomic * atomic) { return atomic->clump; }
//RpInterpolator *RpAtomicGetInterpolator(RpAtomic * atomic);
RpGeometry *RpAtomicGetGeometry(const RpAtomic * atomic) { return atomic->geometry; }
// WARNING: illegal cast
void RpAtomicSetRenderCallBack(RpAtomic * atomic, RpAtomicCallBackRender callback) { atomic->setRenderCB((Atomic::RenderCB)callback); }
RpAtomicCallBackRender RpAtomicGetRenderCallBack(const RpAtomic * atomic) { return (RpAtomicCallBackRender)atomic->renderCB; }
//RwBool RpAtomicInstance(RpAtomic *atomic);
//RwUInt32 RpAtomicStreamGetSize(RpAtomic * atomic);
//RpAtomic *RpAtomicStreamRead(RwStream * stream);
//RpAtomic *RpAtomicStreamWrite(RpAtomic * atomic, RwStream * stream);
RwInt32 RpAtomicRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB)
	{ return Atomic::registerPlugin(size, pluginID, constructCB, destructCB, (CopyConstructor)copyCB); }
//RwInt32 RpAtomicRegisterPluginStream(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB, RwPluginDataChunkWriteCallBack writeCB, RwPluginDataChunkGetSizeCallBack getSizeCB);
//RwInt32 RpAtomicSetStreamAlwaysCallBack(RwUInt32 pluginID, RwPluginDataChunkAlwaysCallBack alwaysCB);
//RwInt32 RpAtomicSetStreamRightsCallBack(RwUInt32 pluginID, RwPluginDataChunkRightsCallBack rightsCB);
//RwInt32 RpAtomicGetPluginOffset(RwUInt32 pluginID);
//RwBool RpAtomicValidatePlugins(const RpAtomic * atomic);

RpAtomic *AtomicDefaultRenderCallBack(RpAtomic * atomic) { Atomic::defaultRenderCB(atomic); return atomic; }


// TODO: this is extremely simplified
RpWorld     *RpWorldCreate(RwBBox * boundingBox) { return World::create(); }
RwBool       RpWorldDestroy(RpWorld * world) { world->destroy(); return true; }

RwBool       RpWorldPluginAttach(void) {
	registerMeshPlugin();
	registerNativeDataPlugin();
	registerAtomicRightsPlugin();
	registerMaterialRightsPlugin();

	// not sure if this goes here
	rw::xbox::registerVertexFormatPlugin();
	return true;
}

RpWorld *RpWorldRemoveCamera(RpWorld *world, RwCamera *camera) { world->removeCamera(camera); return world; }
RpWorld *RpWorldAddCamera(RpWorld *world, RwCamera *camera) { world->addCamera(camera); return world; }
RpWorld *RwCameraGetWorld(const RwCamera *camera);
RpWorld *RpWorldRemoveAtomic(RpWorld *world, RpAtomic *atomic);
RpWorld *RpWorldAddAtomic(RpWorld *world, RpAtomic *atomic);
RpWorld *RpAtomicGetWorld(const RpAtomic *atomic);
RpWorld *RpWorldAddClump(RpWorld *world, RpClump *clump);
RpWorld *RpWorldRemoveClump(RpWorld *world, RpClump *clump);
RpWorld *RpClumpGetWorld(const RpClump *clump);
RpWorld *RpWorldAddLight(RpWorld *world, RpLight *light) { world->addLight(light); return world; }
RpWorld *RpWorldRemoveLight(RpWorld *world, RpLight *light) { world->removeLight(light); return world; }
RpWorld *RpLightGetWorld(const RpLight *light);
RwCamera *RwCameraForAllClumpsInFrustum(RwCamera *camera, void *data);
RwCamera *RwCameraForAllClumpsNotInFrustum(RwCamera *camera, RwInt32 numClumps, void *data);




RwBool RpMatFXPluginAttach( void ) { registerMatFXPlugin(); return true; }
RpAtomic *RpMatFXAtomicEnableEffects( RpAtomic *atomic ) { MatFX::enableEffects(atomic); return atomic; }
RpMatFXMaterialFlags RpMatFXMaterialGetEffects( const RpMaterial *material ){ return (RpMatFXMaterialFlags)MatFX::getEffects(material); }
RpMaterial *RpMatFXMaterialSetEffects( RpMaterial *material, RpMatFXMaterialFlags flags ) { MatFX::setEffects(material, (uint32)flags); return material; }
RpMaterial *RpMatFXMaterialSetupEnvMap( RpMaterial *material, RwTexture *texture, RwFrame *frame, RwBool useFrameBufferAlpha, RwReal coef ) {
	MatFX *mfx = MatFX::get(material);
	mfx->setEnvTexture(texture);
	mfx->setEnvFrame(frame);
	mfx->setEnvCoefficient(coef);
	return material;
}
RpMaterial *RpMatFXMaterialSetEnvMapFrame( RpMaterial *material, RwFrame *frame )
{
	MatFX *mfx = MatFX::get(material);
	mfx->setEnvFrame(frame);
	return material;
}
RpMaterial *RpMatFXMaterialSetEnvMapFrameBufferAlpha( RpMaterial *material, RwBool useFrameBufferAlpha )
{
	MatFX *mfx = MatFX::get(material);
	mfx->setEnvFBAlpha(useFrameBufferAlpha);
	return material;
}
RpMaterial *RpMatFXMaterialSetEnvMapCoefficient( RpMaterial *material, RwReal coef )
{
	MatFX *mfx = MatFX::get(material);
	mfx->setEnvCoefficient(coef);
	return material;
}
RwReal RpMatFXMaterialGetEnvMapCoefficient( const RpMaterial *material )
{
	MatFX *mfx = MatFX::get(material);
	return mfx->getEnvCoefficient();
}



RwBool RpHAnimPluginAttach(void) {
	registerHAnimPlugin();
	return true;
}

RwInt32 RpHAnimFrameGetID(RwFrame *frame) { return HAnimData::get(frame)->id; }

RwInt32 RpHAnimIDGetIndex(RpHAnimHierarchy *hierarchy, RwInt32 ID) { return hierarchy->getIndex(ID); }

RwBool RpHAnimFrameSetHierarchy(RwFrame *frame, RpHAnimHierarchy *hierarchy) { HAnimData::get(frame)->hierarchy = hierarchy; return true; }
RpHAnimHierarchy *RpHAnimFrameGetHierarchy(RwFrame *frame) { return HAnimHierarchy::get(frame); }

RpHAnimHierarchy *RpHAnimHierarchySetFlags(RpHAnimHierarchy *hierarchy, RpHAnimHierarchyFlag flags) { hierarchy->flags = flags; return hierarchy; }

RwBool RpHAnimHierarchySetCurrentAnim(RpHAnimHierarchy *hierarchy, RpHAnimAnimation *anim) { hierarchy->interpolator->setCurrentAnim(anim); return true; }
RwBool RpHAnimHierarchyAddAnimTime(RpHAnimHierarchy *hierarchy, RwReal time) { hierarchy->interpolator->addTime(time); return true; }

RwMatrix *RpHAnimHierarchyGetMatrixArray(RpHAnimHierarchy *hierarchy) { return hierarchy->matrices; }
RwBool RpHAnimHierarchyUpdateMatrices(RpHAnimHierarchy *hierarchy) { hierarchy->updateMatrices(); return true; }

RpHAnimAnimation *RpHAnimAnimationCreate(RwInt32 typeID, RwInt32 numFrames, RwInt32 flags, RwReal duration)
	{ return Animation::create(AnimInterpolatorInfo::find(typeID), numFrames, flags, duration); }
RpHAnimAnimation  *RpHAnimAnimationDestroy(RpHAnimAnimation *animation) { animation->destroy(); return animation; }
RpHAnimAnimation  *RpHAnimAnimationStreamRead(RwStream *stream) { return Animation::streamRead(stream); }






RwBool RpSkinPluginAttach(void) {
	registerSkinPlugin();
	return true;
}

RwUInt32 RpSkinGetNumBones( RpSkin *skin ) { return skin->numBones; }
const RwMatrixWeights *RpSkinGetVertexBoneWeights( RpSkin *skin ) { return (RwMatrixWeights*)skin->weights; }
const RwUInt32 *RpSkinGetVertexBoneIndices( RpSkin *skin ) { return (RwUInt32*)skin->indices; }
const RwMatrix *RpSkinGetSkinToBoneMatrices( RpSkin *skin ) { return (const RwMatrix*)skin->inverseMatrices; }

RpSkin *RpSkinGeometryGetSkin( RpGeometry *geometry ) { return Skin::get(geometry); }

RpAtomic *RpSkinAtomicSetHAnimHierarchy( RpAtomic *atomic, RpHAnimHierarchy *hierarchy ) { Skin::setHierarchy(atomic, hierarchy); return atomic; }
RpHAnimHierarchy *RpSkinAtomicGetHAnimHierarchy( const RpAtomic *atomic ) { return Skin::getHierarchy(atomic); }

RwImage *
RtBMPImageWrite(RwImage *image, const RwChar *imageName)
{
#ifndef _WIN32
	char *r = casepath(imageName);
	if (r) {
		rw::writeBMP(image, r);
		free(r);
	} else {
		rw::writeBMP(image, imageName);
	}
	
#else
	rw::writeBMP(image, imageName);
#endif
	return image;
}
RwImage *
RtBMPImageRead(const RwChar *imageName)
{
#ifndef _WIN32
	RwImage *image;
	char *r = casepath(imageName);
	if (r) {
		image = rw::readBMP(r);
		free(r);
	} else {
		image = rw::readBMP(imageName);
	}
	return image;

#else
	return rw::readBMP(imageName);
#endif
}


RwImage *
RtPNGImageWrite(RwImage *image, const RwChar *imageName)
{
#ifndef _WIN32
	char *r = casepath(imageName);
	if (r) {
		rw::writePNG(image, r);
		free(r);
	} else {
		rw::writePNG(image, imageName);
	}
	
#else
	rw::writePNG(image, imageName);
#endif
	return image;
}
RwImage *
RtPNGImageRead(const RwChar *imageName)
{
#ifndef _WIN32
	RwImage *image;
	char *r = casepath(imageName);
	if (r) {
		image = rw::readPNG(r);
		free(r);
	} else {
		image = rw::readPNG(imageName);
	}
	return image;

#else
	return rw::readPNG(imageName);
#endif
}

#include "rtquat.h"

RtQuat *RtQuatRotate(RtQuat * quat, const RwV3d * axis, RwReal angle, RwOpCombineType combineOp) { return (RtQuat*)((rw::Quat*)quat)->rotate(axis, angle/180.0f*3.14159f, (CombineOp)combineOp); }
void RtQuatConvertToMatrix(const RtQuat * const qpQuat, RwMatrix * const mpMatrix) { mpMatrix->rotate(*(rw::Quat*)qpQuat, COMBINEREPLACE); }


#include "rtcharse.h"

RwBool       RtCharsetOpen(void) { return Charset::open(); }
void         RtCharsetClose(void) { return Charset::close(); }
RtCharset   *RtCharsetPrint(RtCharset * charSet, const RwChar * string, RwInt32 x, RwInt32 y) { charSet->print(string, x, y, true); return charSet; }
RtCharset   *RtCharsetPrintBuffered(RtCharset * charSet, const RwChar * string, RwInt32 x, RwInt32 y, RwBool hideSpaces) { charSet->printBuffered(string, x, y, hideSpaces); return charSet; }
RwBool       RtCharsetBufferFlush(void) { Charset::flushBuffer(); return true; }
RtCharset   *RtCharsetSetColors(RtCharset * charSet, const RwRGBA * foreGround, const RwRGBA * backGround) { return charSet->setColors(foreGround, backGround); }
RtCharset   *RtCharsetGetDesc(RtCharset * charset, RtCharsetDesc * desc) { *desc = charset->desc; return charset; }
RtCharset   *RtCharsetCreate(const RwRGBA * foreGround, const RwRGBA * backGround) { return Charset::create(foreGround, backGround); }
RwBool       RtCharsetDestroy(RtCharset * charSet) { charSet->destroy(); return true; }



#include <rpanisot.h>

RwInt8      RpAnisotGetMaxSupportedMaxAnisotropy(void) { return rw::getMaxSupportedMaxAnisotropy(); }
RwTexture    *RpAnisotTextureSetMaxAnisotropy(RwTexture *tex, RwInt8 val) { tex->setMaxAnisotropy(val); return tex; }
RwInt8       RpAnisotTextureGetMaxAnisotropy(RwTexture *tex) { return tex->getMaxAnisotropy(); }
RwBool       RpAnisotPluginAttach(void) { rw::registerAnisotropyPlugin(); return true; }
