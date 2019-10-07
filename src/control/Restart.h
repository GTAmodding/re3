#pragma once

class CRestart
{
public:
	static void AddPoliceRestartPoint(const CVector&, float);
	static void AddHospitalRestartPoint(const CVector&, float);
	static void OverrideNextRestart(const CVector&, float);

	static void FindClosestHospitalRestartPoint(const CVector &, CVector *, float *);
	static void FindClosestPoliceRestartPoint(const CVector &, CVector *, float *);

	static bool &OverrideHospitalLevel;
	static bool &OverridePoliceStationLevel;
	static bool &bFadeInAfterNextArrest;
	static bool &bFadeInAfterNextDeath;
};
