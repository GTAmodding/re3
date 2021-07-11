#pragma once

#include "Ped.h"

class CAccident;
class CFire;

enum EmergencyPedState
{
	EMERGENCY_PED_READY = 0x0,
	EMERGENCY_PED_DETERMINE_NEXT_STATE = 0x1, // you can set that anytime you want
	EMERGENCY_PED_START_CPR = 0x2,
	EMERGENCY_PED_FLAG_4 = 0x4, // unused
	EMERGENCY_PED_FLAG_8 = 0x8, // unused
	EMERGENCY_PED_FACE_TO_PATIENT = 0x10, // for CPR
	EMERGENCY_PED_PERFORM_CPR = 0x20,
	EMERGENCY_PED_STOP_CPR = 0x40,
	EMERGENCY_PED_STAND_STILL = 0x80, // waiting colleagues for medics, "extinguishing" fire for firemen
	EMERGENCY_PED_STOP = 0x100,
};

class CEmergencyPed : public CPed
{
public:
	CPed *m_pRevivedPed;
	EmergencyPedState m_nEmergencyPedState;
	CAccident *m_pAttendedAccident;
	CFire *m_pAttendedFire;
	bool m_bStartedToCPR; // set but unused
	int32 field_1360; // set to 0 but unused

	CEmergencyPed(uint32);
	~CEmergencyPed() { }
	bool InRange(CPed*);
	void ProcessControl(void);
	void FiremanAI(void);
	void MedicAI(void);
};
//VALIDATE_SIZE(CEmergencyPed, 0x554);
