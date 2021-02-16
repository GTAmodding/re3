#include "common.h"

#include "Garages.h"
#include "main.h"

#ifdef FIX_BUGS
#include "Boat.h"
#endif
#include "DMAudio.h"
#include "General.h"
#include "Font.h"
#include "HandlingMgr.h"
#include "Hud.h"
#include "Messages.h"
#include "ModelIndices.h"
#include "Pad.h"
#include "Particle.h"
#include "PlayerPed.h"
#include "Replay.h"
#include "Stats.h"
#include "Streaming.h"
#include "Text.h"
#include "Timer.h"
#include "Vehicle.h"
#include "Wanted.h"
#include "World.h"

#define CRUSHER_GARAGE_X1 (1135.5f)
#define CRUSHER_GARAGE_Y1 (57.0f)
#define CRUSHER_GARAGE_Z1 (-1.0f)
#define CRUSHER_GARAGE_X2 (1149.5f)
#define CRUSHER_GARAGE_Y2 (63.7f)
#define CRUSHER_GARAGE_Z2 (3.5f)

#define ROTATED_DOOR_OPEN_SPEED (0.015f)
#define ROTATED_DOOR_CLOSE_SPEED (0.02f)
#define DEFAULT_DOOR_OPEN_SPEED (0.035f)
#define DEFAULT_DOOR_CLOSE_SPEED (0.04f)
#define CRUSHER_CRANE_SPEED (0.005f)

// Prices
#define BOMB_PRICE (1000)
#define RESPRAY_PRICE (1000)

// Distances
#define DISTANCE_TO_CALL_OFF_CHASE (10.0f)
#define DISTANCE_FOR_MRWHOOP_HACK (4.0f)
#define DISTANCE_TO_ACTIVATE_GARAGE (8.0f)
#define DISTANCE_TO_ACTIVATE_KEEPCAR_GARAGE (17.0f)
#define DISTANCE_TO_CLOSE_MISSION_GARAGE (30.0f)
#define DISTANCE_TO_CLOSE_COLLECTSPECIFICCARS_GARAGE (25.0f)
#define DISTANCE_TO_CLOSE_COLLECTCARS_GARAGE (40.0f)
#define DISTANCE_TO_CLOSE_HIDEOUT_GARAGE_ON_FOOT (2.2f)
#define DISTANCE_TO_CLOSE_HIDEOUT_GARAGE_IN_CAR (15.0f)
#define DISTANCE_TO_FORCE_CLOSE_HIDEOUT_GARAGE (70.0f)
#define DISTANCE_TO_OPEN_HIDEOUT_GARAGE_ON_FOOT (1.7f)
#define DISTANCE_TO_OPEN_HIDEOUT_GARAGE_IN_CAR (10.0f)
#define DISTANCE_TO_SHOW_HIDEOUT_MESSAGE (5.0f)

#define DISTANCE_TO_CONSIDER_DOOR_FOR_GARAGE (20.0f)

// Time
#define TIME_TO_RESPRAY (2000)
#define TIME_TO_SETUP_BOMB (2000)
#define TIME_TO_CRUSH_CAR (3000)
#define TIME_TO_PROCESS_KEEPCAR_GARAGE (2000)

// Respray stuff
#define FREE_RESPRAY_HEALTH_THRESHOLD (970.0f)
#define NUM_PARTICLES_IN_RESPRAY (200)
#define RESPRAY_CENTERING_COEFFICIENT (0.75f)

// Bomb stuff
#define KGS_OF_EXPLOSIVES_IN_BOMB (10)

// Collect specific cars stuff
#define REWARD_FOR_FIRST_POLICE_CAR (5000)
#define REWARD_FOR_FIRST_BANK_VAN (5000)
#define MAX_POLICE_CARS_TO_COLLECT (10)
#define MAX_BANK_VANS_TO_COLLECT (10)

// Collect cars stuff
#define MAX_SPEED_TO_SHOW_COLLECTED_MESSAGE (0.03f)
#define IMPORT_REWARD (1000)
#define IMPORT_ALLCARS_REWARD (200000)

// Crusher stuff
#define CRUSHER_VEHICLE_TEST_SPAN (8)
#define CRUSHER_MIN_REWARD (25)
#define CRUSHER_MAX_REWARD (125)
#define CRUSHER_REWARD_COEFFICIENT (1.0f/500000)

// Hideout stuff
#define MAX_STORED_CARS_IN_INDUSTRIAL (1)
#define MAX_STORED_CARS_IN_COMMERCIAL (NUM_GARAGE_STORED_CARS)
#define MAX_STORED_CARS_IN_SUBURBAN (NUM_GARAGE_STORED_CARS)
#define LIMIT_CARS_IN_INDUSTRIAL (1)
#define LIMIT_CARS_IN_COMMERCIAL (2)
#define LIMIT_CARS_IN_SUBURBAN (3)
#define HIDEOUT_DOOR_SPEED_COEFFICIENT (1.7f)
#define TIME_BETWEEN_HIDEOUT_MESSAGES (18000)

// Camera stuff
#define MARGIN_FOR_CAMERA_COLLECTCARS (1.3f)
#define MARGIN_FOR_CAMERA_DEFAULT (4.0f)

const int32 gaCarsToCollectInCraigsGarages[TOTAL_COLLECTCARS_GARAGES][TOTAL_COLLECTCARS_CARS] =
{
	{ MI_SECURICA, MI_MOONBEAM, MI_COACH,    MI_FLATBED,  MI_LINERUN,  MI_TRASH,    MI_PATRIOT,  MI_MRWHOOP,  MI_BLISTA,   MI_MULE,     MI_YANKEE,   MI_BOBCAT,   MI_DODO,     MI_BUS,      MI_RUMPO,    MI_PONY     },
	{ MI_SENTINEL, MI_CHEETAH,  MI_BANSHEE,  MI_IDAHO,    MI_INFERNUS, MI_TAXI,     MI_KURUMA,   MI_STRETCH,  MI_PEREN,    MI_STINGER,  MI_MANANA,   MI_LANDSTAL, MI_STALLION, MI_BFINJECT, MI_CABBIE,   MI_ESPERANT },
	{ MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_LANDSTAL, MI_CHEETAH,  MI_TAXI,     MI_ESPERANT, MI_SENTINEL, MI_IDAHO    }
};

const int32 gaCarsToCollectIn60Seconds[] = { MI_CHEETAH, MI_TAXI, MI_ESPERANT, MI_SENTINEL, MI_IDAHO };

int32 CGarages::BankVansCollected;
bool CGarages::BombsAreFree;
bool CGarages::RespraysAreFree;
int32 CGarages::CarsCollected;
int32 CGarages::CarTypesCollected[TOTAL_COLLECTCARS_GARAGES];
int32 CGarages::CrushedCarId;
uint32 CGarages::LastTimeHelpMessage;
int32 CGarages::MessageNumberInString;
char CGarages::MessageIDString[MESSAGE_LENGTH];
int32 CGarages::MessageNumberInString2;
uint32 CGarages::MessageStartTime;
uint32 CGarages::MessageEndTime;
uint32 CGarages::NumGarages;
bool CGarages::PlayerInGarage;
int32 CGarages::PoliceCarsCollected;
CStoredCar CGarages::aCarsInSafeHouse1[NUM_GARAGE_STORED_CARS];
CStoredCar CGarages::aCarsInSafeHouse2[NUM_GARAGE_STORED_CARS];
CStoredCar CGarages::aCarsInSafeHouse3[NUM_GARAGE_STORED_CARS];
int32 hGarages = AEHANDLE_NONE;
CGarage CGarages::aGarages[NUM_GARAGES];
bool CGarages::bCamShouldBeOutisde;

void CGarages::Init(void)
{
	CrushedCarId = -1;
	NumGarages = 0;
	MessageEndTime = 0;
	MessageStartTime = 0;
	PlayerInGarage = false;
	BombsAreFree = false;
#ifdef FIX_BUGS
	RespraysAreFree = false;
#endif
	CarsCollected = 0;
	BankVansCollected = 0;
	PoliceCarsCollected = 0;
	for (int i = 0; i < TOTAL_COLLECTCARS_GARAGES; i++)
		CarTypesCollected[i] = 0;
	LastTimeHelpMessage = 0;
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++)
		aCarsInSafeHouse1[i].Init();
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++)
		aCarsInSafeHouse2[i].Init();
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++)
		aCarsInSafeHouse3[i].Init();
	hGarages = DMAudio.CreateEntity(AUDIOTYPE_GARAGE, (void*)1);
	if (hGarages >= 0)
		DMAudio.SetEntityStatus(hGarages, true);
	AddOne(
		CVector(CRUSHER_GARAGE_X1, CRUSHER_GARAGE_Y1, CRUSHER_GARAGE_Z1),
		CVector(CRUSHER_GARAGE_X2, CRUSHER_GARAGE_Y2, CRUSHER_GARAGE_Z2),
		GARAGE_CRUSHER, 0);
}

#ifndef PS2
void CGarages::Shutdown(void)
{
	NumGarages = 0;
	if (hGarages < 0)
		return;
	DMAudio.DestroyEntity(hGarages);
	hGarages = AEHANDLE_NONE;
}
#endif

void CGarages::Update(void)
{
	static int GarageToBeTidied = 0;
	if (CReplay::IsPlayingBack())
		return;
	bCamShouldBeOutisde = false;
	TheCamera.pToGarageWeAreIn = nil;
	TheCamera.pToGarageWeAreInForHackAvoidFirstPerson = nil;
	for (int i = 0; i < NUM_GARAGES; i++) {
		if (aGarages[i].IsUsed())
			aGarages[i].Update();
	}
	if ((CTimer::GetFrameCounter() & 0xF) != 0xC)
		return;
	if (++GarageToBeTidied >= NUM_GARAGES)
		GarageToBeTidied = 0;
	if (!aGarages[GarageToBeTidied].IsUsed())
		return;
	if (!aGarages[GarageToBeTidied].IsFar())
		aGarages[GarageToBeTidied].TidyUpGarageClose();
	else
		aGarages[GarageToBeTidied].TidyUpGarage();
}

int16 CGarages::AddOne(CVector p1, CVector p2, uint8 type, int32 targetId)
{
	if (NumGarages >= NUM_GARAGES) {
		assert(0);
		return NumGarages++;
	}
	CGarage* pGarage = &aGarages[NumGarages];
	pGarage->m_fX1 = Min(p1.x, p2.x);
	pGarage->m_fX2 = Max(p1.x, p2.x);
	pGarage->m_fY1 = Min(p1.y, p2.y);
	pGarage->m_fY2 = Max(p1.y, p2.y);
	pGarage->m_fZ1 = Min(p1.z, p2.z);
	pGarage->m_fZ2 = Max(p1.z, p2.z);
	pGarage->m_pDoor1 = nil;
	pGarage->m_pDoor2 = nil;
	pGarage->m_fDoor1Z = p1.z;
	pGarage->m_fDoor2Z = p1.z;
	pGarage->m_eGarageType = type;
	pGarage->m_bRecreateDoorOnNextRefresh = false;
	pGarage->m_bRotatedDoor = false;
	pGarage->m_bCameraFollowsPlayer = false;
	pGarage->RefreshDoorPointers(true);
	if (pGarage->m_pDoor1) {
		pGarage->m_fDoor1Z = pGarage->m_pDoor1->GetPosition().z;
		pGarage->m_fDoor1X = pGarage->m_pDoor1->GetPosition().x;
		pGarage->m_fDoor1Y = pGarage->m_pDoor1->GetPosition().y;
	}
	if (pGarage->m_pDoor2) {
		pGarage->m_fDoor2Z = pGarage->m_pDoor2->GetPosition().z;
		pGarage->m_fDoor2X = pGarage->m_pDoor2->GetPosition().x;
		pGarage->m_fDoor2Y = pGarage->m_pDoor2->GetPosition().y;
	}
	pGarage->m_fDoorHeight = pGarage->m_pDoor1 ? FindDoorHeightForMI(pGarage->m_pDoor1->GetModelIndex()) : 4.0f;
	pGarage->m_fDoorPos = 0.0f;
	pGarage->m_eGarageState = GS_FULLYCLOSED;
	pGarage->m_nTimeToStartAction = 0;
	pGarage->field_2 = false;
	pGarage->m_nTargetModelIndex = targetId;
	pGarage->field_96 = nil;
	pGarage->m_bCollectedCarsState = 0;
	pGarage->m_bDeactivated = false;
	pGarage->m_bResprayHappened = false;
	switch (type) {
	case GARAGE_MISSION:
	case GARAGE_COLLECTORSITEMS:
	case GARAGE_COLLECTSPECIFICCARS:
	case GARAGE_COLLECTCARS_1:
	case GARAGE_COLLECTCARS_2:
	case GARAGE_COLLECTCARS_3:
	case GARAGE_FORCARTOCOMEOUTOF:
	case GARAGE_60SECONDS:
	case GARAGE_MISSION_KEEPCAR:
	case GARAGE_FOR_SCRIPT_TO_OPEN:
	case GARAGE_HIDEOUT_ONE:
	case GARAGE_HIDEOUT_TWO:
	case GARAGE_HIDEOUT_THREE:
	case GARAGE_FOR_SCRIPT_TO_OPEN_AND_CLOSE:
	case GARAGE_KEEPS_OPENING_FOR_SPECIFIC_CAR:
	case GARAGE_MISSION_KEEPCAR_REMAINCLOSED:
		pGarage->m_eGarageState = GS_FULLYCLOSED;
		pGarage->m_fDoorPos = 0.0f;
		break;
	case GARAGE_BOMBSHOP1:
	case GARAGE_BOMBSHOP2:
	case GARAGE_BOMBSHOP3:
	case GARAGE_RESPRAY:
		pGarage->m_eGarageState = GS_OPENED;
		pGarage->m_fDoorPos = pGarage->m_fDoorHeight;
		break;
	case GARAGE_CRUSHER:
		pGarage->m_eGarageState = GS_OPENED;
		pGarage->m_fDoorPos = HALFPI;
		break;
	default:
		assert(false);
	}
	if (type == GARAGE_CRUSHER)
		pGarage->UpdateCrusherAngle();
	else
		pGarage->UpdateDoorsHeight();
	return NumGarages++;
}

void CGarages::ChangeGarageType(int16 garage, uint8 type, int32 mi)
{
	CGarage* pGarage = &aGarages[garage];
	pGarage->m_eGarageType = type;
	pGarage->m_nTargetModelIndex = mi;
	pGarage->m_eGarageState = GS_FULLYCLOSED;
}

void CGarage::Update()
{
	if (m_eGarageType != GARAGE_CRUSHER) {
		switch (m_eGarageState) {
		case GS_FULLYCLOSED:
		case GS_OPENED:
		case GS_CLOSING:
		case GS_OPENING:
		case GS_OPENEDCONTAINSCAR:
		case GS_CLOSEDCONTAINSCAR:
			if (FindPlayerPed() && !m_bCameraFollowsPlayer) {
				CVehicle* pVehicle = FindPlayerVehicle();
				if (IsEntityEntirelyInside3D(FindPlayerPed(), 0.25f)) {
					TheCamera.pToGarageWeAreIn = this;
					CGarages::bCamShouldBeOutisde = true;
				}
				if (pVehicle) {
					if (!IsEntityEntirelyOutside(pVehicle, 0.0f))
						TheCamera.pToGarageWeAreInForHackAvoidFirstPerson = this;
					if (pVehicle->GetModelIndex() == MI_MRWHOOP) {
						if (pVehicle->IsWithinArea(
							m_fX1 - DISTANCE_FOR_MRWHOOP_HACK,
							m_fY1 + DISTANCE_FOR_MRWHOOP_HACK,
							m_fX2 - DISTANCE_FOR_MRWHOOP_HACK,
							m_fY2 + DISTANCE_FOR_MRWHOOP_HACK)) {
							TheCamera.pToGarageWeAreIn = this;
							CGarages::bCamShouldBeOutisde = true;
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
	if (m_bDeactivated && m_eGarageState == GS_FULLYCLOSED)
		return;
	switch (m_eGarageType) {
	case GARAGE_RESPRAY:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (IsStaticPlayerCarEntirelyInside() && !IsAnyOtherCarTouchingGarage(FindPlayerVehicle())) {
				if (CGarages::IsCarSprayable(FindPlayerVehicle())) {
					if (CWorld::Players[CWorld::PlayerInFocus].m_nMoney >= RESPRAY_PRICE || CGarages::RespraysAreFree) {
						m_eGarageState = GS_CLOSING;
						CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
						FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
					}
					else {
						CGarages::TriggerMessage("GA_3", -1, 4000, -1); // No more freebies. $1000 to respray!
						m_eGarageState = GS_OPENEDCONTAINSCAR;
						DMAudio.PlayFrontEndSound(SOUND_GARAGE_NO_MONEY, 1);
					}
				}
				else {
					CGarages::TriggerMessage("GA_1", -1, 4000, -1); // Whoa! I don't touch nothing THAT hot!
					m_eGarageState = GS_OPENEDCONTAINSCAR;
					DMAudio.PlayFrontEndSound(SOUND_GARAGE_BAD_VEHICLE, 1);
				}
			}
			if (FindPlayerVehicle()) {
				if (CalcDistToGarageRectangleSquared(FindPlayerVehicle()->GetPosition().x, FindPlayerVehicle()->GetPosition().y) < SQR(DISTANCE_TO_ACTIVATE_GARAGE))
					CWorld::CallOffChaseForArea(
						m_fX1 - DISTANCE_TO_CALL_OFF_CHASE,
						m_fY1 - DISTANCE_TO_CALL_OFF_CHASE,
						m_fX2 + DISTANCE_TO_CALL_OFF_CHASE,
						m_fY2 + DISTANCE_TO_CALL_OFF_CHASE);
			}
			break;
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				m_nTimeToStartAction = CTimer::GetTimeInMilliseconds() + TIME_TO_RESPRAY;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				CStats::CheckPointReachedSuccessfully();
			}
			UpdateDoorsHeight();
#ifdef FIX_BUGS
			if (FindPlayerVehicle() && FindPlayerVehicle()->IsCar())
#else
			if (FindPlayerVehicle())
#endif
				((CAutomobile*)(FindPlayerVehicle()))->m_fFireBlowUpTimer = 0.0f;
			CWorld::CallOffChaseForArea(
				m_fX1 - DISTANCE_TO_CALL_OFF_CHASE,
				m_fY1 - DISTANCE_TO_CALL_OFF_CHASE,
				m_fX2 + DISTANCE_TO_CALL_OFF_CHASE,
				m_fY2 + DISTANCE_TO_CALL_OFF_CHASE);
			break;
		case GS_FULLYCLOSED:
			if (CTimer::GetTimeInMilliseconds() > m_nTimeToStartAction) {
				m_eGarageState = GS_OPENING;
				DMAudio.PlayFrontEndSound(SOUND_GARAGE_OPENING, 1);
				bool bTakeMoney = false;
				if (FindPlayerPed()->m_pWanted->GetWantedLevel() != 0)
					bTakeMoney = true;
				FindPlayerPed()->m_pWanted->Reset();
				CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
#ifdef FIX_BUGS
				bool bChangedColour = false;
#else
				bool bChangedColour;
#endif
				if (FindPlayerVehicle() && FindPlayerVehicle()->IsCar()) {
					if (FindPlayerVehicle()->m_fHealth < FREE_RESPRAY_HEALTH_THRESHOLD)
						bTakeMoney = true;
					FindPlayerVehicle()->m_fHealth = 1000.0f;
					((CAutomobile*)(FindPlayerVehicle()))->m_fFireBlowUpTimer = 0.0f;
					((CAutomobile*)(FindPlayerVehicle()))->Fix();
					if (FindPlayerVehicle()->GetUp().z < 0.0f) {
						FindPlayerVehicle()->GetUp() = -FindPlayerVehicle()->GetUp();
						FindPlayerVehicle()->GetRight() = -FindPlayerVehicle()->GetRight();
					}
					bChangedColour = false;
					if (!((CAutomobile*)(FindPlayerVehicle()))->bFixedColour) {
						uint8 colour1, colour2;
						uint16 attempt;
						FindPlayerVehicle()->GetModelInfo()->ChooseVehicleColour(colour1, colour2);
						for (attempt = 0; attempt < 10; attempt++) {
							if (colour1 != FindPlayerVehicle()->m_currentColour1 || colour2 != FindPlayerVehicle()->m_currentColour2)
								break;
							FindPlayerVehicle()->GetModelInfo()->ChooseVehicleColour(colour1, colour2);
						}
						bChangedColour = (attempt < 10);
						FindPlayerVehicle()->m_currentColour1 = colour1;
						FindPlayerVehicle()->m_currentColour2 = colour2;
						if (bChangedColour) {
							for (int i = 0; i < NUM_PARTICLES_IN_RESPRAY; i++) {
								CVector pos;
#ifdef FIX_BUGS
								pos.x = CGeneral::GetRandomNumberInRange(m_fX1 + 0.5f, m_fX2 - 0.5f);
								pos.y = CGeneral::GetRandomNumberInRange(m_fY1 + 0.5f, m_fY2 - 0.5f);
								pos.z = CGeneral::GetRandomNumberInRange(m_fDoor1Z - 3.0f, m_fDoor1Z + 1.0f);
#else
								// wtf is this
								pos.x = m_fX1 + 0.5f + (uint8)(CGeneral::GetRandomNumber()) / 256.0f * (m_fX2 - m_fX1 - 1.0f);
								pos.y = m_fY1 + 0.5f + (uint8)(CGeneral::GetRandomNumber()) / 256.0f * (m_fY2 - m_fY1 - 1.0f);
								pos.z = m_fDoor1Z - 3.0f + (uint8)(CGeneral::GetRandomNumber()) / 256.0f * 4.0f;
#endif
								CParticle::AddParticle(PARTICLE_GARAGEPAINT_SPRAY, pos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, CVehicleModelInfo::ms_vehicleColourTable[colour1]);
							}
						}
					}
					CenterCarInGarage(FindPlayerVehicle());
				}
				if (bTakeMoney) {
					if (!CGarages::RespraysAreFree)
						CWorld::Players[CWorld::PlayerInFocus].m_nMoney = Max(0, CWorld::Players[CWorld::PlayerInFocus].m_nMoney - RESPRAY_PRICE);
					CGarages::TriggerMessage("GA_2", -1, 4000, -1); // New engine and paint job. The cops won't recognize you!
				}
				else if (bChangedColour) {
					if (CGeneral::GetRandomTrueFalse())
						CGarages::TriggerMessage("GA_15", -1, 4000, -1); // Hope you like the new color.
					else
						CGarages::TriggerMessage("GA_16", -1, 4000, -1); // Respray is complementary.
				}
				m_bResprayHappened = true;
			}
			CWorld::CallOffChaseForArea(
				m_fX1 - DISTANCE_TO_CALL_OFF_CHASE,
				m_fY1 - DISTANCE_TO_CALL_OFF_CHASE,
				m_fX2 + DISTANCE_TO_CALL_OFF_CHASE,
				m_fY2 + DISTANCE_TO_CALL_OFF_CHASE);
			break;
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENEDCONTAINSCAR;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_OPENEDCONTAINSCAR:
			if (IsPlayerOutsideGarage())
				m_eGarageState = GS_OPENED;
			break;
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_BOMBSHOP1:
	case GARAGE_BOMBSHOP2:
	case GARAGE_BOMBSHOP3:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (IsStaticPlayerCarEntirelyInside() && !IsAnyOtherCarTouchingGarage(FindPlayerVehicle())) {
#ifdef FIX_BUGS // FindPlayerVehicle() can never be NULL here because IsStaticPlayerCarEntirelyInside() is true, and there is no IsCar() check
				if (FindPlayerVehicle()->IsCar() && ((CAutomobile*)FindPlayerVehicle())->m_bombType) {
#else
				if (!FindPlayerVehicle() || ((CAutomobile*)FindPlayerVehicle())->m_bombType) {
#endif
					CGarages::TriggerMessage("GA_5", -1, 4000, -1); //"Your car is already fitted with a bomb"
					m_eGarageState = GS_OPENEDCONTAINSCAR;
					DMAudio.PlayFrontEndSound(SOUND_GARAGE_BOMB_ALREADY_SET, 1);
					break;
				}
				if (!CGarages::BombsAreFree && CWorld::Players[CWorld::PlayerInFocus].m_nMoney < BOMB_PRICE) {
					CGarages::TriggerMessage("GA_4", -1, 4000, -1); // "Car bombs are $1000 each" - weird that the price is hardcoded in message
					m_eGarageState = GS_OPENEDCONTAINSCAR;
					DMAudio.PlayFrontEndSound(SOUND_GARAGE_NO_MONEY, 1);
					break;
				}
				m_eGarageState = GS_CLOSING;
				CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
				}
			break;
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				m_nTimeToStartAction = CTimer::GetTimeInMilliseconds() + TIME_TO_SETUP_BOMB;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (CTimer::GetTimeInMilliseconds() > m_nTimeToStartAction) {
				switch (m_eGarageType) {
				case GARAGE_BOMBSHOP1: DMAudio.PlayFrontEndSound(SOUND_GARAGE_BOMB1_SET, 1); break;
				case GARAGE_BOMBSHOP2: DMAudio.PlayFrontEndSound(SOUND_GARAGE_BOMB2_SET, 1); break;
				case GARAGE_BOMBSHOP3: DMAudio.PlayFrontEndSound(SOUND_GARAGE_BOMB3_SET, 1); break;
				default: break;
				}
				m_eGarageState = GS_OPENING;
				if (!CGarages::BombsAreFree)
					CWorld::Players[CWorld::PlayerInFocus].m_nMoney = Max(0, CWorld::Players[CWorld::PlayerInFocus].m_nMoney - BOMB_PRICE);
				if (FindPlayerVehicle() && FindPlayerVehicle()->IsCar()) {
					((CAutomobile*)(FindPlayerVehicle()))->m_bombType = CGarages::GetBombTypeForGarageType(m_eGarageType);
					((CAutomobile*)(FindPlayerVehicle()))->m_pBombRigger = FindPlayerPed();
					if (m_eGarageType == GARAGE_BOMBSHOP3)
						CGarages::GivePlayerDetonator();
					CStats::KgsOfExplosivesUsed += KGS_OF_EXPLOSIVES_IN_BOMB;
				}
#ifdef DETECT_PAD_INPUT_SWITCH
				int16 Mode = CPad::IsAffectedByController ? CPad::GetPad(0)->Mode : 0;
#else
				int16 Mode = CPad::GetPad(0)->Mode;
#endif
				switch (m_eGarageType) {
				case GARAGE_BOMBSHOP1:
					switch (Mode) {
					case 0:
					case 1:
					case 2:
						CHud::SetHelpMessage(TheText.Get("GA_6"), false); // Arm with ~h~~k~~PED_FIREWEAPON~ button~w~. Bomb will go off when engine is started.
						break;
					case 3:
						CHud::SetHelpMessage(TheText.Get("GA_6B"), false); // Arm with ~h~~k~~PED_FIREWEAPON~ button~w~. Bomb will go off when engine is started.
						break;
					}
					break;
				case GARAGE_BOMBSHOP2:
					switch (Mode) {
					case 0:
					case 1:
					case 2:
						CHud::SetHelpMessage(TheText.Get("GA_7"), false); // Park it, prime it by pressing the ~h~~k~~PED_FIREWEAPON~ button~w~ and LEG IT!
						break;
					case 3:
						CHud::SetHelpMessage(TheText.Get("GA_7B"), false); // Park it, prime it by pressing the ~h~~k~~PED_FIREWEAPON~ button~w~ and LEG IT!
						break;
					}
					break;
				case GARAGE_BOMBSHOP3:
					CHud::SetHelpMessage(TheText.Get("GA_8"), false); // Use the detonator to activate the bomb.
					break;
				default: break;
				}
				CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
			}
			break;
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENEDCONTAINSCAR;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_OPENEDCONTAINSCAR:
			if (IsPlayerOutsideGarage())
				m_eGarageState = GS_OPENED;
			break;
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
			}
		break;
	case GARAGE_MISSION:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (((CVector2D)FindPlayerCoors() - CVector2D(GetGarageCenterX(), GetGarageCenterY())).MagnitudeSqr() > SQR(DISTANCE_TO_CLOSE_MISSION_GARAGE)) {
				if ((CTimer::GetFrameCounter() & 0x1F) == 0 && !IsAnyOtherCarTouchingGarage(nil)) {
					m_eGarageState = GS_CLOSING;
					m_bClosingWithoutTargetCar = true;
				}
			}
			else if (!FindPlayerVehicle() && m_pTarget && IsEntityEntirelyInside3D(m_pTarget, 0.0f) &&
				!IsAnyOtherCarTouchingGarage(m_pTarget) && IsEntityEntirelyOutside(FindPlayerPed(), 2.0f) &&
				!IsAnyOtherCarTouchingGarage(m_pTarget)) {
				CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
				m_eGarageState = GS_CLOSING;
				m_bClosingWithoutTargetCar = false;
			}
			break;
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				if (m_bClosingWithoutTargetCar)
					m_eGarageState = GS_FULLYCLOSED;
				else {
					if (m_pTarget) {
						m_eGarageState = GS_CLOSEDCONTAINSCAR;
						DestroyVehicleAndDriverAndPassengers(m_pTarget);
						m_pTarget = nil;
					}
					else {
						m_eGarageState = GS_FULLYCLOSED;
					}
					CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
					FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
				}
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() == m_pTarget && m_pTarget) {
				if (CalcDistToGarageRectangleSquared(
					FindPlayerVehicle()->GetPosition().x,
					FindPlayerVehicle()->GetPosition().y) < SQR(DISTANCE_TO_ACTIVATE_GARAGE))
					m_eGarageState = GS_OPENING;
			}
			break;
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_COLLECTSPECIFICCARS:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (FindPlayerVehicle() && m_nTargetModelIndex == FindPlayerVehicle()->GetModelIndex()) {
				m_pTarget = FindPlayerVehicle();
				m_pTarget->RegisterReference((CEntity**)&m_pTarget);
			}
			if (!FindPlayerVehicle()) {
				if (m_pTarget && IsEntityEntirelyInside3D(m_pTarget, 0.0f) && !IsAnyOtherCarTouchingGarage(m_pTarget)) {
					if (IsEntityEntirelyOutside(FindPlayerPed(), 2.0f)) {
						CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
						FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
						m_eGarageState = GS_CLOSING;
					}
				}
				else if (Abs(FindPlayerCoors().x - GetGarageCenterX()) > DISTANCE_TO_CLOSE_COLLECTSPECIFICCARS_GARAGE ||
					Abs(FindPlayerCoors().y - GetGarageCenterY()) > DISTANCE_TO_CLOSE_COLLECTSPECIFICCARS_GARAGE) {
					m_eGarageState = GS_CLOSING;
					m_pTarget = nil;
				}
			}
			break;
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				if (m_pTarget) {
					DestroyVehicleAndDriverAndPassengers(m_pTarget);
					m_pTarget = nil;
					CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
					FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
					int16 reward;
					switch (m_nTargetModelIndex) {
					case MI_POLICE:
						reward = REWARD_FOR_FIRST_POLICE_CAR * (MAX_POLICE_CARS_TO_COLLECT - CGarages::PoliceCarsCollected++) / MAX_POLICE_CARS_TO_COLLECT;
						break;
					case MI_SECURICA:
						reward = REWARD_FOR_FIRST_BANK_VAN * (MAX_BANK_VANS_TO_COLLECT - CGarages::BankVansCollected++) / MAX_BANK_VANS_TO_COLLECT;
						break;
#ifdef FIX_BUGS // not possible though
					default:
						reward = 0;
						break;
#endif
					}
					if (reward > 0) {
						CWorld::Players[CWorld::PlayerInFocus].m_nMoney += reward;
						CGarages::TriggerMessage("GA_10", reward, 4000, -1); // Nice one. Here's your $~1~
						DMAudio.PlayFrontEndSound(SOUND_GARAGE_VEHICLE_ACCEPTED, 1);
					}
					else {
						CGarages::TriggerMessage("GA_11", -1, 4000, -1); // We got these wheels already. It's worthless to us!
						DMAudio.PlayFrontEndSound(SOUND_GARAGE_VEHICLE_DECLINED, 1);
					}
				}
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() && m_nTargetModelIndex == FindPlayerVehicle()->GetModelIndex()) {
				if (CalcDistToGarageRectangleSquared(FindPlayerVehicle()->GetPosition().x, FindPlayerVehicle()->GetPosition().y) < SQR(DISTANCE_TO_ACTIVATE_GARAGE))
					m_eGarageState = GS_OPENING;
			}
			break;
		case GS_OPENING:
			if (FindPlayerVehicle() && m_nTargetModelIndex == FindPlayerVehicle()->GetModelIndex()) {
				m_pTarget = FindPlayerVehicle();
				m_pTarget->RegisterReference((CEntity**)&m_pTarget);
			}
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_COLLECTCARS_1:
	case GARAGE_COLLECTCARS_2:
	case GARAGE_COLLECTCARS_3:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (FindPlayerVehicle() && DoesCraigNeedThisCar(FindPlayerVehicle()->GetModelIndex())) {
				m_pTarget = FindPlayerVehicle();
				m_pTarget->RegisterReference((CEntity**)&m_pTarget);
			}
			if (Abs(FindPlayerCoors().x - GetGarageCenterX()) > DISTANCE_TO_CLOSE_COLLECTCARS_GARAGE ||
				Abs(FindPlayerCoors().y - GetGarageCenterY()) > DISTANCE_TO_CLOSE_COLLECTCARS_GARAGE) {
				m_eGarageState = GS_CLOSING;
				m_pTarget = nil;
				break;
			}
			if (m_pTarget && !FindPlayerVehicle() && IsEntityEntirelyInside3D(m_pTarget, 0.0f) &&
				!IsAnyOtherCarTouchingGarage(m_pTarget) && IsEntityEntirelyOutside(FindPlayerPed(), 2.0f)) {
#ifdef FIX_BUGS
				if (!m_pTarget->IsCar() ||
					((CAutomobile*)(m_pTarget))->Damage.GetEngineStatus() <= ENGINE_STATUS_ON_FIRE &&
					((CAutomobile*)(m_pTarget))->m_fFireBlowUpTimer == 0.0f) {
#else
				if (((CAutomobile*)(m_pTarget))->Damage.GetEngineStatus() <= ENGINE_STATUS_ON_FIRE &&
					((CAutomobile*)(m_pTarget))->m_fFireBlowUpTimer == 0.0f) {
#endif
					if (m_pTarget->GetStatus() != STATUS_WRECKED) {
						CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
						FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
						m_eGarageState = GS_CLOSING;
						TheCamera.SetCameraDirectlyBehindForFollowPed_CamOnAString();
					}
				}
			}
			break;
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				if (m_pTarget) {
					MarkThisCarAsCollectedForCraig(m_pTarget->GetModelIndex());
					DestroyVehicleAndDriverAndPassengers(m_pTarget);
					m_pTarget = nil;
					CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
					FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
				}
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() &&
				CalcSmallestDistToGarageDoorSquared(
					FindPlayerVehicle()->GetPosition().x,
					FindPlayerVehicle()->GetPosition().y
					) < SQR(DISTANCE_TO_ACTIVATE_GARAGE)) {
				if (DoesCraigNeedThisCar(FindPlayerVehicle()->GetModelIndex())) {
					if (FindPlayerVehicle()->VehicleCreatedBy == MISSION_VEHICLE)
						CGarages::TriggerMessage("GA_1A", -1, 5000, -1); // Come back when you're not so busy...
					else
						m_eGarageState = GS_OPENING;
				}
				else {
					if (HasCraigCollectedThisCar(FindPlayerVehicle()->GetModelIndex()))
						CGarages::TriggerMessage("GA_20", -1, 5000, -1); // We got more of these than we can shift. Sorry man, no deal.
					else if (FindPlayerSpeed().Magnitude() < MAX_SPEED_TO_SHOW_COLLECTED_MESSAGE)
						CGarages::TriggerMessage("GA_19", -1, 5000, -1); // We're not interested in that model.
				}
			}
			m_pTarget = nil;
			break;
		case GS_OPENING:
			if (FindPlayerVehicle() && DoesCraigNeedThisCar(FindPlayerVehicle()->GetModelIndex())) {
				m_pTarget = FindPlayerVehicle();
				m_pTarget->RegisterReference((CEntity**)&m_pTarget);
			}
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_FORCARTOCOMEOUTOF:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (IsGarageEmpty())
				m_eGarageState = GS_CLOSING;
			break;
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
			}
			if (!IsGarageEmpty())
				m_eGarageState = GS_OPENING;
			break;
		case GS_FULLYCLOSED:
			break;
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_CRUSHER:
		switch (m_eGarageState) {
		case GS_OPENED:
		{
			int i = CPools::GetVehiclePool()->GetSize() * (CTimer::GetFrameCounter() % CRUSHER_VEHICLE_TEST_SPAN) / CRUSHER_VEHICLE_TEST_SPAN;
			int end = CPools::GetVehiclePool()->GetSize() * (CTimer::GetFrameCounter() % CRUSHER_VEHICLE_TEST_SPAN + 1) / CRUSHER_VEHICLE_TEST_SPAN;
			for (; i < end; i++) {
				CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
				if (!pVehicle)
					continue;
				if (pVehicle->IsCar() && IsEntityEntirelyInside3D(pVehicle, 0.0f)) {
					m_eGarageState = GS_CLOSING;
					m_pTarget = pVehicle;
					m_pTarget->RegisterReference((CEntity**)&m_pTarget);
				}
			}
			break;
		}
		case GS_CLOSING:
			if (m_pTarget) {
				m_fDoorPos = Max(0.0f, m_fDoorPos - CRUSHER_CRANE_SPEED * CTimer::GetTimeStep());
				if (m_fDoorPos < TWOPI / 5) {
					m_pTarget->bUsesCollision = false;
					m_pTarget->bAffectedByGravity = false;
					m_pTarget->SetMoveSpeed(0.0f, 0.0f, 0.0f);
				}
				else {
					m_pTarget->SetMoveSpeed(m_pTarget->GetMoveSpeed() * Pow(0.8f, CTimer::GetTimeStep()));
				}
				if (m_fDoorPos == 0.0f) {
					CGarages::CrushedCarId = CPools::GetVehiclePool()->GetIndex(m_pTarget);
					float reward = Min(CRUSHER_MAX_REWARD, CRUSHER_MIN_REWARD + m_pTarget->pHandling->nMonetaryValue * m_pTarget->m_fHealth * CRUSHER_REWARD_COEFFICIENT);
					CWorld::Players[CWorld::PlayerInFocus].m_nMoney += reward;
					DestroyVehicleAndDriverAndPassengers(m_pTarget);
					++CStats::CarsCrushed;
					m_pTarget = nil;
					m_eGarageState = GS_AFTERDROPOFF;
					m_nTimeToStartAction = CTimer::GetTimeInMilliseconds() + TIME_TO_CRUSH_CAR;
					DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				}
			}
			else
				m_eGarageState = GS_OPENING;
			UpdateCrusherAngle();
			break;
		case GS_AFTERDROPOFF:
			if (CTimer::GetTimeInMilliseconds() <= m_nTimeToStartAction) {
				UpdateCrusherShake((myrand() & 0xFF - 128) * 0.0002f, (myrand() & 0xFF - 128) * 0.0002f);
			}
			else {
				UpdateCrusherShake(0.0f, 0.0f);
				m_eGarageState = GS_OPENING;
			}
			break;
		case GS_OPENING:
			m_fDoorPos = Min(HALFPI, m_fDoorPos + CTimer::GetTimeStep() * CRUSHER_CRANE_SPEED);
			if (m_fDoorPos == HALFPI) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateCrusherAngle();
			break;
		//case GS_FULLYCLOSED:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_OPENEDCONTAINSCAR:
		default:
			break;
		}
		if (!FindPlayerVehicle() && (CTimer::GetFrameCounter() & 0x1F) == 0x17 && IsEntityEntirelyInside(FindPlayerPed()))
			FindPlayerPed()->InflictDamage(nil, WEAPONTYPE_RAMMEDBYCAR, 300.0f, PEDPIECE_TORSO, 0);
		break;
	case GARAGE_MISSION_KEEPCAR:
	case GARAGE_MISSION_KEEPCAR_REMAINCLOSED:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (((CVector2D)FindPlayerCoors() - CVector2D(GetGarageCenterX(), GetGarageCenterY())).MagnitudeSqr() > SQR(DISTANCE_TO_CLOSE_MISSION_GARAGE) &&
				!IsAnyOtherCarTouchingGarage(nil)) {
				m_eGarageState = GS_CLOSING;
				m_bClosingWithoutTargetCar = true;
			}
			else if (m_pTarget && m_pTarget == FindPlayerVehicle() && IsStaticPlayerCarEntirelyInside() && !IsAnyCarBlockingDoor()) {
				CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
				m_eGarageState = GS_CLOSING;
				m_bClosingWithoutTargetCar = false;
			}
			break;
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				if (m_bClosingWithoutTargetCar)
					m_eGarageState = GS_FULLYCLOSED;
				else {
					if (m_pTarget) {
						m_eGarageState = GS_CLOSEDCONTAINSCAR;
						m_nTimeToStartAction = CTimer::GetTimeInMilliseconds() + TIME_TO_PROCESS_KEEPCAR_GARAGE;
						m_pTarget = nil;
					}
					else
						m_eGarageState = GS_FULLYCLOSED;
					CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
					FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
				}
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() == m_pTarget && m_pTarget &&
				CalcDistToGarageRectangleSquared(
					FindPlayerVehicle()->GetPosition().x,
					FindPlayerVehicle()->GetPosition().y
					) < SQR(DISTANCE_TO_ACTIVATE_KEEPCAR_GARAGE))
				m_eGarageState = GS_OPENING;
			break;
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_CLOSEDCONTAINSCAR:
			if (m_eGarageType == GARAGE_MISSION_KEEPCAR && CTimer::GetTimeInMilliseconds() > m_nTimeToStartAction)
				m_eGarageState = GS_OPENING;
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_FOR_SCRIPT_TO_OPEN:
		switch (m_eGarageState) {
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENED:
		//case GS_CLOSING:
		//case GS_FULLYCLOSED:
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_FOR_SCRIPT_TO_OPEN_AND_CLOSE:
		switch (m_eGarageState) {
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENED:
		//case GS_FULLYCLOSED:
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_HIDEOUT_ONE:
	case GARAGE_HIDEOUT_TWO:
	case GARAGE_HIDEOUT_THREE:
		switch (m_eGarageState) {
		case GS_OPENED:
		{
			float distance = CalcDistToGarageRectangleSquared(FindPlayerCoors().x, FindPlayerCoors().y);
			// Close car doors either if player is far, or if he is in vehicle and garage is full,
			// or if player is very very far so that we can remove whatever is blocking garage door without him noticing
			if ((distance > SQR(DISTANCE_TO_CLOSE_HIDEOUT_GARAGE_IN_CAR) ||
				!FindPlayerVehicle() && distance > SQR(DISTANCE_TO_CLOSE_HIDEOUT_GARAGE_ON_FOOT)) &&
				!IsAnyCarBlockingDoor())
				m_eGarageState = GS_CLOSING;
			else if (FindPlayerVehicle() &&
				CountCarsWithCenterPointWithinGarage(FindPlayerVehicle()) >=
				CGarages::FindMaxNumStoredCarsForGarage(m_eGarageType)) {
				m_eGarageState = GS_CLOSING;
			}
			else if (distance > SQR(DISTANCE_TO_FORCE_CLOSE_HIDEOUT_GARAGE)) {
				m_eGarageState = GS_CLOSING;
				RemoveCarsBlockingDoorNotInside();
			}
			break;
		}
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - HIDEOUT_DOOR_SPEED_COEFFICIENT * (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (!IsPlayerOutsideGarage())
				m_eGarageState = GS_OPENING;
			else if (m_fDoorPos == 0.0f) {
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				m_eGarageState = GS_FULLYCLOSED;
				switch (m_eGarageType) {
				case GARAGE_HIDEOUT_ONE:   StoreAndRemoveCarsForThisHideout(CGarages::aCarsInSafeHouse1, MAX_STORED_CARS_IN_INDUSTRIAL); break;
				case GARAGE_HIDEOUT_TWO:   StoreAndRemoveCarsForThisHideout(CGarages::aCarsInSafeHouse2, MAX_STORED_CARS_IN_COMMERCIAL); break;
				case GARAGE_HIDEOUT_THREE: StoreAndRemoveCarsForThisHideout(CGarages::aCarsInSafeHouse3, MAX_STORED_CARS_IN_SUBURBAN);   break;
				default: break;
				}
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
		{
			float distance = CalcDistToGarageRectangleSquared(FindPlayerCoors().x, FindPlayerCoors().y);
			if (distance < SQR(DISTANCE_TO_OPEN_HIDEOUT_GARAGE_ON_FOOT) ||
				distance < SQR(DISTANCE_TO_OPEN_HIDEOUT_GARAGE_IN_CAR) && FindPlayerVehicle()) {
				if (FindPlayerVehicle() && CGarages::CountCarsInHideoutGarage(m_eGarageType) >= CGarages::FindMaxNumStoredCarsForGarage(m_eGarageType)) {
					if (m_pDoor1) {
						if (((CVector2D)FindPlayerVehicle()->GetPosition() - (CVector2D)m_pDoor1->GetPosition()).MagnitudeSqr() < SQR(DISTANCE_TO_SHOW_HIDEOUT_MESSAGE) &&
							CTimer::GetTimeInMilliseconds() - CGarages::LastTimeHelpMessage > TIME_BETWEEN_HIDEOUT_MESSAGES) {
							CHud::SetHelpMessage(TheText.Get("GA_21"), false); // You cannot store any more cars in this garage.
							CGarages::LastTimeHelpMessage = CTimer::GetTimeInMilliseconds();
						}
					}
				}
				else {
#ifdef FIX_BUGS
					bool bCreatedAllCars = false;
#else
					bool bCreatedAllCars;
#endif
					switch (m_eGarageType) {
					case GARAGE_HIDEOUT_ONE:   bCreatedAllCars = RestoreCarsForThisHideout(CGarages::aCarsInSafeHouse1); break;
					case GARAGE_HIDEOUT_TWO:   bCreatedAllCars = RestoreCarsForThisHideout(CGarages::aCarsInSafeHouse2); break;
					case GARAGE_HIDEOUT_THREE: bCreatedAllCars = RestoreCarsForThisHideout(CGarages::aCarsInSafeHouse3); break;
					default: break;
					}
					if (bCreatedAllCars)
						m_eGarageState = GS_OPENING;
				}
			}
			break;
		}
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + HIDEOUT_DOOR_SPEED_COEFFICIENT * (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_KEEPS_OPENING_FOR_SPECIFIC_CAR:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (((CVector2D)FindPlayerCoors() - CVector2D(GetGarageCenterX(), GetGarageCenterY())).MagnitudeSqr() > SQR(DISTANCE_TO_CLOSE_MISSION_GARAGE)) {
				if (m_pTarget && IsEntityEntirelyOutside(m_pTarget, 0.0f) && !IsAnyOtherCarTouchingGarage(nil)) {
					m_eGarageState = GS_CLOSING;
					m_bClosingWithoutTargetCar = true;
				}
			}
			break;
		case GS_CLOSING:
			m_fDoorPos = Max(0.0f, m_fDoorPos - (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() == m_pTarget && m_pTarget &&
				CalcDistToGarageRectangleSquared(
					FindPlayerVehicle()->GetPosition().x,
					FindPlayerVehicle()->GetPosition().y
					) < SQR(DISTANCE_TO_ACTIVATE_GARAGE))
				m_eGarageState = GS_OPENING;
			break;
		case GS_OPENING:
			m_fDoorPos = Min(m_fDoorHeight, m_fDoorPos + (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED) * CTimer::GetTimeStep());
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio.PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
		//case GARAGE_COLLECTORSITEMS:
		//case GARAGE_60SECONDS:
	default:
		break;
	}
}

bool CGarage::IsStaticPlayerCarEntirelyInside()
{
	if (!FindPlayerVehicle())
		return false;
	if (!FindPlayerVehicle()->IsCar())
		return false;
	if (FindPlayerPed()->GetPedState() != PED_DRIVING)
		return false;
	if (FindPlayerPed()->m_objective == OBJECTIVE_LEAVE_CAR)
		return false;
	CVehicle* pVehicle = FindPlayerVehicle();
	if (pVehicle->GetPosition().x < m_fX1 || pVehicle->GetPosition().x > m_fX2 ||
		pVehicle->GetPosition().y < m_fY1 || pVehicle->GetPosition().y > m_fY2)
		return false;
	if (Abs(pVehicle->GetSpeed().x) > 0.01f ||
		Abs(pVehicle->GetSpeed().y) > 0.01f ||
		Abs(pVehicle->GetSpeed().z) > 0.01f)
		return false;
	if (pVehicle->GetSpeed().MagnitudeSqr() > SQR(0.01f))
		return false;
	return IsEntityEntirelyInside3D(pVehicle, 0.0f);
}

bool CGarage::IsEntityEntirelyInside(CEntity * pEntity)
{
	if (pEntity->GetPosition().x < m_fX1 || pEntity->GetPosition().x > m_fX2 ||
		pEntity->GetPosition().y < m_fY1 || pEntity->GetPosition().y > m_fY2)
		return false;
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * pColModel->spheres[i].center;
		float radius = pColModel->spheres[i].radius;
		if (pos.x - radius < m_fX1 || pos.x + radius > m_fX2 ||
			pos.y - radius < m_fY1 || pos.y + radius > m_fY2)
			return false;
	}
	return true;
}

bool CGarage::IsEntityEntirelyInside3D(CEntity * pEntity, float fMargin)
{
	if (pEntity->GetPosition().x < m_fX1 - fMargin || pEntity->GetPosition().x > m_fX2 + fMargin ||
		pEntity->GetPosition().y < m_fY1 - fMargin || pEntity->GetPosition().y > m_fY2 + fMargin ||
		pEntity->GetPosition().z < m_fZ1 - fMargin || pEntity->GetPosition().z > m_fZ2 + fMargin)
		return false;
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * pColModel->spheres[i].center;
		float radius = pColModel->spheres[i].radius;
		if (pos.x + radius < m_fX1 - fMargin || pos.x - radius > m_fX2 + fMargin ||
			pos.y + radius < m_fY1 - fMargin || pos.y - radius > m_fY2 + fMargin ||
			pos.z + radius < m_fZ1 - fMargin || pos.z - radius > m_fZ2 + fMargin)
			return false;
	}
	return true;
}

bool CGarage::IsEntityEntirelyOutside(CEntity * pEntity, float fMargin)
{
	if (pEntity->GetPosition().x > m_fX1 - fMargin && pEntity->GetPosition().x < m_fX2 + fMargin &&
		pEntity->GetPosition().y > m_fY1 - fMargin && pEntity->GetPosition().y < m_fY2 + fMargin)
		return false;
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * pColModel->spheres[i].center;
		float radius = pColModel->spheres[i].radius;
		if (pos.x + radius > m_fX1 - fMargin && pos.x - radius < m_fX2 + fMargin &&
			pos.y + radius > m_fY1 - fMargin && pos.y - radius < m_fY2 + fMargin)
			return false;
	}
	return true;
}

bool CGarage::IsGarageEmpty()
{
	int16 num;
	CWorld::FindObjectsIntersectingCube(CVector(m_fX1, m_fY1, m_fZ1), CVector(m_fX2, m_fY2, m_fZ2), &num, 2, nil, false, true, true, false, false);
	return num == 0;
}

bool CGarage::IsPlayerOutsideGarage()
{
	if (FindPlayerVehicle())
		return IsEntityEntirelyOutside(FindPlayerVehicle(), 0.0f);
	return IsEntityEntirelyOutside(FindPlayerPed(), 0.0f);
}

bool CGarage::IsEntityTouching3D(CEntity * pEntity)
{
	float radius = pEntity->GetBoundRadius();
	if (m_fX1 - radius > pEntity->GetPosition().x || m_fX2 + radius < pEntity->GetPosition().x ||
		m_fY1 - radius > pEntity->GetPosition().y || m_fY2 + radius < pEntity->GetPosition().y ||
		m_fZ1 - radius > pEntity->GetPosition().z || m_fZ2 + radius < pEntity->GetPosition().z)
		return false;
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * pColModel->spheres[i].center;
		radius = pColModel->spheres[i].radius;
		if (pos.x + radius > m_fX1 && pos.x - radius < m_fX2 &&
			pos.y + radius > m_fY1 && pos.y - radius < m_fY2 &&
			pos.z + radius > m_fZ1 && pos.z - radius < m_fZ2)
			return true;
	}
	return false;
}

bool CGarage::EntityHasASphereWayOutsideGarage(CEntity * pEntity, float fMargin)
{
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * pColModel->spheres[i].center;
		float radius = pColModel->spheres[i].radius;
		if (pos.x + radius + fMargin < m_fX1 || pos.x - radius - fMargin > m_fX2 ||
			pos.y + radius + fMargin < m_fY1 || pos.y - radius - fMargin > m_fY2 ||
			pos.z + radius + fMargin < m_fZ1 || pos.z - radius - fMargin > m_fZ2)
			return true;
	}
	return false;
}

bool CGarage::IsAnyOtherCarTouchingGarage(CVehicle * pException)
{
	uint32 i = CPools::GetVehiclePool()->GetSize();
	while (i--) {
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle || pVehicle == pException)
			continue;
		if (!IsEntityTouching3D(pVehicle))
			continue;
		CColModel* pColModel = pVehicle->GetColModel();
		for (int i = 0; i < pColModel->numSpheres; i++) {
			CVector pos = pVehicle->GetMatrix() * pColModel->spheres[i].center;
			float radius = pColModel->spheres[i].radius;
			if (pos.x + radius > m_fX1 && pos.x - radius < m_fX2 &&
				pos.y + radius > m_fY1 && pos.y - radius < m_fY2 &&
				pos.z + radius > m_fZ1 && pos.z - radius < m_fZ2)
				return true;
		}
	}
	return false;
}

bool CGarage::IsAnyOtherPedTouchingGarage(CPed * pException)
{
	uint32 i = CPools::GetPedPool()->GetSize();
	while (i--) {
		CPed* pPed = CPools::GetPedPool()->GetSlot(i);
		if (!pPed || pPed == pException)
			continue;
		if (!IsEntityTouching3D(pPed))
			continue;
		CColModel* pColModel = pException->GetColModel();
		for (int i = 0; i < pColModel->numSpheres; i++) {
			CVector pos = pPed->GetMatrix() * pColModel->spheres[i].center;
			float radius = pColModel->spheres[i].radius;
			if (pos.x + radius > m_fX1 && pos.x - radius < m_fX2 &&
				pos.y + radius > m_fY1 && pos.y - radius < m_fY2 &&
				pos.z + radius > m_fZ1 && pos.z - radius < m_fZ2)
				return true;
		}
	}
	return false;
}

bool CGarage::IsAnyCarBlockingDoor()
{
	uint32 i = CPools::GetVehiclePool()->GetSize();
	while (i--) {
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle)
			continue;
		if (!IsEntityTouching3D(pVehicle))
			continue;
		CColModel* pColModel = pVehicle->GetColModel();
		for (int i = 0; i < pColModel->numSpheres; i++) {
			CVector pos = pVehicle->GetMatrix() * pColModel->spheres[i].center;
			float radius = pColModel->spheres[i].radius;
			if (pos.x + radius < m_fX1 || pos.x - radius > m_fX2 ||
				pos.y + radius < m_fY1 || pos.y - radius > m_fY2 ||
				pos.z + radius < m_fZ1 || pos.z - radius > m_fZ2)
				return true;
		}
	}
	return false;
}

int32 CGarage::CountCarsWithCenterPointWithinGarage(CEntity * pException)
{
	int32 total = 0;
	uint32 i = CPools::GetVehiclePool()->GetSize();
	while (i--) {
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle || pVehicle == pException)
			continue;
		if (pVehicle->GetPosition().x > m_fX1 && pVehicle->GetPosition().x < m_fX2 &&
			pVehicle->GetPosition().y > m_fY1 && pVehicle->GetPosition().y < m_fY2 &&
			pVehicle->GetPosition().z > m_fZ1 && pVehicle->GetPosition().z < m_fZ2)
			total++;
	}
	return total;
}

void CGarage::RemoveCarsBlockingDoorNotInside()
{
	uint32 i = CPools::GetVehiclePool()->GetSize();
	while (i--) {
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle)
			continue;
		if (!IsEntityTouching3D(pVehicle))
			continue;
		if (pVehicle->GetPosition().x < m_fX1 || pVehicle->GetPosition().x > m_fX2 ||
			pVehicle->GetPosition().y < m_fY1 || pVehicle->GetPosition().y > m_fY2 ||
			pVehicle->GetPosition().z < m_fZ1 || pVehicle->GetPosition().z > m_fZ2) {
			if (!pVehicle->bIsLocked && pVehicle->CanBeDeleted()) {
				CWorld::Remove(pVehicle);
				delete pVehicle;
#ifndef FIX_BUGS
				return; // makes no sense
#endif
			}
		}
	}
}

void CGarages::PrintMessages()
{
	if (CTimer::GetTimeInMilliseconds() > MessageStartTime && CTimer::GetTimeInMilliseconds() < MessageEndTime) {
#ifdef FIX_BUGS
		CFont::SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.5f));
#else
		CFont::SetScale(1.2f, 1.5f);
#endif
		CFont::SetPropOn();
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
#ifdef FIX_BUGS
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 50));
#else
		CFont::SetCentreSize(SCREEN_WIDTH - 50);
#endif
		CFont::SetCentreOn();
		CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
		CFont::SetColor(CRGBA(0, 0, 0, 255));

#if defined(PS2_HUD) || defined (FIX_BUGS)
		float y_offset = SCREEN_HEIGHT / 3; // THIS is PS2 calculation
#else
		float y_offset = SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(84.0f); // This is PC and results in text being written over some HUD elements
#endif

		if (MessageNumberInString2 >= 0) {
			CMessages::InsertNumberInString(TheText.Get(MessageIDString), MessageNumberInString, MessageNumberInString2, -1, -1, -1, -1, gUString);
#ifdef FIX_BUGS
			CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X(2.0f), y_offset - SCREEN_SCALE_Y(40.0f) + SCREEN_SCALE_Y(2.0f), gUString);
#else
			CFont::PrintString(SCREEN_WIDTH / 2 + 2.0f, y_offset - 40.0f + 2.0f, gUString);
#endif
			CFont::SetColor(CRGBA(89, 115, 150, 255));
#ifdef FIX_BUGS
			CFont::PrintString(SCREEN_WIDTH / 2, y_offset - SCREEN_SCALE_Y(40.0f), gUString);
#else
			CFont::PrintString(SCREEN_WIDTH / 2, y_offset - 40.0f, gUString);
#endif
		}
		else if (MessageNumberInString >= 0) {
			CMessages::InsertNumberInString(TheText.Get(MessageIDString), MessageNumberInString, -1, -1, -1, -1, -1, gUString);
#ifdef FIX_BUGS
			CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X(2.0f), y_offset - SCREEN_SCALE_Y(40.0f) + SCREEN_SCALE_Y(2.0f), gUString);
#else
			CFont::PrintString(SCREEN_WIDTH / 2 + 2.0f, y_offset - 40.0f + 2.0f, gUString);
#endif

			CFont::SetColor(CRGBA(89, 115, 150, 255));
				
#ifdef FIX_BUGS
			CFont::PrintString(SCREEN_WIDTH / 2, y_offset - SCREEN_SCALE_Y(40.0f), gUString);
#else
			CFont::PrintString(SCREEN_WIDTH / 2, y_offset - 40.0f, gUString);
#endif
		}
		else {
#ifdef FIX_BUGS
			CFont::PrintString(SCREEN_WIDTH / 2 - SCREEN_SCALE_X(2.0f), y_offset - SCREEN_SCALE_Y(2.0f), TheText.Get(MessageIDString));
#else
			CFont::PrintString(SCREEN_WIDTH / 2 - 2.0f, y_offset - 2.0f, TheText.Get(MessageIDString));
#endif
			CFont::SetColor(CRGBA(89, 115, 150, 255));
			CFont::PrintString(SCREEN_WIDTH / 2, y_offset, TheText.Get(MessageIDString));
		}
	}
}

bool CGarages::IsCarSprayable(CVehicle * pVehicle)
{
	switch (pVehicle->GetModelIndex()) {
	case MI_FIRETRUCK:
	case MI_AMBULAN:
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_BUS:
	case MI_RHINO:
	case MI_BARRACKS:
	case MI_DODO:
	case MI_COACH:
		return false;
	default:
		break;
	}
	return true;
}

void CGarage::UpdateDoorsHeight()
{
	RefreshDoorPointers(false);
	if (m_pDoor1) {
		m_pDoor1->GetMatrix().GetPosition().z = m_fDoorPos + m_fDoor1Z;
		if (m_bRotatedDoor)
			BuildRotatedDoorMatrix(m_pDoor1, m_fDoorPos / m_fDoorHeight);
		m_pDoor1->GetMatrix().UpdateRW();
		m_pDoor1->UpdateRwFrame();
	}
	if (m_pDoor2) {
		m_pDoor2->GetMatrix().GetPosition().z = m_fDoorPos + m_fDoor2Z;
		if (m_bRotatedDoor)
			BuildRotatedDoorMatrix(m_pDoor2, m_fDoorPos / m_fDoorHeight);
		m_pDoor2->GetMatrix().UpdateRW();
		m_pDoor2->UpdateRwFrame();
	}
}

void CGarage::BuildRotatedDoorMatrix(CEntity * pDoor, float fPosition)
{
	float fAngle = -fPosition * HALFPI;
	CVector up(-Sin(fAngle) * pDoor->GetForward().y, Sin(fAngle) * pDoor->GetForward().x, Cos(fAngle));
	pDoor->GetRight() = CrossProduct(up, pDoor->GetForward());
	pDoor->GetUp() = up;
}

void CGarage::UpdateCrusherAngle()
{
	RefreshDoorPointers(false);
	m_pDoor2->GetMatrix().SetRotateXOnly(TWOPI - m_fDoorPos);
	m_pDoor2->GetMatrix().UpdateRW();
	m_pDoor2->UpdateRwFrame();
}

void CGarage::UpdateCrusherShake(float X, float Y)
{
	RefreshDoorPointers(false);
	m_pDoor1->GetMatrix().GetPosition().x += X;
	m_pDoor1->GetMatrix().GetPosition().y += Y;
	m_pDoor1->GetMatrix().UpdateRW();
	m_pDoor1->UpdateRwFrame();
	m_pDoor1->GetMatrix().GetPosition().x -= X;
	m_pDoor1->GetMatrix().GetPosition().y -= Y;
	m_pDoor2->GetMatrix().GetPosition().x += X;
	m_pDoor2->GetMatrix().GetPosition().y += Y;
	m_pDoor2->GetMatrix().UpdateRW();
	m_pDoor2->UpdateRwFrame();
	m_pDoor2->GetMatrix().GetPosition().x -= X;
	m_pDoor2->GetMatrix().GetPosition().y -= Y;
}

void CGarage::RefreshDoorPointers(bool bCreate)
{
	bool bNeedToFindDoorEntities = bCreate || m_bRecreateDoorOnNextRefresh;
	m_bRecreateDoorOnNextRefresh = false;
	if (m_pDoor1) {
		if (m_bDoor1IsDummy) {
			if (CPools::GetDummyPool()->GetIsFree(CPools::GetDummyPool()->GetJustIndex_NoFreeAssert((CDummy*)m_pDoor1)))
				bNeedToFindDoorEntities = true;
			else {
				if (m_bDoor1PoolIndex != (CPools::GetDummyPool()->GetIndex((CDummy*)m_pDoor1) & 0x7F))
					bNeedToFindDoorEntities = true;
				if (!CGarages::IsModelIndexADoor(m_pDoor1->GetModelIndex()))
					bNeedToFindDoorEntities = true;
			}
		}
		else {
			if (CPools::GetObjectPool()->GetIsFree(CPools::GetObjectPool()->GetJustIndex_NoFreeAssert((CObject*)m_pDoor1)))
				bNeedToFindDoorEntities = true;
			else {
				if (m_bDoor1PoolIndex != (CPools::GetObjectPool()->GetIndex((CObject*)m_pDoor1) & 0x7F))
					bNeedToFindDoorEntities = true;
				if (!CGarages::IsModelIndexADoor(m_pDoor1->GetModelIndex()))
					bNeedToFindDoorEntities = true;
			}
		}
	}
	if (m_pDoor2) {
		if (m_bDoor2IsDummy) {
			if (CPools::GetDummyPool()->GetIsFree(CPools::GetDummyPool()->GetJustIndex_NoFreeAssert((CDummy*)m_pDoor2)))
				bNeedToFindDoorEntities = true;
			else {
				if (m_bDoor2PoolIndex != (CPools::GetDummyPool()->GetIndex((CDummy*)m_pDoor2) & 0x7F))
					bNeedToFindDoorEntities = true;
				if (!CGarages::IsModelIndexADoor(m_pDoor2->GetModelIndex()))
					bNeedToFindDoorEntities = true;
			}
		}
		else {
			if (CPools::GetObjectPool()->GetIsFree(CPools::GetObjectPool()->GetJustIndex_NoFreeAssert((CObject*)m_pDoor2)))
				bNeedToFindDoorEntities = true;
			else {
				if (m_bDoor2PoolIndex != (CPools::GetObjectPool()->GetIndex((CObject*)m_pDoor2) & 0x7F))
					bNeedToFindDoorEntities = true;
				if (!CGarages::IsModelIndexADoor(m_pDoor2->GetModelIndex()))
					bNeedToFindDoorEntities = true;
			}
		}
	}
	if (bNeedToFindDoorEntities)
		FindDoorsEntities();
}

void CGarages::TriggerMessage(const char* text, int16 num1, uint16 time, int16 num2)
{
	if (strcmp(text, MessageIDString) == 0 &&
		CTimer::GetTimeInMilliseconds() >= MessageStartTime &&
		CTimer::GetTimeInMilliseconds() <= MessageEndTime) {
		if (CTimer::GetTimeInMilliseconds() - MessageStartTime <= 500)
			return;
		MessageStartTime = CTimer::GetTimeInMilliseconds() - 500;
		MessageEndTime = CTimer::GetTimeInMilliseconds() - 500 + time;
	}
	else {
		strcpy(MessageIDString, text);
		MessageStartTime = CTimer::GetTimeInMilliseconds();
		MessageEndTime = CTimer::GetTimeInMilliseconds() + time;
	}
	MessageNumberInString = num1;
	MessageNumberInString2 = num2;
}

void CGarages::SetTargetCarForMissonGarage(int16 garage, CVehicle * pVehicle)
{
	assert(garage >= 0 && garage < NUM_GARAGES);
	if (pVehicle) {
		aGarages[garage].m_pTarget = pVehicle;
		if (aGarages[garage].m_eGarageState == GS_CLOSEDCONTAINSCAR)
			aGarages[garage].m_eGarageState = GS_FULLYCLOSED;
	}
	else
		aGarages[garage].m_pTarget = nil;
}

bool CGarages::HasCarBeenDroppedOffYet(int16 garage)
{
	return aGarages[garage].m_eGarageState == GS_CLOSEDCONTAINSCAR;
}

void CGarages::DeActivateGarage(int16 garage)
{
	aGarages[garage].m_bDeactivated = true;
}

void CGarages::ActivateGarage(int16 garage)
{
	aGarages[garage].m_bDeactivated = false;
	if (aGarages[garage].m_eGarageType == GARAGE_FORCARTOCOMEOUTOF && aGarages[garage].m_eGarageState == GS_FULLYCLOSED)
		aGarages[garage].m_eGarageState = GS_OPENING;
}

int32 CGarages::QueryCarsCollected(int16 garage)
{
	return 0;
}

bool CGarages::HasImportExportGarageCollectedThisCar(int16 garage, int8 car)
{
	return CarTypesCollected[GetCarsCollectedIndexForGarageType(aGarages[garage].m_eGarageType)] & (BIT(car));
}

bool CGarages::IsGarageOpen(int16 garage)
{
	return aGarages[garage].IsOpen();
}

bool CGarages::IsGarageClosed(int16 garage)
{
	return aGarages[garage].IsClosed();
}

bool CGarages::HasThisCarBeenCollected(int16 garage, uint8 id)
{
	return aGarages[garage].m_bCollectedCarsState & BIT(id);
}

bool CGarage::DoesCraigNeedThisCar(int32 mi)
{
	if (mi == MI_CORPSE)
		mi = MI_MANANA;
	int ct = CGarages::GetCarsCollectedIndexForGarageType(m_eGarageType);
	for (int i = 0; i < TOTAL_COLLECTCARS_CARS; i++) {
		if (mi == gaCarsToCollectInCraigsGarages[ct][i])
			return (CGarages::CarTypesCollected[ct] & BIT(i)) == 0;
	}
	return false;
}

bool CGarage::HasCraigCollectedThisCar(int32 mi)
{
	if (mi == MI_CORPSE)
		mi = MI_MANANA;
	int ct = CGarages::GetCarsCollectedIndexForGarageType(m_eGarageType);
	for (int i = 0; i < TOTAL_COLLECTCARS_CARS; i++) {
		if (mi == gaCarsToCollectInCraigsGarages[ct][i])
			return CGarages::CarTypesCollected[ct] & BIT(i);
	}
	return false;
}

bool CGarage::MarkThisCarAsCollectedForCraig(int32 mi)
{
	if (mi == MI_CORPSE)
		mi = MI_MANANA;
	int ct = CGarages::GetCarsCollectedIndexForGarageType(m_eGarageType);
	int index;
	for (index = 0; index < TOTAL_COLLECTCARS_CARS; index++) {
		if (mi == gaCarsToCollectInCraigsGarages[ct][index])
			break;
	}
	if (index >= TOTAL_COLLECTCARS_CARS)
		return false;
	CGarages::CarTypesCollected[ct] |= BIT(index);
	CWorld::Players[CWorld::PlayerInFocus].m_nMoney += IMPORT_REWARD;
	for (int i = 0; i < TOTAL_COLLECTCARS_CARS; i++) {
		if ((CGarages::CarTypesCollected[ct] & BIT(i)) == 0) {
			CGarages::TriggerMessage("GA_13", -1, 5000, -1); // Delivered like a pro. Complete the list and there'll be a bonus for you.
			return false;
		}
	}
	CWorld::Players[CWorld::PlayerInFocus].m_nMoney += IMPORT_ALLCARS_REWARD;
	CGarages::TriggerMessage("GA_14", -1, 5000, -1); // All the cars. NICE! Here's a little something.
	return true;
}

void CGarage::OpenThisGarage()
{
	if (m_eGarageState == GS_FULLYCLOSED || m_eGarageState == GS_CLOSING || m_eGarageState == GS_CLOSEDCONTAINSCAR)
		m_eGarageState = GS_OPENING;
}

void CGarage::CloseThisGarage()
{
	if (m_eGarageState == GS_OPENED || m_eGarageState == GS_OPENING)
		m_eGarageState = GS_CLOSING;
}

float CGarage::CalcDistToGarageRectangleSquared(float X, float Y)
{
	float distX, distY;
	if (X < m_fX1)
		distX = m_fX1 - X;
	else if (X > m_fX2)
		distX = X - m_fX2;
	else
		distX = 0.0f;
	if (Y < m_fY1)
		distY = m_fY1 - Y;
	else if (Y > m_fY2)
		distY = Y - m_fY2;
	else
		distY = 0.0f;
	return SQR(distX) + SQR(distY);
}

float CGarage::CalcSmallestDistToGarageDoorSquared(float X, float Y)
{
	float dist1 = 10000000.0f;
	float dist2 = 10000000.0f;
	if (m_pDoor1)
		dist1 = SQR(m_fDoor1X - X) + SQR(m_fDoor1Y - Y);
	if (m_pDoor2)
		dist2 = SQR(m_fDoor2X - X) + SQR(m_fDoor2Y - Y);
	return Min(dist1, dist2);
}

void CGarage::FindDoorsEntities()
{
	m_pDoor1 = nil;
	m_pDoor2 = nil;
	int xstart = Max(0, CWorld::GetSectorIndexX(m_fX1));
	int xend = Min(NUMSECTORS_X - 1, CWorld::GetSectorIndexX(m_fX2));
	int ystart = Max(0, CWorld::GetSectorIndexY(m_fY1));
	int yend = Min(NUMSECTORS_Y - 1, CWorld::GetSectorIndexY(m_fY2));
	assert(xstart <= xend);
	assert(ystart <= yend);

	CWorld::AdvanceCurrentScanCode();

	for (int y = ystart; y <= yend; y++) {
		for (int x = xstart; x <= xend; x++) {
			CSector* s = CWorld::GetSector(x, y);
			FindDoorsEntitiesSectorList(s->m_lists[ENTITYLIST_OBJECTS], false);
			FindDoorsEntitiesSectorList(s->m_lists[ENTITYLIST_OBJECTS_OVERLAP], false);
			FindDoorsEntitiesSectorList(s->m_lists[ENTITYLIST_DUMMIES], true);
			FindDoorsEntitiesSectorList(s->m_lists[ENTITYLIST_DUMMIES_OVERLAP], true);
		}
	}
	if (!m_pDoor1 || !m_pDoor2)
		return;
	if (m_pDoor1->GetModelIndex() == MI_CRUSHERBODY || m_pDoor1->GetModelIndex() == MI_CRUSHERLID)
		return;
	CVector2D vecDoor1ToGarage(m_pDoor1->GetPosition().x - GetGarageCenterX(), m_pDoor1->GetPosition().y - GetGarageCenterY());
	CVector2D vecDoor2ToGarage(m_pDoor2->GetPosition().x - GetGarageCenterX(), m_pDoor2->GetPosition().y - GetGarageCenterY());
	if (DotProduct2D(vecDoor1ToGarage, vecDoor2ToGarage) > 0.0f) {
		if (vecDoor1ToGarage.MagnitudeSqr() >= vecDoor2ToGarage.MagnitudeSqr()) {
			m_pDoor1 = m_pDoor2;
			m_bDoor1IsDummy = m_bDoor2IsDummy;
		}
		m_pDoor2 = nil;
		m_bDoor2IsDummy = false;
	}
}

void CGarage::FindDoorsEntitiesSectorList(CPtrList& list, bool dummy)
{
	CPtrNode* node;
	for (node = list.first; node; node = node->next) {
		CEntity* pEntity = (CEntity*)node->item;
		if (pEntity->m_scanCode == CWorld::GetCurrentScanCode())
			continue;
		pEntity->m_scanCode = CWorld::GetCurrentScanCode();
		if (!pEntity || !CGarages::IsModelIndexADoor(pEntity->GetModelIndex()))
			continue;
		if (Abs(pEntity->GetPosition().x - GetGarageCenterX()) >= DISTANCE_TO_CONSIDER_DOOR_FOR_GARAGE)
			continue;
		if (Abs(pEntity->GetPosition().y - GetGarageCenterY()) >= DISTANCE_TO_CONSIDER_DOOR_FOR_GARAGE)
			continue;
		if (pEntity->GetModelIndex() == MI_CRUSHERBODY) {
			m_pDoor1 = pEntity;
			m_bDoor1IsDummy = dummy;
			// very odd pool operations, they could have used GetJustIndex
			if (dummy)
				m_bDoor1PoolIndex = (CPools::GetDummyPool()->GetIndex((CDummy*)pEntity)) & 0x7F;
			else
				m_bDoor1PoolIndex = (CPools::GetObjectPool()->GetIndex((CObject*)pEntity)) & 0x7F;
			continue;
		}
		if (pEntity->GetModelIndex() == MI_CRUSHERLID) {
			m_pDoor2 = pEntity;
			m_bDoor2IsDummy = dummy;
			if (dummy)
				m_bDoor2PoolIndex = (CPools::GetDummyPool()->GetIndex((CDummy*)pEntity)) & 0x7F;
			else
				m_bDoor2PoolIndex = (CPools::GetObjectPool()->GetIndex((CObject*)pEntity)) & 0x7F;
			continue;
		}
		if (!m_pDoor1) {
			m_pDoor1 = pEntity;
			m_bDoor1IsDummy = dummy;
			if (dummy)
				m_bDoor1PoolIndex = (CPools::GetDummyPool()->GetIndex((CDummy*)pEntity)) & 0x7F;
			else
				m_bDoor1PoolIndex = (CPools::GetObjectPool()->GetIndex((CObject*)pEntity)) & 0x7F;
			continue;
		}
		else {
			m_pDoor2 = pEntity;
			m_bDoor2IsDummy = dummy;
			if (dummy)
				m_bDoor2PoolIndex = (CPools::GetDummyPool()->GetIndex((CDummy*)pEntity)) & 0x7F;
			else
				m_bDoor2PoolIndex = (CPools::GetObjectPool()->GetIndex((CObject*)pEntity)) & 0x7F;
		}
	}
}

bool CGarages::HasResprayHappened(int16 garage)
{
	bool result = aGarages[garage].m_bResprayHappened;
	aGarages[garage].m_bResprayHappened = false;
	return result;
}

void CGarages::SetGarageDoorToRotate(int16 garage)
{
	if (aGarages[garage].m_bRotatedDoor)
		return;
	aGarages[garage].m_bRotatedDoor = true;
	aGarages[garage].m_fDoorHeight /= 2.0f;
	aGarages[garage].m_fDoorHeight -= 0.1f;
}

void CGarages::SetLeaveCameraForThisGarage(int16 garage)
{
	aGarages[garage].m_bCameraFollowsPlayer = true;
}

bool CGarages::IsThisCarWithinGarageArea(int16 garage, CEntity * pCar)
{
	return aGarages[garage].IsEntityEntirelyInside3D(pCar, 0.0f);
}

bool CGarages::HasCarBeenCrushed(int32 handle)
{
	return CrushedCarId == handle;
}

void CStoredCar::StoreCar(CVehicle* pVehicle)
{
	m_nModelIndex = pVehicle->GetModelIndex();
	m_vecPos = pVehicle->GetPosition();
	m_vecAngle = pVehicle->GetForward();
	m_nPrimaryColor = pVehicle->m_currentColour1;
	m_nSecondaryColor = pVehicle->m_currentColour2;
	m_nRadioStation = pVehicle->m_nRadioStation;
	m_nVariationA = pVehicle->m_aExtras[0];
	m_nVariationB = pVehicle->m_aExtras[1];
	m_bBulletproof = pVehicle->bBulletProof;
	m_bFireproof = pVehicle->bFireProof;
	m_bExplosionproof = pVehicle->bExplosionProof;
	m_bCollisionproof = pVehicle->bCollisionProof;
	m_bMeleeproof = pVehicle->bMeleeProof;
	if (pVehicle->IsCar())
		m_nCarBombType = ((CAutomobile*)pVehicle)->m_bombType;
}

CVehicle* CStoredCar::RestoreCar()
{
	CStreaming::RequestModel(m_nModelIndex, STREAMFLAGS_DEPENDENCY);
	if (!CStreaming::HasModelLoaded(m_nModelIndex))
		return nil;
#ifdef FIX_BUGS
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	assert(pModelInfo);
	if (pModelInfo->m_numComps != 0)
#endif
	{
		CVehicleModelInfo::SetComponentsToUse(m_nVariationA, m_nVariationB);
	}
#ifdef FIX_BUGS
	CVehicle* pVehicle;
	if (CModelInfo::IsBoatModel(m_nModelIndex))
		pVehicle = new CBoat(m_nModelIndex, RANDOM_VEHICLE);
	else
		pVehicle = new CAutomobile(m_nModelIndex, RANDOM_VEHICLE);
#else
	CVehicle* pVehicle = new CAutomobile(m_nModelIndex, RANDOM_VEHICLE);
#endif
	pVehicle->SetPosition(m_vecPos);
	pVehicle->SetStatus(STATUS_ABANDONED);
	pVehicle->GetForward() = m_vecAngle;
	pVehicle->GetRight() = CVector(m_vecAngle.y, -m_vecAngle.x, 0.0f);
	pVehicle->GetUp() = CVector(0.0f, 0.0f, 1.0f);
	pVehicle->pDriver = nil;
	pVehicle->m_currentColour1 = m_nPrimaryColor;
	pVehicle->m_currentColour2 = m_nSecondaryColor;
	pVehicle->m_nRadioStation = m_nRadioStation;
	pVehicle->bFreebies = false;
#ifdef FIX_BUGS
	if (pVehicle->IsCar())
#endif
	{
		((CAutomobile*)pVehicle)->m_bombType = m_nCarBombType;
#ifdef FIX_BUGS
		if (m_nCarBombType != CARBOMB_NONE)
			((CAutomobile*)pVehicle)->m_pBombRigger = FindPlayerPed();
#endif
	}
	pVehicle->bHasBeenOwnedByPlayer = true;
	pVehicle->m_nDoorLock = CARLOCK_UNLOCKED;
	pVehicle->bBulletProof = m_bBulletproof;
	pVehicle->bFireProof = m_bFireproof;
	pVehicle->bExplosionProof = m_bExplosionproof;
	pVehicle->bCollisionProof = m_bCollisionproof;
	pVehicle->bMeleeProof = m_bMeleeproof;
	return pVehicle;
}

void CGarage::StoreAndRemoveCarsForThisHideout(CStoredCar* aCars, int32 nMax)
{
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++)
		aCars[i].Clear();
	int i = CPools::GetVehiclePool()->GetSize();
	int index = 0;
	while (i--) {
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle)
			continue;
		if (pVehicle->GetPosition().x > m_fX1 && pVehicle->GetPosition().x < m_fX2 &&
			pVehicle->GetPosition().y > m_fY1 && pVehicle->GetPosition().y < m_fY2 &&
			pVehicle->GetPosition().z > m_fZ1 && pVehicle->GetPosition().z < m_fZ2) {
			if (pVehicle->VehicleCreatedBy != MISSION_VEHICLE) {
				if (index < Max(NUM_GARAGE_STORED_CARS, nMax) && !EntityHasASphereWayOutsideGarage(pVehicle, 1.0f))
					aCars[index++].StoreCar(pVehicle);
				CWorld::Players[CWorld::PlayerInFocus].CancelPlayerEnteringCars(pVehicle);
				CWorld::Remove(pVehicle);
				delete pVehicle;
			}
		}
	}
	// why?
	for (i = index; i < NUM_GARAGE_STORED_CARS; i++)
		aCars[i].Clear();
}

bool CGarage::RestoreCarsForThisHideout(CStoredCar* aCars)
{
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		if (aCars[i].HasCar()) {
			CVehicle* pVehicle = aCars[i].RestoreCar();
			if (pVehicle) {
				CWorld::Add(pVehicle);
				aCars[i].Clear();
			}
		}
	}
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		if (aCars[i].HasCar())
			return false;
	}
	return true;
}

bool CGarages::IsPointInAGarageCameraZone(CVector point)
{
	for (int i = 0; i < NUM_GARAGES; i++) {
		switch (aGarages[i].m_eGarageType) {
		case GARAGE_NONE:
			break;
		case GARAGE_COLLECTCARS_1:
		case GARAGE_COLLECTCARS_2:
		case GARAGE_COLLECTCARS_3:
			if (aGarages[i].m_fX1 - MARGIN_FOR_CAMERA_COLLECTCARS <= point.x &&
				aGarages[i].m_fX2 + MARGIN_FOR_CAMERA_COLLECTCARS >= point.x &&
				aGarages[i].m_fY1 - MARGIN_FOR_CAMERA_COLLECTCARS <= point.y &&
				aGarages[i].m_fY2 + MARGIN_FOR_CAMERA_COLLECTCARS >= point.y)
				return true;
			break;
		default:
			if (aGarages[i].m_fX1 - MARGIN_FOR_CAMERA_DEFAULT <= point.x &&
				aGarages[i].m_fX2 + MARGIN_FOR_CAMERA_DEFAULT >= point.x &&
				aGarages[i].m_fY1 - MARGIN_FOR_CAMERA_DEFAULT <= point.y &&
				aGarages[i].m_fY2 + MARGIN_FOR_CAMERA_DEFAULT >= point.y)
				return true;
			break;
		}
	}
	return false;
}

bool CGarages::CameraShouldBeOutside()
{
	return bCamShouldBeOutisde;
}

void CGarages::GivePlayerDetonator()
{
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_DETONATOR, 1);
	FindPlayerPed()->GetWeapon(FindPlayerPed()->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_eWeaponState = WEAPONSTATE_READY;
}

float CGarages::FindDoorHeightForMI(int32 mi)
{
	return CModelInfo::GetModelInfo(mi)->GetColModel()->boundingBox.max.z - CModelInfo::GetModelInfo(mi)->GetColModel()->boundingBox.min.z - 0.1f;
}

void CGarage::TidyUpGarage()
{
	uint32 i = CPools::GetVehiclePool()->GetSize();
#ifdef FIX_BUGS
	while (i--) {
#else
	while (--i) {
#endif
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle || !pVehicle->IsCar())
			continue;
		if (pVehicle->GetPosition().x > m_fX1 && pVehicle->GetPosition().x < m_fX2 &&
			pVehicle->GetPosition().y > m_fY1 && pVehicle->GetPosition().y < m_fY2 &&
			pVehicle->GetPosition().z > m_fZ1 && pVehicle->GetPosition().z < m_fZ2) {
			if (pVehicle->GetStatus() == STATUS_WRECKED || pVehicle->GetUp().z < 0.5f) {
				CWorld::Remove(pVehicle);
				delete pVehicle;
			}
		}
	}
}

void CGarage::TidyUpGarageClose()
{
	uint32 i = CPools::GetVehiclePool()->GetSize();
#ifdef FIX_BUGS
	while (i--) {
#else
	while (--i) {
#endif
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!pVehicle || !pVehicle->IsCar())
			continue;
		if (!pVehicle->IsCar() || pVehicle->GetStatus() != STATUS_WRECKED || !IsEntityTouching3D(pVehicle))
			continue;
		bool bRemove = false;
		if (m_eGarageState != GS_FULLYCLOSED) {
			CColModel* pColModel = pVehicle->GetColModel();
			for (int i = 0; i < pColModel->numSpheres; i++) {
				CVector pos = pVehicle->GetMatrix() * pColModel->spheres[i].center;
				float radius = pColModel->spheres[i].radius;
				if (pos.x + radius < m_fX1 || pos.x - radius > m_fX2 ||
					pos.y + radius < m_fY1 || pos.y - radius > m_fY2 ||
					pos.z + radius < m_fZ1 || pos.z - radius > m_fZ2) {
					bRemove = true;
				}
			}
		}
		else
			bRemove = true;
		if (bRemove) {
			// no MISSION_VEHICLE check???
			CWorld::Remove(pVehicle);
			delete pVehicle;
		}
	}
}

void CGarages::PlayerArrestedOrDied()
{
	static int GarageToBeTidied = 0; // lol
	for (int i = 0; i < NUM_GARAGES; i++) {
		if (aGarages[i].m_eGarageType != GARAGE_NONE)
			aGarages[i].PlayerArrestedOrDied();
	}
	MessageEndTime = 0;
	MessageStartTime = 0;
}

void CGarage::PlayerArrestedOrDied()
{
	switch (m_eGarageType) {
	case GARAGE_MISSION:
	case GARAGE_COLLECTORSITEMS:
	case GARAGE_COLLECTSPECIFICCARS:
	case GARAGE_COLLECTCARS_1:
	case GARAGE_COLLECTCARS_2:
	case GARAGE_COLLECTCARS_3:
	case GARAGE_FORCARTOCOMEOUTOF:
	case GARAGE_60SECONDS:
	case GARAGE_MISSION_KEEPCAR:
	case GARAGE_FOR_SCRIPT_TO_OPEN:
	case GARAGE_HIDEOUT_ONE:
	case GARAGE_HIDEOUT_TWO:
	case GARAGE_HIDEOUT_THREE:
	case GARAGE_FOR_SCRIPT_TO_OPEN_AND_CLOSE:
	case GARAGE_KEEPS_OPENING_FOR_SPECIFIC_CAR:
	case GARAGE_MISSION_KEEPCAR_REMAINCLOSED:
		switch (m_eGarageState) {
		case GS_OPENED:
		case GS_CLOSING:
		case GS_OPENING:
			m_eGarageState = GS_CLOSING;
			break;
		default:
			break;
		}
		break;
	case GARAGE_BOMBSHOP1:
	case GARAGE_BOMBSHOP2:
	case GARAGE_BOMBSHOP3:
	case GARAGE_RESPRAY:
	case GARAGE_CRUSHER:
		switch (m_eGarageState) {
		case GS_FULLYCLOSED:
		case GS_CLOSING:
		case GS_OPENING:
			m_eGarageState = GS_OPENING;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CGarage::CenterCarInGarage(CVehicle* pVehicle)
{
	if (IsAnyOtherCarTouchingGarage(FindPlayerVehicle()))
		return;
	if (IsAnyOtherPedTouchingGarage(FindPlayerPed()))
		return;
	CVector pos = pVehicle->GetPosition();
	float garageX = GetGarageCenterX();
	float garageY = GetGarageCenterY();
	float offsetX = garageX - pos.x;
	float offsetY = garageY - pos.y;
	float offsetZ = pos.z - pos.z;
	float distance = CVector(offsetX, offsetY, offsetZ).Magnitude();
	if (distance < RESPRAY_CENTERING_COEFFICIENT) {
		pVehicle->GetMatrix().GetPosition().x = GetGarageCenterX();
		pVehicle->GetMatrix().GetPosition().y = GetGarageCenterY();
	}
	else {
		pVehicle->GetMatrix().GetPosition().x += offsetX * RESPRAY_CENTERING_COEFFICIENT / distance;
		pVehicle->GetMatrix().GetPosition().y += offsetY * RESPRAY_CENTERING_COEFFICIENT / distance;
	}
	if (!IsEntityEntirelyInside3D(pVehicle, 0.1f))
		pVehicle->SetPosition(pos);
}

void CGarages::CloseHideOutGaragesBeforeSave()
{
	for (int i = 0; i < NUM_GARAGES; i++) {
		if (aGarages[i].m_eGarageType != GARAGE_HIDEOUT_ONE &&
			aGarages[i].m_eGarageType != GARAGE_HIDEOUT_TWO &&
			aGarages[i].m_eGarageType != GARAGE_HIDEOUT_THREE)
			continue;
		if (aGarages[i].m_eGarageState != GS_FULLYCLOSED &&
			(aGarages[i].m_eGarageType != GARAGE_HIDEOUT_ONE || !aGarages[i].IsAnyCarBlockingDoor())) {
			aGarages[i].m_eGarageState = GS_FULLYCLOSED;
			switch (aGarages[i].m_eGarageType) {
			case GARAGE_HIDEOUT_ONE:
				aGarages[i].StoreAndRemoveCarsForThisHideout(aCarsInSafeHouse1, NUM_GARAGE_STORED_CARS);
				aGarages[i].RemoveCarsBlockingDoorNotInside();
				break;
			case GARAGE_HIDEOUT_TWO:
				aGarages[i].StoreAndRemoveCarsForThisHideout(aCarsInSafeHouse2, NUM_GARAGE_STORED_CARS);
				aGarages[i].RemoveCarsBlockingDoorNotInside();
				break;
			case GARAGE_HIDEOUT_THREE:
				aGarages[i].StoreAndRemoveCarsForThisHideout(aCarsInSafeHouse3, NUM_GARAGE_STORED_CARS);
				aGarages[i].RemoveCarsBlockingDoorNotInside();
				break;
			default:
				break;
			}
		}
		aGarages[i].m_fDoorPos = 0.0f;
		aGarages[i].UpdateDoorsHeight();
	}
}

int32 CGarages::CountCarsInHideoutGarage(uint8 type)
{
	int32 total = 0;
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		switch (type) {
		case GARAGE_HIDEOUT_ONE:
			total += (aCarsInSafeHouse1[i].HasCar());
			break;
		case GARAGE_HIDEOUT_TWO:
			total += (aCarsInSafeHouse2[i].HasCar());
			break;
		case GARAGE_HIDEOUT_THREE:
			total += (aCarsInSafeHouse3[i].HasCar());
			break;
		default: break;
		}
	}
	return total;
}

int32 CGarages::FindMaxNumStoredCarsForGarage(uint8 type)
{
	switch (type) {
	case GARAGE_HIDEOUT_ONE:
		return LIMIT_CARS_IN_INDUSTRIAL;
	case GARAGE_HIDEOUT_TWO:
		return LIMIT_CARS_IN_COMMERCIAL;
	case GARAGE_HIDEOUT_THREE:
		return LIMIT_CARS_IN_SUBURBAN;
	default: break;
	}
	return 0;
}

bool CGarages::IsPointWithinHideOutGarage(Const CVector& point)
{
	for (int i = 0; i < NUM_GARAGES; i++) {
		switch (aGarages[i].m_eGarageType) {
		case GARAGE_HIDEOUT_ONE:
		case GARAGE_HIDEOUT_TWO:
		case GARAGE_HIDEOUT_THREE:
			if (point.x > aGarages[i].m_fX1 && point.x < aGarages[i].m_fX2 &&
				point.y > aGarages[i].m_fY1 && point.y < aGarages[i].m_fY2 &&
				point.z > aGarages[i].m_fZ1 && point.z < aGarages[i].m_fZ2)
				return true;
		default: break;
		}
	}
	return false;
}

bool CGarages::IsPointWithinAnyGarage(Const CVector& point)
{
	for (int i = 0; i < NUM_GARAGES; i++) {
		switch (aGarages[i].m_eGarageType) {
		case GARAGE_NONE:
			continue;
		default:
			if (point.x > aGarages[i].m_fX1 && point.x < aGarages[i].m_fX2 &&
				point.y > aGarages[i].m_fY1 && point.y < aGarages[i].m_fY2 &&
				point.z > aGarages[i].m_fZ1 && point.z < aGarages[i].m_fZ2)
				return true;
		}
	}
	return false;
}

void CGarages::SetAllDoorsBackToOriginalHeight()
{
	for (int i = 0; i < NUM_GARAGES; i++) {
		switch (aGarages[i].m_eGarageType) {
		case GARAGE_NONE:
			continue;
		default:
			aGarages[i].RefreshDoorPointers(true);
			if (aGarages[i].m_pDoor1) {
				aGarages[i].m_pDoor1->GetMatrix().GetPosition().z = aGarages[i].m_fDoor1Z;
				if (aGarages[i].m_pDoor1->IsObject())
					((CObject*)aGarages[i].m_pDoor1)->m_objectMatrix.GetPosition().z = aGarages[i].m_fDoor1Z;
				if (aGarages[i].m_bRotatedDoor)
					aGarages[i].BuildRotatedDoorMatrix(aGarages[i].m_pDoor1, 0.0f);
				aGarages[i].m_pDoor1->GetMatrix().UpdateRW();
				aGarages[i].m_pDoor1->UpdateRwFrame();
			}
			if (aGarages[i].m_pDoor2) {
				aGarages[i].m_pDoor2->GetMatrix().GetPosition().z = aGarages[i].m_fDoor2Z;
				if (aGarages[i].m_pDoor2->IsObject())
					((CObject*)aGarages[i].m_pDoor2)->m_objectMatrix.GetPosition().z = aGarages[i].m_fDoor2Z;
				if (aGarages[i].m_bRotatedDoor)
					aGarages[i].BuildRotatedDoorMatrix(aGarages[i].m_pDoor2, 0.0f);
				aGarages[i].m_pDoor2->GetMatrix().UpdateRW();
				aGarages[i].m_pDoor2->UpdateRwFrame();
			}
		}
	}
}

void CGarages::Save(uint8 * buf, uint32 * size)
{
#ifdef FIX_GARAGE_SIZE
	INITSAVEBUF
	*size = (6 * sizeof(uint32) + TOTAL_COLLECTCARS_GARAGES * sizeof(*CarTypesCollected) + sizeof(uint32) + 3 * NUM_GARAGE_STORED_CARS * sizeof(CStoredCar) + NUM_GARAGES * sizeof(CGarage));
#else
	* size = 5484;
#endif
	CloseHideOutGaragesBeforeSave();
	WriteSaveBuf(buf, NumGarages);
	WriteSaveBuf(buf, (uint32)BombsAreFree);
	WriteSaveBuf(buf, (uint32)RespraysAreFree);
	WriteSaveBuf(buf, CarsCollected);
	WriteSaveBuf(buf, BankVansCollected);
	WriteSaveBuf(buf, PoliceCarsCollected);
	for (int i = 0; i < TOTAL_COLLECTCARS_GARAGES; i++)
		WriteSaveBuf(buf, CarTypesCollected[i]);
	WriteSaveBuf(buf, LastTimeHelpMessage);
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		WriteSaveBuf(buf, aCarsInSafeHouse1[i]);
		WriteSaveBuf(buf, aCarsInSafeHouse2[i]);
		WriteSaveBuf(buf, aCarsInSafeHouse3[i]);
	}
	for (int i = 0; i < NUM_GARAGES; i++)
		WriteSaveBuf(buf, aGarages[i]);
#ifdef FIX_GARAGE_SIZE
	VALIDATESAVEBUF(*size);
#endif
}

const CStoredCar &CStoredCar::operator=(const CStoredCar & other)
{
	m_nModelIndex = other.m_nModelIndex;
	m_vecPos = other.m_vecPos;
	m_vecAngle = other.m_vecAngle;
	m_bBulletproof = other.m_bBulletproof;
	m_bFireproof = other.m_bFireproof;
	m_bExplosionproof = other.m_bExplosionproof;
	m_bCollisionproof = other.m_bCollisionproof;
	m_bMeleeproof = other.m_bMeleeproof;
	m_nPrimaryColor = other.m_nPrimaryColor;
	m_nSecondaryColor = other.m_nSecondaryColor;
	m_nRadioStation = other.m_nRadioStation;
	m_nVariationA = other.m_nVariationA;
	m_nVariationB = other.m_nVariationB;
	m_nCarBombType = other.m_nCarBombType;
	return *this;
}

void CGarages::Load(uint8* buf, uint32 size)
{
#ifdef FIX_GARAGE_SIZE
	INITSAVEBUF
	assert(size == (6 * sizeof(uint32) + TOTAL_COLLECTCARS_GARAGES * sizeof(*CarTypesCollected) + sizeof(uint32) + 3 * NUM_GARAGE_STORED_CARS * sizeof(CStoredCar) + NUM_GARAGES * sizeof(CGarage));
#else
	assert(size == 5484);
#endif
	CloseHideOutGaragesBeforeSave();
	NumGarages = ReadSaveBuf<uint32>(buf);
	BombsAreFree = ReadSaveBuf<uint32>(buf);
	RespraysAreFree = ReadSaveBuf<uint32>(buf);
	CarsCollected = ReadSaveBuf<int32>(buf);
	BankVansCollected = ReadSaveBuf<int32>(buf);
	PoliceCarsCollected = ReadSaveBuf<int32>(buf);
	for (int i = 0; i < TOTAL_COLLECTCARS_GARAGES; i++)
		CarTypesCollected[i] = ReadSaveBuf<uint32>(buf);
	LastTimeHelpMessage = ReadSaveBuf<uint32>(buf);
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		aCarsInSafeHouse1[i] = ReadSaveBuf<CStoredCar>(buf);
		aCarsInSafeHouse2[i] = ReadSaveBuf<CStoredCar>(buf);
		aCarsInSafeHouse3[i] = ReadSaveBuf<CStoredCar>(buf);
	}
	for (int i = 0; i < NUM_GARAGES; i++) {
		aGarages[i] = ReadSaveBuf<CGarage>(buf);
		aGarages[i].m_pDoor1 = nil;
		aGarages[i].m_pDoor2 = nil;
		aGarages[i].m_pTarget = nil;
		aGarages[i].field_96 = nil;
		aGarages[i].m_bRecreateDoorOnNextRefresh = true;
		aGarages[i].RefreshDoorPointers(true);
		if (aGarages[i].m_eGarageType == GARAGE_CRUSHER)
			aGarages[i].UpdateCrusherAngle();
		else
			aGarages[i].UpdateDoorsHeight();
	}
#ifdef FIX_GARAGE_SIZE
	VALIDATESAVEBUF(size);
#endif

	MessageEndTime = 0;
	bCamShouldBeOutisde = false;
	MessageStartTime = 0;
}

bool
CGarages::IsModelIndexADoor(uint32 id)
{
	return id == MI_GARAGEDOOR1 ||
		id == MI_GARAGEDOOR2 ||
		id == MI_GARAGEDOOR3 ||
		id == MI_GARAGEDOOR4 ||
		id == MI_GARAGEDOOR5 ||
		id == MI_GARAGEDOOR6 ||
		id == MI_GARAGEDOOR7 ||
		id == MI_GARAGEDOOR9 ||
		id == MI_GARAGEDOOR10 ||
		id == MI_GARAGEDOOR11 ||
		id == MI_GARAGEDOOR12 ||
		id == MI_GARAGEDOOR13 ||
		id == MI_GARAGEDOOR14 ||
		id == MI_GARAGEDOOR15 ||
		id == MI_GARAGEDOOR16 ||
		id == MI_GARAGEDOOR17 ||
		id == MI_GARAGEDOOR18 ||
		id == MI_GARAGEDOOR19 ||
		id == MI_GARAGEDOOR20 ||
		id == MI_GARAGEDOOR21 ||
		id == MI_GARAGEDOOR22 ||
		id == MI_GARAGEDOOR23 ||
		id == MI_GARAGEDOOR24 ||
		id == MI_GARAGEDOOR25 ||
		id == MI_GARAGEDOOR26 ||
		id == MI_GARAGEDOOR27 ||
		id == MI_GARAGEDOOR28 ||
		id == MI_GARAGEDOOR29 ||
		id == MI_GARAGEDOOR30 ||
		id == MI_GARAGEDOOR31 ||
		id == MI_GARAGEDOOR32 ||
		id == MI_CRUSHERBODY ||
		id == MI_CRUSHERLID;
}

void CGarages::StopCarFromBlowingUp(CAutomobile* pCar)
{
	pCar->m_fFireBlowUpTimer = 0.0f;
	pCar->m_fHealth = Max(pCar->m_fHealth, 300.0f);
	pCar->Damage.SetEngineStatus(Max(pCar->Damage.GetEngineStatus(), 275));
}

bool CGarage::Does60SecondsNeedThisCarAtAll(int mi)
{
	for (int i = 0; i < ARRAY_SIZE(gaCarsToCollectIn60Seconds); i++) {
		if (gaCarsToCollectIn60Seconds[i] == mi)
			return true;
	}
	return false;
}

bool CGarage::Does60SecondsNeedThisCar(int mi)
{
	for (int i = 0; i < ARRAY_SIZE(gaCarsToCollectIn60Seconds); i++) {
		if (gaCarsToCollectIn60Seconds[i] == mi)
			return m_bCollectedCarsState & BIT(i);
	}
	return false;
}

void CGarage::MarkThisCarAsCollectedFor60Seconds(int mi)
{
	for (int i = 0; i < ARRAY_SIZE(gaCarsToCollectIn60Seconds); i++) {
		if (gaCarsToCollectIn60Seconds[i] == mi)
			m_bCollectedCarsState |= BIT(i);
	}
}

bool CGarage::IsPlayerEntirelyInsideGarage()
{
	return IsEntityEntirelyInside3D(FindPlayerVehicle() ? (CEntity*)FindPlayerVehicle() : (CEntity*)FindPlayerPed(), 0.0f);
}
