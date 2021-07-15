#pragma once

class CSpecialFX
{
public:
	static void Render(void);
	static void Update(void);
	static void Init(void);
	static void Shutdown(void);
};

class CRegisteredMotionBlurStreak
{
public:
	uintptr m_id;
	uint8 m_red;
	uint8 m_green;
	uint8 m_blue;
	CVector m_pos1[3];
	CVector m_pos2[3];
	bool m_isValid[3];

	void Update(void);
	void Render(void);
};

class CMotionBlurStreaks
{
	static CRegisteredMotionBlurStreak aStreaks[NUMMBLURSTREAKS];
public:
	static void Init(void);
	static void Update(void);
	static void RegisterStreak(uintptr id, uint8 r, uint8 g, uint8 b, CVector p1, CVector p2);
	static void Render(void);
};

struct CBulletTrace
{
	CVector m_vecCurrentPos;
	CVector m_vecTargetPos;
	bool m_bInUse;
	uint8 m_framesInUse;
	uint8 m_lifeTime;

	void Update(void);
};

class CBulletTraces
{
public:
	static CBulletTrace aTraces[NUMBULLETTRACES];

	static void Init(void);
	static void AddTrace(CVector*, CVector*);
	static void Render(void);
	static void Update(void);
};

enum
{
	MARKERTYPE_0 = 0,
	MARKERTYPE_ARROW,
	MARKERTYPE_2,
	MARKERTYPE_3,
	MARKERTYPE_CYLINDER,
	NUMMARKERTYPES,

	MARKERTYPE_INVALID = 0x101
};


class C3dMarker
{
public:
	CMatrix m_Matrix;
	RpAtomic *m_pAtomic;
	RpMaterial *m_pMaterial;
	uint16 m_nType;
	bool m_bIsUsed;
	uint32 m_nIdentifier;
	RwRGBA m_Color;
	uint16 m_nPulsePeriod;
	int16 m_nRotateRate;
	uint32 m_nStartTime;
	float m_fPulseFraction;
	float m_fStdSize;
	float m_fSize;
	float m_fBrightness;
	float m_fCameraRange;

	bool AddMarker(uint32 identifier, uint16 type, float fSize, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate);
	void DeleteMarkerObject();
	void Render();
};

class C3dMarkers
{
public:
	static void Init();
	static void Shutdown();
	static C3dMarker *PlaceMarker(uint32 id, uint16 type, CVector &pos, float size, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate);
	static void PlaceMarkerSet(uint32 id, uint16 type, CVector &pos, float size, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate);
	static void Render();
	static void Update();

	static C3dMarker m_aMarkerArray[NUM3DMARKERS];
	static int32 NumActiveMarkers;
	static RpClump* m_pRpClumpArray[NUMMARKERTYPES];
};

enum
{
	BRIGHTLIGHT_INVALID,
	BRIGHTLIGHT_TRAFFIC_GREEN,
	BRIGHTLIGHT_TRAFFIC_YELLOW,
	BRIGHTLIGHT_TRAFFIC_RED,

	// white
	BRIGHTLIGHT_FRONT_LONG,
	BRIGHTLIGHT_FRONT_SMALL,
	BRIGHTLIGHT_FRONT_BIG,
	BRIGHTLIGHT_FRONT_TALL,

	// red
	BRIGHTLIGHT_REAR_LONG,
	BRIGHTLIGHT_REAR_SMALL,
	BRIGHTLIGHT_REAR_BIG,
	BRIGHTLIGHT_REAR_TALL,

	BRIGHTLIGHT_SIREN,	// unused

	BRIGHTLIGHT_FRONT = BRIGHTLIGHT_FRONT_LONG,
	BRIGHTLIGHT_REAR = BRIGHTLIGHT_REAR_LONG,
};

class CBrightLight
{
public:
	CVector m_pos;
	CVector m_up;
	CVector m_side;
	CVector m_front;
	float m_camDist;
	uint8 m_type;
	uint8 m_red;
	uint8 m_green;
	uint8 m_blue;
};

class CBrightLights
{
	static int NumBrightLights;
	static CBrightLight aBrightLights[NUMBRIGHTLIGHTS];
public:
	static void Init(void);
	static void RegisterOne(CVector pos, CVector up, CVector side, CVector front,
		uint8 type, uint8 red = 0, uint8 green = 0, uint8 blue = 0);
	static void Render(void);
	static void RenderOutGeometryBuffer(void);
};


enum
{
	SHINYTEXT_WALK = 1,
	SHINYTEXT_FLAT
};

class CShinyText
{
public:
	CVector m_verts[4];
	CVector2D m_texCoords[4];
	float m_camDist;
	uint8 m_type;
	uint8 m_red;
	uint8 m_green;
	uint8 m_blue;
};

class CShinyTexts
{
	static int NumShinyTexts;
	static CShinyText aShinyTexts[NUMSHINYTEXTS];
public:
	static void Init(void);
	static void RegisterOne(CVector p0, CVector p1, CVector p2, CVector p3,
		float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3,
		uint8 type, uint8 red, uint8 green, uint8 blue, float maxDist);
	static void Render(void);
	static void RenderOutGeometryBuffer(void);
};

class CMoneyMessage
{
	friend class CMoneyMessages;

	uint32	m_nTimeRegistered;
	CVector	m_vecPosition;
	wchar	m_aText[16];
	CRGBA	m_Colour;
	float	m_fSize;
	float	m_fOpacity;
public:
	void Render();
};

class CMoneyMessages
{
	static CMoneyMessage aMoneyMessages[NUMMONEYMESSAGES];
public:
	static void Init();
	static void Render();
	static void	RegisterOne(CVector vecPos, const char *pText, uint8 bRed, uint8 bGreen, uint8 bBlue, float fSize, float fOpacity);
};

class CSpecialParticleStuff
{
	static uint32 BoatFromStart;
public:
	static void CreateFoamAroundObject(CMatrix*, float, float, float, int32);
	static void StartBoatFoamAnimation();
	static void UpdateBoatFoamAnimation(CMatrix*);
};
