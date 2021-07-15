#include "common.h"

#include "main.h"
#include "PlayerSkin.h"
#include "TxdStore.h"
#include "rtbmp.h"
#include "ClumpModelInfo.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "PlayerInfo.h"
#include "CdStream.h"
#include "FileMgr.h"
#include "Directory.h"
#include "RwHelper.h"
#include "Timer.h"
#include "Lights.h"
#include "MemoryMgr.h"

RpClump *gpPlayerClump;
float gOldFov;

int CPlayerSkin::m_txdSlot;

void
FindPlayerDff(uint32 &offset, uint32 &size)
{
	int file;
	CDirectory::DirectoryInfo info;

	file = CFileMgr::OpenFile("models\\gta3.dir", "rb");

	do {
		if (!CFileMgr::Read(file, (char*)&info, sizeof(CDirectory::DirectoryInfo)))
			return;
	} while (strcasecmp("player.dff", info.name) != 0);

	offset = info.offset;
	size = info.size;
}

void
LoadPlayerDff(void)
{
	RwStream *stream;
	RwMemory mem;
	uint32 offset, size;
	uint8 *buffer;
	bool streamWasAdded = false;

	if (CdStreamGetNumImages() == 0) {
		CdStreamAddImage("models\\gta3.img");
		streamWasAdded = true;
	}

	FindPlayerDff(offset, size);
	buffer = (uint8*)RwMallocAlign(size << 11, 2048);
	CdStreamRead(0, buffer, offset, size);
	CdStreamSync(0);

	mem.start = buffer;
	mem.length = size << 11;
	stream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &mem);

	if (RwStreamFindChunk(stream, rwID_CLUMP, nil, nil))
		gpPlayerClump = RpClumpStreamRead(stream);

	RwStreamClose(stream, &mem);
	RwFreeAlign(buffer);

	if (streamWasAdded)
		CdStreamRemoveImages();
}

void
CPlayerSkin::Initialise(void)
{
	m_txdSlot = CTxdStore::AddTxdSlot("skin");
	CTxdStore::Create(m_txdSlot);
	CTxdStore::AddRef(m_txdSlot);
}

void
CPlayerSkin::Shutdown(void)
{
	CTxdStore::RemoveTxdSlot(m_txdSlot);
}

RwTexture *
CPlayerSkin::GetSkinTexture(const char *texName)
{
	RwTexture *tex;
	RwRaster *raster;
	int32 width, height, depth, format;

	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(m_txdSlot);
	tex = RwTextureRead(texName, NULL);
	CTxdStore::PopCurrentTxd();
	if (tex != nil) return tex;

	if (strcmp(DEFAULT_SKIN_NAME, texName) == 0)
		sprintf(gString, "models\\generic\\player.bmp");
	else
		sprintf(gString, "skins\\%s.bmp", texName);

	if (RwImage *image = RtBMPImageRead(gString)) {
		RwImageFindRasterFormat(image, rwRASTERTYPETEXTURE, &width, &height, &depth, &format);
		raster = RwRasterCreate(width, height, depth, format);
		RwRasterSetFromImage(raster, image);

		tex = RwTextureCreate(raster);
		RwTextureSetName(tex, texName);
#ifdef FIX_BUGS
		RwTextureSetFilterMode(tex, rwFILTERLINEAR); // filtering bugfix from VC
#endif
		RwTexDictionaryAddTexture(CTxdStore::GetSlot(m_txdSlot)->texDict, tex);

		RwImageDestroy(image);
	}
	return tex;
}

void
CPlayerSkin::BeginFrontendSkinEdit(void)
{
	LoadPlayerDff();
	RpClumpForAllAtomics(gpPlayerClump, CClumpModelInfo::SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPlayerCB);
	CWorld::Players[0].LoadPlayerSkin();
	gOldFov = CDraw::GetFOV();
	CDraw::SetFOV(30.0f);
}

void
CPlayerSkin::EndFrontendSkinEdit(void)
{
	RpClumpDestroy(gpPlayerClump);
	gpPlayerClump = NULL;
	CDraw::SetFOV(gOldFov);
}

void
CPlayerSkin::RenderFrontendSkinEdit(void)
{
	static float rotation = 0.0f;
	RwRGBAReal AmbientColor = { 0.65f, 0.65f, 0.65f, 1.0f };
	const RwV3d pos = { 1.35f, 0.35f, 7.725f };
	const RwV3d axis1 = { 1.0f, 0.0f, 0.0f };
	const RwV3d axis2 = { 0.0f, 0.0f, 1.0f };
	static uint32 LastFlash = 0;

	RwFrame *frame = RpClumpGetFrame(gpPlayerClump);

	if (CTimer::GetTimeInMillisecondsPauseMode() - LastFlash > 7) {
		rotation += 2.0f;
		if (rotation > 360.0f)
			rotation -= 360.0f;
		LastFlash = CTimer::GetTimeInMillisecondsPauseMode();
	}
	RwFrameTransform(frame, RwFrameGetMatrix(RwCameraGetFrame(Scene.camera)), rwCOMBINEREPLACE);
	RwFrameTranslate(frame, &pos, rwCOMBINEPRECONCAT);
	RwFrameRotate(frame, &axis1, -90.0f, rwCOMBINEPRECONCAT);
	RwFrameRotate(frame, &axis2, rotation, rwCOMBINEPRECONCAT);
	RwFrameUpdateObjects(frame);
	SetAmbientColours(&AmbientColor);
	RpClumpRender(gpPlayerClump);
}
