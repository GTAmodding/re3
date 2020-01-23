#pragma once

#include "Vehicle.h"

class CBoat : public CVehicle
{
public:
	// 0x288
	float field_288;
	float field_28C;
	float field_290;
	float field_294;
	float field_298;
	float field_29C;
	float field_2A0;
	float field_2A4;
	float m_fMovingHiRotation;
	int32 _unk0;
	RwFrame *m_aBoatNodes[4];
	uint8 m_bBoatFlag1 : 1;
	uint8 m_bBoatFlag2 : 1;
	uint8 m_bBoatFlag3 : 1;
	uint8 m_bBoatFlag4 : 1;
	uint8 m_bBoatFlag5 : 1;
	uint8 m_bBoatFlag6 : 1;
	uint8 m_bBoatFlag7 : 1;
	uint8 m_bBoatFlag8 : 1;
	bool m_bIsAnchored;
	char _pad0[2];
	float field_2C4;
	int32 _unk1;
	float field_2CC;
	CEntity *field_2D0;
	bool _unk2;
	char _pad1[3];
	float m_fAccelerate;
	float m_fBrake;
	float m_fSteeringLeftRight;
	uint8 m_nPadID;
	char _pad2[3];
	int32 _unk3;
	float m_fTurnForceZ;
	CVector m_vecMoveForce;
	float field_2FC;
	uint16 field_300;
	uint16 m_nNumWakePoints;
	CVector2D m_avec2dWakePoints[32];
	float m_afWakePointLifeTime[32];

	CBoat(int, uint8);

	virtual void SetModelIndex(uint32 id);
	virtual void ProcessControl();
	virtual void Teleport(CVector v);
	virtual void PreRender(void) {};
	virtual void Render(void);
	virtual void ProcessControlInputs(uint8);
	virtual void GetComponentWorldPosition(int32 component, CVector &pos);
	virtual bool IsComponentPresent(int32 component) { return true; }
	virtual void BlowUpCar(CEntity *ent);
	
	void SetupModelNodes();

	static CBoat *(&apFrameWakeGeneratingBoats)[4];
	
	static bool IsSectorAffectedByWake(CVector2D sector, float fSize, CBoat **apBoats);
	static float IsVertexAffectedByWake(CVector vecVertex, CBoat *pBoat);
	static void FillBoatList(void);

};
static_assert(sizeof(CBoat) == 0x484, "CBoat: error");

extern float MAX_WAKE_LENGTH;
extern float MIN_WAKE_INTERVAL;
extern float WAKE_LIFETIME;