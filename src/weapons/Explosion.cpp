#include "common.h"

#include "Automobile.h"
#include "Bike.h"
#include "Camera.h"
#include "Coronas.h"
#include "DMAudio.h"
#include "Entity.h"
#include "EventList.h"
#include "Explosion.h"
#include "General.h"
#include "Fire.h"
#include "Pad.h"
#include "Particle.h"
#include "PointLights.h"
#include "Shadows.h"
#include "Timer.h"
#include "Vehicle.h"
#include "WaterLevel.h"
#include "World.h"

CExplosion gaExplosion[NUM_EXPLOSIONS];

// these two were not initialised in original code, I'm really not sure what were they meant to be
RwRGBA colMedExpl = { 0, 0, 0, 0 };
RwRGBA colUpdate = { 0, 0, 0, 0 };

int AudioHandle = AEHANDLE_NONE;

void
CExplosion::Initialise()
{
	debug("Initialising CExplosion...\n");
	for (int i = 0; i < ARRAY_SIZE(gaExplosion); i++) {
		gaExplosion[i].m_ExplosionType = EXPLOSION_GRENADE;
		gaExplosion[i].m_vecPosition = CVector(0.0f, 0.0f, 0.0f);
		gaExplosion[i].m_fRadius = 1.0f;
		gaExplosion[i].m_fPropagationRate = 0.0f;
		gaExplosion[i].m_fZshift = 0.0f;
		gaExplosion[i].m_pCreatorEntity = nil;
		gaExplosion[i].m_pVictimEntity = nil;
		gaExplosion[i].m_fStopTime = 0.0f;
		gaExplosion[i].m_nIteration = 0;
		gaExplosion[i].m_fStartTime = 0.0f;
		gaExplosion[i].m_bIsBoat = false;
	}
	AudioHandle = DMAudio.CreateEntity(AUDIOTYPE_EXPLOSION, (void*)1);
	if (AudioHandle >= 0)
		DMAudio.SetEntityStatus(AudioHandle, true);
	debug("CExplosion ready\n");
}

void
CExplosion::Shutdown()
{
	debug("Shutting down CExplosion...\n");
	if (AudioHandle >= 0) {
		DMAudio.DestroyEntity(AudioHandle);
		AudioHandle = AEHANDLE_NONE;
	}
	debug("CExplosion shut down\n");
}

int8
CExplosion::GetExplosionActiveCounter(uint8 id)
{
	return gaExplosion[id].m_nActiveCounter;
}

void
CExplosion::ResetExplosionActiveCounter(uint8 id)
{
	gaExplosion[id].m_nActiveCounter = 0;
}

uint8
CExplosion::GetExplosionType(uint8 id)
{
	return gaExplosion[id].m_ExplosionType;
}

CVector *
CExplosion::GetExplosionPosition(uint8 id)
{
	return &gaExplosion[id].m_vecPosition;
}

bool
CExplosion::AddExplosion(CEntity *explodingEntity, CEntity *culprit, eExplosionType type, const CVector &pos, uint32 lifetime)
{
	CVector pPosn;
	CVector posGround;

	RwRGBA colorMedium = colMedExpl;
	bool bDontExplode = false;
	const RwRGBA color = { 160, 160, 160, 255 };
	pPosn = pos;
	pPosn.z += 5.0f;
#ifdef FIX_BUGS
	CShadows::AddPermanentShadow(SHADOWTEX_CAR, gpShadowHeliTex, &pPosn, 8.0f, 0.0f, 0.0f, -8.0f, 200, 0, 0, 0, 10.0f, 30000, 1.0f);
#else
	// last two arguments are swapped resulting in no shadow
	CShadows::AddPermanentShadow(SHADOWTEX_CAR, gpShadowHeliTex, &pPosn, 8.0f, 0.0f, 0.0f, -8.0f, 200, 0, 0, 0, 10.0f, 1, 30000.0f);
#endif

	int n = 0;
#ifdef FIX_BUGS
	while (n < ARRAY_SIZE(gaExplosion) && gaExplosion[n].m_nIteration != 0)
#else
	// array overrun is UB
	while (gaExplosion[n].m_nIteration != 0 && n < ARRAY_SIZE(gaExplosion))
#endif
		n++;
	if (n == ARRAY_SIZE(gaExplosion))
		return false;

	CExplosion &explosion = gaExplosion[n];
	explosion.m_ExplosionType = type;
	explosion.m_vecPosition = pos;
	explosion.m_fRadius = 1.0f;
	explosion.m_fZshift = 0.0f;
	explosion.m_pCreatorEntity = culprit;
	if (culprit != nil)
		culprit->RegisterReference(&explosion.m_pCreatorEntity);
	explosion.m_pVictimEntity = explodingEntity;
	if (explodingEntity != nil)
		explodingEntity->RegisterReference(&explosion.m_pVictimEntity);
	explosion.m_nIteration = 1;
	explosion.m_nActiveCounter = 1;
	explosion.m_bIsBoat = false;
	explosion.m_nParticlesExpireTime = lifetime != 0 ? CTimer::GetTimeInMilliseconds() + lifetime : 0;
	switch (type)
	{
	case EXPLOSION_GRENADE:
		explosion.m_fRadius = 9.0f;
		explosion.m_fPower = 300.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 750;
		explosion.m_fPropagationRate = 0.5f;
		posGround = pos;
		posGround.z = CWorld::FindGroundZFor3DCoord(posGround.x, posGround.y, posGround.z + 3.0f, nil);
		CEventList::RegisterEvent(EVENT_EXPLOSION, posGround, 250);
		if (Distance(explosion.m_vecPosition, TheCamera.GetPosition()) < 40.0f)
			CParticle::AddParticle(PARTICLE_EXPLOSION_LFAST, explosion.m_vecPosition, CVector(0.0f, 0.0f, 0.0f), nil, 5.5f, color);
		break;
	case EXPLOSION_MOLOTOV:
	{
		explosion.m_fRadius = 6.0f;
		explosion.m_fPower = 0.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 3000;
		explosion.m_fPropagationRate = 0.5f;
		posGround = pos;
		bool found;
		posGround.z = CWorld::FindGroundZFor3DCoord(posGround.x, posGround.y, posGround.z + 3.0f, &found);
		if (found) {
			float waterLevel;
			if (CWaterLevel::GetWaterLevelNoWaves(posGround.x, posGround.y, posGround.z, &waterLevel)
				&& posGround.z < waterLevel
				&& waterLevel - 6.0f < posGround.z) // some subway/tunnels check?
				bDontExplode = true;
			else
				gFireManager.StartFire(posGround, 1.8f, false);
		}
		else
			bDontExplode = true;
		break;
	}
	case EXPLOSION_ROCKET:
		explosion.m_fRadius = 10.0f;
		explosion.m_fPower = 300.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 750;
		explosion.m_fPropagationRate = 0.5f;
		CEventList::RegisterEvent(EVENT_EXPLOSION, pos, 250);
		if (Distance(explosion.m_vecPosition, TheCamera.GetPosition()) < 40.0f)
			CParticle::AddParticle(PARTICLE_EXPLOSION_LFAST, explosion.m_vecPosition, CVector(0.0f, 0.0f, 0.0f), nil, 5.5f, color);
		break;
	case EXPLOSION_CAR:
	case EXPLOSION_CAR_QUICK:
		explosion.m_fRadius = 9.0f;
		explosion.m_fPower = 300.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 4250;
		explosion.m_fPropagationRate = 0.5f;
		explosion.m_fStartTime = CTimer::GetTimeInMilliseconds();
		if (explosion.m_pVictimEntity != nil) {
			if (explosion.m_pVictimEntity->IsVehicle() && ((CVehicle*)explosion.m_pVictimEntity)->IsBoat())
				explosion.m_bIsBoat = true;
			CEventList::RegisterEvent(EVENT_EXPLOSION, EVENT_ENTITY_VEHICLE, explosion.m_pVictimEntity, nil, 1000);
		} else 
			CEventList::RegisterEvent(EVENT_EXPLOSION, pos, 1000);

		if (explosion.m_pVictimEntity != nil && !explosion.m_bIsBoat) {
			int rn = (CGeneral::GetRandomNumber() & 1) + 2;
			for (int i = 0; i < rn; i++) {
				CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM, explosion.m_pVictimEntity->GetPosition(), CVector(0.0f, 0.0f, 0.0f), nil, 3.5f, colMedExpl);
				CParticle::AddParticle(PARTICLE_EXPLOSION_LFAST, explosion.m_pVictimEntity->GetPosition(), CVector(0.0f, 0.0f, 0.0f), nil, 5.5f, color);
			}
			CVehicle *veh = (CVehicle*)explosion.m_pVictimEntity;
			int32 component = CAR_WING_LR;

			// miami leftover
			if (veh->IsBike())
				component = BIKE_FORKS_REAR;

			if (veh->IsComponentPresent(component)) {
				CVector componentPos;
				veh->GetComponentWorldPosition(component, componentPos);
				rn = (CGeneral::GetRandomNumber() & 1) + 1;
				for (int i = 0; i < rn; i++)
					CParticle::AddJetExplosion(componentPos, 1.4f, 0.0f);
			}
		}
		break;
	case EXPLOSION_HELI:
		explosion.m_fRadius = 6.0f;
		explosion.m_fPower = 300.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 750;
		explosion.m_fPropagationRate = 0.5f;
		explosion.m_fStartTime = CTimer::GetTimeInMilliseconds();
		for (int i = 0; i < 10; i++) {
			CVector randpos;
			uint8 x, y, z;

			x = CGeneral::GetRandomNumber();
			y = CGeneral::GetRandomNumber();
			z = CGeneral::GetRandomNumber();
			randpos = pos + CVector(x - 128, y - 128, z - 128) / 20.0f;

			CParticle::AddParticle(PARTICLE_EXPLOSION_MFAST, randpos, CVector(0.0f, 0.0f, 0.0f), nil, 2.5f, color);

			x = CGeneral::GetRandomNumber();
			y = CGeneral::GetRandomNumber();
			z = CGeneral::GetRandomNumber();
			randpos = pos + CVector(x - 128, y - 128, z - 128) / 20.0f;

			CParticle::AddParticle(PARTICLE_EXPLOSION_LFAST, randpos, CVector(0.0f, 0.0f, 0.0f), nil, 5.0f, color);

			x = CGeneral::GetRandomNumber();
			y = CGeneral::GetRandomNumber();
			z = CGeneral::GetRandomNumber();
			randpos = pos + CVector(x - 128, y - 128, z - 128) / 20.0f;

			CParticle::AddJetExplosion(randpos, 1.4f, 3.0f);
		}
		CEventList::RegisterEvent(EVENT_EXPLOSION, pos, 1000);
		break;
	case EXPLOSION_MINE:
		explosion.m_fRadius = 10.0f;
		explosion.m_fPower = 150.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 750;
		explosion.m_fPropagationRate = 0.5f;
		posGround = pos;
		//posGround.z = 
			CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 4.0f, nil); // BUG? result is unused
		CEventList::RegisterEvent(EVENT_EXPLOSION, posGround, 250);
		break;
	case EXPLOSION_BARREL:
		explosion.m_fRadius = 7.0f;
		explosion.m_fPower = 150.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 750;
		explosion.m_fPropagationRate = 0.5f;
		for (int i = 0; i < 6; i++) {
			CVector randpos;
			uint8 x, y, z;

			x = CGeneral::GetRandomNumber();
			y = CGeneral::GetRandomNumber();
			z = CGeneral::GetRandomNumber();
			randpos = CVector(x - 128, y - 128, z - 128);

			randpos.x /= 50.0f;
			randpos.y /= 50.0f;
			randpos.z /= 25.0f;
			randpos += pos;
			CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM, randpos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, colorMedium);
		}
		posGround = pos;
		//posGround.z = 
			CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 4.0f, nil); // BUG? result is unused
		CEventList::RegisterEvent(EVENT_EXPLOSION, posGround, 250);
		break;
	case EXPLOSION_TANK_GRENADE:
		explosion.m_fRadius = 10.0f;
		explosion.m_fPower = 150.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 750;
		explosion.m_fPropagationRate = 0.5f;
		posGround = pos;
		//posGround.z = 
			CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 4.0f, nil); // BUG? result is unused
		CEventList::RegisterEvent(EVENT_EXPLOSION, posGround, 250);
		break;
	case EXPLOSION_HELI_BOMB:
		explosion.m_fRadius = 8.0f;
		explosion.m_fPower = 50.0f;
		explosion.m_fStopTime = lifetime + CTimer::GetTimeInMilliseconds() + 750;
		explosion.m_fPropagationRate = 0.5f;
		posGround = pos;
		//posGround.z = 
			CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 4.0f, nil); // BUG? result is unused
		CEventList::RegisterEvent(EVENT_EXPLOSION, posGround, 250);
		break;
	}
	if (bDontExplode) {
		explosion.m_nIteration = 0;
		return false;
	}

	if (explosion.m_fPower != 0.0f && explosion.m_nParticlesExpireTime == 0)
		CWorld::TriggerExplosion(pos, explosion.m_fRadius, explosion.m_fPower, culprit, (type == EXPLOSION_ROCKET || type == EXPLOSION_CAR_QUICK || type == EXPLOSION_MINE || type == EXPLOSION_BARREL || type == EXPLOSION_TANK_GRENADE || type == EXPLOSION_HELI_BOMB));

	TheCamera.CamShake(0.6f, pos.x, pos.y, pos.z);
	CPad::GetPad(0)->StartShake_Distance(300, 128, pos.x, pos.y, pos.z);
	return true;
}

void
CExplosion::Update()
{
	RwRGBA color = colUpdate;
	for (int i = 0; i < ARRAY_SIZE(gaExplosion); i++) {
		CExplosion &explosion = gaExplosion[i];
		if (explosion.m_nIteration == 0) continue;

		if (explosion.m_nParticlesExpireTime != 0) {
			if (CTimer::GetTimeInMilliseconds() > explosion.m_nParticlesExpireTime) {
				explosion.m_nParticlesExpireTime = 0;
				if (explosion.m_fPower != 0.0f)
					CWorld::TriggerExplosion(explosion.m_vecPosition, explosion.m_fRadius, explosion.m_fPower, explosion.m_pCreatorEntity, (explosion.m_ExplosionType == EXPLOSION_ROCKET || explosion.m_ExplosionType == EXPLOSION_CAR_QUICK || explosion.m_ExplosionType == EXPLOSION_MINE || explosion.m_ExplosionType == EXPLOSION_BARREL || explosion.m_ExplosionType == EXPLOSION_TANK_GRENADE || explosion.m_ExplosionType == EXPLOSION_HELI_BOMB));
			}
		} else {
			explosion.m_fRadius += explosion.m_fPropagationRate * CTimer::GetTimeStep();
			int32 someTime = explosion.m_fStopTime - CTimer::GetTimeInMilliseconds();
			switch (explosion.m_ExplosionType)
			{
			case EXPLOSION_GRENADE:
			case EXPLOSION_ROCKET:
			case EXPLOSION_HELI:
			case EXPLOSION_MINE:
			case EXPLOSION_BARREL:
				if (CTimer::GetFrameCounter() & 1) {
					CPointLights::AddLight(CPointLights::LIGHT_POINT, explosion.m_vecPosition, CVector(0.0f, 0.0f, 0.0f), 20.0f, 1.0f, 1.0f, 0.5f, CPointLights::FOG_NONE, true);
					CCoronas::RegisterCorona((uintptr)&explosion, 255, 255, 200, 255, explosion.m_vecPosition, 8.0f, 120.0f, gpCoronaTexture[0], CCoronas::TYPE_NORMAL, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
				} else
					CCoronas::RegisterCorona((uintptr)&explosion, 128, 128, 100, 255, explosion.m_vecPosition, 8.0f, 120.0f, gpCoronaTexture[0], CCoronas::TYPE_NORMAL, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
				CCoronas::RegisterCorona((uintptr)&explosion + 1, 30, 30, 25, 255, explosion.m_vecPosition, explosion.m_fRadius, 120.0f, gpCoronaTexture[7], CCoronas::TYPE_STAR, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
				break;
			case EXPLOSION_MOLOTOV:
				CWorld::SetPedsOnFire(explosion.m_vecPosition.x, explosion.m_vecPosition.y, explosion.m_vecPosition.z, 6.0f, explosion.m_pCreatorEntity);
				CWorld::SetCarsOnFire(explosion.m_vecPosition.x, explosion.m_vecPosition.y, explosion.m_vecPosition.z, 6.0f, explosion.m_pCreatorEntity);
				if (explosion.m_nIteration < 10) {
					if (explosion.m_nIteration == 1) {
						CVector point1 = explosion.m_vecPosition;
						point1.z += 5.0f;
						CColPoint colPoint;
						CEntity *pEntity;
						CWorld::ProcessVerticalLine(point1, -1000.0f, colPoint, pEntity, true, false, false, false, true, false, nil);
						explosion.m_fZshift = colPoint.point.z;
					}
					float ff = ((float)explosion.m_nIteration * 0.55f);
					for (int i = 0; i < 5 * ff; i++) {
						float angle = CGeneral::GetRandomNumber() / 256.0f * 6.28f;

						CVector pos = explosion.m_vecPosition;
						pos.x += ff * Sin(angle);
						pos.y += ff * Cos(angle);
						pos.z += 5.0f; // what is the point of this?

						pos.z = explosion.m_fZshift + 0.5f;
						CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM, pos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, color, CGeneral::GetRandomNumberInRange(-3.0f, 3.0f), CGeneral::GetRandomNumberInRange(-180.0f, 180.0f));
					}
				}
				break;
			case EXPLOSION_CAR:
			case EXPLOSION_CAR_QUICK:
				if (someTime >= 3500) {
					if (explosion.m_pVictimEntity != nil && !explosion.m_bIsBoat) {
						if ((CGeneral::GetRandomNumber() & 0xF) == 0) {
							CVehicle *veh = (CVehicle*)explosion.m_pVictimEntity;
							uint8 component = CAR_WING_LR;

							// miami leftover
							if (veh->IsBike())
								component = BIKE_FORKS_REAR;

							if (veh->IsComponentPresent(component)) {
								CVector componentPos;
								veh->GetComponentWorldPosition(component, componentPos);
								CParticle::AddJetExplosion(componentPos, 1.5f, 0.0f);
							}
						}
						if (CTimer::GetTimeInMilliseconds() > explosion.m_fStartTime) {
							explosion.m_fStartTime = CTimer::GetTimeInMilliseconds() + 125 + (CGeneral::GetRandomNumber() & 0x7F);
							CVector pos = explosion.m_pVictimEntity->GetPosition();
							for (int i = 0; i < (CGeneral::GetRandomNumber() & 1) + 1; i++) {
								CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM, pos, CVector(0.0f, 0.0f, 0.0f), nil, 3.5f, color);
								CParticle::AddParticle(PARTICLE_EXPLOSION_LARGE, pos, CVector(0.0f, 0.0f, 0.0f), nil, 5.5f, color);
							}
						}
					}
					if (CTimer::GetFrameCounter() & 1) {
						CPointLights::AddLight(CPointLights::LIGHT_POINT, explosion.m_vecPosition, CVector(0.0f, 0.0f, 0.0f), 15.0f, 1.0f, 0.0f, 0.0f, CPointLights::FOG_NONE, true);
						CCoronas::RegisterCorona((uintptr)&explosion, 200, 100, 0, 255, explosion.m_vecPosition, 6.0f, 80.0f, gpCoronaTexture[0], CCoronas::TYPE_NORMAL, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
					} else
						CCoronas::RegisterCorona((uintptr)&explosion, 128, 0, 0, 255, explosion.m_vecPosition, 8.0f, 80.0f, gpCoronaTexture[0], CCoronas::TYPE_NORMAL, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);

					CCoronas::RegisterCorona((uintptr)&explosion + 1, 30, 15, 0, 255, explosion.m_vecPosition, explosion.m_fRadius, 80.0f, gpCoronaTexture[7], CCoronas::TYPE_STAR, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
				} else if (explosion.m_nIteration & 1) {
					if (explosion.m_pVictimEntity != nil)
						CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, explosion.m_pVictimEntity->GetPosition(), CVector(0.0f, 0.0f, 0.0f), nil, CGeneral::GetRandomNumberInRange(0.5f, 0.8f), color);
					CVector pos = explosion.m_vecPosition;
					pos.z += 1.0f;
					CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, pos, CVector(0.0f, 0.0f, 0.11f), nil, CGeneral::GetRandomNumberInRange(0.5f, 2.0f), color);
				}
				break;
			case EXPLOSION_TANK_GRENADE:
			case EXPLOSION_HELI_BOMB:
				if (explosion.m_nIteration < 5) {
					float ff = ((float)explosion.m_nIteration * 0.65f);
					for (int i = 0; i < 10 * ff; i++) {
						uint8 x = CGeneral::GetRandomNumber(), y = CGeneral::GetRandomNumber(), z = CGeneral::GetRandomNumber();
						CVector pos(x - 128, y - 128, (z % 128) + 1);

						pos.Normalise();
						pos *= ff / 5.0f;
						pos += explosion.m_vecPosition;
						pos.z += 0.5f;
						CParticle::AddParticle(PARTICLE_EXPLOSION_LARGE, pos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, color, CGeneral::GetRandomNumberInRange(-3.0f, 3.0f), CGeneral::GetRandomNumberInRange(-180.0f, 180.0f));
					}
				}
				break;
			}
			if (someTime > 0)
				explosion.m_nIteration++;
			else
				explosion.m_nIteration = 0;
		}
	}
}

bool
CExplosion::TestForExplosionInArea(eExplosionType type, float x1, float x2, float y1, float y2, float z1, float z2)
{
	for (int i = 0; i < ARRAY_SIZE(gaExplosion); i++) {
		if (gaExplosion[i].m_nIteration != 0) {
			if (type == gaExplosion[i].m_ExplosionType) {
				if (gaExplosion[i].m_vecPosition.x >= x1 && gaExplosion[i].m_vecPosition.x <= x2) {
					if (gaExplosion[i].m_vecPosition.y >= y1 && gaExplosion[i].m_vecPosition.y <= y2) {
						if (gaExplosion[i].m_vecPosition.z >= z1 && gaExplosion[i].m_vecPosition.z <= z2)
							return true;
					}
				}
			}
		}
	}
	return false;
}

void
CExplosion::RemoveAllExplosionsInArea(CVector pos, float radius)
{
	for (int i = 0; i < ARRAY_SIZE(gaExplosion); i++) {
		if (gaExplosion[i].m_nIteration != 0) {
			if ((pos - gaExplosion[i].m_vecPosition).MagnitudeSqr() < SQR(radius))
				gaExplosion[i].m_nIteration = 0;
		}
	}
}