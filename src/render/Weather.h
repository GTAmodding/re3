enum {
	WEATHER_SUNNY,
	WEATHER_CLOUDY,
	WEATHER_RAINY,
	WEATHER_FOGGY
};

class CWeather
{
public:
	static int32 &SoundHandle;

	static int32 &WeatherTypeInList;
	static int16 &OldWeatherType;
	static int16 &NewWeatherType;
	static int16 &ForcedWeatherType;

	static bool &LightningFlash;
	static bool &LightningBurst;
	static uint32 &LightningStart;
	static uint32 &LightningFlashLastChange;
	static uint32 &WhenToPlayLightningSound;
	static uint32 &LightningDuration;

	static float &Foggyness;
	static float &CloudCoverage;
	static float &Wind;
	static float &Rain;
	static float &InterpolationValue;
	static float &WetRoads;
	static float &Rainbow;

	static bool &bScriptsForceRain;
	static bool &Stored_StateStored;

	static void RenderRainStreaks(void);
	static void Update(void);
	
	static void ReleaseWeather();
	static void ForceWeather(int16);
	static void ForceWeatherNow(int16);
};
