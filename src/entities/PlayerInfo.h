#pragma once
#include "Automobile.h"
#include "PlayerPed.h"

enum eWastedBustedState : __int8 {
	WBSTATE_PLAYING = 0x0,
	WBSTATE_WASTED = 0x1,
	WBSTATE_BUSTED = 0x2,
	WBSTATE_FAILED_CRITICAL_MISSION = 0x3,
};

struct CCivilianPed {

};

class CPlayerInfo {
public:
	CPlayerPed *m_pPed;
	CVehicle *m_pRemoteVehicle;
	CColModel m_ColModel;
	CVehicle *m_pVehicleEx;
	char m_aszPlayerName[70];
private:
	char _pad0[2];
public:
	int m_nMoney;
	int m_nVisibleMoney;
	int m_nCollectedPackages;
	int m_nTotalPackages;
	int field_188;
	int m_nSwitchTaxiTime;
	char m_bSwitchTaxi;
	char field_197;
	char field_198;
	char field_199;
	int m_nNextSexFrequencyUpdateTime;
	int m_nNextSexMoneyUpdateTime;
	int m_nSexFrequency;
	CCivilianPed *m_pHooker;
	eWastedBustedState m_bWBState;
	char field_217;
	char field_218;
	char field_219;
	int m_nWBTime;
	char m_bInRemoteMode;
	char field_225;
	char field_226;
	char field_227;
	int m_nTimeLostRemoteCar;
	int m_nTimeLastHealthLoss;
	int m_nTimeLastArmourLoss;
	int field_240;
	int m_nUpsideDownCounter;
	int field_248;
	__int16 m_nTrafficMultiplier;
	char field_254;
	char field_255;
	float m_fRoadDensity;
	int m_nPreviousTimeRewardedForExplosion;
	int m_nExplosionsSinceLastReward;
	int field_268;
	int field_272;
	char m_bInfiniteSprint;
	char m_bFastReload;
	char m_bGetOutOfJailFree;
	char m_bGetOutOfHospitalFree;
	unsigned __int8 m_aSkinName[32];
	RwTexture *m_pSkinTexture;
};

static_assert(sizeof(CPlayerInfo) == 0x13C, "CPlayerPed: error");
