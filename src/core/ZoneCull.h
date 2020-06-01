class CEntity;

enum eZoneAttribs
{
	ATTRZONE_CAMCLOSEIN		= 1,
	ATTRZONE_STAIRS			= 2,
	ATTRZONE_1STPERSON		= 4,
	ATTRZONE_NORAIN			= 8,
	ATTRZONE_NOPOLICE		= 0x10,
	ATTRZONE_NOTCULLZONE		= 0x20,
	ATTRZONE_DOINEEDCOLLISION	= 0x40,
	ATTRZONE_SUBWAYVISIBLE		= 0x80,
	
	ATTRZONE_WATERFUDGE		    = 0x400,
};

struct CAttributeZone
{
	float minx;
	float maxx;
	float miny;
	float maxy;
	float minz;
	float maxz;
	int16 attributes;
	int16 wantedLevel;
};

class CCullZones
{
public:
	static int32     NumAttributeZones;
	static CAttributeZone aAttributeZones[NUMATTRIBZONES];

	static int32 CurrentWantedLevelDrop_Player;
	static int32 CurrentFlags_Camera;
	static int32 CurrentFlags_Player;
	static bool bCurrentSubwayIsInvisible;

	static void Init(void);
	static void Update(void);
	static void ForceCullZoneCoors(CVector coors);
	static int32 FindAttributesForCoors(CVector coors, int32 *wantedLevel);
	static CAttributeZone *FindZoneWithStairsAttributeForPlayer(void);
	static void MarkSubwayAsInvisible(bool visible);
	static void AddCullZone(CVector const &position,
		float minx, float maxx,
		float miny, float maxy,
		float minz, float maxz,
		uint16 flag, int16 wantedLevel);
	static bool CamCloseInForPlayer(void) { return (CurrentFlags_Player & ATTRZONE_CAMCLOSEIN) != 0; }
	static bool CamStairsForPlayer(void) { return (CurrentFlags_Player & ATTRZONE_STAIRS) != 0; }
	static bool Cam1stPersonForPlayer(void) { return (CurrentFlags_Player & ATTRZONE_1STPERSON) != 0; }
	static bool NoPolice(void) { return (CurrentFlags_Player & ATTRZONE_NOPOLICE) != 0; }
	static bool DoINeedToLoadCollision(void) { return (CurrentFlags_Player & ATTRZONE_DOINEEDCOLLISION) != 0; }
	static bool PlayerNoRain(void) { return (CurrentFlags_Player & ATTRZONE_NORAIN) != 0; }
	static bool CamNoRain(void) { return (CurrentFlags_Camera & ATTRZONE_NORAIN) != 0; }
	static bool WaterFudge(void) { return (CurrentFlags_Camera & ATTRZONE_WATERFUDGE) != 0; }
	static int32 GetWantedLevelDrop(void) { return CurrentWantedLevelDrop_Player; }

	//--MIAMI: TODO
	static bool PoliceAbandonCars(void) { return false; }
};
