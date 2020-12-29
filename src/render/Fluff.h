#pragma once

#include "Vector.h"
#include "Object.h"
#include "Plane.h"

enum {
	SCRIPT_PATH_DISABLED = 0,
	SCRIPT_PATH_INITIALIZED,
	SCRIPT_PATH_ACTIVE
};

class CScriptPath
{
public:
	int32 m_numNodes;
	CPlaneNode *m_pNode;
	float m_fTotalLength;
	float m_fSpeed;
	float m_fPosition;
	float m_fObjectLength;
	int32 m_state;
	CObject *m_pObjects[6];

	void Clear(void);
	void Update(void);
	void InitialiseOne(int32 numNodes, float length);
	void FindCoorsFromDistanceOnPath(float t, float *pX, float *pY, float *pZ);
	void SetObjectToControl(CObject *pObj);
};

class CScriptPaths
{
public:
	static CScriptPath aArray[3];
	static void Init(void);
	static void Shutdown(void);
	static void Update(void);
	static bool IsOneActive(void);
	static void Save(uint8 *buf, uint32 *size);
	static void Load(uint8 *buf, uint32 size);
	static void Save_ForReplay();
	static void Load_ForReplay();
};

class CPlaneTrail
{
	CVector m_pos[16];
	int32 m_time[16];
public:
	void Init(void);
	void Render(float visibility);
	void RegisterPoint(CVector pos);
};

class CPlaneTrails
{
	static CPlaneTrail aArray[6];	// NB: 3 CPlanes and 3 hardcoded far away ones
public:
	static void Init(void);
	static void Update(void);
	static void Render(void);
	static void RegisterPoint(CVector pos, uint32 id);
};

class CPlaneBanner
{
	CVector m_pos[8];
public:
	void Init(void);
	void Update(void);
	void Render(void);
	void RegisterPoint(CVector pos);
};

class CPlaneBanners
{
	static CPlaneBanner aArray[5];
public:
	static void Init(void);
	static void Update(void);
	static void Render(void);
	static void RegisterPoint(CVector pos, uint32 id);
};

class CEscalator
{
	CVector m_pos0;
	CVector m_pos1;
	CVector m_pos2;
	CVector m_pos3;
	CMatrix m_matrix;
	bool m_bIsActive;
	bool m_bIsMovingDown;
	int32 m_stepsCount;
	float m_lowerEnd; 
	float m_upperEnd; 
	CVector m_midPoint;
	float m_radius;
	CObject *m_pSteps[24];
public:
	CEscalator();
	void Update(void);
	void SwitchOff(void);
	void AddThisOne(CVector pos0, CVector pos1, CVector pos2, CVector pos3, bool b_isMovingDown);
	bool IsActive() const { return m_bIsActive; };
	const CVector& GetPosition() const { return m_midPoint; };
};

class CEscalators
{
	static CEscalator aEscalators[NUM_ESCALATORS];
public:
	static int32 NumEscalators;
	static void Init(void);
	static void Update(void);
	static void AddOne(CVector pos0, CVector pos1, CVector pos2, CVector pos3, bool b_isMovingDown);
	static void Shutdown(void);
	static const CEscalator& GetEscalator(int ind) { return aEscalators[ind]; };
};

class CMovingThing
{
public:
	CMovingThing *m_pNext;
	CMovingThing *m_pPrev;
	int16 m_nType;
	int16 m_farAway;
	CVector m_vecPosn;
	CEntity* m_pEntity;

	void Update();
	void AddToList(CMovingThing *pThing);
	void RemoveFromList();
	int16 SizeList();
};

#define NUMMOVINGTHINGS 48

class CMovingThings
{
public:
	static CMovingThing StartCloseList;
	static CMovingThing EndCloseList;
	static int16 Num;
	static CMovingThing aMovingThings[NUMMOVINGTHINGS];

	static void Init();
	static void Shutdown();
	static void Update();
	static void Render();
	static void PossiblyAddThisEntity(CEntity *pEnt);
	static void RegisterOne(CEntity *pEnt, uint16 nType);
};

class CScrollBar
{
private:
	uint8       m_Counter;
	const char* m_pMessage;
	CVector     m_Position;
	uint32      m_MessageCurrentChar;
	uint32      m_MessageLength;
	CVector     m_Size;
	float       m_fIntensity;
	uint8       m_MessageBar[40];
	uint8       m_Type;
	bool        m_bVisible;
	uint8       m_uRed;
	uint8       m_uGreen;
	uint8       m_uBlue;
	float       m_fScale;

public:
	static int TonightsEvent;

public:
	void SetVisibility(bool visible) { m_bVisible = visible; }
	bool IsVisible() { return m_bVisible; }

	void Init(CVector pos1, CVector pos2, uint8 type, uint8 red, uint8 green, uint8 blue, float scale);
	void Update();
	void Render();
};

class CSmokeTrail {
	CVector m_pos[16];
	float m_opacity[16];
	int m_time[16];
	char m_unused[536];
	int m_seed;
public:
	void Render(void);
	void RegisterPoint(CVector position, float a);
	void Init(int num);
};

class CSmokeTrails {
	static CSmokeTrail aSmoke[3];
public:
	static bool CigOn;
	static void Update(void);
	static void Render(void);
	static void Init(void);
};