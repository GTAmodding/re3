#include "common.h"
#include "main.h"

#include "General.h"
#include "Darkel.h"
#include "Stats.h"
#include "SurfaceTable.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "Camera.h"
#include "VisibilityPlugins.h"
#include "ZoneCull.h"
#include "Particle.h"
#include "Shadows.h"
#include "Coronas.h"
#include "Explosion.h"
#include "Timecycle.h"
#include "TempColModels.h"
#include "World.h"
#include "WaterLevel.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "DMAudio.h"
#include "Object.h"
#include "HandlingMgr.h"
#include "Heli.h"
#ifdef FIX_BUGS
#include "Replay.h"
#endif

enum
{
	HELI_STATUS_HOVER,
	HELI_STATUS_CHASE_PLAYER,
	HELI_STATUS_FLY_AWAY,
	HELI_STATUS_SHOT_DOWN,
	HELI_STATUS_HOVER2,
};

CHeli *CHeli::pHelis[NUM_HELIS];
int16 CHeli::NumRandomHelis;
uint32 CHeli::TestForNewRandomHelisTimer;
int16 CHeli::NumScriptHelis;	// unused
bool CHeli::CatalinaHeliOn;
bool CHeli::CatalinaHasBeenShotDown;
bool CHeli::ScriptHeliOn;

CHeli::CHeli(int32 id, uint8 CreatedBy)
 : CVehicle(CreatedBy)
{
	int i;

	CVehicleModelInfo *mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);
	m_vehType = VEHICLE_TYPE_HELI;
	pHandling = mod_HandlingManager.GetHandlingData((tVehicleType)mi->m_handlingId);
	SetModelIndex(id);
	m_heliStatus = HELI_STATUS_HOVER;
	m_pathState = 0;

	m_fMass = 100000000.0f;
	m_fTurnMass = 100000000.0f;
	m_fAirResistance = 0.9994f;
	m_fElasticity = 0.05f;

	m_nHeliId = 0;
	m_fRotorRotation = 0.0f;
	m_nBulletDamage = 0;
	m_fAngularSpeed = 0.0f;
	m_fRotation = 0.0f;
	m_nSearchLightTimer = CTimer::GetTimeInMilliseconds();
	for(i = 0; i < 6; i++){
		m_aSearchLightHistoryX[i] = 0.0f;
		m_aSearchLightHistoryY[i] = 0.0f;
	}

	for(i = 0; i < 8; i++)
		m_fHeliDustZ[i] = -50.0f;

	m_nPoliceShoutTimer = CTimer::GetTimeInMilliseconds();
	SetStatus(STATUS_HELI);
	m_bTestRight = true;
	m_fTargetOffset = 0.0f;
	m_fSearchLightX = m_fSearchLightY = 0.0f;

	// BUG: not in game but gets initialized to CDCDCDCD in debug
	m_nLastShotTime = 0;
}

void
CHeli::SetModelIndex(uint32 id)
{
	int i;

	CVehicle::SetModelIndex(id);
	for(i = 0; i < NUM_HELI_NODES; i++)
		m_aHeliNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetClump(), m_aHeliNodes);
}

static float CatalinaTargetX[7] = { -478.0, -677.0, -907.0, -1095.0, -1152.0, -1161.0, -1161.0 };
static float CatalinaTargetY[7] = { 227.0, 206.0, 210.0, 242.0, 278.0, 341.0, 341.0 };
static float CatalinaTargetZ[7] = { 77.0, 66.0, 60.0, 53.0, 51.0, 46.0, 30.0 };
static float DamPathX[6] = { -1191.0, -1176.0, -1128.0, -1072.0, -1007.0, -971.0 };
static float DamPathY[6] = { 350.0, 388.0, 429.0, 447.0, 449.0, 416.0 };
static float DamPathZ[6] = { 42.0, 37.0, 28.0, 28.0, 31.0, 33.0 };
static float ShortPathX[4] = { -974.0, -1036.0, -1112.0, -1173.0 };
static float ShortPathY[4] = { 340.0, 312.0, 317.0, 294.0 };
static float ShortPathZ[4] = { 41.0, 38.0, 32.0, 39.0 };
static float LongPathX[7] = { -934.0, -905.0, -906.0, -1063.0, -1204.0, -1233.0, -1207.0 };
static float LongPathY[7] = { 371.0, 362.0, 488.0, 548.0, 451.0, 346.0, 308.0 };
static float LongPathZ[7] = { 57.0, 90.0, 105.0, 100.0, 81.0, 79.0, 70.0 };

static int PathPoint;

void
CHeli::ProcessControl(void)
{
	int i;

	if(gbModelViewer)
		return;

	// Find target
	CVector target(0.0f, 0.0f, 0.0f);
	CVector2D vTargetDist;
	if(m_heliType == HELI_TYPE_CATALINA && m_heliStatus != HELI_STATUS_SHOT_DOWN){
		switch(m_pathState){
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			target.x = CatalinaTargetX[m_pathState];
			target.y = CatalinaTargetY[m_pathState];
			target.z = CatalinaTargetZ[m_pathState];
			if((target - GetPosition()).Magnitude() < 9.0f)
				m_pathState++;
			break;
		case 6:
			target.x = CatalinaTargetX[m_pathState];
			target.y = CatalinaTargetY[m_pathState];
			target.z = CatalinaTargetZ[m_pathState];
			if(GetPosition().z > 31.55f)
				break;
			m_pathState = 7;
			GetMatrix().GetPosition().z = 31.55f;
			m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			break;
		case 7:
			GetMatrix().GetPosition().z = 31.55f;
			target = GetPosition();
			break;


		// Take off
		case 8:
			target.x = GetPosition().x;
			target.y = GetPosition().y;
			target.z = 74.0f;
			if(GetPosition().z < 40.0f)
				break;
			PathPoint = 2;
			m_pathState = 9;
			break;
		// Circle around dam
		case 9:
			target.x = DamPathX[PathPoint];
			target.y = DamPathY[PathPoint];
			target.z = DamPathZ[PathPoint];
			if((target - GetPosition()).Magnitude() < 9.0f){
				PathPoint++;
				if(PathPoint >= 6){
					m_pathState = 10;
					PathPoint = 0;
				}
			}
			break;
		case 10:
			target.x = ShortPathX[PathPoint];
			target.y = ShortPathY[PathPoint];
			target.z = ShortPathZ[PathPoint];
			if((target - GetPosition()).Magnitude() < 9.0f){
				PathPoint++;
				if(PathPoint >= 3){
					m_pathState = 9;
					PathPoint = 1;
				}
			}
			break;
		// how do we get here?
		case 11:
			target.x = LongPathX[PathPoint];
			target.y = LongPathY[PathPoint];
			target.z = LongPathZ[PathPoint];
			if((target - GetPosition()).Magnitude() < 9.0f){
				PathPoint++;
				if(PathPoint >= 7){
					m_pathState = 9;
					PathPoint = 0;
				}
			}
			break;


		// Fly away
		case 12:
			target.x = GetPosition().x;
			target.y = GetPosition().y;
			target.z = 200.0f;
			break;
		}

		vTargetDist = target - GetPosition();
		m_fTargetZ = target.z;
		if(m_pathState == 6){
			GetMatrix().GetPosition().x = GetMatrix().GetPosition().x*0.99f + target.x*0.01f;
			GetMatrix().GetPosition().y = GetMatrix().GetPosition().y*0.99f + target.y*0.01f;
		}
	}else{
		vTargetDist = FindPlayerCoors() - GetPosition();
		m_fTargetZ = FindPlayerCoors().z;

		// Heli flies away to (0, 0)
		if(m_heliStatus == HELI_STATUS_FLY_AWAY && GetPosition().z > 20.0f){
			vTargetDist.x = 0.0f - GetPosition().x;
			vTargetDist.y = 0.0f - GetPosition().y;
		}

		float groundZ;
		switch(m_heliStatus){
		case HELI_STATUS_HOVER:
			groundZ = CWorld::FindGroundZFor3DCoord(GetPosition().x, GetPosition().y, 1000.0f, nil);
			m_fTargetZ = Max(groundZ, m_fTargetZ) + 8.0f;
			break;
		case HELI_STATUS_SHOT_DOWN:
			groundZ = CWorld::FindGroundZFor3DCoord(GetPosition().x, GetPosition().y, 1000.0f, nil);
			m_fTargetZ = Max(groundZ, m_fTargetZ) + 8.0f + m_fTargetOffset;
			break;
		case HELI_STATUS_HOVER2:
			groundZ = CWorld::FindGroundZFor3DCoord(GetPosition().x, GetPosition().y, 1000.0f, nil);
			m_fTargetZ = Max(groundZ, m_fTargetZ) + 8.0f + m_fTargetOffset;
			break;
		default:
			groundZ = CWorld::FindGroundZFor3DCoord(GetPosition().x, GetPosition().y, 1000.0f, nil);
			m_fTargetZ = Max(groundZ, m_fTargetZ) + 12.0f;
			break;
		}

		// Move up if too low
		if(GetPosition().z - 2.0f < groundZ && m_heliStatus != HELI_STATUS_SHOT_DOWN)
			m_vecMoveSpeed.z += CTimer::GetTimeStep()*0.01f;
		m_vecMoveSpeed.z = clamp(m_vecMoveSpeed.z, -0.3f, 0.3f);
	}

	float fTargetDist = vTargetDist.Magnitude();

	switch(m_heliStatus){
	case HELI_STATUS_HOVER:
	case HELI_STATUS_HOVER2:{
		float targetHeight;
		if(m_heliType == HELI_TYPE_CATALINA)
			targetHeight = 8.0f;
		else
			targetHeight = 40.0f - m_nHeliId*10.0f;
		if(fTargetDist > targetHeight)
			m_heliStatus = HELI_STATUS_CHASE_PLAYER;
		}
#ifdef FIX_BUGS
		break;
#endif
	case HELI_STATUS_CHASE_PLAYER:{
		float targetHeight;
		if(m_heliType == HELI_TYPE_CATALINA)
			targetHeight = 4.0f;
		else
			targetHeight = 30.0f - m_nHeliId*7.5f;
		if(fTargetDist < 1.0f ||
		   fTargetDist < targetHeight && CWorld::GetIsLineOfSightClear(GetPosition(), FindPlayerCoors(), true, false, false, false, false, false))
			m_heliStatus = HELI_STATUS_HOVER;
		}
	}

	// Find xy speed
	float speed;
	if(fTargetDist > 100.0f)
		speed = 1.0f;
	else if(fTargetDist > 75.0f)
		speed = 0.7f;
	else
		speed = 0.4f;
	if(m_heliStatus == HELI_STATUS_HOVER || m_heliStatus == HELI_STATUS_HOVER2 || m_heliStatus == HELI_STATUS_SHOT_DOWN)
		speed = 0.0f;

	if(fTargetDist != 0.0f)
		vTargetDist /= fTargetDist;
	else
		vTargetDist.x = 1.0f;
	CVector2D targetSpeed = vTargetDist * speed;

	if(m_heliStatus == HELI_STATUS_HOVER2 || m_heliStatus == HELI_STATUS_SHOT_DOWN){
		bool force = !!((CTimer::GetFrameCounter() + m_randomSeed) & 8);
		if(m_bTestRight){
			if(force || CWorld::TestSphereAgainstWorld(GetPosition() + 4.0f*GetRight(), 2.0f, this, true, false, false, false, false, false) == nil){
				if(m_heliStatus == HELI_STATUS_SHOT_DOWN){
					m_fTargetOffset -= CTimer::GetTimeStep()*0.05f;
					targetSpeed.x -= -vTargetDist.x*0.15f;
					targetSpeed.y -= vTargetDist.y*0.15f;
				}else{
					targetSpeed.x -= -vTargetDist.x*0.05f;
					targetSpeed.y -= vTargetDist.y*0.05f;
				}
			}else{
				m_bTestRight = false;
				if(m_heliStatus == HELI_STATUS_HOVER2)
					m_fTargetOffset += 5.0f;
				else
					m_fTargetOffset -= 5.0f;
			}
		}else{
			if(force || CWorld::TestSphereAgainstWorld(GetPosition() - 4.0f*GetRight(), 2.0f, this, true, false, false, false, false, false) == nil){
				if(m_heliStatus == HELI_STATUS_SHOT_DOWN){
					m_fTargetOffset -= CTimer::GetTimeStep()*0.05f;
					targetSpeed.x += -vTargetDist.x*0.15f;
					targetSpeed.y += vTargetDist.y*0.15f;
				}else{
					targetSpeed.x += -vTargetDist.x*0.05f;
					targetSpeed.y += vTargetDist.y*0.05f;
				}
			}else{
				m_bTestRight = true;
				if(m_heliStatus == HELI_STATUS_HOVER2)
					m_fTargetOffset += 5.0f;
				else
					m_fTargetOffset -= 5.0f;
			}
		}

		if(m_fTargetOffset > 30.0f)
			m_fTargetOffset = 30.0f;

		if(m_heliStatus == HELI_STATUS_SHOT_DOWN && force){
			if(CWorld::TestSphereAgainstWorld(GetPosition() + 1.5f*GetForward(), 2.0f, this, true, false, false, false, false, false) ||
			   CWorld::TestSphereAgainstWorld(GetPosition() - 1.5f*GetForward(), 2.0f, this, true, false, false, false, false, false))
				m_nExplosionTimer = CTimer::GetPreviousTimeInMilliseconds();
		}
	}else
		if(m_fTargetOffset >= 2.0f)
			m_fTargetOffset -= 2.0f;

	if(m_heliType == HELI_TYPE_CATALINA)
		if(m_pathState == 9 || m_pathState == 11 || m_pathState == 10){
			float f = Pow(0.997f, CTimer::GetTimeStep());
			m_vecMoveSpeed.x *= f;
			m_vecMoveSpeed.y *= f;
		}

	CVector2D speedDir = targetSpeed - m_vecMoveSpeed;
	float speedDiff = speedDir.Magnitude();
	if(speedDiff != 0.0f)
		speedDir /= speedDiff;
	else
		speedDir.x = 1.0f;
	float speedInc = CTimer::GetTimeStep()*0.002f;
	if(speedDiff < speedInc){
		m_vecMoveSpeed.x = targetSpeed.x;
		m_vecMoveSpeed.y = targetSpeed.y;
	}else{
		m_vecMoveSpeed.x += speedDir.x*speedInc;
		m_vecMoveSpeed.y += speedDir.y*speedInc;
	}
	GetMatrix().GetPosition().x += m_vecMoveSpeed.x*CTimer::GetTimeStep();
	GetMatrix().GetPosition().y += m_vecMoveSpeed.y*CTimer::GetTimeStep();

	// Find z target
	if(m_heliStatus == HELI_STATUS_FLY_AWAY)
		m_fTargetZ = 1000.0f;
	if((CTimer::GetTimeInMilliseconds() + 800*m_nHeliId) & 0x800)
		m_fTargetZ += 2.0f;
	m_fTargetZ += m_nHeliId*5.0f;

	// Find z speed
	float targetSpeedZ = (m_fTargetZ - GetPosition().z)*0.01f;
	float speedDiffZ = targetSpeedZ - m_vecMoveSpeed.z;
	float speedIncZ = CTimer::GetTimeStep()*0.001f;
	if(m_heliStatus == HELI_STATUS_FLY_AWAY)
		speedIncZ *= 1.5f;
	if(Abs(speedDiffZ) < speedIncZ)
		m_vecMoveSpeed.z = targetSpeedZ;
	else if(speedDiffZ < 0.0f)
		m_vecMoveSpeed.z -= speedIncZ;
	else
		m_vecMoveSpeed.z += speedIncZ*1.5f;
	GetMatrix().GetPosition().z += m_vecMoveSpeed.z*CTimer::GetTimeStep();

	// Find angular speed
	float targetAngularSpeed;
	m_fAngularSpeed *= Pow(0.995f, CTimer::GetTimeStep());
	if(fTargetDist < 8.0f)
		targetAngularSpeed = 0.0f;
	else{
		float rotationDiff = CGeneral::GetATanOfXY(vTargetDist.x, vTargetDist.y) - m_fRotation;
		while(rotationDiff < -3.14f) rotationDiff += 6.28f;
		while(rotationDiff > 3.14f) rotationDiff -= 6.28f;
		if(Abs(rotationDiff) > 0.4f){
			if(rotationDiff < 0.0f)
				targetAngularSpeed = -0.2f;
			else
				targetAngularSpeed = 0.2f;
		}else
			targetAngularSpeed = 0.0f;
	}
	float angularSpeedDiff = targetAngularSpeed - m_fAngularSpeed;
	float angularSpeedInc = CTimer::GetTimeStep()*0.0001f;
	if(Abs(angularSpeedDiff) < angularSpeedInc)
		m_fAngularSpeed = targetAngularSpeed;
	else if(angularSpeedDiff < 0.0f)
		m_fAngularSpeed -= angularSpeedInc;
	else
		m_fAngularSpeed += angularSpeedInc;
	m_fRotation += m_fAngularSpeed * CTimer::GetTimeStep();

	// Set matrix
	CVector up(3.0f*m_vecMoveSpeed.x, 3.0f*m_vecMoveSpeed.y, 1.0f);
	up.Normalise();
	CVector fwd(-Cos(m_fRotation), -Sin(m_fRotation), 0.0f);	// not really forward
	CVector right = CrossProduct(up, fwd);
	fwd = CrossProduct(up, right);
	GetRight() = right;
	GetForward() = fwd;
	GetUp() = up;

	// Search light and shooting
	if(m_heliStatus == HELI_STATUS_FLY_AWAY || m_heliType == HELI_TYPE_CATALINA || CCullZones::PlayerNoRain())
		m_fSearchLightIntensity = 0.0f;
	else {
		// Update search light history once every 1000ms
		int timeDiff = CTimer::GetTimeInMilliseconds() - m_nSearchLightTimer;
		while (timeDiff > 1000) {
			for (i = 5; i > 0; i--) {
				m_aSearchLightHistoryX[i] = m_aSearchLightHistoryX[i - 1];
				m_aSearchLightHistoryY[i] = m_aSearchLightHistoryY[i - 1];
			}
			m_aSearchLightHistoryX[0] = FindPlayerCoors().x + FindPlayerSpeed().x * 50.0f * (m_nHeliId + 2);
			m_aSearchLightHistoryY[0] = FindPlayerCoors().y + FindPlayerSpeed().y * 50.0f * (m_nHeliId + 2);

			timeDiff -= 1000;
			m_nSearchLightTimer += 1000;
		}
		assert(timeDiff <= 1000);
		float f1 = timeDiff / 1000.0f;
		float f2 = 1.0f - f1;
		m_fSearchLightX = m_aSearchLightHistoryX[m_nHeliId + 2] * f2 + m_aSearchLightHistoryX[m_nHeliId + 2 - 1] * f1;
		m_fSearchLightY = m_aSearchLightHistoryY[m_nHeliId + 2] * f2 + m_aSearchLightHistoryY[m_nHeliId + 2 - 1] * f1;

		float searchLightDist = (CVector2D(m_fSearchLightX, m_fSearchLightY) - GetPosition()).Magnitude();
		if (searchLightDist > 60.0f)
			m_fSearchLightIntensity = 0.0f;
		else if (searchLightDist < 40.0f)
			m_fSearchLightIntensity = 1.0f;
		else
			m_fSearchLightIntensity = 1.0f - (40.0f - searchLightDist) / (60.0f-40.0f);

		if (m_fSearchLightIntensity < 0.9f || sq(FindPlayerCoors().x - m_fSearchLightX) + sq(FindPlayerCoors().y - m_fSearchLightY) > sq(7.0f))
			m_nShootTimer = CTimer::GetTimeInMilliseconds();
		else if (CTimer::GetTimeInMilliseconds() > m_nPoliceShoutTimer) {
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_PED_HELI_PLAYER_FOUND, 0.0f);
			m_nPoliceShoutTimer = CTimer::GetTimeInMilliseconds() + 4500 + (CGeneral::GetRandomNumber() & 0xFFF);
		}
#ifdef FIX_BUGS
		if (!CReplay::IsPlayingBack())
#endif
		{
			// Shoot
			int shootTimeout;
			if (m_heliType == HELI_TYPE_RANDOM) {
				switch (FindPlayerPed()->m_pWanted->GetWantedLevel()) {
				case 0:
				case 1:
				case 2: shootTimeout = 999999; break;
				case 3: shootTimeout = 10000; break;
				case 4: shootTimeout = 5000; break;
				case 5: shootTimeout = 3500; break;
				case 6: shootTimeout = 2000; break;
				}
				if (CCullZones::NoPolice())
					shootTimeout /= 2;
			}
			else
				shootTimeout = 1500;

			if (FindPlayerPed()->m_pWanted->IsIgnored())
				m_nShootTimer = CTimer::GetTimeInMilliseconds();
			else {
				// Check if line of sight is clear
				if (CTimer::GetTimeInMilliseconds() > m_nShootTimer + shootTimeout &&
					CTimer::GetPreviousTimeInMilliseconds() <= m_nShootTimer + shootTimeout) {
					if (CWorld::GetIsLineOfSightClear(GetPosition(), FindPlayerCoors(), true, false, false, false, false, false)) {
						if (m_heliStatus == HELI_STATUS_HOVER2)
							m_heliStatus = HELI_STATUS_HOVER;
					}
					else {
						m_nShootTimer = CTimer::GetTimeInMilliseconds();
						if (m_heliStatus == HELI_STATUS_HOVER)
							m_heliStatus = HELI_STATUS_HOVER2;
					}
				}

				// Shoot!
				if (CTimer::GetTimeInMilliseconds() > m_nShootTimer + shootTimeout &&
					CTimer::GetTimeInMilliseconds() > m_nLastShotTime + 200) {
					CVector shotTarget = FindPlayerCoors();
					// some inaccuracy
					shotTarget.x += ((CGeneral::GetRandomNumber() & 0xFF) - 128) / 50.0f;
					shotTarget.y += ((CGeneral::GetRandomNumber() & 0xFF) - 128) / 50.0f;
					CVector direction = FindPlayerCoors() - GetPosition();
					direction.Normalise();
					shotTarget += 3.0f * direction;
					CVector shotSource = GetPosition();
					shotSource += 3.0f * direction;
					FireOneInstantHitRound(&shotSource, &shotTarget, 20);
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
					m_nLastShotTime = CTimer::GetTimeInMilliseconds();
				}
			}
		}
	}

	// Drop Catalina's bombs
	if(m_heliType == HELI_TYPE_CATALINA && m_pathState > 8 && (CTimer::GetTimeInMilliseconds()>>9) != (CTimer::GetPreviousTimeInMilliseconds()>>9)){
		CVector bombPos = GetPosition() - 60.0f*m_vecMoveSpeed;
		if(sq(FindPlayerCoors().x-bombPos.x) + sq(FindPlayerCoors().y-bombPos.y) < sq(35.0f)){
			bool found;
			float groundZ = CWorld::FindGroundZFor3DCoord(bombPos.x, bombPos.y, bombPos.z, &found);
			float waterZ;
			if(!CWaterLevel::GetWaterLevelNoWaves(bombPos.x, bombPos.y, bombPos.z, &waterZ))
				waterZ = 0.0f;
			if(groundZ > waterZ){
				bombPos.z = groundZ + 2.0f;
				CExplosion::AddExplosion(nil, this, EXPLOSION_HELI_BOMB, bombPos, 0);
			}else{
				bombPos.z = waterZ;
				CVector dir;
				for(i = 0; i < 16; i++){
					dir.x = ((CGeneral::GetRandomNumber()&0xFF)-127)*0.001f;
					dir.y = ((CGeneral::GetRandomNumber()&0xFF)-127)*0.001f;
					dir.z = 0.5f;
					CParticle::AddParticle(PARTICLE_BOAT_SPLASH, bombPos, dir, nil, 0.2f);
				}
			}
		}
	}

	RemoveAndAdd();
	bIsInSafePosition = true;
	GetMatrix().UpdateRW();
	UpdateRwFrame();
}

void
CHeli::PreRender(void)
{
	float angle;
	uint8 i;
	CColPoint point;
	CEntity *entity;
	uint8 r, g, b;
	float testLowZ = FindPlayerCoors().z - 10.0f;
	float radius = (GetPosition().z - FindPlayerCoors().z - 10.0f - 1.0f) * 0.3f + 10.0f;
	int frm = CTimer::GetFrameCounter() & 7;

	i = 0;
	for(angle = 0.0f; angle < TWOPI; angle += TWOPI/32){
		CVector pos(radius*Cos(angle), radius*Sin(angle), 0.0f);
		CVector dir = CVector(pos.x, pos.y, 1.0f)*0.01f;
		pos += GetPosition();

		if(CWorld::ProcessVerticalLine(pos, testLowZ, point, entity, true, false, false, false, true, false, nil))
			m_fHeliDustZ[frm] = point.point.z;
		else
			m_fHeliDustZ[frm] = -101.0f;

		switch(point.surfaceB){
		default:
		case SURFACE_TARMAC:
			r = 10;
			g = 10;
			b = 10;
			break;
		case SURFACE_GRASS:
			r = 10;
			g = 6;
			b = 3;
			break;
		case SURFACE_GRAVEL:
			r = 10;
			g = 8;
			b = 7;
			break;
		case SURFACE_MUD_DRY:
			r = 10;
			g = 6;
			b = 3;
			break;
		}
		RwRGBA col = { r, g, b, 32 };
#ifdef FIX_BUGS
		pos.z = m_fHeliDustZ[frm];
#else
		// What the hell is the point of this?
		pos.z = m_fHeliDustZ[(i - (i&3))/4];	// advance every 4 iterations, why not just /4?
#endif
		if(pos.z > -200.0f && GetPosition().z - pos.z < 20.0f)
			CParticle::AddParticle(PARTICLE_HELI_DUST, pos, dir, nil, 0.0f, col);
		i++;
	}
}

void
CHeli::Render(void)
{
	CMatrix mat;
	CVector pos;

	mat.Attach(RwFrameGetMatrix(m_aHeliNodes[HELI_TOPROTOR]));
	pos = mat.GetPosition();
	mat.SetRotateZ(m_fRotorRotation);
	mat.Translate(pos);
	mat.UpdateRW();

	m_fRotorRotation += 3.14f/6.5f;
	if(m_fRotorRotation > 6.28f)
		m_fRotorRotation -= 6.28f;

	mat.Attach(RwFrameGetMatrix(m_aHeliNodes[HELI_BACKROTOR]));
	pos = mat.GetPosition();
	mat.SetRotateX(m_fRotorRotation);
	mat.Translate(pos);
	mat.UpdateRW();

	CEntity::Render();
}

void
CHeli::PreRenderAlways(void)
{
	CVector shadowPos(m_fSearchLightX, m_fSearchLightY, GetPosition().z);
	if(m_fSearchLightIntensity > 0.0f){
		CShadows::StoreShadowToBeRendered(SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &shadowPos,
			6.0f, 0.0f, 0.0f, -6.0f,
			80*m_fSearchLightIntensity, 80*m_fSearchLightIntensity, 80*m_fSearchLightIntensity, 80*m_fSearchLightIntensity,
			50.0f, true, 1.0f);

		CVector front = GetMatrix() * CVector(0.0f, 7.0f, 0.0f);
		CVector toPlayer = FindPlayerCoors() - front;
		toPlayer.Normalise();
		float intensity = m_fSearchLightIntensity*sq(CTimeCycle::GetSpriteBrightness());
		if(DotProduct(toPlayer, TheCamera.GetForward()) < -0.8f)
			CCoronas::RegisterCorona((uintptr)this, 255*intensity, 255*intensity, 255*intensity, 255,
				front, 10.0f, 60.0f, CCoronas::TYPE_STAR,
				CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		else
			CCoronas::RegisterCorona((uintptr)this, 200*intensity, 200*intensity, 200*intensity, 255,
				front, 8.0f, 60.0f, CCoronas::TYPE_STAR,
				CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	CVector back = GetMatrix() * CVector(0.0f, -9.0f, 0.0f);
	if(CTimer::GetTimeInMilliseconds() & 0x100)
		CCoronas::RegisterCorona((uintptr)this + 2, 255, 0, 0, 255,
			back, 1.0f, 60.0f, CCoronas::TYPE_STAR,
			CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	else
		CCoronas::RegisterCorona((uintptr)this + 2, 0, 0, 0, 255,
			back, 1.0f, 60.0f, CCoronas::TYPE_STAR,
			CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
}

RwObject*
GetHeliAtomicObjectCB(RwObject *object, void *data)
{
	RpAtomic *atomic = (RpAtomic*)object;
	assert(RwObjectGetType(object) == rpATOMIC);
	if(RpAtomicGetFlags(atomic) & rpATOMICRENDER)
		*(RpAtomic**)data = atomic;
	return object;
}

CObject*
CHeli::SpawnFlyingComponent(int32 component)
{
	RpAtomic *atomic;
	RwFrame *frame;
	RwMatrix *matrix;
	CObject *obj;

	if(m_aHeliNodes[component] == nil)
		return nil;

	atomic = nil;
	RwFrameForAllObjects(m_aHeliNodes[component], GetHeliAtomicObjectCB, &atomic);
	if(atomic == nil)
		return nil;

	obj = new CObject;
	if(obj == nil)
		return nil;

	obj->SetModelIndexNoCreate(MI_CAR_WHEEL);
	// object needs base model
	obj->RefModelInfo(GetModelIndex());

	// create new atomic
	matrix = RwFrameGetLTM(m_aHeliNodes[component]);
	frame = RwFrameCreate();
	atomic = RpAtomicClone(atomic);
	*RwFrameGetMatrix(frame) = *matrix;
	RpAtomicSetFrame(atomic, frame);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
	obj->AttachToRwObject((RwObject*)atomic);

	// init object
	obj->m_fMass = 10.0f;
	obj->m_fTurnMass = 25.0f;
	obj->m_fAirResistance = 0.99f;
	obj->m_fElasticity = 0.1f;
	obj->m_fBuoyancy = obj->m_fMass*GRAVITY/0.75f;
	obj->ObjectCreatedBy = TEMP_OBJECT;
	obj->SetIsStatic(false);
	obj->bIsPickup = false;

	// life time
	CObject::nNoTempObjects++;
	if(component == HELI_TOPROTOR)
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 1000;
	else
		obj->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + 3000;

	obj->m_vecMoveSpeed = m_vecMoveSpeed;
	if(obj->m_vecMoveSpeed.z > 0.0f)
		obj->m_vecMoveSpeed.z = 0.3f;
	else
		obj->m_vecMoveSpeed.z = 0.0f;

	obj->m_vecTurnSpeed = m_vecTurnSpeed*2.0f;

	if(component == HELI_BACKROTOR)
		obj->m_vecTurnSpeed.x = 0.5f;
	else if(component == HELI_TOPROTOR || component == HELI_TOPKNOT)
		obj->m_vecTurnSpeed.z = 0.5f;
	else
		obj->m_vecTurnSpeed.y = 0.5f;

	obj->bRenderScorched = true;

	CWorld::Add(obj);

	atomic = nil;
	RwFrameForAllObjects(m_aHeliNodes[component], GetHeliAtomicObjectCB, &atomic);
	if(atomic)
		RpAtomicSetFlags(atomic, 0);

	return obj;
}



void
CHeli::InitHelis(void)
{
	int i;

	NumRandomHelis = 0;
	TestForNewRandomHelisTimer = 0;
	NumScriptHelis = 0;
	CatalinaHeliOn = false;
	ScriptHeliOn = false;
	for(i = 0; i < NUM_HELIS; i++)
		pHelis[i] = nil;

#if GTA_VERSION >= GTA3_PS2_160
	((CVehicleModelInfo*)CModelInfo::GetModelInfo(MI_ESCAPE))->SetColModel(&CTempColModels::ms_colModelPed1);
	((CVehicleModelInfo*)CModelInfo::GetModelInfo(MI_CHOPPER))->SetColModel(&CTempColModels::ms_colModelPed1);
#endif
}

CHeli*
CHeli::GenerateHeli(bool catalina)
{
	CHeli *heli;
	CVector heliPos;
	int i;

#if GTA_VERSION < GTA3_PS2_160
	if(catalina)
		((CVehicleModelInfo*)CModelInfo::GetModelInfo(MI_ESCAPE))->SetColModel(&CTempColModels::ms_colModelPed1);
	else
		((CVehicleModelInfo*)CModelInfo::GetModelInfo(MI_CHOPPER))->SetColModel(&CTempColModels::ms_colModelPed1);
#endif

	if(catalina)
		heli = new CHeli(MI_ESCAPE, PERMANENT_VEHICLE);
	else
		heli = new CHeli(MI_CHOPPER, PERMANENT_VEHICLE);

	if(catalina)
		heliPos = CVector(-224.0f, 201.0f, 83.0f);
	else{
		heliPos = FindPlayerCoors();
		float angle = (float)(CGeneral::GetRandomNumber() & 0xFF)/0x100 * 6.28f;
		heliPos.x += 250.0f*Sin(angle);
		heliPos.y += 250.0f*Cos(angle);
		if(heliPos.x < -2000.0f || heliPos.x > 2000.0f || heliPos.y < -2000.0f || heliPos.y > 2000.0f){
			heliPos = FindPlayerCoors();
			heliPos.x -= 250.0f*Sin(angle);
			heliPos.y -= 250.0f*Cos(angle);
		}
		heliPos.z += 50.0f;
	}
	heli->GetMatrix().SetTranslate(heliPos);
	if(catalina)
		heli->GetMatrix().SetRotateZOnly(DEGTORAD(270.0f));	// game actually uses 3.14 here

	heli->SetStatus(STATUS_ABANDONED);
	heli->bIsLocked = true;

	int id = -1;
	bool found = false;
	while(!found){
		id++;
		found = true;
		for(i = 0; i < 4; i++)
			if(pHelis[i] && pHelis[i]->m_nHeliId == id)
				found = false;
	}
	heli->m_nHeliId = id;

	CWorld::Add(heli);

	return heli;
}

void
CHeli::UpdateHelis(void)
{
	int i, j;

	// Spawn new police helis
	int numHelisRequired = 
#ifdef FIX_BUGS
		CReplay::IsPlayingBack() ? 0 :
#endif
		FindPlayerPed()->m_pWanted->NumOfHelisRequired();
	if(CStreaming::HasModelLoaded(MI_CHOPPER) && CTimer::GetTimeInMilliseconds() > TestForNewRandomHelisTimer){
		// Spawn a police heli
		TestForNewRandomHelisTimer = CTimer::GetTimeInMilliseconds() + 15000;
		if(NumRandomHelis < numHelisRequired){
			NumRandomHelis++;
			CHeli *heli = GenerateHeli(false);
			heli->m_heliType = HELI_TYPE_RANDOM;
			if(pHelis[HELI_RANDOM0] == nil)
				pHelis[HELI_RANDOM0] = heli;
			else if(pHelis[HELI_RANDOM1] == nil)
				pHelis[HELI_RANDOM1] = heli;
			else
				assert(0 && "too many helis");
		}
	}

	// Handle script heli
	if(ScriptHeliOn){
		if(CStreaming::HasModelLoaded(MI_CHOPPER) && pHelis[HELI_SCRIPT] == nil){
			pHelis[HELI_SCRIPT] = GenerateHeli(false);
			pHelis[HELI_SCRIPT]->m_heliType = HELI_TYPE_SCRIPT;
		}else
			CStreaming::RequestModel(MI_CHOPPER, 0);
	}else{
		if(pHelis[HELI_SCRIPT])
			pHelis[HELI_SCRIPT]->m_heliStatus = HELI_STATUS_FLY_AWAY;
	}

	// Handle Catalina's heli
	if(CatalinaHeliOn){
		if(CStreaming::HasModelLoaded(MI_ESCAPE) && pHelis[HELI_CATALINA] == nil){
			pHelis[HELI_CATALINA] = GenerateHeli(true);
			pHelis[HELI_CATALINA]->m_heliType = HELI_TYPE_CATALINA;
		}else
			CStreaming::RequestModel(MI_ESCAPE, STREAMFLAGS_DONT_REMOVE);
	}else{
		if(pHelis[HELI_CATALINA])
			pHelis[HELI_CATALINA]->m_heliStatus = HELI_STATUS_FLY_AWAY;
	}

	// Delete helis that we no longer need
	for(i = 0; i < NUM_HELIS; i++)
		if(pHelis[i] && pHelis[i]->m_heliStatus == HELI_STATUS_FLY_AWAY && pHelis[i]->GetPosition().z > 150.0f){
			CWorld::Remove(pHelis[i]);
			delete pHelis[i];
			pHelis[i] = nil;
			if(i != HELI_SCRIPT && i != HELI_CATALINA)
				NumRandomHelis--;
		}

	// Handle explosions
	for(i = 0; i < NUM_HELIS; i++){
		if(pHelis[i] && pHelis[i]->m_heliStatus == HELI_STATUS_SHOT_DOWN && CTimer::GetTimeInMilliseconds() > pHelis[i]->m_nExplosionTimer){
			// Second part of explosion
			static int nFrameGen;
			CRGBA colors[8];

			TheCamera.CamShake(0.7f, pHelis[i]->GetPosition().x, pHelis[i]->GetPosition().y, pHelis[i]->GetPosition().z);

			colors[0] = CRGBA(0, 0, 0, 255);
			colors[1] = CRGBA(224, 230, 238, 255);
			colors[2] = CRGBA(0, 0, 0, 255);
			colors[3] = CRGBA(0, 0, 0, 255);
			colors[4] = CRGBA(66, 162, 252, 255);
			colors[5] = CRGBA(0, 0, 0, 255);
			colors[6] = CRGBA(0, 0, 0, 255);
			colors[7] = CRGBA(0, 0, 0, 255);

			CVector pos = pHelis[i]->GetPosition();
			CVector dir;
			for(j = 0; j < 40; j++){
				dir.x = CGeneral::GetRandomNumberInRange(-2.0f, 2.0f);
				dir.y = CGeneral::GetRandomNumberInRange(-2.0f, 2.0f);
				dir.z = CGeneral::GetRandomNumberInRange(0.0f, 2.0f);
				int rotSpeed = CGeneral::GetRandomNumberInRange(10, 30);
				if(CGeneral::GetRandomNumber() & 1)
					rotSpeed = -rotSpeed;
				int f = ++nFrameGen & 3;
				CParticle::AddParticle(PARTICLE_HELI_DEBRIS, pos, dir,
					nil, CGeneral::GetRandomNumberInRange(0.1f, 1.0f),
					colors[nFrameGen], rotSpeed, 0, f, 0);
			}

			CExplosion::AddExplosion(nil, nil, EXPLOSION_HELI, pos, 0);

			pHelis[i]->SpawnFlyingComponent(HELI_SKID_LEFT);
			pHelis[i]->SpawnFlyingComponent(HELI_SKID_RIGHT);
			pHelis[i]->SpawnFlyingComponent(HELI_TOPROTOR);

			CDarkel::RegisterCarBlownUpByPlayer(pHelis[i]);
			CWorld::Remove(pHelis[i]);
			delete pHelis[i];
			pHelis[i] = nil;
			if(i != HELI_SCRIPT && i != HELI_CATALINA)
				NumRandomHelis--;
			if(i == HELI_CATALINA)
				CatalinaHasBeenShotDown = true;

			CStats::HelisDestroyed++;
			CStats::PeopleKilledByPlayer += 2;
			CStats::PedsKilledOfThisType[PEDTYPE_COP] += 2;
			CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 250;
			pos = CWorld::Players[CWorld::PlayerInFocus].m_pPed->GetPosition();
			CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pWanted->RegisterCrime_Immediately(CRIME_SHOOT_HELI,
				pos, i + 19843, false);

			TestForNewRandomHelisTimer = CTimer::GetTimeInMilliseconds() + 50000;
		}else if(pHelis[i] && pHelis[i]->m_heliStatus == HELI_STATUS_SHOT_DOWN && CTimer::GetTimeInMilliseconds()+7000 > pHelis[i]->m_nExplosionTimer){
			// First part of explosion
			if(CTimer::GetPreviousTimeInMilliseconds()+7000 < pHelis[i]->m_nExplosionTimer){
				pHelis[i]->SpawnFlyingComponent(HELI_BACKROTOR);
				pHelis[i]->SpawnFlyingComponent(HELI_TAIL);
				pHelis[i]->m_fAngularSpeed *= -2.5f;
				pHelis[i]->bRenderScorched = true;

				TheCamera.CamShake(0.4f, pHelis[i]->GetPosition().x, pHelis[i]->GetPosition().y, pHelis[i]->GetPosition().z);

				CVector pos = pHelis[i]->GetPosition() - 2.5f*pHelis[i]->GetForward();
				CExplosion::AddExplosion(nil, nil, EXPLOSION_HELI, pos, 0);
			}else
				pHelis[i]->m_fAngularSpeed *= 1.03f;
		}
	}

	// Find police helis to remove
	for(i = 0; i < 2; i++)
		if(pHelis[i] && pHelis[i]->m_heliStatus != HELI_STATUS_FLY_AWAY){
			if(numHelisRequired > 0)
				numHelisRequired--;
			else
				pHelis[i]->m_heliStatus = HELI_STATUS_FLY_AWAY;
		}

	// Remove all helis if in a tunnel or under water
	if(FindPlayerCoors().z < - 2.0f)
		for(i = 0; i < NUM_HELIS; i++)
			if(pHelis[i] && pHelis[i]->m_heliStatus != HELI_STATUS_SHOT_DOWN)
				pHelis[i]->m_heliStatus = HELI_STATUS_FLY_AWAY;
}

void
CHeli::SpecialHeliPreRender(void)
{
	int i;
	for(i = 0; i < NUM_HELIS; i++)
		if(pHelis[i])
			pHelis[i]->PreRenderAlways();
}

bool
CHeli::TestRocketCollision(CVector *rocketPos)
{
	int i;
	bool hit = false;

	for(i = 0; i < NUM_HELIS; i++){
		if(pHelis[i] && !pHelis[i]->bExplosionProof && (*rocketPos - pHelis[i]->GetPosition()).MagnitudeSqr() < sq(8.0f)){
			pHelis[i]->m_fAngularSpeed = CGeneral::GetRandomTrueFalse() ? 0.05f : -0.05f;
			pHelis[i]->m_heliStatus = HELI_STATUS_SHOT_DOWN;
			pHelis[i]->m_nExplosionTimer = CTimer::GetTimeInMilliseconds() + 10000;
			hit = true;
		}
	}
	return hit;
}

bool
CHeli::TestBulletCollision(CVector *line0, CVector *line1, CVector *bulletPos, int32 damage)
{
	int i;
	bool hit = false;

	for(i = 0; i < NUM_HELIS; i++)
		if(pHelis[i] && !pHelis[i]->bBulletProof && CCollision::DistToLine(line0, line1, &pHelis[i]->GetPosition()) < 5.0f){
			// Find bullet position
			float distToHeli = (pHelis[i]->GetPosition() - *line0).Magnitude();
			CVector line = (*line1 - *line0);
			float lineLength = line.Magnitude();
			*bulletPos = *line0 + line*Max(1.0f, distToHeli-5.0f)/lineLength;

			pHelis[i]->m_nBulletDamage += damage;

			if(pHelis[i]->m_heliType == HELI_CATALINA && pHelis[i]->m_nBulletDamage > 400 ||
			   pHelis[i]->m_heliType != HELI_CATALINA && pHelis[i]->m_nBulletDamage > 700){
				pHelis[i]->m_fAngularSpeed = CGeneral::GetRandomTrueFalse() ? 0.05f : -0.05f;
				pHelis[i]->m_heliStatus = HELI_STATUS_SHOT_DOWN;
				pHelis[i]->m_nExplosionTimer = CTimer::GetTimeInMilliseconds() + 10000;
			}

			hit = true;
		}
	return hit;
}

void CHeli::StartCatalinaFlyBy(void)
{
	CatalinaHeliOn = true;
	CatalinaHasBeenShotDown = false;
}

void
CHeli::RemoveCatalinaHeli(void)
{
	CatalinaHeliOn = false;
	if(pHelis[HELI_CATALINA]){
		CWorld::Remove(pHelis[HELI_CATALINA]);
		delete pHelis[HELI_CATALINA];
		pHelis[HELI_CATALINA] = nil;
	}
}

CHeli *CHeli::FindPointerToCatalinasHeli(void) { return pHelis[HELI_CATALINA]; }
void CHeli::CatalinaTakeOff(void) { pHelis[HELI_CATALINA]->m_pathState = 8; }
void CHeli::MakeCatalinaHeliFlyAway(void) { pHelis[HELI_CATALINA]->m_pathState = 12; }
bool CHeli::HasCatalinaBeenShotDown(void) { return CatalinaHasBeenShotDown; }

void CHeli::ActivateHeli(bool activate) { ScriptHeliOn = activate; }
