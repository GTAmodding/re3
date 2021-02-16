#pragma once
#include "Sprite2d.h"
#include "Draw.h"

enum eBlipType
{
	BLIP_NONE,
	BLIP_CAR,
	BLIP_CHAR,
	BLIP_OBJECT,
	BLIP_COORD,
	BLIP_CONTACT_POINT
};

enum eBlipDisplay
{
	BLIP_DISPLAY_NEITHER = 0,
	BLIP_DISPLAY_MARKER_ONLY = 1,
	BLIP_DISPLAY_BLIP_ONLY = 2,
	BLIP_DISPLAY_BOTH = 3,
};

enum eRadarSprite
{
#ifdef MENU_MAP
	RADAR_SPRITE_ENTITY_BLIP = -2,
	RADAR_SPRITE_COORD_BLIP = -1,
#endif
	RADAR_SPRITE_NONE = 0,
	RADAR_SPRITE_ASUKA,
	RADAR_SPRITE_BOMB,
	RADAR_SPRITE_CAT,
	RADAR_SPRITE_CENTRE,
	RADAR_SPRITE_COPCAR,
	RADAR_SPRITE_DON,
	RADAR_SPRITE_EIGHT,
	RADAR_SPRITE_EL,
	RADAR_SPRITE_ICE,
	RADAR_SPRITE_JOEY,
	RADAR_SPRITE_KENJI,
	RADAR_SPRITE_LIZ,
	RADAR_SPRITE_LUIGI,
	RADAR_SPRITE_NORTH,
	RADAR_SPRITE_RAY,
	RADAR_SPRITE_SAL,
	RADAR_SPRITE_SAVE,
	RADAR_SPRITE_SPRAY,
	RADAR_SPRITE_TONY,
	RADAR_SPRITE_WEAPON,
	RADAR_SPRITE_COUNT
};

enum
{
	RADAR_TRACE_RED,
	RADAR_TRACE_GREEN,
	RADAR_TRACE_LIGHT_BLUE,
	RADAR_TRACE_GRAY,
	RADAR_TRACE_YELLOW,
	RADAR_TRACE_MAGENTA,
	RADAR_TRACE_CYAN
};

enum
{
	BLIP_MODE_TRIANGULAR_UP = 0,
	BLIP_MODE_TRIANGULAR_DOWN,
	BLIP_MODE_SQUARE,
};

struct sRadarTrace
{
	uint32 m_nColor;
	uint32 m_eBlipType; // eBlipType
	int32 m_nEntityHandle;
	CVector2D m_vec2DPos;
	CVector m_vecPos;
	uint16 m_BlipIndex;
	bool m_bDim;
	bool m_bInUse;
	float m_Radius;
	int16 m_wScale;
	uint16 m_eBlipDisplay; // eBlipDisplay
	uint16 m_eRadarSprite; // eRadarSprite
};
VALIDATE_SIZE(sRadarTrace, 0x30);

// Values for screen space
#define RADAR_LEFT (40.0f)
#ifdef PS2_HUD
#define RADAR_BOTTOM (44.0f)
#else
#define RADAR_BOTTOM (47.0f)
#endif

#ifdef FIX_RADAR
/*
	The values are from an early screenshot taken before R* broke radar
		#define RADAR_WIDTH (82.0f)
		#define RADAR_HEIGHT (82.0f)
*/
#define RADAR_WIDTH ((CDraw::ms_bFixRadar)  ? (82.0f) : (94.0f))	
#define RADAR_HEIGHT ((CDraw::ms_bFixRadar) ? (82.0f) : (76.0f))
#else
/*
	broken since forever, someone tried to fix size for 640x512(PAL)
		http://aap.rockstarvision.com/pics/gta3/ps2screens/gta3_interface.jpg
	but failed:
		http://aap.rockstarvision.com/pics/gta3/artwork/gta3_artwork_16.jpg
	most likely the guy used something like this:
		int y = 82 * (640.0/512.0)/(640.0/480.0);
		int x = y * (640.0/512.0);
*/
#define RADAR_WIDTH (94.0f)
#define RADAR_HEIGHT (76.0f)
#endif

class CRadar
{
public:
	static float m_radarRange;
	static sRadarTrace ms_RadarTrace[NUMRADARBLIPS];
	static CSprite2d AsukaSprite;
	static CSprite2d BombSprite;
	static CSprite2d CatSprite;
	static CSprite2d CentreSprite;
	static CSprite2d CopcarSprite;
	static CSprite2d DonSprite;
	static CSprite2d EightSprite;
	static CSprite2d ElSprite;
	static CSprite2d IceSprite;
	static CSprite2d JoeySprite;
	static CSprite2d KenjiSprite;
	static CSprite2d LizSprite;
	static CSprite2d LuigiSprite;
	static CSprite2d NorthSprite;
	static CSprite2d RaySprite;
	static CSprite2d SalSprite;
	static CSprite2d SaveSprite;
	static CSprite2d SpraySprite;
	static CSprite2d TonySprite;
	static CSprite2d WeaponSprite;
	static CSprite2d *RadarSprites[RADAR_SPRITE_COUNT];
	static float cachedCos;
	static float cachedSin;
#ifdef MENU_MAP
	static int TargetMarkerId;
	static CVector TargetMarkerPos;

	static void InitFrontEndMap();
	static void DrawYouAreHereSprite(float, float);
	static void ToggleTargetMarker(float, float);
#endif
	static uint8 CalculateBlipAlpha(float dist);
	static void ChangeBlipBrightness(int32 i, int32 bright);
	static void ChangeBlipColour(int32 i, int32);
	static void ChangeBlipDisplay(int32 i, eBlipDisplay display);
	static void ChangeBlipScale(int32 i, int32 scale);
	static void ClearBlip(int32 i);
	static void ClearBlipForEntity(eBlipType type, int32 id);
	static int ClipRadarPoly(CVector2D *out, const CVector2D *in);
	static bool DisplayThisBlip(int32 i);
	static void Draw3dMarkers();
	static void DrawBlips();
	static void DrawMap();
	static void DrawRadarMap();
	static void DrawRadarMask();
	static void DrawRadarSection(int32 x, int32 y);
	static void DrawRadarSprite(uint16 sprite, float x, float y, uint8 alpha);
	static void DrawRotatingRadarSprite(CSprite2d* sprite, float x, float y, float angle, int32 alpha);
	static int32 GetActualBlipArrayIndex(int32 i);
	static int32 GetNewUniqueBlipIndex(int32 i);
	static uint32 GetRadarTraceColour(uint32 color, bool bright);
	static void Initialise();
	static float LimitRadarPoint(CVector2D &point);
	static void LoadAllRadarBlips(uint8 *buf, uint32 size);
	static void LoadTextures();
	static void RemoveRadarSections();
	static void SaveAllRadarBlips(uint8*, uint32*);
	static void SetBlipSprite(int32 i, int32 icon);
	static int32 SetCoordBlip(eBlipType type, CVector pos, int32, eBlipDisplay);
	static int32 SetEntityBlip(eBlipType type, int32, int32, eBlipDisplay);
	static void SetRadarMarkerState(int32 i, bool flag);
	static void ShowRadarMarker(CVector pos, uint32 color, float radius);
	static void ShowRadarTrace(float x, float y, uint32 size, uint8 red, uint8 green, uint8 blue, uint8 alpha);
	static void ShowRadarTraceWithHeight(float x, float y, uint32 size, uint8 red, uint8 green, uint8 blue, uint8 alpha, uint8 mode);
	static void Shutdown();
	static void StreamRadarSections(const CVector &posn);
	static void StreamRadarSections(int32 x, int32 y);
	static void TransformRealWorldToTexCoordSpace(CVector2D &out, const CVector2D &in, int32 x, int32 y);
	static void TransformRadarPointToRealWorldSpace(CVector2D &out, const CVector2D &in);
	static void TransformRadarPointToScreenSpace(CVector2D &out, const CVector2D &in);
	static void TransformRealWorldPointToRadarSpace(CVector2D &out, const CVector2D &in);

	// no in CRadar in the game:	
	static void CalculateCachedSinCos();
};
