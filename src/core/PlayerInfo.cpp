#include "common.h"

#include "Automobile.h"
#include "Bridge.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "Cranes.h"
#include "Darkel.h"
#include "Explosion.h"
#include "Fire.h"
#include "Frontend.h"
#include "General.h"
#include "HandlingMgr.h"
#include "Messages.h"
#include "Pad.h"
#include "PathFind.h"
#include "PlayerInfo.h"
#include "PlayerPed.h"
#include "PlayerSkin.h"
#include "ProjectileInfo.h"
#include "Remote.h"
#include "Renderer.h"
#include "Replay.h"
#include "Script.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Streaming.h"
#include "Text.h"
#include "Wanted.h"
#include "WaterLevel.h"
#include "World.h"
#include "ZoneCull.h"
#include "main.h"


void
CPlayerInfo::Clear(void)
{
	m_pPed = nil;
	m_pRemoteVehicle = nil;
	if (m_pVehicleEx) {
		m_pVehicleEx->bUsingSpecialColModel = false;
		m_pVehicleEx = nil;
	}
	m_nVisibleMoney = 0;
	m_nMoney = m_nVisibleMoney;
	m_WBState = WBSTATE_PLAYING;
	m_nWBTime = 0;
	m_nTrafficMultiplier = 0;
	m_fRoadDensity = 1.0f;
	m_bInRemoteMode = false;
	m_bUnusedTaxiThing = false;
	m_nUnusedTaxiTimer = 0;
	m_nCollectedPackages = 0;
	m_nTotalPackages = 3;
	m_nTimeLastHealthLoss = 0;
	m_nTimeLastArmourLoss = 0;
	m_nNextSexFrequencyUpdateTime = 0;
	m_nNextSexMoneyUpdateTime = 0;
	m_nSexFrequency = 0;
	m_pHooker = nil;
	m_nTimeTankShotGun = 0;
	field_248 = 0;
	m_nUpsideDownCounter = 0;
	m_bInfiniteSprint = false;
	m_bFastReload = false;
	m_bGetOutOfJailFree = false;
	m_bGetOutOfHospitalFree = false;
	m_nPreviousTimeRewardedForExplosion = 0;
	m_nExplosionsSinceLastReward = 0;
}

void
CPlayerInfo::Process(void)
{
#ifdef FIX_BUGS
	if (CReplay::IsPlayingBack())
		return;
#endif
	// Unused taxi feature. Gives you a dollar for every second with a passenger. Can be toggled via 0x29A opcode.
	bool startTaxiTimer = true;
	if (m_bUnusedTaxiThing && m_pPed->bInVehicle) {
		CVehicle *veh = m_pPed->m_pMyVehicle;
		if ((veh->GetModelIndex() == MI_TAXI || veh->GetModelIndex() == MI_CABBIE || veh->GetModelIndex() == MI_BORGNINE)
			&& veh->pDriver == m_pPed && veh->m_nNumPassengers != 0) {
			for (uint32 timePassed = CTimer::GetTimeInMilliseconds() - m_nUnusedTaxiTimer; timePassed >= 1000; m_nUnusedTaxiTimer += 1000) {
				timePassed -= 1000;
				++m_nMoney;
			}
			startTaxiTimer = false;
		}
	}
	if (startTaxiTimer)
		m_nUnusedTaxiTimer = CTimer::GetTimeInMilliseconds();

	// The effect that makes money counter does while earning/losing money
	if (m_nVisibleMoney != m_nMoney) {
		int diff = m_nMoney - m_nVisibleMoney;
		int diffAbs = Abs(diff);
		int changeBy;

		if (diffAbs > 100000)
			changeBy = 12345;
		else if (diffAbs > 10000)
			changeBy = 1234;
		else if (diffAbs > 1000)
			changeBy = 123;
		else if (diffAbs > 50)
			changeBy = 42;
		else
			changeBy = 1;

		if (diff < 0)
			m_nVisibleMoney -= changeBy;
		else
			m_nVisibleMoney += changeBy;
	}

	if (!(CTimer::GetFrameCounter() & 15)) {
		CVector2D playerPos = m_pPed->bInVehicle ? m_pPed->m_pMyVehicle->GetPosition() : m_pPed->GetPosition();
		m_fRoadDensity = ThePaths.CalcRoadDensity(playerPos.x, playerPos.y);
	}

	m_fRoadDensity = clamp(m_fRoadDensity, 0.4f, 1.45f);

	// Because vehicle enter/exit use same key binding.
	bool enterOrExitVeh;
	if (m_pPed->bVehExitWillBeInstant && m_pPed->bInVehicle)
		enterOrExitVeh = CPad::GetPad(0)->ExitVehicleJustDown();
	else
		enterOrExitVeh = CPad::GetPad(0)->GetExitVehicle();

	if (enterOrExitVeh && m_pPed->m_nPedState != PED_SNIPER_MODE && m_pPed->m_nPedState != PED_ROCKET_MODE) {
		if (m_pPed->bInVehicle) {
			if (!m_pRemoteVehicle) {
				CEntity *surfaceBelowVeh = m_pPed->m_pMyVehicle->m_pCurGroundEntity;
				if (!surfaceBelowVeh || !CBridge::ThisIsABridgeObjectMovingUp(surfaceBelowVeh->GetModelIndex())) {
					CVehicle *veh = m_pPed->m_pMyVehicle;
					if (!veh->IsBoat() || veh->m_nDoorLock == CARLOCK_LOCKED_PLAYER_INSIDE) {

						// This condition will always return true, else block was probably WIP Miami code.
						if (veh->m_vehType != VEHICLE_TYPE_BIKE || veh->m_nDoorLock == CARLOCK_LOCKED_PLAYER_INSIDE) {
							if (veh->GetStatus() != STATUS_WRECKED && veh->GetStatus() != STATUS_TRAIN_MOVING && veh->m_nDoorLock != CARLOCK_LOCKED_PLAYER_INSIDE) {
								if (veh->m_vecMoveSpeed.Magnitude() < 0.17f && CTimer::GetTimeScale() >= 0.5f && !veh->bIsInWater) {
									m_pPed->SetObjective(OBJECTIVE_LEAVE_CAR, veh);
								}
							}
						} else {
							CVector sth = 0.7f * veh->GetRight() + veh->GetPosition();
							bool found = false;
							float groundZ = CWorld::FindGroundZFor3DCoord(sth.x, sth.y, 2.0f + sth.z, &found);

							if (found)
								sth.z = 1.0f + groundZ;
							m_pPed->SetPedState(PED_IDLE);
							m_pPed->SetMoveState(PEDMOVE_STILL);
							CPed::PedSetOutCarCB(0, m_pPed);
							CAnimManager::BlendAnimation(m_pPed->GetClump(), m_pPed->m_animGroup, ANIM_STD_IDLE, 100.0f);
							CAnimManager::BlendAnimation(m_pPed->GetClump(), ASSOCGRP_STD, ANIM_STD_FALL_LAND, 100.0f);
							m_pPed->SetPosition(sth);
							m_pPed->SetMoveState(PEDMOVE_STILL);
							m_pPed->m_vecMoveSpeed = veh->m_vecMoveSpeed;
						}
					} else {
						// The code in here was under CPed::SetExitBoat in VC, did the same for here.
						m_pPed->SetExitBoat(veh);
						m_pPed->bTryingToReachDryLand = true;
					}
				}
			}
		} else {
			// Enter vehicle
			if (CPad::GetPad(0)->ExitVehicleJustDown()) {
				bool weAreOnBoat = false;
				float lastCloseness = 0.0f;
				CVehicle *carBelow = nil;
				CEntity *surfaceBelow = m_pPed->m_pCurrentPhysSurface;
				if (surfaceBelow && surfaceBelow->IsVehicle()) {
					carBelow = (CVehicle*)surfaceBelow;
					if (carBelow->IsBoat()) {
						weAreOnBoat = true;
						m_pPed->bOnBoat = true;
#ifdef VC_PED_PORTS
						if (carBelow->GetStatus() != STATUS_WRECKED && carBelow->GetUp().z > 0.3f)
#else
						if (carBelow->GetStatus() != STATUS_WRECKED)
#endif
							m_pPed->SetSeekBoatPosition(carBelow);
					}
				}
				// Find closest car
				if (!weAreOnBoat) {
					float minX = m_pPed->GetPosition().x - 10.0f;
					float maxX = 10.0f + m_pPed->GetPosition().x;
					float minY = m_pPed->GetPosition().y - 10.0f;
					float maxY = 10.0f + m_pPed->GetPosition().y;

					int minXSector = CWorld::GetSectorIndexX(minX);
					if (minXSector < 0) minXSector = 0;
					int minYSector = CWorld::GetSectorIndexY(minY);
					if (minYSector < 0) minYSector = 0;
					int maxXSector = CWorld::GetSectorIndexX(maxX);
					if (maxXSector > NUMSECTORS_X - 1) maxXSector = NUMSECTORS_X - 1;
					int maxYSector = CWorld::GetSectorIndexY(maxY);
					if (maxYSector > NUMSECTORS_Y - 1) maxYSector = NUMSECTORS_Y - 1;

					CWorld::AdvanceCurrentScanCode();

					for (int curY = minYSector; curY <= maxYSector; curY++) {
						for (int curX = minXSector; curX <= maxXSector; curX++) {
							CSector *sector = CWorld::GetSector(curX, curY);
							FindClosestCarSectorList(sector->m_lists[ENTITYLIST_VEHICLES], m_pPed,
								minX, minY, maxX, maxY, &lastCloseness, &carBelow);
							FindClosestCarSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], m_pPed,
								minX, minY, maxX, maxY, &lastCloseness, &carBelow);
						}
					}
				}
				// carBelow is now closest vehicle
				if (carBelow && !weAreOnBoat) {
					if (carBelow->GetStatus() == STATUS_TRAIN_NOT_MOVING) {
						m_pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, carBelow);
					} else if (carBelow->IsBoat()) {
						if (!carBelow->pDriver) {
							m_pPed->m_vehDoor = 0;
							m_pPed->SetEnterCar(carBelow, m_pPed->m_vehDoor);
						}
					} else {
						m_pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, carBelow);
					}
				}
			}
		}
	}
	if (m_bInRemoteMode) {
		uint32 timeWithoutRemoteCar = CTimer::GetTimeInMilliseconds() - m_nTimeLostRemoteCar;
		if (CTimer::GetPreviousTimeInMilliseconds() - m_nTimeLostRemoteCar < 1000 && timeWithoutRemoteCar >= 1000 && m_WBState == WBSTATE_PLAYING) {
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(1.0f, FADE_OUT);
		}
		if (timeWithoutRemoteCar > 2000) {
			if (m_WBState == WBSTATE_PLAYING) {
				TheCamera.RestoreWithJumpCut();
				TheCamera.SetFadeColour(0, 0, 0);
				TheCamera.Fade(1.0f, FADE_IN);
				TheCamera.Process();
				CTimer::Stop();
				CCullZones::ForceCullZoneCoors(TheCamera.GetPosition());
				CRenderer::RequestObjectsInFrustum();
				CStreaming::LoadAllRequestedModels(false);
				CTimer::Update();
			}
			m_bInRemoteMode = false;
			CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle = nil;
			if (FindPlayerVehicle()) {
				FindPlayerVehicle()->SetStatus(STATUS_PLAYER);
			}
		}
	}
	if (!(CTimer::GetFrameCounter() & 31)) {
		CVehicle *veh = FindPlayerVehicle();
		if (veh && m_pPed->bInVehicle && veh->GetUp().z < 0.0f
			&& veh->m_vecMoveSpeed.Magnitude() < 0.05f && veh->IsCar() && !veh->bIsInWater) {

			if (veh->GetUp().z < -0.5f) {
				m_nUpsideDownCounter += 2;
			
			} else {
				m_nUpsideDownCounter++;
			}
		} else {
			m_nUpsideDownCounter = 0;
		}

		if (m_nUpsideDownCounter > 6 && veh->bCanBeDamaged) {
			veh->m_fHealth = 249.0f < veh->m_fHealth ? 249.0f : veh->m_fHealth;
			if (veh->IsCar()) {
				CAutomobile* car = (CAutomobile*)veh;
				car->Damage.SetEngineStatus(225);
				car->m_pSetOnFireEntity = nil;
			}
		}
	}
	if (FindPlayerVehicle()) {
		CVehicle *veh = FindPlayerVehicle();
		veh->m_nZoneLevel = LEVEL_IGNORE;
		for (int i = 0; i < ARRAY_SIZE(veh->pPassengers); i++) {
			if (veh->pPassengers[i])
				veh->pPassengers[i]->m_nZoneLevel = LEVEL_GENERIC;
		}
		CStats::DistanceTravelledInVehicle += veh->m_fDistanceTravelled;
	} else {
		CStats::DistanceTravelledOnFoot += FindPlayerPed()->m_fDistanceTravelled;
	}
}

bool
CPlayerInfo::IsPlayerInRemoteMode()
{
	return m_pRemoteVehicle || m_bInRemoteMode;
}

void
CPlayerInfo::SavePlayerInfo(uint8 *buf, uint32 *size)
{
	// Interesting
	*size = sizeof(CPlayerInfo);

#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); buf += sizeof(data);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nMoney);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_WBState);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nWBTime);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nTrafficMultiplier);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_fRoadDensity);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nTotalPackages);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bInfiniteSprint);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bFastReload);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfJailFree);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfHospitalFree);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_aPlayerName);
#undef CopyToBuf
}

void
CPlayerInfo::LoadPlayerInfo(uint8 *buf, uint32 size)
{
#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); buf += sizeof(data);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nMoney);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_WBState);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nWBTime);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nTrafficMultiplier);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_fRoadDensity);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nTotalPackages);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bInfiniteSprint);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bFastReload);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfJailFree);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfHospitalFree);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_aPlayerName)
#undef CopyFromBuf
}

void
CPlayerInfo::FindClosestCarSectorList(CPtrList& carList, CPed* ped, float unk1, float unk2, float unk3, float unk4, float* lastCloseness, CVehicle** closestCarOutput)
{
	for (CPtrNode* node = carList.first; node; node = node->next) {
		CVehicle *car = (CVehicle*)node->item;
		if(car->m_scanCode != CWorld::GetCurrentScanCode()) {
			if (!car->bUsesCollision || !car->IsVehicle())
				continue;

			car->m_scanCode = CWorld::GetCurrentScanCode();
			if (car->GetStatus() != STATUS_WRECKED && car->GetStatus() != STATUS_TRAIN_MOVING
				&& (car->GetUp().z > 0.3f || (car->IsVehicle() && ((CVehicle*)car)->m_vehType == VEHICLE_TYPE_BIKE))) {
				CVector carCentre = car->GetBoundCentre();

				if (Abs(ped->GetPosition().z - carCentre.z) < 2.0f) {
					float dist = (ped->GetPosition() - carCentre).Magnitude2D();
					if (dist <= 10.0f && !CCranes::IsThisCarBeingCarriedByAnyCrane(car)) {
						EvaluateCarPosition(car, ped, dist, lastCloseness, closestCarOutput);
					}
				}
			}
		}
	}
}

// lastCloseness is passed to other calls of this function
void
CPlayerInfo::EvaluateCarPosition(CEntity *carToTest, CPed *player, float carBoundCentrePedDist, float *lastCloseness, CVehicle **closestCarOutput)
{
	// This dist used for determining the angle to face
	CVector2D dist(carToTest->GetPosition() - player->GetPosition());
	float neededTurn = CGeneral::GetATanOfXY(player->GetForward().x, player->GetForward().y) - CGeneral::GetATanOfXY(dist.x, dist.y);
	while (neededTurn >= PI) {
		neededTurn -= 2 * PI;
	}

	while (neededTurn < -PI) {
		neededTurn += 2 * PI;
	}

	// This dist used for evaluating cars' distances, weird...
	// Accounts inverted needed turn (or needed turn in long way) and car dist.
	float closeness = (1.0f - Abs(neededTurn) / TWOPI) * (10.0f - carBoundCentrePedDist);
	if (closeness > *lastCloseness) {
		*lastCloseness = closeness;
		*closestCarOutput = (CVehicle*)carToTest;
	}
}

const CVector &
CPlayerInfo::GetPos()
{
#ifdef FIX_BUGS
	if (!m_pPed)
		return TheCamera.GetPosition();
#endif
	if (m_pPed->InVehicle())
		return m_pPed->m_pMyVehicle->GetPosition();
	return m_pPed->GetPosition();
}

CVector
FindPlayerCoors(void)
{
#ifdef FIX_BUGS
	if (CReplay::IsPlayingBack())
		return TheCamera.GetPosition();
#endif
	CPlayerPed *ped = FindPlayerPed();
	if(ped->InVehicle())
		return ped->m_pMyVehicle->GetPosition();
	else
		return ped->GetPosition();
}

const CVector &
FindPlayerSpeed(void)
{
#ifdef FIX_BUGS
	static CVector vecTmpVector(0.0f, 0.0f, 0.0f);
	if (CReplay::IsPlayingBack())
		return vecTmpVector;
#endif
	CPlayerPed *ped = FindPlayerPed();
	if(ped->InVehicle())
		return ped->m_pMyVehicle->m_vecMoveSpeed;
	else
		return ped->m_vecMoveSpeed;
}

CVehicle *
FindPlayerVehicle(void)
{
	CPlayerPed *ped = FindPlayerPed();
	if(ped && ped->InVehicle()) return ped->m_pMyVehicle;
	return nil;
}

CEntity *
FindPlayerEntity(void)
{
	CPlayerPed *ped = FindPlayerPed();
	if(ped->InVehicle())
		return ped->m_pMyVehicle;
	else
		return ped;
}

CVehicle *
FindPlayerTrain(void)
{
	if(FindPlayerVehicle() && FindPlayerVehicle()->IsTrain())
		return FindPlayerVehicle();
	else
		return nil;
}

CPlayerPed *
FindPlayerPed(void)
{
	return CWorld::Players[CWorld::PlayerInFocus].m_pPed;
}

const CVector &
FindPlayerCentreOfWorld(int32 player)
{
#ifdef FIX_BUGS
	if(CReplay::IsPlayingBack()) return TheCamera.GetPosition();
#endif
	if(CCarCtrl::bCarsGeneratedAroundCamera) return TheCamera.GetPosition();
	if(CWorld::Players[player].m_pRemoteVehicle) return CWorld::Players[player].m_pRemoteVehicle->GetPosition();
	if(FindPlayerVehicle()) return FindPlayerVehicle()->GetPosition();
	return CWorld::Players[player].m_pPed->GetPosition();
}

const CVector &
FindPlayerCentreOfWorld_NoSniperShift(void)
{
#ifdef FIX_BUGS
	if (CReplay::IsPlayingBack()) return TheCamera.GetPosition();
#endif
	if(CCarCtrl::bCarsGeneratedAroundCamera) return TheCamera.GetPosition();
	if(CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle)
		return CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->GetPosition();
	if(FindPlayerVehicle()) return FindPlayerVehicle()->GetPosition();
	return FindPlayerPed()->GetPosition();
}

float
FindPlayerHeading(void)
{
	if(CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle)
		return CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->GetForward().Heading();
	if(FindPlayerVehicle()) return FindPlayerVehicle()->GetForward().Heading();
	return FindPlayerPed()->GetForward().Heading();
}

bool
CPlayerInfo::IsRestartingAfterDeath()
{
	return m_WBState == WBSTATE_WASTED;
}

bool
CPlayerInfo::IsRestartingAfterArrest()
{
	return m_WBState == WBSTATE_BUSTED;
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

void
CPlayerInfo::PlayerFailedCriticalMission()
{
	if (m_WBState != WBSTATE_PLAYING)
		return;
	m_WBState = WBSTATE_FAILED_CRITICAL_MISSION;
	m_nWBTime = CTimer::GetTimeInMilliseconds();
	CDarkel::ResetOnPlayerDeath();
}

void
CPlayerInfo::CancelPlayerEnteringCars(CVehicle *car)
{
	if (!car || car == m_pPed->m_pMyVehicle) {
		if (m_pPed->EnteringCar())
			m_pPed->QuitEnteringCar();
	}
	if (m_pPed->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER || m_pPed->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER)
		m_pPed->ClearObjective();
}

void
CPlayerInfo::MakePlayerSafe(bool toggle)
{
	if (toggle) {
		CTheScripts::ResetCountdownToMakePlayerUnsafe();
		m_pPed->m_pWanted->m_bIgnoredByEveryone = true;
		CWorld::StopAllLawEnforcersInTheirTracks();
		CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_PLAYERINFO);
		CPad::StopPadsShaking();
		m_pPed->bBulletProof = true;
		m_pPed->bFireProof = true;
		m_pPed->bCollisionProof = true;
		m_pPed->bMeleeProof = true;
		m_pPed->bOnlyDamagedByPlayer = true;
		m_pPed->bExplosionProof = true;
		m_pPed->m_bCanBeDamaged = false;
		((CPlayerPed*)m_pPed)->ClearAdrenaline();
		CancelPlayerEnteringCars(nil);
		gFireManager.ExtinguishPoint(GetPos(), 4000.0f);
		CExplosion::RemoveAllExplosionsInArea(GetPos(), 4000.0f);
		CProjectileInfo::RemoveAllProjectiles();
		CWorld::SetAllCarsCanBeDamaged(false);
		CWorld::ExtinguishAllCarFiresInArea(GetPos(), 4000.0f);
		CReplay::DisableReplays();

	} else if (!CGame::playingIntro && !CTheScripts::IsCountdownToMakePlayerUnsafeOn()) {
		m_pPed->m_pWanted->m_bIgnoredByEveryone = false;
		CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_PLAYERINFO);
		m_pPed->bBulletProof = false;
		m_pPed->bFireProof = false;
		m_pPed->bCollisionProof = false;
		m_pPed->bMeleeProof = false;
		m_pPed->bOnlyDamagedByPlayer = false;
		m_pPed->bExplosionProof = false;
		m_pPed->m_bCanBeDamaged = true;
		CWorld::SetAllCarsCanBeDamaged(true);
		CReplay::EnableReplays();
	}
}

void
CPlayerInfo::BlowUpRCBuggy(void)
{
	if (!m_pRemoteVehicle || m_pRemoteVehicle->bRemoveFromWorld)
		return;

	CRemote::TakeRemoteControlledCarFromPlayer();
	m_pRemoteVehicle->BlowUpCar(FindPlayerPed());
}

// There is something unfinished in here... Sadly all IDBs we have have it unfinished.
void
CPlayerInfo::AwardMoneyForExplosion(CVehicle *wreckedCar)
{
	if (CTimer::GetTimeInMilliseconds() - m_nPreviousTimeRewardedForExplosion < 6000)
		++m_nExplosionsSinceLastReward;
	else
		m_nExplosionsSinceLastReward = 1;

	m_nPreviousTimeRewardedForExplosion = CTimer::GetTimeInMilliseconds();
	int award = wreckedCar->pHandling->nMonetaryValue * 0.002f;
	sprintf(gString, "$%d", award);
#ifdef MONEY_MESSAGES
	// This line is a leftover from PS2, I don't know what it was meant to be.
	// CVector sth(TheCamera.GetPosition() * 4.0f);

	CMoneyMessages::RegisterOne(wreckedCar->GetPosition() + CVector(0.0f, 0.0f, 2.0f), gString, 0, 255, 0, 2.0f, 0.5f);
#endif
	CWorld::Players[CWorld::PlayerInFocus].m_nMoney += award;

	for (int i = m_nExplosionsSinceLastReward; i > 1; --i) {
		CGeneral::GetRandomNumber();
		CWorld::Players[CWorld::PlayerInFocus].m_nMoney += award;
	}
}

#ifdef GTA_PC
void
CPlayerInfo::SetPlayerSkin(const char *skin)
{
	strncpy(m_aSkinName, skin, 32);
	LoadPlayerSkin();
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
		m_pSkinTexture = nil;
	}
}
#endif