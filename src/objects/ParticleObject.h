#pragma once

#define MAX_PARTICLEOBJECTS 70
#define MAX_AUDIOHYDRANTS   8

enum eParticleObjectType
{
	POBJECT_PAVEMENT_STEAM = 0,
	POBJECT_PAVEMENT_STEAM_SLOWMOTION,
	POBJECT_WALL_STEAM,
	POBJECT_WALL_STEAM_SLOWMOTION,
	POBJECT_DARK_SMOKE,
	POBJECT_FIRE_HYDRANT,
	POBJECT_CAR_WATER_SPLASH,
	POBJECT_PED_WATER_SPLASH,
	POBJECT_SPLASHES_AROUND,
	POBJECT_SMALL_FIRE,
	POBJECT_BIG_FIRE,
	POBJECT_DRY_ICE,
	POBJECT_DRY_ICE_SLOWMOTION,
	POBJECT_WATER_FOUNTAIN_VERT,
	POBJECT_WATER_FOUNTAIN_HORIZ,
	POBJECT_FIRE_TRAIL,
	POBJECT_SMOKE_TRAIL,
	POBJECT_FIREBALL_AND_SMOKE,
	POBJECT_ROCKET_TRAIL,
	POBJECT_EXPLOSION_ONCE,
	POBJECT_CATALINAS_GUNFLASH,
	POBJECT_CATALINAS_SHOTGUNFLASH,
};

enum eParticleObjectState
{
	POBJECTSTATE_INITIALISED = 0,
	POBJECTSTATE_UPDATE_CLOSE,
	POBJECTSTATE_UPDATE_FAR,
	POBJECTSTATE_FREE,
};

class CParticle;

class CParticleObject : public CPlaceable
{
public:
	CParticleObject    *m_pNext;
	CParticleObject    *m_pPrev;
	CParticle          *m_pParticle;
	uint32               m_nRemoveTimer;
	eParticleObjectType m_Type;
	tParticleType       m_ParticleType;
	uint8               m_nNumEffectCycles;
	uint8               m_nSkipFrames;
	uint16              m_nFrameCounter;
	uint16              m_nState;
	CVector             m_vecTarget;
	float               m_fRandVal;
	float               m_fSize;
	CRGBA               m_Color;
	uint8               m_bRemove;
	int8                m_nCreationChance;
	
	static CParticleObject *pCloseListHead;
	static CParticleObject *pFarListHead;
	static CParticleObject *pUnusedListHead;
	
	CParticleObject();
	~CParticleObject();
	
	static void Initialise(void);

	static CParticleObject *AddObject(uint16        type, CVector const &pos,                                                                                                                     uint8 remove);
	static CParticleObject *AddObject(uint16        type, CVector const &pos,                        float size,                                                                                  uint8 remove);
	static CParticleObject *AddObject(uint16        type, CVector const &pos, CVector const &target, float size,                                                                                  uint8 remove);	
	static CParticleObject *AddObject(uint16        type, CVector const &pos, CVector const &target, float size, uint32 lifeTime, RwRGBA const &color,                                            uint8 remove);
	static CParticleObject *AddObject(tParticleType type, CVector const &pos, CVector const &target, float size, uint32 lifeTime, uint8 numEffectCycles, uint8 skipFrames, uint16 creationChance, uint8 remove);

	void RemoveObject(void);
	
	static void UpdateAll(void);
	void UpdateClose(void);
	void UpdateFar(void);
	
	static bool SaveParticle(uint8 *buffer, uint32 *length);
	static bool LoadParticle(uint8 *buffer, uint32  length);
	
	static void RemoveAllExpireableParticleObjects(void);
	static void RemoveAllParticleObjects(void);
	static void MoveToList(CParticleObject **from, CParticleObject **to, CParticleObject *obj);
};

extern CParticleObject gPObjectArray[MAX_PARTICLEOBJECTS];

class CAudioHydrant
{
public:
	int32 AudioEntity;
	CParticleObject *pParticleObject;
	
	CAudioHydrant();
	
	static bool Add   (CParticleObject *particleobject);
	static void Remove(CParticleObject *particleobject);

	static CAudioHydrant *Get(int n);	// for neo screen droplets
};