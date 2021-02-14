#pragma once

class CEntity;
class CPed;

enum eEventType
{
	EVENT_NULL,
	EVENT_ASSAULT,
	EVENT_RUN_REDLIGHT,
	EVENT_ASSAULT_POLICE,
	EVENT_GUNSHOT,
	EVENT_INJURED_PED,
	EVENT_DEAD_PED,
	EVENT_FIRE,
	EVENT_STEAL_CAR,
	EVENT_HIT_AND_RUN,
	EVENT_HIT_AND_RUN_COP,
	EVENT_SHOOT_PED,
	EVENT_SHOOT_COP,
	EVENT_EXPLOSION,
	EVENT_PED_SET_ON_FIRE,
	EVENT_COP_SET_ON_FIRE,
	EVENT_CAR_SET_ON_FIRE,
	EVENT_ASSAULT_NASTYWEAPON, // not sure
	EVENT_ICECREAM,
	EVENT_ATM,
	EVENT_SHOPSTALL, // used on graffitis
	EVENT_LAST_EVENT
};

enum eEventEntity
{
	EVENT_ENTITY_NONE,
	EVENT_ENTITY_PED,
	EVENT_ENTITY_VEHICLE,
	EVENT_ENTITY_OBJECT
};

struct CEvent
{
	eEventType type;
	eEventEntity entityType;
	int32 entityRef;
	CPed *criminal;
	CVector posn;
	uint32 timeout;
	int32 state;
};

class CEventList
{
	static int32 ms_nFirstFreeSlotIndex;
public:
	static void Initialise(void);
	static void Update(void);
	static void RegisterEvent(eEventType type, eEventEntity entityType, CEntity *ent, CPed *criminal, int32 timeout);
	static void RegisterEvent(eEventType type, CVector posn, int32 timeout);
	static bool GetEvent(eEventType type, int32 *event);
	static void ClearEvent(int32 event);
	static bool FindClosestEvent(eEventType type, CVector posn, int32 *event);
	static void ReportCrimeForEvent(eEventType type, intptr, bool);
};

extern CEvent gaEvent[NUMEVENTS];