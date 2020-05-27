#include "common.h"

#include "General.h"
#include "Timer.h"
#include "TxdStore.h"
#include "Sprite.h"
#include "Camera.h"
#include "Collision.h"
#include "World.h"
#include "Shadows.h"
#include "Replay.h"
#include "Stats.h"
#include "Weather.h"
#include "MBlur.h"
#include "main.h"
#include "AudioScriptObject.h"
#include "ParticleObject.h"
#include "Particle.h"
#include "soundlist.h"


#define MAX_PARTICLES_ON_SCREEN   (750)


//(5)
#define MAX_SMOKE_FILES           ARRAY_SIZE(SmokeFiles)

//(5) 
#define MAX_RUBBER_FILES          ARRAY_SIZE(RubberFiles)
//(5)
#define MAX_RAINSPLASH_FILES      ARRAY_SIZE(RainSplashFiles)
//(3)
#define MAX_WATERSPRAY_FILES      ARRAY_SIZE(WatersprayFiles)
//(6)
#define MAX_EXPLOSIONMEDIUM_FILES ARRAY_SIZE(ExplosionMediumFiles)
//(4)
#define MAX_GUNFLASH_FILES        ARRAY_SIZE(GunFlashFiles)
//(2)
#define MAX_RAINSPLASHUP_FILES    ARRAY_SIZE(RainSplashupFiles)
//(4)
#define MAX_BIRDFRONT_FILES       ARRAY_SIZE(BirdfrontFiles)
//(8)
#define MAX_BOAT_FILES            ARRAY_SIZE(BoatFiles)
//(4)
#define MAX_CARDEBRIS_FILES       ARRAY_SIZE(CardebrisFiles)
//(4)
#define MAX_CARSPLASH_FILES       ARRAY_SIZE(CarsplashFiles)

#define MAX_RAINDRIP_FILES       (2)

	
const char SmokeFiles[][6+1] =
{
	"smoke1",
	"smoke2",
	"smoke3",
	"smoke4",
	"smoke5"
};


const char RubberFiles[][7+1] =
{
	"rubber1",
	"rubber2",
	"rubber3",
	"rubber4",
	"rubber5"
};

const char RainSplashFiles[][7+1] =
{
	"splash1",
	"splash2",
	"splash3",
	"splash4",
	"splash5"
};

const char WatersprayFiles[][11+1] =
{
	"waterspray1",
	"waterspray2",
	"waterspray3"
};

const char ExplosionMediumFiles[][7+1] =
{
	"explo01",
	"explo02",
	"explo03",
	"explo04",
	"explo05",
	"explo06"
};

const char GunFlashFiles[][9+1] =
{
	"gunflash1",
	"gunflash2",
	"gunflash3",
	"gunflash4"
};

const char RainSplashupFiles[][10+1] =
{
	"splash_up1",
	"splash_up2"
};

const char BirdfrontFiles[][8+1] =
{
	"birdf_01",
	"birdf_02",
	"birdf_03",
	"birdf_04"
};

const char BoatFiles[][8+1] =
{
	"boats_01",
	"boats_02",
	"boats_03",
	"boats_04",
	"boats_05",
	"boats_06",
	"boats_07",
	"boats_08"
};

const char CardebrisFiles[][12+1] =
{
	"cardebris_01",
	"cardebris_02",
	"cardebris_03",
	"cardebris_04"
};
				
const char CarsplashFiles[][12+1] =
{
	"carsplash_01",
	"carsplash_02",
	"carsplash_03",
	"carsplash_04"
};

CParticle gParticleArray[MAX_PARTICLES_ON_SCREEN];

RwTexture *gpSmokeTex[MAX_SMOKE_FILES];
RwTexture *gpSmoke2Tex;
RwTexture *gpRubberTex[MAX_RUBBER_FILES];
RwTexture *gpRainSplashTex[MAX_RAINSPLASH_FILES];
RwTexture *gpWatersprayTex[MAX_WATERSPRAY_FILES];
RwTexture *gpExplosionMediumTex[MAX_EXPLOSIONMEDIUM_FILES];
RwTexture *gpGunFlashTex[MAX_GUNFLASH_FILES];
RwTexture *gpRainSplashupTex[MAX_RAINSPLASHUP_FILES];
RwTexture *gpBirdfrontTex[MAX_BIRDFRONT_FILES];
RwTexture *gpBoatTex[MAX_BOAT_FILES];
RwTexture *gpCarDebrisTex[MAX_CARDEBRIS_FILES];
RwTexture *gpCarSplashTex[MAX_CARSPLASH_FILES];

RwTexture *gpBoatWakeTex;
RwTexture *gpFlame1Tex;
RwTexture *gpFlame5Tex;
RwTexture *gpRainDropSmallTex;
RwTexture *gpBloodTex;
RwTexture *gpLeafTex;
RwTexture *gpCloudTex1;
RwTexture *gpCloudTex4;
RwTexture *gpBloodSmallTex;
RwTexture *gpGungeTex;
RwTexture *gpCollisionSmokeTex;
RwTexture *gpBulletHitTex;
RwTexture *gpGunShellTex;
RwTexture *gpPointlightTex;

RwRaster  *gpSmokeRaster[MAX_SMOKE_FILES];
RwRaster  *gpSmoke2Raster;
RwRaster  *gpRubberRaster[MAX_RUBBER_FILES];
RwRaster  *gpRainSplashRaster[MAX_RAINSPLASH_FILES];
RwRaster  *gpWatersprayRaster[MAX_WATERSPRAY_FILES];
RwRaster  *gpExplosionMediumRaster[MAX_EXPLOSIONMEDIUM_FILES];
RwRaster  *gpGunFlashRaster[MAX_GUNFLASH_FILES];
RwRaster  *gpRainSplashupRaster[MAX_RAINSPLASHUP_FILES];
RwRaster  *gpBirdfrontRaster[MAX_BIRDFRONT_FILES];
RwRaster  *gpBoatRaster[MAX_BOAT_FILES];
RwRaster  *gpCarDebrisRaster[MAX_CARDEBRIS_FILES];
RwRaster  *gpCarSplashRaster[MAX_CARSPLASH_FILES];

RwRaster  *gpBoatWakeRaster;
RwRaster  *gpFlame1Raster;
RwRaster  *gpFlame5Raster;
RwRaster  *gpRainDropSmallRaster;
RwRaster  *gpBloodRaster;
RwRaster  *gpLeafRaster;
RwRaster  *gpCloudRaster1;
RwRaster  *gpCloudRaster4;
RwRaster  *gpBloodSmallRaster;
RwRaster  *gpGungeRaster;
RwRaster  *gpCollisionSmokeRaster;
RwRaster  *gpBulletHitRaster;
RwRaster  *gpGunShellRaster;
RwRaster  *gpPointlightRaster;

RwTexture *gpRainDropTex;
RwRaster  *gpRainDropRaster;

RwTexture *gpLetterTex;
RwRaster *gpLetterRaster;

RwTexture *gpSparkTex;
RwTexture *gpNewspaperTex;
RwTexture *gpGunSmokeTex;
RwTexture *gpDotTex;
RwTexture *gpHeathazeTex;
RwTexture *gpBeastieTex;
RwTexture *gpRaindripTex1[MAX_RAINDRIP_FILES];
RwTexture *gpRaindripTex2[MAX_RAINDRIP_FILES];

RwRaster *gpSparkRaster;
RwRaster *gpNewspaperRaster;
RwRaster *gpGunSmokeRaster;
RwRaster *gpDotRaster;
RwRaster *gpHeathazeRaster;
RwRaster *gpBeastieRaster;
RwRaster *gpRaindripRaster1[MAX_RAINDRIP_FILES];
RwRaster *gpRaindripRaster2[MAX_RAINDRIP_FILES];

float      CParticle::ms_afRandTable[CParticle::RAND_TABLE_SIZE];
CParticle *CParticle::m_pUnusedListHead;
float      CParticle::m_SinTable[CParticle::SIN_COS_TABLE_SIZE];
float      CParticle::m_CosTable[CParticle::SIN_COS_TABLE_SIZE]; 

int32 Randomizer;
int32 nParticleCreationInterval = 1;
float PARTICLE_WIND_TEST_SCALE  = 0.002f;
float fParticleScaleLimit       = 0.5f;

bool clearWaterDrop;
int32 numWaterDropOnScreen;

#ifdef DEBUGMENU
SETTWEAKPATH("Particle");
TWEAKINT32(nParticleCreationInterval, 0, 5, 1);
TWEAKFLOAT(fParticleScaleLimit, 0.0f, 1.0f, 0.1f);
TWEAKFUNC(CParticle::ReloadConfig);
#endif



void CParticle::ReloadConfig()
{
	debug("Initialising CParticleMgr...");
	
	mod_ParticleSystemManager.Initialise();
	
	debug("Initialising CParticle...");
	
	m_pUnusedListHead = gParticleArray;
	
	for ( int32 i = 0; i < MAX_PARTICLES_ON_SCREEN; i++ )
	{
		if ( i == MAX_PARTICLES_ON_SCREEN - 1 )
			gParticleArray[i].m_pNext = nil;
		else
			gParticleArray[i].m_pNext = &gParticleArray[i + 1];
		
		gParticleArray[i].m_vecPosition = CVector(0.0f, 0.0f, 0.0f);

		gParticleArray[i].m_vecVelocity = CVector(0.0f, 0.0f, 0.0f);

		gParticleArray[i].m_nTimeWhenWillBeDestroyed = 0;

		gParticleArray[i].m_nTimeWhenColorWillBeChanged = 0;

		gParticleArray[i].m_fSize = 0.2f;

		gParticleArray[i].m_fExpansionRate = 0.0f;

		gParticleArray[i].m_nColorIntensity = 255;

		gParticleArray[i].m_nFadeToBlackTimer = 0;

		gParticleArray[i].m_nAlpha = 255;

		gParticleArray[i].m_nFadeAlphaTimer = 0;

		gParticleArray[i].m_nCurrentZRotation = 0;

		gParticleArray[i].m_nZRotationTimer = 0;

		gParticleArray[i].m_fCurrentZRadius = 0.0f;

		gParticleArray[i].m_nZRadiusTimer = 0;

		gParticleArray[i].m_nCurrentFrame = 0;

		gParticleArray[i].m_nAnimationSpeedTimer = 0;

		gParticleArray[i].m_nRotation = 0;

		gParticleArray[i].m_nRotationStep = 0;
	}
}

void CParticle::Initialise()
{
	ReloadConfig();

	CParticleObject::Initialise();

	float randVal = -1.0f;
	for ( int32 i = 0; i < RAND_TABLE_SIZE; i++ )
	{
		ms_afRandTable[i] = randVal;
		randVal += 0.1f;
	}
	
	for ( int32 i = 0; i < SIN_COS_TABLE_SIZE; i++ )
	{		
		float angle = DEGTORAD(float(i) * float(360.0f / SIN_COS_TABLE_SIZE));

		m_SinTable[i] = ::Sin(angle);
		m_CosTable[i] = ::Cos(angle);
	}
	
	int32 slot = CTxdStore::FindTxdSlot("particle");

	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(slot);
	
	for ( int32 i = 0; i < MAX_SMOKE_FILES; i++ )
	{
		gpSmokeTex[i] = RwTextureRead(SmokeFiles[i], nil);
		gpSmokeRaster[i] = RwTextureGetRaster(gpSmokeTex[i]);
	}
	
	gpSmoke2Tex = RwTextureRead("smokeII_3", nil);
	gpSmoke2Raster = RwTextureGetRaster(gpSmoke2Tex);
	
	for ( int32 i = 0; i < MAX_RUBBER_FILES; i++ )
	{
		gpRubberTex[i] = RwTextureRead(RubberFiles[i], nil);
		gpRubberRaster[i] = RwTextureGetRaster(gpRubberTex[i]);
	}
	
	for ( int32 i = 0; i < MAX_RAINSPLASH_FILES; i++ )
	{
		gpRainSplashTex[i] = RwTextureRead(RainSplashFiles[i], nil);
		gpRainSplashRaster[i] = RwTextureGetRaster(gpRainSplashTex[i]);
	}
	
	for ( int32 i = 0; i < MAX_WATERSPRAY_FILES; i++ )
	{
		gpWatersprayTex[i] = RwTextureRead(WatersprayFiles[i], nil);
		gpWatersprayRaster[i] = RwTextureGetRaster(gpWatersprayTex[i]);
	}
	
	for ( int32 i = 0; i < MAX_EXPLOSIONMEDIUM_FILES; i++ )
	{
		gpExplosionMediumTex[i] = RwTextureRead(ExplosionMediumFiles[i], nil);
		gpExplosionMediumRaster[i] = RwTextureGetRaster(gpExplosionMediumTex[i]);
	}
	
	for ( int32 i = 0; i < MAX_GUNFLASH_FILES; i++ )
	{
		gpGunFlashTex[i] = RwTextureRead(GunFlashFiles[i], nil);
		gpGunFlashRaster[i] = RwTextureGetRaster(gpGunFlashTex[i]);
	}
	
	gpRainDropTex = RwTextureRead("raindrop4", nil);
	gpRainDropRaster = RwTextureGetRaster(gpRainDropTex);

	
	for ( int32 i = 0; i < MAX_RAINSPLASHUP_FILES; i++ )
	{
		gpRainSplashupTex[i] = RwTextureRead(RainSplashupFiles[i], nil);
		gpRainSplashupRaster[i] = RwTextureGetRaster(gpRainSplashupTex[i]);
	}
	
	for ( int32 i = 0; i < MAX_BIRDFRONT_FILES; i++ )
	{
		gpBirdfrontTex[i] = RwTextureRead(BirdfrontFiles[i], nil);
		gpBirdfrontRaster[i] = RwTextureGetRaster(gpBirdfrontTex[i]);
	}
	
	for ( int32 i = 0; i < MAX_BOAT_FILES; i++ )
	{
		gpBoatTex[i] = RwTextureRead(BoatFiles[i], nil);
		gpBoatRaster[i] = RwTextureGetRaster(gpBoatTex[i]);
	}
	
	for ( int32 i = 0; i < MAX_CARDEBRIS_FILES; i++ )
	{
		gpCarDebrisTex[i] = RwTextureRead(CardebrisFiles[i], nil);
		gpCarDebrisRaster[i] = RwTextureGetRaster(gpCarDebrisTex[i]);
	}
	
	for ( int32 i = 0; i < MAX_CARSPLASH_FILES; i++ )
	{
		gpCarSplashTex[i] = RwTextureRead(CarsplashFiles[i], nil);
		gpCarSplashRaster[i] = RwTextureGetRaster(gpCarSplashTex[i]);
	}

	gpBoatWakeTex = RwTextureRead("boatwake2", nil);
	gpBoatWakeRaster = RwTextureGetRaster(gpBoatWakeTex);
	
	gpFlame1Tex = RwTextureRead("flame1", nil);
	gpFlame1Raster = RwTextureGetRaster(gpFlame1Tex);

	gpFlame5Tex = RwTextureRead("flame5", nil);
	
#ifdef FIX_BUGS
	gpFlame5Raster = RwTextureGetRaster(gpFlame5Tex);
#else
	gpFlame5Raster = RwTextureGetRaster(gpFlame1Tex);	// copy-paste bug ?
#endif

	gpRainDropSmallTex = RwTextureRead("rainsmall", nil);
	gpRainDropSmallRaster = RwTextureGetRaster(gpRainDropSmallTex);

	gpBloodTex = RwTextureRead("blood", nil);
	gpBloodRaster = RwTextureGetRaster(gpBloodTex);

	gpLeafTex = RwTextureRead("gameleaf01_64", nil);
	gpLeafRaster = RwTextureGetRaster(gpLeafTex);
	
	gpLetterTex = RwTextureRead("letter", nil);
	gpLetterRaster = RwTextureGetRaster(gpLetterTex);

	gpCloudTex1 = RwTextureRead("cloud3", nil);
	gpCloudRaster1 = RwTextureGetRaster(gpCloudTex1);

	gpCloudTex4 = RwTextureRead("cloudmasked", nil);
	gpCloudRaster4 = RwTextureGetRaster(gpCloudTex4);

	gpBloodSmallTex = RwTextureRead("bloodsplat2", nil);
	gpBloodSmallRaster = RwTextureGetRaster(gpBloodSmallTex);

	gpGungeTex = RwTextureRead("gunge", nil);
	gpGungeRaster = RwTextureGetRaster(gpGungeTex);

	gpCollisionSmokeTex = RwTextureRead("collisionsmoke", nil);
	gpCollisionSmokeRaster = RwTextureGetRaster(gpCollisionSmokeTex);

	gpBulletHitTex = RwTextureRead("bullethitsmoke", nil);
	gpBulletHitRaster = RwTextureGetRaster(gpBulletHitTex);

	gpGunShellTex = RwTextureRead("gunshell", nil);
	gpGunShellRaster = RwTextureGetRaster(gpGunShellTex);

	gpPointlightTex = RwTextureRead("pointlight", nil);
	gpPointlightRaster = RwTextureGetRaster(gpPointlightTex);
	
	gpSparkTex = RwTextureRead("spark", nil);
	gpSparkRaster = RwTextureGetRaster(gpSparkTex);
	
	gpNewspaperTex = RwTextureRead("newspaper02_64", nil);
	gpNewspaperRaster = RwTextureGetRaster(gpNewspaperTex);
	
	gpGunSmokeTex = RwTextureRead("gunsmoke3", nil);
	gpGunSmokeRaster = RwTextureGetRaster(gpGunSmokeTex);
	
	gpDotTex = RwTextureRead("dot", nil);
	gpDotRaster = RwTextureGetRaster(gpDotTex);
	
	gpHeathazeTex = RwTextureRead("heathaze", nil);
	gpHeathazeRaster = RwTextureGetRaster(gpHeathazeTex);
	
	gpBeastieTex = RwTextureRead("beastie", nil);
	gpBeastieRaster = RwTextureGetRaster(gpBeastieTex);
	
	gpRaindripTex1[0] = RwTextureRead("raindrip64", nil);
	gpRaindripRaster1[0] = RwTextureGetRaster(gpRaindripTex1[0]);
	
	gpRaindripTex1[1] = RwTextureRead("raindripb64", nil);
	gpRaindripRaster1[1] = RwTextureGetRaster(gpRaindripTex1[1]);
	
	gpRaindripTex2[0] = RwTextureRead("raindrip64_d", nil);
	gpRaindripRaster2[0] = RwTextureGetRaster(gpRaindripTex2[0]);
	
	gpRaindripTex2[1] = RwTextureRead("raindripb64_d", nil);
	gpRaindripRaster2[1] = RwTextureGetRaster(gpRaindripTex2[1]);
	
	CTxdStore::PopCurrentTxd();
	
	for ( int32 i = 0; i < MAX_PARTICLES; i++ )
	{
		tParticleSystemData *entry = &mod_ParticleSystemManager.m_aParticles[i];
		
		switch ( i )
		{
			case PARTICLE_SPARK:
			case PARTICLE_SPARK_SMALL:
			case PARTICLE_RAINDROP_SMALL:
			case PARTICLE_HELI_ATTACK:
				entry->m_ppRaster = &gpRainDropSmallRaster;
				break;
			
			case PARTICLE_WATER_SPARK:
				entry->m_ppRaster = &gpSparkRaster;
				break;
			
			case PARTICLE_WHEEL_DIRT:
			case PARTICLE_SAND:
			case PARTICLE_STEAM2:
			case PARTICLE_STEAM_NY:
			case PARTICLE_STEAM_NY_SLOWMOTION:
			case PARTICLE_GROUND_STEAM:
			case PARTICLE_ENGINE_STEAM:
			case PARTICLE_PEDFOOT_DUST:
			case PARTICLE_CAR_DUST:
			case PARTICLE_EXHAUST_FUMES:
				entry->m_ppRaster = &gpSmoke2Raster;
				break;
			
			case PARTICLE_WHEEL_WATER:
			case PARTICLE_WATER:
			case PARTICLE_SMOKE:
			case PARTICLE_SMOKE_SLOWMOTION:
			case PARTICLE_DRY_ICE:
			case PARTICLE_GARAGEPAINT_SPRAY:
			case PARTICLE_STEAM:
			case PARTICLE_WATER_CANNON:
			case PARTICLE_EXTINGUISH_STEAM:
			case PARTICLE_HELI_DUST:
			case PARTICLE_PAINT_SMOKE:
			case PARTICLE_BULLETHIT_SMOKE:
				entry->m_ppRaster = gpSmokeRaster;
				break;
			
			case PARTICLE_BLOOD:
				entry->m_ppRaster = &gpBloodRaster;
				break;
			
			case PARTICLE_BLOOD_SMALL:
			case PARTICLE_BLOOD_SPURT:
				entry->m_ppRaster = &gpBloodSmallRaster;
				break;
			
			case PARTICLE_DEBRIS:
			case PARTICLE_TREE_LEAVES:
				entry->m_ppRaster = &gpLeafRaster;
				break;
			
			case PARTICLE_DEBRIS2:
				entry->m_ppRaster = &gpGungeRaster;
				break;
			
			case PARTICLE_FLYERS:
				entry->m_ppRaster = &gpNewspaperRaster;
				break;
			
			case PARTICLE_FLAME:
			case PARTICLE_CARFLAME:
				entry->m_ppRaster = &gpFlame1Raster;
				break;
			
			case PARTICLE_FIREBALL:
				entry->m_ppRaster = &gpFlame5Raster;
				break;
			
			case PARTICLE_GUNFLASH:
			case PARTICLE_GUNFLASH_NOANIM:
				entry->m_ppRaster = gpGunFlashRaster;
				break;
			
			
			case PARTICLE_GUNSMOKE:
			case PARTICLE_WATERDROP:
			case PARTICLE_BLOODDROP:
			case PARTICLE_HEATHAZE:
			case PARTICLE_HEATHAZE_IN_DIST:
				entry->m_ppRaster = nil;
				break;
			
			case PARTICLE_GUNSMOKE2:
			case PARTICLE_BOAT_THRUSTJET:
			case PARTICLE_RUBBER_SMOKE:
				entry->m_ppRaster = gpRubberRaster;
				break;
			
			case PARTICLE_CIGARETTE_SMOKE:
				entry->m_ppRaster = &gpGunSmokeRaster;
				break;
			
			case PARTICLE_TEARGAS:
				entry->m_ppRaster = &gpHeathazeRaster;
				break;
			
			case PARTICLE_SHARD:
			case PARTICLE_RAINDROP:
			case PARTICLE_RAINDROP_2D:
				entry->m_ppRaster = &gpRainDropRaster;
				break;
			
			case PARTICLE_SPLASH:
			case PARTICLE_PED_SPLASH:
			case PARTICLE_CAR_SPLASH:
			case PARTICLE_WATER_HYDRANT:
				entry->m_ppRaster = gpCarSplashRaster;
				break;
			
			case PARTICLE_RAIN_SPLASH:
			case PARTICLE_RAIN_SPLASH_BIGGROW:
				entry->m_ppRaster = gpRainSplashRaster;
				break;
			
			case PARTICLE_RAIN_SPLASHUP:
				entry->m_ppRaster = gpRainSplashupRaster;
				break;
			
			case PARTICLE_WATERSPRAY:
				entry->m_ppRaster = gpWatersprayRaster;
				break;
			
			case PARTICLE_EXPLOSION_MEDIUM:
			case PARTICLE_EXPLOSION_LARGE:
			case PARTICLE_EXPLOSION_MFAST:
			case PARTICLE_EXPLOSION_LFAST:
				entry->m_ppRaster = gpExplosionMediumRaster;
				break;
			
			case PARTICLE_BOAT_SPLASH:
				entry->m_ppRaster = &gpBoatWakeRaster;
				break;
			
			case PARTICLE_ENGINE_SMOKE:
			case PARTICLE_ENGINE_SMOKE2:
			case PARTICLE_CARFLAME_SMOKE:
			case PARTICLE_FIREBALL_SMOKE:
			case PARTICLE_ROCKET_SMOKE:
			case PARTICLE_TEST:
				entry->m_ppRaster = &gpCloudRaster4;
				break;
			
			case PARTICLE_CARCOLLISION_DUST:
			case PARTICLE_BURNINGRUBBER_SMOKE:
				entry->m_ppRaster = &gpCollisionSmokeRaster;
				break;
			
			case PARTICLE_CAR_DEBRIS:
			case PARTICLE_HELI_DEBRIS:
			case PARTICLE_BIRD_DEBRIS:
				entry->m_ppRaster = gpCarDebrisRaster;
				break;
			
			case PARTICLE_GUNSHELL_FIRST:
			case PARTICLE_GUNSHELL:
			case PARTICLE_GUNSHELL_BUMP1:
			case PARTICLE_GUNSHELL_BUMP2:
				entry->m_ppRaster = &gpGunShellRaster;
				break;
			
			
			case PARTICLE_BIRD_FRONT:
				entry->m_ppRaster = gpBirdfrontRaster;
				break;
			
			case PARTICLE_SHIP_SIDE:
				entry->m_ppRaster = gpBoatRaster;
				break;
			
			case PARTICLE_BEASTIE:
				entry->m_ppRaster = &gpBeastieRaster;
				break;
		}
	}

	debug("CParticle ready");
}

void CParticle::Shutdown()
{
	debug("Shutting down CParticle...");

	for ( int32 i = 0; i < MAX_SMOKE_FILES; i++ )
	{
		RwTextureDestroy(gpSmokeTex[i]);
		gpSmokeTex[i] = nil;
	}

	RwTextureDestroy(gpSmoke2Tex);
	gpSmoke2Tex = nil;
	 
	for ( int32 i = 0; i < MAX_RUBBER_FILES; i++ )
	{
		RwTextureDestroy(gpRubberTex[i]);
		gpRubberTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_RAINSPLASH_FILES; i++ )
	{
		RwTextureDestroy(gpRainSplashTex[i]);
		gpRainSplashTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_WATERSPRAY_FILES; i++ )
	{
		RwTextureDestroy(gpWatersprayTex[i]);
		gpWatersprayTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_EXPLOSIONMEDIUM_FILES; i++ )
	{
		RwTextureDestroy(gpExplosionMediumTex[i]);
		gpExplosionMediumTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_GUNFLASH_FILES; i++ )
	{
		RwTextureDestroy(gpGunFlashTex[i]);
		gpGunFlashTex[i] = nil;
	}
	
	RwTextureDestroy(gpRainDropTex);
	gpRainDropTex = nil;
	
	for ( int32 i = 0; i < MAX_RAINSPLASHUP_FILES; i++ )
	{
		RwTextureDestroy(gpRainSplashupTex[i]);
		gpRainSplashupTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_BIRDFRONT_FILES; i++ )
	{
		RwTextureDestroy(gpBirdfrontTex[i]);
		gpBirdfrontTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_BOAT_FILES; i++ )
	{
		RwTextureDestroy(gpBoatTex[i]);
		gpBoatTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_CARDEBRIS_FILES; i++ )
	{
		RwTextureDestroy(gpCarDebrisTex[i]);
		gpCarDebrisTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_CARSPLASH_FILES; i++ )
	{
		RwTextureDestroy(gpCarSplashTex[i]);
		gpCarSplashTex[i] = nil;
	}
	
	for ( int32 i = 0; i < MAX_RAINDRIP_FILES; i++ )
	{
		RwTextureDestroy(gpRaindripTex1[i]);
		gpRaindripTex1[i] = nil;

		RwTextureDestroy(gpRaindripTex2[i]);
		gpRaindripTex2[i] = nil;
	}
	
	RwTextureDestroy(gpBoatWakeTex);
	gpBoatWakeTex = nil;

	RwTextureDestroy(gpFlame1Tex);
	gpFlame1Tex = nil;

	RwTextureDestroy(gpFlame5Tex);
	gpFlame5Tex = nil;
	
	RwTextureDestroy(gpRainDropSmallTex);
	gpRainDropSmallTex = nil;
	
	RwTextureDestroy(gpBloodTex);
	gpBloodTex = nil;
	
	RwTextureDestroy(gpLeafTex);
	gpLeafTex = nil;

	RwTextureDestroy(gpLetterTex);
	gpLetterTex = nil;
	
	RwTextureDestroy(gpCloudTex1);
	gpCloudTex1 = nil;
	
	RwTextureDestroy(gpCloudTex4);
	gpCloudTex4 = nil;
	
	RwTextureDestroy(gpBloodSmallTex);
	gpBloodSmallTex = nil;
	
	RwTextureDestroy(gpGungeTex);
	gpGungeTex = nil;
	
	RwTextureDestroy(gpCollisionSmokeTex);
	gpCollisionSmokeTex = nil;
	
	RwTextureDestroy(gpBulletHitTex);
	gpBulletHitTex = nil;
	
	RwTextureDestroy(gpGunShellTex);
	gpGunShellTex = nil;
	
	RwTextureDestroy(gpPointlightTex);
	gpPointlightTex = nil;

	RwTextureDestroy(gpSparkTex);
	gpSparkTex = nil;
	
	RwTextureDestroy(gpNewspaperTex);
	gpNewspaperTex = nil;

	RwTextureDestroy(gpGunSmokeTex);
	gpGunSmokeTex = nil;
	
	RwTextureDestroy(gpDotTex);
	gpDotTex = nil;
	RwTextureDestroy(gpHeathazeTex);
	gpHeathazeTex = nil;
	
	RwTextureDestroy(gpBeastieTex);
	gpBeastieTex = nil;

	int32 slot;

	slot = CTxdStore::FindTxdSlot("particle");
	CTxdStore::RemoveTxdSlot(slot);

	debug("CParticle shut down");
}


void CParticle::AddParticlesAlongLine(tParticleType type, CVector const &vecStart, CVector const &vecEnd, CVector const &vecDir, float fPower, CEntity *pEntity, float fSize, int32 nRotationSpeed, int32 nRotation, int32 nCurFrame, int32 nLifeSpan)
{
	CVector vecDist = vecEnd - vecStart;
	float fDist = vecDist.Magnitude();
	float fSteps = Max(fDist / fPower, 1.0f);
	int32 nSteps = (int32)fSteps;

	CVector vecStep = vecDist * (1.0f / (float)nSteps); 

	for ( int32 i = 0; i < nSteps; i++ )
	{
		CVector vecPos = float(i) * vecStep + vecStart;
		AddParticle(type, vecPos, vecDir, pEntity, fSize, nRotationSpeed, nRotation, nCurFrame, nLifeSpan);
	}
}

void CParticle::AddParticlesAlongLine(tParticleType type, CVector const &vecStart, CVector const &vecEnd, CVector const &vecDir, float fPower, CEntity *pEntity, float fSize, RwRGBA const &color, int32 nRotationSpeed, int32 nRotation, int32 nCurFrame, int32 nLifeSpan)
{
	CVector vecDist = vecEnd - vecStart;
	float fDist = vecDist.Magnitude();
	float fSteps = Max(fDist / fPower, 1.0f);
	int32 nSteps = (int32)fSteps;
	
	CVector vecStep = vecDist * (1.0f / (float)nSteps); 

	for ( int32 i = 0; i < nSteps; i++ )
	{		
		CVector vecPos = float(i) * vecStep + vecStart;
		
		AddParticle(type, vecPos, vecDir, pEntity, fSize, color, nRotationSpeed, nRotation, nCurFrame, nLifeSpan);
	}
}

CParticle *CParticle::AddParticle(tParticleType type, CVector const &vecPos, CVector const &vecDir, CEntity *pEntity, float fSize, int32 nRotationSpeed, int32 nRotation, int32 nCurFrame, int32 nLifeSpan)
{
	CRGBA color(0, 0, 0, 0);
	return AddParticle(type, vecPos, vecDir, pEntity, fSize, color, nRotationSpeed, nRotation, nCurFrame, nLifeSpan);
}

CParticle *CParticle::AddParticle(tParticleType type, CVector const &vecPos, CVector const &vecDir, CEntity *pEntity, float fSize, RwRGBA const &color, int32 nRotationSpeed, int32 nRotation, int32 nCurFrame, int32 nLifeSpan)
{
	if ( CTimer::GetIsPaused() )
		return nil;

	if ( ( type == PARTICLE_ENGINE_SMOKE
		|| type == PARTICLE_ENGINE_SMOKE2
		|| type == PARTICLE_ENGINE_STEAM
		|| type == PARTICLE_CARFLAME_SMOKE
		|| type == PARTICLE_RUBBER_SMOKE
		|| type == PARTICLE_BURNINGRUBBER_SMOKE
		|| type == PARTICLE_EXHAUST_FUMES
		|| type == PARTICLE_CARCOLLISION_DUST )
		&& nParticleCreationInterval & CTimer::GetFrameCounter() )
	{
		return nil;
	}

	if ( !CReplay::IsPlayingBack() )
		CReplay::RecordParticle(type, vecPos, vecDir, fSize, color);
	
	CParticle *pParticle = m_pUnusedListHead;
	
	if ( pParticle == nil )
		return nil;
	
	tParticleSystemData *psystem = &mod_ParticleSystemManager.m_aParticles[type];
	
	if ( psystem->m_fCreateRange != 0.0f && psystem->m_fCreateRange < ( TheCamera.GetPosition() - vecPos ).MagnitudeSqr() )
		return nil;
	
	
	pParticle->m_fSize = psystem->m_fDefaultInitialRadius;
	pParticle->m_fExpansionRate = psystem->m_fExpansionRate;
	
	if ( nLifeSpan != 0 )
		pParticle->m_nTimeWhenWillBeDestroyed = CTimer::GetTimeInMilliseconds() + nLifeSpan;
	else
		pParticle->m_nTimeWhenWillBeDestroyed = CTimer::GetTimeInMilliseconds() + psystem->m_nLifeSpan;

	pParticle->m_nColorIntensity = psystem->m_nFadeToBlackInitialIntensity;
	
	pParticle->m_nFadeToBlackTimer = psystem->m_nFadeToBlackAmount;
	
	if ( psystem->m_nFadeToBlackTime )
		pParticle->m_nFadeToBlackTimer /= psystem->m_nFadeToBlackTime;
	
	pParticle->m_nAlpha = psystem->m_nFadeAlphaInitialIntensity;
	
	pParticle->m_nFadeAlphaTimer = psystem->m_nFadeAlphaAmount;
	
	if ( psystem->m_nFadeAlphaTime )
		pParticle->m_nFadeAlphaTimer /= psystem->m_nFadeAlphaTime;
	
	pParticle->m_nCurrentZRotation = psystem->m_nZRotationInitialAngle;
	pParticle->m_fCurrentZRadius = psystem->m_fInitialZRadius;
	
	if ( nCurFrame != 0 )
		pParticle->m_nCurrentFrame = nCurFrame;
	else
		pParticle->m_nCurrentFrame = psystem->m_nStartAnimationFrame;
	
	
	pParticle->m_nZRotationTimer = 0;
	pParticle->m_nZRadiusTimer = 0;
	pParticle->m_nAnimationSpeedTimer = 0;
	pParticle->m_fZGround = 0.0f;
	
	if ( type != PARTICLE_HEATHAZE )
		pParticle->m_vecPosition = vecPos;
	else
	{
		CVector screen;
		float w, h;

		if ( !CSprite::CalcScreenCoors(vecPos, screen, &w, &h, true) )
			return nil;
		
		pParticle->m_vecPosition = screen;
		psystem->m_vecTextureStretch.x = w;
		psystem->m_vecTextureStretch.y = h;
	}
	
	pParticle->m_vecVelocity = vecDir;
	
	pParticle->m_vecParticleMovementOffset = CVector(0.0f, 0.0f, 0.0f);
	pParticle->m_nTimeWhenColorWillBeChanged = 0;
	
	if ( color.alpha != 0 )
		RwRGBAAssign(&pParticle->m_Color, &color);
	else
	{
		RwRGBAAssign(&pParticle->m_Color, psystem->m_RenderColouring);

		if ( psystem->m_ColorFadeTime != 0 )
			pParticle->m_nTimeWhenColorWillBeChanged = CTimer::GetTimeInMilliseconds() + psystem->m_ColorFadeTime;

		if ( psystem->m_InitialColorVariation != 0 )
		{
			int32 ColorVariation = CGeneral::GetRandomNumberInRange(-psystem->m_InitialColorVariation, psystem->m_InitialColorVariation);
			//float ColorVariation = CGeneral::GetRandomNumberInRange((float)-psystem->m_InitialColorVariation, (float)psystem->m_InitialColorVariation);
  
			pParticle->m_Color.red   = clamp(pParticle->m_Color.red +
				PERCENT(pParticle->m_Color.red, ColorVariation),
				0, 255);
			
			pParticle->m_Color.green = clamp(pParticle->m_Color.green +
				PERCENT(pParticle->m_Color.green, ColorVariation),
				0, 255);
			
			pParticle->m_Color.blue  = clamp(pParticle->m_Color.blue +
				PERCENT(pParticle->m_Color.blue, ColorVariation),
				0, 255);
		}
	}

	pParticle->m_nRotation = nRotation;
		
	if ( nRotationSpeed != 0 )
		pParticle->m_nRotationStep = nRotationSpeed;
	else
		pParticle->m_nRotationStep = psystem->m_nRotationSpeed;
	
	if ( CGeneral::GetRandomNumber() & 1 )
		pParticle->m_nRotationStep = -pParticle->m_nRotationStep;
	
	if ( psystem->m_fPositionRandomError != 0.0f )
	{
		pParticle->m_vecPosition.x += psystem->m_fPositionRandomError * ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE];
		pParticle->m_vecPosition.y += psystem->m_fPositionRandomError * ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE];
		
		if ( psystem->Flags & RAND_VERT_V )
			pParticle->m_vecPosition.z += psystem->m_fPositionRandomError * ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE];
	}
	
	if ( psystem->m_fVelocityRandomError != 0.0f )
	{
		pParticle->m_vecVelocity.x += psystem->m_fVelocityRandomError * ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE];
		pParticle->m_vecVelocity.y += psystem->m_fVelocityRandomError * ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE];
    
		if ( psystem->Flags & RAND_VERT_V )
			pParticle->m_vecVelocity.z += psystem->m_fVelocityRandomError * ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE];
	}
	
	if ( psystem->m_fExpansionRateError != 0.0f && !(psystem->Flags & SCREEN_TRAIL) )
		pParticle->m_fExpansionRate += psystem->m_fExpansionRateError * ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE] + psystem->m_fExpansionRateError;
	
	if ( psystem->m_nRotationRateError != 0 )
		pParticle->m_nRotationStep += CGeneral::GetRandomNumberInRange(-psystem->m_nRotationRateError, psystem->m_nRotationRateError);

	if ( psystem->m_nLifeSpanErrorShape != 0 )
	{
		float randVal = ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE];
		if ( randVal > 0.0f )
			pParticle->m_nTimeWhenWillBeDestroyed += int32(float(psystem->m_nLifeSpan) * randVal * float(psystem->m_nLifeSpanErrorShape));
		else
			pParticle->m_nTimeWhenWillBeDestroyed += int32(float(psystem->m_nLifeSpan) * randVal / float(psystem->m_nLifeSpanErrorShape));
	}
	
	if ( psystem->Flags & ZCHECK_FIRST )
	{
		static bool bValidGroundFound = false;
		static CVector LastTestCoors;
		static float LastTestGroundZ;
		
		if ( bValidGroundFound 
			&& vecPos.x == LastTestCoors.x 
			&& vecPos.y == LastTestCoors.y 
			&& vecPos.z == LastTestCoors.z )
		{
			pParticle->m_fZGround = LastTestGroundZ;
		}
		else
		{
			bValidGroundFound = false;
			
			CColPoint point;
			CEntity *entity;
			
			if ( !CWorld::ProcessVerticalLine(
						pParticle->m_vecPosition + CVector(0.0f, 0.0f, 0.5f),
						-100.0f, point, entity, true, true, false, false, true, false, nil) )
			{
				return nil;
			}
			
			if ( point.point.z >= pParticle->m_vecPosition.z )
				return nil;
			
			pParticle->m_fZGround = point.point.z;
			bValidGroundFound = true;
			LastTestCoors = vecPos;
			LastTestGroundZ = point.point.z;
		}
	}
	
	if ( psystem->Flags & ZCHECK_BUMP )
	{
		static float Z_Ground = 0.0f;
		
		if ( psystem->Flags & ZCHECK_BUMP_FIRST )
		{
			bool bZFound = false;

			Z_Ground = CWorld::FindGroundZFor3DCoord(vecPos.x, vecPos.y, vecPos.z, (bool *)&bZFound);

			if ( bZFound == false )
				return nil;

			pParticle->m_fZGround = Z_Ground;
		}
		
		pParticle->m_fZGround = Z_Ground;
	}
	
	switch ( type )
	{
		case PARTICLE_DEBRIS:
			pParticle->m_vecVelocity.z *= CGeneral::GetRandomNumberInRange(0.5f, 3.0f);
			break;
		
		case PARTICLE_EXPLOSION_MEDIUM:
			pParticle->m_nColorIntensity -= 30 * (CGeneral::GetRandomNumber() & 1); // mb "+= -30 * rand" here ?
			pParticle->m_nAnimationSpeedTimer = CGeneral::GetRandomNumber() & 7;
			pParticle->m_fSize = CGeneral::GetRandomNumberInRange(0.3f, 0.8f);
			pParticle->m_vecPosition.z -= CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
			break;
		
		case PARTICLE_EXPLOSION_LARGE:
			pParticle->m_nColorIntensity -= 30 * (CGeneral::GetRandomNumber() & 1); // mb "+= -30 * rand" here ?
			pParticle->m_nAnimationSpeedTimer = CGeneral::GetRandomNumber() & 7;
			pParticle->m_fSize = CGeneral::GetRandomNumberInRange(0.8f, 1.4f);
			pParticle->m_vecPosition.z -= CGeneral::GetRandomNumberInRange(-0.3f, 0.3f);
			break;
		
		case PARTICLE_WATER_HYDRANT:
			pParticle->m_vecPosition.z += 20.0f * psystem->m_fPositionRandomError * ms_afRandTable[CGeneral::GetRandomNumber() % RAND_TABLE_SIZE];
			break;
		default: break;
	}
	
	if ( fSize != 0.0f )
		pParticle->m_fSize = fSize;
	
	m_pUnusedListHead = pParticle->m_pNext;

	pParticle->m_pNext = psystem->m_pParticles;

	psystem->m_pParticles = pParticle;
	
	return pParticle;
}

void CParticle::Update()
{
	if ( CTimer::GetIsPaused() )
		return;

	CRGBA color(0, 0, 0, 0);
	
	float fFricDeccel50 = pow(0.50f, CTimer::GetTimeStep());
	float fFricDeccel80 = pow(0.80f, CTimer::GetTimeStep());
	float fFricDeccel90 = pow(0.90f, CTimer::GetTimeStep());
	float fFricDeccel95 = pow(0.95f, CTimer::GetTimeStep());
	float fFricDeccel96 = pow(0.96f, CTimer::GetTimeStep());
	float fFricDeccel99 = pow(0.99f, CTimer::GetTimeStep());
	
	CParticleObject::UpdateAll();

	for ( int32 i = 0; i < MAX_PARTICLES; i++ )
	{
		tParticleSystemData *psystem = &mod_ParticleSystemManager.m_aParticles[i];
		CParticle *particle = psystem->m_pParticles;
		CParticle *prevParticle = nil;
		bool bRemoveParticle;
		
		if ( particle == nil )
			continue;
				
		for ( ; particle != nil; _Next(particle, prevParticle, psystem, bRemoveParticle) )
		{
			CVector vecWind(0.0f, 0.0f, 0.0f);
			
			bRemoveParticle = false;

			CVector vecMoveStep = particle->m_vecVelocity * CTimer::GetTimeStep();
			CVector vecPos = particle->m_vecPosition;
			
			if ( numWaterDropOnScreen == 0 )
				clearWaterDrop = false;
			
			if ( psystem->m_Type == PARTICLE_WATERDROP )
			{
				if ( CGame::IsInInterior() || clearWaterDrop == true )
				{
					bRemoveParticle = true;
					continue;
				}
				
				static uint8 nWaterDropCount;

				if ( nWaterDropCount == 5 )
				{
					vecMoveStep = CVector(0.0f, 0.0f, 0.0f);
					particle->m_nTimeWhenWillBeDestroyed += 1250;
					nWaterDropCount = 0;
				}
				else
				{
					if ( TheCamera.m_CameraAverageSpeed > 0.35f )
					{
						if ( vecMoveStep.Magnitude() > 0.5f )
						{
							if ( vecMoveStep.Magnitude() > 0.4f && vecMoveStep.Magnitude() < 0.8f )
							{
								vecMoveStep.x += TheCamera.m_CameraAverageSpeed * 1.5f;
								vecMoveStep.y += TheCamera.m_CameraAverageSpeed * 1.5f;
							}
							else if ( vecMoveStep.Magnitude() != 0.0f )
							{
								vecMoveStep.x += CGeneral::GetRandomNumberInRange(0.01f, 0.05f);
								vecMoveStep.y += CGeneral::GetRandomNumberInRange(0.01f, 0.05f);
							}
						}
					}
					
					nWaterDropCount++;
				}
				
				if ( vecPos.z <= 1.5f )
					vecMoveStep.z = 0.0f;
			}
			
			if ( psystem->m_Type == PARTICLE_HEATHAZE || psystem->m_Type == PARTICLE_HEATHAZE_IN_DIST )
			{
				int32 nSinCosIndex = int32(DEGTORAD((float)particle->m_nRotation) * float(SIN_COS_TABLE_SIZE) / TWOPI) % SIN_COS_TABLE_SIZE;
				
				vecMoveStep.x = Sin(nSinCosIndex);
				vecMoveStep.y = Sin(nSinCosIndex);
				
				if ( psystem->m_Type == PARTICLE_HEATHAZE_IN_DIST )
					particle->m_nRotation = int16((float)particle->m_nRotation + 0.75f);
				else
					particle->m_nRotation = int16((float)particle->m_nRotation + 1.0f);
			}
			
			if ( psystem->m_Type == PARTICLE_BEASTIE )
			{
				int32 nSinCosIndex = int32(DEGTORAD((float)particle->m_nRotation) * float(SIN_COS_TABLE_SIZE) / TWOPI) % SIN_COS_TABLE_SIZE;
				
				particle->m_vecVelocity.x = 0.50f * Cos(nSinCosIndex);
				particle->m_vecVelocity.y = Cos(nSinCosIndex);
				particle->m_vecVelocity.z = 0.25f * Sin(nSinCosIndex);
				
				if ( particle->m_vecVelocity.Magnitude() > 2.0f
						|| vecPos.z > 40.0f
						|| (TheCamera.GetPosition() - vecPos).Magnitude() < 60.0f
					)
				{
					bRemoveParticle = true;
					continue;
				}
			}
			
			vecPos += vecMoveStep;
			
			if ( psystem->m_Type == PARTICLE_FIREBALL )
			{
				  AddParticle(PARTICLE_HEATHAZE, particle->m_vecPosition, CVector(0.0f, 0.0f, 0.0f),
					nil, particle->m_fSize * 5.0f);
			}
			
			if ( psystem->m_Type == PARTICLE_GUNSMOKE2 )
			{
				if ( CTimer::GetFrameCounter() & 10 )
				{
					if ( FindPlayerPed()->GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN )
					{
						AddParticle(PARTICLE_HEATHAZE, particle->m_vecPosition, CVector(0.0f, 0.0f, 0.0f));
					}
				}
			}
			
			if ( CWeather::Wind > 0.0f )
			{
				if ( vecMoveStep.Magnitude() != 0.0f )
				{
					vecWind.x = CGeneral::GetRandomNumberInRange(0.75f, 1.25f) * -CWeather::Wind;
					vecWind.y = CGeneral::GetRandomNumberInRange(0.75f, 1.25f) * -CWeather::Wind;
					vecWind *= PARTICLE_WIND_TEST_SCALE * psystem->m_fWindFactor * CTimer::GetTimeStep();
					particle->m_vecVelocity += vecWind;
				}
			}
			
			if ( psystem->m_Type == PARTICLE_RAINDROP
				|| psystem->m_Type == PARTICLE_RAINDROP_SMALL
				|| psystem->m_Type == PARTICLE_RAIN_SPLASH
				|| psystem->m_Type == PARTICLE_RAIN_SPLASH_BIGGROW
				|| psystem->m_Type == PARTICLE_CAR_SPLASH
				|| psystem->m_Type == PARTICLE_BOAT_SPLASH
				|| psystem->m_Type == PARTICLE_RAINDROP_2D )
			{
				int32 nMaxDrops = int32(6.0f * TheCamera.m_CameraAverageSpeed + 1.0f);
				float fDistToCam = 0.0f;
				
				if ( psystem->m_Type == PARTICLE_BOAT_SPLASH || psystem->m_Type == PARTICLE_CAR_SPLASH )
				{
					if ( vecPos.z + particle->m_fSize < 5.0f )
					{
						bRemoveParticle = true;
						continue;
					}
					
					switch ( TheCamera.GetLookDirection() )
					{
						case LOOKING_LEFT:
						case LOOKING_RIGHT:
						case LOOKING_FORWARD:
							nMaxDrops /= 2;
							break;
						
						default:
							nMaxDrops = 0;
							break;
					}
					
					fDistToCam = (TheCamera.GetPosition() - vecPos).Magnitude();
				}

				if ( numWaterDropOnScreen < nMaxDrops && numWaterDropOnScreen < 63
					&& fDistToCam < 10.0f
					&& clearWaterDrop == false
					&& !CGame::IsInInterior() )
				{
					CVector vecWaterdropTarget
					(
						CGeneral::GetRandomNumberInRange(-0.25f, 0.25f),
						CGeneral::GetRandomNumberInRange(0.1f, 0.75f),
						-0.01f
					);
					
					CVector vecWaterdropPos;
					
					if ( TheCamera.m_CameraAverageSpeed < 0.35f )
						vecWaterdropPos.x = (float)CGeneral::GetRandomNumberInRange(50, int32(SCREEN_WIDTH) - 50);
					else
						vecWaterdropPos.x = (float)CGeneral::GetRandomNumberInRange(200, int32(SCREEN_WIDTH) - 200);
					
					if ( psystem->m_Type == PARTICLE_BOAT_SPLASH || psystem->m_Type == PARTICLE_CAR_SPLASH )
						vecWaterdropPos.y = (float)CGeneral::GetRandomNumberInRange(SCREEN_HEIGHT / 2, SCREEN_HEIGHT);
					else
					{
						if ( TheCamera.m_CameraAverageSpeed < 0.35f )
							vecWaterdropPos.y  = (float)CGeneral::GetRandomNumberInRange(0, int32(SCREEN_HEIGHT));
						else
							vecWaterdropPos.y  = (float)CGeneral::GetRandomNumberInRange(150, int32(SCREEN_HEIGHT) - 200);
					}
					
					vecWaterdropPos.z = 2.0f;

					if ( AddParticle(PARTICLE_WATERDROP,
										vecWaterdropPos,
										vecWaterdropTarget,
										nil,
										CGeneral::GetRandomNumberInRange(0.1f, 0.15f),
										0,
										0,
										CGeneral::GetRandomNumber() & 1,
										0) != nil )
					{
						numWaterDropOnScreen++;
					}
				}
			}
			
			if (  CTimer::GetTimeInMilliseconds() > particle->m_nTimeWhenWillBeDestroyed || particle->m_nAlpha == 0 )
			{
				bRemoveParticle = true;
				continue;
			}

			if ( particle->m_nTimeWhenColorWillBeChanged != 0 )
			{
				if ( particle->m_nTimeWhenColorWillBeChanged > CTimer::GetTimeInMilliseconds() )
				{
					float colorMul = 1.0f - float(particle->m_nTimeWhenColorWillBeChanged - CTimer::GetTimeInMilliseconds()) / float(psystem->m_ColorFadeTime);
				
					particle->m_Color.red = clamp(
						psystem->m_RenderColouring.red + int32(float(psystem->m_FadeDestinationColor.red - psystem->m_RenderColouring.red) * colorMul),
						0, 255);
					
					particle->m_Color.green = clamp(
						psystem->m_RenderColouring.green + int32(float(psystem->m_FadeDestinationColor.green - psystem->m_RenderColouring.green) * colorMul),
						0, 255);
						
					particle->m_Color.blue = clamp(
						psystem->m_RenderColouring.blue + int32(float(psystem->m_FadeDestinationColor.blue - psystem->m_RenderColouring.blue) * colorMul),
						0, 255);
				}
				else
					RwRGBAAssign(&particle->m_Color, psystem->m_FadeDestinationColor);
			}
			
			if ( psystem->Flags & CLIPOUT2D )
			{
				if ( particle->m_vecPosition.x < -10.0f || particle->m_vecPosition.x > SCREEN_WIDTH + 10.0f
					|| particle->m_vecPosition.y < -10.0f || particle->m_vecPosition.y > SCREEN_HEIGHT + 10.0f )
				{
					bRemoveParticle = true;
					continue;
				}
			}
			
			if ( !(psystem->Flags & SCREEN_TRAIL) )
			{
				float size;

				if ( particle->m_fExpansionRate > 0.0f )
				{
					float speed = Max(vecWind.Magnitude(), vecMoveStep.Magnitude());
					
					if ( psystem->m_Type == PARTICLE_EXHAUST_FUMES || psystem->m_Type == PARTICLE_ENGINE_STEAM )
						speed *= 2.0f;
					
					if ( ( psystem->m_Type == PARTICLE_BOAT_SPLASH || psystem->m_Type == PARTICLE_CAR_SPLASH )
							&& particle->m_fSize > 1.2f )
					{
						size = particle->m_fSize - (1.0f + speed) * particle->m_fExpansionRate;
						particle->m_vecVelocity.z -= 0.15f;
					}
					else
						size = particle->m_fSize + (1.0f + speed) * particle->m_fExpansionRate;
				}
				else
					size = particle->m_fSize + particle->m_fExpansionRate;
				
				if ( psystem->m_Type == PARTICLE_WATERDROP )
					size = (size - Abs(vecMoveStep.x) * 0.000150000007f) + (Abs(vecMoveStep.z) * 0.0500000007f); //TODO:
				
				if ( size < 0.0f )
				{
					bRemoveParticle = true;
					continue;
				}
				
				particle->m_fSize = size;
			}
			
			switch ( psystem->m_nFrictionDecceleration )
			{
				case 50:
					particle->m_vecVelocity *= fFricDeccel50;
					break;
		
				case 80:
					particle->m_vecVelocity *= fFricDeccel80;
					break;
		
				case 90:
					particle->m_vecVelocity *= fFricDeccel90;
					break;
		
				case 95:
					particle->m_vecVelocity *= fFricDeccel95;
					break;
		
				case 96:
					particle->m_vecVelocity *= fFricDeccel96;
					break;
		
				case 99:
					particle->m_vecVelocity *= fFricDeccel99;
					break;				
			}
			
			if ( psystem->m_fGravitationalAcceleration > 0.0f )
			{
				if ( -50.0f * psystem->m_fGravitationalAcceleration < particle->m_vecVelocity.z )
					particle->m_vecVelocity.z -= psystem->m_fGravitationalAcceleration * CTimer::GetTimeStep();

				if ( psystem->Flags & ZCHECK_FIRST )
				{
					if ( particle->m_vecPosition.z < particle->m_fZGround )
					{
						switch ( psystem->m_Type )
						{
							case PARTICLE_RAINDROP:
							case PARTICLE_RAINDROP_SMALL:
								{
									bRemoveParticle = true;
									
									if ( CGeneral::GetRandomNumber() & 1 )
									{
										AddParticle(PARTICLE_RAIN_SPLASH,
													CVector
													(
														particle->m_vecPosition.x,
														particle->m_vecPosition.y,
														0.05f + particle->m_fZGround
													),
													CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, 0, 0, 0, 0);
									}
									else
									{
										AddParticle(PARTICLE_RAIN_SPLASHUP,
													CVector
													(
														particle->m_vecPosition.x,
														particle->m_vecPosition.y,
														0.05f + particle->m_fZGround
													),
													CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, 0, 0, 0, 0);
									}
									
									continue;
								}
								break;

							case PARTICLE_WHEEL_WATER:
								{
									bRemoveParticle = true;
									
									int32 randVal = CGeneral::GetRandomNumber();
									
									if ( randVal & 1 )
									{
										if ( (randVal % 5) == 0 )
										{
											AddParticle(PARTICLE_RAIN_SPLASH,
														CVector
														(
															particle->m_vecPosition.x,
															particle->m_vecPosition.y,
															0.05f + particle->m_fZGround
														),
														CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, 0, 0, 0, 0);
										}
										else
										{
											AddParticle(PARTICLE_RAIN_SPLASHUP,
														CVector
														(
															particle->m_vecPosition.x,
															particle->m_vecPosition.y,
															0.05f + particle->m_fZGround
														),
														CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, 0, 0, 0, 0);
										}
										
									}
									continue;
								}
								break;

							case PARTICLE_BLOOD:
							case PARTICLE_BLOOD_SMALL:
								{
									bRemoveParticle = true;
									
									CVector vecPosn = particle->m_vecPosition;
									vecPosn.z += 1.0f;
									
									Randomizer++;
									int32 randVal = int32(Randomizer & 7);
									
									if ( randVal == 5 )
									{
										CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &vecPosn,
												0.1f, 0.0f, 0.0f, -0.1f,
												255,
												255, 0, 0,
												4.0f, (CGeneral::GetRandomNumber() & 4095) + 2000, 1.0f);
									}
									else if ( randVal == 2 )
									{
										CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpBloodPoolTex, &vecPosn,
												0.2f, 0.0f, 0.0f, -0.2f,
												255,
												255, 0, 0,
												4.0f, (CGeneral::GetRandomNumber() & 4095) + 8000, 1.0f);
									}
									continue;
								}
								break;
							default: break;
						}
					}
				}
				else if ( psystem->Flags & ZCHECK_STEP )
				{
					CColPoint point;
					CEntity *entity;

					if ( CWorld::ProcessVerticalLine(particle->m_vecPosition, vecPos.z, point, entity, 
														true, true, false, false, true, false, nil) )
					{
						if ( vecPos.z <= point.point.z )
						{
							vecPos.z = point.point.z;
							if ( psystem->m_Type == PARTICLE_DEBRIS2 )
							{
								particle->m_vecVelocity.x *= 0.8f;
								particle->m_vecVelocity.y *= 0.8f;
								particle->m_vecVelocity.z *= -0.4f;
								if ( particle->m_vecVelocity.z < 0.005f )
									particle->m_vecVelocity.z = 0.0f;
							}
						}
					}
				}
				else if ( psystem->Flags & ZCHECK_BUMP )
				{
					if ( particle->m_vecPosition.z < particle->m_fZGround )
					{
						switch ( psystem->m_Type )
						{
							case PARTICLE_GUNSHELL_FIRST:
							case PARTICLE_GUNSHELL:
								{
									bRemoveParticle = true;

									AddParticle(PARTICLE_GUNSHELL_BUMP1,
												CVector
												(
													particle->m_vecPosition.x,
													particle->m_vecPosition.y,
													0.05f + particle->m_fZGround
												),
												CVector
												(
													CGeneral::GetRandomNumberInRange(-0.02f, 0.02f),
													CGeneral::GetRandomNumberInRange(-0.02f, 0.02f),
													CGeneral::GetRandomNumberInRange(0.05f, 0.1f)
												),
												nil,
												particle->m_fSize, color, particle->m_nRotationStep, 0, 0, 0);
									
									PlayOneShotScriptObject(SCRIPT_SOUND_GUNSHELL_DROP, particle->m_vecPosition);
								}
								break;
							
							case PARTICLE_GUNSHELL_BUMP1:
								{
									bRemoveParticle = true;
									
									AddParticle(PARTICLE_GUNSHELL_BUMP2,
												CVector
												(
													particle->m_vecPosition.x,
													particle->m_vecPosition.y,
													0.05f + particle->m_fZGround
												),
												CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.03f, 0.06f)),
												nil,
												particle->m_fSize, color, 0, 0, 0, 0);
									
									PlayOneShotScriptObject(SCRIPT_SOUND_GUNSHELL_DROP_SOFT, particle->m_vecPosition);
								}
								break;
								
							case PARTICLE_GUNSHELL_BUMP2:
								{
									bRemoveParticle = true;
									continue;
								}
								break;
							default: break;
						}
					}
				}
			}
			else
			{
				if ( psystem->m_fGravitationalAcceleration < 0.0f )
				{
					if ( -5.0f * psystem->m_fGravitationalAcceleration > particle->m_vecVelocity.z )
						particle->m_vecVelocity.z -= psystem->m_fGravitationalAcceleration * CTimer::GetTimeStep();
				}
				else
				{
					if ( psystem->Flags & ZCHECK_STEP )
					{
						CColPoint point;
						CEntity *entity;
			
						if ( CWorld::ProcessVerticalLine(particle->m_vecPosition, vecPos.z, point, entity,
														true, false, false, false, true, false, nil) )
						{
							if ( vecPos.z <= point.point.z )
							{
								vecPos.z = point.point.z;
								if ( psystem->m_Type == PARTICLE_HELI_ATTACK )
								{
									bRemoveParticle = true;
									AddParticle(PARTICLE_STEAM, vecPos, CVector(0.0f, 0.0f, 0.05f), nil, 0.2f, 0, 0, 0, 0);
									continue;
								}
							}
						}
					}
				}
			}

			if ( particle->m_nFadeToBlackTimer != 0 )
			{
				particle->m_nColorIntensity = clamp(particle->m_nColorIntensity - particle->m_nFadeToBlackTimer,
														0, 255);
			}

			if ( particle->m_nFadeAlphaTimer != 0 )
			{
				particle->m_nAlpha = clamp(particle->m_nAlpha - particle->m_nFadeAlphaTimer,
														0, 255);
				if ( particle->m_nAlpha == 0 )
				{
					bRemoveParticle = true;
					continue;
				}
			}
			
			if ( psystem->m_nZRotationAngleChangeAmount != 0 )
			{
				if ( particle->m_nZRotationTimer >= psystem->m_nZRotationChangeTime )
				{
					particle->m_nZRotationTimer = 0;
					particle->m_nCurrentZRotation += psystem->m_nZRotationAngleChangeAmount;
				}
				else
					++particle->m_nZRotationTimer;
			}
			
			if ( psystem->m_fZRadiusChangeAmount != 0.0f )
			{
				if ( particle->m_nZRadiusTimer >= psystem->m_nZRadiusChangeTime )
				{
					particle->m_nZRadiusTimer = 0;
					particle->m_fCurrentZRadius += psystem->m_fZRadiusChangeAmount;
				}
				else
					++particle->m_nZRadiusTimer;
			}

			if ( psystem->m_nAnimationSpeed != 0 )
			{
				if ( particle->m_nAnimationSpeedTimer > psystem->m_nAnimationSpeed )
				{
					particle->m_nAnimationSpeedTimer = 0;
					
					if ( ++particle->m_nCurrentFrame > psystem->m_nFinalAnimationFrame )
					{
						if ( psystem->Flags & CYCLE_ANIM )
							particle->m_nCurrentFrame = psystem->m_nStartAnimationFrame;
						else
							--particle->m_nCurrentFrame;
					}	
				}
				else
					++particle->m_nAnimationSpeedTimer;
			}
			
			if ( particle->m_nRotationStep != 0 )
				particle->m_nRotation += particle->m_nRotationStep;
			
			if ( particle->m_fCurrentZRadius != 0.0f )
			{
				int32 nSinCosIndex = particle->m_nCurrentZRotation % SIN_COS_TABLE_SIZE;
				
				float fX = (Cos(nSinCosIndex) - Sin(nSinCosIndex)) * particle->m_fCurrentZRadius;
				
				float fY = (Sin(nSinCosIndex) + Cos(nSinCosIndex)) * particle->m_fCurrentZRadius;

				vecPos -= particle->m_vecParticleMovementOffset;

				vecPos += CVector(fX, fY, 0.0f);
				
				particle->m_vecParticleMovementOffset = CVector(fX, fY, 0.0f);
			}
			
			particle->m_vecPosition = vecPos;
		}
	}
}

void CParticle::Render()
{
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSWRAP);
	RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	
	CSprite::InitSpriteBuffer2D();
	
	uint32 flags = DRAW_OPAQUE;
	
	RwRaster *prevFrame = nil;
	
	for ( int32 i = 0; i < MAX_PARTICLES; i++ )
	{
		tParticleSystemData *psystem = &mod_ParticleSystemManager.m_aParticles[i];
		bool particleBanned = false;
		CParticle *particle = psystem->m_pParticles;
		
		RwRaster **frames = psystem->m_ppRaster;
		tParticleType type = psystem->m_Type;

		if ( type == PARTICLE_ENGINE_SMOKE
			|| type == PARTICLE_ENGINE_SMOKE2
			|| type == PARTICLE_ENGINE_STEAM
			|| type == PARTICLE_CARFLAME_SMOKE
			|| type == PARTICLE_RUBBER_SMOKE
			|| type == PARTICLE_BURNINGRUBBER_SMOKE
			|| type == PARTICLE_EXHAUST_FUMES
			|| type == PARTICLE_CARCOLLISION_DUST )
		{
			particleBanned = true;
		}

		if ( particle )
		{
			if ( (flags & DRAW_OPAQUE) != (psystem->Flags & DRAW_OPAQUE)
				|| (flags & DRAW_DARK) != (psystem->Flags & DRAW_DARK) )
			{
				CSprite::FlushSpriteBuffer();
				
				if ( psystem->Flags & DRAW_OPAQUE )
				{
					RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
					RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
				}
				else
				{
					if ( psystem->Flags & DRAW_DARK )
						RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
					else
						RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
	
					RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
				}

				flags = psystem->Flags;
			}
			
			if ( frames != nil )
			{
				RwRaster *curFrame = *frames;
				if ( curFrame != prevFrame )
				{
					CSprite::FlushSpriteBuffer();
					RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)curFrame);
					prevFrame = curFrame;
				}
			}
		}
		
		while ( particle != nil )
		{
			bool canDraw = true;

			if ( particle->m_nAlpha == 0 )
				canDraw = false;
			
			if ( canDraw && psystem->m_nFinalAnimationFrame != 0 && frames != nil )
			{
				RwRaster *curFrame = frames[particle->m_nCurrentFrame];
				if ( prevFrame != curFrame )
				{
					CSprite::FlushSpriteBuffer();
					RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)curFrame);
					prevFrame = curFrame;
				}
			}
			
			if ( canDraw && psystem->Flags & DRAWTOP2D )
			{
				float screenZ = (particle->m_vecPosition.z - CDraw::GetNearClipZ())
					* (CSprite::GetFarScreenZ() - CSprite::GetNearScreenZ())
					* CDraw::GetFarClipZ()
					/ ( (CDraw::GetFarClipZ() - CDraw::GetNearClipZ()) * particle->m_vecPosition.z )
					+ CSprite::GetNearScreenZ();
				
				float stretchTexW;
				float stretchTexH;
					
				if ( i == PARTICLE_RAINDROP || i == PARTICLE_RAINDROP_SMALL || i == PARTICLE_RAINDROP_2D )
				{
					stretchTexW = CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.x * (float)particle->m_nCurrentFrame + 63.0f;
					stretchTexH = CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.y * (float)particle->m_nCurrentFrame + 63.0f;
				}
				else
				{
					stretchTexW = CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.x + 63.0f;
					stretchTexH = CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.y + 63.0f;
				}

				
				if ( i == PARTICLE_WATERDROP )
				{
					int32 timeLeft = (particle->m_nTimeWhenWillBeDestroyed - CTimer::GetTimeInMilliseconds()) / particle->m_nTimeWhenWillBeDestroyed;
					
					stretchTexH += (1.0f - (float)timeLeft ) * psystem->m_vecTextureStretch.y;
					
					RwRect rect;
					
					rect.x = int32(particle->m_vecPosition.x - SCREEN_STRETCH_X(particle->m_fSize * stretchTexW));
					rect.y = int32(particle->m_vecPosition.y - SCREEN_STRETCH_Y(particle->m_fSize * stretchTexH));
					rect.w = int32(particle->m_vecPosition.x + SCREEN_STRETCH_X(particle->m_fSize * stretchTexW));
					rect.h = int32(particle->m_vecPosition.y + SCREEN_STRETCH_Y(particle->m_fSize * stretchTexH));
					
					FxType fxtype;

					if ( particle->m_nCurrentFrame != 0 )
						fxtype = FXTYPE_1;
					else
						fxtype = FXTYPE_0;
		
					CMBlur::AddRenderFx(Scene.camera, &rect, screenZ, fxtype);
					
					canDraw = false;
				}
				
				if ( i == PARTICLE_BLOODDROP )
				{
					int32 timeLeft = (particle->m_nTimeWhenWillBeDestroyed - CTimer::GetTimeInMilliseconds()) / particle->m_nTimeWhenWillBeDestroyed;
					
					stretchTexH += (1.0f + (float)timeLeft) * psystem->m_vecTextureStretch.y;
					stretchTexW += (1.0f - (float)timeLeft) * psystem->m_vecTextureStretch.x;

					RwRect rect;

					rect.x = int32(particle->m_vecPosition.x - SCREEN_STRETCH_X(particle->m_fSize * stretchTexW));
					rect.y = int32(particle->m_vecPosition.y - SCREEN_STRETCH_Y(particle->m_fSize * stretchTexH));
					rect.w = int32(particle->m_vecPosition.x + SCREEN_STRETCH_X(particle->m_fSize * stretchTexW));
					rect.h = int32(particle->m_vecPosition.y + SCREEN_STRETCH_Y(particle->m_fSize * stretchTexH));
					
					FxType fxtype;
					
					if ( particle->m_nCurrentFrame )
						fxtype = FXTYPE_3;
					else
						fxtype = FXTYPE_2;
					
					CMBlur::AddRenderFx(Scene.camera, &rect, screenZ, fxtype);
					
					canDraw = false;
				}
				
				if ( i == PARTICLE_HEATHAZE_IN_DIST )
				{
					RwRect rect;
					
					rect.x = int32(particle->m_vecPosition.x - SCREEN_STRETCH_X(particle->m_fSize * stretchTexW));
					rect.y = int32(particle->m_vecPosition.y - SCREEN_STRETCH_Y(particle->m_fSize * stretchTexH * 0.15f));
					rect.w = int32(particle->m_vecPosition.x + SCREEN_STRETCH_X(particle->m_fSize * stretchTexW));
					rect.h = int32(particle->m_vecPosition.y + SCREEN_STRETCH_Y(particle->m_fSize * stretchTexH * 0.15f));
					
					CMBlur::AddRenderFx(Scene.camera, &rect, screenZ, FXTYPE_4);
					
					canDraw = false;
				}
				
				if ( i == PARTICLE_HEATHAZE )
				{
					RwRect rect;
					
					switch ( TheCamera.GetLookDirection() )
					{
						case LOOKING_LEFT:
							rect.x = int32(particle->m_vecPosition.x - SCREEN_STRETCH_X(particle->m_fSize * psystem->m_vecTextureStretch.x * 2.0f));
							rect.y = int32(particle->m_vecPosition.y - SCREEN_STRETCH_Y(particle->m_fSize * psystem->m_vecTextureStretch.y));
							rect.w = int32(particle->m_vecPosition.x - SCREEN_STRETCH_X(particle->m_fSize * psystem->m_vecTextureStretch.x));
							rect.h = int32(particle->m_vecPosition.y + SCREEN_STRETCH_Y(particle->m_fSize * psystem->m_vecTextureStretch.y));
					
							break;

						case LOOKING_RIGHT:
							rect.x = int32(particle->m_vecPosition.x + SCREEN_STRETCH_X(particle->m_fSize * psystem->m_vecTextureStretch.x));
							rect.y = int32(particle->m_vecPosition.y - SCREEN_STRETCH_Y(particle->m_fSize * psystem->m_vecTextureStretch.y));
							rect.w = int32(particle->m_vecPosition.x + SCREEN_STRETCH_X(particle->m_fSize * psystem->m_vecTextureStretch.x * 4.0f));
							rect.h = int32(particle->m_vecPosition.y + SCREEN_STRETCH_Y(particle->m_fSize * psystem->m_vecTextureStretch.y));
					
							break;

						default:
							rect.x = int32(particle->m_vecPosition.x - SCREEN_STRETCH_X(particle->m_fSize * psystem->m_vecTextureStretch.x));
							rect.y = int32(particle->m_vecPosition.y - SCREEN_STRETCH_Y(particle->m_fSize * psystem->m_vecTextureStretch.y));
							rect.w = int32(particle->m_vecPosition.x + SCREEN_STRETCH_X(particle->m_fSize * psystem->m_vecTextureStretch.x));
							rect.h = int32(particle->m_vecPosition.y + SCREEN_STRETCH_Y(particle->m_fSize * psystem->m_vecTextureStretch.y));
					
							break;
					}
														 
					CMBlur::AddRenderFx(Scene.camera, &rect, screenZ, FXTYPE_4);
					
					canDraw = false;
				}
				
				if ( canDraw )
				{
					if ( particle->m_nRotation != 0 )
					{
						CSprite::RenderBufferedOneXLUSprite2D_Rotate_Dimension(
								particle->m_vecPosition.x,
								particle->m_vecPosition.y,
								particle->m_fSize * stretchTexW,
								particle->m_fSize * stretchTexH,
								particle->m_Color,
								particle->m_nColorIntensity,
								DEGTORAD((float)particle->m_nRotation),
								particle->m_nAlpha);
					}
					else
					{
						CSprite::RenderBufferedOneXLUSprite2D(
								particle->m_vecPosition.x,
								particle->m_vecPosition.y,
								particle->m_fSize * stretchTexW,
								particle->m_fSize * stretchTexH,
								particle->m_Color,
								particle->m_nColorIntensity,
								particle->m_nAlpha);
					}
				}
				
				canDraw = false;
			}
			
			if ( canDraw )
			{
				CVector coors;
				float w;
				float h;

				if ( CSprite::CalcScreenCoors(particle->m_vecPosition, coors, &w, &h, true) )
				{
					
					if ( i == PARTICLE_ENGINE_STEAM
						|| i == PARTICLE_ENGINE_SMOKE
						|| i == PARTICLE_ENGINE_SMOKE2
						|| i == PARTICLE_CARFLAME_SMOKE
						|| i == PARTICLE_CARCOLLISION_DUST
						|| i == PARTICLE_EXHAUST_FUMES
						|| i == PARTICLE_RUBBER_SMOKE
						|| i == PARTICLE_BURNINGRUBBER_SMOKE )
					{
						switch ( TheCamera.GetLookDirection() )
						{
							case LOOKING_LEFT:
							case LOOKING_RIGHT:
								w += CGeneral::GetRandomNumberInRange(1.0f, 7.5f) * psystem->m_vecTextureStretch.x;
								h += CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.y;
								break;

							default:
								w += CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.x;
								h += CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.y;
								break;
						}
					}
					else if ( i == PARTICLE_WATER_HYDRANT )
					{
						int32 timeLeft = (particle->m_nTimeWhenWillBeDestroyed - CTimer::GetTimeInMilliseconds()) / particle->m_nTimeWhenWillBeDestroyed;
					
						w += (1.0f - (float)timeLeft) * psystem->m_vecTextureStretch.x;
						h += (1.0f - (float)timeLeft) * psystem->m_vecTextureStretch.y;
					}
					else if ( i == PARTICLE_FLYERS )
					{
						w += psystem->m_vecTextureStretch.x;
						h += psystem->m_vecTextureStretch.y;
						
						w = Max(w, 12.0f);
						h = Max(h, 12.0f);
					}
					else
					{
						w += CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.x;
						h += CGeneral::GetRandomNumberInRange(0.1f, 1.0f) * psystem->m_vecTextureStretch.y;
					}
					
					if ( i == PARTICLE_WATER_HYDRANT
							|| (!particleBanned || SCREEN_WIDTH * fParticleScaleLimit >= w)
							&& SCREEN_HEIGHT * fParticleScaleLimit >= h )
					{
						if ( i == PARTICLE_WATER_HYDRANT )
						{
							RwRect rect;
							
							if ( w > 0.0f )
							{
								rect.x = int32(coors.x - SCREEN_STRETCH_X(particle->m_fSize * w));
								rect.w = int32(coors.x + SCREEN_STRETCH_X(particle->m_fSize * w));
							}
							else
							{
								rect.w = int32(coors.x - SCREEN_STRETCH_X(particle->m_fSize * w));
								rect.x = int32(coors.x + SCREEN_STRETCH_X(particle->m_fSize * w));
							}
							
							if ( h > 0.0f )
							{
								rect.y = int32(coors.y - SCREEN_STRETCH_Y(particle->m_fSize * h));
								rect.h = int32(coors.y + SCREEN_STRETCH_Y(particle->m_fSize * h));
							}
							else
							{
								rect.h = int32(coors.y - SCREEN_STRETCH_Y(particle->m_fSize * h));
								rect.y = int32(coors.y + SCREEN_STRETCH_Y(particle->m_fSize * h));
							}
							
							float screenZ = (coors.z - CDraw::GetNearClipZ())
								* (CSprite::GetFarScreenZ() - CSprite::GetNearScreenZ()) * CDraw::GetFarClipZ()
								/ ( (CDraw::GetFarClipZ() - CDraw::GetNearClipZ()) * coors.z ) + CSprite::GetNearScreenZ();

							CMBlur::AddRenderFx(Scene.camera, &rect, screenZ, FXTYPE_5);
						}
						else
						{
							if ( particle->m_nRotation != 0 && i != PARTICLE_BEASTIE )
							{					
								CSprite::RenderBufferedOneXLUSprite_Rotate_Dimension(coors.x, coors.y, coors.z,
										particle->m_fSize * w, particle->m_fSize * h,
										particle->m_Color.red,
										particle->m_Color.green,
										particle->m_Color.blue,
										particle->m_nColorIntensity,
										1.0f / coors.z,
										DEGTORAD((float)particle->m_nRotation),
										particle->m_nAlpha);
							}
							else if ( psystem->Flags & SCREEN_TRAIL )
							{
								float fRotation;
								float fTrailLength;
								
								if ( particle->m_fZGround == 0.0f )
								{
									fTrailLength = 0.0f;
									fRotation = 0.0f;
								}
								else
								{
									CVector2D vecDist
									(
										coors.x - particle->m_fZGround,
										coors.y - particle->m_fExpansionRate
									);
	
									float fDist = vecDist.Magnitude();
	
									fTrailLength = fDist;
									
									float fRot = Asin(vecDist.x / fDist);
	
									fRotation = fRot;
	
									if ( vecDist.y < 0.0f )
										fRotation = -1.0f * fRot + DEGTORAD(180.0f);
									
									float fSpeed = particle->m_vecVelocity.Magnitude();
									
									float fNewTrailLength = fSpeed * CTimer::GetTimeStep() * w * 2.0f;
									
									if ( fDist > fNewTrailLength )
										fTrailLength = fNewTrailLength;
								}
								
								CSprite::RenderBufferedOneXLUSprite_Rotate_Dimension(coors.x, coors.y, coors.z,
										particle->m_fSize * w,
										particle->m_fSize * h + fTrailLength * psystem->m_fTrailLengthMultiplier,
										particle->m_Color.red,
										particle->m_Color.green,
										particle->m_Color.blue,
										particle->m_nColorIntensity,
										1.0f / coors.z,
										fRotation,
										particle->m_nAlpha);
				
								particle->m_fZGround = coors.x;				// WTF ?
								particle->m_fExpansionRate =  coors.y;		// WTF ?
							}
							else if ( psystem->Flags & SPEED_TRAIL )
							{
								CVector vecPrevPos = particle->m_vecPosition - particle->m_vecVelocity;
								float fRotation;
								float fTrailLength;
								CVector vecScreenPosition;
								
								if ( CSprite::CalcScreenCoors(vecPrevPos, vecScreenPosition, &fTrailLength, &fRotation, true) )
								{
									CVector2D vecDist
									(
										coors.x - vecScreenPosition.x,
										coors.y - vecScreenPosition.y
									);
									
									float fDist = vecDist.Magnitude();
									
									fTrailLength = fDist;
									
									float fRot = Asin(vecDist.x / fDist);
									
									fRotation = fRot;
									
									if ( vecDist.y < 0.0f )
										fRotation = -1.0f * fRot + DEGTORAD(180.0f);
								}
								else
								{
									fRotation = 0.0f;
									fTrailLength = 0.0f;
								}
								
								CSprite::RenderBufferedOneXLUSprite_Rotate_Dimension(coors.x, coors.y, coors.z,
										particle->m_fSize * w,
										particle->m_fSize * h + fTrailLength * psystem->m_fTrailLengthMultiplier,
										particle->m_Color.red,
										particle->m_Color.green,
										particle->m_Color.blue,
										particle->m_nColorIntensity,
										1.0f / coors.z,
										fRotation,
										particle->m_nAlpha);
							}
							else if ( psystem->Flags & VERT_TRAIL )
							{
								float fTrailLength = fabsf(particle->m_vecVelocity.z * 10.0f);
	
								CSprite::RenderBufferedOneXLUSprite(coors.x, coors.y, coors.z,
										particle->m_fSize * w,
										(particle->m_fSize + fTrailLength * psystem->m_fTrailLengthMultiplier) * h,
										particle->m_Color.red,
										particle->m_Color.green,
										particle->m_Color.blue,
										particle->m_nColorIntensity,
										1.0f / coors.z,
										particle->m_nAlpha);
							}
							else if ( i == PARTICLE_RAINDROP_SMALL )
							{
								CSprite::RenderBufferedOneXLUSprite(coors.x, coors.y, coors.z,
										particle->m_fSize * w * 0.05f,
										particle->m_fSize * h,
										particle->m_Color.red,
										particle->m_Color.green,
										particle->m_Color.blue,
										particle->m_nColorIntensity,
										1.0f / coors.z,
										particle->m_nAlpha);
							}
							/*else if ( i == PARTICLE_BOAT_WAKE )*/
							else
							{							
								CSprite::RenderBufferedOneXLUSprite(coors.x, coors.y, coors.z,
										particle->m_fSize * w,
										particle->m_fSize * h,
										particle->m_Color.red,
										particle->m_Color.green,
										particle->m_Color.blue,
										particle->m_nColorIntensity,
										1.0f / coors.z,
										particle->m_nAlpha);
							}
						}
					}
				}
			}
			
			particle = particle->m_pNext;
		}

		CSprite::FlushSpriteBuffer();

	}
	
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
}

void CParticle::RemovePSystem(tParticleType type)
{
	tParticleSystemData *psystemdata = &mod_ParticleSystemManager.m_aParticles[type];
	
	for ( CParticle *particle = psystemdata->m_pParticles; particle; particle = psystemdata->m_pParticles )
		RemoveParticle(particle, nil, psystemdata);
}

void CParticle::RemoveParticle(CParticle *pParticle, CParticle *pPrevParticle, tParticleSystemData *pPSystemData)
{
	if ( pPSystemData->m_Type == PARTICLE_WATERDROP )
		--numWaterDropOnScreen;
	
	if ( pPrevParticle )
		pPrevParticle->m_pNext = pParticle->m_pNext;
	else
		pPSystemData->m_pParticles = pParticle->m_pNext;

	pParticle->m_pNext = m_pUnusedListHead;
	m_pUnusedListHead = pParticle;
}

void CParticle::AddJetExplosion(CVector const &vecPos, float fPower, float fSize)
{
	CRGBA color(240, 240, 240, 255);

	if ( fPower < 1.0f )
		fPower = 1.0f;
	
	CVector vecRandOffset
	(
		CGeneral::GetRandomNumberInRange(-0.4f, 0.4f),
		CGeneral::GetRandomNumberInRange(-0.4f, 0.4f),
		CGeneral::GetRandomNumberInRange(0.1f, 0.3f)
	);
	
	vecRandOffset *= 2.0f;

	CVector vecStepPos = vecPos;

	for ( int32 i = 0; i < int32(fPower * 4.0f); i++ )
	{
		AddParticle(PARTICLE_EXPLOSION_MFAST,
					vecStepPos,
					CVector
					(
						CGeneral::GetRandomNumberInRange(-0.2f, 0.2f),
						CGeneral::GetRandomNumberInRange(-0.2f, 0.2f),
						CGeneral::GetRandomNumberInRange(-0.2f, 0.0f)
					),
					nil,
					fSize, color, 0, 0, 0, 0);

		AddParticle(PARTICLE_EXPLOSION_MFAST,
					vecStepPos,
					CVector
					(
						CGeneral::GetRandomNumberInRange(-0.04f, 0.04f),
						CGeneral::GetRandomNumberInRange(-0.04f, 0.04f),
						CGeneral::GetRandomNumberInRange(0.0f, 0.07f)
					),
					nil,
					fSize, color, 0, 0, 0, 0);

		AddParticle(PARTICLE_EXPLOSION_MFAST,
					vecStepPos,
					CVector
					(
						CGeneral::GetRandomNumberInRange(-0.04f, 0.04f),
						CGeneral::GetRandomNumberInRange(-0.04f, 0.04f),
						CGeneral::GetRandomNumberInRange(0.0f, 0.07f)
					),
					nil,
					fSize, color, 0, 0, 0, 0);
		
		vecStepPos += vecRandOffset;
	}
}

void CParticle::AddYardieDoorSmoke(CVector const &vecPos, CMatrix const &matMatrix)
{
	CRGBA color(0, 0, 0, 0);
	
	CMatrix invMat(Invert(matMatrix));
	
	CVector vecBasePos = matMatrix * (invMat * vecPos + CVector(0.0f, -1.0f, 0.5f));
	
	for ( int32 i = 0; i < 5; i++ )
	{
		CVector pos = vecBasePos;

		pos.x += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f);
		pos.y += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f);
		
		AddParticle(PARTICLE_CARCOLLISION_DUST,
					pos,
					CVector(0.0f, 0.0f, 0.0f),
					nil,
					0.3f, color, 0, 0, 0, 0);
	}
}

void CParticle::CalWindDir(CVector *vecDirIn, CVector *vecDirOut)
{
	vecDirOut->x = (Cos(128) * vecDirIn->x) + (Sin(128) * vecDirIn->y);

	vecDirOut->x = (Cos(128) * vecDirIn->x) + (Sin(128) * vecDirIn->y) * CWeather::Wind;
	vecDirOut->y = (Sin(128) * vecDirIn->x) - (Cos(128) * vecDirIn->y) * CWeather::Wind;
}

void CParticle::HandleShipsAtHorizonStuff()
{
	tParticleSystemData *psystemdata = &mod_ParticleSystemManager.m_aParticles[PARTICLE_SHIP_SIDE];

	for ( CParticle *particle = psystemdata->m_pParticles; particle; particle = particle->m_pNext )
	{
		if ( CTimer::GetTimeInMilliseconds() > particle->m_nTimeWhenWillBeDestroyed - 32000 
				&& CTimer::GetTimeInMilliseconds() < particle->m_nTimeWhenWillBeDestroyed - 22000 )
		{		
			particle->m_nAlpha = Min(particle->m_nAlpha + 1, 96);
		}
		if ( CTimer::GetTimeInMilliseconds() > particle->m_nTimeWhenWillBeDestroyed - 10000 )
			particle->m_nFadeAlphaTimer = 1;
	}
}

void CParticle::HandleShootableBirdsStuff(CEntity *entity, CVector const&camPos)
{
	float fHeadingRad = entity->GetForward().Heading();
	float fHeading = RADTODEG(fHeadingRad);
	float fBirdAngle = Cos(DEGTORAD(1.5f));
	
	tParticleSystemData *psystem = &mod_ParticleSystemManager.m_aParticles[PARTICLE_BIRD_FRONT];
	CParticle *particle = psystem->m_pParticles;
	CParticle *prevParticle = nil;
	bool bRemoveParticle;
			
	for ( ; particle != nil; _Next(particle, prevParticle, psystem, bRemoveParticle) )
	{
		bRemoveParticle = false;
		
		CVector2D vecPos(particle->m_vecPosition.x, particle->m_vecPosition.y);
		CVector2D vecCamPos(camPos.x, camPos.y);

		CVector2D vecDist = vecPos - vecCamPos;
		vecDist.Normalise();
		
		float fHead = DEGTORAD(fHeading);

		CVector2D vecDir(-::Sin(fHead), ::Cos(fHead));
		vecDir.Normalise();
		
		float fDot = DotProduct2D(vecDir, vecDist);
		
		if ( fDot > 0.0f && fDot > fBirdAngle )
		{
			if ( (camPos - particle->m_vecPosition).MagnitudeSqr() < 40000.0f )
			{
				CStats::SeagullsKilled++;
				
				bRemoveParticle = true;

				for ( int32 i = 0; i < 8; i++ )
				{	
					CParticle *pBirdDerbis = AddParticle(PARTICLE_BIRD_DEBRIS,
												particle->m_vecPosition,
												CVector
												(
													CGeneral::GetRandomNumberInRange(-3.0f, 3.0f),
													CGeneral::GetRandomNumberInRange(-3.0f, 3.0f),
													CGeneral::GetRandomNumberInRange(-3.0f, 3.0f)
												),
												nil,
												0.3f,
												particle->m_Color,
												CGeneral::GetRandomNumberInRange(20, 40),
												0,
												CGeneral::GetRandomNumber() & 3,
												200);
					if ( pBirdDerbis )
						pBirdDerbis->m_nAlpha = particle->m_nAlpha;
				}
			}
		}
	}
	
}