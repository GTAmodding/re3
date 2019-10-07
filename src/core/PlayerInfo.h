#pragma once

#include "Collision.h"

enum eWastedBustedState
{
	WBSTATE_PLAYING,
	WBSTATE_WASTED,
	WBSTATE_BUSTED,
	WBSTATE_FAILED_CRITICAL_MISSION,
};

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
	int32 m_nSwitchTaxiTime;
	bool m_bSwitchTaxi;
	int8 field_197;
	int8 field_198;
	int8 field_199;
	int32 m_nNextSexFrequencyUpdateTime;
	int32 m_nNextSexMoneyUpdateTime;
	int32 m_nSexFrequency;
	CCivilianPed *m_pHooker;
	int8 m_WBState; // eWastedBustedState
	int8 field_217;
	int8 field_218;
	int8 field_219;
	int32 m_nWBTime;
	bool m_bInRemoteMode;
	int8 field_225;
	int8 field_226;
	int8 field_227;
	uint32 m_nTimeLostRemoteCar;
	uint32 m_nTimeLastHealthLoss;
	uint32 m_nTimeLastArmourLoss;
	uint32 m_nTimeTankShotGun;
	int32 m_nUpsideDownCounter;
	int32 field_248;
	int16 m_nTrafficMultiplier;
	int8 field_254;
	int8 field_255;
	float m_fRoadDensity;
	int32 m_nPreviousTimeRewardedForExplosion;
	int32 m_nExplosionsSinceLastReward;
	int32 field_268;
	int32 field_272;
	bool m_bInfiniteSprint;
	bool m_bFastReload;
	bool m_bGetOutOfJailFree;
	bool m_bGetOutOfHospitalFree;
	char m_aSkinName[32];
	RwTexture *m_pSkinTexture;

	void MakePlayerSafe(bool);
	void LoadPlayerSkin();
	void DeletePlayerSkin();
	void AwardMoneyForExplosion(CVehicle *vehicle);	
	void SetPlayerSkin(char* skin);
	CVector& GetPos();
	void Process(void);
	void KillPlayer(void);
	void ArrestPlayer(void);
};

static_assert(sizeof(CPlayerInfo) == 0x13C, "CPlayerInfo: error");
