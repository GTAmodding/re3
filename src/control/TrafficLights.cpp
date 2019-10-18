#include "common.h"
#include "patcher.h"
#include "TrafficLights.h"
#include "Timer.h"
#include "Vehicle.h"

WRAPPER void CTrafficLights::DisplayActualLight(CEntity *ent) { EAXJMP(0x455800); }
WRAPPER void CTrafficLights::ScanForLightsOnMap(void) { EAXJMP(0x454F40); }
WRAPPER bool CTrafficLights::ShouldCarStopForLight(CVehicle*, bool) { EAXJMP(0x455350); }
WRAPPER bool CTrafficLights::ShouldCarStopForBridge(CVehicle*) { EAXJMP(0x456460); }

uint8
CTrafficLights::LightForPeds(void)
{
	uint32 period = CTimer::GetTimeInMilliseconds() & 0x3FFF; // Equals to % 16384

	if (period >= 15384)
		return PED_LIGHTS_WALK_BLINK;
	else if (period >= 12000)
		return PED_LIGHTS_WALK;
	else
		return PED_LIGHTS_DONT_WALK;
}