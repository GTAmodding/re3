#include "common.h"
#include "patcher.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Frontend.h"
#include "Vehicle.h"
#include "PlayerSkin.h"
#include "Darkel.h"
#include "Messages.h"
#include "Text.h"
#include "Stats.h"

WRAPPER void CPlayerInfo::MakePlayerSafe(bool) { EAXJMP(0x4A1400); }
WRAPPER void CPlayerInfo::AwardMoneyForExplosion(CVehicle *vehicle) { EAXJMP(0x4A15F0); }
WRAPPER void CPlayerInfo::Process(void) { EAXJMP(0x49FD30); }

void
CPlayerInfo::SetPlayerSkin(char *skin)
{
	strncpy(m_aSkinName, skin, 32);
	LoadPlayerSkin();
}

CVector&
CPlayerInfo::GetPos()
{
	if (m_pPed->bInVehicle && m_pPed->m_pMyVehicle)
		return m_pPed->m_pMyVehicle->GetPosition();
	return m_pPed->GetPosition();
}

void
CPlayerInfo::LoadPlayerSkin()
{
	DeletePlayerSkin();

	m_pSkinTexture = CPlayerSkin::GetSkinTexture(m_aSkinName);
	if (!m_pSkinTexture)
		m_pSkinTexture = CPlayerSkin::GetSkinTexture(DEFAULT_SKIN_NAME);
}

void
CPlayerInfo::DeletePlayerSkin()
{
	if (m_pSkinTexture) {
		RwTextureDestroy(m_pSkinTexture);
		m_pSkinTexture = NULL;
	}
}

void
CPlayerInfo::KillPlayer()
{
	if (m_WBState != WBSTATE_PLAYING) return;

	m_WBState = WBSTATE_WASTED;
	m_nWBTime = CTimer::GetTimeInMilliseconds();
	CDarkel::ResetOnPlayerDeath();
	CMessages::AddBigMessage(TheText.Get("DEAD"), 4000, 2);
	CStats::TimesDied++;
}

void
CPlayerInfo::ArrestPlayer()
{
	if (m_WBState != WBSTATE_PLAYING) return;

	m_WBState = WBSTATE_BUSTED;
	m_nWBTime = CTimer::GetTimeInMilliseconds();
	CDarkel::ResetOnPlayerDeath();
	CMessages::AddBigMessage(TheText.Get("BUSTED"), 5000, 2);
	CStats::TimesArrested++;
}

STARTPATCHES
InjectHook(0x4A1700, &CPlayerInfo::LoadPlayerSkin, PATCH_JUMP);
InjectHook(0x4A1750, &CPlayerInfo::DeletePlayerSkin, PATCH_JUMP);
InjectHook(0x4A12E0, &CPlayerInfo::KillPlayer, PATCH_JUMP);
InjectHook(0x4A1330, &CPlayerInfo::ArrestPlayer, PATCH_JUMP);
ENDPATCHES
