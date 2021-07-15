#pragma once
#include "CutsceneObject.h"

#define CUTSCENENAMESIZE 8

class CDirectory;
class CAnimBlendAssocGroup;
class CCutsceneHead;

class CCutsceneMgr
{
	static bool ms_running;
	static CCutsceneObject *ms_pCutsceneObjects[NUMCUTSCENEOBJECTS];
	
	static int32 ms_numCutsceneObjs;
	static bool ms_loaded;
	static bool ms_animLoaded;
	static bool ms_useLodMultiplier;
	static bool ms_blurSetting;

	static char ms_cutsceneName[CUTSCENENAMESIZE];
	static CAnimBlendAssocGroup ms_cutsceneAssociations;
	static CVector ms_cutsceneOffset;
	static float ms_cutsceneTimer;
	static bool ms_cutsceneProcessing;
public:
	static CDirectory *ms_pCutsceneDir;
	static uint32 ms_cutsceneLoadStatus;

	static void StartCutsceneProcessing() { ms_cutsceneProcessing = true; }
	static bool IsRunning(void) { return ms_running; }
	static bool HasLoaded(void) { return ms_loaded; }
	static bool IsCutsceneProcessing(void) { return ms_cutsceneProcessing; }
	static bool UseLodMultiplier(void) { return ms_useLodMultiplier; }
	static CCutsceneObject* GetCutsceneObject(int id) { return ms_pCutsceneObjects[id]; }
	static int GetCutsceneTimeInMilleseconds(void) { return 1000.0f * ms_cutsceneTimer; }
	static char *GetCutsceneName(void) { return ms_cutsceneName; }
	static void SetCutsceneOffset(const CVector& vec) { ms_cutsceneOffset = vec; }
	static bool HasCutsceneFinished(void);

	static void Initialise(void);
	static void Shutdown(void);
	static void LoadCutsceneData(const char *szCutsceneName);
	static void FinishCutscene(void);
	static void SetHeadAnim(const char *animName, CObject *pObject);
	static void SetupCutsceneToStart(void);
	static void SetCutsceneAnim(const char *animName, CObject *pObject);
	static CCutsceneHead *AddCutsceneHead(CObject *pObject, int modelId);
	static CCutsceneObject *CreateCutsceneObject(int modelId);
	static void DeleteCutsceneData(void);
	static void Update(void);
};
