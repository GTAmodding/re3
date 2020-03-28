#pragma once
#include "ModelIndices.h"

class CVehicle;
class CPed;
enum eWeaponType;

enum
{
	KILLFRENZY_NONE,
	KILLFRENZY_ONGOING,
	KILLFRENZY_PASSED,
	KILLFRENZY_FAILED,
};

class CDarkel
{
private:
	static int32 &TimeLimit;
	static int32 &PreviousTime;
	static int32 &TimeOfFrenzyStart;
	static int32 &WeaponType;
	static int32 &AmmoInterruptedWeapon;
	static int32 &KillsNeeded;
	static int8 &InterruptedWeapon;
	static int8 &bStandardSoundAndMessages;
	static int8 &bNeedHeadShot;
	static int8 &bProperKillFrenzy;
	static uint16 &Status;
	static uint16 (&RegisteredKills)[NUM_DEFAULT_MODELS];
	static int32 &ModelToKill;
	static int32 &ModelToKill2;
	static int32 &ModelToKill3;
	static int32 &ModelToKill4;
	static wchar *pStartMessage;

public:
	static uint8 CalcFade(uint32 time, uint32 min, uint32 max);
	static void DrawMessages(void);
	static bool FrenzyOnGoing();
	static void Init();
	static uint16 QueryModelsKilledByPlayer(int32 modelId);
	static uint16 ReadStatus();
	static void RegisterCarBlownUpByPlayer(CVehicle *vehicle);
	static void RegisterKillByPlayer(CPed *victim, eWeaponType weapontype, bool headshot = false);
	static void RegisterKillNotByPlayer(CPed* victim, eWeaponType weapontype);
	static void ResetModelsKilledByPlayer();
	static void ResetOnPlayerDeath();
	static void StartFrenzy(eWeaponType weaponType, int32 time, uint16 kill, int32 modelId0, wchar *text, int32 modelId2, int32 modelId3, int32 modelId4, bool standardSound, bool needHeadShot);
	static void Update();

};
