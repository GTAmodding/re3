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
#include "CutsceneHead.h"
#include "RpAnimBlend.h"
#include "ModelIndices.h"
#include "TempColModels.h"

const struct {
	const char *szTrackName;
	int iTrackId;
} musicNameIdAssoc[] = {
	{ "JB",			STREAMED_SOUND_NEWS_INTRO },
	{ "BET",		STREAMED_SOUND_BANK_INTRO },
	{ "L1_LG",		STREAMED_SOUND_CUTSCENE_LUIGI1_LG },
	{ "L2_DSB",		STREAMED_SOUND_CUTSCENE_LUIGI2_DSB },
	{ "L3_DM",		STREAMED_SOUND_CUTSCENE_LUIGI3_DM },
	{ "L4_PAP",		STREAMED_SOUND_CUTSCENE_LUIGI4_PAP },
	{ "L5_TFB",		STREAMED_SOUND_CUTSCENE_LUIGI5_TFB },
	{ "J0_DM2",		STREAMED_SOUND_CUTSCENE_JOEY0_DM2 },
	{ "J1_LFL",		STREAMED_SOUND_CUTSCENE_JOEY1_LFL },
	{ "J2_KCL",		STREAMED_SOUND_CUTSCENE_JOEY2_KCL },
	{ "J3_VH",		STREAMED_SOUND_CUTSCENE_JOEY3_VH },
	{ "J4_ETH",		STREAMED_SOUND_CUTSCENE_JOEY4_ETH },
	{ "J5_DST",		STREAMED_SOUND_CUTSCENE_JOEY5_DST },
	{ "J6_TBJ",		STREAMED_SOUND_CUTSCENE_JOEY6_TBJ },
	{ "T1_TOL",		STREAMED_SOUND_CUTSCENE_TONI1_TOL },
	{ "T2_TPU",		STREAMED_SOUND_CUTSCENE_TONI2_TPU },
	{ "T3_MAS",		STREAMED_SOUND_CUTSCENE_TONI3_MAS },
	{ "T4_TAT",		STREAMED_SOUND_CUTSCENE_TONI4_TAT },
	{ "T5_BF",		STREAMED_SOUND_CUTSCENE_TONI5_BF },
	{ "S0_MAS",		STREAMED_SOUND_CUTSCENE_SAL0_MAS },
	{ "S1_PF",		STREAMED_SOUND_CUTSCENE_SAL1_PF },
	{ "S2_CTG",		STREAMED_SOUND_CUTSCENE_SAL2_CTG },
	{ "S3_RTC",		STREAMED_SOUND_CUTSCENE_SAL3_RTC },
	{ "S5_LRQ",		STREAMED_SOUND_CUTSCENE_SAL5_LRQ },
	{ "S4_BDBA",	STREAMED_SOUND_CUTSCENE_SAL4_BDBA },
	{ "S4_BDBB",	STREAMED_SOUND_CUTSCENE_SAL4_BDBB },
	{ "S2_CTG2",	STREAMED_SOUND_CUTSCENE_SAL2_CTG2 },
	{ "S4_BDBD",	STREAMED_SOUND_CUTSCENE_SAL4_BDBD },
	{ "S5_LRQB",	STREAMED_SOUND_CUTSCENE_SAL5_LRQB },
	{ "S5_LRQC",	STREAMED_SOUND_CUTSCENE_SAL5_LRQC },
	{ "A1_SS0",		STREAMED_SOUND_CUTSCENE_ASUKA_1_SSO },
	{ "A2_PP",		STREAMED_SOUND_CUTSCENE_ASUKA_2_PP },
	{ "A3_SS",		STREAMED_SOUND_CUTSCENE_ASUKA_3_SS },
	{ "A4_PDR",		STREAMED_SOUND_CUTSCENE_ASUKA_4_PDR },
	{ "A5_K2FT",	STREAMED_SOUND_CUTSCENE_ASUKA_5_K2FT},
	{ "K1_KBO",		STREAMED_SOUND_CUTSCENE_KENJI1_KBO },
	{ "K2_GIS",		STREAMED_SOUND_CUTSCENE_KENJI2_GIS },
	{ "K3_DS",		STREAMED_SOUND_CUTSCENE_KENJI3_DS },
	{ "K4_SHI",		STREAMED_SOUND_CUTSCENE_KENJI4_SHI },
	{ "K5_SD",		STREAMED_SOUND_CUTSCENE_KENJI5_SD },
	{ "R0_PDR2",	STREAMED_SOUND_CUTSCENE_RAY0_PDR2 },
	{ "R1_SW",		STREAMED_SOUND_CUTSCENE_RAY1_SW },
	{ "R2_AP",		STREAMED_SOUND_CUTSCENE_RAY2_AP },
	{ "R3_ED",		STREAMED_SOUND_CUTSCENE_RAY3_ED },
	{ "R4_GF",		STREAMED_SOUND_CUTSCENE_RAY4_GF },
	{ "R5_PB",		STREAMED_SOUND_CUTSCENE_RAY5_PB },
	{ "R6_MM",		STREAMED_SOUND_CUTSCENE_RAY6_MM },
	{ "D1_STOG",	STREAMED_SOUND_CUTSCENE_DONALD1_STOG },
	{ "D2_KK",		STREAMED_SOUND_CUTSCENE_DONALD2_KK },
	{ "D3_ADO",		STREAMED_SOUND_CUTSCENE_DONALD3_ADO },
	{ "D5_ES",		STREAMED_SOUND_CUTSCENE_DONALD5_ES },
	{ "D7_MLD",		STREAMED_SOUND_CUTSCENE_DONALD7_MLD },
	{ "D4_GTA",		STREAMED_SOUND_CUTSCENE_DONALD4_GTA },
	{ "D4_GTA2",	STREAMED_SOUND_CUTSCENE_DONALD4_GTA2 },
	{ "D6_STS",		STREAMED_SOUND_CUTSCENE_DONALD6_STS },
	{ "A6_BAIT",	STREAMED_SOUND_CUTSCENE_ASUKA6_BAIT },
	{ "A7_ETG",		STREAMED_SOUND_CUTSCENE_ASUKA7_ETG },
	{ "A8_PS",		STREAMED_SOUND_CUTSCENE_ASUKA8_PS },
	{ "A9_ASD",		STREAMED_SOUND_CUTSCENE_ASUKA9_ASD },
	{ "K4_SHI2",	STREAMED_SOUND_CUTSCENE_KENJI4_SHI2 },
	{ "C1_TEX",		STREAMED_SOUND_CUTSCENE_CATALINA1_TEX },
	{ "EL_PH1",		STREAMED_SOUND_CUTSCENE_ELBURRO1_PH1 },
	{ "EL_PH2",		STREAMED_SOUND_CUTSCENE_ELBURRO2_PH2 },
	{ "EL_PH3",		STREAMED_SOUND_CUTSCENE_ELBURRO3_PH3 },
	{ "EL_PH4",		STREAMED_SOUND_CUTSCENE_ELBURRO4_PH4 },
	{ "YD_PH1",		STREAMED_SOUND_CUTSCENE_YARDIE_PH1 },
	{ "YD_PH2",		STREAMED_SOUND_CUTSCENE_YARDIE_PH2 },
	{ "YD_PH3",		STREAMED_SOUND_CUTSCENE_YARDIE_PH3 },
	{ "YD_PH4",		STREAMED_SOUND_CUTSCENE_YARDIE_PH4 },
	{ "HD_PH1",		STREAMED_SOUND_CUTSCENE_HOODS_PH1 },
	{ "HD_PH2",		STREAMED_SOUND_CUTSCENE_HOODS_PH2 },
	{ "HD_PH3",		STREAMED_SOUND_CUTSCENE_HOODS_PH3 },
	{ "HD_PH4",		STREAMED_SOUND_CUTSCENE_HOODS_PH4 },
	{ "HD_PH5",		STREAMED_SOUND_CUTSCENE_HOODS_PH5 },
	{ "MT_PH1",		STREAMED_SOUND_CUTSCENE_MARTY_PH1 },
	{ "MT_PH2",		STREAMED_SOUND_CUTSCENE_MARTY_PH2 },
	{ "MT_PH3",		STREAMED_SOUND_CUTSCENE_MARTY_PH3 },
	{ "MT_PH4",		STREAMED_SOUND_CUTSCENE_MARTY_PH4 },
	{ NULL,			0 }
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
	file = CFileMgr::OpenFile("ANIM\\CUTS.IMG", "rb");

	// Load animations
	sprintf(gString, "%s.IFP", szCutsceneName);
	if (ms_pCutsceneDir->FindItem(gString, offset, size)) {
		CStreaming::MakeSpaceFor(size << 11);
		CStreaming::ImGonnaUseStreamingMemory();
		CFileMgr::Seek(file, offset << 11, SEEK_SET);
		CAnimManager::LoadAnimFile(file, false);
		ms_cutsceneAssociations.CreateAssociations(szCutsceneName);
		CStreaming::IHaveUsedStreamingMemory();
		ms_animLoaded = true;
	} else {
		ms_animLoaded = false;
	}

	// Load camera data
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
	//CTimer::Update(1000/30);

	pPlayerPed->m_pWanted->ClearQdCrimes();
	pPlayerPed->bIsVisible = false;
	pPlayerPed->m_fCurrentStamina = pPlayerPed->m_fMaxStamina;
	CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_CUTSCENE);
	CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(true);
}

void
CCutsceneMgr::SetHeadAnim(const char *animName, CObject *pObject)
{
	CCutsceneHead *pCutsceneHead = (CCutsceneHead*)pObject;
	char szAnim[CUTSCENENAMESIZE * 2];

	sprintf(szAnim, "%s_%s", ms_cutsceneName, animName);
	pCutsceneHead->PlayAnimation(szAnim);
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

	//CTimer::Update(1000/30);
	//CTimer::Update(1000/30);
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
	CCutsceneHead *pHead = new CCutsceneHead(pObject);
	pHead->SetModelIndex(modelId);
	CWorld::Add(pHead);
	ms_pCutsceneObjects[ms_numCutsceneObjs++] = pHead;
	return pHead;
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
	CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_CUTSCENE);
	CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(false);

	if (CGeneral::faststricmp(ms_cutsceneName, "end")) {
		DMAudio.StopCutSceneMusic();
		if (CGeneral::faststricmp(ms_cutsceneName, "bet"))
			DMAudio.ChangeMusicMode(MUSICMODE_GAME);
	}
	CTimer::Stop();
	CGame::DrasticTidyUpMemory(TheCamera.GetScreenFadeStatus() == FADE_2);
	//CTimer::Update(1000/30);
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

