#pragma once
#include "common.h"
#include "PedType.h"
#include "Text.h"
#include "Sprite2d.h"

class CEntity;
class CBuilding;
class CVehicle;
class CPed;
class CObject;
class CPlayerInfo;

struct CScriptRectangle 
{
	int8 m_bIsUsed;
	bool m_bBeforeFade;
	int16 m_nTextureId;
	CRect m_sRect;
	CRGBA m_sColor;
};

static_assert(sizeof(CScriptRectangle) == 0x18, "Script.h: error");

enum {
	SCRIPT_TEXT_MAX_LENGTH = 500
};

struct CTextLine 
{
	float m_fScaleX;
	float m_fScaleY;
	CRGBA m_sColor;
	bool m_bJustify;
	bool m_bCentered;
	bool m_bBackground;
	bool m_bBackgroundOnly;
	float m_fWrapX;
	float m_fCenterSize;
	CRGBA m_sBackgroundColor;
	bool m_bTextProportional;
	bool m_bTextBeforeFade;
	bool m_bRightJustify;
	int32 m_nFont;
	float m_fAtX;
	float m_fAtY;
	wchar m_Text[SCRIPT_TEXT_MAX_LENGTH];

	void Reset();
};

static_assert(sizeof(CTextLine) == 0x414, "Script.h: error");

struct CScriptSphere
{
	bool m_bInUse;
	uint16 m_Index;
	uint32 m_Id;
	CVector m_vecCenter;
	float m_fRadius;
};

enum {
	MAX_STACK_DEPTH = 6,
	NUM_LOCAL_VARS = 16,
	NUM_TIMERS = 2
};

class CRunningScript
{
	CRunningScript *next;
	CRunningScript *prev;
	char m_abScriptName[8];
	uint32 m_nIp;
	uint32 m_anStack[MAX_STACK_DEPTH];
	uint16 m_nStackPointer;
	int32 m_anLocalVariables[NUM_LOCAL_VARS + NUM_TIMERS];
	bool m_bCondResult;
	bool m_bIsMissionScript;
	bool m_bSkipWakeTime;
	uint32 m_nWakeTime;
	uint16 m_nAndOrState;
	bool m_bNotFlag;
	bool m_bDeatharrestEnabled;
	bool m_bDeatharrestExecuted;
	bool m_bMissionFlag;

public:
	void SetIP(uint32 ip) { m_nIp = ip; }
	CRunningScript* GetNext() { return next; }
	void UpdateTimers(float timeStep){
		m_anLocalVariables[NUM_LOCAL_VARS] += timeStep;
		m_anLocalVariables[NUM_LOCAL_VARS + 1] += timeStep;
	}

	bool ThisIsAValidRandomPed(uint32 pedtype){
		switch (pedtype){
		case PEDTYPE_CIVMALE:
		case PEDTYPE_CIVFEMALE:
		case PEDTYPE_GANG1:
		case PEDTYPE_GANG2:
		case PEDTYPE_GANG3:
		case PEDTYPE_GANG4:
		case PEDTYPE_GANG5:
		case PEDTYPE_GANG6:
		case PEDTYPE_GANG7:
		case PEDTYPE_GANG8:
		case PEDTYPE_GANG9:
		case PEDTYPE_CRIMINAL:
		case PEDTYPE_PROSTITUTE:
			return true;
		default:
			return false;
		}
	}

	float LimitAngleOnCircle(float angle) { return angle < 0.0f ? angle + 360.0f : angle; }

	void CollectParameters(uint32*, int16);
	int32 CollectNextParameterWithoutIncreasingPC(uint32);
	int32* GetPointerToScriptVariable(uint32*, int16);
	void StoreParameters(uint32*, int16);
	void Init();
	void RemoveScriptFromList(CRunningScript**);
	void AddScriptToList(CRunningScript**);
	void Process();
	int8 ProcessOneCommand();
	void DoDeatharrestCheck();
	int8 ProcessCommandsFrom0To99(int32);
	int8 ProcessCommandsFrom100To199(int32);
	int8 ProcessCommandsFrom200To299(int32);
	int8 ProcessCommandsFrom300To399(int32);
	int8 ProcessCommandsFrom400To499(int32);
	int8 ProcessCommandsFrom500To599(int32);
	int8 ProcessCommandsFrom600To699(int32);
	int8 ProcessCommandsFrom700To799(int32);
	int8 ProcessCommandsFrom800To899(int32);
	int8 ProcessCommandsFrom900To999(int32);
	int8 ProcessCommandsFrom1000To1099(int32);
	int8 ProcessCommandsFrom1100To1199(int32);
	void UpdateCompareFlag(bool);
	int16 GetPadState(uint16, uint16);
	void LocatePlayerCommand(int32, uint32*);
	void LocatePlayerCharCommand(int32, uint32*);
	void LocatePlayerCarCommand(int32, uint32*);
	void LocateCharCommand(int32, uint32*);
	void LocateCharCharCommand(int32, uint32*);
	void LocateCharCarCommand(int32, uint32*);
	void LocateCharObjectCommand(int32, uint32*);
	void LocateCarCommand(int32, uint32*);
	void LocateSniperBulletCommand(int32, uint32*);
	void PlayerInAreaCheckCommand(int32, uint32*);
	void PlayerInAngledAreaCheckCommand(int32, uint32*);
	void CharInAreaCheckCommand(int32, uint32*);
	void CarInAreaCheckCommand(int32, uint32*);
private:
	enum {
		ANDOR_NONE = 0,
		ANDS_1 = 1,
		ANDS_2,
		ANDS_3,
		ANDS_4,
		ANDS_5,
		ANDS_6,
		ANDS_7,
		ANDS_8,
		ORS_1 = 21,
		ORS_2,
		ORS_3,
		ORS_4,
		ORS_5,
		ORS_6,
		ORS_7,
		ORS_8
	};
};

enum {
	CLEANUP_UNUSED = 0,
	CLEANUP_CAR,
	CLEANUP_CHAR,
	CLEANUP_OBJECT
};

struct CMissionCleanupEntity
{
	uint8 type;
	int32 id;
};

enum {
	MAX_CLEANUP = 50,
	MAX_UPSIDEDOWN_CAR_CHECKS = 6,
	MAX_STUCK_CAR_CHECKS = 6
};

class CMissionCleanup
{
	CMissionCleanupEntity m_sEntities[MAX_CLEANUP];
	uint8 m_nCount;

public:
	CMissionCleanup();

	void Init();
	CMissionCleanupEntity* FindFree();
	void AddEntityToList(int32, uint8);
	void RemoveEntityFromList(int32, uint8);
	void Process();
};

struct CUpsideDownCarCheckEntry
{
	int32 m_nVehicleIndex;
	uint32 m_nUpsideDownTimer;
};

class CUpsideDownCarCheck
{
	CUpsideDownCarCheckEntry m_sCars[MAX_UPSIDEDOWN_CAR_CHECKS];

public:
	void Init();
	bool IsCarUpsideDown(int32);
	void UpdateTimers();
	bool AreAnyCarsUpsideDown();
	void AddCarToCheck(int32);
	void RemoveCarFromCheck(int32);
	bool HasCarBeenUpsideDownForAWhile(int32);
};

struct CStuckCarCheckEntry
{
	int32 m_nVehicleIndex;
	CVector m_vecPos;
	int32 m_nLastCheck;
	float m_fRadius;
	uint32 m_nStuckTime;
	bool m_bStuck;

	inline void Reset();
};

class CStuckCarCheck
{
	CStuckCarCheckEntry m_sCars[MAX_STUCK_CAR_CHECKS];

public:
	void Init();
	void Process();
	void AddCarToCheck(int32, float, uint32);
	void RemoveCarFromCheck(int32);
	bool HasCarBeenStuckForAWhile(int32);
};

enum {
	ARGUMENT_END = 0,
	ARGUMENT_INT32,
	ARGUMENT_GLOBALVAR,
	ARGUMENT_LOCALVAR,
	ARGUMENT_INT8,
	ARGUMENT_INT16,
	ARGUMENT_FLOAT
};

struct tCollectiveData
{
	int32 index;
	uint32 unk_data;
};

enum {
	USED_OBJECT_NAME_LENGTH = 24
};

struct tUsedObject
{
	char name[USED_OBJECT_NAME_LENGTH];
	int32 index;
};

struct tBuildingSwap
{
	CBuilding* m_pBuilding;
	int32 m_nNewModel;
	int32 m_nOldModel;
};


enum {
	VAR_LOCAL = 1,
	VAR_GLOBAL = 2,
};

enum {
	SIZE_MAIN_SCRIPT = 128 * 1024,
	SIZE_MISSION_SCRIPT = 32 * 1024,
	SIZE_SCRIPT_SPACE = SIZE_MAIN_SCRIPT + SIZE_MISSION_SCRIPT
};

enum {
	MAX_NUM_SCRIPTS = 128,
	MAX_NUM_CONTACTS = 16,
	MAX_NUM_INTRO_TEXT_LINES = 2,
	MAX_NUM_INTRO_RECTANGLES = 16,
	MAX_NUM_SCRIPT_SRPITES = 16,
	MAX_NUM_SCRIPT_SPHERES = 16,
	MAX_NUM_COLLECTIVES = 32,
	MAX_NUM_USED_OBJECTS = 200,
	MAX_NUM_MISSION_SCRIPTS = 120,
	MAX_NUM_BUILDING_SWAPS = 25,
	MAX_NUM_INVISIBILITY_SETTINGS = 20
};

class CTheScripts
{
public:
	static uint8(&ScriptSpace)[SIZE_SCRIPT_SPACE];
	static CRunningScript(&ScriptsArray)[MAX_NUM_SCRIPTS];
	static int32(&BaseBriefIdForContact)[MAX_NUM_CONTACTS];
	static int32(&OnAMissionForContactFlag)[MAX_NUM_CONTACTS];
	static CTextLine(&IntroTextLines)[MAX_NUM_INTRO_TEXT_LINES];
	static CScriptRectangle(&IntroRectangles)[MAX_NUM_INTRO_RECTANGLES];
	static CSprite2d(&ScriptSprites)[MAX_NUM_SCRIPT_SRPITES];
	static CScriptSphere(&ScriptSphereArray)[MAX_NUM_SCRIPT_SPHERES];
	static tCollectiveData(&CollectiveArray)[MAX_NUM_COLLECTIVES];
	static tUsedObject(&UsedObjectArray)[MAX_NUM_USED_OBJECTS];
	static int32(&MultiScriptArray)[MAX_NUM_MISSION_SCRIPTS];
	static tBuildingSwap(&BuildingSwapArray)[MAX_NUM_BUILDING_SWAPS];
	static CEntity*(&InvisibilitySettingArray)[MAX_NUM_INVISIBILITY_SETTINGS];
	static bool &DbgFlag;
	static uint32 &OnAMissionFlag;
	static CMissionCleanup &MissionCleanup;
	static CStuckCarCheck &StuckCars;
	static CUpsideDownCarCheck &UpsideDownCars;
	static int32 &StoreVehicleIndex;
	static bool &StoreVehicleWasRandom;
	static CRunningScript *&pIdleScripts;
	static CRunningScript *&pActiveScripts;
	static uint32 &NextFreeCollectiveIndex;
	static int32 &LastRandomPedId;
	static uint16 &NumberOfUsedObjects;
	static bool &bAlreadyRunningAMissionScript;
	static bool &bUsingAMultiScriptFile;
	static uint16 &NumberOfMissionScripts;
	static uint32 &LargestMissionScriptSize;
	static uint32 &MainScriptSize;
	static uint8 &FailCurrentMission;
	static uint8 &CountdownToMakePlayerUnsafe;
	static uint8 &DelayMakingPlayerUnsafeThisTime;
	static uint16 &NumScriptDebugLines;
	static uint16 &NumberOfIntroRectanglesThisFrame;
	static uint16 &NumberOfIntroTextLinesThisFrame;
	static bool &UseTextCommands;
	static uint16 &CommandsExecuted;
	static uint16 &ScriptsUpdated;
public:
	static void ScriptDebugLine3D(float x1, float y1, float z1, float x2, float y2, float z2, int col, int col2);
	static void CleanUpThisVehicle(CVehicle*);
	static void CleanUpThisPed(CPed*);
	static void CleanUpThisObject(CObject*);
	static void Init();
	static CRunningScript* StartNewScript(uint32);
	static void Process();
	static CRunningScript* StartTestScript();
	static bool IsPlayerOnAMission();
	static bool IsPedStopped(CPed*);
	static bool IsPlayerStopped(CPlayerInfo*);
	static bool IsVehicleStopped(CVehicle*);

	static void ReadObjectNamesFromScript();
	static void UpdateObjectIndices();
	static void ReadMultiScriptFileOffsetsFromScript();
	static void DrawScriptSpheres();
	static void ClearSpaceForMissionEntity(const CVector&, CEntity*);
	static void HighlightImportantArea(uint32, float, float, float, float, float);
	static void DrawDebugSquare(float, float, float, float);
	static void DrawDebugCube(float, float, float, float, float, float);
	static void AddToInvisibilitySwapArray(CEntity*, bool);

	static int32 Read4BytesFromScript(uint32* pIp){
		int32 retval = 0;
		for (int i = 0; i < 4; i++){
			retval |= ScriptSpace[(*pIp)++] << (8 * i);
		}
		return retval;
	}
	static int16 Read2BytesFromScript(uint32* pIp){
		int16 retval = 0;
		for (int i = 0; i < 2; i++){
			retval |= ScriptSpace[(*pIp)++] << (8 * i);
		}
		return retval;
	}
	static int8 Read1ByteFromScript(uint32* pIp){
		int8 retval = 0;
		for (int i = 0; i < 1; i++){
			retval |= ScriptSpace[(*pIp)++] << (8 * i);
		}
		return retval;
	}
	static float ReadFloatFromScript(uint32* pIp){
		return Read2BytesFromScript(pIp) / 16.0f;
	}
	static void ReadTextLabelFromScript(uint32* pIp, char* buf){
		strncpy(buf, (const char*)&ScriptSpace[*pIp], 8);
	}
	static wchar* GetTextByKeyFromScript(uint32* pIp) {
		wchar* text = TheText.Get((const char*)&ScriptSpace[*pIp]);
		*pIp += 8;
		return text;
	}
};
