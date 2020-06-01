#pragma once

class CAutomobile;

class CGameLogic
{
public:
	enum {
		SHORTCUT_NONE = 0,
		SHORTCUT_INIT,
		SHORTCUT_IDLE,
		SHORTCUT_GETTING_IN,
		SHORTCUT_TRANSITION,
		SHORTCUT_ARRIVING,
		SHORTCUT_GETTING_OUT
	};

	static void InitAtStartOfGame();
	static void PassTime(uint32 time);
	static void SortOutStreamingAndMemory(const CVector &pos);
	static void Update();
	static void RestorePlayerStuffDuringResurrection(class CPlayerPed *pPlayerPed, CVector pos, float angle);

	static void ClearShortCut();
	static void SetUpShortCut(CVector, float, CVector, float);
	static void AbandonShortCutIfTaxiHasBeenMessedWith();
	static void AbandonShortCutIfPlayerMilesAway();
	static void UpdateShortCut();
	static void AddShortCutPointAfterDeath(CVector, float);
	static void AddShortCutDropOffPointForMission(CVector, float);
	static void RemoveShortCutDropOffPointForMission();
	static void AfterDeathArrestSetUpShortCutTaxi();

	static void Save(uint8*, uint32*);
	static void Load(uint8*, uint32);

	static uint8 ActivePlayers;
	static uint8 ShortCutState;
	static CAutomobile* pShortCutTaxi;
	static uint32 NumAfterDeathStartPoints;
	static CVector ShortCutStart;
	static float ShortCutStartOrientation;
	static CVector ShortCutDestination;
	static float ShortCutDestinationOrientation;
	static uint32 ShortCutTimer;
	static CVector AfterDeathStartPoints[NUM_SHORTCUT_START_POINTS];
	static float AfterDeathStartPointOrientation[NUM_SHORTCUT_START_POINTS];
	static CVector ShortCutDropOffForMission;
	static float ShortCutDropOffOrientationForMission;
	static bool MissionDropOffReadyToBeUsed;
};