#include "common.h"
#include "patcher.h"
#include "Pools.h"
#include "ModelIndices.h"
#include "World.h"
#include "Wanted.h"
#include "EventList.h"
#include "Messages.h"
#include "Text.h"
#include "main.h"

int32 CEventList::ms_nFirstFreeSlotIndex;
CEvent gaEvent[NUMEVENTS];
//CEvent *gaEvent = (CEvent*)0x6EF830;

enum
{
	EVENT_STATE_0,
	EVENT_STATE_CANDELETE,
	EVENT_STATE_CLEAR,
};

void
CEventList::Initialise(void)
{
	int i;

	debug("Initialising CEventList...");
	for(i = 0; i < NUMEVENTS; i++){
		gaEvent[i].type = EVENT_NULL;
		gaEvent[i].entityType = EVENT_ENTITY_NONE;
		gaEvent[i].entityRef = 0;
		gaEvent[i].posn.x = 0.0f;
		gaEvent[i].posn.y = 0.0f;
		gaEvent[i].posn.z = 0.0f;
		gaEvent[i].timeout = 0;
		gaEvent[i].state = EVENT_STATE_0;
	}
	ms_nFirstFreeSlotIndex = 0;
}

void
CEventList::Update(void)
{
	int i;

	ms_nFirstFreeSlotIndex = 0;
	for(i = 0; i < NUMEVENTS; i++){
		if(gaEvent[i].type == EVENT_NULL)
			continue;
		if(CTimer::GetTimeInMilliseconds() > gaEvent[i].timeout || gaEvent[i].state == EVENT_STATE_CANDELETE){
			gaEvent[i].type = EVENT_NULL;
			gaEvent[i].state = EVENT_STATE_0;
		}
		if(gaEvent[i].state == EVENT_STATE_CLEAR)
			gaEvent[i].state = EVENT_STATE_CANDELETE;
	}
}

void
CEventList::RegisterEvent(eEventType type, eEventEntity entityType, CEntity *ent, CPed *criminal, int32 timeout)
{
	int i;
	int ref;
	bool copsDontCare;

	copsDontCare = false;
	switch(entityType){
	case EVENT_ENTITY_PED:
		ref = CPools::GetPedRef((CPed*)ent);
		if(ent->GetModelIndex() >= MI_GANG01 && ent->GetModelIndex() <= MI_CRIMINAL02)
			copsDontCare = true;
		break;
	case EVENT_ENTITY_VEHICLE:
		ref = CPools::GetVehicleRef((CVehicle*)ent);
		break;
	case EVENT_ENTITY_OBJECT:
		ref = CPools::GetObjectRef((CObject*)ent);
		break;
	default:
		Error("Undefined entity type, RegisterEvent, EventList.cpp");
		ref = 0;
		break;
	}

	// only update time if event exists already
	for(i = 0; i < NUMEVENTS; i++)
		if(gaEvent[i].type == type &&
		   gaEvent[i].entityType == entityType &&
		   gaEvent[i].entityRef == ref){
			gaEvent[i].timeout = CTimer::GetTimeInMilliseconds() + timeout;
			return;
		}

	for(i = ms_nFirstFreeSlotIndex; i < NUMEVENTS; i++)
		if(gaEvent[i].type == EVENT_NULL){
			ms_nFirstFreeSlotIndex = i;
			break;
		}
	if(i < NUMEVENTS){
		gaEvent[i].type = type;
		gaEvent[i].entityType = entityType;
		gaEvent[i].timeout = CTimer::GetTimeInMilliseconds() + timeout;
		gaEvent[i].entityRef = ref;
		gaEvent[i].posn = ent->GetPosition();
		gaEvent[i].criminal = criminal;
		if(gaEvent[i].criminal)
			gaEvent[i].criminal->RegisterReference((CEntity**)&gaEvent[i].criminal);
		if(type == EVENT_GUNSHOT)
			gaEvent[i].state = EVENT_STATE_CLEAR;
		else
			gaEvent[i].state = EVENT_STATE_0;
	}

	if(criminal == FindPlayerPed())
		ReportCrimeForEvent(type, (uintptr)ent, copsDontCare);
}

void
CEventList::RegisterEvent(eEventType type, CVector posn, int32 timeout)
{
	int i;

	// only update time if event exists already
	for(i = 0; i < NUMEVENTS; i++)
		if(gaEvent[i].type == type &&
		   gaEvent[i].posn.x == posn.x &&
		   gaEvent[i].posn.y == posn.y &&
		   gaEvent[i].posn.z == posn.z &&
		   gaEvent[i].entityType == EVENT_ENTITY_NONE){
			gaEvent[i].timeout = CTimer::GetTimeInMilliseconds() + timeout;
			return;
		}

	for(i = ms_nFirstFreeSlotIndex; i < NUMEVENTS; i++)
		if(gaEvent[i].type == EVENT_NULL){
			ms_nFirstFreeSlotIndex = i;
			break;
		}
	if(i < NUMEVENTS){
		gaEvent[i].type = type;
		gaEvent[i].entityType = EVENT_ENTITY_NONE;
		gaEvent[i].timeout = CTimer::GetTimeInMilliseconds() + timeout;
		gaEvent[i].posn = posn;
		gaEvent[i].entityRef = 0;
		if(type == EVENT_GUNSHOT)
			gaEvent[i].state = EVENT_STATE_CLEAR;
		else
			gaEvent[i].state = EVENT_STATE_0;
	}
}

bool
CEventList::GetEvent(eEventType type, int32 *event)
{
	int i;
	for(i = 0; i < NUMEVENTS; i++)
		if(gaEvent[i].type == type){
			*event = i;
			return true;
		}
	return false;
}

void
CEventList::ClearEvent(int32 event)
{
	if(gaEvent[event].state != EVENT_STATE_CANDELETE)
		gaEvent[event].state = EVENT_STATE_CLEAR;
}

bool
CEventList::FindClosestEvent(eEventType type, CVector posn, int32 *event)
{
	int i;
	float dist;
	bool found = false;
	float mindist = 60.0f;

	for(i = 0; i < NUMEVENTS; i++){
		if(gaEvent[i].type != type)
			continue;
		dist = (posn - gaEvent[i].posn).Magnitude();
		if(dist < mindist){
			mindist = dist;
			found = true;
			*event = i;
		}
	}
	return found;
}

void
CEventList::ReportCrimeForEvent(eEventType type, int32 crimeId, bool copsDontCare)
{
	eCrimeType crime;
	switch(type){
	case EVENT_ASSAULT: crime = CRIME_HIT_PED; break;
	case EVENT_RUN_REDLIGHT: crime = CRIME_RUN_REDLIGHT; break;
	case EVENT_ASSAULT_POLICE: crime = CRIME_HIT_COP; break;
	case EVENT_GUNSHOT: crime = CRIME_POSSESSION_GUN; break;
	case EVENT_STEAL_CAR: crime = CRIME_STEAL_CAR; break;
	case EVENT_HIT_AND_RUN: crime = CRIME_RUNOVER_PED; break;
	case EVENT_HIT_AND_RUN_COP: crime = CRIME_RUNOVER_COP; break;
	case EVENT_SHOOT_PED: crime = CRIME_SHOOT_PED; break;
	case EVENT_SHOOT_COP: crime = CRIME_SHOOT_COP; break;
	case EVENT_PED_SET_ON_FIRE: crime = CRIME_PED_BURNED; break;
	case EVENT_COP_SET_ON_FIRE: crime = CRIME_COP_BURNED; break;
	case EVENT_CAR_SET_ON_FIRE: crime = CRIME_VEHICLE_BURNED; break;
	default: crime = CRIME_NONE; break;
	}

#ifdef VC_PED_PORTS
	if (crime == CRIME_HIT_PED && ((CPed*)crimeId)->IsPointerValid() &&
		FindPlayerPed()->m_pWanted->m_nWantedLevel == 0 && ((CPed*)crimeId)->m_ped_flagE2) {

		if(!((CPed*)crimeId)->DyingOrDead()) {
			sprintf(gString, "$50 Good Citizen Bonus!");
			AsciiToUnicode(gString, gUString);
			CMessages::AddBigMessage(gUString, 5000, 0);
			CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 50;
		}
	} else
#endif
		if(crime == CRIME_NONE)
			return;

	CVector playerPedCoors = FindPlayerPed()->GetPosition();
	CVector playerCoors = FindPlayerCoors();

	if(CWanted::WorkOutPolicePresence(playerCoors, 14.0f) != 0){
		FindPlayerPed()->m_pWanted->RegisterCrime_Immediately(crime, playerPedCoors, crimeId, copsDontCare);
		FindPlayerPed()->m_pWanted->SetWantedLevelNoDrop(1);
	}else
		FindPlayerPed()->m_pWanted->RegisterCrime(crime, playerPedCoors, crimeId, copsDontCare);

	if(type == EVENT_ASSAULT_POLICE)
		FindPlayerPed()->SetWantedLevelNoDrop(1);
	if(type == EVENT_SHOOT_COP)
		FindPlayerPed()->SetWantedLevelNoDrop(2);

}

STARTPATCHES
	InjectHook(0x475B60, CEventList::Initialise, PATCH_JUMP);
	InjectHook(0x475BE0, CEventList::Update, PATCH_JUMP);
	InjectHook(0x475C50, (void (*)(eEventType,eEventEntity,CEntity *,CPed *,int32))CEventList::RegisterEvent, PATCH_JUMP);
	InjectHook(0x475E10, (void (*)(eEventType,CVector,int32))CEventList::RegisterEvent, PATCH_JUMP);
	InjectHook(0x475F40, CEventList::GetEvent, PATCH_JUMP);
	InjectHook(0x475F70, CEventList::ClearEvent, PATCH_JUMP);
	InjectHook(0x475F90, CEventList::FindClosestEvent, PATCH_JUMP);
	InjectHook(0x476070, CEventList::ReportCrimeForEvent, PATCH_JUMP);
ENDPATCHES
