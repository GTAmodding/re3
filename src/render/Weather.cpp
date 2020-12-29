#include "common.h"

#include "Weather.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Camera.h"
#include "Clock.h"
#include "CutsceneMgr.h"
#include "DMAudio.h"
#include "General.h"
#include "Pad.h"
#include "Particle.h"
#include "PlayerPed.h"
#include "RenderBuffer.h"
#include "Replay.h"
#include "Shadows.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Timecycle.h"
#include "Timer.h"
#include "Vehicle.h"
#include "World.h"
#include "ZoneCull.h"

//--MIAMI: file done

int32 CWeather::SoundHandle = -1;

int32 CWeather::WeatherTypeInList;
int16 CWeather::OldWeatherType;
int16 CWeather::NewWeatherType;
int16 CWeather::ForcedWeatherType;

bool CWeather::LightningFlash;
bool CWeather::LightningBurst;
uint32 CWeather::LightningStart;
uint32 CWeather::LightningFlashLastChange;
uint32 CWeather::WhenToPlayLightningSound;
uint32 CWeather::LightningDuration;
int32 CWeather::StreamAfterRainTimer;

float CWeather::ExtraSunnyness;
float CWeather::Foggyness;
float CWeather::CloudCoverage;
float CWeather::Wind;
float CWeather::Rain;
float CWeather::InterpolationValue;
float CWeather::WetRoads;
float CWeather::Rainbow;
float CWeather::SunGlare;
float CWeather::WindClipped;
float CWeather::TrafficLightBrightness;

bool CWeather::bScriptsForceRain;

tRainStreak Streaks[NUM_RAIN_STREAKS];

const int16 WeatherTypesList[] = {
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_CLOUDY,
	WEATHER_RAINY, WEATHER_RAINY, WEATHER_RAINY, WEATHER_RAINY,
	WEATHER_CLOUDY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY
};

const int16 WeatherTypesList_WithHurricanes[] = {
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_CLOUDY,
	WEATHER_HURRICANE, WEATHER_HURRICANE, WEATHER_CLOUDY, WEATHER_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_CLOUDY, WEATHER_HURRICANE, WEATHER_HURRICANE, WEATHER_HURRICANE,
	WEATHER_CLOUDY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY
};

const float Windyness[] = {
	0.25f,// WEATHER_SUNNY
	0.7f, // WEATHER_CLOUDY
	1.0f, // WEATHER_RAINY
	0.0f, // WEATHER_FOGGY
	0.0f, // WEATHER_EXTRA_SUNNY
	2.0f, // WEATHER_HURRICANE
	0.0f
};

#define MIN_TIME_BETWEEN_LIGHTNING_FLASH_CHANGES (50)

#define RAIN_CHANGE_SPEED (0.003f)

#define DROPLETS_LEFT_OFFSET (10.0f)
#define DROPLETS_RIGHT_OFFSET (10.0f)
#define DROPLETS_TOP_OFFSET (10.0f)
#define DROPLETS_BOTTOM_OFFSET (10.0f)

#define STREAK_U (10.0f)
#define STREAK_V (18.0f)
#define LARGE_STREAK_COEFFICIENT (1.23f)
#define STREAK_MIN_DISTANCE (8.0f)
#define STREAK_MAX_DISTANCE (16.0f)

#define SPLASH_CHECK_RADIUS (7.0f)
#define SPLASH_OFFSET_RADIUS (2.0f)

#define STREAK_LIFETIME (4.0f)
#define STREAK_INTEROLATION_TIME (0.3f)

#define RAIN_COLOUR_R (200)
#define RAIN_COLOUR_G (200)
#define RAIN_COLOUR_B (256)
#define RAIN_ALPHA (255)

void CWeather::Init(void)
{
	NewWeatherType = WEATHER_EXTRA_SUNNY;
	bScriptsForceRain = false;
	OldWeatherType = WEATHER_EXTRA_SUNNY;
	InterpolationValue = 0.0f;
	WhenToPlayLightningSound = 0;
	WeatherTypeInList = 0;
	ForcedWeatherType = WEATHER_RANDOM;
	SoundHandle = DMAudio.CreateEntity(AUDIOTYPE_WEATHER, (void*)1);
	if (SoundHandle >= 0)
		DMAudio.SetEntityStatus(SoundHandle, true);
}

void CWeather::Update(void)
{
	if(!CReplay::IsPlayingBack()){
		float fNewInterpolation = (CClock::GetMinutes() + CClock::GetSeconds()/60.0f)/60.0f;
		if (fNewInterpolation < InterpolationValue) {
			// new hour
			OldWeatherType = NewWeatherType;
			if (ForcedWeatherType >= 0)
				NewWeatherType = ForcedWeatherType;
			else {
				WeatherTypeInList = (WeatherTypeInList + 1) % ARRAY_SIZE(WeatherTypesList);
				NewWeatherType = CStats::NoMoreHurricanes ? WeatherTypesList[WeatherTypeInList] : WeatherTypesList_WithHurricanes[WeatherTypeInList];
			}
		}
		InterpolationValue = fNewInterpolation;
	}

#ifndef FINAL
	if (CPad::GetPad(1)->GetRightShockJustDown()) {
		NewWeatherType = (NewWeatherType + 1) % WEATHER_TOTAL;
		OldWeatherType = NewWeatherType;
	}
#endif

	// Lightning
	if (NewWeatherType != WEATHER_RAINY || OldWeatherType != WEATHER_RAINY) {
		LightningFlash = false;
		LightningBurst = false;
	}
	else{
		if (LightningBurst) {
			if ((CGeneral::GetRandomNumber() & 255) >= 32) {
				// 0.875 probability
				if (CTimer::GetTimeInMilliseconds() - LightningFlashLastChange > MIN_TIME_BETWEEN_LIGHTNING_FLASH_CHANGES) {
					bool bOldLightningFlash = LightningFlash;
					LightningFlash = CGeneral::GetRandomTrueFalse();
					if (LightningFlash != bOldLightningFlash)
						LightningFlashLastChange = CTimer::GetTimeInMilliseconds();
				}
			}
			else {
				// 0.125 probability
				LightningBurst = false;
				LightningDuration = Min(CTimer::GetFrameCounter() - LightningStart, 20);
				LightningFlash = false;
				WhenToPlayLightningSound = CTimer::GetTimeInMilliseconds() + 150 * (20 - LightningDuration);
			}
		}
		else {
			if (CGeneral::GetRandomNumber() >= 200) {
				// lower probability on PC due to randomness bug
				LightningFlash = false;
			}
			else {
				LightningBurst = true;
				LightningStart = CTimer::GetFrameCounter();
				LightningFlashLastChange = CTimer::GetTimeInMilliseconds();
				LightningFlash = true;
			}
		}
	}
	if (WhenToPlayLightningSound && CTimer::GetTimeInMilliseconds() > WhenToPlayLightningSound) {
		DMAudio.PlayOneShot(SoundHandle, SOUND_LIGHTNING, LightningDuration);
		CPad::GetPad(0)->StartShake(40 * LightningDuration + 100, 2 * LightningDuration + 80);
		WhenToPlayLightningSound = 0;
	}

	// Wet roads
	if (OldWeatherType == WEATHER_RAINY || OldWeatherType == WEATHER_HURRICANE) {
		if (NewWeatherType == WEATHER_RAINY || NewWeatherType == WEATHER_HURRICANE)
			WetRoads = 1.0f;
		else
			WetRoads = 1.0f - InterpolationValue;
	}
	else {
		if (NewWeatherType == WEATHER_RAINY || NewWeatherType == WEATHER_HURRICANE)
			WetRoads = InterpolationValue;
		else
			WetRoads = 0.0f;
	}

	// Rain
	float fNewRain;
	if (NewWeatherType == WEATHER_RAINY || NewWeatherType == WEATHER_HURRICANE) {
		// if raining for >1 hour, values: 0, 0.33, 0.66, 0.99, switching every ~16.5s
		fNewRain = ((uint16)CTimer::GetTimeInMilliseconds() >> 14) * 0.33f;
		if (OldWeatherType != WEATHER_RAINY && OldWeatherType != WEATHER_HURRICANE) {
			if (InterpolationValue < 0.4f)
				// if rain has just started (<24 minutes), always 0.5
				fNewRain = 0.5f;
			else
				// if rain is ongoing for >24 minutes, values: 0.25, 0.5, 0.75, 1.0, switching every ~16.5s
				fNewRain = 0.25f + ((uint16)CTimer::GetTimeInMilliseconds() >> 14) * 0.25f;
		}
		fNewRain = Max(fNewRain, 0.5f);
	}
	else
		fNewRain = 0.0f;
	Rain = fNewRain;

	// Clouds
	if (OldWeatherType != WEATHER_SUNNY && OldWeatherType != WEATHER_EXTRA_SUNNY)
		CloudCoverage = 1.0f - InterpolationValue;
	else
		CloudCoverage = 0.0f;
	if (NewWeatherType != WEATHER_SUNNY && OldWeatherType != WEATHER_EXTRA_SUNNY)
		CloudCoverage += InterpolationValue;
	
	// Fog
	if (OldWeatherType == WEATHER_FOGGY)
		Foggyness = 1.0f - InterpolationValue;
	else
		Foggyness = 0.0f;
	if (NewWeatherType == WEATHER_FOGGY)
		Foggyness += InterpolationValue;

	// Extra Sunnyness
	if (OldWeatherType == WEATHER_EXTRA_SUNNY)
		ExtraSunnyness = 1.0f - InterpolationValue;
	else
		ExtraSunnyness = 0.0f;
	if (NewWeatherType == WEATHER_EXTRA_SUNNY)
		ExtraSunnyness += InterpolationValue;

	// Rainbow
	if (OldWeatherType == WEATHER_CLOUDY && (NewWeatherType == WEATHER_SUNNY || NewWeatherType == WEATHER_EXTRA_SUNNY) &&
		InterpolationValue < 0.5f && CClock::GetHours() > 6 && CClock::GetHours() < 21)
		Rainbow = 1.0f - 4.0f * Abs(InterpolationValue - 0.25f) / 4.0f;
	else
		Rainbow = 0.0f;

	// Sun Glare
	if (OldWeatherType == WEATHER_EXTRA_SUNNY)
		SunGlare = 1.0f - InterpolationValue;
	else
		SunGlare = 0.0f;
	if (NewWeatherType == WEATHER_EXTRA_SUNNY)
		SunGlare += InterpolationValue;

	if (SunGlare > 0.0f) {
		SunGlare *= Min(1.0f, 7.0 * CTimeCycle::GetSunDirection().z);
		SunGlare = clamp(SunGlare, 0.0f, 1.0f);
		if (!CSpecialFX::bSnapShotActive)
			SunGlare *= (1.0f - (CGeneral::GetRandomNumber()&0x1F)*0.007f);
	}

	Wind = InterpolationValue * Windyness[NewWeatherType] + (1.0f - InterpolationValue) * Windyness[OldWeatherType];
	WindClipped = Min(1.0f, Wind);

	if (CClock::GetHours() > 20)
		TrafficLightBrightness = 1.0f;
	else if (CClock::GetHours() > 19)
		TrafficLightBrightness = CClock::GetMinutes() / 60.0f;
	else if (CClock::GetHours() > 6)
		TrafficLightBrightness = 0.0f;
	else if (CClock::GetHours() > 5)
		TrafficLightBrightness = 1.0f - CClock::GetMinutes() / 60.0f;
	else
		TrafficLightBrightness = 1.0f;
	TrafficLightBrightness = Max(WetRoads, TrafficLightBrightness);
	TrafficLightBrightness = Max(Foggyness, TrafficLightBrightness);
	TrafficLightBrightness = Max(Rain, TrafficLightBrightness);

	AddRain();

	if ((NewWeatherType == WEATHER_SUNNY || NewWeatherType == WEATHER_EXTRA_SUNNY) &&
		!CGame::IsInInterior() && !CCutsceneMgr::IsRunning() && (CTimer::GetFrameCounter() & 7) == 0) {
#ifdef FIX_BUGS
		if (FindPlayerPed() && (!FindPlayerPed()->CheckIfInTheAir() || FindPlayerPed()->CheckIfInTheAir() && FindPlayerPed()->GetPosition().z < 7.5f &&
			CClock::GetHours() > 6 && CClock::GetHours() < 18))
#else
		if (!FindPlayerPed()->CheckIfInTheAir() || FindPlayerPed()->CheckIfInTheAir() && FindPlayerPed()->GetPosition().z < 7.5f &&
			CClock::GetHours() > 6 && CClock::GetHours() < 18)
#endif
			AddHeatHaze();
	}

	if ((NewWeatherType == WEATHER_SUNNY || NewWeatherType == WEATHER_EXTRA_SUNNY) && !CGame::IsInInterior() && !CCutsceneMgr::IsRunning())
		AddBeastie();
}

void CWeather::AddHeatHaze()
{
	if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED)
		return;
	CVector pos;
	pos.x = SCREEN_WIDTH*0.5f;
	if(TheCamera.GetLookingForwardFirstPerson())
		pos.y = CGeneral::GetRandomNumberInRange(SCREEN_HEIGHT*0.25f, SCREEN_HEIGHT*0.9f);
	else
		pos.y = CGeneral::GetRandomNumberInRange(SCREEN_HEIGHT*0.4f, SCREEN_HEIGHT*0.9f);
	pos.z = 100.0f;
	CParticle::AddParticle(PARTICLE_HEATHAZE_IN_DIST, pos, CVector(0.0f, 0.0f, 0.0f));
}

void CWeather::AddBeastie()
{
	if(FindPlayerVehicle() || CTimer::GetFrameCounter()%10 || (CGeneral::GetRandomNumber()&5) == 0)
		return;
	CVector pos = TheCamera.GetPosition();
	float dist = CGeneral::GetRandomNumberInRange(90.0f, 60.0f);
	int angle = CGeneral::GetRandomNumber() % CParticle::SIN_COS_TABLE_SIZE;
	float c = CParticle::m_CosTable[angle];
	float s = CParticle::m_SinTable[angle];
	pos.x += dist*(c - s);
	pos.y += dist*(c + s);
	pos.z += CGeneral::GetRandomNumberInRange(7.5f, 30.0f);
	CParticle::AddParticle(PARTICLE_BEASTIE, pos, CVector(0.0f, 0.0f, 0.0f));
}

void CWeather::ForceWeather(int16 weather)
{
	ForcedWeatherType = weather;
}

void CWeather::ForceWeatherNow(int16 weather)
{
	OldWeatherType = weather;
	NewWeatherType = weather;
	ForcedWeatherType = weather;
}

void CWeather::ReleaseWeather()
{
	ForcedWeatherType = -1;
}

void CWeather::AddSplashesDuringHurricane()
{
	RwRGBA colour = { 255, 255, 255, 32 };
	CVector pos = TheCamera.pTargetEntity ? TheCamera.pTargetEntity->GetPosition() : TheCamera.GetPosition();
	bool foundGround;
	float groundZ = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &foundGround) + 0.1f;
	if(!foundGround)
		groundZ = pos.z + 0.5f;
	for(int i = 0; i < 20; i++){
		float dist = (CGeneral::GetRandomNumber()&0xFF)/255.0f +
			CGeneral::GetRandomNumberInRange(-10.0f, 30.0f);
		float angle;
		uint8 rnd = CGeneral::GetRandomNumber();
		if(rnd&1)
			angle = (CGeneral::GetRandomNumber()&0x7F)/128.0f * TWOPI;
		else
			angle = TheCamera.Orientation + (rnd-128)/160.0f;
		pos.x = TheCamera.GetPosition().x + dist*Sin(angle);
		pos.y = TheCamera.GetPosition().y + dist*Cos(angle);
		pos.z = groundZ;
		if(foundGround)
			CParticle::AddParticle(PARTICLE_GROUND_STEAM, pos, CVector(-0.002f, -0.002f, 0.015f), nil, 0.0f, colour);
	}
}

static int startStreamAfterRain;

void CWeather::AddStreamAfterRain()
{
	if(CClock::GetHours() > 6 && CClock::GetHours() < 18){
		RwRGBA colour = { 255, 255, 255, 24 };
		CVector pos = TheCamera.pTargetEntity ? TheCamera.pTargetEntity->GetPosition() : TheCamera.GetPosition();
		bool foundGround;
		float groundZ = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &foundGround) + 0.2f;
		if(!foundGround)
			groundZ = pos.z + 0.75f;
		for(int i = 0; i < 20; i++){
			float dist = (CGeneral::GetRandomNumber()&0xFF)/255.0f +
				CGeneral::GetRandomNumberInRange(-10.0f, 30.0f);
			float angle;
			uint8 rnd = CGeneral::GetRandomNumber();
			if(rnd&1)
				angle = (CGeneral::GetRandomNumber()&0x7F)/128.0f * TWOPI;
			else
				angle = TheCamera.Orientation + (rnd-128)/160.0f;
			pos.x = TheCamera.GetPosition().x + dist*Sin(angle);
			pos.y = TheCamera.GetPosition().y + dist*Cos(angle);
			pos.z = groundZ;
			CParticle::AddParticle(PARTICLE_GROUND_STEAM, pos, CVector(0.0f, 0.0f, 0.015f), nil, 0.0f, colour);
		}
	}else{
		startStreamAfterRain = 0;
		StreamAfterRainTimer = 800;
	}
}

void CWeather::AddRain()
{
	if (CCullZones::CamNoRain() || CCullZones::PlayerNoRain())
		return;
	if (TheCamera.GetLookingLRBFirstPerson()) {
		CVehicle* pVehicle = FindPlayerVehicle();
		if (pVehicle && pVehicle->CarHasRoof()) {
			CParticle::RemovePSystem(PARTICLE_RAINDROP_2D);
			return;
		}
	}

	if(Rain > 0.0){
		startStreamAfterRain = 1;
		StreamAfterRainTimer = 800;
	}else if(startStreamAfterRain){
		if(StreamAfterRainTimer > 0){
			AddStreamAfterRain();
			StreamAfterRainTimer--;
		}else{
			startStreamAfterRain = 0;
			StreamAfterRainTimer = 800;
		}
	}

	if (Wind > 1.1f)
		AddSplashesDuringHurricane();

	if (Rain <= 0.1f)
		return;
	static RwRGBA colour;
	int numDrops = 5.0f * Rain;
	int numSplashes = 2.0f * Rain;
	CVector pos, dir;
	for(int i = 0; i < numDrops; i++){
		pos.x = CGeneral::GetRandomNumberInRange(0, (int)SCREEN_WIDTH);
		pos.y = CGeneral::GetRandomNumberInRange(0, (int)SCREEN_HEIGHT/5);
		pos.z = 0.0f;
		dir.x = 0.0f;
		dir.y = CGeneral::GetRandomNumberInRange(30.0f, 40.0f);
		dir.z = 0.0f;
		CParticle::AddParticle(PARTICLE_RAINDROP_2D, pos, dir, nil, CGeneral::GetRandomNumberInRange(0.1f, 0.75f), 0, 0, (int)Rain&3, 0);

		pos.x = CGeneral::GetRandomNumberInRange(0, (int)SCREEN_WIDTH);
		pos.y = CGeneral::GetRandomNumberInRange((int)SCREEN_HEIGHT/5, (int)SCREEN_HEIGHT/2);
		pos.z = 0.0f;
		dir.x = 0.0f;
		dir.y = CGeneral::GetRandomNumberInRange(30.0f, 40.0f);
		dir.z = 0.0f;
		CParticle::AddParticle(PARTICLE_RAINDROP_2D, pos, dir, nil, CGeneral::GetRandomNumberInRange(0.1f, 0.75f), 0, 0, (int)Rain&3, 0);

		pos.x = CGeneral::GetRandomNumberInRange(0, (int)SCREEN_WIDTH);
		pos.y = 0.0f;
		pos.z = 0.0f;
		dir.x = 0.0f;
		dir.y = CGeneral::GetRandomNumberInRange(30.0f, 40.0f);
		dir.z = 0.0f;
		CParticle::AddParticle(PARTICLE_RAINDROP_2D, pos, dir, nil, CGeneral::GetRandomNumberInRange(0.1f, 0.75f), 0, 0, (int)Rain&3, 0);

		float dist = CGeneral::GetRandomNumberInRange(0.0f, Max(10.0f*Rain, 40.0f)/2.0f);
		float angle;
		uint8 rnd = CGeneral::GetRandomNumber();
		if(rnd&1)
			angle = (CGeneral::GetRandomNumber()&0x7F)/128.0f * TWOPI;
		else
			angle = TheCamera.Orientation + (rnd-128)/160.0f;
		pos.x = TheCamera.GetPosition().x + dist*Sin(angle);
		pos.y = TheCamera.GetPosition().y + dist*Cos(angle);
		pos.z = 0.0f;
		CColPoint point;
		CEntity *ent;
		if(CWorld::ProcessVerticalLine(pos+CVector(0.0f, 0.0f, 40.0f), -40.0f, point, ent, true, false, false, false, true, false, nil)){
			pos.z = point.point.z;
			for(int j = 0; j < numSplashes+15; j++){
				CVector pos2 = pos;
				pos2.x += CGeneral::GetRandomNumberInRange(-15.0f, 15.0f);
				pos2.y += CGeneral::GetRandomNumberInRange(-15.0f, 15.0f);
				if(CGeneral::GetRandomNumber() & 1)
					CParticle::AddParticle(PARTICLE_RAIN_SPLASH, pos2, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, colour);
				else
					CParticle::AddParticle(PARTICLE_RAIN_SPLASHUP, pos2, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, colour);
			}
		}
	}
}

void RenderOneRainStreak(CVector pos, CVector unused, int intensity, bool scale, float distance)
{
	static float RandomTex;
	static float RandomTexX;
	static float RandomTexY;
	TempBufferRenderIndexList[TempBufferIndicesStored + 0] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[TempBufferIndicesStored + 1] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[TempBufferIndicesStored + 2] = TempBufferVerticesStored + 1;
	TempBufferRenderIndexList[TempBufferIndicesStored + 3] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[TempBufferIndicesStored + 4] = TempBufferVerticesStored + 3;
	TempBufferRenderIndexList[TempBufferIndicesStored + 5] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[TempBufferIndicesStored + 6] = TempBufferVerticesStored + 1;
	TempBufferRenderIndexList[TempBufferIndicesStored + 7] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[TempBufferIndicesStored + 8] = TempBufferVerticesStored + 4;
	TempBufferRenderIndexList[TempBufferIndicesStored + 9] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[TempBufferIndicesStored + 10] = TempBufferVerticesStored + 3;
	TempBufferRenderIndexList[TempBufferIndicesStored + 11] = TempBufferVerticesStored + 4;
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored + 0], 0, 0, 0, 0);
	RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored + 0], pos.x + 11.0f * TheCamera.GetUp().x, pos.y + 11.0f * TheCamera.GetUp().y, pos.z + 11.0f * TheCamera.GetUp().z);
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored + 1], 0, 0, 0, 0);
	RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored + 1], pos.x - 9.0f * TheCamera.GetRight().x, pos.y - 9.0f * TheCamera.GetRight().y, pos.z - 9.0f * TheCamera.GetRight().z);
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored + 2], RAIN_COLOUR_R * intensity / 256, RAIN_COLOUR_G * intensity / 256, RAIN_COLOUR_B * intensity / 256, RAIN_ALPHA);
	RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored + 2], pos.x, pos.y, pos.z);
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored + 3], 0, 0, 0, 0);
	RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored + 3], pos.x + 9.0f * TheCamera.GetRight().x, pos.y + 9.0f * TheCamera.GetRight().y, pos.z + 9.0f * TheCamera.GetRight().z);
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored + 4], 0, 0, 0, 0); 
	RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored + 4], pos.x - 11.0f * TheCamera.GetUp().x, pos.y - 11.0f * TheCamera.GetUp().y, pos.z - 11.0f * TheCamera.GetUp().z);
	float u = STREAK_U;
	float v = STREAK_V;
	if (scale) {
		u *= LARGE_STREAK_COEFFICIENT;
		v *= LARGE_STREAK_COEFFICIENT;
	}
	float distance_coefficient;
	if (distance < STREAK_MIN_DISTANCE)
		distance_coefficient = 1.0f;
	else if (distance > STREAK_MAX_DISTANCE)
		distance_coefficient = 0.5f;
	else
		distance_coefficient = 1.0f - 0.5f * (distance - STREAK_MIN_DISTANCE) / (STREAK_MAX_DISTANCE - STREAK_MIN_DISTANCE);
	u *= distance_coefficient;
	v *= distance_coefficient;
	if (!CTimer::GetIsPaused()) {
		RandomTex = 0.0f;
		RandomTexX = 0.0f;
		RandomTexY = 0.0f;
	}
	RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored + 0], 0.5f * u - RandomTex + RandomTexX);
	RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored + 0], -v * 0.5f + RandomTexY);
	RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored + 1], RandomTexX);
	RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored + 1], RandomTexY);
	RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored + 2], 0.5f * u + RandomTexX);
	RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored + 2], RandomTexY);
	RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored + 3], u + RandomTexX);
	RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored + 3], RandomTexY);
	RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored + 4], 0.5f * u + RandomTex + RandomTexX);
	RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored + 5], 0.5f * v + RandomTexY);
	TempBufferIndicesStored += 12;
	TempBufferVerticesStored += 5;
}

void CWeather::RenderRainStreaks(void)
{
	if (CTimer::GetIsCodePaused())
		return;
	int base_intensity = (64.0f - CTimeCycle::GetFogReduction()) / 64.0f * int(255 * Rain);
	if (base_intensity == 0)
		return;
	if (TheCamera.m_CameraAverageSpeed > 1.75f)
		return;
	TempBufferIndicesStored = 0;
	TempBufferVerticesStored = 0;
	for (int i = 0; i < NUM_RAIN_STREAKS; i++) {
		if (Streaks[i].timer) {
			float secondsElapsed = (CTimer::GetTimeInMilliseconds() - Streaks[i].timer) / 1024.0f;
			if (secondsElapsed > STREAK_LIFETIME)
				Streaks[i].timer = 0;
			else{
				int intensity;
				if (secondsElapsed < STREAK_INTEROLATION_TIME)
					intensity = base_intensity * 0.25f * secondsElapsed / STREAK_INTEROLATION_TIME;
				else if (secondsElapsed > (STREAK_LIFETIME - STREAK_INTEROLATION_TIME))
					intensity = (STREAK_LIFETIME - secondsElapsed) * 0.25f * base_intensity / STREAK_INTEROLATION_TIME;
				else
					intensity = base_intensity * 0.25f;
				CVector dir = Streaks[i].direction;
				dir.Normalise();
				CVector pos = Streaks[i].position + secondsElapsed * Streaks[i].direction;
				RenderOneRainStreak(pos, dir, intensity, false, (pos - TheCamera.GetPosition()).Magnitude());
#ifndef FIX_BUGS // remove useless code
				if (secondsElapsed > 1.0f && secondsElapsed < STREAK_LIFETIME - 1.0f) {
					CGeneral::GetRandomNumber(), CGeneral::GetRandomNumber();
				}
#endif
			}
		}
		else if ((CGeneral::GetRandomNumber() & 0xF00) == 0){
			// 1/16 probability
			Streaks[i].direction = CVector(0.0f, 0.0f, -12.0f);
			Streaks[i].position = 6.0f * TheCamera.GetForward() + TheCamera.GetPosition() + CVector(-1.8f * Streaks[i].direction.x, -1.8f * Streaks[i].direction.y, 8.0f);
			if (!CCutsceneMgr::IsRunning()) {
				Streaks[i].position.x += 2.0f * FindPlayerSpeed().x * 60.0f;
				Streaks[i].position.y += 2.0f * FindPlayerSpeed().y * 60.0f;
			}
			else
				Streaks[i].position += (TheCamera.GetPosition() - TheCamera.m_RealPreviousCameraPosition) * 20.0f;
			Streaks[i].position.x += ((CGeneral::GetRandomNumber() & 255) - 128) * 0.04f;
			Streaks[i].position.y += ((CGeneral::GetRandomNumber() & 255) - 128) * 0.04f;
			Streaks[i].timer = CTimer::GetTimeInMilliseconds();
		}
	}
	if (TempBufferIndicesStored){
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void*)rwFOGTYPELINEAR);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpRainDropTex));
		if (RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, 1))
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
			RwIm3DEnd();
		}
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	}
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
}
