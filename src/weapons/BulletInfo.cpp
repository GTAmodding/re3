#include "common.h"

#include "BulletInfo.h"

#include "AnimBlendAssociation.h"
#include "DMAudio.h"
#include "AudioScriptObject.h"
#ifdef FIX_BUGS
#include "Collision.h"
#endif
#include "RpAnimBlend.h"
#include "Entity.h"
#include "EventList.h"
#include "Fire.h"
#include "Glass.h"
#include "Particle.h"
#include "Ped.h"
#include "Object.h"
#include "Stats.h"
#include "Timer.h"
#include "Vehicle.h"
#include "Weapon.h"
#include "WeaponInfo.h"
#include "World.h"
#include "SurfaceTable.h"

#ifdef SQUEEZE_PERFORMANCE
uint32 bulletInfoInUse;
#endif

#define BULLET_LIFETIME (1000)
#define NUM_PED_BLOOD_PARTICLES (8)
#define BLOOD_PARTICLE_OFFSET (CVector(0.0f, 0.0f, 0.0f))
#define NUM_VEHICLE_SPARKS (16)
#define NUM_OTHER_SPARKS (8)
#define BULLET_HIT_FORCE (7.5f)
#define MAP_BORDER (1960.0f)

CBulletInfo gaBulletInfo[CBulletInfo::NUM_BULLETS];
bool bPlayerSniperBullet;
CVector PlayerSniperBulletStart;
CVector PlayerSniperBulletEnd;

void CBulletInfo::Initialise(void)
{
	debug("Initialising CBulletInfo...\n");
	for (int i = 0; i < NUM_BULLETS; i++) {
		gaBulletInfo[i].m_bInUse = false;
		gaBulletInfo[i].m_eWeaponType = WEAPONTYPE_COLT45;
		gaBulletInfo[i].m_fTimer = 0.0f;
		gaBulletInfo[i].m_pSource = nil;
	}
	debug("CBulletInfo ready\n");
#ifdef SQUEEZE_PERFORMANCE
	bulletInfoInUse = 0;
#endif
}

void CBulletInfo::Shutdown(void)
{
	debug("Shutting down CBulletInfo...\n");
	debug("CBulletInfo shut down\n");
}

bool CBulletInfo::AddBullet(CEntity* pSource, eWeaponType type, CVector vecPosition, CVector vecSpeed)
{
	int i;
	for (i = 0; i < NUM_BULLETS; i++) {
		if (!gaBulletInfo[i].m_bInUse)
			break;
	}
	if (i == NUM_BULLETS)
		return false;
	gaBulletInfo[i].m_pSource = pSource;
	gaBulletInfo[i].m_eWeaponType = type;
	gaBulletInfo[i].m_nDamage = CWeaponInfo::GetWeaponInfo(type)->m_nDamage;
	gaBulletInfo[i].m_vecPosition = vecPosition;
	gaBulletInfo[i].m_vecSpeed = vecSpeed;
	gaBulletInfo[i].m_fTimer = CTimer::GetTimeInMilliseconds() + BULLET_LIFETIME;
	gaBulletInfo[i].m_bInUse = true;

#ifdef SQUEEZE_PERFORMANCE
	bulletInfoInUse++;
#endif
	return true;
}

void CBulletInfo::Update(void)
{
#ifdef SQUEEZE_PERFORMANCE
	if (bulletInfoInUse == 0)
		return;
#endif
	bool bAddSound = true;
	bPlayerSniperBullet = false;
	for (int i = 0; i < NUM_BULLETS; i++) {
		CBulletInfo* pBullet = &gaBulletInfo[i];
		if (pBullet->m_pSource && pBullet->m_pSource->IsPed() && !((CPed*)pBullet->m_pSource)->IsPointerValid())
			pBullet->m_pSource = nil;
		if (!pBullet->m_bInUse)
			continue;
		if (CTimer::GetTimeInMilliseconds() > pBullet->m_fTimer) {
			pBullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
			bulletInfoInUse--;
#endif
		}
		CVector vecOldPos = pBullet->m_vecPosition;
		CVector vecNewPos = pBullet->m_vecPosition + pBullet->m_vecSpeed * CTimer::GetTimeStep() * 0.5f;
		CWorld::bIncludeCarTyres = true;
		CWorld::bIncludeDeadPeds = true;
		CWorld::pIgnoreEntity = pBullet->m_pSource;
		CColPoint point;
		CEntity* pHitEntity;
		if (CWorld::ProcessLineOfSight(vecOldPos, vecNewPos, point, pHitEntity, true, true, true, true, true, true)) {
			if (pBullet->m_pSource && (pHitEntity->IsPed() || pHitEntity->IsVehicle()))
				CStats::InstantHitsHitByPlayer++;
			if (pHitEntity->IsPed()) {
				CPed* pPed = (CPed*)pHitEntity;
				if (!pPed->DyingOrDead() && pPed != pBullet->m_pSource) {
					if (pPed->DoesLOSBulletHitPed(point)) {
						if (pPed->IsPedInControl() && !pPed->bIsDucking) {
							pPed->ClearAttackByRemovingAnim();
							CAnimBlendAssociation* pAnim = CAnimManager::AddAnimation(pPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HITBYGUN_FRONT);
							pAnim->SetBlend(0.0f, 8.0f);
						}
						pPed->InflictDamage(pBullet->m_pSource, pBullet->m_eWeaponType, pBullet->m_nDamage, (ePedPieceTypes)point.pieceB, pPed->GetLocalDirection(pPed->GetPosition() - point.point));
						CEventList::RegisterEvent(pPed->m_nPedType == PEDTYPE_COP ? EVENT_SHOOT_COP : EVENT_SHOOT_PED, EVENT_ENTITY_PED, pPed, (CPed*)pBullet->m_pSource, 1000);
						pBullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
						bulletInfoInUse--;
#endif
						vecNewPos = point.point;
					}
					else {
						bAddSound = false;
					}
				}
				if (CGame::nastyGame) {
					CVector vecParticleDirection = (point.point - pPed->GetPosition()) * 0.01f;
					vecParticleDirection.z = 0.01f;
					if (pPed->GetIsOnScreen()) {
						for (int j = 0; j < NUM_PED_BLOOD_PARTICLES; j++)
							CParticle::AddParticle(PARTICLE_BLOOD_SMALL, point.point + BLOOD_PARTICLE_OFFSET, vecParticleDirection);
					}
					if (pPed->GetPedState() == PED_DEAD) {
						CAnimBlendAssociation* pAnim;
						if (RpAnimBlendClumpGetFirstAssociation(pPed->GetClump(), ASSOC_FRONTAL))
							pAnim = CAnimManager::BlendAnimation(pPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR_FRONT, 8.0f);
						else
							pAnim = CAnimManager::BlendAnimation(pPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR, 8.0f);
						if (pAnim) {
							pAnim->SetCurrentTime(0.0f);
							pAnim->flags |= ASSOC_RUNNING;
							pAnim->flags &= ~ASSOC_FADEOUTWHENDONE;
						}
					}
					pBullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
					bulletInfoInUse--;
#endif
					vecNewPos = point.point;
				}
			}
			else if (pHitEntity->IsVehicle()) {
				CVehicle* pVehicle = (CVehicle*)pHitEntity;
				pVehicle->InflictDamage(pBullet->m_pSource, pBullet->m_eWeaponType, pBullet->m_nDamage);
				if (pBullet->m_eWeaponType == WEAPONTYPE_FLAMETHROWER) // huh?
					gFireManager.StartFire(pVehicle, pBullet->m_pSource, 0.8f, true);
				else {
					for (int j = 0; j < NUM_VEHICLE_SPARKS; j++)
						CParticle::AddParticle(PARTICLE_SPARK, point.point, point.normal / 20);
				}
#ifdef FIX_BUGS
				pBullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
				bulletInfoInUse--;
#endif
				vecNewPos = point.point;
#endif
			}
			else {
				for (int j = 0; j < NUM_OTHER_SPARKS; j++)
					CParticle::AddParticle(PARTICLE_SPARK, point.point, point.normal / 20);
				if (pHitEntity->IsObject()) {
					CObject* pObject = (CObject*)pHitEntity;
					if (!pObject->bInfiniteMass) {
						if (pObject->GetIsStatic() && pObject->m_fUprootLimit <= 0.0f) {
							pObject->SetIsStatic(false);
							pObject->AddToMovingList();
						}
						if (!pObject->GetIsStatic())
							pObject->ApplyMoveForce(-BULLET_HIT_FORCE * point.normal);
					}
				}
#ifdef FIX_BUGS
				pBullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
				bulletInfoInUse--;
#endif
				vecNewPos = point.point;
#endif
			}
			if (pBullet->m_eWeaponType == WEAPONTYPE_SNIPERRIFLE && bAddSound) {
				cAudioScriptObject* pAudio;
				switch (pHitEntity->GetType()) {
				case ENTITY_TYPE_BUILDING:
					pAudio = new cAudioScriptObject();
					pAudio->Posn = pHitEntity->GetPosition();
					pAudio->AudioId = SCRIPT_SOUND_BULLET_HIT_GROUND_1;
					pAudio->AudioEntity = AEHANDLE_NONE;
					DMAudio.CreateOneShotScriptObject(pAudio);
					break;
				case ENTITY_TYPE_OBJECT:
					pAudio = new cAudioScriptObject();
					pAudio->Posn = pHitEntity->GetPosition();
					pAudio->AudioId = SCRIPT_SOUND_BULLET_HIT_GROUND_2;
					pAudio->AudioEntity = AEHANDLE_NONE;
					DMAudio.CreateOneShotScriptObject(pAudio);
					break;
				case ENTITY_TYPE_DUMMY:
					pAudio = new cAudioScriptObject();
					pAudio->Posn = pHitEntity->GetPosition();
					pAudio->AudioId = SCRIPT_SOUND_BULLET_HIT_GROUND_3;
					pAudio->AudioEntity = AEHANDLE_NONE;
					DMAudio.CreateOneShotScriptObject(pAudio);
					break;
				case ENTITY_TYPE_PED:
					DMAudio.PlayOneShot(((CPed*)pHitEntity)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
					((CPed*)pHitEntity)->Say(SOUND_PED_BULLET_HIT);
					break;
				case ENTITY_TYPE_VEHICLE:
					DMAudio.PlayOneShot(((CVehicle*)pHitEntity)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
					break;
				default: break;
				}
			}
			CGlass::WasGlassHitByBullet(pHitEntity, point.point);
			CWeapon::BlowUpExplosiveThings(pHitEntity);
		}
		CWorld::pIgnoreEntity = nil;
		CWorld::bIncludeDeadPeds = false;
		CWorld::bIncludeCarTyres = false;
		if (pBullet->m_eWeaponType == WEAPONTYPE_SNIPERRIFLE) {
			bPlayerSniperBullet = true;
			PlayerSniperBulletStart = pBullet->m_vecPosition;
			PlayerSniperBulletEnd = vecNewPos;
		}
		pBullet->m_vecPosition = vecNewPos;
		if (pBullet->m_vecPosition.x < -MAP_BORDER || pBullet->m_vecPosition.x > MAP_BORDER ||
			pBullet->m_vecPosition.y < -MAP_BORDER || pBullet->m_vecPosition.y > MAP_BORDER) {
			pBullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
			bulletInfoInUse--;
#endif
		}
	}
}

bool CBulletInfo::TestForSniperBullet(float x1, float x2, float y1, float y2, float z1, float z2)
{
	if (!bPlayerSniperBullet)
		return false;
#ifdef FIX_BUGS // original code is not going work anyway...
	CColLine line(PlayerSniperBulletStart, PlayerSniperBulletEnd);
	CColBox box;
	box.Set(CVector(x1, y1, z1), CVector(x2, y2, z2), SURFACE_DEFAULT, 0);
	return CCollision::TestLineBox(line, box);
#else
	float minP = 0.0f;
	float maxP = 1.0f;
	float minX = Min(PlayerSniperBulletStart.x, PlayerSniperBulletEnd.x);
	float maxX = Max(PlayerSniperBulletStart.x, PlayerSniperBulletEnd.x);
	if (minX < x2 || maxX > x1) {
		if (minX < x1)
			minP = Min(minP, (x1 - minX) / (maxX - minX));
		if (maxX > x2)
			maxP = Max(maxP, (maxX - x2) / (maxX - minX));
	}
	else
		return false;
	float minY = Min(PlayerSniperBulletStart.y, PlayerSniperBulletEnd.y);
	float maxY = Max(PlayerSniperBulletStart.y, PlayerSniperBulletEnd.y);
	if (minY < y2 || maxY > y1) {
		if (minY < y1)
			minP = Min(minP, (y1 - minY) / (maxY - minY));
		if (maxY > y2)
			maxP = Max(maxP, (maxY - y2) / (maxY - minY));
	}
#ifdef FIX_BUGS
	else
		return false;
#endif
	float minZ = Min(PlayerSniperBulletStart.z, PlayerSniperBulletEnd.z);
	float maxZ = Max(PlayerSniperBulletStart.z, PlayerSniperBulletEnd.z);
	if (minZ < z2 || maxZ > z1) {
		if (minZ < z1)
			minP = Min(minP, (z1 - minZ) / (maxZ - minZ));
		if (maxZ > z2)
			maxP = Max(maxP, (maxZ - z2) / (maxZ - minZ));
	}
	else
		return false;
	return minP <= maxP;
#endif
}
