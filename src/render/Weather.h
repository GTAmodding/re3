enum {
	WEATHER_RANDOM = -1,
	WEATHER_SUNNY = 0,
	WEATHER_CLOUDY,
	WEATHER_RAINY,
	WEATHER_FOGGY,
	WEATHER_EXTRA_SUNNY,
	WEATHER_HURRICANE,
	WEATHER_TOTAL
};

class CWeather
{
public:
	static int32 SoundHandle;

	static int32 WeatherTypeInList;
	static int16 OldWeatherType;
	static int16 NewWeatherType;
	static int16 ForcedWeatherType;

	static bool LightningFlash;
	static bool LightningBurst;
	static uint32 LightningStart;
	static uint32 LightningFlashLastChange;
	static uint32 WhenToPlayLightningSound;
	static uint32 LightningDuration;

	static float ExtraSunnyness;
	static float Foggyness;
	static float CloudCoverage;
	static float Wind;
	static float Rain;
	static float InterpolationValue;
	static float WetRoads;
	static float Rainbow;
	static float SunGlare;
	static float WindClipped;
	static float TrafficLightBrightness;

	static bool bScriptsForceRain;

	static void RenderRainStreaks(void);
	static void Update(void);
	static void Init(void);
	
	static void ReleaseWeather();
	static void ForceWeather(int16);
	static void ForceWeatherNow(int16);
	static void AddRain();
	static void AddHeatHaze();
	static void AddBeastie();
};

enum {
	NUM_RAIN_STREAKS = 35
};

struct tRainStreak
{
	CVector position;
	CVector direction;
	uint32 timer;
};

extern RwTexture* gpRainDropTex;