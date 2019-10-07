#include "common.h"
#include "patcher.h"
#include "Restart.h"

bool &CRestart::OverrideHospitalLevel = *(bool*)0x95CD4C;
bool &CRestart::OverridePoliceStationLevel = *(bool*)0x95CD50;
bool &CRestart::bFadeInAfterNextArrest = *(bool*)0x95CD69;
bool &CRestart::bFadeInAfterNextDeath = *(bool*)0x95CD9D;

WRAPPER void CRestart::AddHospitalRestartPoint(const CVector&, float) { EAXJMP(0x436100); }
WRAPPER void CRestart::AddPoliceRestartPoint(const CVector&, float) { EAXJMP(0x436150); }
WRAPPER void CRestart::OverrideNextRestart(const CVector&, float) { EAXJMP(0x4366C0); }
WRAPPER void CRestart::FindClosestHospitalRestartPoint(const CVector &, CVector *, float *) { EAXJMP(0x4361A0); }
WRAPPER void CRestart::FindClosestPoliceRestartPoint(const CVector &, CVector *, float *) { EAXJMP(0x436450); }