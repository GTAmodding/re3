#pragma once

#include "ColModel.h"

enum eWastedBustedState
{
	WBSTATE_PLAYING,
	WBSTATE_WASTED,
	WBSTATE_BUSTED,
	WBSTATE_FAILED_CRITICAL_MISSION,
};

class CEntity;
class CPed;
class CVehicle;
class CPlayerPed;
class CCivilianPed;

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
	uint32 m_nTimeLostRemoteCar;
	uint32 m_nTimeLastHealthLoss;
	uint32 m_nTimeLastArmourLoss;
	uint32 m_nTimeTankShotGun;
	int32 m_nUpsideDownCounter;
	int32 field_248;
	int16 m_nTrafficMultiplier;
	float m_fRoadDensity;
	uint32 m_nPreviousTimeRewardedForExplosion;
	int32 m_nExplosionsSinceLastReward;
	int32 field_268;
	int32 field_272;
	bool m_bInfiniteSprint;
	bool m_bFastReload;
	bool m_bGetOutOfJailFree;
	bool m_bGetOutOfHospitalFree;
#ifdef GTA_PC
	char m_aSkinName[32];
	RwTexture *m_pSkinTexture;
#endif

	void MakePlayerSafe(bool);
	void AwardMoneyForExplosion(CVehicle *vehicle);	
	const CVector &GetPos();
	void Process(void);
	void KillPlayer(void);
	void ArrestPlayer(void);
	bool IsPlayerInRemoteMode(void);
	void PlayerFailedCriticalMission(void);
	void Clear(void);
	void BlowUpRCBuggy(void);
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

VALIDATE_SIZE(CPlayerInfo, 0x13C);
