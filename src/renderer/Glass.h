#pragma once

class CEntity;

class CFallingGlassPane : public CMatrix
{
public:
	CVector m_vecMoveSpeed;
	CVector m_vecTurn;
	uint32 m_nTimer;
	float m_fGroundZ;
	float m_fStep;
	uint8 m_nTriIndex;
	bool m_bActive;
	bool m_bShattered;

	CFallingGlassPane()  { }
	~CFallingGlassPane() { }

	void Update(void);
	void Render(void);
};

VALIDATE_SIZE(CFallingGlassPane, 0x70);

enum
{
	NUM_GLASSTRIANGLES = 5,
};

class CGlass
{
	static uint32 NumGlassEntities;
	static CEntity *apEntitiesToBeRendered[NUM_GLASSENTITIES];
	static CFallingGlassPane aGlassPanes[NUM_GLASSPANES];
public:
	static void Init(void);
	static void Update(void);
	static void Render(void);
	static CFallingGlassPane *FindFreePane(void);
	static void GeneratePanesForWindow(uint32 type, CVector pos, CVector up, CVector right, CVector speed, CVector point, float moveSpeed, bool cracked, bool explosion);
	static void AskForObjectToBeRenderedInGlass(CEntity *entity);
	static void RenderEntityInGlass(CEntity *entity);
	static int32 CalcAlphaWithNormal(CVector *normal);
	static void RenderHiLightPolys(void);
	static void RenderShatteredPolys(void);
	static void RenderReflectionPolys(void);
	static void WindowRespondsToCollision(CEntity *entity, float amount, CVector speed, CVector point, bool explosion);
	static void WindowRespondsToSoftCollision(CEntity *entity, float amount);
	static void WasGlassHitByBullet(CEntity *entity, CVector point);
	static void WindowRespondsToExplosion(CEntity *entity, CVector point);
};