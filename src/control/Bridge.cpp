#include "common.h"
#include "patcher.h"
#include "Bridge.h"
#include "Pools.h"
#include "ModelIndices.h"
#include "PathFind.h"
#include "Stats.h"

CEntity*& CBridge::pLiftRoad = *(CEntity**)0x8E2C8C;
CEntity*& CBridge::pLiftPart = *(CEntity**)0x8E2C94;
CEntity*& CBridge::pWeight   = *(CEntity**)0x8E28BC;

int& CBridge::State    = *(int*)0x8F2A1C;
int& CBridge::OldState = *(int*)0x8F2A20;

float& CBridge::DefaultZLiftPart   = *(float*)0x941430;
float& CBridge::DefaultZLiftRoad   = *(float*)0x941438;
float& CBridge::DefaultZLiftWeight = *(float*)0x8F1A44;

float& CBridge::OldLift = *(float*)0x8F6254;

uint32& CBridge::TimeOfBridgeBecomingOperational = *(uint32*)0x8F2BC0;

void CBridge::Init()
{
	FindBridgeEntities();
	OldLift = -1.0f;
	if (pLiftPart && pWeight)
	{
		DefaultZLiftPart   = pLiftPart->GetPosition().z;
		DefaultZLiftWeight = pWeight->GetPosition().z;

		if (pLiftRoad)
			DefaultZLiftRoad = pLiftRoad->GetPosition().z;

		ThePaths.SetLinksBridgeLights(-330.0, -230.0, -700.0, -588.0, true);
	}
}

void CBridge::Update()
{
	if (!pLiftPart || !pWeight)
		return;

	OldState = State;

	float liftHeight;

	// Set bridge height and state
	if (CStats::CommercialPassed)
	{
		if (TimeOfBridgeBecomingOperational == 0)
			TimeOfBridgeBecomingOperational = CTimer::GetTimeInMilliseconds();

		// Time remaining for bridge to become operational
		// uint16, so after about a minute it overflows to 0 and the cycle repeats
		uint16 timeElapsed = CTimer::GetTimeInMilliseconds() - TimeOfBridgeBecomingOperational;

		// Calculate lift part height and bridge state
		if (timeElapsed < 10000)
		{
			State = STATE_LIFT_PART_MOVING_DOWN;
			liftHeight = 25.0 - timeElapsed / 10000.0 * 25.0f;
		}
		else if (timeElapsed < 40000)
		{
			liftHeight = 0.0f;
			State = STATE_LIFT_PART_IS_DOWN;
		}
		else if (timeElapsed < 50000)
		{
			liftHeight = 0.0f;
			State = STATE_LIFT_PART_ABOUT_TO_MOVE_UP;
		}
		else if (timeElapsed < 60000)
		{
			State = STATE_LIFT_PART_MOVING_UP;
			liftHeight = (timeElapsed - 50000) / 10000.0 * 25.0f;
		}
		else
		{
			liftHeight = 25.0f;
			State = STATE_LIFT_PART_IS_UP;
		}
	}
	else
	{
		liftHeight = 25.0f;
		TimeOfBridgeBecomingOperational = 0;
		State = STATE_BRIDGE_LOCKED;
	}

	// Move bridge part
	if (liftHeight != OldLift)
	{
		pLiftPart->GetPosition().z = DefaultZLiftPart + liftHeight;
		pLiftPart->GetMatrix().UpdateRW();
		pLiftPart->UpdateRwFrame();
		if (pLiftRoad)
		{
			pLiftRoad->GetPosition().z = DefaultZLiftRoad + liftHeight;
			pLiftRoad->GetMatrix().UpdateRW();
			pLiftRoad->UpdateRwFrame();
		}
		pWeight->GetPosition().z = DefaultZLiftWeight - liftHeight;
		pWeight->GetMatrix().UpdateRW();
		pWeight->UpdateRwFrame();

		OldLift = liftHeight;
	}

	if (State == STATE_LIFT_PART_ABOUT_TO_MOVE_UP && OldState == STATE_LIFT_PART_IS_DOWN)
		ThePaths.SetLinksBridgeLights(-330.0, -230.0, -700.0, -588.0, true);
	else if (State == STATE_LIFT_PART_IS_DOWN && OldState == STATE_LIFT_PART_MOVING_DOWN)
		ThePaths.SetLinksBridgeLights(-330.0, -230.0, -700.0, -588.0, false);
}

bool CBridge::ShouldLightsBeFlashing() { return State != STATE_LIFT_PART_IS_DOWN; }

void CBridge::FindBridgeEntities()
{
	pWeight   = nil;
	pLiftRoad = nil;
	pLiftPart = nil;

	for (int i = CPools::GetBuildingPool()->GetSize()-1; i >= 0; i--) {
		CBuilding* entry = CPools::GetBuildingPool()->GetSlot(i);
		if (entry)
		{
			if (entry->GetModelIndex() == MI_BRIDGELIFT)
				pLiftPart = entry;
			else if (entry->GetModelIndex() == MI_BRIDGEROADSEGMENT)
				pLiftRoad = entry;
			else if (entry->GetModelIndex() == MI_BRIDGEWEIGHT)
				pWeight = entry;
		}
	}
}

bool CBridge::ThisIsABridgeObjectMovingUp(int index)
{
	if (index != MI_BRIDGEROADSEGMENT && index != MI_BRIDGELIFT)
		return false;

	return State == STATE_LIFT_PART_ABOUT_TO_MOVE_UP || State == STATE_LIFT_PART_MOVING_UP;
}

STARTPATCHES
	InjectHook(0x413A30, &CBridge::Init, PATCH_JUMP);
	InjectHook(0x413AC0, &CBridge::Update, PATCH_JUMP);
	InjectHook(0x413D10, &CBridge::ShouldLightsBeFlashing, PATCH_JUMP);
	InjectHook(0x413D20, &CBridge::FindBridgeEntities, PATCH_JUMP);
	InjectHook(0x413DE0, &CBridge::ThisIsABridgeObjectMovingUp, PATCH_JUMP);
ENDPATCHES