#pragma once
#include "Weapon.h"

enum ePickupType
{
	PICKUP_NONE = 0,
	PICKUP_IN_SHOP,
	PICKUP_ON_STREET,
	PICKUP_ONCE,
	PICKUP_ONCE_TIMEOUT,
	PICKUP_COLLECTABLE1,
	PICKUP_IN_SHOP_OUT_OF_STOCK,
	PICKUP_MONEY,
	PICKUP_MINE_INACTIVE,
	PICKUP_MINE_ARMED,
	PICKUP_NAUTICAL_MINE_INACTIVE,
	PICKUP_NAUTICAL_MINE_ARMED,
	PICKUP_FLOATINGPACKAGE,
	PICKUP_FLOATINGPACKAGE_FLOATING,
	PICKUP_ON_STREET_SLOW,
	PICKUP_NUMOFTYPES
};

class CEntity;
class CObject;
class CVehicle;
class CPlayerPed;

class CPickup
{
public:
	uint8 m_eType;
	bool m_bRemoved;
	uint16 m_nQuantity;
	CObject *m_pObject;
	uint32 m_nTimer;
	int16 m_eModelIndex;
	uint16 m_nIndex;
	CVector m_vecPos;

	CObject *GiveUsAPickUpObject(int32 handle);
	bool Update(CPlayerPed *player, CVehicle *vehicle, int playerId);
private:
	inline bool IsMine() { return m_eType >= PICKUP_MINE_INACTIVE && m_eType <= PICKUP_FLOATINGPACKAGE_FLOATING; }
	inline bool CanBePickedUp(CPlayerPed *player);
	inline void Remove();
};

VALIDATE_SIZE(CPickup, 0x1C);

struct tPickupMessage
{
	CVector2D m_pos;
	eWeaponType m_weaponType;
	CVector2D m_dist;
	CRGBA m_color;
	uint8 m_bOutOfStock : 1;
	uint8 m_quantity;
};

class CPickups
{
	static int32 aPickUpsCollected[NUMCOLLECTEDPICKUPS];
	static int16 CollectedPickUpIndex;
	static int16 NumMessages;
	static tPickupMessage aMessages[NUMPICKUPMESSAGES];
public:
	static void Init();
	static void Update();
	static void RenderPickUpText();
	static void DoCollectableEffects(CEntity *ent);
	static void DoMoneyEffects(CEntity *ent);
	static void DoMineEffects(CEntity *ent);
	static void DoPickUpEffects(CEntity *ent);
	static int32 GenerateNewOne(CVector pos, uint32 modelIndex, uint8 type, uint32 quantity);
	static int32 GenerateNewOne_WeaponType(CVector pos, eWeaponType weaponType, uint8 type, uint32 quantity);
	static void RemovePickUp(int32 pickupIndex);
	static void RemoveAllFloatingPickups();
	static void AddToCollectedPickupsArray(int32 index);
	static bool IsPickUpPickedUp(int32 pickupId);
	static int32 ModelForWeapon(eWeaponType weaponType);
	static enum eWeaponType WeaponForModel(int32 model);
	static int32 FindColourIndexForWeaponMI(int32 model);
	static int32 GetActualPickupIndex(int32 index);
	static int32 GetNewUniquePickupIndex(int32 slot);
	static void PassTime(uint32 time);
	static bool GivePlayerGoodiesWithPickUpMI(int16 modelIndex, int playerIndex);
	static void Load(uint8 *buf, uint32 size);
	static void Save(uint8 *buf, uint32 *size);

	static CPickup aPickUps[NUMPICKUPS];

	// unused
	static bool bPickUpcamActivated;
	static CVehicle *pPlayerVehicle;
	static CVector StaticCamCoors;
	static uint32 StaticCamStartTime;
};

extern uint16 AmmoForWeapon[20];
extern uint16 AmmoForWeapon_OnStreet[20];
extern uint16 CostOfWeapon[20];

enum ePacmanPickupType
{
	PACMAN_NONE,
	PACMAN_SCRAMBLE,
	PACMAN_RACE,
};

class CPacManPickup
{
public:
	CVector m_vecPosn;
	CObject *m_pObject;
	uint8 m_eType;

	void Update();
};

class CPacManPickups
{
	friend class CPacManPickup;

	static CPacManPickup aPMPickUps[NUMPACMANPICKUPS];
	static CVector LastPickUpCoors;
	static int PillsEatenInRace;
	static bool bPMActive;
public:
	static void Init(void);
	static void Update(void);
	static void GeneratePMPickUps(CVector, float, int16, uint8);
	static void GeneratePMPickUpsForRace(int32);
	static void GenerateOnePMPickUp(CVector);
	static void Render(void);
	static void StartPacManRace(int32);
	static void StartPacManRecord(void);
	static uint32 QueryPowerPillsEatenInRace(void);
	static void ResetPowerPillsEatenInRace(void);
	static void ClearPMPickUps(void);
	static void CleanUpPacManStuff(void);
	static void StartPacManScramble(CVector, float, int16);
	static uint32 QueryPowerPillsCarriedByPlayer(void);
	static void ResetPowerPillsCarriedByPlayer(void);

};
