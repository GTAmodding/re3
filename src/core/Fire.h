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
	CFire m_aFires[NUM_FIRES];
public:
	void StartFire(CEntity *entityOnFire, CEntity *culprit, float, uint32);
	void Update(void);
	CFire *FindFurthestFire_NeverMindFireMen(CVector coors, float, float);
	CFire *FindNearestFire(CVector, float*);
	uint32 GetTotalActiveFires() const { return m_nTotalFires; }
};
extern CFireManager &gFireManager;
