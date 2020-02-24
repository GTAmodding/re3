#pragma once
#include "common.h"

class CVehicle;
class CEntity;
class CObject;

class CCrane
{
public:
	CEntity *m_pObject;
	CObject *m_pMagnet;
	int m_nAudioEntity;
	float m_fPickupX1;
	float m_fPickupX2;
	float m_fPickupY1;
	float m_fPickupY2;
	CVector m_vecDropoffTarget;
	float m_fDropoffHeading;
	float m_fPickupAngle;
	float m_fDropoffAngle;
	float m_fPickupDistance;
	float m_fDropoffDistance;
	float m_fAngle;
	float m_fDistance;
	float m_fHeight;
	float m_fHookOffset;
	float m_fHookHeight;
	CVector m_vecHookInitPos;
	CVector m_vecHookCurPos;
	float m_fHookVelocityX;
	float m_fHookVelocityY;
	CVehicle *m_pVehiclePickedUp;
	int m_nUpdateTimer;
	char m_bCraneActive;
	char m_bCraneStatus;
	char m_bVehiclesCollected;
	char m_bIsCrusher;
	char m_bIsMilitaryCrane;
	char field_125;
	char m_bNotMilitaryCrane;
	char gap_127[1];
};

static_assert(sizeof(CCrane) == 128, "CCrane: error");

class CCranes
{
public:
	static bool IsThisCarBeingTargettedByAnyCrane(CVehicle*);
	static bool IsThisCarBeingCarriedByAnyCrane(CVehicle*);
	static bool IsThisCarPickedUp(float, float, CVehicle*);
	static bool HaveAllCarsBeenCollectedByMilitaryCrane();
	static void ActivateCrane(float, float, float, float, float, float, float, float, bool, bool, float, float);
	static void DeActivateCrane(float, float);
	static void InitCranes(void);
	static void UpdateCranes(void);
	static void Save(uint8*, uint32*);
};

void CranesLoad(uint8*, uint32);	// is this really outside CCranes?
