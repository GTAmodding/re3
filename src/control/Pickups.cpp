#include "common.h"

#include "main.h"

#include "Camera.h"
#include "Coronas.h"
#include "Darkel.h"
#include "Entity.h"
#include "Explosion.h"
#include "Font.h"
#include "Garages.h"
#include "General.h"
#include "ModelIndices.h"
#include "Object.h"
#include "Pad.h"
#include "Pickups.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "DMAudio.h"
#include "Fire.h"
#include "PointLights.h"
#include "Pools.h"
#ifdef FIX_BUGS
#include "Replay.h"
#endif
#include "Script.h"
#include "Shadows.h"
#include "SpecialFX.h"
#include "Sprite.h"
#include "Timer.h"
#include "WaterLevel.h"
#include "World.h"
#include "Hud.h"
#include "Messages.h"
#include "Streaming.h"
#include "SaveBuf.h"

CPickup CPickups::aPickUps[NUMPICKUPS];
int16 CPickups::NumMessages;
int32 CPickups::aPickUpsCollected[NUMCOLLECTEDPICKUPS];
int16 CPickups::CollectedPickUpIndex;

int32 CPickups::PlayerOnWeaponPickup;
int32 CollectPickupBuffer;

// unused
bool CPickups::bPickUpcamActivated;
CVehicle *CPickups::pPlayerVehicle;
CVector CPickups::StaticCamCoors;
uint32 CPickups::StaticCamStartTime;

tPickupMessage CPickups::aMessages[NUMPICKUPMESSAGES];

uint16 AmmoForWeapon[WEAPONTYPE_TOTALWEAPONS + 1] = { 0,  1,  1,  1,   1,   1,   1,   1,   1,   1,  1,  1, 8,   8,   8,    8, 8,   68, 24,
													 32, 28, 20, 200, 120, 120, 120, 120, 120, 40, 28, 8, 300, 200, 1000, 1, 400, 36, 0 };

uint16 AmmoForWeapon_OnStreet[WEAPONTYPE_TOTALWEAPONS + 1] = { 0,  1,  1,  1,   1,  1,  1,  1,  1,  1,  1,  1, 4,   4,   4,   4, 4,   34, 12,
															  16, 14, 10, 100, 60, 60, 60, 60, 60, 20, 14, 4, 150, 100, 500, 1, 400, 36, 0 };

uint16 CostOfWeapon[WEAPONTYPE_TOTALWEAPONS + 3] = { 0,     10,    10,   10,   10,   10,    10,   10,    10,  10,  10,  10,   1000, 1000,
													1000,  500,   8000, 250,  400,  1200,  1250, 1250,  800, 800, 650, 1200, 5000, 400,
													10000, 10000, 8000, 8000, 8000, 10000, 1000, 11000, 500, 20,  10,  0 };

struct
{
	uint8 r,g,b;
	float unk;
} aPickupColors[] = {
	{ 128, 128, 128, 1.0f },
	{ 128, 128, 128, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 149, 194, 24, 1.0f },
	{ 149, 194, 24, 1.0f },
	{ 45, 155, 90, 1.0f },
	{ 45, 155, 90, 1.0f },
	{ 45, 155, 90, 1.0f },
	{ 255, 227, 79, 1.0f },
	{ 255, 227, 79, 1.0f },
	{ 255, 227, 79, 1.0f },
	{ 255, 227, 79, 1.0f },
	{ 254, 137, 0, 1.0f },
	{ 254, 137, 0, 1.0f },
	{ 249, 131, 215, 1.0f },
	{ 249, 131, 215, 1.0f },
	{ 164, 40, 178, 1.0f },
	{ 164, 40, 178, 1.0f },
	{ 164, 40, 178, 1.0f },
	{ 164, 40, 178, 1.0f },
	{ 69, 69, 69, 1.0f },
	{ 69, 69, 69, 1.0f },
	{ 69, 69, 69, 1.0f },
	{ 255, 100, 100, 1.0f },
	{ 128, 255, 128, 1.0f },
	{ 100, 100, 255, 1.0f },
	{ 255, 255, 100, 1.0f },
	{ 255, 100, 100, 1.0f },
	{ 100, 255, 100, 1.0f },
	{ 255, 255, 255, 1.0f }
};


void
ModifyStringLabelForControlSetting(char *str)
{
	int len = (int)strlen(str);
	if (len <= 2)
		return;

	if (str[len - 2] != '_')
		return;

	switch (CPad::GetPad(0)->Mode) {
		case 0:
		case 1:
			str[len - 1] = 'L';
			break;
		case 2:
			str[len - 1] = 'T';
			break;
		case 3:
			str[len - 1] = 'C';
			break;
		default:
			return;
	}
}

void
CPickup::ExtractAmmoFromPickup(CPlayerPed *player)
{
	eWeaponType weaponType = CPickups::WeaponForModel(m_pObject->GetModelIndex());
	
	if (m_eType == PICKUP_IN_SHOP || !CWeaponInfo::IsWeaponSlotAmmoMergeable(CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot))
		return;

	uint32 ammo = m_nQuantity;
	if (ammo == 0) {
		if (!m_bWasAmmoCollected)
			ammo = AmmoForWeapon_OnStreet[weaponType];
		else
			goto removeAmmo;
	}
	player->GrantAmmo(weaponType, ammo);
	DMAudio.PlayOneShot(player->m_audioEntityId, SOUND_WEAPON_RELOAD, weaponType); // BUG? weapon type as volume, wtf?
removeAmmo:
	m_nQuantity = 0;
	m_bWasAmmoCollected = true;
}

void
CPickup::Remove()
{
	GetRidOfObjects();
	m_bRemoved = true;
	m_eType = PICKUP_NONE;
}

CObject *
CPickup::GiveUsAPickUpObject(CObject **ppObject, CObject **ppExtraObject, int32 handle, int32 extraHandle)
{
	CObject *&object = *ppObject;
	CObject *&extraObject = *ppExtraObject;

	object = extraObject = nil;

	int32 modelId = -1;
	if (CModelInfo::GetModelInfo(m_eModelIndex)->GetModelType() == MITYPE_WEAPON) {
		CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(((CWeaponModelInfo*)CModelInfo::GetModelInfo(m_eModelIndex))->GetWeaponInfo());
		modelId = weaponInfo->m_nModelId;
		if (modelId == m_eModelIndex)
			modelId = weaponInfo->m_nModel2Id;
	}

	if (handle >= 0) {
		CPools::MakeSureSlotInObjectPoolIsEmpty(handle);
		if (extraHandle >= 0)
			CPools::MakeSureSlotInObjectPoolIsEmpty(extraHandle);
		if (object == nil)
			object = new(handle) CObject(m_eModelIndex, false);

		if (extraHandle >= 0 && modelId != -1 && extraObject == nil)
			extraObject = new(extraHandle) CObject(modelId, false);
	} else {
		object = new CObject(m_eModelIndex, false);
		if (modelId != -1)
			extraObject = new CObject(modelId, false);
	}

	if (object == nil) return nil;
	object->ObjectCreatedBy = MISSION_OBJECT;
	object->SetPosition(m_vecPos);
	object->SetOrientation(0.0f, 0.0f, -HALFPI);
	object->GetMatrix().UpdateRW();
	object->UpdateRwFrame();

	object->bAffectedByGravity = false;
	object->bExplosionProof = true;
	object->bUsesCollision = false;
	object->bIsPickup = true;
	object->bAmmoCollected = m_bWasAmmoCollected;
	object->bHasPreRenderEffects = true;

	if (extraObject) {
		extraObject->ObjectCreatedBy = MISSION_OBJECT;
		extraObject->SetPosition(m_vecPos);
		extraObject->SetOrientation(0.0f, 0.0f, -HALFPI);
		extraObject->GetMatrix().UpdateRW();
		extraObject->UpdateRwFrame();

		extraObject->bAffectedByGravity = false;
		extraObject->bExplosionProof = true;
		extraObject->bUsesCollision = false;
		extraObject->bIsPickup = true;
		extraObject->bAmmoCollected = true;
		extraObject->bHasPreRenderEffects = true;
		extraObject->m_nBonusValue = 0;
		extraObject->bPickupObjWithMessage = false;
		extraObject->bOutOfStock = false;
	}

	object->m_nBonusValue = (m_eModelIndex == MI_PICKUP_BONUS || m_eModelIndex == MI_PICKUP_CLOTHES) ? m_nQuantity : 0;

	switch (m_eType)
	{
	case PICKUP_IN_SHOP:
		object->bPickupObjWithMessage = true;
		object->bOutOfStock = false;
		if (m_eModelIndex == MI_PICKUP_HEALTH || m_eModelIndex == MI_PICKUP_ADRENALINE)
			object->m_nCostValue = 0;
		else
			object->m_nCostValue = CostOfWeapon[CPickups::WeaponForModel(m_eModelIndex)];
		break;
	case PICKUP_ON_STREET:
	case PICKUP_ONCE:
	case PICKUP_ONCE_TIMEOUT:
	case PICKUP_COLLECTABLE1:
	case PICKUP_MONEY:
	case PICKUP_MINE_INACTIVE:
	case PICKUP_MINE_ARMED:
	case PICKUP_NAUTICAL_MINE_INACTIVE:
	case PICKUP_NAUTICAL_MINE_ARMED:
	case PICKUP_FLOATINGPACKAGE:
	case PICKUP_ON_STREET_SLOW:
		object->bPickupObjWithMessage = false;
		object->bOutOfStock = false;
		break;
	case PICKUP_IN_SHOP_OUT_OF_STOCK:
		object->bPickupObjWithMessage = false;
		object->bOutOfStock = true;
		object->bRenderScorched = true;
		break;
	case PICKUP_FLOATINGPACKAGE_FLOATING:
	default:
		break;
	}
	return object;
}

bool
CPickup::CanBePickedUp(CPlayerPed *player, int playerId)
{
	assert(m_pObject != nil);
	bool cannotBePickedUp =
		(m_pObject->GetModelIndex() == MI_PICKUP_BODYARMOUR && player->m_fArmour > CWorld::Players[playerId].m_nMaxArmour - 0.2f)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_HEALTH && player->m_fHealth > CWorld::Players[playerId].m_nMaxHealth - 0.2f)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_BRIBE && player->m_pWanted->GetWantedLevel() == 0)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_KILLFRENZY && (CTheScripts::IsPlayerOnAMission() || CDarkel::FrenzyOnGoing() || !CGame::nastyGame))
		|| (m_eType == PICKUP_ASSET_REVENUE && m_fRevenue < 10.0f);
	return !cannotBePickedUp;
}

bool
CPickup::Update(CPlayerPed *player, CVehicle *vehicle, int playerId)
{
	bool result = false;
	float waterLevel;

	if (m_pObject) {
		m_pObject->GetMatrix().GetPosition() = m_vecPos;
		if (m_pExtraObject)
			m_pExtraObject->GetMatrix().GetPosition() = m_vecPos;
	}
	if (m_eType == PICKUP_ASSET_REVENUE) {
		uint32 timePassed = CTimer::GetTimeInMilliseconds() - m_nTimer;
		m_nTimer = CTimer::GetTimeInMilliseconds();

		if (Distance(FindPlayerCoors(), m_vecPos) > 10.0f)
			m_fRevenue += float(timePassed * m_nMoneySpeed) / SQR(1200.0f);

		m_fRevenue = Min(m_fRevenue, m_nQuantity);

		m_pObject->m_nCostValue = m_fRevenue < 10 ? 0 : m_fRevenue;
	}

	if (m_bRemoved) {
		if (CTimer::GetTimeInMilliseconds() > m_nTimer) {
			// respawn pickup if we're far enough
			float dist = (FindPlayerCoors().x - m_vecPos.x) * (FindPlayerCoors().x - m_vecPos.x) + (FindPlayerCoors().y - m_vecPos.y) * (FindPlayerCoors().y - m_vecPos.y);
			if (dist > 100.0f || m_eType == PICKUP_IN_SHOP && dist > 2.4f) {
				m_pObject = GiveUsAPickUpObject(&m_pObject, &m_pExtraObject, -1, -1);
				if (m_pObject) {
					CWorld::Add(m_pObject);
					m_bRemoved = false;
				}
			}
		}
		return false;
	}

	if (!m_pObject) {
		GiveUsAPickUpObject(&m_pObject, &m_pExtraObject, -1, -1);
		if (m_pObject)
			CWorld::Add(m_pObject);
		if (m_pExtraObject)
			CWorld::Add(m_pExtraObject);
	}

	if (!m_pObject) return false;

	if (!IsMine()) {
		// let's check if we touched the pickup
		bool isPickupTouched = false;
		if (m_pObject->GetModelIndex() == MI_PICKUP_BRIBE) {
			if (vehicle != nil) {
				if (vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f))
					isPickupTouched = true;
			}
			else {
				if (Abs(player->GetPosition().z - m_pObject->GetPosition().z) < 2.0f) {
					if ((player->GetPosition().x - m_pObject->GetPosition().x) * (player->GetPosition().x - m_pObject->GetPosition().x) +
						(player->GetPosition().y - m_pObject->GetPosition().y) * (player->GetPosition().y - m_pObject->GetPosition().y) < 1.8f)
						isPickupTouched = true;
				}
			}
		} else if (m_pObject->GetModelIndex() == MI_PICKUP_CAMERA) {
			if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f)) {
				isPickupTouched = true;
			}
		} else if (vehicle == nil) {
			if (Abs(player->GetPosition().z - m_pObject->GetPosition().z) < 2.0f) {
				if ((player->GetPosition().x - m_pObject->GetPosition().x) * (player->GetPosition().x - m_pObject->GetPosition().x) +
					(player->GetPosition().y - m_pObject->GetPosition().y) * (player->GetPosition().y - m_pObject->GetPosition().y) < 1.8f)
					isPickupTouched = true;
			}
		}

		if (isPickupTouched) {
			eWeaponType weaponType = CPickups::WeaponForModel(m_pObject->GetModelIndex());
			if (weaponType < WEAPONTYPE_TOTALWEAPONS && CDarkel::FrenzyOnGoing()) {
				isPickupTouched = false;
				m_bWasControlMessageShown = false;
			} else if (weaponType < WEAPONTYPE_TOTALWEAPONS && weaponType != WEAPONTYPE_UNARMED) {
				uint32 slot = CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
				eWeaponType plrWeaponSlot = FindPlayerPed()->GetWeapon(slot).m_eWeaponType;
				if (plrWeaponSlot != weaponType) {
					if (CStreaming::ms_aInfoForModel[m_pObject->GetModelIndex()].m_loadState == STREAMSTATE_LOADED) {
						if (plrWeaponSlot == WEAPONTYPE_UNARMED || (FindPlayerPed()->GetWeapon(slot).m_nAmmoTotal == 0 && !CWeaponInfo::IsWeaponSlotAmmoMergeable(slot))) {
							if (CTimer::GetTimeInMilliseconds() - FindPlayerPed()->m_nPadDownPressedInMilliseconds < 1500) {
								CPickups::PlayerOnWeaponPickup = 6;
								isPickupTouched = false;
							}
						} else {
							CPickups::PlayerOnWeaponPickup = 6;
							if (CWeaponInfo::IsWeaponSlotAmmoMergeable(slot)) {
								if (m_eType == PICKUP_ONCE_TIMEOUT || m_eType == PICKUP_ONCE || m_eType == PICKUP_ON_STREET) {
									ExtractAmmoFromPickup(player);
									FindPlayerPed()->GetWeapon(slot).Reload();
								}
							}
							if (!m_bWasControlMessageShown) {
								switch (CPad::GetPad(0)->Mode)
								{
								case 0:
								case 1:
									CHud::SetHelpMessage(TheText.Get("PU_CF1"), false);
									break;
								case 2:
									CHud::SetHelpMessage(TheText.Get("PU_CF3"), false);
									break;
								case 3:
									CHud::SetHelpMessage(TheText.Get("PU_CF4"), false);
									break;
								default:
									break;
								}
								m_bWasControlMessageShown = true;
							}
							if (CollectPickupBuffer == 0)
								isPickupTouched = false;
							if (CTimer::GetTimeInMilliseconds() - FindPlayerPed()->m_nPadDownPressedInMilliseconds < 1500)
								isPickupTouched = false;
						}
					} else
						isPickupTouched = false;
				}
			}
		} else
			m_bWasControlMessageShown = false;

		// if we didn't then we've got nothing to do
		if (isPickupTouched && CanBePickedUp(player, playerId)) {
			if (m_pObject->GetModelIndex() != MI_PICKUP_PROPERTY && m_pObject->GetModelIndex() != MI_PICKUP_PROPERTY_FORSALE)
				CPad::GetPad(0)->StartShake(120, 100);

			eWeaponType weaponType = CPickups::WeaponForModel(m_pObject->GetModelIndex());
			switch (m_eType)
			{
			case PICKUP_IN_SHOP:
				if (CWorld::Players[playerId].m_nMoney < CostOfWeapon[weaponType])
					CGarages::TriggerMessage("PU_MONY", -1, 6000, -1);
				else {
					CWorld::Players[playerId].m_nMoney -= CostOfWeapon[weaponType];
					if (!CPickups::GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
						if (!player->DoesPlayerWantNewWeapon(weaponType, false))
							break; 
						player->GiveWeapon(weaponType, AmmoForWeapon[weaponType]);
						player->m_nSelectedWepSlot = player->GetWeaponSlot(weaponType);
						DMAudio.PlayFrontEndSound(SOUND_PICKUP_WEAPON_BOUGHT, m_pObject->GetModelIndex() - MI_GRENADE);
					}
					result = true;
					Remove();
				}
				break;
			case PICKUP_ON_STREET:
			case PICKUP_ON_STREET_SLOW:
				if (!CPickups::GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
					if (!player->DoesPlayerWantNewWeapon(weaponType, false))
						break;
					if (weaponType != WEAPONTYPE_UNARMED) {
						player->GiveWeapon(weaponType, m_nQuantity != 0 ? m_nQuantity : (m_bWasAmmoCollected ? 0 : AmmoForWeapon_OnStreet[weaponType]), true);

						if (player->m_nSelectedWepSlot == player->GetWeaponSlot(WEAPONTYPE_UNARMED)) 
							player->m_nSelectedWepSlot = player->GetWeaponSlot(weaponType);

						DMAudio.PlayFrontEndSound(SOUND_PICKUP_WEAPON, m_pObject->GetModelIndex() - MI_GRENADE);
					} else if (m_pObject->GetModelIndex() == MI_PICKUP_CAMERA && vehicle != nil) {
						DMAudio.PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
						CPickups::bPickUpcamActivated = true;
						CPickups::pPlayerVehicle = FindPlayerVehicle();
						CPickups::StaticCamCoors = m_pObject->GetPosition();
						CPickups::StaticCamStartTime = CTimer::GetTimeInMilliseconds();
					}
				}
				if (m_eType == PICKUP_ON_STREET)
					m_nTimer = CTimer::GetTimeInMilliseconds() + 30000;
				else if (m_eType == PICKUP_ON_STREET_SLOW) {
					if (MI_PICKUP_BRIBE == m_pObject->GetModelIndex())
						m_nTimer = CTimer::GetTimeInMilliseconds() + 300000;
					else
						m_nTimer = CTimer::GetTimeInMilliseconds() + 720000;
				}

				result = true;
				GetRidOfObjects();
				m_bRemoved = true;
				break;
			case PICKUP_ONCE:
			case PICKUP_ONCE_TIMEOUT:
			case PICKUP_ONCE_TIMEOUT_SLOW:
				if (!CPickups::GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
					if (!player->DoesPlayerWantNewWeapon(weaponType, false)) {
						ExtractAmmoFromPickup(player);
						break;
					}

					if (weaponType != WEAPONTYPE_UNARMED) {
						player->GiveWeapon(weaponType, m_nQuantity != 0 ? m_nQuantity : (m_bWasAmmoCollected ? 0 : AmmoForWeapon[weaponType]), true);
						if (player->m_nSelectedWepSlot == player->GetWeaponSlot(WEAPONTYPE_UNARMED))
							player->m_nSelectedWepSlot = player->GetWeaponSlot(weaponType);
					}
					if (MI_PICKUP_SAVEGAME != m_pObject->GetModelIndex())
						DMAudio.PlayFrontEndSound(SOUND_PICKUP_WEAPON, m_pObject->GetModelIndex() - MI_GRENADE);
				}
				result = true;
				Remove();
				break;
			case PICKUP_COLLECTABLE1:
				CWorld::Players[playerId].m_nCollectedPackages++;
				CWorld::Players[playerId].m_nMoney += 100;

				if (CWorld::Players[playerId].m_nCollectedPackages == CWorld::Players[playerId].m_nTotalPackages) {
					printf("All collectables have been picked up\n");
					CGarages::TriggerMessage("CO_ALL", -1, 5000, -1);
					CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 100000;
				} else
					CGarages::TriggerMessage("CO_ONE", CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages, 5000, CWorld::Players[CWorld::PlayerInFocus].m_nTotalPackages);

				result = true;
				Remove();
				DMAudio.PlayFrontEndSound(SOUND_PICKUP_HIDDEN_PACKAGE, 0);
				break;
			case PICKUP_MONEY:
				CWorld::Players[playerId].m_nMoney += m_nQuantity;
				sprintf(gString, "$%d", m_nQuantity);
#ifdef MONEY_MESSAGES
				CMoneyMessages::RegisterOne(m_vecPos + CVector(0.0f, 0.0f, 1.0f), gString, 0, 255, 0, 0.5f, 0.5f);
#endif
				result = true;
				Remove();
				DMAudio.PlayFrontEndSound(SOUND_PICKUP_MONEY, 0);
				player->Say(SOUND_PED_MUGGING);
				break;
			case PICKUP_ASSET_REVENUE:
				CWorld::Players[CWorld::PlayerInFocus].m_nMoney += m_fRevenue;
				m_fRevenue = 0.0f;
				DMAudio.PlayFrontEndSound(SOUND_PICKUP_MONEY, 0);
				break;
			case PICKUP_PROPERTY_LOCKED:
				if (!m_bWasControlMessageShown) {
					m_bWasControlMessageShown = true;
					CHud::SetHelpMessage(TheText.Get(m_sTextKey), false);
				}
				break;
			case PICKUP_PROPERTY_FORSALE:
				ModifyStringLabelForControlSetting(m_sTextKey);
				CMessages::InsertNumberInString(TheText.Get(m_sTextKey), m_nQuantity,
					0, 0, 0, 0, 0, gUString);
				if (!CHud::IsHelpMessageBeingDisplayed())
					CHud::SetHelpMessage(gUString, false);
				if (CollectPickupBuffer == 0)
					break;
				if (CTheScripts::IsPlayerOnAMission())
					CHud::SetHelpMessage(TheText.Get("PROP_2"), true);
				else {
					if (CWorld::Players[CWorld::PlayerInFocus].m_nMoney >= m_nQuantity) {
						CWorld::Players[CWorld::PlayerInFocus].m_nMoney -= m_nQuantity;
						CHud::SetHelpMessage(nil, true);
						result = true;
						Remove();
						break;
					}
					CHud::SetHelpMessage(TheText.Get("PROP_1"), true);
				}
				break;
			default:
				break;
			}
		}
	} else {
		switch (m_eType)
		{
		case PICKUP_MINE_INACTIVE:
			if (vehicle != nil && !vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f)) {
				m_eType = PICKUP_MINE_ARMED;
				m_nTimer = CTimer::GetTimeInMilliseconds() + 10000;
			}
			break;
		case PICKUP_NAUTICAL_MINE_INACTIVE:
		{
			if (CWaterLevel::GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, false))
				m_pObject->GetMatrix().GetPosition().z = waterLevel + 0.6f;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();

			bool touched = false;
			for (int32 i = CPools::GetVehiclePool()->GetSize()-1; i >= 0; i--) {
				CVehicle *vehicle = CPools::GetVehiclePool()->GetSlot(i);
				if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 1.5f)) {
					touched = true;
#ifdef FIX_BUGS
					break; // added break here
#endif
				}
			}

			if (!touched) {
				m_eType = PICKUP_NAUTICAL_MINE_ARMED;
				m_nTimer = CTimer::GetTimeInMilliseconds() + 10000;
			}
			break;
		}
		case PICKUP_NAUTICAL_MINE_ARMED:
			if (CWaterLevel::GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, false))
				m_pObject->GetMatrix().GetPosition().z = waterLevel + 0.6f;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			// no break here
		case PICKUP_MINE_ARMED:
		{
			bool explode = false;
			if (CTimer::GetTimeInMilliseconds() > m_nTimer)
				explode = true;
#ifdef FIX_BUGS
			else// added else here since vehicle lookup is useless
#endif
			{
				for (int32 i = CPools::GetVehiclePool()->GetSize()-1; i >= 0; i--) {
					CVehicle *vehicle = CPools::GetVehiclePool()->GetSlot(i);
					if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 1.5f)) {
						explode = true;
#ifdef FIX_BUGS
						break; // added break here
#endif
					}
				}
			}
			if (explode) {
				CExplosion::AddExplosion(nil, nil, EXPLOSION_MINE, m_pObject->GetPosition(), 0);
				Remove();
			}
			break;
		}
		case PICKUP_FLOATINGPACKAGE:
			m_pObject->m_vecMoveSpeed.z -= 0.01f * CTimer::GetTimeStep();
			m_pObject->GetMatrix().GetPosition() += m_pObject->GetMoveSpeed() * CTimer::GetTimeStep();

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			if (CWaterLevel::GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, false) && waterLevel >= m_pObject->GetPosition().z)
				m_eType = PICKUP_FLOATINGPACKAGE_FLOATING;
			break;
		case PICKUP_FLOATINGPACKAGE_FLOATING:
			if (CWaterLevel::GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, false))
				m_pObject->GetMatrix().GetPosition().z = waterLevel;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f)) {
				Remove();
				result = true;
				DMAudio.PlayFrontEndSound(SOUND_PICKUP_FLOAT_PACKAGE, 0);
			}
			break;
		default: break;
		}
	}

	if (!m_bRemoved && (m_eType == PICKUP_ONCE_TIMEOUT || m_eType == PICKUP_ONCE_TIMEOUT_SLOW || m_eType == PICKUP_MONEY) && CTimer::GetTimeInMilliseconds() > m_nTimer)
		Remove();
	
	return result;
}

void
CPickup::ProcessGunShot(CVector *vec1, CVector *vec2)
{
	CColLine line(*vec1, *vec2);
	if (m_pObject) {
		CColSphere sphere;
		sphere.radius = 4.0f;
		sphere.center = m_pObject->GetPosition();
		if (CCollision::TestLineSphere(line, sphere)) {
			CExplosion::AddExplosion(nil, nil, EXPLOSION_MINE, m_pObject->GetPosition(), 0);
			CWorld::Remove(m_pObject);
			delete m_pObject;
			m_pObject = nil;
			m_bRemoved = true;
			m_eType = PICKUP_NONE;
		}
	}
}

void
CPickup::GetRidOfObjects()
{
	if (m_pObject) {
		CWorld::Remove(m_pObject);
		delete m_pObject;
		m_pObject = nil;
	}
	if (m_pExtraObject) {
		CWorld::Remove(m_pExtraObject);
		delete m_pExtraObject;
		m_pExtraObject = nil;
	}
}

void
CPickups::Init(void)
{
	NumMessages = 0;
	for (int i = 0; i < NUMPICKUPS; i++) {
		aPickUps[i].m_eType = PICKUP_NONE;
		aPickUps[i].m_nIndex = 1;
		aPickUps[i].m_pObject = nil;
		aPickUps[i].m_pExtraObject = nil;
	}

	for (int i = 0; i < NUMCOLLECTEDPICKUPS; i++)
		aPickUpsCollected[i] = 0;

	CollectedPickUpIndex = 0;
}

bool
CPickups::TestForPickupsInBubble(CVector pos, float range)
{
	for (int i = 0; i < NUMPICKUPS; i++) {
		if ((aPickUps[i].m_vecPos - pos).Magnitude() < range)
			return true;
	}
	return false;
}

bool
CPickups::TryToMerge_WeaponType(CVector pos, eWeaponType weapon, uint8 type, uint32 quantity, bool unused) {
	for (int i = 0; i < NUMPICKUPS; i++) {
		if (aPickUps[i].m_eType == type && aPickUps[i].m_eModelIndex == ModelForWeapon(weapon))
			if ((aPickUps[i].m_vecPos - pos).Magnitude() < 7.5f) {
				aPickUps[i].m_nQuantity += quantity;
				return true;
			}
	}
	return false;
}

bool
CPickups::IsPickUpPickedUp(int32 pickupId)
{
	for (int i = 0; i < NUMCOLLECTEDPICKUPS; i++) {
		if (pickupId == aPickUpsCollected[i]) {
			aPickUpsCollected[i] = 0;
			return true;
		}
	}
	return false;
}

void
CPickups::PassTime(uint32 time)
{
	for (int i = 0; i < NUMPICKUPS; i++) {
		if (aPickUps[i].m_eType != PICKUP_NONE && aPickUps[i].m_eType != PICKUP_ASSET_REVENUE) {
			if (aPickUps[i].m_nTimer <= time)
				aPickUps[i].m_nTimer = 0;
			else
				aPickUps[i].m_nTimer -= time;
		}
	}
}

int32
CPickups::GetActualPickupIndex(int32 index)
{
	if (index == -1) return -1;

	// doesn't look nice
	if ((uint16)((index & 0xFFFF0000) >> 16) != aPickUps[(uint16)index].m_nIndex) return -1;
	return (uint16)index;
}

bool
CPickups::GivePlayerGoodiesWithPickUpMI(int16 modelIndex, int playerIndex)
{
	CPlayerPed *player;

	if (playerIndex <= 0) player = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
	else player = CWorld::Players[playerIndex].m_pPed;

	if (modelIndex == MI_PICKUP_ADRENALINE) {
		player->m_bAdrenalineActive = true;
		player->m_nAdrenalineTime = CTimer::GetTimeInMilliseconds() + 20000;
		player->m_fCurrentStamina = player->m_fMaxStamina;
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_ADRENALINE, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_BODYARMOUR) {
		player->m_fArmour = CWorld::Players[playerIndex].m_nMaxArmour;
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_INFO) {
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_HEALTH) {
		player->m_fHealth = CWorld::Players[playerIndex].m_nMaxHealth;
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_HEALTH, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_BONUS) {
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_BRIBE) {
		int32 level = Max(FindPlayerPed()->m_pWanted->GetWantedLevel() - 1, 0);
		player->SetWantedLevel(level);
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_KILLFRENZY) {
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	}
	return false;
}

void
CPickups::RemovePickUp(int32 pickupIndex)
{
	int32 index = GetActualPickupIndex(pickupIndex);
	if (index == -1) return;

	if (aPickUps[index].m_pObject) {
		CWorld::Remove(aPickUps[index].m_pObject);
		delete aPickUps[index].m_pObject;
		aPickUps[index].m_pObject = nil;
	}
	if (aPickUps[index].m_pExtraObject) {
		CWorld::Remove(aPickUps[index].m_pExtraObject);
		delete aPickUps[index].m_pExtraObject;
		aPickUps[index].m_pExtraObject = nil;
	}
	aPickUps[index].m_eType = PICKUP_NONE;
	aPickUps[index].m_bRemoved = true;
}

int32
CPickups::GenerateNewOne(CVector pos, uint32 modelIndex, uint8 type, uint32 quantity, uint32 rate, bool highPriority, char* pText)
{
	bool bFreeFound = false;
	int32 slot = 0;

	if (type == PICKUP_FLOATINGPACKAGE || type == PICKUP_NAUTICAL_MINE_INACTIVE || highPriority) {
		for (slot = NUMPICKUPS-1; slot >= 0; slot--) {
			if (aPickUps[slot].m_eType == PICKUP_NONE) {
				bFreeFound = true;
				break;
			}
		}
	}
	if (!bFreeFound) {
		for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
			if (aPickUps[slot].m_eType == PICKUP_NONE) {
				bFreeFound = true;
				break;
			}
		}
	}

	if (!bFreeFound) {
		for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
			if (aPickUps[slot].m_eType == PICKUP_MONEY) break;
		}

		if (slot >= NUMGENERALPICKUPS) {
			for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
				if (aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT || aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT_SLOW) break;
			}

			if (slot >= NUMGENERALPICKUPS) return -1;
			aPickUps[slot].GetRidOfObjects();
		}
	}

	if (slot >= NUMPICKUPS) return -1;

	aPickUps[slot].m_eType = type;
	aPickUps[slot].m_bRemoved = false;
	aPickUps[slot].m_nQuantity = quantity;
	aPickUps[slot].m_nMoneySpeed = rate;
	aPickUps[slot].m_fRevenue = 0.0f;
	aPickUps[slot].m_nTimer = CTimer::GetTimeInMilliseconds();
	aPickUps[slot].m_bWasAmmoCollected = highPriority;
	aPickUps[slot].m_bWasControlMessageShown = false;
	if (type == PICKUP_ONCE_TIMEOUT)
		aPickUps[slot].m_nTimer = CTimer::GetTimeInMilliseconds() + 20000;
	else if (type == PICKUP_ONCE_TIMEOUT_SLOW)
		aPickUps[slot].m_nTimer = CTimer::GetTimeInMilliseconds() + 120000;
	else if (type == PICKUP_MONEY)
		aPickUps[slot].m_nTimer = CTimer::GetTimeInMilliseconds() + 30000;
	else if (type == PICKUP_MINE_INACTIVE || type == PICKUP_MINE_ARMED) {
		aPickUps[slot].m_eType = PICKUP_MINE_INACTIVE;
		aPickUps[slot].m_nTimer = CTimer::GetTimeInMilliseconds() + 1500;
	} else if (type == PICKUP_NAUTICAL_MINE_INACTIVE || type == PICKUP_NAUTICAL_MINE_ARMED) {
		aPickUps[slot].m_eType = PICKUP_NAUTICAL_MINE_INACTIVE;
		aPickUps[slot].m_nTimer = CTimer::GetTimeInMilliseconds() + 1500;
	}
	aPickUps[slot].m_eModelIndex = modelIndex;
	if (pText)
		strncpy(aPickUps[slot].m_sTextKey, pText, 8);
	else
		aPickUps[slot].m_sTextKey[0] = '\0';

	aPickUps[slot].m_vecPos = pos;
	aPickUps[slot].m_pObject = aPickUps[slot].GiveUsAPickUpObject(&aPickUps[slot].m_pObject, &aPickUps[slot].m_pExtraObject, -1, -1);
	if (aPickUps[slot].m_pObject)
		CWorld::Add(aPickUps[slot].m_pObject);
	if (aPickUps[slot].m_pExtraObject)
		CWorld::Add(aPickUps[slot].m_pExtraObject);
	return GetNewUniquePickupIndex(slot);
}

int32
CPickups::GenerateNewOne_WeaponType(CVector pos, eWeaponType weaponType, uint8 type, uint32 quantity)
{
	return GenerateNewOne(pos, ModelForWeapon(weaponType), type, quantity);
}

int32
CPickups::GetNewUniquePickupIndex(int32 slot)
{
	if (aPickUps[slot].m_nIndex >= 0xFFFE)
		aPickUps[slot].m_nIndex = 1;
	else
		aPickUps[slot].m_nIndex++;
	return slot | (aPickUps[slot].m_nIndex << 16);
}

int32
CPickups::ModelForWeapon(eWeaponType weaponType)
{
	return CWeaponInfo::GetWeaponInfo(weaponType)->m_nModelId;
}

eWeaponType
CPickups::WeaponForModel(int32 model)
{
	if (model == MI_PICKUP_BODYARMOUR) return WEAPONTYPE_ARMOUR;
	if (model == MI_PICKUP_HEALTH) return WEAPONTYPE_HEALTH;
	if (model == MI_PICKUP_ADRENALINE) return WEAPONTYPE_ARMOUR;
	if (model == -1) return WEAPONTYPE_UNARMED;
	return ((CWeaponModelInfo*)CModelInfo::GetModelInfo(model))->GetWeaponInfo();
}

void
CPickups::AddToCollectedPickupsArray(int32 index)
{
	aPickUpsCollected[CollectedPickUpIndex++] = index | (aPickUps[index].m_nIndex << 16);
	if (CollectedPickUpIndex >= NUMCOLLECTEDPICKUPS)
		CollectedPickUpIndex = 0;
}

void
CPickups::Update()
{
#ifdef FIX_BUGS // RIP speedrunning (solution from SA)
	if (CReplay::IsPlayingBack())
		return;
#endif
#ifdef CAMERA_PICKUP
	if ( bPickUpcamActivated ) // taken from PS2
	{
		float dist = Distance2D(StaticCamCoors, FindPlayerCoors());
		float mult;
		if ( dist < 10.0f )
			mult = 1.0f - (dist / 10.0f );
		else
			mult = 0.0f;

		CVector pos = StaticCamCoors;
		pos.z += (pPlayerVehicle->GetColModel()->boundingBox.GetSize().z + 2.0f) * mult;

		if ( (CTimer::GetTimeInMilliseconds() - StaticCamStartTime) > 750 )
		{
			TheCamera.SetCamPositionForFixedMode(pos, CVector(0.0f, 0.0f, 0.0f));
			TheCamera.TakeControl(FindPlayerVehicle(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_SCRIPT);
		}

		if ( FindPlayerVehicle() != pPlayerVehicle || Distance(StaticCamCoors, FindPlayerCoors()) > 40.0f
			|| ((CTimer::GetTimeInMilliseconds() - StaticCamStartTime) > 60000) )
		{
			TheCamera.RestoreWithJumpCut();
			bPickUpcamActivated = false;
		}
	}
#endif
	if (CPad::GetPad(0)->CollectPickupJustDown())
		CollectPickupBuffer = 6;
	else
		CollectPickupBuffer = Max(0, CollectPickupBuffer - 1);

	if (PlayerOnWeaponPickup)
		PlayerOnWeaponPickup = Max(0, PlayerOnWeaponPickup - 1);

#define PICKUPS_FRAME_SPAN (6)
#ifdef FIX_BUGS
	for (uint32 i = NUMGENERALPICKUPS * (CTimer::GetFrameCounter() % PICKUPS_FRAME_SPAN) / PICKUPS_FRAME_SPAN; i < NUMGENERALPICKUPS * (CTimer::GetFrameCounter() % PICKUPS_FRAME_SPAN + 1) / PICKUPS_FRAME_SPAN; i++) {
#else // BUG: this code can only reach 318 out of 320 pickups
	for (uint32 i = NUMGENERALPICKUPS / PICKUPS_FRAME_SPAN * (CTimer::GetFrameCounter() % PICKUPS_FRAME_SPAN); i < NUMGENERALPICKUPS / PICKUPS_FRAME_SPAN * (CTimer::GetFrameCounter() % PICKUPS_FRAME_SPAN + 1); i++) {
#endif
		if (aPickUps[i].m_eType != PICKUP_NONE && aPickUps[i].Update(FindPlayerPed(), FindPlayerVehicle(), CWorld::PlayerInFocus))
			AddToCollectedPickupsArray(i);
	}
#undef PICKUPS_FRAME_SPAN
	for (uint32 i = NUMGENERALPICKUPS; i < NUMPICKUPS; i++) {
		if (aPickUps[i].m_eType != PICKUP_NONE && aPickUps[i].Update(FindPlayerPed(), FindPlayerVehicle(), CWorld::PlayerInFocus))
			AddToCollectedPickupsArray(i);
	}
}

CPickup*
CPickups::FindPickUpForThisObject(CEntity *object)
{
	for (uint32 i = 0; i < NUMPICKUPS; i++) {
		if (aPickUps[i].m_eType != PICKUP_NONE && (aPickUps[i].m_pObject == object || aPickUps[i].m_pExtraObject == object)) {
			return &aPickUps[i];
		}
	}
	return &aPickUps[0];
}

void
CPickups::DoPickUpEffects(CEntity *entity)
{
	CPickup *pickup = FindPickUpForThisObject(entity);

	if (entity->GetModelIndex() == MI_PICKUP_KILLFRENZY)
		entity->bDoNotRender = CTheScripts::IsPlayerOnAMission() || CDarkel::FrenzyOnGoing() || !CGame::nastyGame;

	if (!entity->bDoNotRender) {
		float modifiedSin = 0.3f * (Sin((float)((CTimer::GetTimeInMilliseconds() + (uintptr)entity) & 0x7FF) * DEGTORAD(360.0f / 0x800)) + 1.0f);

#ifdef FIX_BUGS
		int16 colorId = 0;
#else
		int16 colorId;
#endif
		bool doInnerGlow = false;
		bool doOuterGlow = true;

		if (entity->GetModelIndex() == MI_PICKUP_ADRENALINE || entity->GetModelIndex() == MI_PICKUP_CAMERA) {
			colorId = WEAPONTYPE_TOTALWEAPONS;
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_BODYARMOUR) {
			colorId = WEAPONTYPE_ARMOUR;
		} else if (entity->GetModelIndex() == MI_PICKUP_BRIBE) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_INFO || entity->GetModelIndex() == MI_PICKUP_KILLFRENZY) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_HEALTH || entity->GetModelIndex() == MI_PICKUP_BONUS) {
			colorId = WEAPONTYPE_HEALTH;
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_PROPERTY) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_PROPERTY_FORSALE) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_REVENUE) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_SAVEGAME) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_CLOTHES) {
			colorId = WEAPONTYPE_TOTALWEAPONS;
			doOuterGlow = false;
			doInnerGlow = true;
		} else
			colorId = WeaponForModel(entity->GetModelIndex());

		const CVector& pos = pickup->m_vecPos;
		if (doOuterGlow) {
			bool corona1 = false;
			bool corona2 = false;
			int timerVal = (CTimer::GetTimeInMilliseconds() >> 9) & 7;

			if (timerVal < 3)
				corona1 = false;
			else if (timerVal == 3)
				corona1 = (CGeneral::GetRandomNumber() & 3) != 0;
			else
				corona1 = true;

			timerVal = (timerVal - 1) & 7;
			if (timerVal < 3)
				corona2 = false;
			else if (timerVal == 3)
				corona2 = (CGeneral::GetRandomNumber() & 3) != 0;
			else
				corona2 = true;

			if (((CObject*)entity)->bAmmoCollected) {
				corona2 = false;
				corona1 = false;
			}

			if (corona1) {
				CCoronas::RegisterCorona((uintptr)entity,
					aPickupColors[colorId].r * 0.45f, aPickupColors[colorId].g * 0.45f, aPickupColors[colorId].b * 0.45f,
					255, pos, 0.76f, 65.0f,
					CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF,
					0.0f, false, -0.4f);
				CShadows::StoreStaticShadow((uintptr)entity,
					SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0,
					aPickupColors[colorId].r * 0.3f, aPickupColors[colorId].g * 0.3f, aPickupColors[colorId].b * 0.3f,
					4.0f, 1.0f, 40.0f, false, 0.0f);
				float radius = (CGeneral::GetRandomNumber() & 0xF) * 0.1f + 3.0f;
				CPointLights::AddLight(CPointLights::LIGHT_POINT, pos, CVector(0.0f, 0.0f, 0.0f), radius, aPickupColors[colorId].r / 256.0f, aPickupColors[colorId].g / 256.0f, aPickupColors[colorId].b / 256.0f, CPointLights::FOG_NONE, true);
			} else
				CCoronas::RegisterCorona((uintptr)entity, 0, 0, 0, 255, pos, 0.57f, 65.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);

			if (corona2) {
				CCoronas::RegisterCorona(
					(uintptr)entity + 1,
					aPickupColors[colorId].r * 0.55f, aPickupColors[colorId].g * 0.55f, aPickupColors[colorId].b * 0.55f,
					255,
					pos,
					0.6f,
					65.0f,
					CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF,
					0.0f, false, -0.4f);
				if (!corona1)
					CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0,
						aPickupColors[colorId].r * 0.25f, aPickupColors[colorId].g * 0.25f, aPickupColors[colorId].b * 0.25f,
						4.0f, 1.0f, 40.0f, false, 0.0f);
			} else
				CCoronas::RegisterCorona((uintptr)entity + 1, 0, 0, 0, 255, pos, 0.45f, 65.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		}

		CObject *object = (CObject*)entity;
		if (object->bPickupObjWithMessage || object->bOutOfStock || object->m_nBonusValue || object->m_nCostValue) {
			
			float dist = Distance2D(pos, TheCamera.GetPosition());
			const float MAXDIST = 14.0f;

			if (dist < MAXDIST && NumMessages < NUMPICKUPMESSAGES) {
				RwV3d vecOut;
				float fDistX, fDistY;
				if (CSprite::CalcScreenCoors(entity->GetPosition() + CVector(0.0f, 0.0f, 0.7f), &vecOut, &fDistX, &fDistY, true)) {
					aMessages[NumMessages].m_pos.x = vecOut.x;
					aMessages[NumMessages].m_pos.y = vecOut.y;
					aMessages[NumMessages].m_dist.x = fDistX;
					aMessages[NumMessages].m_dist.y = fDistY;
					aMessages[NumMessages].m_weaponType = WeaponForModel(entity->GetModelIndex());
					aMessages[NumMessages].m_color.red = aPickupColors[colorId].r;
					aMessages[NumMessages].m_color.green = aPickupColors[colorId].g;
					aMessages[NumMessages].m_color.blue = aPickupColors[colorId].b;
					aMessages[NumMessages].m_color.alpha = (1.0f - dist / MAXDIST) * 128.0f;
					aMessages[NumMessages].m_bOutOfStock = object->bOutOfStock;
					aMessages[NumMessages].m_quantity = object->m_nBonusValue;
					aMessages[NumMessages].money = object->m_nCostValue;
					NumMessages++;
				}
			}
		}

		uint32 model = entity->GetModelIndex();
		CColModel *colModel = entity->GetColModel();
		CVector colLength = colModel->boundingBox.max - colModel->boundingBox.min;
		float maxDimension = Max(colLength.x, Max(colLength.y, colLength.z));

		float scale = (Max(1.f, 1.2f / maxDimension) - 1.0f) * 0.6f + 1.0f;
		if (model == MI_MINIGUN || model == MI_MINIGUN2)
			scale = 1.2f;

		float angle = (float)(CTimer::GetTimeInMilliseconds() & 0x7FF) * DEGTORAD(360.0f / 0x800);
		float c = Cos(angle) * scale;
		float s = Sin(angle) * scale;

		// we know from SA they were setting each field manually like this
		entity->GetMatrix().rx = c;
		entity->GetMatrix().ry = s;
		entity->GetMatrix().rz = 0.0f;
		entity->GetMatrix().fx = -s;
		entity->GetMatrix().fy = c;
		entity->GetMatrix().fz = 0.0f;
		entity->GetMatrix().ux = 0.0f;
		entity->GetMatrix().uy = 0.0f;
		entity->GetMatrix().uz = scale;

		if (entity->GetModelIndex() == MI_MINIGUN2) {
			CMatrix matrix1;
			CMatrix matrix2; // unused
			entity->SetPosition(pickup->m_vecPos);
			matrix1.SetRotateX(0.0f);
			matrix1.Rotate(DEGTORAD(4.477f), DEGTORAD(-29.731f), DEGTORAD(-1.064f));
			matrix1.Translate(CVector(0.829f, -0.001f, 0.226f));
			entity->GetMatrix() *= matrix1;
		}

		if (doOuterGlow) {
			CVector scale(0.0f, 0.0f, 0.0f);
			if (colLength.x == maxDimension)
				scale.x = colLength.x;
			else if (colLength.y == maxDimension)
				scale.y = colLength.y;
			else
				scale.z = colLength.z;

			for (int i = 0; i < 4; i++) {
				CVector pos = entity->GetMatrix() * (scale * ((float)i / 3.0f));
				CCoronas::RegisterCorona(
					(uintptr)entity + 8 + i,
					aPickupColors[colorId].r * 0.15f,
					aPickupColors[colorId].g * 0.15f,
					aPickupColors[colorId].b * 0.15f,
					255,
					pos,
					1.0f,
					65.0f,
					CCoronas::TYPE_STAR, CCoronas::FLARE_NONE,
					CCoronas::REFLECTION_OFF,
					CCoronas::LOSCHECK_OFF,
					CCoronas::STREAK_OFF,
					0.0f,
					false,
					-0.5f);
			}
		}

		if (doInnerGlow)
			CCoronas::RegisterCorona(
#ifdef FIX_BUGS
				(uintptr)entity + 8 + 4,
#else
				(uintptr)entity + 9,
#endif
				126, 69, 121, 255, entity->GetPosition(), 1.2f, 50.0f,
				CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
	}
}

void
CPickups::DoMineEffects(CEntity *entity)
{
	const CVector &pos = entity->GetPosition();
	float dist = Distance(pos, TheCamera.GetPosition());
	const float MAXDIST = 20.0f;

	if (dist < MAXDIST) {
		float s = Sin((float)((CTimer::GetTimeInMilliseconds() + (uintptr)entity) & 0x1FF) * DEGTORAD(360.0f / 0x200));

		int32 red = (MAXDIST - dist) * (0.5f * s + 0.5f) / MAXDIST * 64.0f;
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0, red, 0, 0, 4.0f, 1.0f, 40.0f,
		                            false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, red, 0, 0, 255, pos, 0.6f, 60.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	entity->GetMatrix().SetRotateZOnly((float)(CTimer::GetTimeInMilliseconds() & 0x3FF) * DEGTORAD(360.0f / 0x400));
}

void
CPickups::DoMoneyEffects(CEntity *entity)
{
	const CVector &pos = entity->GetPosition();
	float dist = Distance(pos, TheCamera.GetPosition());
	const float MAXDIST = 20.0f;

	if (dist < MAXDIST) {
		float s = Sin((float)((CTimer::GetTimeInMilliseconds() + (uintptr)entity) & 0x3FF) * DEGTORAD(360.0f / 0x400));

		int32 green = (MAXDIST - dist) * (0.2f * s + 0.3f) / MAXDIST * 64.0f;
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0, 0, green, 0, 4.0f, 1.0f,
		                            40.0f, false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, 0, green, 0, 255, pos, 0.4f, 40.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	entity->GetMatrix().SetRotateZOnly((float)(CTimer::GetTimeInMilliseconds() & 0x7FF) * DEGTORAD(360.0f / 0x800));
}

void
CPickups::DoCollectableEffects(CEntity *entity)
{
	const CVector &pos = entity->GetPosition();
	float dist = Distance(pos, TheCamera.GetPosition());
	const float MAXDIST = 14.0f;

	if (dist < MAXDIST) {
		float s = Sin((float)((CTimer::GetTimeInMilliseconds() + (uintptr)entity) & 0x7FF) * DEGTORAD(360.0f / 0x800));

		int32 color = (MAXDIST - dist) * (0.5f * s + 0.5f) / MAXDIST * 255.0f;
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0, color, color, color, 4.0f,
		                            1.0f, 40.0f, false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, color, color, color, 255, pos, 0.6f, 40.0f, CCoronas::TYPE_HEX, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	entity->GetMatrix().SetRotateZOnly((float)(CTimer::GetTimeInMilliseconds() & 0xFFF) * DEGTORAD(360.0f / 0x1000));
}

void
CPickups::RenderPickUpText()
{
	wchar *strToPrint;
	for (int32 i = 0; i < NumMessages; i++) {

		if (aMessages[i].money != 0) {
			sprintf(gString, "$%d", aMessages[i].money);
			AsciiToUnicode(gString, gUString);
			strToPrint = gUString;
		} else {
			switch (aMessages[i].m_quantity) // could use some enum maybe
			{
			case 0:
				if (aMessages[i].m_weaponType == WEAPONTYPE_HEALTH || aMessages[i].m_weaponType == WEAPONTYPE_ARMOUR) {
					strToPrint = nil;
				} else {
					if (aMessages[i].m_bOutOfStock)
						strToPrint = TheText.Get("STOCK");
					else {
						sprintf(gString, "$%d", CostOfWeapon[aMessages[i].m_weaponType]);
						AsciiToUnicode(gString, gUString);
						strToPrint = gUString;
					}
				}
				break;
			case 1:
				strToPrint = TheText.Get("OUTFT1");
				break;
			case 2:
				strToPrint = TheText.Get("OUTFT2");
				break;
			case 3:
				strToPrint = TheText.Get("OUTFT3");
				break;
			case 4:
				strToPrint = TheText.Get("OUTFT4");
				break;
			case 5:
				strToPrint = TheText.Get("OUTFT5");
				break;
			case 6:
				strToPrint = TheText.Get("OUTFT6");
				break;
			case 7:
				strToPrint = TheText.Get("OUTFT7");
				break;
			case 8:
				strToPrint = TheText.Get("OUTFT8");
				break;
			case 9:
				strToPrint = TheText.Get("OUTFT9");
				break;
			case 10:
				strToPrint = TheText.Get("OUTFT10");
				break;
			case 11:
				strToPrint = TheText.Get("OUTFT11");
				break;
			case 12:
				strToPrint = TheText.Get("OUTFT12");
				break;
			case 13:
				strToPrint = TheText.Get("OUTFT13");
				break;
			default:
				break;
			}
		}
		if (strToPrint == nil)
			continue;
		CFont::SetPropOn();
		CFont::SetBackgroundOff();

#ifdef FIX_BUGS
		const float MAX_SCALE = SCREEN_WIDTH / DEFAULT_SCREEN_WIDTH;
#else
		float MAX_SCALE = RsGlobal.width / DEFAULT_SCREEN_WIDTH;
#endif

		float fScaleY = aMessages[i].m_dist.y / 30.0f;
		if (fScaleY > MAX_SCALE) fScaleY = MAX_SCALE;

		float fScaleX = aMessages[i].m_dist.x / 30.0f;
		if (fScaleX > MAX_SCALE) fScaleX = MAX_SCALE;

		CFont::SetScale(fScaleX, fScaleY); // this shouldn't be scaled
		CFont::SetCentreOn();
		CFont::SetCentreSize(SCREEN_WIDTH);
		CFont::SetJustifyOff();

		CFont::SetColor(CRGBA(aMessages[i].m_color.red, aMessages[i].m_color.green, aMessages[i].m_color.blue, aMessages[i].m_color.alpha));
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetFontStyle(FONT_STANDARD);
		CFont::PrintString(aMessages[i].m_pos.x, aMessages[i].m_pos.y, strToPrint);
	}
	NumMessages = 0;
}

void
CPickups::CreateSomeMoney(CVector pos, int money)
{
	bool found;

	int pickupCount = Min(money / 20 + 1, 7);
	int moneyPerPickup = money / pickupCount;

	for (int i = 0; i < pickupCount; i++) {
		// (CGeneral::GetRandomNumber() % 256) * PI / 128 gives a float up to something TWOPI-ish.
		pos.x += 1.5f * Sin((CGeneral::GetRandomNumber() % 256) * PI / 128);
		pos.y += 1.5f * Cos((CGeneral::GetRandomNumber() % 256) * PI / 128);
		pos.z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &found) + 0.5f;
		if (found) {
			CPickups::GenerateNewOne(CVector(pos.x, pos.y, pos.z), MI_MONEY, PICKUP_MONEY, moneyPerPickup + (CGeneral::GetRandomNumber() & 3));
		}
	}
}

void
CPickups::RemoveAllPickupsOfACertainWeaponGroupWithNoAmmo(eWeaponType weaponType)
{
	uint32 weaponSlot = CWeaponInfo::GetWeaponInfo(weaponType)->m_nWeaponSlot;
	if (CWeaponInfo::IsWeaponSlotAmmoMergeable(weaponSlot)) {
		for (int slot = 0; slot < NUMPICKUPS; slot++) {
			if (aPickUps[slot].m_eType == PICKUP_ONCE || aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT || aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT_SLOW) {
				if (aPickUps[slot].m_pObject) {
					if (CWeaponInfo::GetWeaponInfo(WeaponForModel(aPickUps[slot].m_pObject->GetModelIndex()))->m_nWeaponSlot == weaponSlot &&
						aPickUps[slot].m_nQuantity == 0) {
						CWorld::Remove(aPickUps[slot].m_pObject);
						delete aPickUps[slot].m_pObject;
						aPickUps[slot].m_bRemoved = true;
						aPickUps[slot].m_pObject = nil;
						aPickUps[slot].m_eType = PICKUP_NONE;
					}
				}
			}
		}
	}
}

void
CPickups::DetonateMinesHitByGunShot(CVector *vec1, CVector *vec2)
{
	for (int i = 0; i < NUMGENERALPICKUPS; i++) {
		if (aPickUps[i].m_eType == PICKUP_NAUTICAL_MINE_ARMED)
			aPickUps[i].ProcessGunShot(vec1, vec2);
	}
}

void
CPickups::RemoveUnnecessaryPickups(const CVector& center, float radius)
{
	for (int i = 0; i < NUMPICKUPS; i++) {
		if (aPickUps[i].m_eType == PICKUP_ONCE_TIMEOUT || aPickUps[i].m_eType == PICKUP_MONEY) {
			if (Distance(center, aPickUps[i].m_vecPos) < radius) {
				aPickUps[i].GetRidOfObjects();
				aPickUps[i].m_bRemoved = true;
				aPickUps[i].m_eType = PICKUP_NONE;
			}
		}
	}
}

void
CPickups::Load(uint8 *buf, uint32 size)
{
INITSAVEBUF

	for (int32 i = 0; i < NUMPICKUPS; i++) {
		ReadSaveBuf(&aPickUps[i], buf);

		if (aPickUps[i].m_eType != PICKUP_NONE) {
			if (aPickUps[i].m_pObject != nil)
				aPickUps[i].m_pObject = CPools::GetObjectPool()->GetSlot((uintptr)aPickUps[i].m_pObject - 1);
			if (aPickUps[i].m_pExtraObject != nil)
				aPickUps[i].m_pExtraObject = CPools::GetObjectPool()->GetSlot((uintptr)aPickUps[i].m_pExtraObject - 1);
		}
			
	}

	ReadSaveBuf(&CollectedPickUpIndex, buf);
	SkipSaveBuf(buf, 2);
	NumMessages = 0;

	for (uint16 i = 0; i < NUMCOLLECTEDPICKUPS; i++)
		ReadSaveBuf(&aPickUpsCollected[i], buf);

VALIDATESAVEBUF(size)
}

void
CPickups::Save(uint8 *buf, uint32 *size)
{
	*size = sizeof(aPickUps);
	*size += sizeof(uint16) + sizeof(uint16) + sizeof(aPickUpsCollected);

INITSAVEBUF

	for (int32 i = 0; i < NUMPICKUPS; i++) {
		CPickup *buf_pickup = WriteSaveBuf(buf, aPickUps[i]);
		if (buf_pickup->m_eType != PICKUP_NONE) {
			if (buf_pickup->m_pObject != nil)
				buf_pickup->m_pObject = (CObject*)(CPools::GetObjectPool()->GetJustIndex_NoFreeAssert(buf_pickup->m_pObject) + 1);
			if (buf_pickup->m_pExtraObject != nil)
				buf_pickup->m_pExtraObject = (CObject*)(CPools::GetObjectPool()->GetJustIndex_NoFreeAssert(buf_pickup->m_pExtraObject) + 1);
		}
	}

	WriteSaveBuf(buf, CollectedPickUpIndex);
	WriteSaveBuf(buf, (uint16)0); // possibly was NumMessages

	for (uint16 i = 0; i < NUMCOLLECTEDPICKUPS; i++)
		WriteSaveBuf(buf, aPickUpsCollected[i]);

VALIDATESAVEBUF(*size)
}

void
CPacManPickup::Update()
{
}

int32 CollectGameState;
int16 ThingsToCollect;

CPacManPickup CPacManPickups::aPMPickUps[NUMPACMANPICKUPS];
CVector CPacManPickups::LastPickUpCoors;
int32 CPacManPickups::PillsEatenInRace;
bool CPacManPickups::bPMActive;

void
CPacManPickups::Init()
{
}

void
CPacManPickups::Update()
{
}

void
CPacManPickups::GeneratePMPickUps(CVector pos, float scrambleMult, int16 count, uint8 type)
{
}

// diablo porn mission pickups
static const CVector aRacePoints1[] = {
	CVector(913.62219f, -155.13692f, 4.9699469f),
	CVector(913.92401f, -124.12943f, 4.9692569f),
	CVector(913.27899f, -93.524231f, 7.4325991f),
	CVector(912.60852f, -63.15905f, 7.4533591f),
	CVector(934.22144f, -42.049122f, 7.4511471f),
	CVector(0.0f, 0.0f, 0.0f),
};

void
CPacManPickups::GeneratePMPickUpsForRace(int32 race)
{
}

void
CPacManPickups::GenerateOnePMPickUp(CVector pos)
{
}

void
CPacManPickups::Render()
{
}

void
CPacManPickups::ClearPMPickUps()
{
}

void
CPacManPickups::StartPacManRace(int32 race)
{
}

void
CPacManPickups::StartPacManRecord()
{
}

uint32
CPacManPickups::QueryPowerPillsEatenInRace()
{
	return 0;
}

void
CPacManPickups::ResetPowerPillsEatenInRace()
{
}

void
CPacManPickups::CleanUpPacManStuff()
{
}

void
CPacManPickups::StartPacManScramble(CVector pos, float scrambleMult, int16 count)
{
}

uint32
CPacManPickups::QueryPowerPillsCarriedByPlayer()
{
	return 0;
}

void
CPacManPickups::ResetPowerPillsCarriedByPlayer()
{
}

void
CPed::CreateDeadPedMoney(void)
{
	if (!CGame::nastyGame)
		return;

	int mi = GetModelIndex();

	if ((mi >= MI_COP && mi <= MI_FIREMAN) || (CharCreatedBy == MISSION_CHAR && !bMoneyHasBeenGivenByScript) || bInVehicle)
		return;

	int money = m_nPedMoney;
	if (money < 10)
		return;

	CVector pickupPos = GetPosition();
	CPickups::CreateSomeMoney(pickupPos, money);
	m_nPedMoney = 0;
}

void
CPed::CreateDeadPedWeaponPickups(void)
{
	CVector pickupPos;

	if (bInVehicle)
		return;

	for(int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {

		eWeaponType weapon = GetWeapon(i).m_eWeaponType;
		int weaponAmmo = GetWeapon(i).m_nAmmoTotal;
		if (weapon == WEAPONTYPE_UNARMED || weapon == WEAPONTYPE_DETONATOR || (weaponAmmo == 0 && !GetWeapon(i).IsTypeMelee()))
			continue;

		int quantity = Min(weaponAmmo, AmmoForWeapon_OnStreet[weapon] / 2);
		CreateDeadPedPickupCoors(&pickupPos.x, &pickupPos.y, &pickupPos.z);
		pickupPos.z += 0.3f;
		if (!CPickups::TryToMerge_WeaponType(pickupPos, weapon, PICKUP_ONCE_TIMEOUT, quantity, false)) {
			CPickups::GenerateNewOne_WeaponType(pickupPos, weapon, PICKUP_ONCE_TIMEOUT, Min(weaponAmmo, quantity));
		}
	}
	ClearWeapons();
}

void
CPed::CreateDeadPedPickupCoors(float *x, float *y, float *z)
{
	bool found = false;
	CVector pickupPos;

#define NUMBER_OF_ATTEMPTS 32
	for (int i = 0; i < NUMBER_OF_ATTEMPTS; i++) {

		pickupPos = GetPosition();
		pickupPos.x = 1.5f * Sin((CGeneral::GetRandomNumber() % 256)/256.0f * TWOPI) + GetPosition().x;
		pickupPos.y = 1.5f * Cos((CGeneral::GetRandomNumber() % 256)/256.0f * TWOPI) + GetPosition().y;
		pickupPos.z = CWorld::FindGroundZFor3DCoord(pickupPos.x, pickupPos.y, pickupPos.z, &found) + 0.5f;

		if (!found)
			continue;

		CVector pedPos = GetPosition();
		pedPos.z += 0.3f;

		CVector pedToPickup = pickupPos - pedPos;
		float distance = pedToPickup.Magnitude();

		// outer edge of pickup
		distance = (distance + 0.4f) / distance;
		CVector pickupPos2 = pedPos;
		pickupPos2 += distance * pedToPickup;

		if ((pickupPos - FindPlayerCoors()).Magnitude2D() > 2.0f || i > NUMBER_OF_ATTEMPTS / 2) {

			if (i > NUMBER_OF_ATTEMPTS / 2 || !CPickups::TestForPickupsInBubble(pickupPos, 1.3f)) {

				if (CWorld::GetIsLineOfSightClear(pickupPos2, pedPos,
					true, i < NUMBER_OF_ATTEMPTS / 2, false, i < NUMBER_OF_ATTEMPTS / 2, false, false, false)) {

					if (i > NUMBER_OF_ATTEMPTS / 2 || !CWorld::TestSphereAgainstWorld(pickupPos, 1.2f, nil, false, true, false, false, false, false)) {
						*x = pickupPos.x;
						*y = pickupPos.y;
						*z = pickupPos.z;
						return;
					}
				}
			}
		}
	}
	*x = GetPosition().x;
	*y = GetPosition().y;
	*z = GetPosition().z + 0.4f;
#undef NUMBER_OF_ATTEMPTS
}