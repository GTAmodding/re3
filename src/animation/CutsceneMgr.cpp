#include "common.h"

#include "General.h"
#include "CutsceneMgr.h"
#include "Directory.h"
#include "Camera.h"
#include "Streaming.h"
#include "FileMgr.h"
#include "main.h"
#include "AnimManager.h"
#include "AnimBlendAssociation.h"
#include "AnimBlendAssocGroup.h"
#include "AnimBlendClumpData.h"
#include "Pad.h"
#include "DMAudio.h"
#include "World.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "RpAnimBlend.h"
#include "ModelIndices.h"
#include "TempColModels.h"

const struct {
	const char *szTrackName;
	int iTrackId;
} musicNameIdAssoc[] = {
	{ "ASS_1", STREAMED_SOUND_CUTSCENE_ASS_1 },
	{ "ASS_2", STREAMED_SOUND_CUTSCENE_ASS_2 },
	{ "BANK_1", STREAMED_SOUND_CUTSCENE_BANK_1 },
	{ "BANK_2A", STREAMED_SOUND_CUTSCENE_BANK_2A },
	{ "BANK_2B", STREAMED_SOUND_CUTSCENE_BANK_2B },
	{ "BANK_3A", STREAMED_SOUND_CUTSCENE_BANK_3A },
	{ "BANK_3B", STREAMED_SOUND_CUTSCENE_BANK_3B },
	{ "BANK_4", STREAMED_SOUND_CUTSCENE_BANK_4 },
	{ "BIKE_1", STREAMED_SOUND_CUTSCENE_BIKE_1 },
	{ "BIKE_2", STREAMED_SOUND_CUTSCENE_BIKE_2 },
	{ "BIKE_3", STREAMED_SOUND_CUTSCENE_BIKE_3 },
	{ "BUD_1", STREAMED_SOUND_CUTSCENE_BUD_1 },
	{ "BUD_2", STREAMED_SOUND_CUTSCENE_BUD_2 },
	{ "BUD_3", STREAMED_SOUND_CUTSCENE_BUD_3 },
	{ "CAP_1", STREAMED_SOUND_CUTSCENE_CAP_1 },
	{ "CAR_1", STREAMED_SOUND_CUTSCENE_CAR_1 },
	{ "CNT_1A", STREAMED_SOUND_CUTSCENE_CNT_1A },
	{ "CNT_1B", STREAMED_SOUND_CUTSCENE_CNT_1B },
	{ "CNT_2", STREAMED_SOUND_CUTSCENE_CNT_2 },
	{ "COK_1", STREAMED_SOUND_CUTSCENE_COK_1 },
	{ "COK_2A", STREAMED_SOUND_CUTSCENE_COK_2A },
	{ "COK_2B", STREAMED_SOUND_CUTSCENE_COK_2B },
	{ "COK_3", STREAMED_SOUND_CUTSCENE_COK_3 },
	{ "COK_4A", STREAMED_SOUND_CUTSCENE_COK_4A },
	{ "COK_4A2", STREAMED_SOUND_CUTSCENE_COK_4A2 },
	{ "COK_4B", STREAMED_SOUND_CUTSCENE_COK_4B },
	{ "COL_1", STREAMED_SOUND_CUTSCENE_COL_1 },
	{ "COL_2", STREAMED_SOUND_CUTSCENE_COL_2 },
	{ "COL_3A", STREAMED_SOUND_CUTSCENE_COL_3A },
	{ "COL_4A", STREAMED_SOUND_CUTSCENE_COL_4A },
	{ "COL_5A", STREAMED_SOUND_CUTSCENE_COL_5A },
	{ "COL_5B", STREAMED_SOUND_CUTSCENE_COL_5B },
	{ "CUB_1", STREAMED_SOUND_CUTSCENE_CUB_1 },
	{ "CUB_2", STREAMED_SOUND_CUTSCENE_CUB_2 },
	{ "CUB_3", STREAMED_SOUND_CUTSCENE_CUB_3 },
	{ "CUB_4", STREAMED_SOUND_CUTSCENE_CUB_4 },
	{ "DRUG_1", STREAMED_SOUND_CUTSCENE_DRUG_1 },
	{ "FIN", STREAMED_SOUND_CUTSCENE_FIN },
	{ "FIN_2", STREAMED_SOUND_CUTSCENE_FIN2 },
	{ "FINALE", STREAMED_SOUND_CUTSCENE_FINALE },
	{ "HAT_1", STREAMED_SOUND_CUTSCENE_HAT_1 },
	{ "HAT_2", STREAMED_SOUND_CUTSCENE_HAT_2 },
	{ "HAT_3", STREAMED_SOUND_CUTSCENE_HAT_3 },
	{ "ICE_1", STREAMED_SOUND_CUTSCENE_ICE_1 },
	{ "INT_A", STREAMED_SOUND_CUTSCENE_INT_A },
	{ "INT_B", STREAMED_SOUND_CUTSCENE_INT_B },
	{ "INT_D", STREAMED_SOUND_CUTSCENE_INT_D },
	{ "INT_M", STREAMED_SOUND_CUTSCENE_INT_M },
	{ "LAW_1A", STREAMED_SOUND_CUTSCENE_LAW_1A },
	{ "LAW_1B", STREAMED_SOUND_CUTSCENE_LAW_1B },
	{ "LAW_2A", STREAMED_SOUND_CUTSCENE_LAW_2A },
	{ "LAW_2B", STREAMED_SOUND_CUTSCENE_LAW_2B },
	{ "LAW_2C", STREAMED_SOUND_CUTSCENE_LAW_2C },
	{ "LAW_3", STREAMED_SOUND_CUTSCENE_LAW_3 },
	{ "LAW_4", STREAMED_SOUND_CUTSCENE_LAW_4 },
	{ "PHIL_1", STREAMED_SOUND_CUTSCENE_PHIL_1 },
	{ "PHIL_2", STREAMED_SOUND_CUTSCENE_PHIL_2 },
	{ "PORN_1", STREAMED_SOUND_CUTSCENE_PORN_1 },
	{ "PORN_2", STREAMED_SOUND_CUTSCENE_PORN_2 },
	{ "PORN_3", STREAMED_SOUND_CUTSCENE_PORN_3 },
	{ "PORN_4", STREAMED_SOUND_CUTSCENE_PORN_4 },
	{ "RESC_1A", STREAMED_SOUND_CUTSCENE_RESC_1A },
	{ "ROK_1", STREAMED_SOUND_CUTSCENE_ROK_1 },
	{ "ROK_2", STREAMED_SOUND_CUTSCENE_ROK_2 },
	{ "ROK_3A", STREAMED_SOUND_CUTSCENE_ROK_3A },
	{ "STRIPA", STREAMED_SOUND_CUTSCENE_STRIPA },
	{ "TAX_1", STREAMED_SOUND_CUTSCENE_TAX_1 },
	{ "TEX_1", STREAMED_SOUND_CUTSCENE_TEX_1 },
	{ "TEX_2", STREAMED_SOUND_CUTSCENE_TEX_2 },
	{ "TEX_3", STREAMED_SOUND_CUTSCENE_TEX_3 },
	{ "GSPOT", STREAMED_SOUND_CUTSCENE_GLIGHT },
	{ "FIST", STREAMED_SOUND_CUTSCENE_FIST },
	{ "EL_PH1", STREAMED_SOUND_CUTSCENE_ELBURRO1_PH1 },
	{ "EL_PH2", STREAMED_SOUND_CUTSCENE_ELBURRO2_PH2 },
	{ NULL, 0 }
};

int
FindCutsceneAudioTrackId(const char *szCutsceneName)
{
	for (int i = 0; musicNameIdAssoc[i].szTrackName; i++) {
		if (!CGeneral::faststricmp(musicNameIdAssoc[i].szTrackName, szCutsceneName))
			return musicNameIdAssoc[i].iTrackId;
	}
	return -1;
}

bool CCutsceneMgr::ms_running;
bool CCutsceneMgr::ms_cutsceneProcessing;
CDirectory *CCutsceneMgr::ms_pCutsceneDir;
CCutsceneObject *CCutsceneMgr::ms_pCutsceneObjects[NUMCUTSCENEOBJECTS];
int32 CCutsceneMgr::ms_numCutsceneObjs;
bool CCutsceneMgr::ms_loaded;
bool CCutsceneMgr::ms_animLoaded;
bool CCutsceneMgr::ms_useLodMultiplier;
char CCutsceneMgr::ms_cutsceneName[CUTSCENENAMESIZE];
CAnimBlendAssocGroup CCutsceneMgr::ms_cutsceneAssociations;
CVector CCutsceneMgr::ms_cutsceneOffset;
float CCutsceneMgr::ms_cutsceneTimer;
uint32 CCutsceneMgr::ms_cutsceneLoadStatus;

RpAtomic *
CalculateBoundingSphereRadiusCB(RpAtomic *atomic, void *data)
{
	float radius = RpAtomicGetBoundingSphere(atomic)->radius;
	RwV3d center = RpAtomicGetBoundingSphere(atomic)->center;

	for (RwFrame *frame = RpAtomicGetFrame(atomic); RwFrameGetParent(frame); frame = RwFrameGetParent(frame))
		RwV3dTransformPoints(&center, &center, 1, RwFrameGetMatrix(frame));

	float size = RwV3dLength(&center) + radius;
	if (size > *(float *)data)
		*(float *)data = size;
	return atomic;
}

void
CCutsceneMgr::Initialise(void)
{
	ms_numCutsceneObjs = 0;
	ms_loaded = false;
	ms_running = false;
	ms_animLoaded = false;
	ms_cutsceneProcessing = false;
	ms_useLodMultiplier = false;

	ms_pCutsceneDir = new CDirectory(CUTSCENEDIRSIZE);
	ms_pCutsceneDir->ReadDirFile("ANIM\\CUTS.DIR");
}

void
CCutsceneMgr::Shutdown(void)
{
	delete ms_pCutsceneDir;
}

void
CCutsceneMgr::LoadCutsceneData(const char *szCutsceneName)
{
	int file;
	uint32 size;
	uint32 offset;
	CPlayerPed *pPlayerPed;

	ms_cutsceneProcessing = true;
	if (!strcasecmp(szCutsceneName, "jb"))
		ms_useLodMultiplier = true;
	CTimer::Stop();

	ms_pCutsceneDir->numEntries = 0;
	ms_pCutsceneDir->ReadDirFile("ANIM\\CUTS.DIR");

	CStreaming::RemoveUnusedModelsInLoadedList();
	CGame::DrasticTidyUpMemory(true);

	strcpy(ms_cutsceneName, szCutsceneName);

	RwStream *stream;
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, "ANIM\\CUTS.IMG");
	assert(stream);

	// Load animations
	sprintf(gString, "%s.IFP", szCutsceneName);
	if (ms_pCutsceneDir->FindItem(gString, offset, size)) {
		CStreaming::MakeSpaceFor(size << 11);
		CStreaming::ImGonnaUseStreamingMemory();
		RwStreamSkip(stream,  offset << 11);
		CAnimManager::LoadAnimFile(stream, false);
		ms_cutsceneAssociations.CreateAssociations(szCutsceneName);
		CStreaming::IHaveUsedStreamingMemory();
		ms_animLoaded = true;
	} else {
		ms_animLoaded = false;
	}
	RwStreamClose(stream, nil);

	// Load camera data
	file = CFileMgr::OpenFile("ANIM\\CUTS.IMG", "rb");
	sprintf(gString, "%s.DAT", szCutsceneName);
	if (ms_pCutsceneDir->FindItem(gString, offset, size)) {
		CFileMgr::Seek(file, offset << 11, SEEK_SET);
		TheCamera.LoadPathSplines(file);
	}

	CFileMgr::CloseFile(file);

	if (CGeneral::faststricmp(ms_cutsceneName, "end")) {
		DMAudio.ChangeMusicMode(MUSICMODE_CUTSCENE);
		int trackId = FindCutsceneAudioTrackId(szCutsceneName);
		if (trackId != -1) {
			printf("Start preload audio %s\n", szCutsceneName);
			DMAudio.PreloadCutSceneMusic(trackId);
			printf("End preload audio %s\n", szCutsceneName);
		}
	}

	ms_cutsceneTimer = 0.0f;
	ms_loaded = true;
	ms_cutsceneOffset = CVector(0.0f, 0.0f, 0.0f);

	pPlayerPed = FindPlayerPed();
	CTimer::Update();

	pPlayerPed->m_pWanted->ClearQdCrimes();
	pPlayerPed->bIsVisible = false;
	pPlayerPed->m_fCurrentStamina = pPlayerPed->m_fMaxStamina;
	CPad::GetPad(0)->DisablePlayerControls |= PLAYERCONTROL_DISABLED_80;
	CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(true);
}

void
CCutsceneMgr::FinishCutscene()
{
	CCutsceneMgr::ms_cutsceneTimer = TheCamera.GetCutSceneFinishTime() * 0.001f;
	TheCamera.FinishCutscene();

	FindPlayerPed()->bIsVisible = true;
	CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(false);
}

void
CCutsceneMgr::SetupCutsceneToStart(void)
{
	TheCamera.SetCamCutSceneOffSet(ms_cutsceneOffset);
	TheCamera.TakeControlWithSpline(JUMP_CUT);
	TheCamera.SetWideScreenOn();

	ms_cutsceneOffset.z++;

	for (int i = ms_numCutsceneObjs - 1; i >= 0; i--) {
		assert(RwObjectGetType(ms_pCutsceneObjects[i]->m_rwObject) == rpCLUMP);
		if (CAnimBlendAssociation *pAnimBlendAssoc = RpAnimBlendClumpGetFirstAssociation((RpClump*)ms_pCutsceneObjects[i]->m_rwObject)) {
			assert(pAnimBlendAssoc->hierarchy->sequences[0].HasTranslation());
			ms_pCutsceneObjects[i]->SetPosition(ms_cutsceneOffset + ((KeyFrameTrans*)pAnimBlendAssoc->hierarchy->sequences[0].GetKeyFrame(0))->translation);
			CWorld::Add(ms_pCutsceneObjects[i]);
			pAnimBlendAssoc->SetRun();
		} else {
			ms_pCutsceneObjects[i]->SetPosition(ms_cutsceneOffset);
		}
	}

	CTimer::Update();
	CTimer::Update();
	ms_running = true;
	ms_cutsceneTimer = 0.0f;
}

void
CCutsceneMgr::SetCutsceneAnim(const char *animName, CObject *pObject)
{
	CAnimBlendAssociation *pNewAnim;
	CAnimBlendClumpData *pAnimBlendClumpData;

	assert(RwObjectGetType(pObject->m_rwObject) == rpCLUMP);
	RpAnimBlendClumpRemoveAllAssociations((RpClump*)pObject->m_rwObject);

	pNewAnim = ms_cutsceneAssociations.CopyAnimation(animName);
	pNewAnim->SetCurrentTime(0.0f);
	pNewAnim->flags |= ASSOC_HAS_TRANSLATION;
	pNewAnim->flags &= ~ASSOC_RUNNING;

	pAnimBlendClumpData = *RPANIMBLENDCLUMPDATA(pObject->m_rwObject);
	pAnimBlendClumpData->link.Prepend(&pNewAnim->link);
}

CCutsceneHead *
CCutsceneMgr::AddCutsceneHead(CObject *pObject, int modelId)
{
	return nil;
}

CCutsceneObject *
CCutsceneMgr::CreateCutsceneObject(int modelId)
{
	CBaseModelInfo *pModelInfo;
	CColModel *pColModel;
	float radius;
	RpClump *clump;
	CCutsceneObject *pCutsceneObject;

	if (modelId >= MI_CUTOBJ01 && modelId <= MI_CUTOBJ05) {
		pModelInfo = CModelInfo::GetModelInfo(modelId);
		pColModel = &CTempColModels::ms_colModelCutObj[modelId - MI_CUTOBJ01];
		radius = 0.0f;

		pModelInfo->SetColModel(pColModel);
		clump = (RpClump*)pModelInfo->GetRwObject();
		assert(RwObjectGetType((RwObject*)clump) == rpCLUMP);
		RpClumpForAllAtomics(clump, CalculateBoundingSphereRadiusCB, &radius);

		pColModel->boundingSphere.radius = radius;
		pColModel->boundingBox.min = CVector(-radius, -radius, -radius);
		pColModel->boundingBox.max = CVector(radius, radius, radius);
	}

	pCutsceneObject = new CCutsceneObject();
	pCutsceneObject->SetModelIndex(modelId);
	ms_pCutsceneObjects[ms_numCutsceneObjs++] = pCutsceneObject;
	return pCutsceneObject;
}

void
CCutsceneMgr::DeleteCutsceneData(void)
{
	if (!ms_loaded) return;

	ms_cutsceneProcessing = false;
	ms_useLodMultiplier = false;

	for (--ms_numCutsceneObjs; ms_numCutsceneObjs >= 0; ms_numCutsceneObjs--) {
		CWorld::Remove(ms_pCutsceneObjects[ms_numCutsceneObjs]);
		ms_pCutsceneObjects[ms_numCutsceneObjs]->DeleteRwObject();
		delete ms_pCutsceneObjects[ms_numCutsceneObjs];
		ms_pCutsceneObjects[ms_numCutsceneObjs] = nil;
	}
	ms_numCutsceneObjs = 0;

	if (ms_animLoaded)
		CAnimManager::RemoveLastAnimFile();

	ms_animLoaded = false;
	TheCamera.RestoreWithJumpCut();
	TheCamera.SetWideScreenOff();
	ms_running = false;
	ms_loaded = false;

	FindPlayerPed()->bIsVisible = true;
	CPad::GetPad(0)->DisablePlayerControls &= ~PLAYERCONTROL_DISABLED_80;
	CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(false);

	if (CGeneral::faststricmp(ms_cutsceneName, "end")) {
		DMAudio.StopCutSceneMusic();
		if (CGeneral::faststricmp(ms_cutsceneName, "bet"))
			DMAudio.ChangeMusicMode(MUSICMODE_GAME);
	}
	CTimer::Stop();
	CGame::DrasticTidyUpMemory(TheCamera.GetScreenFadeStatus() == 2);
	CTimer::Update();
}

void
CCutsceneMgr::Update(void)
{
	enum {
		CUTSCENE_LOADING_0 = 0,
		CUTSCENE_LOADING_AUDIO,
		CUTSCENE_LOADING_2,
		CUTSCENE_LOADING_3,
		CUTSCENE_LOADING_4
	};

	switch (ms_cutsceneLoadStatus) {
	case CUTSCENE_LOADING_AUDIO:
		SetupCutsceneToStart();
		if (CGeneral::faststricmp(ms_cutsceneName, "end"))
			DMAudio.PlayPreloadedCutSceneMusic();
		ms_cutsceneLoadStatus++;
		break;
	case CUTSCENE_LOADING_2:
	case CUTSCENE_LOADING_3:
		ms_cutsceneLoadStatus++;
		break;
	case CUTSCENE_LOADING_4:
		ms_cutsceneLoadStatus = CUTSCENE_LOADING_0;
		break;
	default:
		break;
	}

	if (!ms_running) return;

	ms_cutsceneTimer += CTimer::GetTimeStepNonClippedInSeconds();
	if (CGeneral::faststricmp(ms_cutsceneName, "end") && TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FLYBY && ms_cutsceneLoadStatus == CUTSCENE_LOADING_0) {
		if (CPad::GetPad(0)->GetCrossJustDown()
			|| (CGame::playingIntro && CPad::GetPad(0)->GetStartJustDown())
			|| CPad::GetPad(0)->GetLeftMouseJustDown()
			|| CPad::GetPad(0)->GetEnterJustDown()
			|| CPad::GetPad(0)->GetCharJustDown(' '))
			FinishCutscene();
	}
}

bool CCutsceneMgr::HasCutsceneFinished(void) { return TheCamera.GetPositionAlongSpline() == 1.0f; }

