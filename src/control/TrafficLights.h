#pragma once

class CEntity;
class CVehicle;

enum {
	PED_LIGHTS_WALK,
	PED_LIGHTS_WALK_BLINK,
	PED_LIGHTS_DONT_WALK,
};

class CTrafficLights
{
public:
	static void DisplayActualLight(CEntity *ent);
	static void ScanForLightsOnMap(void);
	static uint8 LightForPeds(void);
	static bool ShouldCarStopForLight(CVehicle*, bool);
	static bool ShouldCarStopForBridge(CVehicle*);
};
