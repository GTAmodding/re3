#include "common.h"
#include "patcher.h"
#include "main.h"
#include "Pickups.h"
#include "Camera.h"
#include "Entity.h"
#include "Timer.h"
#include "Shadows.h"
#include "Coronas.h"
#include "World.h"
#include "ModelIndices.h"
#include "PlayerPed.h"
#include "Object.h"
#include "Pools.h"
#include "Pad.h"
#include "Script.h"
#include "Darkel.h"
#include "Garages.h"
#include "Explosion.h"
#include "WaterLevel.h"
#include "SpecialFX.h"
#include "PointLights.h"
#include "Sprite.h"
#include "Font.h"

CPickup(&CPickups::aPickUps)[NUMPICKUPS] = *(CPickup(*)[NUMPICKUPS])*(uintptr*)0x878C98;
int16 CPickups::NumMessages;// = *(int16*)0x95CC98;
int32 CPickups::aPickUpsCollected[NUMCOLLECTEDPICKUPS];// = *(int32(*)[NUMCOLLECTEDPICKUPS])*(uintptr*)0x87C538;
int16 CPickups::CollectedPickUpIndex;// = *(int16*)0x95CC8A;

// unused
bool &CPickups::bPickUpcamActivated = *(bool*)0x95CD71;
CVehicle *&CPickups::pPlayerVehicle = *(CVehicle**)0x8F29E8;
CVector &CPickups::StaticCamCoors = *(CVector*)0x9404C8;
uint32 &CPickups::StaticCamStartTime = *(uint32*)0x8E289C;

tPickupMessage CPickups::aMessages[NUMPICKUPMESSAGES];

// 20 ?! Some Miami leftover? (Originally at 0x5ED8D4)
uint16 AmmoForWeapon[20] = { 0, 1, 45, 125, 25, 150, 300, 25, 5, 250, 5, 5, 0, 500, 0, 100, 0, 0, 0, 0 };
uint16 AmmoForWeapon_OnStreet[20] = { 0, 1, 9, 25, 5, 30, 60, 5, 1, 50, 1, 1, 0, 200, 0, 100, 0, 0, 0, 0 };
uint16 CostOfWeapon[20] = { 0, 10, 250, 800, 1500, 3000, 5000, 10000, 25000, 25000, 2000, 2000, 0, 50000, 0, 3000, 0, 0, 0, 0 };

uint8 aWeaponReds[] = { 255, 0, 128, 255, 255, 0, 255, 0, 128, 128, 255, 255, 128, 0, 255, 0 };
uint8 aWeaponGreens[] = { 0, 255, 128, 255, 0, 255, 128, 255, 0, 255, 255, 0, 255, 0, 255, 0 };
uint8 aWeaponBlues[] = { 0, 0, 255, 0, 255, 255, 0, 128, 255, 0, 255, 0, 128, 255, 0, 0 };
float aWeaponScale[] = { 1.0f, 2.0f, 1.5f, 1.0f, 1.0f, 1.5f, 1.0f, 2.0f, 1.0f, 2.0f, 2.5f, 1.0f, 1.0f, 1.0f, 1.0f };

WRAPPER void CPacManPickups::Render(void) { EAXJMP(0x432F60); }
WRAPPER void CPacManPickups::Update(void) { EAXJMP(0x432800); }


void
CPickup::RemoveKeepType()
{
	CWorld::Remove(m_pObject);
	delete m_pObject;

	m_bRemoved = true;
	m_pObject = nil;
}

void
CPickup::Remove()
{
	RemoveKeepType();
	m_eType = PICKUP_NONE;
}

CObject *
CPickup::GiveUsAPickUpObject(int32 handle)
{
	CObject *object;

	if (handle <= 0) object = new CObject(m_eModelIndex, false);
	else {
		CPools::MakeSureSlotInObjectPoolIsEmpty(handle);
		object = new(handle) CObject(m_eModelIndex, false);
	}

	if (object == nil) return nil;
	object->ObjectCreatedBy = MISSION_OBJECT;
	object->GetPosition() = m_vecPos;
	object->SetOrientation(0.0f, 0.0f, -HALFPI);
	object->GetMatrix().UpdateRW();
	object->UpdateRwFrame();

	object->bAffectedByGravity = false;
	object->bExplosionProof = true;
	object->bUsesCollision = false;
	object->bIsPickup = true;

	object->field_172 = m_eModelIndex == MI_PICKUP_BONUS ? m_nQuantity : 0;

	switch (m_eType)
	{
	case PICKUP_IN_SHOP:
		object->m_obj_flag2 = true;
		object->bOutOfStock = false;
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
		object->m_obj_flag2 = false;
		object->bOutOfStock = false;
		break;
	case PICKUP_IN_SHOP_OUT_OF_STOCK:
		object->m_obj_flag2 = false;
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
CPickup::CanBePickedUp(CPlayerPed *player)
{
	assert(m_pObject != nil);
	bool cannotBePickedUp =
		(m_pObject->GetModelIndex() == MI_PICKUP_BODYARMOUR && player->m_fArmour > 99.5f)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_HEALTH && player->m_fHealth > 99.5f)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_BRIBE && player->m_pWanted->m_nWantedLevel == 0)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_KILLFRENZY && (CTheScripts::IsPlayerOnAMission() || CDarkel::FrenzyOnGoing() || !CGame::nastyGame));
	return !cannotBePickedUp;
}

bool
CPickup::Update(CPlayerPed *player, CVehicle *vehicle, int playerId)
{
	float waterLevel;

	if (m_bRemoved) {
		if (CTimer::GetTimeInMilliseconds() > m_nTimer) {
			// respawn pickup if we're far enough
			float dist = (FindPlayerCoors().x - m_vecPos.x) * (FindPlayerCoors().x - m_vecPos.x) + (FindPlayerCoors().y - m_vecPos.y) * (FindPlayerCoors().y - m_vecPos.y);
			if (dist > 100.0f || m_eType == PICKUP_IN_SHOP && dist > 2.4f) {
				m_pObject = GiveUsAPickUpObject(-1);
				if (m_pObject) {
					CWorld::Add(m_pObject);
					m_bRemoved = false;
				}
			}
		}
		return false;
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

		// if we didn't then we've got nothing to do
		if (isPickupTouched && CanBePickedUp(player)) {
			CPad::GetPad(0)->StartShake(120, 100);
			switch (m_eType)
			{
			case PICKUP_IN_SHOP:
				if (CWorld::Players[playerId].m_nMoney < CostOfWeapon[CPickups::WeaponForModel(m_pObject->GetModelIndex())]) {
					CGarages::TriggerMessage("PU_MONY", -1, 6000, -1);
				} else {
					CWorld::Players[playerId].m_nMoney -= CostOfWeapon[CPickups::WeaponForModel(m_pObject->GetModelIndex())];
					if (!CPickups::GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
						player->GiveWeapon(CPickups::WeaponForModel(m_pObject->GetModelIndex()), AmmoForWeapon[CPickups::WeaponForModel(m_pObject->GetModelIndex())]);
						player->m_nSelectedWepSlot = player->GetWeaponSlot(CPickups::WeaponForModel(m_pObject->GetModelIndex()));
						DMAudio.PlayFrontEndSound(SOUND_PICKUP_WEAPON_BOUGHT, m_pObject->GetModelIndex() - MI_GRENADE);
					}
					RemoveKeepType();
					m_nTimer = CTimer::GetTimeInMilliseconds() + 5000;
					return true;
				}
				break;
			case PICKUP_ON_STREET:
			case PICKUP_ON_STREET_SLOW:
				if (!CPickups::GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
					if (CPickups::WeaponForModel(m_pObject->GetModelIndex())) {
						player->GiveWeapon(CPickups::WeaponForModel(m_pObject->GetModelIndex()), m_nQuantity != 0 ? m_nQuantity : AmmoForWeapon_OnStreet[CPickups::WeaponForModel(m_pObject->GetModelIndex())]);
						if (player->m_nSelectedWepSlot == player->GetWeaponSlot(WEAPONTYPE_UNARMED)) {
							player->m_nSelectedWepSlot = player->GetWeaponSlot(CPickups::WeaponForModel(m_pObject->GetModelIndex()));
						}
						DMAudio.PlayFrontEndSound(SOUND_PICKUP_WEAPON, m_pObject->GetModelIndex() - MI_GRENADE);
					} else if (m_pObject->GetModelIndex() == MI_PICKUP_CAMERA && vehicle != nil) {
						DMAudio.PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
						CPickups::bPickUpcamActivated = true;
						CPickups::pPlayerVehicle = FindPlayerVehicle();
						CPickups::StaticCamCoors = m_pObject->GetPosition();
						CPickups::StaticCamStartTime = CTimer::GetTimeInMilliseconds();
					}
				}
				if (m_eType == PICKUP_ON_STREET) {
					m_nTimer = CTimer::GetTimeInMilliseconds() + 30000;
				} else if (m_eType == PICKUP_ON_STREET_SLOW) {
					if (MI_PICKUP_BRIBE == m_pObject->m_modelIndex)
						m_nTimer = CTimer::GetTimeInMilliseconds() + 300000;
					else
						m_nTimer = CTimer::GetTimeInMilliseconds() + 720000;
				}

				RemoveKeepType();
				return true;
			case PICKUP_ONCE:
			case PICKUP_ONCE_TIMEOUT:
				if (!CPickups::GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
					if (CPickups::WeaponForModel(m_pObject->GetModelIndex())) {
						player->GiveWeapon(CPickups::WeaponForModel(m_pObject->GetModelIndex()), m_nQuantity != 0 ? m_nQuantity : AmmoForWeapon[CPickups::WeaponForModel(m_pObject->GetModelIndex())]);
						if (player->m_nSelectedWepSlot == player->GetWeaponSlot(WEAPONTYPE_UNARMED))
							player->m_nSelectedWepSlot = player->GetWeaponSlot(CPickups::WeaponForModel(m_pObject->GetModelIndex()));
					}
					DMAudio.PlayFrontEndSound(SOUND_PICKUP_WEAPON, m_pObject->GetModelIndex() - MI_GRENADE);
				}
				Remove();
				return true;
			case PICKUP_COLLECTABLE1:
				CWorld::Players[playerId].m_nCollectedPackages++;
				CWorld::Players[playerId].m_nMoney += 1000;

				if (CWorld::Players[playerId].m_nCollectedPackages == CWorld::Players[playerId].m_nTotalPackages) {
					printf("All collectables have been picked up\n");
					CGarages::TriggerMessage("CO_ALL", -1, 5000, -1);
					CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 1000000;
				} else
					CGarages::TriggerMessage("CO_ONE", CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages, 5000, CWorld::Players[CWorld::PlayerInFocus].m_nTotalPackages);

				Remove();
				DMAudio.PlayFrontEndSound(SOUND_PICKUP_HIDDEN_PACKAGE, 0);
				return true;
			case PICKUP_MONEY:
				CWorld::Players[playerId].m_nMoney += m_nQuantity;
				sprintf(gString, "$%d", m_nQuantity);
#ifdef MONEY_MESSAGES
				CMoneyMessages::RegisterOne(m_vecPos + CVector(0.0f, 0.0f, 1.0f), gString, 0, 255, 0, 0.5f, 0.5f);
#endif
				Remove();
				DMAudio.PlayFrontEndSound(SOUND_PICKUP_MONEY, 0);
				return true;
			//case PICKUP_IN_SHOP_OUT_OF_STOCK:
			//case PICKUP_MINE_INACTIVE:
			//case PICKUP_MINE_ARMED:
			//case PICKUP_NAUTICAL_MINE_INACTIVE:
			//case PICKUP_NAUTICAL_MINE_ARMED:
			//case PICKUP_FLOATINGPACKAGE:
			//case PICKUP_FLOATINGPACKAGE_FLOATING:
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
				m_pObject->GetPosition().z = waterLevel + 0.6f;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();

			bool touched = false;
			for (int32 i = CPools::GetVehiclePool()->GetSize(); i > 0; i--) { // TODO: check if i > 0 is not a R* mistake
				CVehicle *vehicle = CPools::GetVehiclePool()->GetSlot(i);
				if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 1.5f)) {
					touched = true;
					break; // added break here
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
				m_pObject->GetPosition().z = waterLevel + 0.6f;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			// no break here
		case PICKUP_MINE_ARMED:
		{
			bool explode = false;
			if (CTimer::GetTimeInMilliseconds() > m_nTimer)
				explode = true;
			else {// added else here since vehicle lookup is useless
				for (int32 i = CPools::GetVehiclePool()->GetSize(); i > 0; i--) { // TODO: check if i > 0 is not a R* mistake
					CVehicle *vehicle = CPools::GetVehiclePool()->GetSlot(i);
					if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 1.5f)) {
						explode = true;
						break; // added break here
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
			m_pObject->GetPosition() += m_pObject->GetMoveSpeed() * CTimer::GetTimeStep();

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			if (CWaterLevel::GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, 0) && waterLevel >= m_pObject->GetPosition().z)
				m_eType = PICKUP_FLOATINGPACKAGE_FLOATING;
			break;
		case PICKUP_FLOATINGPACKAGE_FLOATING:
			if (CWaterLevel::GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, 0))
				m_pObject->GetPosition().z = waterLevel;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f)) {
				Remove();
				DMAudio.PlayFrontEndSound(SOUND_PICKUP_FLOAT_PACKAGE, 0);
				return true;
			}
			break;
		}
	}
	if (!m_bRemoved && (m_eType == PICKUP_ONCE_TIMEOUT || m_eType == PICKUP_MONEY) && CTimer::GetTimeInMilliseconds() > m_nTimer)
		Remove();
	return false;
}

void
CPickups::Init(void)
{
	NumMessages = 0;
	for (int i = 0; i < NUMPICKUPS; i++) {
		aPickUps[i].m_eType = PICKUP_NONE;
		aPickUps[i].m_nIndex = 1;
		aPickUps[i].m_pObject = nil;
	}

	for (int i = 0; i < NUMCOLLECTEDPICKUPS; i++)
		aPickUpsCollected[i] = 0;

	CollectedPickUpIndex = 0;
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
		if (aPickUps[i].m_eType != PICKUP_NONE) {
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
		player->m_fArmour = 100.0f;
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_INFO) {
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_HEALTH) {
		player->m_fHealth = 100.0f;
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_HEALTH, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_BONUS) {
		DMAudio.PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_BRIBE) {
		int32 level = FindPlayerPed()->m_pWanted->m_nWantedLevel - 1;
		if (level < 0) level = 0;
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
CPickups::RemoveAllFloatingPickups()
{
	for (int i = 0; i < NUMPICKUPS; i++) {
		if (aPickUps[i].m_eType == PICKUP_FLOATINGPACKAGE || aPickUps[i].m_eType == PICKUP_FLOATINGPACKAGE_FLOATING) {
			if (aPickUps[i].m_pObject) {
				CWorld::Remove(aPickUps[i].m_pObject);
				delete aPickUps[i].m_pObject;
				aPickUps[i].m_pObject = nil;
			}
		}
	}
}

void
CPickups::RemovePickUp(int32 pickupIndex)
{
	int32 index = CPickups::GetActualPickupIndex(pickupIndex);
	if (index == -1) return;

	if (aPickUps[index].m_pObject) {
		CWorld::Remove(aPickUps[index].m_pObject);
		delete aPickUps[index].m_pObject;
		aPickUps[index].m_pObject = nil;
	}
	aPickUps[index].m_eType = PICKUP_NONE;
	aPickUps[index].m_bRemoved = true;
}

int32
CPickups::GenerateNewOne(CVector pos, uint32 modelIndex, uint8 type, uint32 quantity)
{
	bool bFreeFound = false;
	int32 slot = 0;

	if (type == PICKUP_FLOATINGPACKAGE || type == PICKUP_NAUTICAL_MINE_INACTIVE) {
		for (slot = NUMPICKUPS; slot >= 0; slot--) {
			if (aPickUps[slot].m_eType == PICKUP_NONE) {
				bFreeFound = true;
				break;
			}
		}
	} else {
		for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
			if (aPickUps[slot].m_eType == PICKUP_NONE) {
				bFreeFound = true;
				break;
			}
		}
	}

	if (!bFreeFound)
	{
		for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
			if (aPickUps[slot].m_eType == PICKUP_MONEY) break;
		}

		if (slot >= NUMGENERALPICKUPS)
		{
			for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
				if (aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT) break;
			}

			if (slot >= NUMGENERALPICKUPS) return -1;
		}
	}

	if (slot >= NUMPICKUPS) return -1;

	aPickUps[slot].m_eType = (ePickupType)type;
	aPickUps[slot].m_bRemoved = false;
	aPickUps[slot].m_nQuantity = quantity;
	if (type == PICKUP_ONCE_TIMEOUT)
		aPickUps[slot].m_nTimer = CTimer::GetTimeInMilliseconds() + 20000;
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
	aPickUps[slot].m_vecPos = pos;
	aPickUps[slot].m_pObject = aPickUps[slot].GiveUsAPickUpObject(-1);
	if (aPickUps[slot].m_pObject)
		CWorld::Add(aPickUps[slot].m_pObject);
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
	switch (weaponType)
	{
	case WEAPONTYPE_BASEBALLBAT: return MI_BASEBALL_BAT;
	case WEAPONTYPE_COLT45: return MI_COLT;
	case WEAPONTYPE_UZI: return MI_UZI;
	case WEAPONTYPE_SHOTGUN: return MI_SHOTGUN;
	case WEAPONTYPE_AK47: return MI_AK47;
	case WEAPONTYPE_M16: return MI_M16;
	case WEAPONTYPE_SNIPERRIFLE: return MI_SNIPER;
	case WEAPONTYPE_ROCKETLAUNCHER: return MI_ROCKETLAUNCHER;
	case WEAPONTYPE_FLAMETHROWER: return MI_FLAMETHROWER;
	case WEAPONTYPE_MOLOTOV: return MI_MOLOTOV;
	case WEAPONTYPE_GRENADE: return MI_GRENADE;
	}
	return 0;
}

eWeaponType
CPickups::WeaponForModel(int32 model)
{
	if (model == MI_PICKUP_BODYARMOUR) return WEAPONTYPE_ARMOUR;
	switch (model)
	{
	case MI_GRENADE: return WEAPONTYPE_GRENADE;
	case MI_AK47: return WEAPONTYPE_AK47;
	case MI_BASEBALL_BAT: return WEAPONTYPE_BASEBALLBAT;
	case MI_COLT: return WEAPONTYPE_COLT45;
	case MI_MOLOTOV: return WEAPONTYPE_MOLOTOV;
	case MI_ROCKETLAUNCHER: return WEAPONTYPE_ROCKETLAUNCHER;
	case MI_SHOTGUN: return WEAPONTYPE_SHOTGUN;
	case MI_SNIPER: return WEAPONTYPE_SNIPERRIFLE;
	case MI_UZI: return WEAPONTYPE_UZI;
	case MI_MISSILE: return WEAPONTYPE_UNARMED;
	case MI_M16: return WEAPONTYPE_M16;
	case MI_FLAMETHROWER: return WEAPONTYPE_FLAMETHROWER;
	}
	return WEAPONTYPE_UNARMED;
}

int32
CPickups::FindColourIndexForWeaponMI(int32 model)
{
	return WeaponForModel(model) - 1;
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
#ifndef FIX_BUGS
	// BUG: this code can only reach 318 out of 320 pickups
#define PICKUPS_FRAME_SPAN (6)
#define PICKUPS_PER_FRAME (NUMGENERALPICKUPS/PICKUPS_FRAME_SPAN)

	for (uint32 i = PICKUPS_PER_FRAME * (CTimer::GetFrameCounter() % PICKUPS_FRAME_SPAN); i < PICKUPS_PER_FRAME * (CTimer::GetFrameCounter() % PICKUPS_FRAME_SPAN + 1); i++) {
		if (aPickUps[i].m_eType != PICKUP_NONE && aPickUps[i].Update(FindPlayerPed(), FindPlayerVehicle(), CWorld::PlayerInFocus)) {
			AddToCollectedPickupsArray(i);
		}
	}

	for (uint32 i = NUMGENERALPICKUPS; i < NUMPICKUPS; i++) {
		if (aPickUps[i].m_eType != PICKUP_NONE && aPickUps[i].Update(FindPlayerPed(), FindPlayerVehicle(), CWorld::PlayerInFocus)) {
			AddToCollectedPickupsArray(i);
		}
	}

#undef PICKUPS_FRAME_SPAN
#undef PICKUPS_PER_FRAME
#else
	for (uint32 i = 0; i < NUMPICKUPS; i++) {
		if (aPickUps[i].m_eType != PICKUP_NONE && aPickUps[i].Update(FindPlayerPed(), FindPlayerVehicle(), CWorld::PlayerInFocus)) {
			AddToCollectedPickupsArray(i);
		}
	}
#endif
}

void
CPickups::DoPickUpEffects(CEntity *entity)
{
	if (entity->GetModelIndex() == MI_PICKUP_KILLFRENZY)
		entity->m_flagD80 = CTheScripts::IsPlayerOnAMission() || CDarkel::FrenzyOnGoing() || !CGame::nastyGame;

	if (!entity->m_flagD80) {
		float s = Sin((float)((CTimer::GetTimeInMilliseconds() + (uintptr)entity) & 0x7FF) * DEGTORAD(360.0f / 0x800));
		float modifiedSin = 0.3 * (s + 1.0f);


		int16 colorId;

		if (entity->GetModelIndex() == MI_PICKUP_ADRENALINE || entity->GetModelIndex() == MI_PICKUP_CAMERA)
			colorId = 11;
		else if (entity->GetModelIndex() == MI_PICKUP_BODYARMOUR || entity->GetModelIndex() == MI_PICKUP_BRIBE)
			colorId = 12;
		else if (entity->GetModelIndex() == MI_PICKUP_INFO || entity->GetModelIndex() == MI_PICKUP_KILLFRENZY)
			colorId = 13;
		else if (entity->GetModelIndex() == MI_PICKUP_HEALTH || entity->GetModelIndex() == MI_PICKUP_BONUS)
			colorId = 14;
		else
			colorId = FindColourIndexForWeaponMI(entity->GetModelIndex());

		assert(colorId >= 0);

		CVector &pos = entity->GetPosition();

		float colorModifier = ((double)(rand() & 0x1F) * 0.015f + 1.0f) * modifiedSin * 0.15f;
		CShadows::StoreStaticShadow(
			(uintptr)entity,
			SHADOWTYPE_ADDITIVE,
			gpShadowExplosionTex,
			&pos,
			2.0f, 0.0f, 0.0f, -2.0f,
			255,                                        // this is 0 on PC which results in no shadow
			aWeaponReds[colorId] * colorModifier, aWeaponGreens[colorId] * colorModifier, aWeaponBlues[colorId] * colorModifier,
			4.0f, 1.0f, 40.0f, false, 0.0f);

		float radius = (double)(rand() & 0xF) * 0.1 + 3.0;
		CPointLights::AddLight(CPointLights::LIGHT_POINT, pos, CVector(0.0f, 0.0f, 0.0f), radius, aWeaponReds[colorId] * modifiedSin / 256.0f, aWeaponGreens[colorId] * modifiedSin / 256.0f, aWeaponBlues[colorId] * modifiedSin / 256.0f, CPointLights::FOG_NONE, true);
		float size = (double)(rand() & 0xF) * 0.0005 + 0.6;
		CCoronas::RegisterCorona( (uintptr)entity,
			aWeaponReds[colorId] * modifiedSin / 2.0f, aWeaponGreens[colorId] * modifiedSin / 2.0f, aWeaponBlues[colorId] * modifiedSin / 2.0f,
			255,
			pos,
			size, 65.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);

		CObject *object = (CObject*)entity;
		if (object->m_obj_flag2 || object->bOutOfStock || object->field_172) {
			float dist = (TheCamera.GetPosition() - pos).Magnitude();
			const float MAXDIST = 12.0f;

			if (dist < MAXDIST && NumMessages < NUMPICKUPMESSAGES) {
				RwV3d vecOut;
				float fDistX, fDistY;
				if (CSprite::CalcScreenCoors(entity->GetPosition() + CVector(0.0f, 0.0f, 0.7f), &vecOut, &fDistX, &fDistY, true)) {
					aMessages[NumMessages].m_pos.x = vecOut.x;
					aMessages[NumMessages].m_pos.y = vecOut.y;
					aMessages[NumMessages].m_dist.x = fDistX;
					aMessages[NumMessages].m_dist.y = fDistY;
					aMessages[NumMessages].m_weaponType = WeaponForModel(entity->GetModelIndex());
					aMessages[NumMessages].m_color.red = aWeaponReds[colorId];
					aMessages[NumMessages].m_color.green = aWeaponGreens[colorId];
					aMessages[NumMessages].m_color.blue = aWeaponBlues[colorId];
					aMessages[NumMessages].m_color.alpha = (1.0f - dist / MAXDIST) * 128.0f;
					aMessages[NumMessages].m_bOutOfStock = object->bOutOfStock;
					aMessages[NumMessages].m_quantity = object->field_172;
					NumMessages++;
				}
			}
		}

		entity->GetMatrix().SetRotateZOnlyScaled((float)(CTimer::GetTimeInMilliseconds() & 0x7FF) * DEGTORAD(360.0f / 0x800), aWeaponScale[colorId]);
	}
}

void
CPickups::DoMineEffects(CEntity *entity)
{
	CVector &pos = entity->GetPosition();
	float dist = (TheCamera.GetPosition() - pos).Magnitude();
	const float MAXDIST = 20.0f;

	if (dist < MAXDIST) {
		float s = Sin((float)((CTimer::GetTimeInMilliseconds() + (uintptr)entity) & 0x1FF) * DEGTORAD(360.0f / 0x200));

		int32 red = (MAXDIST - dist) * (0.5f * s + 0.5f) / MAXDIST * 64.0f;
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos,
			2.0f, 0.0f, 0.0f, -2.0f,
			255,                                        // this is 0 on PC which results in no shadow
			red, 0, 0,
			4.0f, 1.0f, 40.0f, false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, red, 0, 0, 255, pos, 0.6f, 60.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	entity->GetMatrix().SetRotateZOnly((float)(CTimer::GetTimeInMilliseconds() & 0x3FF) * DEGTORAD(360.0f / 0x400));
}

void
CPickups::DoMoneyEffects(CEntity *entity)
{
	CVector &pos = entity->GetPosition();
	float dist = (TheCamera.GetPosition() - pos).Magnitude();
	const float MAXDIST = 20.0f;

	if (dist < MAXDIST) {
		float s = Sin((float)((CTimer::GetTimeInMilliseconds() + (uintptr)entity) & 0x3FF) * DEGTORAD(360.0f / 0x400));

		int32 green = (MAXDIST - dist) * (0.2f * s + 0.3f) / MAXDIST * 64.0f;
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos,
			2.0f, 0.0f, 0.0f, -2.0f,
			255,                                        // this is 0 on PC which results in no shadow
			0, green, 0,
			4.0f, 1.0f, 40.0f, false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, 0, green, 0, 255, pos, 0.4f, 40.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	entity->GetMatrix().SetRotateZOnly((float)(CTimer::GetTimeInMilliseconds() & 0x7FF) * DEGTORAD(360.0f / 0x800));
}

void
CPickups::DoCollectableEffects(CEntity *entity)
{
	CVector &pos = entity->GetPosition();
	float dist = (TheCamera.GetPosition() - pos).Magnitude();
	const float MAXDIST = 14.0f;

	if (dist < MAXDIST) {
		float s = Sin((float)((CTimer::GetTimeInMilliseconds() + (uintptr)entity) & 0x7FF) * DEGTORAD(360.0f / 0x800));

		int32 color = (MAXDIST - dist) * (0.5f * s + 0.5f) / MAXDIST * 255.0f;
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos,
			2.0f, 0.0f, 0.0f, -2.0f,
			255,                                        // this is 0 on PC which results in no shadow
			color, color, color,
			4.0f, 1.0f, 40.0f, false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, color, color, color, 255, pos, 0.6f, 40.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	entity->GetMatrix().SetRotateZOnly((float)(CTimer::GetTimeInMilliseconds() & 0xFFF) * DEGTORAD(360.0f / 0x1000));
}

void
CPickups::RenderPickUpText()
{
	wchar *strToPrint;
	for (int32 i = 0; i < NumMessages; i++) {
		if (aMessages[i].m_quantity <= 39) {
			switch (aMessages[i].m_quantity) // could use some enum maybe
			{
			case 0:
				if (aMessages[i].m_weaponType == WEAPONTYPE_TOTALWEAPONS) { // unreachable code?
					// what is this??
					sprintf(gString, "%d/%d", CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages, 2903);
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
				strToPrint = TheText.Get("SECURI");
				break;
			case 2:
				strToPrint = TheText.Get("MOONBM");
				break;
			case 3:
				strToPrint = TheText.Get("COACH");
				break;
			case 4:
				strToPrint = TheText.Get("FLATBED");
				break;
			case 5:
				strToPrint = TheText.Get("LINERUN");
				break;
			case 6:
				strToPrint = TheText.Get("TRASHM");
				break;
			case 7:
				strToPrint = TheText.Get("PATRIOT");
				break;
			case 8:
				strToPrint = TheText.Get("WHOOPEE");
				break;
			case 9:
				strToPrint = TheText.Get("BLISTA");
				break;
			case 10:
				strToPrint = TheText.Get("MULE");
				break;
			case 11:
				strToPrint = TheText.Get("YANKEE");
				break;
			case 12:
				strToPrint = TheText.Get("BOBCAT");
				break;
			case 13:
				strToPrint = TheText.Get("DODO");
				break;
			case 14:
				strToPrint = TheText.Get("BUS");
				break;
			case 15:
				strToPrint = TheText.Get("RUMPO");
				break;
			case 16:
				strToPrint = TheText.Get("PONY");
				break;
			case 17:
				strToPrint = TheText.Get("SENTINL");
				break;
			case 18:
				strToPrint = TheText.Get("CHEETAH");
				break;
			case 19:
				strToPrint = TheText.Get("BANSHEE");
				break;
			case 20:
				strToPrint = TheText.Get("IDAHO");
				break;
			case 21:
				strToPrint = TheText.Get("INFERNS");
				break;
			case 22:
				strToPrint = TheText.Get("TAXI");
				break;
			case 23:
				strToPrint = TheText.Get("KURUMA");
				break;
			case 24:
				strToPrint = TheText.Get("STRETCH");
				break;
			case 25:
				strToPrint = TheText.Get("PEREN");
				break;
			case 26:
				strToPrint = TheText.Get("STINGER");
				break;
			case 27:
				strToPrint = TheText.Get("MANANA");
				break;
			case 28:
				strToPrint = TheText.Get("LANDSTK");
				break;
			case 29:
				strToPrint = TheText.Get("STALION");
				break;
			case 30:
				strToPrint = TheText.Get("BFINJC");
				break;
			case 31:
				strToPrint = TheText.Get("CABBIE");
				break;
			case 32:
				strToPrint = TheText.Get("ESPERAN");
				break;
			case 33:
				strToPrint = TheText.Get("FIRETRK");
				break;
			case 34:
				strToPrint = TheText.Get("AMBULAN");
				break;
			case 35:
				strToPrint = TheText.Get("ENFORCR");
				break;
			case 36:
				strToPrint = TheText.Get("FBICAR");
				break;
			case 37:
				strToPrint = TheText.Get("RHINO");
				break;
			case 38:
				strToPrint = TheText.Get("BARRCKS");
				break;
			case 39:
				strToPrint = TheText.Get("POLICAR");
				break;
			default:
				break;
			}
		}
		CFont::SetPropOn();
		CFont::SetBackgroundOff();

		const float MAX_SCALE = 1.0f;

		float fScaleY = aMessages[i].m_dist.y / 100.0f;
		if (fScaleY > MAX_SCALE) fScaleY = MAX_SCALE;

		float fScaleX = aMessages[i].m_dist.x / 100.0f;
		if (fScaleX > MAX_SCALE) fScaleX = MAX_SCALE;

		CFont::SetScale(fScaleX, fScaleY);
		CFont::SetCentreOn();
		CFont::SetCentreSize(SCREEN_WIDTH);
		CFont::SetJustifyOff();

		CFont::SetColor(CRGBA(aMessages[i].m_color.red, aMessages[i].m_color.green, aMessages[i].m_color.blue, aMessages[i].m_color.alpha));
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetFontStyle(FONT_BANK);
		CFont::PrintString(aMessages[i].m_pos.x, aMessages[i].m_pos.y, strToPrint);
	}
	NumMessages = 0;
}

void
CPickups::Load(uint8 *buf, uint32 size)
{
INITSAVEBUF

	for (int32 i = 0; i < NUMPICKUPS; i++) {
		aPickUps[i] = ReadSaveBuf<CPickup>(buf);

		if (aPickUps[i].m_eType != PICKUP_NONE && aPickUps[i].m_pObject != nil)
			aPickUps[i].m_pObject = CPools::GetObjectPool()->GetSlot((int32)aPickUps[i].m_pObject - 1);
	}

	CollectedPickUpIndex = ReadSaveBuf<uint16>(buf);
	ReadSaveBuf<uint16>(buf);
	NumMessages = 0;

	for (uint16 i = 0; i < NUMCOLLECTEDPICKUPS; i++)
		aPickUpsCollected[i] = ReadSaveBuf<int32>(buf);

VALIDATESAVEBUF(size)
}

void
CPickups::Save(uint8 *buf, uint32 *size)
{
	*size = sizeof(aPickUps) + sizeof(uint16) + sizeof(uint16) + sizeof(aPickUpsCollected);

INITSAVEBUF

	for (int32 i = 0; i < NUMPICKUPS; i++) {
		CPickup *buf_pickup = WriteSaveBuf(buf, aPickUps[i]);
		if (buf_pickup->m_eType != PICKUP_NONE && buf_pickup->m_pObject != nil)
			buf_pickup->m_pObject = (CObject*)(CPools::GetObjectPool()->GetJustIndex(buf_pickup->m_pObject) + 1);
	}

	WriteSaveBuf(buf, CollectedPickUpIndex);
	WriteSaveBuf(buf, (uint16)0); // possibly was NumMessages

	for (uint16 i = 0; i < NUMCOLLECTEDPICKUPS; i++)
		WriteSaveBuf(buf, aPickUpsCollected[i]);

VALIDATESAVEBUF(*size)
}

STARTPATCHES
	InjectHook(0x430220, CPickups::Init, PATCH_JUMP);
	InjectHook(0x4303D0, CPickups::Update, PATCH_JUMP);
	InjectHook(0x432440, CPickups::RenderPickUpText, PATCH_JUMP);
	InjectHook(0x431C30, CPickups::DoCollectableEffects, PATCH_JUMP);
	InjectHook(0x431F40, CPickups::DoMoneyEffects, PATCH_JUMP);
	InjectHook(0x4321C0, CPickups::DoMineEffects, PATCH_JUMP);
	InjectHook(0x431520, CPickups::DoPickUpEffects, PATCH_JUMP);
	InjectHook(0x4304B0, CPickups::GenerateNewOne, PATCH_JUMP);
	InjectHook(0x430660, CPickups::GenerateNewOne_WeaponType, PATCH_JUMP);
	InjectHook(0x4307A0, CPickups::RemovePickUp, PATCH_JUMP);
	InjectHook(0x430800, CPickups::RemoveAllFloatingPickups, PATCH_JUMP);
	InjectHook(0x433D60, CPickups::AddToCollectedPickupsArray, PATCH_JUMP);
	InjectHook(0x430770, CPickups::IsPickUpPickedUp, PATCH_JUMP);
	InjectHook(0x430690, CPickups::ModelForWeapon, PATCH_JUMP);
	InjectHook(0x4306F0, CPickups::WeaponForModel, PATCH_JUMP);
	InjectHook(0x431510, CPickups::FindColourIndexForWeaponMI, PATCH_JUMP);/**/
	InjectHook(0x433DF0, CPickups::GetActualPickupIndex, PATCH_JUMP);
	InjectHook(0x433DB0, CPickups::GetNewUniquePickupIndex, PATCH_JUMP);
	InjectHook(0x433B60, CPickups::PassTime, PATCH_JUMP);
	InjectHook(0x4339F0, CPickups::GivePlayerGoodiesWithPickUpMI, PATCH_JUMP);
	InjectHook(0x433F60, CPickups::Load, PATCH_JUMP);
	InjectHook(0x433E40, CPickups::Save, PATCH_JUMP);
	InjectHook(0x433BA0, &CPickup::GiveUsAPickUpObject, PATCH_JUMP);
	InjectHook(0x430860, &CPickup::Update, PATCH_JUMP);
ENDPATCHES