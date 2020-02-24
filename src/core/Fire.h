#pragma once

class CEntity;

class CFire
{
public:
	bool m_bIsOngoing;
	bool m_bIsScriptFire;
	bool m_bPropogationFlag;
	bool m_bAudioSet;
	CVector m_vecPos;
	CEntity *m_pEntity;
	CEntity *m_pSource;
	int m_nExtinguishTime;
	int m_nStartTime;
	int field_20;
	int field_24;
	uint32 m_nFiremenPuttingOut;
	float field_2C;

	void Extinguish(void);
};

class CFireManager
{
	enum {
		MAX_FIREMEN_ATTENDING = 2,
	};
	uint32 m_nTotalFires;
public:
	CFire m_aFires[NUM_FIRES];
	void StartFire(CEntity *entityOnFire, CEntity *culprit, float, uint32);
	void StartFire(CVector, float, uint8);
	void Update(void);
	CFire *FindFurthestFire_NeverMindFireMen(CVector coors, float, float);
	CFire *FindNearestFire(CVector, float*);
	uint32 GetTotalActiveFires() const;
	void ExtinguishPoint(CVector, float);
	int32 StartScriptFire(const CVector& pos, CEntity* culprit, float, uint8);
	bool IsScriptFireExtinguish(int16);
	void RemoveScriptFire(int16);
	void RemoveAllScriptFires(void);
	void SetScriptFireAudio(int16, bool);
};
extern CFireManager &gFireManager;
