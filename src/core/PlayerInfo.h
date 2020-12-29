#pragma once

#include "Collision.h"

enum eWastedBustedState
{
	WBSTATE_PLAYING,
	WBSTATE_WASTED,
	WBSTATE_BUSTED,
	WBSTATE_FAILED_CRITICAL_MISSION,
};

enum eBustedAudioState
{
	BUSTEDAUDIO_NONE,
	BUSTEDAUDIO_LOADING,
	BUSTEDAUDIO_DONE
};

class CEntity;
class CPed;
class CVehicle;
class CPlayerPed;
class CCivilianPed;
class CPtrList;

class CPlayerInfo
{
public:
	CPlayerPed *m_pPed;
	CVehicle *m_pRemoteVehicle;
	CColModel m_ColModel;
	CVehicle *m_pVehicleEx;	// vehicle using the col model above
	char m_aPlayerName[70];
	int32 m_nMoney;
	int32 m_nVisibleMoney;
	int32 m_nCollectedPackages;
	int32 m_nTotalPackages;
	uint32 m_nLastBumpPlayerCarTimer;
	uint32 m_nUnusedTaxiTimer;
	bool m_bUnusedTaxiThing;
	uint32 m_nNextSexFrequencyUpdateTime;
	uint32 m_nNextSexMoneyUpdateTime;
	int32 m_nSexFrequency;
	CCivilianPed *m_pHooker;
	int8 m_WBState; // eWastedBustedState
	uint32 m_nWBTime;
	bool m_bInRemoteMode;
	bool field_D5;
	bool field_D6;
	uint32 m_nTimeLostRemoteCar;
	uint32 m_nTimeLastHealthLoss;
	uint32 m_nTimeLastArmourLoss;
	uint32 m_nTimeTankShotGun;
	int32 m_nUpsideDownCounter;
	int32 field_EC;
	int32 m_nTimeCarSpentOnTwoWheels;
	int32 m_nDistanceCarTravelledOnTwoWheels;
	int32 m_nTimeNotFullyOnGround;
	int32 m_nTimeSpentOnWheelie;
	float m_nDistanceTravelledOnWheelie;
	int32 m_nTimeSpentOnStoppie;
	float m_nDistanceTravelledOnStoppie;
	int32 m_nCancelWheelStuntTimer;
	int32 m_nLastTimeCarSpentOnTwoWheels;
	int32 m_nLastDistanceCarTravelledOnTwoWheels;
	int32 m_nLastTimeSpentOnWheelie;
	int32 m_nLastDistanceTravelledOnWheelie;
	int32 m_nLastTimeSpentOnStoppie;
	int32 m_nLastDistanceTravelledOnStoppie;
	int16 m_nTrafficMultiplier;
	int16 field_12A;
	float m_fRoadDensity;
	uint32 m_nPreviousTimeRewardedForExplosion;
	uint32 m_nExplosionsSinceLastReward;
	uint32 m_nHavocLevel;
	float m_fMediaAttention;
	bool m_bInfiniteSprint;
	bool m_bFastReload;
	bool m_bFireproof;
	uint8 m_nMaxHealth;
	uint8 m_nMaxArmour;
	bool m_bGetOutOfJailFree;
	bool m_bGetOutOfHospitalFree;
	bool m_bDriveByAllowed;
	uint8 m_nBustedAudioStatus;
	int16 m_nCurrentBustedAudio;
#ifdef GTA_PC
	char m_aSkinName[32];
	RwTexture *m_pSkinTexture;
#endif

	void MakePlayerSafe(bool);
	const CVector &GetPos();
	void Process(void);
	void KillPlayer(void);
	void ArrestPlayer(void);
	bool IsPlayerInRemoteMode(void);
	void PlayerFailedCriticalMission(void);
	void Clear(void);
	void BlowUpRCBuggy(bool);
	void CancelPlayerEnteringCars(CVehicle*);
	bool IsRestartingAfterDeath(void);
	bool IsRestartingAfterArrest(void);
	void EvaluateCarPosition(CEntity*, CPed*, float, float*, CVehicle**);
	void LoadPlayerInfo(uint8 *buf, uint32 size);
	void SavePlayerInfo(uint8 *buf, uint32* size);
	void FindClosestCarSectorList(CPtrList&, CPed*, float, float, float, float, float*, CVehicle**);

#ifdef GTA_PC
	void LoadPlayerSkin();
	void SetPlayerSkin(const char *skin);
	void DeletePlayerSkin();
#endif
};

CPlayerPed *FindPlayerPed(void);
CVehicle *FindPlayerVehicle(void);
CVehicle *FindPlayerTrain(void);
CEntity *FindPlayerEntity(void);
CVector FindPlayerCoors(void);
const CVector &FindPlayerSpeed(void);
const CVector &FindPlayerCentreOfWorld(int32 player);
const CVector &FindPlayerCentreOfWorld_NoSniperShift(void);
float FindPlayerHeading(void);