#pragma once

class CObject;

enum eHeliNodes
{
	HELI_CHASSIS = 1,
	HELI_TOPROTOR,
	HELI_BACKROTOR,
	HELI_TAIL,
	HELI_TOPKNOT,
	HELI_SKID_LEFT,
	HELI_SKID_RIGHT,
	NUM_HELI_NODES
};

enum
{
	HELI_RANDOM0,
	HELI_RANDOM1,
	HELI_SCRIPT,
	HELI_CATALINA,	// TODO 2 in VC
	NUM_HELIS
};

enum
{
	HELI_TYPE_RANDOM,
	HELI_TYPE_SCRIPT,
	HELI_TYPE_CATALINA,
};


class CHeli : public CVehicle
{
public:
	RwFrame *m_aHeliNodes[NUM_HELI_NODES];
	int8 m_heliStatus;
	float m_fSearchLightX;
	float m_fSearchLightY;
	uint32 m_nExplosionTimer;
	float m_fRotation;
	float m_fAngularSpeed;
	float m_fTargetZ;
	float m_fSearchLightIntensity;
	int8 m_nHeliId;
	int8 m_heliType;
	int8 m_pathState;
	int8 m_numSwat;
	uint8 m_aSwatState[4];
	float m_aSearchLightHistoryX[6];
	float m_aSearchLightHistoryY[6];
	uint32 m_nSearchLightTimer;
	uint32 m_nShootTimer;
	uint32 m_nLastShotTime;
	uint32 m_nBulletDamage;
	float m_fRotorRotation;
	float m_fHeliDustZ[8];
	uint32 m_nPoliceShoutTimer;
	float m_fTargetOffset;
	bool m_bTestRight;

	static CHeli *pHelis[NUM_HELIS];
	static int16 NumRandomHelis;
	static uint32 TestForNewRandomHelisTimer;
	static bool CatalinaHeliOn;
	static bool CatalinaHasBeenShotDown;
	static bool ScriptHeliOn;

	CHeli(int32 id, uint8 CreatedBy);

	// from CEntity
	void SetModelIndex(uint32 id);
	void ProcessControl(void);
	void PreRender(void);
	void Render(void);

	void PreRenderAlways(void);
	CObject *SpawnFlyingComponent(int32 component);
	CVector FindSwatPositionRelativeToHeli(int n);
	bool SendDownSwat(void);

	static void InitHelis(void);
	static void UpdateHelis(void);
	static void SpecialHeliPreRender(void);
	static bool TestRocketCollision(CVector *coors);
	static bool TestBulletCollision(CVector *line0, CVector *line1, CVector *bulletPos, int32 damage);
	static bool TestSniperCollision(CVector *line0, CVector *line1);

	static void StartCatalinaFlyBy(void);
	static void RemoveCatalinaHeli(void);
	static CHeli *FindPointerToCatalinasHeli(void);
	static void CatalinaTakeOff(void);
	static void MakeCatalinaHeliFlyAway(void);
	static bool HasCatalinaBeenShotDown(void);

	static void ActivateHeli(bool activate);
};
