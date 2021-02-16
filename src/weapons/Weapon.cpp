#include "common.h"

#include "Weapon.h"
#include "AnimBlendAssociation.h"
#include "AudioManager.h"
#include "BulletInfo.h"
#include "Camera.h"
#include "Coronas.h"
#include "DMAudio.h"
#include "Explosion.h"
#include "General.h"
#include "Glass.h"
#include "Heli.h"
#include "ModelIndices.h"
#include "Object.h"
#include "Pad.h"
#include "Particle.h"
#include "Ped.h"
#include "PointLights.h"
#include "Pools.h"
#include "ProjectileInfo.h"
#include "RpAnimBlend.h"
#include "ShotInfo.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "TempColModels.h"
#include "Timer.h"
#include "Automobile.h"
#include "Boat.h"
#include "WaterLevel.h"
#include "WeaponInfo.h"
#include "World.h"

uint16 gReloadSampleTime[WEAPONTYPE_LAST_WEAPONTYPE] =
{
	0,			// UNARMED
	0,			// BASEBALLBAT
	250,		// COLT45
	400,		// UZI
	650,		// SHOTGUN
	300,		// AK47
	300,		// M16
	423,		// SNIPERRIFLE
	400,		// ROCKETLAUNCHER
	0,			// FLAMETHROWER
	0,			// MOLOTOV
	0,			// GRENADE
	0,			// DETONATOR
	0			// HELICANNON
};

CWeaponInfo *
CWeapon::GetInfo()
{
	CWeaponInfo *info = CWeaponInfo::GetWeaponInfo(m_eWeaponType);
	ASSERT(info!=nil);
	return info;
}

void
CWeapon::InitialiseWeapons(void)
{
	CWeaponInfo::Initialise();
	CShotInfo::Initialise();
	CExplosion::Initialise();
	CProjectileInfo::Initialise();
	CBulletInfo::Initialise();
}

void
CWeapon::ShutdownWeapons(void)
{
	CWeaponInfo::Shutdown();
	CShotInfo::Shutdown();
	CExplosion::Shutdown();
	CProjectileInfo::Shutdown();
	CBulletInfo::Shutdown();
}

void
CWeapon::UpdateWeapons(void)
{
	CShotInfo::Update();
	CExplosion::Update();
	CProjectileInfo::Update();
	CBulletInfo::Update();
}

void
CWeapon::Initialise(eWeaponType type, int32 ammo)
{
	m_eWeaponType = type;
	m_eWeaponState = WEAPONSTATE_READY;
	if (ammo > 99999)
		m_nAmmoTotal = 99999;
	else
		m_nAmmoTotal = ammo;
	m_nAmmoInClip = 0;
	Reload();
	m_nTimer = 0;
}

bool
CWeapon::Fire(CEntity *shooter, CVector *fireSource)
{
	ASSERT(shooter!=nil);

	CVector fireOffset(0.0f, 0.0f, 0.6f);
	CVector *source = fireSource;

	if (!fireSource)
	{
		fireOffset = shooter->GetMatrix() * fireOffset;
#ifdef FIX_BUGS
		static CVector tmp;
		tmp = fireOffset;
		source = &tmp;
#else
		source = &fireOffset;
#endif
		
	}
	if ( m_bAddRotOffset )
	{
		float heading = RADTODEG(shooter->GetForward().Heading());
		float angle   = DEGTORAD(heading);
		(*source).x += -Sin(angle) * 0.15f;
		(*source).y +=  Cos(angle) * 0.15f;
	}

	if ( m_eWeaponState != WEAPONSTATE_READY && m_eWeaponState != WEAPONSTATE_FIRING )
		return false;

	bool fired;

	if ( GetInfo()->m_eWeaponFire != WEAPON_FIRE_MELEE )
	{
		if ( m_nAmmoInClip <= 0 )
			return false;

		switch ( m_eWeaponType )
		{
			case WEAPONTYPE_SHOTGUN:
			{
				fired = FireShotgun(shooter, source);

				break;
			}

			case WEAPONTYPE_COLT45:
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_AK47:
			{
				fired = FireInstantHit(shooter, source);

				break;
			}

			case WEAPONTYPE_SNIPERRIFLE:
			{
				fired = FireSniper(shooter);

				break;
			}

			case WEAPONTYPE_M16:
			{
				if ( TheCamera.PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON && shooter == FindPlayerPed() )
					fired = FireM16_1stPerson(shooter);
				else
					fired = FireInstantHit(shooter, source);

				break;
			}

			case WEAPONTYPE_ROCKETLAUNCHER:
			{
				if ( shooter->IsPed() && ((CPed*)shooter)->m_pSeekTarget != nil )
				{
					float distToTarget = (shooter->GetPosition() - ((CPed*)shooter)->m_pSeekTarget->GetPosition()).Magnitude();

					if ( distToTarget > 8.0f || ((CPed*)shooter)->IsPlayer() )
						fired = FireProjectile(shooter, source, 0.0f);
					else
						fired = false;
				}
				else
					fired = FireProjectile(shooter, source, 0.0f);

				break;
			}

			case WEAPONTYPE_MOLOTOV:
			case WEAPONTYPE_GRENADE:
			{
				if ( shooter == FindPlayerPed() )
				{
					fired = FireProjectile(shooter, source, ((CPlayerPed*)shooter)->m_fAttackButtonCounter*0.0375f);
					if ( m_eWeaponType == WEAPONTYPE_GRENADE )
						CStats::KgsOfExplosivesUsed++;
				}
				else if ( shooter->IsPed() && ((CPed*)shooter)->m_pSeekTarget != nil )
				{
					float distToTarget = (shooter->GetPosition() - ((CPed*)shooter)->m_pSeekTarget->GetPosition()).Magnitude();
					float power = clamp((distToTarget-10.0f)*0.02f, 0.2f, 1.0f);

					fired = FireProjectile(shooter, source, power);
				}
				else
					fired = FireProjectile(shooter, source, 0.3f);

				break;
			}

			case WEAPONTYPE_FLAMETHROWER:
			{
				fired = FireAreaEffect(shooter, source);

				break;
			}

			case WEAPONTYPE_DETONATOR:
			{
				CWorld::UseDetonator(shooter);
				m_nAmmoTotal  = 1;
				m_nAmmoInClip = m_nAmmoTotal;
				fired = true;

				break;
			}

			case WEAPONTYPE_HELICANNON:
			{
				if ( (TheCamera.PlayerWeaponMode.Mode == CCam::MODE_HELICANNON_1STPERSON || TheCamera.PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON )
						&& shooter == FindPlayerPed() )
				{
					fired = FireM16_1stPerson(shooter);
				}
				else
					fired = FireInstantHit(shooter, source);

				break;
			}

			default:
			{
				debug("Unknown weapon type, Weapon.cpp");
				break;
			}
		}

		if (fired)
		{
			bool isPlayer = false;

			if (shooter->IsPed())
			{
				CPed* shooterPed = (CPed*)shooter;

				shooterPed->bIsShooting = true;

				if (shooterPed->IsPlayer())
					isPlayer = true;

				DMAudio.PlayOneShot(shooterPed->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
			}

			if (m_nAmmoInClip > 0) m_nAmmoInClip--;
			if (m_nAmmoTotal > 0 && (m_nAmmoTotal < 25000 || isPlayer)) m_nAmmoTotal--;

			if (m_eWeaponState == WEAPONSTATE_READY && m_eWeaponType == WEAPONTYPE_FLAMETHROWER)
				DMAudio.PlayOneShot(((CPhysical*)shooter)->m_audioEntityId, SOUND_WEAPON_FLAMETHROWER_FIRE, 0.0f);

			m_eWeaponState = WEAPONSTATE_FIRING;

			if (m_nAmmoInClip == 0)
			{
				if (m_nAmmoTotal == 0)
					return true;

				m_eWeaponState = WEAPONSTATE_RELOADING;
				m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nReload;

				if (shooter == FindPlayerPed())
				{
					if (CWorld::Players[CWorld::PlayerInFocus].m_bFastReload)
						m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nReload / 4;
				}

				return true;
			}

			m_nTimer = CTimer::GetTimeInMilliseconds() + 1000;
			if (shooter == FindPlayerPed())
				CStats::RoundsFiredByPlayer++;
		}
	}
	else
	{
		if ( m_eWeaponState != WEAPONSTATE_FIRING )
		{
			m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nReload;
			m_eWeaponState = WEAPONSTATE_FIRING;
		}

		FireMelee(shooter, *source);
	}

	if ( m_eWeaponType == WEAPONTYPE_UNARMED || m_eWeaponType == WEAPONTYPE_BASEBALLBAT )
		return true;
	else
		return fired;
}

bool
CWeapon::FireFromCar(CAutomobile *shooter, bool left)
{
	ASSERT(shooter!=nil);

	if ( m_eWeaponState != WEAPONSTATE_READY && m_eWeaponState != WEAPONSTATE_FIRING )
		return false;

	if ( m_nAmmoInClip <= 0 )
		return false;

	if ( FireInstantHitFromCar(shooter, left) )
	{
		DMAudio.PlayOneShot(shooter->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);

		if ( m_nAmmoInClip > 0 ) m_nAmmoInClip--;
		if ( m_nAmmoTotal < 25000 && m_nAmmoTotal > 0 ) m_nAmmoTotal--;

		m_eWeaponState = WEAPONSTATE_FIRING;

		if ( m_nAmmoInClip == 0 )
		{
			if ( m_nAmmoTotal == 0 )
				return true;

			m_eWeaponState = WEAPONSTATE_RELOADING;
			m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nReload;

			return true;
		}

		m_nTimer = CTimer::GetTimeInMilliseconds() + 1000;
		if ( shooter == FindPlayerVehicle() )
			CStats::RoundsFiredByPlayer++;
	}

	return true;
}

bool
CWeapon::FireMelee(CEntity *shooter, CVector &fireSource)
{
	ASSERT(shooter!=nil);

	CWeaponInfo *info = GetInfo();

	bool anim2Playing = false;
	if ( RpAnimBlendClumpGetAssociation(shooter->GetClump(), info->m_Anim2ToPlay) )
		anim2Playing = true;

	ASSERT(shooter->IsPed());

	CPed *shooterPed = (CPed*)shooter;

	for ( int32 i = 0; i < shooterPed->m_numNearPeds; i++ )
	{
		CPed *victimPed = shooterPed->m_nearPeds[i];
		ASSERT(victimPed!=nil);

		if ( (victimPed->m_nPedType != shooterPed->m_nPedType || victimPed == shooterPed->m_pSeekTarget)
				&& victimPed != shooterPed->m_leader || !(CGeneral::GetRandomNumber() & 31) )
		{
			bool collided = false;

			CColModel *victimPedCol = &CTempColModels::ms_colModelPed1;
			if ( victimPed->OnGround() || !victimPed->IsPedHeadAbovePos(-0.3f) )
				victimPedCol = &CTempColModels::ms_colModelPedGroundHit;


			float victimPedRadius = victimPed->GetBoundRadius() + info->m_fRadius;
			if ( victimPed->bUsesCollision || victimPed->Dead() || victimPed->Driving() )
			{
				CVector victimPedPos = victimPed->GetPosition();
				if ( SQR(victimPedRadius) > (victimPedPos-fireSource).MagnitudeSqr() )
				{
					CVector collisionDist;

					int32 s = 0;
					while ( s < victimPedCol->numSpheres )
					{
						CColSphere *sphere = &victimPedCol->spheres[s];
						collisionDist = victimPedPos+sphere->center-fireSource;

						if ( SQR(sphere->radius + info->m_fRadius) > collisionDist.MagnitudeSqr() )
						{
							collided = true;
							break;
						}
						s++;
					}

					if ( !(victimPed->IsPlayer() && victimPed->GetPedState() == PED_GETUP) )
					{
						if ( collided )
						{
							float victimPedHealth = victimPed->m_fHealth;
							CVector bloodPos = fireSource + (collisionDist*0.7f);

							CVector2D posOffset(shooterPed->GetPosition().x-victimPedPos.x, shooterPed->GetPosition().y-victimPedPos.y);

							int32 localDir = victimPed->GetLocalDirection(posOffset);

							bool isBat = m_eWeaponType == WEAPONTYPE_BASEBALLBAT;

							if ( !victimPed->DyingOrDead() )
								victimPed->ReactToAttack(shooterPed);

							uint8 hitLevel = HITLEVEL_HIGH;
							if ( isBat && victimPed->OnGround() )
								hitLevel = HITLEVEL_GROUND;

							victimPed->StartFightDefend(localDir, hitLevel, 10);

							if ( !victimPed->DyingOrDead() )
							{
								if ( shooterPed->IsPlayer() && isBat && anim2Playing )
									victimPed->InflictDamage(shooterPed, m_eWeaponType, 100.0f, PEDPIECE_TORSO, localDir);
								else if ( shooterPed->IsPlayer() && ((CPlayerPed*)shooterPed)->m_bAdrenalineActive )
									victimPed->InflictDamage(shooterPed, m_eWeaponType, 3.5f*info->m_nDamage, PEDPIECE_TORSO, localDir);
								else
								{
									if ( victimPed->IsPlayer() && isBat ) // wtf, it's not fair
										victimPed->InflictDamage(shooterPed, m_eWeaponType, 2.0f*info->m_nDamage, PEDPIECE_TORSO, localDir);
									else
										victimPed->InflictDamage(shooterPed, m_eWeaponType,      info->m_nDamage, PEDPIECE_TORSO, localDir);
								}
							}

							if ( CGame::nastyGame )
							{
								if ( victimPed->GetIsOnScreen() )
								{
									CVector dir = collisionDist * RecipSqrt(1.0f, 10.0f*collisionDist.MagnitudeSqr());

									CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
									CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
									CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);

									if ( isBat )
									{
										dir.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
										dir.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
										CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);

										dir.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
										dir.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
										CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
									}
								}
							}

							if ( !victimPed->OnGround() )
							{
								if ( victimPed->m_fHealth > 0.0f
									&& (victimPed->m_fHealth < 20.0f && victimPedHealth > 20.0f || isBat && !victimPed->IsPlayer()) )
								{
									posOffset.Normalise();
									victimPed->bIsStanding = false;
									victimPed->ApplyMoveForce(posOffset.x*-5.0f, posOffset.y*-5.0f, 3.0f);

									if ( isBat && victimPed->IsPlayer() )
										victimPed->SetFall(3000, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), false);
									else
										victimPed->SetFall(1500, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), false);

									shooterPed->m_pSeekTarget = victimPed;
									shooterPed->m_pSeekTarget->RegisterReference(&shooterPed->m_pSeekTarget);
								}
							}
							else if (victimPed->Dying() && !anim2Playing)
							{
								posOffset.Normalise();
								victimPed->bIsStanding = false;
								victimPed->ApplyMoveForce(posOffset.x*-5.0f, posOffset.y*-5.0f, 3.0f);
							}

							m_eWeaponState = WEAPONSTATE_MELEE_MADECONTACT;

							if ( victimPed->m_nPedType == PEDTYPE_COP )
								CEventList::RegisterEvent(EVENT_ASSAULT_POLICE, EVENT_ENTITY_PED, victimPed, shooterPed, 2000);
							else
								CEventList::RegisterEvent(EVENT_ASSAULT,        EVENT_ENTITY_PED, victimPed, shooterPed, 2000);
						}
					}
				}
			}
		}
	}

	return true;
}

bool
CWeapon::FireInstantHit(CEntity *shooter, CVector *fireSource)
{
	ASSERT(shooter!=nil);
	ASSERT(fireSource!=nil);

	CWeaponInfo *info = GetInfo();

	CVector source, target;
	CColPoint point;
	CEntity *victim = nil;

	float heading = RADTODEG(shooter->GetForward().Heading());
	float angle   = DEGTORAD(heading);

	CVector2D ahead(-Sin(angle), Cos(angle));
	ahead.Normalise();

	CVector vel = ((CPed *)shooter)->m_vecMoveSpeed;
	int32 shooterMoving = false;
	if ( Abs(vel.x) > 0.0f && Abs(vel.y) > 0.0f )
		shooterMoving = true;

	if ( shooter == FindPlayerPed() )
	{
		static float prev_heading = 0.0f;
		prev_heading = ((CPed*)shooter)->m_fRotationCur;
	}

	if ( shooter->IsPed() && ((CPed *)shooter)->m_pPointGunAt )
	{
		CPed *shooterPed = (CPed *)shooter;
		if ( shooterPed->m_pedIK.m_flags & CPedIK::GUN_POINTED_SUCCESSFULLY )
		{
			int32 accuracy   = shooterPed->m_wepAccuracy;
			int32 inaccuracy = 100-accuracy;

			if ( accuracy != 100 )
				FindPlayerPed();	//what ?

			CPed *threatAttack = (CPed*)shooterPed->m_pPointGunAt;
			if ( threatAttack->IsPed() )
			{
				threatAttack->m_pedIK.GetComponentPosition(target, PED_MID);
				threatAttack->ReactToPointGun(shooter);
			}
			else
				target = threatAttack->GetPosition();

			target -= *fireSource;
			target *= info->m_fRange / target.Magnitude();
			target += *fireSource;

			if ( inaccuracy != 0 )
			{
				target.x += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * inaccuracy;
				target.y += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * inaccuracy;
				target.z += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * inaccuracy;
			}

			CWorld::bIncludeDeadPeds = true;
			ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);
			CWorld::bIncludeDeadPeds = false;
		}
		else
		{
			target.x = info->m_fRange;
			target.y = 0.0f;
			target.z = 0.0f;

			shooterPed->TransformToNode(target, PED_HANDR);

			ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);
		}
	}
	else if ( shooter == FindPlayerPed() && TheCamera.Cams[0].Using3rdPersonMouseCam()  )
	{
		CVector src, trgt;
		TheCamera.Find3rdPersonCamTargetVector(info->m_fRange, *fireSource, src, trgt);

		CWorld::bIncludeDeadPeds = true;
		ProcessLineOfSight(src, trgt,point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);
		CWorld::bIncludeDeadPeds = false;

		int32 rotSpeed = 1;
		if ( m_eWeaponType == WEAPONTYPE_M16  )
			rotSpeed = 4;

		CVector bulletPos;
		if ( CHeli::TestBulletCollision(&src, &trgt, &bulletPos, 4) )
		{
			for ( int32 i = 0; i < 16; i++ )
				CParticle::AddParticle(PARTICLE_SPARK, bulletPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, rotSpeed);
		}
	}
	else
	{
		float shooterHeading = RADTODEG(shooter->GetForward().Heading());
		float shooterAngle   = DEGTORAD(shooterHeading);

		CVector2D rotOffset(-Sin(shooterAngle), Cos(shooterAngle));
		rotOffset.Normalise();

		target   = *fireSource;
		target.x += rotOffset.x * info->m_fRange;
		target.y += rotOffset.y * info->m_fRange;

		if ( shooter->IsPed() )
			DoDoomAiming(shooter, fireSource, &target);

		ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);

		int32 rotSpeed = 1;
		if ( m_eWeaponType == WEAPONTYPE_M16  )
			rotSpeed = 4;

		CVector bulletPos;
		if ( CHeli::TestBulletCollision(fireSource, &target, &bulletPos, 4) )
		{
			for ( int32 i = 0; i < 16; i++ )
				CParticle::AddParticle(PARTICLE_SPARK, bulletPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, rotSpeed);
		}
	}

	if ( victim && shooter->IsPed() && victim == ((CPed*)shooter)->m_leader )
		return false;

	CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED, shooter, (CPed *)shooter, 1000);

	if ( shooter == FindPlayerPed() )
	{
		CStats::InstantHitsFiredByPlayer++;
		if ( !(CTimer::GetFrameCounter() & 3) )
			MakePedsJumpAtShot((CPhysical*)shooter, fireSource, &target);
	}

	switch ( m_eWeaponType )
	{
		case WEAPONTYPE_AK47:
		{
			static uint8 counter = 0;

			if ( !(++counter & 1) )
			{
				CPointLights::AddLight(CPointLights::LIGHT_POINT,
					*fireSource, CVector(0.0f, 0.0f, 0.0f), 5.0f,
					1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

				CVector gunflashPos = *fireSource;
				gunflashPos += CVector(0.06f*ahead.x, 0.06f*ahead.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.10f);
				gunflashPos += CVector(0.06f*ahead.x, 0.06f*ahead.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.08f);
				gunflashPos += CVector(0.05f*ahead.x, 0.05f*ahead.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.06f);
				gunflashPos += CVector(0.04f*ahead.x, 0.04f*ahead.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);

				CVector gunsmokePos = *fireSource;
				float rnd = CGeneral::GetRandomNumberInRange(0.05f, 0.25f);
				CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*rnd, ahead.y*rnd, 0.0f));

				CVector gunshellPos = *fireSource;
				gunshellPos -= CVector(0.5f*ahead.x, 0.5f*ahead.y, 0.0f);
				CVector dir = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
				dir.Normalise2D();
				AddGunshell(shooter, gunshellPos, CVector2D(dir.x, dir.y), 0.018f);
			}

			break;
		}

		case WEAPONTYPE_M16:
		{
			static uint8 counter = 0;

			if ( !(++counter & 1) )
			{
				CPointLights::AddLight(CPointLights::LIGHT_POINT,
					*fireSource, CVector(0.0f, 0.0f, 0.0f), 5.0f,
					1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

				CVector gunflashPos = *fireSource;
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.08f);
				gunflashPos += CVector(0.06f*ahead.x, 0.06f*ahead.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.06f);
				gunflashPos += CVector(0.06f*ahead.x, 0.06f*ahead.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.06f);

				gunflashPos = *fireSource;
				gunflashPos += CVector(-0.1f*ahead.x, -0.1f*ahead.y, 0.0f);
				gunflashPos.z += 0.04f;
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
				gunflashPos.z += 0.04f;
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);
				gunflashPos.z += 0.03f;
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

				gunflashPos = *fireSource;
				gunflashPos += CVector(-0.1f*ahead.x, -0.1f*ahead.y, 0.0f);
				gunflashPos.z -= 0.04f;
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
				gunflashPos.z -= 0.04f;
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);
				gunflashPos.z -= 0.03f;
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

				CVector offset = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
				offset.Normalise2D();

				gunflashPos = *fireSource;
				gunflashPos += CVector(-0.1f*ahead.x, -0.1f*ahead.y, 0.0f);
				gunflashPos += CVector(0.06f*offset.x, 0.06f*offset.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
				gunflashPos += CVector(0.04f*offset.x, 0.04f*offset.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.03f);
				gunflashPos += CVector(0.03f*offset.x, 0.03f*offset.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

				gunflashPos = *fireSource;
				gunflashPos += CVector(-0.1f*ahead.x, -0.1f*ahead.y, 0.0f);
				gunflashPos -= CVector(0.06f*offset.x, 0.06f*offset.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
				gunflashPos -= CVector(0.04f*offset.x, 0.04f*offset.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.03f);
				gunflashPos -= CVector(0.03f*offset.x, 0.03f*offset.y, 0.0f);
				CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

				CVector gunsmokePos = *fireSource;
				float rnd = CGeneral::GetRandomNumberInRange(0.05f, 0.25f);
				CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*rnd, ahead.y*rnd, 0.0f));

				CVector gunshellPos = *fireSource;
				gunshellPos -= CVector(0.65f*ahead.x, 0.65f*ahead.y, 0.0f);
				CVector dir = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
				dir.Normalise2D();
				AddGunshell(shooter, gunshellPos, CVector2D(dir.x, dir.y), 0.02f);
			}

			break;
		}

		case WEAPONTYPE_UZI:
		{
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
					*fireSource, CVector(0.0f, 0.0f, 0.0f), 5.0f,
					1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

			CVector gunflashPos = *fireSource;

			if ( shooterMoving )
				gunflashPos += CVector(1.5f*vel.x, 1.5f*vel.y, 0.0f);

			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.07f);
			gunflashPos += CVector(0.06f*ahead.x, 0.06f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.05f);
			gunflashPos += CVector(0.04f*ahead.x, 0.04f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
			gunflashPos += CVector(0.04f*ahead.x, 0.04f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.03f);
			gunflashPos += CVector(0.03f*ahead.x, 0.03f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.03f);
			gunflashPos += CVector(0.03f*ahead.x, 0.03f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);
			gunflashPos += CVector(0.02f*ahead.x, 0.02f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.01f);

			CVector gunsmokePos = *fireSource;
			float rnd = CGeneral::GetRandomNumberInRange(0.05f, 0.25f);
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*rnd, ahead.y*rnd, 0.0f));

			CVector gunshellPos = *fireSource;
			gunshellPos -= CVector(0.2f*ahead.x, 0.2f*ahead.y, 0.0f);
			CVector dir = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
			dir.Normalise2D();
			AddGunshell(shooter, gunshellPos, CVector2D(dir.x, dir.y), 0.015f);

			break;
		}

		case WEAPONTYPE_COLT45:
		{
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
					*fireSource, CVector(0.0f, 0.0f, 0.0f), 5.0f,
					1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

			CVector gunflashPos = *fireSource;

			if ( shooterMoving )
				gunflashPos += CVector(1.5f*vel.x, 1.5f*vel.y, 0.0f);

			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.06f);
			gunflashPos += CVector(0.06f*ahead.x, 0.06f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
			gunflashPos += CVector(0.04f*ahead.x, 0.04f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

			CVector gunsmokePos = *fireSource;
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*0.10f, ahead.y*0.10f, 0.0f), nil, 0.005f);
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*0.15f, ahead.y*0.15f, 0.0f), nil, 0.015f);
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*0.20f, ahead.y*0.20f, 0.0f), nil, 0.025f);

			CVector gunshellPos = *fireSource;
			gunshellPos -= CVector(0.2f*ahead.x, 0.2f*ahead.y, 0.0f);
			CVector dir = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
			dir.Normalise2D();
			AddGunshell(shooter, gunshellPos, CVector2D(dir.x, dir.y), 0.015f);

			break;
		}
		default: break;
	}

	DoBulletImpact(shooter, victim, fireSource, &target, &point, ahead);

	return true;
}

void
CWeapon::AddGunshell(CEntity *shooter, CVector const &source, CVector2D const &direction, float size)
{
	ASSERT(shooter!=nil);

	if ( shooter == nil)
		return;

	CVector dir(direction.x*0.05f, direction.y*0.05f, CGeneral::GetRandomNumberInRange(0.02f, 0.08f));

	static CVector prevEntityPosition(0.0f, 0.0f, 0.0f);
	CVector entityPosition = shooter->GetPosition();

	CVector diff = entityPosition - prevEntityPosition;

	if ( Abs(diff.x)+Abs(diff.y)+Abs(diff.z) > 1.5f )
	{
		prevEntityPosition = entityPosition;

		CParticle::AddParticle(PARTICLE_GUNSHELL_FIRST,
			source, dir, nil, size, CGeneral::GetRandomNumberInRange(-20.0f, 20.0f));
	}
	else
	{
		CParticle::AddParticle(PARTICLE_GUNSHELL,
			source, dir, nil, size, CGeneral::GetRandomNumberInRange(-20.0f, 20.0f));
	}
}

void
CWeapon::DoBulletImpact(CEntity *shooter, CEntity *victim,
		CVector *source, CVector *target, CColPoint *point, CVector2D ahead)
{
	ASSERT(shooter!=nil);
	ASSERT(source!=nil);
	ASSERT(target!=nil);
	ASSERT(point!=nil);

	CWeaponInfo *info = GetInfo();

	if ( victim )
	{
		CGlass::WasGlassHitByBullet(victim, point->point);

		CVector traceTarget = point->point;
		CBulletTraces::AddTrace(source, &traceTarget);

		if ( shooter != nil )
		{
			if ( shooter == FindPlayerPed() )
			{
				if ( victim->IsPed() || victim->IsVehicle() )
					CStats::InstantHitsHitByPlayer++;
			}
		}

		if ( victim->IsPed() && ((CPed*)shooter)->m_nPedType != ((CPed*)victim)->m_nPedType || ((CPed*)shooter)->m_nPedType == PEDTYPE_PLAYER2 )
		{
			CPed *victimPed = (CPed *)victim;
			if ( !victimPed->DyingOrDead() && victim != shooter )
			{
				if ( victimPed->DoesLOSBulletHitPed(*point) )
				{
					CVector pos = victimPed->GetPosition();

					CVector2D posOffset(source->x-pos.x, source->y-pos.y);
					int32 localDir = victimPed->GetLocalDirection(posOffset);

					victimPed->ReactToAttack(shooter);

					if ( !victimPed->IsPedInControl() || victimPed->bIsDucking )
					{
						victimPed->InflictDamage(shooter, m_eWeaponType, info->m_nDamage, (ePedPieceTypes)point->pieceB, localDir);
					}
					else
					{
						if ( m_eWeaponType == WEAPONTYPE_SHOTGUN || m_eWeaponType == WEAPONTYPE_HELICANNON )
						{
							posOffset.Normalise();
							victimPed->bIsStanding = false;

							victimPed->ApplyMoveForce(posOffset.x*-5.0f, posOffset.y*-5.0f, 5.0f);
							victimPed->SetFall(1500, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), false);

							victimPed->InflictDamage(shooter, m_eWeaponType, info->m_nDamage, (ePedPieceTypes)point->pieceB, localDir);
						}
						else
						{
							if ( victimPed->IsPlayer() )
							{
								CPlayerPed *victimPlayer = (CPlayerPed *)victimPed;
								if ( victimPlayer->m_nHitAnimDelayTimer < CTimer::GetTimeInMilliseconds() )
								{
									victimPed->ClearAttackByRemovingAnim();

									CAnimBlendAssociation *asoc = CAnimManager::AddAnimation(victimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + localDir));
									ASSERT(asoc!=nil);

									asoc->blendAmount = 0.0f;
									asoc->blendDelta  = 8.0f;

									if ( m_eWeaponType == WEAPONTYPE_AK47 || m_eWeaponType == WEAPONTYPE_M16 )
										victimPlayer->m_nHitAnimDelayTimer = CTimer::GetTimeInMilliseconds() + 2500;
									else
										victimPlayer->m_nHitAnimDelayTimer = CTimer::GetTimeInMilliseconds() + 1000;
								}
							}
							else
							{
								victimPed->ClearAttackByRemovingAnim();

								CAnimBlendAssociation *asoc = CAnimManager::AddAnimation(victimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + localDir));
								ASSERT(asoc!=nil);

								asoc->blendAmount = 0.0f;
								asoc->blendDelta  = 8.0f;
							}

							victimPed->InflictDamage(shooter, m_eWeaponType, info->m_nDamage, (ePedPieceTypes)point->pieceB, localDir);
						}
					}

					if ( victimPed->m_nPedType == PEDTYPE_COP )
						CEventList::RegisterEvent(EVENT_SHOOT_COP, EVENT_ENTITY_PED, victim, (CPed*)shooter, 10000);
					else
						CEventList::RegisterEvent(EVENT_SHOOT_PED, EVENT_ENTITY_PED, victim, (CPed*)shooter, 10000);

					if ( CGame::nastyGame )
					{
						uint8 bloodAmount = 8;
						if ( m_eWeaponType == WEAPONTYPE_SHOTGUN || m_eWeaponType == WEAPONTYPE_HELICANNON )
							bloodAmount = 32;

						CVector dir = (point->point - victim->GetPosition()) * 0.01f;
						dir.z = 0.01f;

						if ( victimPed->GetIsOnScreen() )
						{
							for ( uint8 i = 0; i < bloodAmount; i++ )
								CParticle::AddParticle(PARTICLE_BLOOD_SMALL, point->point, dir);
						}
					}
				}
			}
			else
			{
				if ( CGame::nastyGame )
				{
					CVector dir = (point->point - victim->GetPosition()) * 0.01f;
					dir.z = 0.01f;

					if ( victim->GetIsOnScreen() )
					{
						for ( int32 i = 0; i < 8; i++ )
							CParticle::AddParticle(PARTICLE_BLOOD_SMALL, point->point + CVector(0.0f, 0.0f, 0.15f), dir);
					}

					if ( victimPed->Dead() )
					{
						CAnimBlendAssociation *asoc;
						if ( RpAnimBlendClumpGetFirstAssociation(victimPed->GetClump(), ASSOC_FRONTAL) )
							asoc = CAnimManager::BlendAnimation(victimPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR_FRONT, 8.0f);
						else
							asoc = CAnimManager::BlendAnimation(victimPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR,   8.0f);

						if ( asoc )
						{
							asoc->SetCurrentTime(0.0f);
							asoc->flags |= ASSOC_RUNNING;
							asoc->flags &= ~ASSOC_FADEOUTWHENDONE;
						}
					}
				}
			}
		}
		else
		{
			switch ( victim->GetType() )
			{
				case ENTITY_TYPE_BUILDING:
				{
					for ( int32 i = 0; i < 16; i++ )
						CParticle::AddParticle(PARTICLE_SPARK, point->point, point->normal*0.05f);

#ifndef FIX_BUGS
				    CVector dist = point->point - (*source);
				    CVector offset = dist - Max(0.2f * dist.Magnitude(), 2.0f) * CVector(ahead.x, ahead.y, 0.0f);
				    CVector smokePos = *source + offset;
#else
				    CVector smokePos = point->point;
#endif // !FIX_BUGS

					smokePos.x += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
					smokePos.y += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
					smokePos.z += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);

					CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, smokePos, CVector(0.0f, 0.0f, 0.0f));

					break;
				}
				case ENTITY_TYPE_VEHICLE:
				{
					((CVehicle *)victim)->InflictDamage(shooter, m_eWeaponType, info->m_nDamage);

					for ( int32 i = 0; i < 16; i++ )
						CParticle::AddParticle(PARTICLE_SPARK, point->point, point->normal*0.05f);

#ifndef FIX_BUGS
					CVector dist = point->point - (*source);
					CVector offset = dist - Max(0.2f*dist.Magnitude(), 0.5f) * CVector(ahead.x, ahead.y, 0.0f);
				    CVector smokePos = *source + offset;
#else
				    CVector smokePos = point->point;
#endif // !FIX_BUGS

					CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, smokePos, CVector(0.0f, 0.0f, 0.0f));

					if ( shooter->IsPed() )
					{
						CPed *shooterPed = (CPed *)shooter;

						if ( shooterPed->bNotAllowedToDuck )
						{
							if ( shooterPed->bKindaStayInSamePlace && victim != shooterPed->m_pPointGunAt )
							{
								shooterPed->bKindaStayInSamePlace = false;
								shooterPed->m_duckAndCoverTimer   = CTimer::GetTimeInMilliseconds() + 15000;
							}
						}
					}

					break;
				}
				case ENTITY_TYPE_OBJECT:
				{
					for ( int32 i = 0; i < 8; i++ )
						CParticle::AddParticle(PARTICLE_SPARK, point->point, point->normal*0.05f);

					CObject *victimObject = (CObject *)victim;

					if ( !victimObject->bInfiniteMass )
					{
						if ( victimObject->GetIsStatic() && victimObject->m_fUprootLimit <= 0.0f )
						{
							victimObject->SetIsStatic(false);
							victimObject->AddToMovingList();
						}

						if ( !victimObject->GetIsStatic())
						{
							CVector moveForce = point->normal*-4.0f;
							victimObject->ApplyMoveForce(moveForce.x, moveForce.y, moveForce.z);
						}
					}

					break;
				}
				default: break;
			}
		}

		switch ( victim->GetType() )
		{
			case ENTITY_TYPE_BUILDING:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_1, point->point);
				break;
			}
			case ENTITY_TYPE_VEHICLE:
			{
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
				break;
			}
			case ENTITY_TYPE_PED:
			{
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
				((CPed*)victim)->Say(SOUND_PED_BULLET_HIT);
				break;
			}
			case ENTITY_TYPE_OBJECT:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_2, point->point);
				break;
			}
			case ENTITY_TYPE_DUMMY:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_3, point->point);
				break;
			}
			default: break;
		}
	}
	else
		CBulletTraces::AddTrace(source, target);

	if ( shooter == FindPlayerPed() )
		CPad::GetPad(0)->StartShake_Distance(240, 128, FindPlayerPed()->GetPosition().x, FindPlayerPed()->GetPosition().y, FindPlayerPed()->GetPosition().z);

	BlowUpExplosiveThings(victim);
}

bool
CWeapon::FireShotgun(CEntity *shooter, CVector *fireSource)
{
	ASSERT(shooter!=nil);
	ASSERT(fireSource!=nil);

	CWeaponInfo *info = GetInfo();

	float heading = RADTODEG(shooter->GetForward().Heading());
	float angle   = DEGTORAD(heading);

	CVector2D rotOffset(-Sin(angle), Cos(angle));
	rotOffset.Normalise();

	CVector gunflashPos = *fireSource;
	gunflashPos += CVector(rotOffset.x*0.1f, rotOffset.y*0.1f, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f);
	gunflashPos += CVector(rotOffset.x*0.1f, rotOffset.y*0.1f, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.15f);
	gunflashPos += CVector(rotOffset.x*0.1f, rotOffset.y*0.1f, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.2f);
	CParticle::AddParticle(PARTICLE_GUNFLASH, *fireSource, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f);

	CVector gunsmokePos = *fireSource;
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(rotOffset.x*0.10f, rotOffset.y*0.10f, 0.0f), nil, 0.1f);
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(rotOffset.x*0.15f, rotOffset.y*0.15f, 0.0f), nil, 0.1f);
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(rotOffset.x*0.20f, rotOffset.y*0.20f, 0.0f), nil, 0.1f);
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(rotOffset.x*0.25f, rotOffset.y*0.25f, 0.0f), nil, 0.1f);

	CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED, shooter, (CPed*)shooter, 1000);

	CPointLights::AddLight(CPointLights::LIGHT_POINT, *fireSource, CVector(0.0, 0.0, 0.0), 5.0f,
							1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

	float shooterAngle;

	if ( shooter->IsPed() && ((CPed*)shooter)->m_pPointGunAt != nil )
	{
		CEntity *threatAttack = ((CPed*)shooter)->m_pPointGunAt;
		shooterAngle = CGeneral::GetAngleBetweenPoints(threatAttack->GetPosition().x, threatAttack->GetPosition().y,
														(*fireSource).x, (*fireSource).y);
	}
	else
		shooterAngle = RADTODEG(shooter->GetForward().Heading());


	for ( int32 i = 0; i < 5; i++ ) // five shoots at once
	{
		float shootAngle = DEGTORAD(7.5f*i + shooterAngle - 15.0f);
		CVector2D shootRot(-Sin(shootAngle), Cos(shootAngle));
		shootRot.Normalise();

		CVector source, target;
		CColPoint point;
		CEntity *victim;

		if ( shooter == FindPlayerPed() && TheCamera.Cams[0].Using3rdPersonMouseCam() )
		{
			TheCamera.Find3rdPersonCamTargetVector(1.0f, *fireSource, source, target);
			CVector Left = CrossProduct(TheCamera.Cams[TheCamera.ActiveCam].Front, TheCamera.Cams[TheCamera.ActiveCam].Up);

			float f = float(i - 2) * (DEGTORAD(7.5f) / 2);
			target  = f * Left + target - source;
			target *= info->m_fRange;
			target += source;

			ProcessLineOfSight(source, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);
		}
		else
		{
			target = *fireSource;
			target.x += shootRot.x * info->m_fRange;
			target.y += shootRot.y * info->m_fRange;

			if ( shooter->IsPed() )
			{
				CPed *shooterPed = (CPed *)shooter;

				if ( shooterPed->m_pPointGunAt == nil )
					DoDoomAiming(shooter, fireSource, &target);
				else
				{
					float distToTarget = (shooterPed->m_pPointGunAt->GetPosition() - (*fireSource)).Magnitude2D();
					target.z += info->m_fRange / distToTarget * (shooterPed->m_pPointGunAt->GetPosition().z - target.z);
				}
			}

			ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);
		}

		if ( victim )
		{
			CGlass::WasGlassHitByBullet(victim, point.point);

			CBulletTraces::AddTrace(fireSource, &point.point);

			if ( victim->IsPed() )
			{
				CPed *victimPed = (CPed *)victim;
				if ( !victimPed->OnGround() && victim != shooter && victimPed->DoesLOSBulletHitPed(point) )
				{
					bool cantStandup = true;

					CVector pos = victimPed->GetPosition();

					CVector2D posOffset((*fireSource).x-pos.x, (*fireSource).y-pos.y);
					int32 localDir = victimPed->GetLocalDirection(posOffset);

					victimPed->ReactToAttack(FindPlayerPed());

					posOffset.Normalise();

					if ( victimPed->m_getUpTimer > (CTimer::GetTimeInMilliseconds() - 3000) )
						cantStandup = false;

					if ( victimPed->bIsStanding && cantStandup )
					{
						victimPed->bIsStanding = false;

						victimPed->ApplyMoveForce(posOffset.x*-6.0f, posOffset.y*-6.0f, 5.0f);
					}
					else
						victimPed->ApplyMoveForce(posOffset.x*-2.0f, posOffset.y*-2.0f, 0.0f);

					if ( cantStandup )
						victimPed->SetFall(1500, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), false);

					victimPed->InflictDamage(shooter, m_eWeaponType, info->m_nDamage, (ePedPieceTypes)point.pieceB, localDir);

					if ( victimPed->m_nPedType == PEDTYPE_COP )
						CEventList::RegisterEvent(EVENT_SHOOT_COP, EVENT_ENTITY_PED, victim, (CPed*)shooter, 10000);
					else
						CEventList::RegisterEvent(EVENT_SHOOT_PED, EVENT_ENTITY_PED, victim, (CPed*)shooter, 10000);

					if ( CGame::nastyGame )
					{
						uint8 bloodAmount = 8;
						if ( m_eWeaponType == WEAPONTYPE_SHOTGUN )
							bloodAmount = 32;

						CVector dir = (point.point - victim->GetPosition()) * 0.01f;
						dir.z = 0.01f;

						if ( victimPed->GetIsOnScreen() )
						{
							for ( uint8 i = 0; i < bloodAmount; i++ )
								CParticle::AddParticle(PARTICLE_BLOOD_SMALL, point.point, dir);
						}
					}
				}
			}
			else
			{
				switch ( victim->GetType() )
				{
					case ENTITY_TYPE_VEHICLE:
					{
						((CVehicle *)victim)->InflictDamage(shooter, m_eWeaponType, info->m_nDamage);

						for ( int32 i = 0; i < 16; i++ )
							CParticle::AddParticle(PARTICLE_SPARK, point.point, point.normal*0.05f);

#ifndef FIX_BUGS
						CVector dist = point.point - (*fireSource);
						CVector offset = dist - Max(0.2f*dist.Magnitude(), 2.0f) * CVector(shootRot.x, shootRot.y, 0.0f);
						CVector smokePos = *fireSource + offset;
#else
					    CVector smokePos = point.point;
#endif

						CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, smokePos, CVector(0.0f, 0.0f, 0.0f));

						break;
					}

					case ENTITY_TYPE_BUILDING:
					case ENTITY_TYPE_OBJECT:
					{
						for ( int32 i = 0; i < 16; i++ )
							CParticle::AddParticle(PARTICLE_SPARK, point.point, point.normal*0.05f);

#ifndef FIX_BUGS
						CVector dist = point.point - (*fireSource);
						CVector offset = dist - Max(0.2f*dist.Magnitude(), 2.0f) * CVector(shootRot.x, shootRot.y, 0.0f);
					    CVector smokePos = *fireSource + offset;
#else
					    CVector smokePos = point.point;
#endif

						smokePos.x += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
						smokePos.y += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
						smokePos.z += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);

						CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, smokePos, CVector(0.0f, 0.0f, 0.0f));

						if ( victim->IsObject() )
						{
							CObject *victimObject = (CObject *)victim;

							if ( !victimObject->bInfiniteMass )
							{
								if ( victimObject->GetIsStatic() && victimObject->m_fUprootLimit <= 0.0f )
								{
									victimObject->SetIsStatic(false);
									victimObject->AddToMovingList();
								}

								if ( !victimObject->GetIsStatic())
								{
									CVector moveForce = point.normal*-5.0f;
									victimObject->ApplyMoveForce(moveForce.x, moveForce.y, moveForce.z);
								}
							}
						}

						break;
					}
					default: break;
				}
			}

			switch ( victim->GetType() )
			{
				case ENTITY_TYPE_BUILDING:
				{
					PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_1, point.point);
					break;
				}
				case ENTITY_TYPE_VEHICLE:
				{
					DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
					break;
				}
				case ENTITY_TYPE_PED:
				{
					DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
					((CPed*)victim)->Say(SOUND_PED_BULLET_HIT);
					break;
				}
				case ENTITY_TYPE_OBJECT:
				{
					PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_2, point.point);
					break;
				}
				case ENTITY_TYPE_DUMMY:
				{
					PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_3, point.point);
					break;
				}
				default: break;
			}
		}
		else
		{
			CVector traceTarget = *fireSource;
			traceTarget += (target - (*fireSource)) * Min(info->m_fRange, 30.0f) / info->m_fRange;
			CBulletTraces::AddTrace(fireSource, &traceTarget);
		}
	}

	if ( shooter == FindPlayerPed() )
		CPad::GetPad(0)->StartShake_Distance(240, 128, FindPlayerPed()->GetPosition().x, FindPlayerPed()->GetPosition().y, FindPlayerPed()->GetPosition().z);

	return true;
}

bool
CWeapon::FireProjectile(CEntity *shooter, CVector *fireSource, float power)
{
	ASSERT(shooter!=nil);
	ASSERT(fireSource!=nil);

	CVector source, target;

	if ( m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER )
	{
		source = *fireSource;

		if ( shooter->IsPed() && ((CPed*)shooter)->IsPlayer() )
		{
			int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
			if (!( mode == CCam::MODE_M16_1STPERSON
				|| mode == CCam::MODE_SNIPER
				|| mode == CCam::MODE_ROCKETLAUNCHER
				|| mode == CCam::MODE_M16_1STPERSON_RUNABOUT
				|| mode == CCam::MODE_SNIPER_RUNABOUT
				|| mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT) )
			{
				return false;
			}

			*fireSource += TheCamera.Cams[TheCamera.ActiveCam].Front;
		}
		else
			*fireSource += shooter->GetForward();

		target = *fireSource;
	}
	else
	{
		float dot = DotProduct(*fireSource-shooter->GetPosition(), shooter->GetForward());

		if ( dot < 0.3f )
			*fireSource += (0.3f-dot) * shooter->GetForward();

		target = *fireSource;

		if ( target.z - shooter->GetPosition().z > 0.0f )
			target += 0.6f*shooter->GetForward();

		source = *fireSource - shooter->GetPosition();

		source = *fireSource - DotProduct(source, shooter->GetForward()) * shooter->GetForward();
	}

	if ( !CWorld::GetIsLineOfSightClear(source, target, true, true, false, true, false, false, false) )
	{
		if ( m_eWeaponType != WEAPONTYPE_GRENADE )
			CProjectileInfo::RemoveNotAdd(shooter, m_eWeaponType, *fireSource);
		else
		{
			if ( shooter->IsPed() )
			{
				source    = shooter->GetPosition() - shooter->GetForward();
				source.z -= 0.4f;

				if ( !CWorld::TestSphereAgainstWorld(source, 0.5f, nil, false, false, true, false, false, false) )
					CProjectileInfo::AddProjectile(shooter, m_eWeaponType, source, 0.0f);
				else
					CProjectileInfo::RemoveNotAdd(shooter, m_eWeaponType, *fireSource);
			}
		}
	}
	else
		CProjectileInfo::AddProjectile(shooter, m_eWeaponType, *fireSource, power);

	return true;
}

void
CWeapon::GenerateFlameThrowerParticles(CVector pos, CVector dir)
{
	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);

	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);

	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);

	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);

	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);
}

bool
CWeapon::FireAreaEffect(CEntity *shooter, CVector *fireSource)
{
	ASSERT(shooter!=nil);
	ASSERT(fireSource!=nil);

	CWeaponInfo *info = GetInfo();

	float heading = RADTODEG(shooter->GetForward().Heading());

	CVector source;
	CVector target;
	CVector dir;

	if ( shooter == FindPlayerPed() && TheCamera.Cams[0].Using3rdPersonMouseCam() )
	{
		TheCamera.Find3rdPersonCamTargetVector(info->m_fRange, *fireSource, source, target);
		float norm = (1.0f / info->m_fRange);
		dir = (target - source) * norm;
	}
	else
	{
		float angle = DEGTORAD(heading);
		dir         = CVector(-Sin(angle)*0.5f, Cos(angle)*0.5f, 0.0f);
		target      = *fireSource + dir;
	}

	CShotInfo::AddShot(shooter, m_eWeaponType, *fireSource, target);
	CWeapon::GenerateFlameThrowerParticles(*fireSource, dir);

	return true;
}

bool
CWeapon::FireSniper(CEntity *shooter)
{
	ASSERT(shooter!=nil);

	int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
	if (!( mode == CCam::MODE_M16_1STPERSON
		|| mode == CCam::MODE_SNIPER
		|| mode == CCam::MODE_ROCKETLAUNCHER
		|| mode == CCam::MODE_M16_1STPERSON_RUNABOUT
		|| mode == CCam::MODE_SNIPER_RUNABOUT
		|| mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT) )
	{
		return false;
	}

#ifndef FIX_BUGS
	CWeaponInfo *info = GetInfo(); //unused
#endif

	CCam *cam = &TheCamera.Cams[TheCamera.ActiveCam];
	ASSERT(cam!=nil);

	CVector source = cam->Source;
	CVector dir    = cam->Front;

	if ( DotProduct(dir, CVector(0.0f, -0.9894f, 0.145f)) > 0.997f )
		CCoronas::bSmallMoon = !CCoronas::bSmallMoon;

	dir.Normalise();
	dir *= 16.0f;

	CBulletInfo::AddBullet(shooter, m_eWeaponType, source, dir);

	if ( shooter == FindPlayerPed() )
		CStats::InstantHitsFiredByPlayer++;

	if ( shooter == FindPlayerPed() )
	{
		CPad::GetPad(0)->StartShake_Distance(240, 128,
			FindPlayerPed()->GetPosition().x,
			FindPlayerPed()->GetPosition().y,
			FindPlayerPed()->GetPosition().z);

		CamShakeNoPos(&TheCamera, 0.2f);
	}

	return true;
}

bool
CWeapon::FireM16_1stPerson(CEntity *shooter)
{
	ASSERT(shooter!=nil);

	int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;

	if (!( mode == CCam::MODE_M16_1STPERSON
		|| mode == CCam::MODE_SNIPER
		|| mode == CCam::MODE_ROCKETLAUNCHER
		|| mode == CCam::MODE_M16_1STPERSON_RUNABOUT
		|| mode == CCam::MODE_SNIPER_RUNABOUT
		|| mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT
		|| mode == CCam::MODE_HELICANNON_1STPERSON) )
	{
		return false;
	}

	CWeaponInfo *info = GetInfo();

	CColPoint point;
	CEntity *victim;

	CWorld::bIncludeCarTyres = true;
	CWorld::pIgnoreEntity = shooter;
	CWorld::bIncludeDeadPeds = true;

	CCam *cam = &TheCamera.Cams[TheCamera.ActiveCam];
	ASSERT(cam!=nil);

	CVector source = cam->Source;
	CVector target = cam->Front*info->m_fRange + source;

	ProcessLineOfSight(source, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);
	CWorld::bIncludeDeadPeds = false;
	CWorld::pIgnoreEntity = nil;
	CWorld::bIncludeCarTyres = false;

	CVector2D front(cam->Front.x, cam->Front.y);
	front.Normalise();

	DoBulletImpact(shooter, victim, &source, &target, &point, front);

	CVector bulletPos;
	if ( CHeli::TestBulletCollision(&source, &target, &bulletPos, 4) )
	{
		for ( int32 i = 0; i < 16; i++ )
			CParticle::AddParticle(PARTICLE_SPARK, bulletPos, CVector(0.0f, 0.0f, 0.0f));
	}

	if ( shooter == FindPlayerPed() )
	{
#ifdef FIX_BUGS
		CStats::InstantHitsFiredByPlayer++;
#endif
		CPad::GetPad(0)->StartShake_Distance(240, 128, FindPlayerPed()->GetPosition().x, FindPlayerPed()->GetPosition().y, FindPlayerPed()->GetPosition().z);

		if ( m_eWeaponType == WEAPONTYPE_M16 )
		{
			TheCamera.Cams[TheCamera.ActiveCam].Beta  += float((CGeneral::GetRandomNumber() & 127) - 64) * 0.0003f;
			TheCamera.Cams[TheCamera.ActiveCam].Alpha += float((CGeneral::GetRandomNumber() & 127) - 64) * 0.0003f;
		}
		else if ( m_eWeaponType == WEAPONTYPE_HELICANNON )
		{
			TheCamera.Cams[TheCamera.ActiveCam].Beta  += float((CGeneral::GetRandomNumber() & 127) - 64) * 0.0001f;
			TheCamera.Cams[TheCamera.ActiveCam].Alpha += float((CGeneral::GetRandomNumber() & 127) - 64) * 0.0001f;
		}
	}

	return true;
}

bool
CWeapon::FireInstantHitFromCar(CAutomobile *shooter, bool left)
{
	CWeaponInfo *info = GetInfo();

	CVehicleModelInfo *modelInfo = shooter->GetModelInfo();

	CVector source, target;
	if ( left )
	{
		source = shooter->GetMatrix() * CVector(-shooter->GetColModel()->boundingBox.max.x + -0.2f,
												float(CGeneral::GetRandomNumber() & 255) * 0.001f + modelInfo->GetFrontSeatPosn().y,
												modelInfo->GetFrontSeatPosn().z + 0.5f);
		source += CTimer::GetTimeStep() * shooter->m_vecMoveSpeed;


		target = shooter->GetMatrix() * CVector(-info->m_fRange,
													modelInfo->GetFrontSeatPosn().y,
													modelInfo->GetFrontSeatPosn().z + 0.5f);
	}
	else
	{
		source = shooter->GetMatrix() * CVector(shooter->GetColModel()->boundingBox.max.x + 0.2f,
												float(CGeneral::GetRandomNumber() & 255) * 0.001f + modelInfo->GetFrontSeatPosn().y,
												modelInfo->GetFrontSeatPosn().z + 0.5f);
		source += CTimer::GetTimeStep() * shooter->m_vecMoveSpeed;

		target = shooter->GetMatrix() * CVector(info->m_fRange,
													modelInfo->GetFrontSeatPosn().y,
													modelInfo->GetFrontSeatPosn().z + 0.5f);
	}
	#undef FRONTSEATPOS

	if ( TheCamera.GetLookingLRBFirstPerson() && !left )
	{
		source -= 0.3f * shooter->GetForward();
		target -= 0.3f * shooter->GetForward();
	}

	target += CVector(float(CGeneral::GetRandomNumber()&255)*0.01f-1.28f,
						float(CGeneral::GetRandomNumber()&255)*0.01f-1.28f,
						float(CGeneral::GetRandomNumber()&255)*0.01f-1.28f);

	DoDriveByAutoAiming(FindPlayerPed(), &source, &target);

	CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED, FindPlayerPed(), FindPlayerPed(), 1000);

	if ( !TheCamera.GetLookingLRBFirstPerson() )
		CParticle::AddParticle(PARTICLE_GUNFLASH, source, CVector(0.0f, 0.0f, 0.0f));
	else
		CamShakeNoPos(&TheCamera, 0.01f);

	CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_VEHICLE, shooter, FindPlayerPed(), 1000);

	CPointLights::AddLight(CPointLights::LIGHT_POINT, source, CVector(0.0f, 0.0f, 0.0f), 5.0f,
		1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

	CColPoint point;
	CEntity *victim;
	ProcessLineOfSight(source, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);

	if ( !(CTimer::GetFrameCounter() & 3) )
		MakePedsJumpAtShot(shooter, &source, &target);

	if ( victim )
	{
		CVector traceTarget = point.point;
		CBulletTraces::AddTrace(&source, &traceTarget);

		if ( victim->IsPed() )
		{
			CPed *victimPed = (CPed*)victim;

			if ( !victimPed->DyingOrDead() && victim != (CEntity *)shooter )
			{
				CVector pos = victimPed->GetPosition();

				CVector2D posOffset(source.x-pos.x, source.y-pos.y);
				int32 localDir = victimPed->GetLocalDirection(posOffset);

				victimPed->ReactToAttack(FindPlayerPed());
				victimPed->ClearAttackByRemovingAnim();

				CAnimBlendAssociation *asoc = CAnimManager::AddAnimation(victimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + localDir));
				ASSERT(asoc!=nil);
				asoc->blendAmount = 0.0f;
				asoc->blendDelta  = 8.0f;

				victimPed->InflictDamage(shooter, WEAPONTYPE_UZI_DRIVEBY, 3*info->m_nDamage, (ePedPieceTypes)point.pieceB, localDir);

				pos.z += 0.8f;

				if ( victimPed->GetIsOnScreen() )
				{
					if ( CGame::nastyGame )
					{
						for ( int32 i = 0; i < 4; i++ )
						{
							CVector dir;
							dir.x = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
							dir.y = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
							dir.z = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);

							CParticle::AddParticle(PARTICLE_BLOOD, pos, dir);
						}
					}
				}

				if ( victimPed->m_nPedType == PEDTYPE_COP )
					CEventList::RegisterEvent(EVENT_SHOOT_COP, EVENT_ENTITY_PED, victimPed, FindPlayerPed(), 10000);
				else
					CEventList::RegisterEvent(EVENT_SHOOT_PED, EVENT_ENTITY_PED, victimPed, FindPlayerPed(), 10000);
			}
		}
		else if ( victim->IsVehicle() )
			((CVehicle *)victim)->InflictDamage(FindPlayerPed(), WEAPONTYPE_UZI_DRIVEBY, info->m_nDamage);
		else
			CGlass::WasGlassHitByBullet(victim, point.point);

		switch ( victim->GetType() )
		{
			case ENTITY_TYPE_BUILDING:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_1, point.point);
				break;
			}
			case ENTITY_TYPE_VEHICLE:
			{
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
				break;
			}
			case ENTITY_TYPE_PED:
			{
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
				((CPed*)victim)->Say(SOUND_PED_BULLET_HIT);
				break;
			}
			case ENTITY_TYPE_OBJECT:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_2, point.point);
				break;
			}
			case ENTITY_TYPE_DUMMY:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_3, point.point);
				break;
			}
			default: break;
		}
	}
	else
	{
		float norm = 30.0f/info->m_fRange;
		CVector traceTarget = (target-source)*norm + source;
		CBulletTraces::AddTrace(&source, &traceTarget);
	}

	if ( shooter == FindPlayerVehicle() )
		CPad::GetPad(0)->StartShake_Distance(240, 128, FindPlayerVehicle()->GetPosition().x, FindPlayerVehicle()->GetPosition().y, FindPlayerVehicle()->GetPosition().z);

	return true;
}

void
CWeapon::DoDoomAiming(CEntity *shooter, CVector *source, CVector *target)
{
	ASSERT(shooter!=nil);
	ASSERT(source!=nil);
	ASSERT(target !=nil);

#ifndef FIX_BUGS
	CEntity entity; // unused
#endif

	CPed *shooterPed = (CPed*)shooter;
	if ( shooterPed->IsPed() && shooterPed->bCrouchWhenShooting )
		return;

	int16 lastEntity;
	CEntity *entities[16];
	CWorld::FindObjectsInRange(*source, (*target-*source).Magnitude(), true, &lastEntity, 15, entities, false, true, true, false, false);

	float closestEntityDist = 10000.0f;
	int16 closestEntity;

	for ( int32 i = 0; i < lastEntity; i++ )
	{
		CEntity *victim = entities[i];
		ASSERT(victim!=nil);

		if ( (CEntity*)shooterPed != victim && shooterPed->CanSeeEntity(victim, DEGTORAD(22.5f)) )
		{
			if ( !(victim->GetStatus() == STATUS_TRAIN_MOVING
				|| victim->GetStatus() == STATUS_TRAIN_NOT_MOVING
				|| victim->GetStatus() == STATUS_HELI
				|| victim->GetStatus() == STATUS_PLANE) )
			{
				float distToVictim   = (shooterPed->GetPosition()-victim->GetPosition()).Magnitude2D();
				float distToVictimZ  = Abs(shooterPed->GetPosition().z-victim->GetPosition().z);

				if ( 1.5f*distToVictimZ < distToVictim )
				{
					float entityDist = Sqrt(SQR(distToVictim) + SQR(distToVictimZ));

					if ( entityDist < closestEntityDist )
					{
						closestEntityDist = entityDist;
						closestEntity = i;
					}
				}
			}
		}
	}

	if ( closestEntityDist < DOOMAUTOAIMING_MAXDIST )
	{
		CEntity *victim = entities[closestEntity];
		ASSERT(victim !=nil);

		float distToTarget = (*target - *source).Magnitude2D();
		float distToSource = (victim->GetPosition() - *source).Magnitude2D();

		float victimZ = victim->GetPosition().z + 0.3f;
		if ( victim->IsPed() )
		{
			if ( ((CPed*)victim)->bIsDucking )
				victimZ -= 0.8f;
		}

		(*target).z = (distToTarget / distToSource) * (victimZ - (*source).z) + (*source).z;
	}
}

void
CWeapon::DoTankDoomAiming(CEntity *shooter, CEntity *driver, CVector *source, CVector *target)
{
	ASSERT(shooter!=nil);
	ASSERT(driver!=nil);
	ASSERT(source!=nil);
	ASSERT(target!=nil);

#ifndef FIX_BUGS
	CEntity entity; // unused
#endif

	int16 lastEntity;
	CEntity *entities[16];
	CWorld::FindObjectsInRange(*source, (*target-*source).Magnitude(), true, &lastEntity, 15, entities, false, true, false, false, false);

	float closestEntityDist = 10000.0f;
	int16 closestEntity;

	float normZ = (target->z - source->z) / (*target-*source).Magnitude();

	for ( int32 i = 0; i < lastEntity; i++ )
	{
		CEntity *victim = entities[i];

		ASSERT(victim!=nil);

		if ( shooter != victim && driver != victim )
		{
			if ( !(victim->GetStatus() == STATUS_TRAIN_MOVING
				|| victim->GetStatus() == STATUS_TRAIN_NOT_MOVING
				|| victim->GetStatus() == STATUS_HELI
				|| victim->GetStatus() == STATUS_PLANE) )
			{
				if ( !(victim->IsVehicle() && victim->bRenderScorched) )
				{
					float distToVictim  = (shooter->GetPosition()-victim->GetPosition()).Magnitude2D();
					float distToVictimZ = Abs(shooter->GetPosition().z - (distToVictim*normZ + victim->GetPosition().z));

					if ( 3.0f*distToVictimZ < distToVictim )
					{
						CVector tmp = CVector(victim->GetPosition().x, victim->GetPosition().y, 0.0f);
						if ( CCollision::DistToLine(source, target,
								&tmp) < victim->GetBoundRadius()*3.0f )
						{
							float vehicleDist = Sqrt(SQR(distToVictim) + SQR(distToVictimZ));
							if ( vehicleDist < closestEntityDist )
							{
								closestEntityDist = vehicleDist;
								closestEntity = i;
							}
						}
					}
				}
			}
		}
	}

	if ( closestEntityDist < DOOMAUTOAIMING_MAXDIST )
	{
		CEntity *victim = entities[closestEntity];
		ASSERT(victim!=nil);

		float distToTarget = (*target - *source).Magnitude2D();
		float distToSource = (victim->GetPosition() - *source).Magnitude2D();

		(*target).z = (distToTarget / distToSource) * (0.3f + victim->GetPosition().z - (*source).z) + (*source).z;
	}
}

void
CWeapon::DoDriveByAutoAiming(CEntity *shooter, CVector *source, CVector *target)
{
	ASSERT(shooter!=nil);
	ASSERT(source!=nil);
	ASSERT(target!=nil);

#ifndef FIX_BUGS
	CEntity entity; // unused
#endif

	CPed *shooterPed = (CPed*)shooter;
	if ( shooterPed->IsPed() && shooterPed->bCrouchWhenShooting )
		return;

	int16 lastEntity;
	CEntity *entities[16];
	CWorld::FindObjectsInRange(*source, (*target-*source).Magnitude(), true, &lastEntity, 15, entities, false, false, true, false, false);

	float closestEntityDist = 10000.0f;
	int16 closestEntity;

	for ( int32 i = 0; i < lastEntity; i++ )
	{
		CEntity *victim = entities[i];
		ASSERT(victim!=nil);

		if ( shooter != victim )
		{
			float lineDist = CCollision::DistToLine(source, target, &victim->GetPosition());
			float distToVictim  = (victim->GetPosition() - shooter->GetPosition()).Magnitude();
			float pedDist = 0.15f*distToVictim + lineDist;

			if ( DotProduct((*target-*source), victim->GetPosition()-*source) > 0.0f && pedDist < closestEntityDist)
			{
				closestEntity = i;
				closestEntityDist = pedDist;
			}
		}
	}

	if ( closestEntityDist < DRIVEBYAUTOAIMING_MAXDIST )
	{
		CEntity *victim = entities[closestEntity];
		ASSERT(victim!=nil);

		float distToTarget = (*source - *target).Magnitude();
		float distToSource      = (*source - victim->GetPosition()).Magnitude();
		*target = (distToTarget / distToSource) * (victim->GetPosition() - *source) + *source;
	}
}

void
CWeapon::Reload(void)
{
	if (m_nAmmoTotal == 0)
		return;

	CWeaponInfo *info = GetInfo();

	if (m_nAmmoTotal >= info->m_nAmountofAmmunition)
		m_nAmmoInClip = info->m_nAmountofAmmunition;
	else
		m_nAmmoInClip = m_nAmmoTotal;
}

void
CWeapon::Update(int32 audioEntity)
{
	switch ( m_eWeaponState )
	{
		case WEAPONSTATE_MELEE_MADECONTACT:
		{
			m_eWeaponState = WEAPONSTATE_READY;
			break;
		}

		case WEAPONSTATE_FIRING:
		{
			if ( m_eWeaponType == WEAPONTYPE_SHOTGUN && AEHANDLE_IS_OK(audioEntity) )
			{
				uint32 timePassed = m_nTimer - gReloadSampleTime[WEAPONTYPE_SHOTGUN];
				if ( CTimer::GetPreviousTimeInMilliseconds() < timePassed && CTimer::GetTimeInMilliseconds() >= timePassed )
					DMAudio.PlayOneShot(audioEntity, SOUND_WEAPON_RELOAD, 0.0f);
			}

			if ( CTimer::GetTimeInMilliseconds() > m_nTimer )
			{
				if ( GetInfo()->m_eWeaponFire != WEAPON_FIRE_MELEE && m_nAmmoTotal == 0 )
					m_eWeaponState = WEAPONSTATE_OUT_OF_AMMO;
				else
					m_eWeaponState = WEAPONSTATE_READY;
			}

			break;
		}

		case WEAPONSTATE_RELOADING:
		{
			if  ( AEHANDLE_IS_OK(audioEntity) && m_eWeaponType < WEAPONTYPE_LAST_WEAPONTYPE )
			{
				uint32 timePassed = m_nTimer - gReloadSampleTime[m_eWeaponType];
				if ( CTimer::GetPreviousTimeInMilliseconds() < timePassed && CTimer::GetTimeInMilliseconds() >= timePassed )
					DMAudio.PlayOneShot(audioEntity, SOUND_WEAPON_RELOAD, 0.0f);
			}

			if ( CTimer::GetTimeInMilliseconds() > m_nTimer )
			{
				Reload();
				m_eWeaponState = WEAPONSTATE_READY;
			}

			break;
		}
		default: break;
	}
}

void
FireOneInstantHitRound(CVector *source, CVector *target, int32 damage)
{
	ASSERT(source!=nil);
	ASSERT(target!=nil);

	CParticle::AddParticle(PARTICLE_GUNFLASH, *source, CVector(0.0f, 0.0f, 0.0f));

	CPointLights::AddLight(CPointLights::LIGHT_POINT,
		*source, CVector(0.0f, 0.0f, 0.0f), 5.0f,
		1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

	CColPoint point;
	CEntity *victim;
	CWorld::ProcessLineOfSight(*source, *target, point, victim, true, true, true, true, true, true, false);

	CParticle::AddParticle(PARTICLE_HELI_ATTACK, *source, ((*target) - (*source)) * 0.15f);

	if ( victim )
	{
		if ( victim->IsPed() )
		{
			CPed *victimPed = (CPed *)victim;
			if ( !victimPed->DyingOrDead() )
			{
				CVector pos = victimPed->GetPosition();

				CVector2D posOffset((*source).x-pos.x, (*source).y-pos.y);
				int32 localDir = victimPed->GetLocalDirection(posOffset);

				victimPed->ClearAttackByRemovingAnim();

				CAnimBlendAssociation *asoc = CAnimManager::AddAnimation(victimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + localDir));
				ASSERT(asoc!=nil);
				asoc->blendAmount = 0.0f;
				asoc->blendDelta  = 8.0f;

				victimPed->InflictDamage(nil, WEAPONTYPE_UZI, damage, (ePedPieceTypes)point.pieceB, localDir);

				pos.z += 0.8f;

				if ( victimPed->GetIsOnScreen() )
				{
					if ( CGame::nastyGame )
					{
						for ( int32 i = 0; i < 4; i++ )
						{
							CVector dir;
							dir.x = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
							dir.y = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
							dir.z = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);

							CParticle::AddParticle(PARTICLE_BLOOD, pos, dir);
						}
					}
				}
			}
		}
		else if ( victim->IsVehicle() )
			((CVehicle *)victim)->InflictDamage(nil, WEAPONTYPE_UZI, damage);
		//BUG ? no CGlass::WasGlassHitByBullet

		switch ( victim->GetType() )
		{
			case ENTITY_TYPE_BUILDING:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_1, point.point);
				CParticle::AddParticle(PARTICLE_SMOKE, point.point, CVector(0.0f, 0.0f, 0.01f));
				break;
			}
			case ENTITY_TYPE_VEHICLE:
			{
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
				break;
			}
			case ENTITY_TYPE_PED:
			{
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
				((CPed*)victim)->Say(SOUND_PED_BULLET_HIT);
				break;
			}
			case ENTITY_TYPE_OBJECT:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_2, point.point);
				break;
			}
			case ENTITY_TYPE_DUMMY:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_3, point.point);
				break;
			}
			default: break;
		}
	}
	else
	{
		float waterLevel;
		if ( CWaterLevel::GetWaterLevel((*target).x, (*target).y, (*target).z + 10.0f, &waterLevel, false) )
		{
			CParticle::AddParticle(PARTICLE_BOAT_SPLASH, CVector((*target).x, (*target).y, waterLevel), CVector(0.0f, 0.0f, 0.01f));
			PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_WATER, point.point); // no sound(empty)
		}
	}
}

bool
CWeapon::IsTypeMelee(void)
{
	return m_eWeaponType == WEAPONTYPE_UNARMED || m_eWeaponType == WEAPONTYPE_BASEBALLBAT;
}

bool
CWeapon::IsType2Handed(void)
{
	return m_eWeaponType >= WEAPONTYPE_SHOTGUN && m_eWeaponType <= WEAPONTYPE_FLAMETHROWER && m_eWeaponType != WEAPONTYPE_ROCKETLAUNCHER;
}

void
CWeapon::MakePedsJumpAtShot(CPhysical *shooter, CVector *source, CVector *target)
{
	ASSERT(shooter!=nil);
	ASSERT(source!=nil);
	ASSERT(target!=nil);

	float minx = Min(source->x, target->x) - 2.0f;
	float maxx = Max(source->x, target->x) + 2.0f;
	float miny = Min(source->y, target->y) - 2.0f;
	float maxy = Max(source->y, target->y) + 2.0f;
	float minz = Min(source->z, target->z) - 2.0f;
	float maxz = Max(source->z, target->z) + 2.0f;

	for ( int32 i = CPools::GetPedPool()->GetSize() - 1; i >= 0; i--)
	{
		CPed *ped = CPools::GetPedPool()->GetSlot(i);

		if ( ped )
		{
			if (   ped->GetPosition().x > minx && ped->GetPosition().x < maxx
				&& ped->GetPosition().y > miny && ped->GetPosition().y < maxy
				&& ped->GetPosition().z > minz && ped->GetPosition().z < maxz )
			{
				if ( ped != FindPlayerPed() && !((uint8)(ped->m_randomSeed ^ CGeneral::GetRandomNumber()) & 31) )
					ped->SetEvasiveDive(shooter, 1);
			}
		}
	}
}

bool
CWeapon::HitsGround(CEntity *holder, CVector *fireSource, CEntity *aimingTo)
{
	ASSERT(holder!=nil);
	ASSERT(aimingTo!=nil);

	if (!holder->IsPed() || !((CPed*)holder)->m_pSeekTarget)
		return false;

	CWeaponInfo *info = GetInfo();

	CVector adjustedOffset = info->m_vecFireOffset;
	adjustedOffset.z += 0.6f;

	CVector source, target;
	CEntity *foundEnt = nil;
	CColPoint foundCol;

	if (fireSource)
		source = *fireSource;
	else
		source = holder->GetMatrix() * adjustedOffset;

	CEntity *aimEntity = aimingTo ? aimingTo : ((CPed*)holder)->m_pSeekTarget;
	ASSERT(aimEntity!=nil);

	target = aimEntity->GetPosition();
	target.z += 0.6f;

	CWorld::ProcessLineOfSight(source, target, foundCol, foundEnt, true, false, false, false, false, false, false);
	if (foundEnt && foundEnt->IsBuilding()) {
		// That was supposed to be Magnitude, according to leftover code in assembly
		float diff = (foundCol.point.z - source.z);
		if (diff < 0.0f && diff > -3.0f)
			return true;
	}

	return false;
}

void
CWeapon::BlowUpExplosiveThings(CEntity *thing)
{
	if ( thing )
	{
		CObject *object = (CObject*)thing;
		int32 mi = object->GetModelIndex();
		if ( IsExplosiveThingModel(mi) && !object->bHasBeenDamaged )
		{
			object->bHasBeenDamaged = true;

			CExplosion::AddExplosion(object, FindPlayerPed(), EXPLOSION_BARREL, object->GetPosition()+CVector(0.0f,0.0f,0.5f), 100);

			if ( MI_EXPLODINGBARREL == mi )
				object->m_vecMoveSpeed.z += 0.75f;
			else
				object->m_vecMoveSpeed.z += 0.45f;

			object->m_vecMoveSpeed.x += float((CGeneral::GetRandomNumber()&255) - 128) * 0.0002f;
			object->m_vecMoveSpeed.y += float((CGeneral::GetRandomNumber()&255) - 128) * 0.0002f;

			if ( object->GetIsStatic())
			{
				object->SetIsStatic(false);
				object->AddToMovingList();
			}
		}
	}
}

bool
CWeapon::HasWeaponAmmoToBeUsed(void)
{
	switch (m_eWeaponType) {
		case WEAPONTYPE_UNARMED:
		case WEAPONTYPE_BASEBALLBAT:
			return true;
		default:
			return m_nAmmoTotal != 0;
	}
}

bool
CPed::IsPedDoingDriveByShooting(void)
{
	if (FindPlayerPed() == this && GetWeapon()->m_eWeaponType == WEAPONTYPE_UZI) {
		if (TheCamera.Cams[TheCamera.ActiveCam].LookingLeft || TheCamera.Cams[TheCamera.ActiveCam].LookingRight)
			return true;
	}
	return false;
}

bool
CWeapon::ProcessLineOfSight(CVector const &point1, CVector const &point2, CColPoint &point, CEntity *&entity, eWeaponType type, CEntity *shooter, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	return CWorld::ProcessLineOfSight(point1, point2, point, entity, checkBuildings, checkVehicles, checkPeds, checkObjects, checkDummies, ignoreSeeThrough, ignoreSomeObjects);
}

#ifdef COMPATIBLE_SAVES
#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
void
CWeapon::Save(uint8*& buf)
{
	CopyToBuf(buf, m_eWeaponType);
	CopyToBuf(buf, m_eWeaponState);
	CopyToBuf(buf, m_nAmmoInClip);
	CopyToBuf(buf, m_nAmmoTotal);
	CopyToBuf(buf, m_nTimer);
	CopyToBuf(buf, m_bAddRotOffset);
	SkipSaveBuf(buf, 3);
}

void
CWeapon::Load(uint8*& buf)
{
	CopyFromBuf(buf, m_eWeaponType);
	CopyFromBuf(buf, m_eWeaponState);
	CopyFromBuf(buf, m_nAmmoInClip);
	CopyFromBuf(buf, m_nAmmoTotal);
	CopyFromBuf(buf, m_nTimer);
	CopyFromBuf(buf, m_bAddRotOffset);
	SkipSaveBuf(buf, 3);
}

#undef CopyFromBuf
#undef CopyToBuf
#endif
