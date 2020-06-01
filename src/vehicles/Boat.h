#pragma once

#include "Vehicle.h"

enum eBoatNodes
{
	BOAT_MOVING = 1,
	BOAT_WINDSCREEN,
	BOAT_RUDDER,
	BOAT_FLAP_LEFT,
	BOAT_FLAP_RIGHT,
	BOAT_REARFLAP_LEFT,
	BOAT_REARFLAP_RIGHT
};

class CBoat : public CVehicle
{
public:
	// 0x288
	float m_fPropellerZ;
	float m_fPropellerY;
	CVector m_waterMoveDrag;
	CVector m_waterTurnDrag;
	float m_fMovingHiRotation;
	int32 _unk0;
	RwFrame *m_aBoatNodes[4];
	uint8 bBoatInWater : 1;
	uint8 bPropellerInWater : 1;
	bool m_bIsAnchored;
	float m_fOrientation;
	int32 _unk1;
	float m_fDamage;
	CEntity *m_pSetOnFireEntity;
	bool _unk2;
	float m_fAccelerate;
	float m_fBrake;
	float m_fSteeringLeftRight;
	uint8 m_nPadID;
	int32 _unk3;
	float m_fVolumeUnderWater;
	CVector m_vecBuoyancePoint;
	float m_fPrevVolumeUnderWater;
	int16 m_nDeltaVolumeUnderWater;
	uint16 m_nNumWakePoints;
	CVector2D m_avec2dWakePoints[32];
	float m_afWakePointLifeTime[32];

	static float MAX_WAKE_LENGTH;
	static float MIN_WAKE_INTERVAL;
	static float WAKE_LIFETIME;

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
	
	void ApplyWaterResistance(void);
	void SetupModelNodes();
	void PruneWakeTrail(void);
	void AddWakePoint(CVector point);
	void DoDriveByShootings(void);

	static CBoat *apFrameWakeGeneratingBoats[4];
	
	static bool IsSectorAffectedByWake(CVector2D sector, float fSize, CBoat **apBoats);
	static float IsVertexAffectedByWake(CVector vecVertex, CBoat *pBoat);
	static void FillBoatList(void);

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif
	static const uint32 nSaveStructSize;

};