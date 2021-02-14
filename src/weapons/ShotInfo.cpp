#include "common.h"

#include "ShotInfo.h"
#include "Entity.h"
#include "Weapon.h"
#include "World.h"
#include "WeaponInfo.h"
#include "General.h"
#include "Timer.h"
#include "Ped.h"
#include "Fire.h"

CShotInfo gaShotInfo[NUMSHOTINFOS];
float CShotInfo::ms_afRandTable[20];

#ifdef SQUEEZE_PERFORMANCE
uint32 shotInfoInUse;
#endif

/*
	Used for flamethrower. I don't know why it's name is CShotInfo.
	Has no relation with any visual, just calculates the area fire affects
	(including spreading and slowing of fire) and make entities burn/flee.
*/

void
CShotInfo::Initialise()
{
	debug("Initialising CShotInfo...\n");
	for(int i=0; i<ARRAY_SIZE(gaShotInfo); i++) {
		gaShotInfo[i].m_inUse = false;
		gaShotInfo[i].m_weapon = WEAPONTYPE_COLT45;
		gaShotInfo[i].m_startPos = CVector(0.0f, 0.0f, 0.0f);
		gaShotInfo[i].m_areaAffected = CVector(0.0f, 0.0f, 0.0f);
		gaShotInfo[i].m_radius = 1.0f;
		gaShotInfo[i].m_sourceEntity = nil;
		gaShotInfo[i].m_timeout = 0;
	}

	// Not random for sure
	float nextVal = -0.05f;
	for (int i = 0; i < ARRAY_SIZE(ms_afRandTable); i++) {
		ms_afRandTable[i] = nextVal;
		nextVal += 0.005f;
	}
	debug("CShotInfo ready\n");
#ifdef SQUEEZE_PERFORMANCE
	shotInfoInUse = 0;
#endif
}

bool
CShotInfo::AddShot(CEntity *sourceEntity, eWeaponType weapon, CVector startPos, CVector endPos)
{
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(weapon);

	int slot;
	for (slot = 0; slot < ARRAY_SIZE(gaShotInfo) && gaShotInfo[slot].m_inUse; slot++);

	if (slot == ARRAY_SIZE(gaShotInfo))
		return false;

#ifdef SQUEEZE_PERFORMANCE
	shotInfoInUse++;
#endif

	gaShotInfo[slot].m_inUse = true;
	gaShotInfo[slot].m_weapon = weapon;
	gaShotInfo[slot].m_startPos = startPos;
	gaShotInfo[slot].m_areaAffected = endPos - startPos;
	gaShotInfo[slot].m_radius = weaponInfo->m_fRadius;

	if (weaponInfo->m_fSpread != 0.0f) {
		gaShotInfo[slot].m_areaAffected.x += CShotInfo::ms_afRandTable[CGeneral::GetRandomNumber() % ARRAY_SIZE(ms_afRandTable)] * weaponInfo->m_fSpread;
		gaShotInfo[slot].m_areaAffected.y += CShotInfo::ms_afRandTable[CGeneral::GetRandomNumber() % ARRAY_SIZE(ms_afRandTable)] * weaponInfo->m_fSpread;
		gaShotInfo[slot].m_areaAffected.z += CShotInfo::ms_afRandTable[CGeneral::GetRandomNumber() % ARRAY_SIZE(ms_afRandTable)];
	}
	gaShotInfo[slot].m_areaAffected.Normalise();
	if (weaponInfo->IsFlagSet(WEAPONFLAG_RAND_SPEED))
		gaShotInfo[slot].m_areaAffected *= CShotInfo::ms_afRandTable[CGeneral::GetRandomNumber() % ARRAY_SIZE(ms_afRandTable)] + weaponInfo->m_fSpeed;
	else
		gaShotInfo[slot].m_areaAffected *= weaponInfo->m_fSpeed;

	gaShotInfo[slot].m_sourceEntity = sourceEntity;
	gaShotInfo[slot].m_timeout = CTimer::GetTimeInMilliseconds() + weaponInfo->m_fLifespan;

	return true;
}

void
CShotInfo::Shutdown()
{
	debug("Shutting down CShotInfo...\n");
	debug("CShotInfo shut down\n");
}

void
CShotInfo::Update()
{
#ifdef SQUEEZE_PERFORMANCE
	if (shotInfoInUse == 0)
		return;
#endif
	for (int slot = 0; slot < ARRAY_SIZE(gaShotInfo); slot++) {
		CShotInfo &shot = gaShotInfo[slot];
		if (shot.m_sourceEntity && shot.m_sourceEntity->IsPed() && !((CPed*)shot.m_sourceEntity)->IsPointerValid())
			shot.m_sourceEntity = nil;

		if (!shot.m_inUse)
			continue;

		CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(shot.m_weapon);
		if (CTimer::GetTimeInMilliseconds() > shot.m_timeout) {
#ifdef SQUEEZE_PERFORMANCE
			shotInfoInUse--;
#endif
			shot.m_inUse = false;
		}

		if (weaponInfo->IsFlagSet(WEAPONFLAG_SLOWS_DOWN))
			shot.m_areaAffected *= pow(0.96, CTimer::GetTimeStep()); // FRAMERATE

		if (weaponInfo->IsFlagSet(WEAPONFLAG_EXPANDS))
			shot.m_radius += 0.075f * CTimer::GetTimeStep();

		shot.m_startPos += CTimer::GetTimeStep() * shot.m_areaAffected;
		if (shot.m_sourceEntity) {
			assert(shot.m_sourceEntity->IsPed());
			CPed *ped = (CPed*) shot.m_sourceEntity;
			float radius = Max(1.0f, shot.m_radius);

			for (int i = 0; i < ped->m_numNearPeds; ++i) {
				CPed *nearPed = ped->m_nearPeds[i];
				if (nearPed->IsPointerValid()) {
					if (nearPed->IsPedInControl() && (nearPed->GetPosition() - shot.m_startPos).MagnitudeSqr() < radius && !nearPed->bFireProof) {

						if (!nearPed->IsPlayer()) {
							nearPed->SetFindPathAndFlee(shot.m_sourceEntity, 10000);
							nearPed->SetMoveState(PEDMOVE_SPRINT);
						}
						gFireManager.StartFire(nearPed, shot.m_sourceEntity, 0.8f, true);
					}
				}
			}
		}
		if (!((CTimer::GetFrameCounter() + slot) & 3))
			CWorld::SetCarsOnFire(shot.m_startPos.x, shot.m_startPos.y, shot.m_startPos.z, 4.0f, shot.m_sourceEntity);
	}
}