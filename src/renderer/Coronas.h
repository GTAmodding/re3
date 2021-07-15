#pragma once

extern RwTexture *gpCoronaTexture[9];

struct CRegisteredCorona
{
	uint32 id;
	uint32 lastLOScheck;
	RwTexture *texture;
	uint8 red;
	uint8 green;
	uint8 blue;
	uint8 alpha;		// alpha when fully visible
	uint8 fadeAlpha;	// actual value used for rendering, faded
	CVector coors;
	float size;
	float someAngle;
	bool registeredThisFrame;
	float drawDist;
	int8 flareType;
	int8 reflection;

	uint8 LOScheck : 1;
	uint8 offScreen : 1;
	uint8 firstUpdate : 1;
	uint8 drawStreak : 1;
	uint8 sightClear : 1;

	bool renderReflection;
	float heightAboveRoad;

	float prevX[6];
	float prevY[6];
	uint8 prevRed[6];
	uint8 prevGreen[6];
	uint8 prevBlue[6];
	bool hasValue[6];

	void Update(void);
};

VALIDATE_SIZE(CRegisteredCorona, 0x80);

class CCoronas
{
	static CRegisteredCorona aCoronas[NUMCORONAS];
public:
	enum {
		SUN_CORE = 1,
		SUN_CORONA
	};
	enum {
		TYPE_STAR,
		TYPE_NORMAL,
		TYPE_MOON,
		TYPE_REFLECT,
		TYPE_HEADLIGHT,
		TYPE_HEX,
		TYPE_CIRCLE,
		TYPE_RING,
		TYPE_STREAK,
	};
	enum {
		FLARE_NONE,
		FLARE_SUN,
		FLARE_HEADLIGHTS
	};
	enum {
		REFLECTION_OFF,
		REFLECTION_ON,
	};
	enum {
		LOSCHECK_OFF,
		LOSCHECK_ON,
	};
	enum {
		STREAK_OFF,
		STREAK_ON,
	};

	static float LightsMult;
	static float SunScreenY;
	static float SunScreenX;
	static bool bSmallMoon;
	static bool SunBlockedByClouds;
	static int bChangeBrightnessImmediately;

	static void Init(void);
	static void Shutdown(void);
	static void Update(void);
	static void RegisterCorona(uint32 id, uint8 red, uint8 green, uint8 blue, uint8 alpha,
		const CVector &coors, float size, float drawDist, RwTexture *tex,
		int8 flareType, uint8 reflection, uint8 LOScheck, uint8 drawStreak, float someAngle);
	static void RegisterCorona(uint32 id, uint8 red, uint8 green, uint8 blue, uint8 alpha,
		const CVector &coors, float size, float drawDist, uint8 type,
		int8 flareType, uint8 reflection, uint8 LOScheck, uint8 drawStreak, float someAngle);
	static void UpdateCoronaCoors(uint32 id, const CVector &coors, float drawDist, float someAngle);
	static void Render(void);
	static void RenderReflections(void);
	static void DoSunAndMoon(void);
};
