#include "common.h"

#include "AudioManager.h"
#include "audio_enums.h"

#include "Automobile.h"
#include "Boat.h"
#include "Bridge.h"
#include "Camera.h"
#include "Cranes.h"
#include "DMAudio.h"
#include "Entity.h"
#include "Explosion.h"
#include "Fire.h"
#include "Garages.h"
#include "General.h"
#include "HandlingMgr.h"
#include "Heli.h"
#include "ModelIndices.h"
#include "MusicManager.h"
#include "Pad.h"
#include "Ped.h"
#include "Physical.h"
#include "Placeable.h"
#include "Plane.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "Projectile.h"
#include "ProjectileInfo.h"
#include "Replay.h"
#include "Stats.h"
#include "SurfaceTable.h"
#include "Train.h"
#include "Transmission.h"
#include "Vehicle.h"
#include "WaterCannon.h"
#include "Weather.h"
#include "ZoneCull.h"
#include "sampman.h"

const int channels = ARRAY_SIZE(AudioManager.m_asActiveSamples);
const int policeChannel = channels + 1;
const int allChannels = channels + 2;

uint32 gPornNextTime;
uint32 gSawMillNextTime;
uint32 gShopNextTime;
uint32 gAirportNextTime;
uint32 gCinemaNextTime;
uint32 gDocksNextTime;
uint32 gHomeNextTime;
uint32 gCellNextTime;
uint32 gNextCryTime;

enum PLAY_STATUS { PLAY_STATUS_STOPPED = 0, PLAY_STATUS_PLAYING, PLAY_STATUS_FINISHED };
enum LOADING_STATUS { LOADING_STATUS_NOT_LOADED = 0, LOADING_STATUS_LOADED, LOADING_STATUS_FAILED };

void
cAudioManager::PreInitialiseGameSpecificSetup() const
{
	BankStartOffset[SFX_BANK_0] = SAMPLEBANK_START;
#ifdef GTA_PS2
	BankStartOffset[SFX_BANK_PACARD] = SFX_CAR_ACCEL_1;
	BankStartOffset[SFX_BANK_PATHFINDER] = SFX_CAR_ACCEL_2;
	BankStartOffset[SFX_BANK_PORSCHE] = SFX_CAR_ACCEL_3;
	BankStartOffset[SFX_BANK_SPIDER] = SFX_CAR_ACCEL_4;
	BankStartOffset[SFX_BANK_MERC] = SFX_CAR_ACCEL_5;
	BankStartOffset[SFX_BANK_TRUCK] = SFX_CAR_ACCEL_6;
	BankStartOffset[SFX_BANK_HOTROD] = SFX_CAR_ACCEL_7;
	BankStartOffset[SFX_BANK_COBRA] = SFX_CAR_ACCEL_8;
	BankStartOffset[SFX_BANK_NONE] = SFX_CAR_ACCEL_9;
	BankStartOffset[SFX_BANK_FRONT_END_MENU] = SFX_PAGE_CHANGE_AND_BACK_LEFT;
	BankStartOffset[SFX_BANK_TRAIN] = SFX_TRAIN_STATION_AMBIENCE_LOOP;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_1] = SFX_CLUB_1;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_2] = SFX_CLUB_2;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_3] = SFX_CLUB_3;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_4] = SFX_CLUB_4;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_5] = SFX_CLUB_5;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_6] = SFX_CLUB_6;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_7] = SFX_CLUB_7;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_8] = SFX_CLUB_8;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_9] = SFX_CLUB_9;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_10] = SFX_CLUB_10;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_11] = SFX_CLUB_11;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_12] = SFX_CLUB_12;
	BankStartOffset[SFX_BANK_BUILDING_CLUB_RAGGA] = SFX_CLUB_RAGGA;
	BankStartOffset[SFX_BANK_BUILDING_STRIP_CLUB_1] = SFX_STRIP_CLUB_1;
	BankStartOffset[SFX_BANK_BUILDING_STRIP_CLUB_2] = SFX_STRIP_CLUB_2;
	BankStartOffset[SFX_BANK_BUILDING_WORKSHOP] = SFX_WORKSHOP_1;
	BankStartOffset[SFX_BANK_BUILDING_PIANO_BAR] = SFX_PIANO_BAR_1;
	BankStartOffset[SFX_BANK_BUILDING_SAWMILL] = SFX_SAWMILL_LOOP;
	BankStartOffset[SFX_BANK_BUILDING_DOG_FOOD_FACTORY] = SFX_DOG_FOOD_FACTORY;
	BankStartOffset[SFX_BANK_BUILDING_LAUNDERETTE] = SFX_LAUNDERETTE_LOOP;
	BankStartOffset[SFX_BANK_BUILDING_RESTAURANT_CHINATOWN] = SFX_RESTAURANT_CHINATOWN;
	BankStartOffset[SFX_BANK_BUILDING_RESTAURANT_ITALY] = SFX_RESTAURANT_ITALY;
	BankStartOffset[SFX_BANK_BUILDING_RESTAURANT_GENERIC_1] = SFX_RESTAURANT_GENERIC_1;
	BankStartOffset[SFX_BANK_BUILDING_RESTAURANT_GENERIC_2] = SFX_RESTAURANT_GENERIC_2;
	BankStartOffset[SFX_BANK_BUILDING_AIRPORT] = SFX_AIRPORT_ANNOUNCEMENT_1;
	BankStartOffset[SFX_BANK_BUILDING_SHOP] = SFX_SHOP_LOOP;
	BankStartOffset[SFX_BANK_BUILDING_CINEMA] = SFX_CINEMA_BASS_1;
	BankStartOffset[SFX_BANK_BUILDING_DOCKS] = SFX_DOCKS_FOGHORN;
	BankStartOffset[SFX_BANK_BUILDING_HOME] = SFX_HOME_1;
	BankStartOffset[SFX_BANK_BUILDING_PORN_1] = SFX_PORN_1_LOOP;
	BankStartOffset[SFX_BANK_BUILDING_PORN_2] = SFX_PORN_2_LOOP;
	BankStartOffset[SFX_BANK_BUILDING_PORN_3] = SFX_PORN_3_LOOP;
	BankStartOffset[SFX_BANK_BUILDING_POLICE_BALL] = SFX_POLICE_BALL_1;
	BankStartOffset[SFX_BANK_BUILDING_BANK_ALARM] = SFX_BANK_ALARM_1;
	BankStartOffset[SFX_BANK_BUILDING_RAVE_INDUSTRIAL] = SFX_RAVE_INDUSTRIAL;
	BankStartOffset[SFX_BANK_BUILDING_RAVE_COMMERCIAL] = SFX_RAVE_COMMERCIAL;
	BankStartOffset[SFX_BANK_BUILDING_RAVE_SUBURBAN] = SFX_RAVE_SUBURBAN;
	BankStartOffset[SFX_BANK_BUILDING_RAVE_COMMERCIAL_2] = SFX_RAVE_COMMERCIAL_2;
	BankStartOffset[SFX_BANK_BUILDING_39] = SFX_CLUB_1_1;
	BankStartOffset[SFX_BANK_BUILDING_40] = SFX_CLUB_1_2;
	BankStartOffset[SFX_BANK_BUILDING_41] = SFX_CLUB_1_3;
	BankStartOffset[SFX_BANK_BUILDING_42] = SFX_CLUB_1_4;
	BankStartOffset[SFX_BANK_BUILDING_43] = SFX_CLUB_1_5;
	BankStartOffset[SFX_BANK_BUILDING_44] = SFX_CLUB_1_6;
	BankStartOffset[SFX_BANK_BUILDING_45] = SFX_CLUB_1_7;
	BankStartOffset[SFX_BANK_BUILDING_46] = SFX_CLUB_1_8;
	BankStartOffset[SFX_BANK_BUILDING_47] = SFX_CLUB_1_9;
	BankStartOffset[SFX_BANK_GENERIC_EXTRA] = SFX_EXPLOSION_1;
#endif // GTA_PS2
	BankStartOffset[SFX_BANK_PED_COMMENTS] = SAMPLEBANK_PED_START;
}

void
cAudioManager::PostInitialiseGameSpecificSetup()
{
	m_nFireAudioEntity = CreateEntity(AUDIOTYPE_FIRE, &gFireManager);
	if (m_nFireAudioEntity >= 0)
		SetEntityStatus(m_nFireAudioEntity, true);

	m_nCollisionEntity = CreateEntity(AUDIOTYPE_COLLISION, (void *)1);
	if (m_nCollisionEntity >= 0)
		SetEntityStatus(m_nCollisionEntity, true);

	m_nFrontEndEntity = CreateEntity(AUDIOTYPE_FRONTEND, (void *)1);
	if (m_nFrontEndEntity >= 0)
		SetEntityStatus(m_nFrontEndEntity, true);

	m_nProjectileEntity = CreateEntity(AUDIOTYPE_PROJECTILE, (void *)1);
	if (m_nProjectileEntity >= 0)
		SetEntityStatus(m_nProjectileEntity, true);

	m_nWaterCannonEntity = CreateEntity(AUDIOTYPE_WATERCANNON, (void *)1);
	if (m_nWaterCannonEntity >= 0)
		SetEntityStatus(m_nWaterCannonEntity, true);

	m_nPoliceChannelEntity = CreateEntity(AUDIOTYPE_POLICERADIO, (void *)1);
	if (m_nPoliceChannelEntity >= 0)
		SetEntityStatus(m_nPoliceChannelEntity, true);

	m_nBridgeEntity = CreateEntity(AUDIOTYPE_BRIDGE, (void *)1);
	if (m_nBridgeEntity >= 0)
		SetEntityStatus(m_nBridgeEntity, true);

	m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
	m_sMissionAudio.m_nLoadingStatus = LOADING_STATUS_NOT_LOADED;
	m_sMissionAudio.m_nPlayStatus = PLAY_STATUS_STOPPED;
	m_sMissionAudio.m_bIsPlaying = false;
	m_sMissionAudio.m_bIsPlayed = false;
	m_sMissionAudio.m_bPredefinedProperties = true;
	m_sMissionAudio.m_nMissionAudioCounter = 0;
	ResetAudioLogicTimers(CTimer::GetTimeInMilliseconds());
}
void
cAudioManager::PreTerminateGameSpecificShutdown()
{
	if (m_nBridgeEntity >= 0) {
		DestroyEntity(m_nBridgeEntity);
		m_nBridgeEntity = AEHANDLE_NONE;
	}
	if (m_nPoliceChannelEntity >= 0) {
		DestroyEntity(m_nPoliceChannelEntity);
		m_nPoliceChannelEntity = AEHANDLE_NONE;
	}
	if (m_nWaterCannonEntity >= 0) {
		DestroyEntity(m_nWaterCannonEntity);
		m_nWaterCannonEntity = AEHANDLE_NONE;
	}
	if (m_nFireAudioEntity >= 0) {
		DestroyEntity(m_nFireAudioEntity);
		m_nFireAudioEntity = AEHANDLE_NONE;
	}
	if (m_nCollisionEntity >= 0) {
		DestroyEntity(m_nCollisionEntity);
		m_nCollisionEntity = AEHANDLE_NONE;
	}
	if (m_nFrontEndEntity >= 0) {
		DestroyEntity(m_nFrontEndEntity);
		m_nFrontEndEntity = AEHANDLE_NONE;
	}
	if (m_nProjectileEntity >= 0) {
		DestroyEntity(m_nProjectileEntity);
		m_nProjectileEntity = AEHANDLE_NONE;
	}
}

void
cAudioManager::PostTerminateGameSpecificShutdown()
{
	;
}

void
cAudioManager::ResetAudioLogicTimers(uint32 timer)
{
	gPornNextTime = timer;
	gNextCryTime = timer;
	gSawMillNextTime = timer;
	gCellNextTime = timer;
	gShopNextTime = timer;
	gHomeNextTime = timer;
	gAirportNextTime = timer;
	gDocksNextTime = timer;
	gCinemaNextTime = timer;
	for (int32 i = 0; i < m_nAudioEntitiesTotal; i++) {
		if (m_asAudioEntities[m_anAudioEntityIndices[i]].m_nType == AUDIOTYPE_PHYSICAL) {
			CPed *ped = (CPed *)m_asAudioEntities[m_anAudioEntityIndices[i]].m_pEntity;
			if (ped->IsPed()) {
				ped->m_lastSoundStart = timer;
				ped->m_soundStart = timer + m_anRandomTable[0] % 3000;
			}
		}
	}
	ClearMissionAudio();
	SampleManager.StopChannel(policeChannel);
}

void
cAudioManager::ProcessReverb() const
{
	if (SampleManager.UpdateReverb() && m_bDynamicAcousticModelingStatus) {
		for (uint32 i = 0; i <
#ifdef FIX_BUGS
		                   channels
#else
		                   28
#endif
		     ;
		     i++) {
			if (m_asActiveSamples[i].m_bReverbFlag)
				SampleManager.SetChannelReverbFlag(i, true);
		}
	}
}

float
cAudioManager::GetDistanceSquared(const CVector &v) const
{
	const CVector &c = TheCamera.GetPosition();
	return sq(v.x - c.x) + sq(v.y - c.y) + sq((v.z - c.z) * 0.2f);
}

void
cAudioManager::CalculateDistance(bool &distCalculated, float dist)
{
	if (!distCalculated) {
		m_sQueueSample.m_fDistance = Sqrt(dist);
		distCalculated = true;
	}
}

void
cAudioManager::ProcessSpecial()
{
	if (m_nUserPause) {
		if (!m_nPreviousUserPause) {
			MusicManager.ChangeMusicMode(MUSICMODE_FRONTEND);
			SampleManager.SetEffectsFadeVolume(MAX_VOLUME);
			SampleManager.SetMusicFadeVolume(MAX_VOLUME);
		}
	} else {
		if (m_nPreviousUserPause) {
			MusicManager.StopFrontEndTrack();
			MusicManager.ChangeMusicMode(MUSICMODE_GAME);
		}
		CPlayerPed *playerPed = FindPlayerPed();
		if (playerPed) {
			if(playerPed->EnteringCar() && !playerPed->bInVehicle)
				SampleManager.StopChannel(m_nActiveSamples);
		}
	}
}

void
cAudioManager::ProcessEntity(int32 id)
{
	if (m_asAudioEntities[id].m_bStatus) {
		m_sQueueSample.m_nEntityIndex = id;
		switch (m_asAudioEntities[id].m_nType) {
		case AUDIOTYPE_PHYSICAL:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessPhysical(id);
			}
			break;
		case AUDIOTYPE_EXPLOSION:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessExplosions(id);
			}
			break;
		case AUDIOTYPE_FIRE:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessFires(id);
			}
			break;
		case AUDIOTYPE_WEATHER:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessWeather(id);
			}
			break;
		case AUDIOTYPE_CRANE:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessCrane();
			}
			break;
		case AUDIOTYPE_SCRIPTOBJECT:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessScriptObject(id);
			}
			break;
		case AUDIOTYPE_BRIDGE:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessBridge();
			}
			break;
		case AUDIOTYPE_FRONTEND:
			m_sQueueSample.m_bReverbFlag = false;
			ProcessFrontEnd();
			break;
		case AUDIOTYPE_PROJECTILE:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessProjectiles();
			}
			break;
		case AUDIOTYPE_GARAGE:
			if (!m_nUserPause)
				ProcessGarages();
			break;
		case AUDIOTYPE_FIREHYDRANT:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessFireHydrant();
			}
			break;
		case AUDIOTYPE_WATERCANNON:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessWaterCannon(id);
			}
			break;
		default:
			return;
		}
	}
}

void
cAudioManager::ProcessPhysical(int32 id)
{
	CPhysical *entity = (CPhysical *)m_asAudioEntities[id].m_pEntity;
	if (entity) {
		switch (entity->GetType()) {
		case ENTITY_TYPE_VEHICLE:
			ProcessVehicle((CVehicle *)m_asAudioEntities[id].m_pEntity);
			break;
		case ENTITY_TYPE_PED:
			ProcessPed((CPhysical *)m_asAudioEntities[id].m_pEntity);
			break;
		default:
			return;
		}
	}
}

#pragma region VEHICLE AUDIO
enum eVehicleModel {
	LANDSTAL,
	IDAHO,
	STINGER,
	LINERUN,
	PEREN,
	SENTINEL,
	PATRIOT,
	FIRETRUK,
	TRASH,
	STRETCH,
	MANANA,
	INFERNUS,
	BLISTA,
	PONY,
	MULE,
	CHEETAH,
	AMBULAN,
	FBICAR,
	MOONBEAM,
	ESPERANT,
	TAXI,
	KURUMA,
	BOBCAT,
	MRWHOOP,
	BFINJECT,
	CORPSE,
	POLICE,
	ENFORCER,
	SECURICA,
	BANSHEE,
	PREDATOR,
	BUS,
	RHINO,
	BARRACKS,
	TRAIN,
	CHOPPER,
	DODO,
	COACH,
	CABBIE,
	STALLION,
	RUMPO,
	RCBANDIT,
	BELLYUP,
	MRWONGS,
	MAFIA,
	YARDIE,
	YAKUZA,
	DIABLOS,
	COLUMB,
	HOODS,
	AIRTRAIN,
	DEADDODO,
	SPEEDER,
	REEFER,
	PANLANT,
	FLATBED,
	YANKEE,
	ESCAPE,
	BORGNINE,
	TOYZ,
	GHOST,
	CAR151,
	CAR152,
	CAR153,
	CAR154,
	CAR155,
	CAR156,
	CAR157,
	CAR158,
	CAR159,
	MAX_CARS
};

enum
{
	OLD_DOOR = 0,
	NEW_DOOR,
	TRUCK_DOOR,
	BUS_DOOR,
};


struct tVehicleSampleData {
	eSfxSample m_nAccelerationSampleIndex;
	uint8 m_nBank;
	eSfxSample m_nHornSample;
	int32 m_nHornFrequency;
	uint8 m_nSirenOrAlarmSample;
	int32 m_nSirenOrAlarmFrequency;
	uint8 m_bDoorType;
};

const tVehicleSampleData aVehicleSettings[MAX_CARS] = {
    {SFX_CAR_REV_2, SFX_BANK_PATHFINDER, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_ALARM_1, 9935, NEW_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 11487, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_8, SFX_BANK_COBRA, SFX_CAR_HORN_PORSCHE, 11025, SFX_CAR_ALARM_1, 10928, NEW_DOOR},
    {SFX_CAR_REV_6, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_ALARM_1, 9935, TRUCK_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 12893, SFX_CAR_ALARM_1, 8941, OLD_DOOR},
    {SFX_CAR_REV_5, SFX_BANK_MERC, SFX_CAR_HORN_BMW328, 10706, SFX_CAR_ALARM_1, 11922, NEW_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_ALARM_1, 7948, TRUCK_DOOR},
    {SFX_CAR_REV_6, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 29711, SFX_POLICE_SIREN_SLOW, 11556, TRUCK_DOOR},
    {SFX_CAR_REV_6, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 31478, SFX_CAR_ALARM_1, 8941, TRUCK_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_BMW328, 9538, SFX_CAR_ALARM_1, 12220, NEW_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_3, SFX_BANK_PORSCHE, SFX_CAR_HORN_BMW328, 12017, SFX_CAR_ALARM_1, 9935, NEW_DOOR},
    {SFX_CAR_REV_2, SFX_BANK_PATHFINDER, SFX_CAR_HORN_JEEP, 22295, SFX_CAR_ALARM_1, 12200, NEW_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS2, 18000, SFX_CAR_ALARM_1, 13400, NEW_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS, 18286, SFX_CAR_ALARM_1, 9935, TRUCK_DOOR},
    {SFX_CAR_REV_3, SFX_BANK_PORSCHE, SFX_CAR_HORN_PORSCHE, 11025, SFX_CAR_ALARM_1, 13600, NEW_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_JEEP, 22295, SFX_AMBULANCE_SIREN_SLOW, 8795, TRUCK_DOOR},
    {SFX_CAR_REV_5, SFX_BANK_MERC, SFX_CAR_HORN_PORSCHE, 9271, SFX_POLICE_SIREN_SLOW, 16168, NEW_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 12170, SFX_CAR_ALARM_1, 8000, NEW_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_BUS2, 12345, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_2, SFX_BANK_PATHFINDER, SFX_CAR_HORN_BMW328, 10796, SFX_CAR_ALARM_1, 8543, NEW_DOOR},
    {SFX_CAR_REV_5, SFX_BANK_MERC, SFX_CAR_HORN_PORSCHE, 9271, SFX_CAR_ALARM_1, 9935, NEW_DOOR},
    {SFX_CAR_REV_2, SFX_BANK_PATHFINDER, SFX_CAR_HORN_PICKUP, 10924, SFX_CAR_ALARM_1, 9935, NEW_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_PICKUP, 11025, SFX_ICE_CREAM_TUNE, 11025, OLD_DOOR},
    {SFX_CAR_REV_7, SFX_BANK_HOTROD, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_ALARM_1, 9935, NEW_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_ALARM_1, 10000, OLD_DOOR},
    {SFX_CAR_REV_5, SFX_BANK_MERC, SFX_CAR_HORN_BMW328, 10706, SFX_POLICE_SIREN_SLOW, 13596, NEW_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS, 17260, SFX_POLICE_SIREN_SLOW, 13000, TRUCK_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_PICKUP, 8670, SFX_CAR_ALARM_1, 9935, TRUCK_DOOR},
    {SFX_CAR_REV_8, SFX_BANK_COBRA, SFX_CAR_HORN_PORSCHE, 10400, SFX_CAR_ALARM_1, 10123, NEW_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 26513, SFX_POLICE_SIREN_SLOW, 13596, OLD_DOOR},
    {SFX_CAR_REV_6, SFX_BANK_TRUCK, SFX_CAR_HORN_BUS2, 11652, SFX_CAR_ALARM_1, 10554, BUS_DOOR},
    {SFX_CAR_REV_6, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_ALARM_1, 8000, TRUCK_DOOR},
    {SFX_CAR_REV_6, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 28043, SFX_CAR_ALARM_1, 9935, TRUCK_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_ALARM_1, 9935, BUS_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CESNA_IDLE, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_6, SFX_BANK_TRUCK, SFX_CAR_HORN_BUS, 16291, SFX_CAR_ALARM_1, 7500, BUS_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 10233, SFX_CAR_ALARM_1, 8935, OLD_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_PICKUP, 8670, SFX_CAR_ALARM_1, 8935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_PICKUP, 2000, SFX_CAR_ALARM_1, 17000, OLD_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_5, SFX_BANK_MERC, SFX_CAR_HORN_BMW328, 9003, SFX_CAR_ALARM_1, 9935, NEW_DOOR},
    {SFX_CAR_REV_2, SFX_BANK_PATHFINDER, SFX_CAR_HORN_PORSCHE, 12375, SFX_CAR_ALARM_1, 9935, NEW_DOOR},
    {SFX_CAR_REV_5, SFX_BANK_MERC, SFX_CAR_HORN_BUS2, 15554, SFX_CAR_ALARM_1, 9935, NEW_DOOR},
    {SFX_CAR_REV_7, SFX_BANK_HOTROD, SFX_CAR_HORN_BUS2, 13857, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_7, SFX_BANK_HOTROD, SFX_CAR_HORN_PICKUP, 10924, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, TRUCK_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 20143, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9000, OLD_DOOR},
    {SFX_CAR_REV_6, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 28043, SFX_CAR_ALARM_1, 9935, TRUCK_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS, 18286, SFX_CAR_ALARM_1, 9935, TRUCK_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_4, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS2, 18000, SFX_CAR_ALARM_1, 13400, NEW_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR},
    {SFX_CAR_REV_1, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_ALARM_1, 9935, OLD_DOOR}};


bool bPlayerJustEnteredCar;

const bool hornPatternsArray[8][44] = {
    {false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false,
     false, true,  true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,  false, false, false, false},
    {false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,  true,
     true,  true,  true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false},
    {false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, true, true, true, true, true, false,
     false, false, true, true, true, true, true, true, true, true, true, true, true,  true,  true,  true,  true, true, true, true, true, false},
    {false, false, true, true, true, true,  true,  false, false, true, true, true, true, true, false, false, false, true, true, true, true, true,
     true,  true,  true, true, true, false, false, false, true,  true, true, true, true, true, true,  true,  true,  true, true, true, true, false},
    {false, false, true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false, false, false, false, false, false,
     false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, true,  true,  true,  false, false, false, true,  true,  true,  false, false, false, false, false, false, false, false, false, false, false,
     false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, true, true, true,  true,  false, false, false, false, true, true, true, false, false, true, true, true, false, false, true,  true,
     true,  true,  true, true, false, false, false, false, false, true,  true, true, true, true,  true,  true, true, true, true,  true,  false, false},
    {false, false, true,  true,  true, true, false, false, true, true, true, true, true, false, false, false, true, true,  true,  true,  true,  true,
     false, false, false, false, true, true, true,  true,  true, true, true, true, true, true,  true,  true,  true, false, false, false, false, false},
};


void
cAudioManager::ProcessVehicle(CVehicle *veh)
{
	tHandlingData *handling = veh->pHandling;
	float velChange;
	cVehicleParams params;
	m_sQueueSample.m_vecPos = veh->GetPosition();

	params.m_bDistanceCalculated = false;
	params.m_pVehicle = veh;
	params.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);

	if (handling != nil)
		params.m_pTransmission = &handling->Transmission;
	else
		params.m_pTransmission = nil;

	params.m_nIndex = veh->GetModelIndex() - MI_FIRST_VEHICLE;
	if (params.m_pVehicle->GetStatus() == STATUS_SIMPLE)
		velChange = params.m_pVehicle->AutoPilot.m_fMaxTrafficSpeed * 0.02f;
	else
		velChange = DotProduct(params.m_pVehicle->m_vecMoveSpeed, params.m_pVehicle->GetForward());
	params.m_fVelocityChange = velChange;
	switch (params.m_pVehicle->m_vehType) {
	case VEHICLE_TYPE_CAR:
		UpdateGasPedalAudio((CAutomobile *)veh);
		if (params.m_nIndex == RCBANDIT) {
			ProcessModelCarEngine(params);
			ProcessVehicleOneShots(params);
			((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
			break;
		}
		if (params.m_nIndex == DODO) {
			if (!ProcessVehicleRoadNoise(params)) {
				ProcessVehicleOneShots(params);
				((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
				break;
			}
			if (CWeather::WetRoads > 0.f)
				ProcessWetRoadNoise(params);
			ProcessVehicleSkidding(params);
		} else {
			if (!ProcessVehicleRoadNoise(params)) {
				ProcessVehicleOneShots(params);
				((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
				break;
			}
			ProcessReverseGear(params);
			if (CWeather::WetRoads > 0.f)
				ProcessWetRoadNoise(params);
			ProcessVehicleSkidding(params);
			ProcessVehicleHorn(params);
			ProcessVehicleSirenOrAlarm(params);
			if (UsesReverseWarning(params.m_nIndex))
				ProcessVehicleReverseWarning(params);
			if (HasAirBrakes(params.m_nIndex))
				ProcessAirBrakes(params);
		}
		ProcessCarBombTick(params);
		ProcessVehicleEngine(params);
		ProcessEngineDamage(params);
		ProcessVehicleDoors(params);

		ProcessVehicleOneShots(params);
		((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
		break;
	case VEHICLE_TYPE_BOAT:
		ProcessBoatEngine(params);
		ProcessBoatMovingOverWater(params);
		ProcessVehicleOneShots(params);
		break;
	case VEHICLE_TYPE_TRAIN:
		ProcessTrainNoise(params);
		ProcessVehicleOneShots(params);
		break;
	case VEHICLE_TYPE_HELI:
		ProcessHelicopter(params);
		ProcessVehicleOneShots(params);
		break;
	case VEHICLE_TYPE_PLANE:
		ProcessPlane(params);
		ProcessVehicleOneShots(params);
		break;
	default:
		break;
	}
	ProcessRainOnVehicle(params);
}

void
cAudioManager::ProcessRainOnVehicle(cVehicleParams& params)
{
	const int rainOnVehicleIntensity = 22;
	if (params.m_fDistance < SQR(rainOnVehicleIntensity) && CWeather::Rain > 0.01f && (!CCullZones::CamNoRain() || !CCullZones::PlayerNoRain())) {
		CVehicle *veh = params.m_pVehicle;
		++veh->m_bRainAudioCounter;
		if (veh->m_bRainAudioCounter >= 2) {
			veh->m_bRainAudioCounter = 0;
			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			float emittingVol = 30.f * CWeather::Rain;
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, rainOnVehicleIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = veh->m_bRainSamplesCounter++;
				if (veh->m_bRainSamplesCounter > 4)
					veh->m_bRainSamplesCounter = 68;
				m_sQueueSample.m_nSampleIndex = (m_anRandomTable[1] & 3) + SFX_CAR_RAIN_1;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nReleasingVolumeModificator = 9;
				m_sQueueSample.m_nFrequency = m_anRandomTable[1] % 4000 + 28000;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nEmittingVolume = (uint8)emittingVol;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_fSoundIntensity = rainOnVehicleIntensity;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_bReverbFlag = false;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
		}
	}
}

bool
cAudioManager::ProcessReverseGear(cVehicleParams& params)
{
	const int reverseGearIntensity = 30;

	CVehicle *veh;
	CAutomobile *automobile;
	int32 emittingVol;
	float modificator;

	if (params.m_fDistance >= SQR(reverseGearIntensity))
		return false;
	veh = params.m_pVehicle;
	if (veh->bEngineOn && (veh->m_fGasPedal < 0.0f || veh->m_nCurrentGear == 0)) {
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		automobile = (CAutomobile *)params.m_pVehicle;
		if (automobile->m_nWheelsOnGround != 0) {
			modificator = params.m_fVelocityChange / params.m_pTransmission->fMaxReverseVelocity;
		} else {
			if (automobile->m_nDriveWheelsOnGround != 0)
				automobile->m_fGasPedalAudio *= 0.4f;
			modificator = automobile->m_fGasPedalAudio;
		}
		modificator = Abs(modificator);
		emittingVol = (24.f * modificator);
		m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, reverseGearIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			if (params.m_pVehicle->m_fGasPedal >= 0.0f) {
				m_sQueueSample.m_nCounter = 62;
				m_sQueueSample.m_nSampleIndex = SFX_REVERSE_GEAR_2;
			} else {
				m_sQueueSample.m_nCounter = 61;
				m_sQueueSample.m_nSampleIndex = SFX_REVERSE_GEAR;
			}
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_nFrequency = (6000.f * modificator) + 7000;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			m_sQueueSample.m_fSoundIntensity = reverseGearIntensity;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 5;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
	return true;
}

void
cAudioManager::ProcessModelCarEngine(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 30.0f;
	CAutomobile *automobile;
	float allowedVelocity;
	int32 emittingVol;
	float velocityChange;

	if (params.m_fDistance < SQR(SOUND_INTENSITY)) {
		automobile = (CAutomobile *)params.m_pVehicle;
		if (automobile->bEngineOn) {
			if (automobile->m_nWheelsOnGround == 0) {
				if (automobile->m_nDriveWheelsOnGround != 0)
					automobile->m_fGasPedalAudio *= 0.4f;
				velocityChange = automobile->m_fGasPedalAudio * params.m_pTransmission->fMaxVelocity;
			} else {
				velocityChange = Abs(params.m_fVelocityChange);
			}
			if (velocityChange > 0.001f) {
				allowedVelocity = 0.5f * params.m_pTransmission->fMaxVelocity;
				if (velocityChange < allowedVelocity)
					emittingVol = (90.f * velocityChange / allowedVelocity);
				else
					emittingVol = 90;
				if (emittingVol) {
					CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
					m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, 30.f, m_sQueueSample.m_fDistance);
					if (m_sQueueSample.m_nVolume != 0) {
						m_sQueueSample.m_nCounter = 2;
						m_sQueueSample.m_nSampleIndex = SFX_REMOTE_CONTROLLED_CAR;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_bIs2D = false;
						m_sQueueSample.m_nReleasingVolumeModificator = 1;
						m_sQueueSample.m_nFrequency = (11025.f * velocityChange / params.m_pTransmission->fMaxVelocity + 11025.f);
						m_sQueueSample.m_nLoopCount = 0;
						m_sQueueSample.m_nEmittingVolume = emittingVol;
						m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.m_fSpeedMultiplier = 3.0f;
						m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
						m_sQueueSample.m_bReleasingSoundFlag = false;
						m_sQueueSample.m_nReleasingVolumeDivider = 3;
						m_sQueueSample.m_bReverbFlag = true;
						m_sQueueSample.m_bRequireReflection = false;
						AddSampleToRequestedQueue();
					}
				}
			}
		}
	}
}



bool
cAudioManager::ProcessVehicleRoadNoise(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 95.0f;

	int32 emittingVol;
	uint32 freq;
	float multiplier;
	int sampleFreq;
	float velocity;

	if (params.m_fDistance >= SQR(SOUND_INTENSITY))
		return false;
	if (params.m_pTransmission != nil) {
		if (((CAutomobile*)params.m_pVehicle)->m_nDriveWheelsOnGround != 0) {
			velocity = Abs(params.m_fVelocityChange);
			if (velocity > 0.0f) {
				CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
				emittingVol = 30.f * Min(1.f, velocity / (0.5f * params.m_pTransmission->fMaxVelocity));
				m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = 0;
					m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nReleasingVolumeModificator = 3;
					if (params.m_pVehicle->m_nSurfaceTouched == SURFACE_WATER) {
						m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
						freq = 6050 * emittingVol / 30 + 16000;
					} else {
						m_sQueueSample.m_nSampleIndex = SFX_ROAD_NOISE;
						multiplier = (m_sQueueSample.m_fDistance / SOUND_INTENSITY) * 0.5f;
						sampleFreq = SampleManager.GetSampleBaseFrequency(SFX_ROAD_NOISE);
						freq = (sampleFreq * multiplier) + ((3 * sampleFreq) / 4);
					}
					m_sQueueSample.m_nFrequency = freq;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_nEmittingVolume = emittingVol;
					m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_fSpeedMultiplier = 6.0f;
					m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
					m_sQueueSample.m_bReleasingSoundFlag = false;
					m_sQueueSample.m_nReleasingVolumeDivider = 4;
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_bRequireReflection = false;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
	return true;
}

bool
cAudioManager::ProcessWetRoadNoise(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 30.0f;

	float relativeVelocity;
	int32 emittingVol;
	float multiplier;
	int freq;
	float velChange;

	if (params.m_fDistance >= SQR(SOUND_INTENSITY))
		return false;
	if (params.m_pTransmission != nil) {
		if (((CAutomobile *)params.m_pVehicle)->m_nDriveWheelsOnGround != 0) {
			velChange = Abs(params.m_fVelocityChange);
			if (velChange > 0.f) {
				CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
				relativeVelocity = Min(1.0f, velChange / (0.5f * params.m_pTransmission->fMaxVelocity));
				emittingVol = 23.0f * relativeVelocity * CWeather::WetRoads;
				m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = 1;
					m_sQueueSample.m_nSampleIndex = SFX_ROAD_NOISE;
					m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nReleasingVolumeModificator = 3;
#ifdef FIX_BUGS
					multiplier = (m_sQueueSample.m_fDistance / SOUND_INTENSITY) * 0.5f;
#else
					multiplier = (m_sQueueSample.m_fDistance / 3.0f) * 0.5f;
#endif
					freq = SampleManager.GetSampleBaseFrequency(SFX_ROAD_NOISE);
					m_sQueueSample.m_nFrequency = freq + freq * multiplier;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_nEmittingVolume = emittingVol;
					m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_fSpeedMultiplier = 6.0f;
					m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
					m_sQueueSample.m_bReleasingSoundFlag = false;
					m_sQueueSample.m_nReleasingVolumeDivider = 4;
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_bRequireReflection = false;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
	return true;
}

void
cAudioManager::ProcessVehicleEngine(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 50.0f;

	CVehicle *playerVeh;
	CVehicle *veh;
	CAutomobile *automobile;
	float relativeGearChange;
	float relativeChange;
	uint8 volume;
	int32 freq = 0; // uninitialized variable
	uint8 emittingVol;
	cTransmission *transmission;
	uint8 currentGear;
	float modificator;
	float traction = 0.f;

	if (params.m_fDistance < SQR(SOUND_INTENSITY)) {
		playerVeh = FindPlayerVehicle();
		veh = params.m_pVehicle;
		if (playerVeh == veh && veh->GetStatus() == STATUS_WRECKED) {
			SampleManager.StopChannel(m_nActiveSamples);
			return;
		}
		if (veh->bEngineOn) {
			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			automobile = (CAutomobile *)params.m_pVehicle;
			if (params.m_nIndex == DODO) {
				ProcessCesna(params);
				return;
			}
			if (FindPlayerVehicle() == veh) {
				ProcessPlayersVehicleEngine(params, automobile);
				return;
			}
			transmission = params.m_pTransmission;
			if (transmission != nil) {
				currentGear = params.m_pVehicle->m_nCurrentGear;
				if (automobile->m_nWheelsOnGround != 0) {
					if (automobile->bIsHandbrakeOn) {
						if (params.m_fVelocityChange == 0.0f)
							traction = 0.9f;
					} else if (params.m_pVehicle->GetStatus() == STATUS_SIMPLE) {
						traction = 0.0f;
					} else {
						switch (transmission->nDriveType) {
						case '4':
							for (int32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelState); i++) {
								if (automobile->m_aWheelState[i] == WHEEL_STATE_SPINNING)
									traction += 0.05f;
							}
							break;
						case 'F':
							if (automobile->m_aWheelState[CARWHEEL_FRONT_LEFT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							if (automobile->m_aWheelState[CARWHEEL_FRONT_RIGHT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							break;
						case 'R':
							if (automobile->m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							if (automobile->m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							break;
						}
					}
					if (transmission->fMaxVelocity <= 0.f) {
						relativeChange = 0.f;
					} else if (currentGear != 0) {
						relativeGearChange =
						    Min(1.0f, (params.m_fVelocityChange - transmission->Gears[currentGear].fShiftDownVelocity) / transmission->fMaxVelocity * 2.5f);
						if (traction == 0.0f && automobile->GetStatus() != STATUS_SIMPLE &&
						    params.m_fVelocityChange < transmission->Gears[1].fShiftUpVelocity) {
							traction = 0.7f;
						}
						relativeChange = traction * automobile->m_fGasPedalAudio * 0.95f + (1.0f - traction) * relativeGearChange;
					} else
						relativeChange =
						    Min(1.0f, 1.0f - Abs((params.m_fVelocityChange - transmission->Gears[0].fShiftDownVelocity) / transmission->fMaxReverseVelocity));
				} else {
					if (automobile->m_nDriveWheelsOnGround != 0)
						automobile->m_fGasPedalAudio *= 0.4f;
					relativeChange = automobile->m_fGasPedalAudio;
				}
				modificator = relativeChange;
				if (currentGear != 0 || automobile->m_nWheelsOnGround == 0)
					freq = 1200 * currentGear + 18000.f * modificator + 14000;
				else
					freq = 13000.f * modificator + 14000;
				if (modificator >= 0.75f) {
					emittingVol = 120;
					volume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				} else {
					emittingVol = modificator * 4.0f / 3.0f * 40.f + 80.f;
					volume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				}
			} else {
				modificator = 0.f;
				emittingVol = 80;
				volume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
			}
			m_sQueueSample.m_nVolume = volume;
			if (m_sQueueSample.m_nVolume != 0) {
				if (automobile->GetStatus() == STATUS_SIMPLE) {
					if (modificator < 0.02f) {
						m_sQueueSample.m_nSampleIndex = aVehicleSettings[params.m_nIndex].m_nBank - CAR_SFX_BANKS_OFFSET + SFX_CAR_IDLE_1;
						freq = modificator * 10000 + 22050;
						m_sQueueSample.m_nCounter = 52;
					} else {
						m_sQueueSample.m_nSampleIndex = aVehicleSettings[params.m_nIndex].m_nAccelerationSampleIndex;
						m_sQueueSample.m_nCounter = 2;
					}
				} else {
					if (automobile->m_fGasPedal < 0.05f) {
						m_sQueueSample.m_nSampleIndex = aVehicleSettings[params.m_nIndex].m_nBank - CAR_SFX_BANKS_OFFSET + SFX_CAR_IDLE_1;
						freq = modificator * 10000 + 22050;
						m_sQueueSample.m_nCounter = 52;
					} else {
						m_sQueueSample.m_nSampleIndex = aVehicleSettings[params.m_nIndex].m_nAccelerationSampleIndex;
						m_sQueueSample.m_nCounter = 2;
					}
				}
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_nFrequency = freq + 100 * m_sQueueSample.m_nEntityIndex % 1000;
				if (m_sQueueSample.m_nSampleIndex == SFX_CAR_IDLE_6 || m_sQueueSample.m_nSampleIndex == SFX_CAR_REV_6)
					m_sQueueSample.m_nFrequency /= 2;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_fSpeedMultiplier = 6.0f;
				m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
				m_sQueueSample.m_bReleasingSoundFlag = false;
				m_sQueueSample.m_nReleasingVolumeDivider = 8;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
		}
	}
}

void
cAudioManager::UpdateGasPedalAudio(CAutomobile *automobile)
{
	float gasPedal = Abs(automobile->m_fGasPedal);
	float gasPedalAudio = automobile->m_fGasPedalAudio;

	if (gasPedalAudio < gasPedal)
		automobile->m_fGasPedalAudio = Min(gasPedalAudio + 0.09f, gasPedal);
	else
		automobile->m_fGasPedalAudio = Max(gasPedalAudio - 0.07f, gasPedal);
}

void
cAudioManager::PlayerJustGotInCar() const
{
	if (m_bIsInitialised)
		bPlayerJustEnteredCar = true;
}

void
cAudioManager::PlayerJustLeftCar(void) const
{
	// UNUSED: This is a perfectly empty function.
}

void
cAudioManager::AddPlayerCarSample(uint8 emittingVolume, int32 freq, uint32 sample, uint8 bank, uint8 counter, bool notLooping)
{
	m_sQueueSample.m_nVolume = ComputeVolume(emittingVolume, 50.f, m_sQueueSample.m_fDistance);
	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = counter;
		m_sQueueSample.m_nSampleIndex = sample;
#ifdef GTA_PS2
		m_sQueueSample.m_nBankIndex = bank;
#else
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#endif // GTA_PS2
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_nReleasingVolumeModificator = 0;
		m_sQueueSample.m_nFrequency = freq;
		if (notLooping) {
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nReleasingVolumeDivider = 8;
		} else {
			m_sQueueSample.m_nLoopCount = 1;
		}
		m_sQueueSample.m_nEmittingVolume = emittingVolume;
		m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_fSpeedMultiplier = 6.0f;
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.m_bReleasingSoundFlag = false;
		m_sQueueSample.m_bReverbFlag = true;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
	}
}

void
cAudioManager::ProcessCesna(cVehicleParams& params)
{
	static uint8 nAccel = 0;

	//((CAutomobile *)params.m_pVehicle)->Damage.GetEngineStatus();

	if (FindPlayerVehicle() == params.m_pVehicle) {
		if (params.m_nIndex == DODO) {
			if (Pads[0].GetAccelerate() <= 0) {
				if (nAccel != 0)
					--nAccel;
			} else if (nAccel < 60) {
				++nAccel;
			}
			AddPlayerCarSample(85 * (60 - nAccel) / 60 + 20, 8500 * nAccel / 60 + 17000, SFX_CESNA_IDLE, SFX_BANK_0, 52, true);
			AddPlayerCarSample(85 * nAccel / 60 + 20, 8500 * nAccel / 60 + 17000, SFX_CESNA_REV, SFX_BANK_0, 2, true);
		}
	} else if (params.m_nIndex == DODO) {
		AddPlayerCarSample(105, 17000, SFX_CESNA_IDLE, SFX_BANK_0, 52, true);
	} else if (params.m_fDistance < SQR(200)) {
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(80, 200.f, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 52;
			m_sQueueSample.m_nSampleIndex = SFX_CESNA_IDLE;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 0;
			m_sQueueSample.m_nFrequency = 12500;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nReleasingVolumeDivider = 8;
			m_sQueueSample.m_nEmittingVolume = 80;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 8.0f;
			m_sQueueSample.m_fSoundIntensity = 200.0f;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
		if (params.m_fDistance < SQR(90)) {
			m_sQueueSample.m_nVolume = ComputeVolume(80, 90.f, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 2;
				m_sQueueSample.m_nSampleIndex = SFX_CESNA_REV;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nReleasingVolumeModificator = 0;
				m_sQueueSample.m_nFrequency = 25000;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nReleasingVolumeDivider = 4;
				m_sQueueSample.m_nEmittingVolume = 80;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_fSpeedMultiplier = 8.0f;
				m_sQueueSample.m_fSoundIntensity = 90.0f;
				m_sQueueSample.m_bReleasingSoundFlag = false;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
		}
	}
}

void
cAudioManager::ProcessPlayersVehicleEngine(cVehicleParams& params, CAutomobile *automobile)
{
	static int32 GearFreqAdj[] = {6000, 6000, 3400, 1200, 0, -1000};

	cTransmission *transmission;
	float velocityChange;
	float relativeVelocityChange;
	float accelerationMultipler;
	uint8 wheelInUseCounter;
	float time;
	int baseFreq;
	uint8 vol;
	int gearNr;
	int32 freq;

	int freqModifier;
	int soundOffset;
	uint8 engineSoundType;
	int16 accelerateState;
	bool channelUsed;
	bool lostTraction;
	bool processedAccelSampleStopped;
	uint8 currentGear;
	float gasPedalAudio;
	CVector pos;

	static int16 LastAccel = 0;
	static int16 LastBrake = 0;
	static uint8 CurrentPretendGear = 1;
	static bool bLostTractionLastFrame = false;
	static bool bHandbrakeOnLastFrame = false;
	static int32 nCruising = 0;
	static bool bAccelSampleStopped = true;

	lostTraction = false;
	processedAccelSampleStopped = false;
	if (bPlayerJustEnteredCar) {
		bAccelSampleStopped = true;
		bPlayerJustEnteredCar = false;
		nCruising = 0;
		LastAccel = 0;
		bLostTractionLastFrame = false;
		LastBrake = 0;
		bHandbrakeOnLastFrame = false;
		CurrentPretendGear = 1;
	}
	if (CReplay::IsPlayingBack())
		accelerateState = 255.f * clamp(automobile->m_fGasPedal, 0.0f, 1.0f);
	else
		accelerateState = Pads[0].GetAccelerate();

	channelUsed = SampleManager.GetChannelUsedFlag(m_nActiveSamples);
	transmission = params.m_pTransmission;
	velocityChange = params.m_fVelocityChange;
	relativeVelocityChange = 2.0f * velocityChange / transmission->fMaxVelocity;

	accelerationMultipler = clamp(relativeVelocityChange, 0.0f, 1.0f);
	gasPedalAudio = accelerationMultipler;
	currentGear = params.m_pVehicle->m_nCurrentGear;

	switch (transmission->nDriveType)
	{
	case '4':
		wheelInUseCounter = 0;
		for (uint8 i = 0; i < ARRAY_SIZE(automobile->m_aWheelState); i++) {
			if (automobile->m_aWheelState[i] != WHEEL_STATE_NORMAL)
				++wheelInUseCounter;
		}
		if (wheelInUseCounter > 2)
			lostTraction = true;
		break;
	case 'F':
		if ((automobile->m_aWheelState[CARWHEEL_FRONT_LEFT] != WHEEL_STATE_NORMAL || automobile->m_aWheelState[CARWHEEL_FRONT_RIGHT] != WHEEL_STATE_NORMAL) &&
		    (automobile->m_aWheelState[CARWHEEL_REAR_LEFT] != WHEEL_STATE_NORMAL || automobile->m_aWheelState[CARWHEEL_REAR_RIGHT] != WHEEL_STATE_NORMAL))
			lostTraction = true;
		break;
	case 'R':
		if ((automobile->m_aWheelState[CARWHEEL_REAR_LEFT] != WHEEL_STATE_NORMAL) || (automobile->m_aWheelState[CARWHEEL_REAR_RIGHT] != WHEEL_STATE_NORMAL))
			lostTraction = true;
		break;
	}

	if (velocityChange != 0.0f) {
		time = params.m_pVehicle->m_vecMoveSpeed.z / velocityChange;
		if (time > 0.0f)
			freqModifier = -(Min(0.2f, time) * 3000.0f * 5.0f);
		else
			freqModifier = -(Max(-0.2f, time) * 3000.0f * 5.0f);
		if (params.m_fVelocityChange < -0.001f)
			freqModifier = -freqModifier;
	} else
		freqModifier = 0;

	engineSoundType = aVehicleSettings[params.m_nIndex].m_nBank;
	soundOffset = 3 * (engineSoundType - CAR_SFX_BANKS_OFFSET);
	if (accelerateState <= 0) {
		if (params.m_fVelocityChange < -0.001f) {
			if (channelUsed) {
				SampleManager.StopChannel(m_nActiveSamples);
				bAccelSampleStopped = true;
			}
			if (automobile->m_nWheelsOnGround == 0 || automobile->bIsHandbrakeOn || lostTraction)
				gasPedalAudio = automobile->m_fGasPedalAudio;
			else
				gasPedalAudio = Min(1.0f, params.m_fVelocityChange / params.m_pTransmission->fMaxReverseVelocity);

			gasPedalAudio = Max(0.0f, gasPedalAudio);
			automobile->m_fGasPedalAudio = gasPedalAudio;
		} else if (LastAccel > 0) {
			if (channelUsed) {
				SampleManager.StopChannel(m_nActiveSamples);
				bAccelSampleStopped = true;
			}
			nCruising = 0;
			if (automobile->m_nWheelsOnGround == 0 || automobile->bIsHandbrakeOn || lostTraction ||
			    params.m_fVelocityChange < 0.01f && automobile->m_fGasPedalAudio > 0.2f) {
				automobile->m_fGasPedalAudio *= 0.6f;
				gasPedalAudio = automobile->m_fGasPedalAudio;
			}
			if (gasPedalAudio > 0.05f) {
				freq = (5000.f * (gasPedalAudio - 0.05f) * 20.f / 19) + 19000;
				if (engineSoundType == SFX_BANK_TRUCK)
					freq /= 2;
				AddPlayerCarSample((25.f * (gasPedalAudio - 0.05f) * 20.f / 19) + 40, freq, (soundOffset + SFX_CAR_FINGER_OFF_ACCEL_1), engineSoundType, 63,
				                   false);
			}
		}
		freq = (10000.f * gasPedalAudio) + 22050;
		if (engineSoundType == SFX_BANK_TRUCK)
			freq /= 2;
		AddPlayerCarSample(110 - (40.f * gasPedalAudio), freq, (engineSoundType - CAR_SFX_BANKS_OFFSET + SFX_CAR_IDLE_1), SFX_BANK_0, 52, true);

		CurrentPretendGear = Max(1, currentGear);
	} else {
		while (nCruising == 0) {
			if (accelerateState < 150 || automobile->m_nWheelsOnGround == 0 || automobile->bIsHandbrakeOn || lostTraction ||
			    currentGear < 2 && velocityChange - automobile->m_fVelocityChangeForAudio < 0.01f) { // here could be used abs
				if (automobile->m_nWheelsOnGround == 0 || automobile->bIsHandbrakeOn || lostTraction) {
					if (automobile->m_nWheelsOnGround == 0 && automobile->m_nDriveWheelsOnGround != 0 ||
					    (automobile->bIsHandbrakeOn && !bHandbrakeOnLastFrame || lostTraction && !bLostTractionLastFrame) && automobile->m_nWheelsOnGround != 0) {
						automobile->m_fGasPedalAudio *= 0.6f;
					}
					freqModifier = 0;
					baseFreq = (15000.f * automobile->m_fGasPedalAudio) + 14000;
					vol = (25.0f * automobile->m_fGasPedalAudio) + 60;
				} else {
					baseFreq = (8000.f * accelerationMultipler) + 16000;
					vol = (25.0f * accelerationMultipler) + 60;
					automobile->m_fGasPedalAudio = accelerationMultipler;
				}
				freq = freqModifier + baseFreq;
				if (engineSoundType == SFX_BANK_TRUCK)
					freq /= 2;
				if (channelUsed) {
					SampleManager.StopChannel(m_nActiveSamples);
					bAccelSampleStopped = true;
				}
				AddPlayerCarSample(vol, freq, (engineSoundType - CAR_SFX_BANKS_OFFSET + SFX_CAR_REV_1), SFX_BANK_0, 2, true);
			} else {
				TranslateEntity(&m_sQueueSample.m_vecPos, &pos);
				if (bAccelSampleStopped) {
					if (CurrentPretendGear != 1 || currentGear != 2) {
						gearNr = currentGear - 1;
						if (gearNr < 1)
							gearNr = 1;
						CurrentPretendGear = gearNr;
					}
					processedAccelSampleStopped = true;
					bAccelSampleStopped = false;
				}

				if (!channelUsed) {
					if (!processedAccelSampleStopped) {
						if (CurrentPretendGear < params.m_pTransmission->nNumberOfGears - 1)
							++CurrentPretendGear;
						else {
							nCruising = 1;
							break; // while was used just for this fucking place
						}
					}

					if (!SampleManager.InitialiseChannel(m_nActiveSamples, soundOffset + SFX_CAR_ACCEL_1, SFX_BANK_0))
						return;
					SampleManager.SetChannelLoopCount(m_nActiveSamples, 1);
					SampleManager.SetChannelLoopPoints(m_nActiveSamples, 0, -1);
				}

				SampleManager.SetChannelEmittingVolume(m_nActiveSamples, 85);
				SampleManager.SetChannel3DPosition(m_nActiveSamples, pos.x, pos.y, pos.z);
				SampleManager.SetChannel3DDistances(m_nActiveSamples, 50.f, 12.5f);
				freq = GearFreqAdj[CurrentPretendGear] + freqModifier + 22050;
				if (engineSoundType == SFX_BANK_TRUCK)
					freq /= 2;
				SampleManager.SetChannelFrequency(m_nActiveSamples, freq);
				if (!channelUsed) {
					SampleManager.SetChannelReverbFlag(m_nActiveSamples, m_bDynamicAcousticModelingStatus != false);
					SampleManager.StartChannel(m_nActiveSamples);
				}
			}
			break;
		}
		if (nCruising != 0) {
			bAccelSampleStopped = true;
			if (accelerateState < 150 || automobile->m_nWheelsOnGround == 0 || automobile->bIsHandbrakeOn || lostTraction ||
			    currentGear < params.m_pTransmission->nNumberOfGears - 1) {
				nCruising = 0;
			} else {
				if (accelerateState >= 220 && params.m_fVelocityChange + 0.001f < automobile->m_fVelocityChangeForAudio) {
					if (nCruising < 800)
						++nCruising;
				} else if (nCruising > 3) {
					--nCruising;
				}
				freq = 27 * nCruising + freqModifier + 22050;
				if (engineSoundType == SFX_BANK_TRUCK)
					freq /= 2;
				AddPlayerCarSample(85, freq, (soundOffset + SFX_CAR_AFTER_ACCEL_1), engineSoundType, 64, true);
			}
		}
	}
	LastAccel = accelerateState;

	bHandbrakeOnLastFrame = !!automobile->bIsHandbrakeOn;
	bLostTractionLastFrame = lostTraction;
}

bool
cAudioManager::ProcessVehicleSkidding(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 40.0f;

	CAutomobile *automobile;
	cTransmission *transmission;
	int32 emittingVol;
	float newSkidVal = 0.0f;
	float skidVal = 0.0f;

	if (params.m_fDistance >= SQR(SOUND_INTENSITY))
		return false;
	automobile = (CAutomobile *)params.m_pVehicle;
	if (automobile->m_nWheelsOnGround == 0)
		return true;
	CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
	for (int32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelState); i++) {
		if (automobile->m_aWheelState[i] == WHEEL_STATE_NORMAL || automobile->Damage.GetWheelStatus(i) == WHEEL_STATUS_MISSING)
			continue;
		transmission = params.m_pTransmission;
		switch (transmission->nDriveType) {
		case '4':
			newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission, params.m_fVelocityChange);
			break;
		case 'F':
			if (i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)
				newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission, params.m_fVelocityChange);
			else
				newSkidVal = GetVehicleNonDriveWheelSkidValue(i, automobile, transmission, params.m_fVelocityChange);
			break;
		case 'R':
			if (i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT)
				newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission, params.m_fVelocityChange);
			else
				newSkidVal = GetVehicleNonDriveWheelSkidValue(i, automobile, transmission, params.m_fVelocityChange);
			break;
		default:
			break;
		}
		skidVal = Max(skidVal, newSkidVal);
	}

	if (skidVal > 0.0f) {
		emittingVol = 50.f * skidVal;
		m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 3;
			switch (params.m_pVehicle->m_nSurfaceTouched) {
			case SURFACE_GRASS:
			case SURFACE_HEDGE:
				m_sQueueSample.m_nSampleIndex = SFX_RAIN;
				emittingVol /= 4;
				m_sQueueSample.m_nFrequency = 13000.f * skidVal + 35000.f;
				m_sQueueSample.m_nVolume /= 4;
				if (m_sQueueSample.m_nVolume == 0)
					return true;
				break;
			case SURFACE_GRAVEL:
			case SURFACE_MUD_DRY:
			case SURFACE_SAND:
			case SURFACE_WATER:
				m_sQueueSample.m_nSampleIndex = SFX_GRAVEL_SKID;
				m_sQueueSample.m_nFrequency = 6000.f * skidVal + 10000.f;
				break;

			default:
				m_sQueueSample.m_nSampleIndex = SFX_SKID;
				m_sQueueSample.m_nFrequency = 5000.f * skidVal + 11000.f;
				break;
			}

			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 8;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
	return true;
}

float
cAudioManager::GetVehicleDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile, cTransmission *transmission, float velocityChange)
{
	float relativeVelChange = 0.0f;
	float gasPedalAudio = automobile->m_fGasPedalAudio;
	float velChange;
	float relativeVel;

	switch (automobile->m_aWheelState[wheel])
	{
	case WHEEL_STATE_SPINNING:
		if (gasPedalAudio > 0.4f)
			relativeVelChange = (gasPedalAudio - 0.4f) * (5.0f / 3.0f) / (4.0f / 3.0f);
		break;
	case WHEEL_STATE_SKIDDING:
		relativeVelChange = Min(1.0f, Abs(velocityChange) / transmission->fMaxVelocity);
		break;
	case WHEEL_STATE_FIXED:
		relativeVel = gasPedalAudio;
		if (relativeVel > 0.4f)
			relativeVel = (gasPedalAudio - 0.4f) * (5.0f / 3.0f);

		velChange = Abs(velocityChange);
		if (velChange > 0.04f)
			relativeVelChange = Min(1.0f, velChange / transmission->fMaxVelocity);
		if (relativeVel > relativeVelChange)
			relativeVelChange = relativeVel;

		break;
	default:
		break;
	}

	return Max(relativeVelChange, Min(1.0f, Abs(automobile->m_vecTurnSpeed.z) * 20.0f));
}

float
cAudioManager::GetVehicleNonDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile, cTransmission *transmission, float velocityChange)
{
	float relativeVelChange = 0.0f;

	if (automobile->m_aWheelState[wheel] == WHEEL_STATE_SKIDDING)
		relativeVelChange = Min(1.0f, Abs(velocityChange) / transmission->fMaxVelocity);

	return Max(relativeVelChange, Min(1.0f, Abs(automobile->m_vecTurnSpeed.z) * 20.0f));
}

void
cAudioManager::ProcessVehicleHorn(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 40.0f;

	CAutomobile *automobile;

	if (params.m_fDistance < SQR(SOUND_INTENSITY)) {
		automobile = (CAutomobile *)params.m_pVehicle;
		if ((!automobile->m_bSirenOrAlarm || !UsesSirenSwitching(params.m_nIndex)) && automobile->GetModelIndex() != MI_MRWHOOP) {
			if (automobile->m_nCarHornTimer) {
				if (params.m_pVehicle->GetStatus() != STATUS_PLAYER) {
					automobile->m_nCarHornTimer = Min(44, automobile->m_nCarHornTimer);
					if (automobile->m_nCarHornTimer == 44)
						automobile->m_nCarHornPattern = (m_FrameCounter + m_sQueueSample.m_nEntityIndex) & 7;
					if (!hornPatternsArray[automobile->m_nCarHornPattern][44 - automobile->m_nCarHornTimer])
						return;
				}

				CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
				m_sQueueSample.m_nVolume = ComputeVolume(80, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = 4;
					m_sQueueSample.m_nSampleIndex = aVehicleSettings[params.m_nIndex].m_nHornSample;
					m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nReleasingVolumeModificator = 2;
					m_sQueueSample.m_nFrequency = aVehicleSettings[params.m_nIndex].m_nHornFrequency;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_nEmittingVolume = 80;
					m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_fSpeedMultiplier = 5.0f;
					m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
					m_sQueueSample.m_bReleasingSoundFlag = false;
					m_sQueueSample.m_nReleasingVolumeDivider = 3;
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_bRequireReflection = false;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

bool
cAudioManager::UsesSiren(int32 model) const
{
	switch (model) {
	case FIRETRUK:
	case AMBULAN:
	case FBICAR:
	case POLICE:
	case ENFORCER:
	case PREDATOR:
		return true;
	default:
		return false;
	}
}

bool
cAudioManager::UsesSirenSwitching(int32 model) const
{
	switch (model) {
	case AMBULAN:
	case POLICE:
	case ENFORCER:
	case PREDATOR:
		return true;
	default:
		return false;
	}
}

bool
cAudioManager::ProcessVehicleSirenOrAlarm(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 110.0f;

	if (params.m_fDistance < SQR(SOUND_INTENSITY)) {
		CVehicle *veh = params.m_pVehicle;
		if (veh->m_bSirenOrAlarm == false && !veh->IsAlarmOn())
			return true;

		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(80, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 5;
			if (UsesSiren(params.m_nIndex)) {
				if (params.m_pVehicle->GetStatus() == STATUS_ABANDONED)
					return true;
				if (veh->m_nCarHornTimer && params.m_nIndex != FIRETRUK) {
					m_sQueueSample.m_nSampleIndex = SFX_SIREN_FAST;
					if (params.m_nIndex == FBICAR)
						m_sQueueSample.m_nFrequency = 16113;
					else
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SIREN_FAST);
					m_sQueueSample.m_nCounter = 60;
				} else {
					m_sQueueSample.m_nSampleIndex = aVehicleSettings[params.m_nIndex].m_nSirenOrAlarmSample;
					m_sQueueSample.m_nFrequency = aVehicleSettings[params.m_nIndex].m_nSirenOrAlarmFrequency;
				}
			} else {
				m_sQueueSample.m_nSampleIndex = aVehicleSettings[params.m_nIndex].m_nSirenOrAlarmSample;
				m_sQueueSample.m_nFrequency = aVehicleSettings[params.m_nIndex].m_nSirenOrAlarmFrequency;
			}
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = 80;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 7.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 5;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
			return true;
		} else
			return true;
	} else
		return false;
}

bool
cAudioManager::UsesReverseWarning(int32 model) const
{
	return model == LINERUN || model == FIRETRUK || model == TRASH || model == BUS || model == COACH;
}

bool
cAudioManager::ProcessVehicleReverseWarning(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 50.0f;

	CVehicle *veh = params.m_pVehicle;

	if (params.m_fDistance >= SQR(SOUND_INTENSITY))
		return false;

	if (veh->bEngineOn && veh->m_fGasPedal < 0.0f) {
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(60, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 12;
			m_sQueueSample.m_nSampleIndex = SFX_REVERSE_WARNING;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 2;
			m_sQueueSample.m_nFrequency = (100 * m_sQueueSample.m_nEntityIndex & 1023) + SampleManager.GetSampleBaseFrequency(SFX_REVERSE_WARNING);
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = 60;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
	return true;
}

bool
cAudioManager::ProcessVehicleDoors(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 40.0f;

	CAutomobile *automobile;
	int8 doorState;
	int32 emittingVol;
	float velocity;

	if (params.m_fDistance >= SQR(SOUND_INTENSITY))
		return false;

	automobile = (CAutomobile *)params.m_pVehicle;
	CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
	for (int32 i = 0; i < ARRAY_SIZE(automobile->Doors); i++) {
		if (automobile->Damage.GetDoorStatus(i) == DOOR_STATUS_SWINGING) {
			doorState = automobile->Doors[i].m_nDoorState;
			if (doorState == DOORST_OPEN || doorState == DOORST_CLOSED) {
				velocity = Min(0.3f, Abs(automobile->Doors[i].m_fAngVel));
				if (velocity > 0.0035f) {
					emittingVol = (100.f * velocity * 10.f / 3.f);
					m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
					if (m_sQueueSample.m_nVolume != 0) {
						m_sQueueSample.m_nCounter = i + 6;
						m_sQueueSample.m_nSampleIndex = m_anRandomTable[1] % 6 + SFX_COL_CAR_PANEL_1;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex) + RandomDisplacement(1000);
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_bIs2D = false;
						m_sQueueSample.m_nReleasingVolumeModificator = 10;
						m_sQueueSample.m_nLoopCount = 1;
						m_sQueueSample.m_nEmittingVolume = emittingVol;
						m_sQueueSample.m_nLoopStart = 0;
						m_sQueueSample.m_nLoopEnd = -1;
						m_sQueueSample.m_fSpeedMultiplier = 1.0f;
						m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
						m_sQueueSample.m_bReleasingSoundFlag = true;
						m_sQueueSample.m_bReverbFlag = true;
						m_sQueueSample.m_bRequireReflection = true;
						AddSampleToRequestedQueue();
					}
				}
			}
		}
	}
	return true;
}

bool
cAudioManager::ProcessAirBrakes(cVehicleParams& params)
{
	CAutomobile *automobile;
	uint8 rand;

	if (params.m_fDistance > SQR(30))
		return false;
	automobile = (CAutomobile *)params.m_pVehicle;
	if (!automobile->bEngineOn)
		return true;

	if ((automobile->m_fVelocityChangeForAudio < 0.025f || params.m_fVelocityChange >= 0.025f) &&
	    (automobile->m_fVelocityChangeForAudio > -0.025f || params.m_fVelocityChange <= 0.025f))
		return true;

	CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
	rand = m_anRandomTable[0] % 10 + 70;
	m_sQueueSample.m_nVolume = ComputeVolume(rand, 30.0f, m_sQueueSample.m_fDistance);
	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 13;
		m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_AIR_BRAKES);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_nReleasingVolumeModificator = 10;
		m_sQueueSample.m_nLoopCount = 1;
		m_sQueueSample.m_nEmittingVolume = rand;
		m_sQueueSample.m_nLoopStart = 0;
		m_sQueueSample.m_nLoopEnd = -1;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_fSoundIntensity = 30.0f;
		m_sQueueSample.m_bReleasingSoundFlag = true;
		m_sQueueSample.m_bReverbFlag = true;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
	}

	return true;
}

bool
cAudioManager::HasAirBrakes(int32 model) const
{
	return model == LINERUN || model == FIRETRUK || model == TRASH || model == BUS || model == COACH;
}

bool
cAudioManager::ProcessEngineDamage(cVehicleParams& params)
{
	const int engineDamageIntensity = 40;

	CAutomobile *veh;
	uint8 engineStatus;
	uint8 emittingVolume;

	if (params.m_fDistance >= SQR(engineDamageIntensity))
		return false;
	veh = (CAutomobile *)params.m_pVehicle;
	if (veh->bEngineOn) {
		engineStatus = veh->Damage.GetEngineStatus();
		if (engineStatus > 250 || engineStatus < 100)
			return true;
		if (engineStatus < 225) {
			m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
			emittingVolume = 6;
			m_sQueueSample.m_nReleasingVolumeModificator = 7;
			m_sQueueSample.m_nFrequency = 40000;
		} else {
			emittingVolume = 60;
			m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
			m_sQueueSample.m_nReleasingVolumeModificator = 7;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CAR_ON_FIRE);
		}
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(emittingVolume, engineDamageIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 28;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = emittingVolume;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = engineDamageIntensity;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
	return true;
}

bool
cAudioManager::ProcessCarBombTick(cVehicleParams& params)
{
	CAutomobile *automobile;

	if (params.m_fDistance >= SQR(40.f))
		return false;
	automobile = (CAutomobile *)params.m_pVehicle;
	if (automobile->bEngineOn && automobile->m_bombType == CARBOMB_TIMEDACTIVE) {
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(60, 40.f, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 35;
			m_sQueueSample.m_nSampleIndex = SFX_COUNTDOWN;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 0;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_COUNTDOWN);
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = 60;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 40.0f;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
	return true;
}

void
cAudioManager::ProcessVehicleOneShots(cVehicleParams& params)
{
	int16 event;
	uint8 emittingVol;
	float relVol;
	float vol;
	bool noReflections;
	float maxDist;

	static uint8 WaveIndex = 41;
	static uint8 GunIndex = 53;
	static uint8 iWheelIndex = 82;
	static uint8 CrunchOffset = 0;

	for (int i = 0; i < m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_AudioEvents; i++) {
		noReflections = 0;
		m_sQueueSample.m_bRequireReflection = false;
		event = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
		switch (event) {
		case SOUND_CAR_DOOR_CLOSE_BONNET:
		case SOUND_CAR_DOOR_CLOSE_BUMPER:
		case SOUND_CAR_DOOR_CLOSE_FRONT_LEFT:
		case SOUND_CAR_DOOR_CLOSE_FRONT_RIGHT:
		case SOUND_CAR_DOOR_CLOSE_BACK_LEFT:
		case SOUND_CAR_DOOR_CLOSE_BACK_RIGHT: {
			const float SOUND_INTENSITY = 50.0f;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[2] % 5 + 122;
			switch (aVehicleSettings[params.m_nIndex].m_bDoorType) {
			case OLD_DOOR:
				m_sQueueSample.m_nSampleIndex = SFX_OLD_CAR_DOOR_CLOSE;
				break;
			case NEW_DOOR:
				m_sQueueSample.m_nSampleIndex = SFX_NEW_CAR_DOOR_CLOSE;
				break;
			case TRUCK_DOOR:
				m_sQueueSample.m_nSampleIndex = SFX_TRUCK_DOOR_CLOSE;
				break;
			case BUS_DOOR:
				m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
				break;
			default:
				m_sQueueSample.m_nSampleIndex = SFX_NEW_CAR_DOOR_CLOSE;
				break;
			}
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#ifdef THIS_IS_STUPID
			m_sQueueSample.m_nCounter = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] + 22;
#else
			m_sQueueSample.m_nCounter = event + 22;
#endif
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		}
		case SOUND_CAR_DOOR_OPEN_BONNET:
		case SOUND_CAR_DOOR_OPEN_BUMPER:
		case SOUND_CAR_DOOR_OPEN_FRONT_LEFT:
		case SOUND_CAR_DOOR_OPEN_FRONT_RIGHT:
		case SOUND_CAR_DOOR_OPEN_BACK_LEFT:
		case SOUND_CAR_DOOR_OPEN_BACK_RIGHT: {
			const float SOUND_INTENSITY = 50.0f;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[1] % 10 + 117;
			switch (aVehicleSettings[params.m_nIndex].m_bDoorType) {
			case OLD_DOOR:
				m_sQueueSample.m_nSampleIndex = SFX_OLD_CAR_DOOR_OPEN;
				break;
			case NEW_DOOR:
			default:
				m_sQueueSample.m_nSampleIndex = SFX_NEW_CAR_DOOR_OPEN;
				break;
			case TRUCK_DOOR:
				m_sQueueSample.m_nSampleIndex = SFX_TRUCK_DOOR_OPEN;
				break;
			case BUS_DOOR:
				m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
				break;
			}
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#ifdef THIS_IS_STUPID
			m_sQueueSample.m_nCounter = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] + 10;
#else
			m_sQueueSample.m_nCounter = event + 10;
#endif
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		}
		case SOUND_CAR_WINDSHIELD_CRACK: {
			const float SOUND_INTENSITY = 30.0f;
			maxDist = SQR(SOUND_INTENSITY);
			m_sQueueSample.m_nSampleIndex = SFX_GLASS_CRACK;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 68;
			emittingVol = m_anRandomTable[1] % 30 + 60;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_GLASS_CRACK);
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
		} break;
		case SOUND_CAR_JUMP: {
			const float SOUND_INTENSITY = 35.0f;
			emittingVol = Max(80.f, 2 * (100.f * m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i]));
			maxDist = SQR(SOUND_INTENSITY);
			m_sQueueSample.m_nSampleIndex = SFX_TYRE_BUMP;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = iWheelIndex++;
			if (iWheelIndex > 85)
				iWheelIndex = 82;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_TYRE_BUMP);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
			if (params.m_nIndex == RCBANDIT) {
				m_sQueueSample.m_nFrequency *= 2;
				emittingVol /= 2;
			}
			m_sQueueSample.m_nReleasingVolumeModificator = 6;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			break;
		}
		case SOUND_CAR_ENGINE_START: {
			const float SOUND_INTENSITY = 40.0f;
			emittingVol = 60;
			maxDist = SQR(SOUND_INTENSITY);
			m_sQueueSample.m_nSampleIndex = SFX_CAR_STARTER;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 33;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CAR_STARTER);
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		}
		case SOUND_CAR_LIGHT_BREAK: {
			const float SOUND_INTENSITY = 30.0f;
			m_sQueueSample.m_nSampleIndex = SFX_GLASS_SHARD_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 37;
			m_sQueueSample.m_nFrequency = 9 * SampleManager.GetSampleBaseFrequency(SFX_GLASS_SHARD_1) / 10;
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 8);
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[4] % 10 + 30;
			break;
		}
		case SOUND_CAR_HYDRAULIC_1:
		case SOUND_CAR_HYDRAULIC_2: {
			const float SOUND_INTENSITY = 35.0f;
			if (event == SOUND_CAR_HYDRAULIC_1)
				m_sQueueSample.m_nFrequency = 15600;
			else
				m_sQueueSample.m_nFrequency = 13118;
			m_sQueueSample.m_nSampleIndex = SFX_SUSPENSION_FAST_MOVE;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 51;
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 8);
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[0] % 15 + 55;
			break;
		}
		case SOUND_CAR_HYDRAULIC_3: {
			const float SOUND_INTENSITY = 35.0f;
			m_sQueueSample.m_nSampleIndex = SFX_SUSPENSION_SLOW_MOVE_LOOP;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 86;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SUSPENSION_SLOW_MOVE_LOOP);
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			m_sQueueSample.m_nReleasingVolumeDivider = 7;
			noReflections = true;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[0] % 15 + 55;
			break;
		}
		case SOUND_CAR_JERK: {
			const float SOUND_INTENSITY = 35.0f;
			m_sQueueSample.m_nSampleIndex = SFX_SHAG_SUSPENSION;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 87;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SHAG_SUSPENSION);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 8);
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[1] % 15 + 55;
			break;
		}
		case SOUND_CAR_SPLASH: {
			const float SOUND_INTENSITY = 40.0f;
			vol = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			if (vol <= 300.f)
				continue;
			if (vol > 1200.f)
				m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i] = 1200.0f;
			relVol = (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i] - 300.f) / 900.f;
			m_sQueueSample.m_nSampleIndex = (m_anRandomTable[0] & 1) + SFX_BOAT_SPLASH_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = WaveIndex++;
			if (WaveIndex > 46)
				WaveIndex = 41;
			m_sQueueSample.m_nFrequency = (7000.f * relVol) + 6000;
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			emittingVol = (55.f * relVol);
			maxDist = SQR(SOUND_INTENSITY);
			break;
		}
		case SOUND_BOAT_SLOWDOWN: {
			const float SOUND_INTENSITY = 50.0f;
			m_sQueueSample.m_nSampleIndex = SFX_POLICE_BOAT_THUMB_OFF;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 47;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_POLICE_BOAT_THUMB_OFF) + RandomDisplacement(600);
			m_sQueueSample.m_nReleasingVolumeModificator = 2;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			emittingVol = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			maxDist = SQR(SOUND_INTENSITY);
			break;
		}
		case SOUND_TRAIN_DOOR_CLOSE:
		case SOUND_TRAIN_DOOR_OPEN: {
			const float SOUND_INTENSITY = 35.0f;
			m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 59;
			m_sQueueSample.m_nFrequency = RandomDisplacement(1000) + 11025;
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 5.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[1] % 20 + 70;
			break;
		}
		case SOUND_CAR_TANK_TURRET_ROTATE: {
			const float SOUND_INTENSITY = 40.0f;
			vol = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			if (vol > 96.0f / 2500.0f)
				vol = 96.0f / 2500.0f;
			m_sQueueSample.m_nSampleIndex = SFX_TANK_TURRET;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 79;
			m_sQueueSample.m_nFrequency = (3000.f * vol * 2500.0f / 96.0f) + 9000;
			m_sQueueSample.m_nReleasingVolumeModificator = 2;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			emittingVol = (37.f * vol * 2500.0f / 96.0f) + 90;
			maxDist = SQR(SOUND_INTENSITY);
			noReflections = true;
			break;
		}
		case SOUND_CAR_BOMB_TICK: {
			const float SOUND_INTENSITY = 30.0f;
			m_sQueueSample.m_nSampleIndex = SFX_BOMB_BEEP;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 80;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_BOMB_BEEP);
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			m_sQueueSample.m_bRequireReflection = true;
			emittingVol = 60;
			break;
		}
		case SOUND_PLANE_ON_GROUND: {
			const float SOUND_INTENSITY = 180.0f;
			m_sQueueSample.m_nSampleIndex = SFX_JUMBO_LAND_WHEELS;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 81;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_JUMBO_LAND_WHEELS);
			m_sQueueSample.m_nReleasingVolumeModificator = 2;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[4] % 25 + 75;
			break;
		}
		case SOUND_WEAPON_SHOT_FIRED: {
			const float SOUND_INTENSITY = 120.0f;
			emittingVol = m_anRandomTable[2];
			maxDist = SQR(SOUND_INTENSITY);
			m_sQueueSample.m_nSampleIndex = SFX_UZI_LEFT;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = GunIndex++;
			emittingVol = emittingVol % 15 + 65;
			if (GunIndex > 58)
				GunIndex = 53;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_UZI_LEFT);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			break;
		}
		case SOUND_WEAPON_HIT_VEHICLE: {
			const float SOUND_INTENSITY = 40.0f;
			m_sQueueSample.m_nSampleIndex = m_anRandomTable[m_sQueueSample.m_nEntityIndex % ARRAY_SIZE(m_anRandomTable)] % 6 + SFX_BULLET_CAR_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 34;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
			m_sQueueSample.m_nReleasingVolumeModificator = 7;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[3] % 20 + 90;
			break;
		}
		case SOUND_BOMB_TIMED_ACTIVATED:
		case SOUND_55:
		case SOUND_BOMB_ONIGNITION_ACTIVATED:
		case SOUND_BOMB_TICK: {
			const float SOUND_INTENSITY = 50.0f;
			m_sQueueSample.m_nSampleIndex = SFX_ARM_BOMB;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 36;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_ARM_BOMB);
			m_sQueueSample.m_nReleasingVolumeModificator = 0;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			m_sQueueSample.m_bRequireReflection = true;
			emittingVol = 50;
			maxDist = SQR(SOUND_INTENSITY);
			break;
		}
		case SOUND_PED_HELI_PLAYER_FOUND:
		{
			cPedParams pedParams;
			pedParams.m_bDistanceCalculated = params.m_bDistanceCalculated;
			pedParams.m_fDistance = params.m_fDistance;
			SetupPedComments(pedParams, SOUND_PED_HELI_PLAYER_FOUND);
			continue;
		}
		case SOUND_PED_BODYCAST_HIT:
		{
			cPedParams pedParams;
			pedParams.m_bDistanceCalculated = params.m_bDistanceCalculated;
			pedParams.m_fDistance = params.m_fDistance;
			SetupPedComments(pedParams, SOUND_PED_BODYCAST_HIT);
			continue;
		}
		case SOUND_WATER_FALL: {
			const float SOUND_INTENSITY = 40.0f;
			m_sQueueSample.m_nSampleIndex = SFX_SPLASH_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 15;
			m_sQueueSample.m_nFrequency = RandomDisplacement(1000) + 16000;
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			m_sQueueSample.m_bRequireReflection = true;
			emittingVol = m_anRandomTable[4] % 20 + 90;
			break;
		}
		case SOUND_SPLATTER: {
			const float SOUND_INTENSITY = 40.0f;
			m_sQueueSample.m_nSampleIndex = CrunchOffset + SFX_PED_CRUNCH_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 48;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_PED_CRUNCH_1) + RandomDisplacement(600);
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 40.0f;
			++CrunchOffset;
			maxDist = SQR(SOUND_INTENSITY);
			emittingVol = m_anRandomTable[4] % 20 + 55;
			CrunchOffset %= 2;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		}
		case SOUND_CAR_PED_COLLISION: {
			const float SOUND_INTENSITY = 40.0f;
			vol = Min(20.0f, m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i]);
			emittingVol = (vol / 20.0f * 127.f);
			if (!emittingVol)
				continue;

			m_sQueueSample.m_nSampleIndex = (m_anRandomTable[2] & 3) + SFX_FIGHT_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 50;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex) / 2;
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			maxDist = SQR(SOUND_INTENSITY);
			break;
		}
		default:
			continue;
		}
		if (params.m_fDistance < maxDist) {
			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				if (noReflections) {
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_bReleasingSoundFlag = false;
				} else {
					m_sQueueSample.m_nLoopCount = 1;
					m_sQueueSample.m_bReleasingSoundFlag = true;
				}
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bIs2D = false;
				AddSampleToRequestedQueue();
			}
		}
	}
}

bool
cAudioManager::ProcessTrainNoise(cVehicleParams& params)
{
	const float SOUND_INTENSITY = 300.0f;

	CTrain *train;
	uint8 emittingVol;
	float speedMultipler;

	if (params.m_fDistance >= SQR(SOUND_INTENSITY))
		return false;

	if (params.m_fVelocityChange > 0.0f) {
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		train = (CTrain *)params.m_pVehicle;
		speedMultipler = Min(1.0f, train->m_fSpeed * 250.f / 51.f);
		emittingVol = (75.f * speedMultipler);
		if (train->m_fWagonPosition == 0.0f) {
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 32;
				m_sQueueSample.m_nSampleIndex = SFX_TRAIN_FAR;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nReleasingVolumeModificator = 2;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_TRAIN_FAR);
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_fSpeedMultiplier = 3.0f;
				m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
				m_sQueueSample.m_bReleasingSoundFlag = false;
				m_sQueueSample.m_nReleasingVolumeDivider = 3;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
		}
		const float SOUND_INTENSITY = 70.0f;
		if (params.m_fDistance < SQR(SOUND_INTENSITY)) {
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 33;
				m_sQueueSample.m_nSampleIndex = SFX_TRAIN_NEAR;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nReleasingVolumeModificator = 5;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_TRAIN_NEAR) + 100 * m_sQueueSample.m_nEntityIndex % 987;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_fSpeedMultiplier = 6.0f;
				m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
				m_sQueueSample.m_bReleasingSoundFlag = false;
				m_sQueueSample.m_nReleasingVolumeDivider = 3;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
		}
	}
	return true;
}

bool
cAudioManager::ProcessBoatEngine(cVehicleParams& params)
{
	CBoat *boat;
	float padRelativeAccerate;
	float gasPedal;
	float padAccelerate;
	uint8 emittingVol;
	float oneShotVol;

	static uint16 LastAccel = 0;
	static uint8 LastVol = 0;

	static const float intensity = 50.0f;

	if (params.m_fDistance < SQR(intensity)) {
		boat = (CBoat *)params.m_pVehicle;
		if (params.m_nIndex == REEFER) {
			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			m_sQueueSample.m_nVolume = ComputeVolume(80, intensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 39;
				m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
				m_sQueueSample.m_nFrequency = 10386;
				m_sQueueSample.m_nFrequency += (m_sQueueSample.m_nEntityIndex * 65536) % 1000;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nEmittingVolume = 80;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				m_sQueueSample.m_fSoundIntensity = intensity;
				m_sQueueSample.m_bReleasingSoundFlag = false;
				m_sQueueSample.m_nReleasingVolumeDivider = 7;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
			if (FindPlayerVehicle() == params.m_pVehicle) {
				padAccelerate = Max(Pads[0].GetAccelerate(), Pads[0].GetBrake());
				padRelativeAccerate = padAccelerate / 255.0f;
				emittingVol = (100.f * padRelativeAccerate) + 15;
				m_sQueueSample.m_nFrequency = (3000.f * padRelativeAccerate) + 6000;
				if (!boat->bPropellerInWater)
					m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
			} else {
				gasPedal = Abs(boat->m_fGasPedal);
				if (gasPedal > 0.0f) {
					m_sQueueSample.m_nFrequency = 6000;
					emittingVol = 15;
				} else {
					emittingVol = (100.f * gasPedal) + 15;
					m_sQueueSample.m_nFrequency = (3000.f * gasPedal) + 6000;
					if (!boat->bPropellerInWater)
						m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
				}
			}
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, intensity, m_sQueueSample.m_fDistance);
			if (!m_sQueueSample.m_nVolume)
				return true;
			m_sQueueSample.m_nCounter = 40;
			m_sQueueSample.m_nSampleIndex = SFX_POLICE_BOAT_ACCEL;
			m_sQueueSample.m_nFrequency += (m_sQueueSample.m_nEntityIndex * 65536) % 1000;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = intensity;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 7;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
		} else {
			if (FindPlayerVehicle() == params.m_pVehicle) {
				padAccelerate = Max(Pads[0].GetAccelerate(), Pads[0].GetBrake());
				if (padAccelerate <= 20) {
					emittingVol = 45 - 45 * padAccelerate / 40;
					m_sQueueSample.m_nFrequency = 100 * padAccelerate + 11025;
					m_sQueueSample.m_nCounter = 39;
					m_sQueueSample.m_nSampleIndex = SFX_POLICE_BOAT_IDLE;
					if (LastAccel > 20) {
						oneShotVol = LastVol;
						PlayOneShot(m_sQueueSample.m_nEntityIndex, SOUND_BOAT_SLOWDOWN, oneShotVol);
					}
				} else {
					emittingVol = 105 * padAccelerate / 255 + 15;
					m_sQueueSample.m_nFrequency = 4000 * padAccelerate / 255 + 8000;
					if (!boat->m_bIsAnchored)
						m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
					m_sQueueSample.m_nCounter = 40;
					m_sQueueSample.m_nSampleIndex = SFX_POLICE_BOAT_ACCEL;
				}
				LastVol = emittingVol;
				LastAccel = padAccelerate;
			} else {
				gasPedal = Abs(boat->m_fGasPedal);
				if (gasPedal > 0.0f) {
					m_sQueueSample.m_nFrequency = 11025;
					emittingVol = 45;
					m_sQueueSample.m_nCounter = 39;
					m_sQueueSample.m_nSampleIndex = SFX_POLICE_BOAT_IDLE;
				} else {
					emittingVol = (105.f * gasPedal) + 15;
					m_sQueueSample.m_nFrequency = (4000.f * gasPedal) + 8000;
					if (!boat->m_bIsAnchored)
						m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
					m_sQueueSample.m_nCounter = 40;
					m_sQueueSample.m_nSampleIndex = SFX_POLICE_BOAT_ACCEL;
				}
			}
			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, intensity, m_sQueueSample.m_fDistance);
			if (!m_sQueueSample.m_nVolume)
				return true;
			m_sQueueSample.m_nFrequency += (m_sQueueSample.m_nEntityIndex * 65536) % 1000;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = intensity;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 7;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
		}
		AddSampleToRequestedQueue();
		return true;
	}
	return false;
}

bool
cAudioManager::ProcessBoatMovingOverWater(cVehicleParams& params)
{
	float velocityChange;
	int32 vol;
	float multiplier;

	if (params.m_fDistance > SQR(50))
		return false;

	velocityChange = Abs(params.m_fVelocityChange);
	if (velocityChange <= 0.0005f && ((CBoat*)params.m_pVehicle)->bBoatInWater)
		return true;

	velocityChange = Min(0.75f, velocityChange);
	multiplier = (velocityChange - 0.0005f) / (1499.0f / 2000.0f);
	CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
	vol = (30.f * multiplier);
	m_sQueueSample.m_nVolume = ComputeVolume(vol, 50.f, m_sQueueSample.m_fDistance);
	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 38;
		m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nFrequency = (6050.f * multiplier) + 16000;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_nEmittingVolume = vol;
		m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.m_bReleasingSoundFlag = false;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_bReverbFlag = true;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
	}

	return true;
}

struct tHelicopterSampleData {
	float m_fMaxDistance;
	float m_fBaseDistance;
	uint8 m_bBaseVolume;
};

bool
cAudioManager::ProcessHelicopter(cVehicleParams& params)
{
	CHeli *heli;
	float MaxDist;
	float dist;
	float baseDist;
	int32 emittingVol;
	static const tHelicopterSampleData gHeliSfxRanges[3] = {{400.f, 380.f, 100}, {100.f, 70.f, MAX_VOLUME}, {60.f, 30.f, MAX_VOLUME}};

	if (SQR(gHeliSfxRanges[0].m_fMaxDistance) <= params.m_fDistance)
		return false;

	CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
	heli = (CHeli *)params.m_pVehicle;
	for (uint32 i = 0; i < ARRAY_SIZE(gHeliSfxRanges); i++) {
		MaxDist = gHeliSfxRanges[i].m_fMaxDistance;
		dist = m_sQueueSample.m_fDistance;
		if (dist >= MaxDist)
			return true;
		baseDist = gHeliSfxRanges[i].m_fBaseDistance;
		if (dist < baseDist)
			emittingVol = (gHeliSfxRanges[i].m_bBaseVolume * ((MaxDist - dist) / (MaxDist - baseDist)));
		else
			emittingVol = gHeliSfxRanges[i].m_bBaseVolume;

		m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, gHeliSfxRanges[i].m_fMaxDistance, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = i + 65;
			m_sQueueSample.m_nSampleIndex = i + SFX_HELI_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 0;
			m_sQueueSample.m_nFrequency = 1200 * heli->m_nHeliId + SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 6.0f;
			m_sQueueSample.m_fSoundIntensity = gHeliSfxRanges[i].m_fMaxDistance;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
	return true;
}

void
cAudioManager::ProcessPlane(cVehicleParams& params)
{
	switch (params.m_nIndex) {
	case AIRTRAIN:
		ProcessJumbo(params);
		break;
	case DEADDODO:
		ProcessCesna(params);
		break;
	default:
		debug("Plane Model Id is %d\n, ", params.m_pVehicle->GetModelIndex());
		break;
	}
}

#pragma region JUMBO
uint8 gJumboVolOffsetPercentage;

void
DoJumboVolOffset()
{
	if (!(AudioManager.GetFrameCounter() % (AudioManager.GetRandomNumber(0) % 6 + 3)))
		gJumboVolOffsetPercentage = AudioManager.GetRandomNumber(1) % 60;
}

void
cAudioManager::ProcessJumbo(cVehicleParams& params)
{
	CPlane *plane;
	float position;

	if (params.m_fDistance < SQR(440)) {
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		plane = (CPlane *)params.m_pVehicle;
		DoJumboVolOffset();
		position = PlanePathPosition[plane->m_nPlaneId];
		if (position <= TakeOffPoint) {
			if (plane->m_fSpeed <= 0.103344f) {
				ProcessJumboTaxi();
				return;
			}

			ProcessJumboAccel(plane);
		} else if (300.0f + TakeOffPoint >= position) {
			ProcessJumboTakeOff(plane);
		} else if (LandingPoint - 350.0f >= position) {
			ProcessJumboFlying();
		} else {
			if (position > LandingPoint) {
				if (plane->m_fSpeed > 0.103344f) {
					ProcessJumboDecel(plane);
					return;
				}
				ProcessJumboTaxi();
				return;
			}
			ProcessJumboLanding(plane);
		}
	}
}

void
cAudioManager::ProcessJumboTaxi()
{
	if (SetupJumboFlySound(20)) {
		if (SetupJumboTaxiSound(75))
			SetupJumboWhineSound(18, 29500);
	}
}

void
cAudioManager::ProcessJumboAccel(CPlane *plane)
{
	int32 engineFreq;
	int32 vol;
	float whineSoundFreq;
	float modificator;

	if (SetupJumboFlySound(20)) {
		modificator = (plane->m_fSpeed - 0.103344f) * 1.6760077f;
		if (modificator > 1.0f)
			modificator = 1.0f;
		if (SetupJumboRumbleSound(MAX_VOLUME * modificator) && SetupJumboTaxiSound((1.0f - modificator) * 75.f)) {
			if (modificator < 0.2f) {
				whineSoundFreq = modificator * 5.f * 14600.0f + 29500;
				vol = modificator * 5.f * MAX_VOLUME;
				engineFreq = modificator * 5.f * 6050.f + 16000;
			} else {
				whineSoundFreq = 44100;
				engineFreq = 22050;
				vol = MAX_VOLUME;
			}
			SetupJumboEngineSound(vol, engineFreq);
			SetupJumboWhineSound(18, whineSoundFreq);
		}
	}
}

void
cAudioManager::ProcessJumboTakeOff(CPlane *plane)
{
	const float modificator = (PlanePathPosition[plane->m_nPlaneId] - TakeOffPoint) / 300.f;

	if (SetupJumboFlySound((107.f * modificator) + 20) && SetupJumboRumbleSound(MAX_VOLUME * (1.f - modificator))) {
		if (SetupJumboEngineSound(MAX_VOLUME, 22050))
			SetupJumboWhineSound(18.f * (1.f - modificator), 44100);
	}
}

void
cAudioManager::ProcessJumboFlying()
{
	if (SetupJumboFlySound(MAX_VOLUME))
		SetupJumboEngineSound(63, 22050);
}

void
cAudioManager::ProcessJumboLanding(CPlane *plane)
{
	const float modificator = (LandingPoint - PlanePathPosition[plane->m_nPlaneId]) / 350.f;
	if (SetupJumboFlySound(107.f * modificator + 20)) {
		if (SetupJumboTaxiSound(75.f * (1.f - modificator))) {
			SetupJumboEngineSound(MAX_VOLUME, 22050);
			SetupJumboWhineSound(18.f * (1.f - modificator), 14600.f * modificator + 29500);
		}
	}
}

void
cAudioManager::ProcessJumboDecel(CPlane *plane)
{
	if (SetupJumboFlySound(20) && SetupJumboTaxiSound(75)) {
		const float modificator = Min(1.f, (plane->m_fSpeed - 0.103344f) * 1.6760077f);
		SetupJumboEngineSound(MAX_VOLUME * modificator, 6050.f * modificator + 16000);
		SetupJumboWhineSound(18, 29500);
	}
}

bool
cAudioManager::SetupJumboTaxiSound(uint8 vol)
{
	const float SOUND_INTENSITY = 180.0f;
	if (m_sQueueSample.m_fDistance >= SOUND_INTENSITY)
		return false;

	uint8 emittingVol = (vol / 2) + ((vol / 2) * m_sQueueSample.m_fDistance / SOUND_INTENSITY);

	if (m_sQueueSample.m_fDistance / SOUND_INTENSITY < 0.7f)
		emittingVol -= emittingVol * gJumboVolOffsetPercentage / 100;
	m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);

	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 1;
		m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_nReleasingVolumeModificator = 1;
		m_sQueueSample.m_nFrequency = GetJumboTaxiFreq();
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_nEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
		m_sQueueSample.m_bReleasingSoundFlag = false;
		m_sQueueSample.m_nReleasingVolumeDivider = 4;
		m_sQueueSample.m_bReverbFlag = true;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
	}
	return true;
}

bool
cAudioManager::SetupJumboWhineSound(uint8 emittingVol, uint32 freq)
{
	const float SOUND_INTENSITY = 170.0f;

	if (m_sQueueSample.m_fDistance >= SOUND_INTENSITY)
		return false;

	m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);

	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 2;
		m_sQueueSample.m_nSampleIndex = SFX_JUMBO_WHINE;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_nReleasingVolumeModificator = 1;
		m_sQueueSample.m_nFrequency = freq;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_nEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
		m_sQueueSample.m_bReleasingSoundFlag = false;
		m_sQueueSample.m_nReleasingVolumeDivider = 4;
		m_sQueueSample.m_bReverbFlag = true;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
	}
	return true;
}

bool
cAudioManager::SetupJumboEngineSound(uint8 vol, uint32 freq)
{
	const float SOUND_INTENSITY = 180.0f;
	if (m_sQueueSample.m_fDistance >= SOUND_INTENSITY)
		return false;

	uint8 emittingVol = vol - gJumboVolOffsetPercentage / 100;
	m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 3;
		m_sQueueSample.m_nSampleIndex = SFX_JUMBO_ENGINE;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_nReleasingVolumeModificator = 1;
		m_sQueueSample.m_nFrequency = freq;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_nEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
		m_sQueueSample.m_bReleasingSoundFlag = false;
		m_sQueueSample.m_nReleasingVolumeDivider = 4;
		m_sQueueSample.m_bReverbFlag = true;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
	}
	return true;
}

bool
cAudioManager::SetupJumboFlySound(uint8 emittingVol)
{
	const float SOUND_INTENSITY = 440.0f;
	if (m_sQueueSample.m_fDistance >= SOUND_INTENSITY)
		return false;

	int32 vol = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
	m_sQueueSample.m_nVolume = vol;
	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nSampleIndex = SFX_JUMBO_DIST_FLY;
		m_sQueueSample.m_nCounter = 0;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_nReleasingVolumeModificator = 1;
		m_sQueueSample.m_nEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_JUMBO_DIST_FLY);
		m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
		m_sQueueSample.m_bReleasingSoundFlag = false;
		m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		m_sQueueSample.m_bReverbFlag = true;
		m_sQueueSample.m_nReleasingVolumeDivider = 5;
		m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		AddSampleToRequestedQueue();
	}
	return true;
}

bool
cAudioManager::SetupJumboRumbleSound(uint8 emittingVol)
{
	const float SOUND_INTENSITY = 240.0f;
	if (m_sQueueSample.m_fDistance >= SOUND_INTENSITY)
		return false;

	m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);

	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 5;
		m_sQueueSample.m_nSampleIndex = SFX_JUMBO_RUMBLE;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_bIs2D = true;
		m_sQueueSample.m_nReleasingVolumeModificator = 1;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_JUMBO_RUMBLE);
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_nEmittingVolume = emittingVol;
		m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
		m_sQueueSample.m_bReleasingSoundFlag = false;
		m_sQueueSample.m_nReleasingVolumeDivider = 12;
		m_sQueueSample.m_nOffset = 0;
		m_sQueueSample.m_bReverbFlag = true;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
		m_sQueueSample.m_nCounter = 6;
		m_sQueueSample.m_nSampleIndex = SFX_JUMBO_RUMBLE;
		m_sQueueSample.m_nFrequency += 200;
		m_sQueueSample.m_nOffset = MAX_VOLUME;
		AddSampleToRequestedQueue();
	}
	return true;
}

int32
cAudioManager::GetJumboTaxiFreq() const
{
	return (60.833f * m_sQueueSample.m_fDistance) + 22050;
}

#pragma endregion Some jumbo crap

#pragma endregion All the vehicle audio code

#pragma region PED AUDIO
void
cAudioManager::ProcessPed(CPhysical *ped)
{
	cPedParams params;

	m_sQueueSample.m_vecPos = ped->GetPosition();

	params.m_bDistanceCalculated = false;
	params.m_pPed = (CPed *)ped;
	params.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (ped->GetModelIndex() == MI_FATMALE02)
		ProcessPedHeadphones(params);
	ProcessPedOneShots(params);
}

void
cAudioManager::ProcessPedHeadphones(cPedParams &params)
{
	CPed *ped;
	CAutomobile *veh;
	uint8 emittingVol;

	if (params.m_fDistance < SQR(7)) {
		ped = params.m_pPed;
		if (!ped->bIsAimingGun || ped->m_bodyPartBleeding != PED_HEAD) {
			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			if (ped->bInVehicle && ped->m_nPedState == PED_DRIVING) {
				emittingVol = 10;
				veh = (CAutomobile *)ped->m_pMyVehicle;
				if (veh && veh->IsCar()) {
					for (int32 i = DOOR_FRONT_LEFT; i < ARRAY_SIZE(veh->Doors); i++) {
						if (!veh->IsDoorClosed((eDoors)i) || veh->IsDoorMissing((eDoors)i)) {
							emittingVol = 42;
							break;
						}
					}
				}
			} else {
				emittingVol = 42;
			}

			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, 7.f, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 64;
				m_sQueueSample.m_nSampleIndex = SFX_HEADPHONES;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nReleasingVolumeModificator = 5;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_HEADPHONES);
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_fSpeedMultiplier = 4.0f;
				m_sQueueSample.m_fSoundIntensity = 7.0f;
				m_sQueueSample.m_bReleasingSoundFlag = false;
				m_sQueueSample.m_nReleasingVolumeDivider = 5;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
			return;
		}
	}
}

void
cAudioManager::ProcessPedOneShots(cPedParams &params)
{
	uint8 emittingVol;
	int32 sampleIndex;

	CPed *ped = params.m_pPed;

	bool narrowSoundRange;
	int16 sound;
	bool stereo;
	CWeapon *weapon;
	float maxDist = 0.f; // uninitialized variable

	static uint8 iSound = 21;

	weapon = params.m_pPed->GetWeapon();
	for (uint32 i = 0; i < m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_AudioEvents; i++) {
		stereo = false;
		narrowSoundRange = false;
		m_sQueueSample.m_bRequireReflection = false;
		sound = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
		switch (sound) {
		case SOUND_STEP_START:
		case SOUND_STEP_END:
			if (params.m_pPed->bIsInTheAir)
				continue;
			emittingVol = m_anRandomTable[3] % 15 + 45;
			if (FindPlayerPed() != m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_pEntity)
				emittingVol /= 2;
			maxDist = 400.f;
			switch (params.m_pPed->m_nSurfaceTouched) {
			case SURFACE_GRASS:
				sampleIndex = m_anRandomTable[1] % 5 + SFX_FOOTSTEP_GRASS_1;
				break;
			case SURFACE_GRAVEL:
			case SURFACE_MUD_DRY:
				sampleIndex = m_anRandomTable[4] % 5 + SFX_FOOTSTEP_GRAVEL_1;
				break;
			case SURFACE_CAR:
			case SURFACE_GARAGE_DOOR:
			case SURFACE_CAR_PANEL:
			case SURFACE_THICK_METAL_PLATE:
			case SURFACE_SCAFFOLD_POLE:
			case SURFACE_LAMP_POST:
			case SURFACE_FIRE_HYDRANT:
			case SURFACE_GIRDER:
			case SURFACE_METAL_CHAIN_FENCE:
			case SURFACE_CONTAINER:
			case SURFACE_NEWS_VENDOR:
				sampleIndex = m_anRandomTable[0] % 5 + SFX_FOOTSTEP_METAL_1;
				break;
			case SURFACE_SAND:
				sampleIndex = (m_anRandomTable[4] & 3) + SFX_FOOTSTEP_SAND_1;
				break;
			case SURFACE_WATER:
				sampleIndex = (m_anRandomTable[3] & 3) + SFX_FOOTSTEP_WATER_1;
				break;
			case SURFACE_WOOD_CRATES:
			case SURFACE_WOOD_BENCH:
			case SURFACE_WOOD_SOLID:
				sampleIndex = m_anRandomTable[2] % 5 + SFX_FOOTSTEP_WOOD_1;
				break;
			case SURFACE_HEDGE:
				sampleIndex = m_anRandomTable[2] % 5 + SFX_COL_VEG_1;
				break;
			default:
				sampleIndex = m_anRandomTable[2] % 5 + SFX_FOOTSTEP_CONCRETE_1;
				break;
			}
			m_sQueueSample.m_nSampleIndex = sampleIndex;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] - SOUND_STEP_START + 1;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 17);
			switch (params.m_pPed->m_nMoveState) {
			case PEDMOVE_WALK:
				emittingVol /= 4;
				m_sQueueSample.m_nFrequency = 9 * m_sQueueSample.m_nFrequency / 10;
				break;
			case PEDMOVE_RUN:
				emittingVol /= 2;
				m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
				break;
			case PEDMOVE_SPRINT:
				m_sQueueSample.m_nFrequency = 12 * m_sQueueSample.m_nFrequency / 10;
				break;
			default:
				break;
			}
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 20.0f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		case SOUND_FALL_LAND:
		case SOUND_FALL_COLLAPSE:
			if (ped->bIsInTheAir)
				continue;
			maxDist = SQR(30);
			emittingVol = m_anRandomTable[3] % 20 + 80;
			if (ped->m_nSurfaceTouched == SURFACE_WATER) {
				m_sQueueSample.m_nSampleIndex = (m_anRandomTable[3] & 3) + SFX_FOOTSTEP_WATER_1;
			} else if (sound == SOUND_FALL_LAND) {
				m_sQueueSample.m_nSampleIndex = SFX_BODY_LAND;
			} else {
				m_sQueueSample.m_nSampleIndex = SFX_BODY_LAND_AND_FALL;
			}
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = 1;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 17);
			m_sQueueSample.m_nReleasingVolumeModificator = 2;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		case SOUND_FIGHT_PUNCH_33:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			m_sQueueSample.m_nFrequency = 18000;
			goto AddFightSound;
		case SOUND_FIGHT_KICK_34:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			m_sQueueSample.m_nFrequency = 16500;
			goto AddFightSound;
		case SOUND_FIGHT_HEADBUTT_35:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			m_sQueueSample.m_nFrequency = 20000;
			goto AddFightSound;
		case SOUND_FIGHT_PUNCH_36:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			m_sQueueSample.m_nFrequency = 18000;
			goto AddFightSound;
		case SOUND_FIGHT_PUNCH_37:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			m_sQueueSample.m_nFrequency = 16500;
			goto AddFightSound;
		case SOUND_FIGHT_CLOSE_PUNCH_38:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			m_sQueueSample.m_nFrequency = 20000;
			goto AddFightSound;
		case SOUND_FIGHT_PUNCH_39:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			m_sQueueSample.m_nFrequency = 18000;
			goto AddFightSound;
		case SOUND_FIGHT_PUNCH_OR_KICK_BELOW_40:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			m_sQueueSample.m_nFrequency = 16500;
			goto AddFightSound;
		case SOUND_FIGHT_PUNCH_41:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			m_sQueueSample.m_nFrequency = 20000;
			goto AddFightSound;
		case SOUND_FIGHT_PUNCH_FROM_BEHIND_42:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			m_sQueueSample.m_nFrequency = 18000;
			goto AddFightSound;
		case SOUND_FIGHT_KNEE_OR_KICK_43:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			m_sQueueSample.m_nFrequency = 16500;
			goto AddFightSound;
		case SOUND_FIGHT_KICK_44:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			m_sQueueSample.m_nFrequency = 20000;
		AddFightSound:
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = iSound;
			narrowSoundRange = true;
			++iSound;
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = SQR(30);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[3] % 26 + 100;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		case SOUND_WEAPON_BAT_ATTACK:
			m_sQueueSample.m_nSampleIndex = SFX_BAT_HIT_LEFT;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = true;
			m_sQueueSample.m_nFrequency = RandomDisplacement(2000) + 22000;
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = SQR(30);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			emittingVol = m_anRandomTable[2] % 20 + 100;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			if (m_bDynamicAcousticModelingStatus)
				m_sQueueSample.m_bRequireReflection = true;
			else
				stereo = true;
			break;
		case SOUND_WEAPON_SHOT_FIRED:
			weapon = ped->GetWeapon();
			switch (weapon->m_eWeaponType) {
			case WEAPONTYPE_COLT45:
				m_sQueueSample.m_nSampleIndex = SFX_COLT45_LEFT;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = true;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_COLT45_LEFT);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 50.0f;
				maxDist = SQR(50);
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				emittingVol = m_anRandomTable[1] % 10 + 90;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				if (m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = true;
				else
					stereo = true;
				break;
			case WEAPONTYPE_UZI:
				m_sQueueSample.m_nSampleIndex = SFX_UZI_LEFT;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = true;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_UZI_LEFT);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 80.0f;
				maxDist = SQR(80);
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				emittingVol = m_anRandomTable[3] % 15 + 70;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				break;
			case WEAPONTYPE_SHOTGUN:
				m_sQueueSample.m_nSampleIndex = SFX_SHOTGUN_LEFT;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = true;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SHOTGUN_LEFT);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 60.0f;
				maxDist = SQR(60);
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				emittingVol = m_anRandomTable[2] % 10 + 100;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				if (m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = true;
				else
					stereo = true;
				break;
			case WEAPONTYPE_AK47:
				m_sQueueSample.m_nSampleIndex = SFX_AK47_LEFT;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = true;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_AK47_LEFT);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 80.0f;
				maxDist = SQR(80);
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				emittingVol = m_anRandomTable[1] % 15 + 70;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				break;
			case WEAPONTYPE_M16:
				m_sQueueSample.m_nSampleIndex = SFX_M16_LEFT;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = true;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_M16_LEFT);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 80.0f;
				maxDist = SQR(80);
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				emittingVol = m_anRandomTable[4] % 15 + 70;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				break;
			case WEAPONTYPE_SNIPERRIFLE:
				m_sQueueSample.m_nSampleIndex = SFX_SNIPER_LEFT;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = true;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SNIPER_LEFT);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 60.0f;
				maxDist = SQR(60);
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				emittingVol = m_anRandomTable[4] % 10 + 110;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				if (m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = true;
				else
					stereo = true;
				break;
			case WEAPONTYPE_ROCKETLAUNCHER:
				m_sQueueSample.m_nSampleIndex = SFX_ROCKET_LEFT;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = true;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_ROCKET_LEFT);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
				m_sQueueSample.m_nReleasingVolumeModificator = 1;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_fSoundIntensity = 90.0f;
				maxDist = SQR(90);
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				emittingVol = m_anRandomTable[0] % 20 + 80;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				if (m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = true;
				else
					stereo = true;
				break;
			case WEAPONTYPE_FLAMETHROWER:
				m_sQueueSample.m_nSampleIndex = SFX_FLAMETHROWER_LEFT;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nCounter = 9;
				emittingVol = 90;
				m_sQueueSample.m_nFrequency = (10 * m_sQueueSample.m_nEntityIndex & 2047) + SampleManager.GetSampleBaseFrequency(SFX_FLAMETHROWER_LEFT);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 4.0f;
				m_sQueueSample.m_fSoundIntensity = 60.0f;
				maxDist = SQR(60);
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nEmittingVolume = 90;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_bReleasingSoundFlag = false;
				m_sQueueSample.m_nReleasingVolumeDivider = 6;
				if (m_bDynamicAcousticModelingStatus)
					m_sQueueSample.m_bRequireReflection = true;
				else
					stereo = true;
				break;
			default:
				continue;
			}

			break;
		case SOUND_WEAPON_RELOAD:
			weapon = &ped->m_weapons[ped->m_currentWeapon];
			switch (weapon->m_eWeaponType) {
			case WEAPONTYPE_COLT45:
				m_sQueueSample.m_nSampleIndex = SFX_PISTOL_RELOAD;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_PISTOL_RELOAD) + RandomDisplacement(300);
				break;
			case WEAPONTYPE_UZI:
				m_sQueueSample.m_nSampleIndex = SFX_M16_RELOAD;
				m_sQueueSample.m_nFrequency = 39243;
				break;
			case WEAPONTYPE_SHOTGUN:
				m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
				m_sQueueSample.m_nFrequency = 30290;
				break;
			case WEAPONTYPE_AK47:
				m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_AK47_RELOAD);
				break;
			case WEAPONTYPE_M16:
				m_sQueueSample.m_nSampleIndex = SFX_M16_RELOAD;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_M16_RELOAD);
				break;
			case WEAPONTYPE_SNIPERRIFLE:
				m_sQueueSample.m_nSampleIndex = SFX_RIFLE_RELOAD;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RIFLE_RELOAD);
				break;
			case WEAPONTYPE_ROCKETLAUNCHER:
				m_sQueueSample.m_nSampleIndex = SFX_ROCKET_RELOAD;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_ROCKET_RELOAD);
				break;
			default:
				continue;
			}
			emittingVol = 75;
			m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = true;
			m_sQueueSample.m_nFrequency += RandomDisplacement(300);
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = SQR(30);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_nEmittingVolume = 75;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		case SOUND_WEAPON_AK47_BULLET_ECHO:
		case SOUND_WEAPON_UZI_BULLET_ECHO:
		case SOUND_WEAPON_M16_BULLET_ECHO:
			m_sQueueSample.m_nSampleIndex = SFX_UZI_END_LEFT;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = true;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_UZI_END_LEFT);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 80.0f;
			maxDist = SQR(80);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			emittingVol = m_anRandomTable[4] % 10 + 40;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			if (m_bDynamicAcousticModelingStatus)
				m_sQueueSample.m_bRequireReflection = true;
			else
				stereo = true;
			break;
		case SOUND_WEAPON_FLAMETHROWER_FIRE:
			m_sQueueSample.m_nSampleIndex = SFX_FLAMETHROWER_START_LEFT;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = iSound++;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_FLAMETHROWER_START_LEFT);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			m_sQueueSample.m_fSoundIntensity = 60.0f;
			maxDist = SQR(60);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			emittingVol = 70;
			m_sQueueSample.m_nEmittingVolume = 70;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			break;
		case SOUND_WEAPON_HIT_PED:
			m_sQueueSample.m_nSampleIndex = SFX_BULLET_PED;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = true;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_BULLET_PED);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 8);
			m_sQueueSample.m_nReleasingVolumeModificator = 7;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 30.0f;
			maxDist = SQR(30);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[0] % 20 + 90;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			break;
		case SOUND_SPLASH:
			m_sQueueSample.m_nSampleIndex = SFX_SPLASH_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = true;
			m_sQueueSample.m_nFrequency = RandomDisplacement(1400) + 20000;
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = 40.0f;
			maxDist = SQR(40);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nLoopStart = 0;
			emittingVol = m_anRandomTable[2] % 30 + 70;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_nEmittingVolume = emittingVol;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			m_sQueueSample.m_bRequireReflection = true;
			break;
		default:
			SetupPedComments(params, sound);
			continue;
		}

		if (narrowSoundRange && iSound > 60)
			iSound = 21;
		if (params.m_fDistance < maxDist) {
			CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				if (stereo) {
					if (m_sQueueSample.m_fDistance < 0.2f * m_sQueueSample.m_fSoundIntensity) {
						m_sQueueSample.m_bIs2D = true;
						m_sQueueSample.m_nOffset = 0;
					} else {
						stereo = false;
					}
				}
				m_sQueueSample.m_bReverbFlag = true;
				AddSampleToRequestedQueue();
				if (stereo) {
					m_sQueueSample.m_nOffset = 127;
					++m_sQueueSample.m_nSampleIndex;
					if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] != SOUND_WEAPON_SHOT_FIRED ||
					    weapon->m_eWeaponType != WEAPONTYPE_FLAMETHROWER) {
						m_sQueueSample.m_nCounter = iSound++;
						if (iSound > 60)
							iSound = 21;
					} else {
						++m_sQueueSample.m_nCounter;
					}
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::SetupPedComments(cPedParams &params, uint16 sound)
{
	CPed *ped = params.m_pPed;
	uint8 emittingVol;
	float soundIntensity;
	tPedComment pedComment;

	if (ped != nil) {
		switch (sound) {
		case SOUND_AMMUNATION_WELCOME_1:
			pedComment.m_nSampleIndex = SFX_AMMU_D;
			break;
		case SOUND_AMMUNATION_WELCOME_2:
			pedComment.m_nSampleIndex = SFX_AMMU_E;
			break;
		case SOUND_AMMUNATION_WELCOME_3:
			pedComment.m_nSampleIndex = SFX_AMMU_F;
			break;
		default:
			pedComment.m_nSampleIndex = GetPedCommentSfx(ped, sound);
			if (pedComment.m_nSampleIndex == NO_SAMPLE)
				return;
			break;
		}

		soundIntensity = 50.0f;
	} else {
		switch (sound) {
		case SOUND_PED_HELI_PLAYER_FOUND:
			soundIntensity = 400.0f;
			pedComment.m_nSampleIndex = GetRandomNumberInRange(m_sQueueSample.m_nEntityIndex % 4, SFX_POLICE_HELI_1, SFX_POLICE_HELI_29);
			break;
		case SOUND_PED_BODYCAST_HIT:
			if (CTimer::GetTimeInMilliseconds() <= gNextCryTime)
				return;
			soundIntensity = 50.0f;
			gNextCryTime = CTimer::GetTimeInMilliseconds() + 500;
			pedComment.m_nSampleIndex = GetRandomNumberInRange(m_sQueueSample.m_nEntityIndex % 4, SFX_PLASTER_BLOKE_1, SFX_PLASTER_BLOKE_4);
			break;
		case SOUND_INJURED_PED_MALE_OUCH:
		case SOUND_INJURED_PED_MALE_PRISON:
			soundIntensity = 50.0f;
			pedComment.m_nSampleIndex = GetRandomNumberInRange(m_sQueueSample.m_nEntityIndex % 4, SFX_GENERIC_MALE_GRUNT_1, SFX_GENERIC_MALE_GRUNT_15);
			break;
		case SOUND_INJURED_PED_FEMALE:
			soundIntensity = 50.0f;
			pedComment.m_nSampleIndex = GetRandomNumberInRange(m_sQueueSample.m_nEntityIndex % 4, SFX_GENERIC_FEMALE_GRUNT_1, SFX_GENERIC_FEMALE_GRUNT_11);
			break;
		default:
			return;
		}
	}

	if (params.m_fDistance < SQR(soundIntensity)) {
		CalculateDistance(params.m_bDistanceCalculated, params.m_fDistance);
		if (sound != SOUND_PAGER) {
			switch (sound) {
			case SOUND_AMMUNATION_WELCOME_1:
			case SOUND_AMMUNATION_WELCOME_2:
			case SOUND_AMMUNATION_WELCOME_3:
				emittingVol = MAX_VOLUME;
				break;
			default:
				if (CWorld::GetIsLineOfSightClear(TheCamera.GetPosition(), m_sQueueSample.m_vecPos, true, false, false, false, false, false))
					emittingVol = MAX_VOLUME;
				else
					emittingVol = 31;
				break;
			}
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, soundIntensity, m_sQueueSample.m_fDistance);
			pedComment.m_nProcess = 10;
			if (m_sQueueSample.m_nVolume != 0) {
				pedComment.m_nEntityIndex = m_sQueueSample.m_nEntityIndex;
				pedComment.m_vecPos = m_sQueueSample.m_vecPos;
				pedComment.m_fDistance = m_sQueueSample.m_fDistance;
				pedComment.m_bVolume = m_sQueueSample.m_nVolume;
				m_sPedComments.Add(&pedComment);
			}
		}
	}
}

int32
cAudioManager::GetPedCommentSfx(CPed *ped, int32 sound)
{
	if (ped->IsPlayer())
		return GetPlayerTalkSfx(sound);

	switch (ped->GetModelIndex()) {
	case MI_COP:
		return GetCopTalkSfx(sound);
	case MI_SWAT:
		return GetSwatTalkSfx(sound);
	case MI_FBI:
		return GetFBITalkSfx(sound);
	case MI_ARMY:
		return GetArmyTalkSfx(sound);
	case MI_MEDIC:
		return GetMedicTalkSfx(sound);
	case MI_FIREMAN:
		return GetFiremanTalkSfx(sound);
	case MI_MALE01:
		return GetNormalMaleTalkSfx(sound);
	case MI_TAXI_D:
		return GetTaxiDriverTalkSfx(sound);
	case MI_PIMP:
		return GetPimpTalkSfx(sound);
	case MI_GANG01:
	case MI_GANG02:
		return GetMafiaTalkSfx(sound);
	case MI_GANG03:
	case MI_GANG04:
		return GetTriadTalkSfx(sound);
	case MI_GANG05:
	case MI_GANG06:
		return GetDiabloTalkSfx(sound);
	case MI_GANG07:
	case MI_GANG08:
		return GetYakuzaTalkSfx(sound);
	case MI_GANG09:
	case MI_GANG10:
		return GetYardieTalkSfx(sound);
	case MI_GANG11:
	case MI_GANG12:
		return GetColumbianTalkSfx(sound);
	case MI_GANG13:
	case MI_GANG14:
		return GetHoodTalkSfx(sound);
	case MI_CRIMINAL01:
		return GetBlackCriminalTalkSfx(sound);
	case MI_CRIMINAL02:
		return GetWhiteCriminalTalkSfx(sound);
	case MI_SPECIAL01:
	case MI_SPECIAL02:
	case MI_SPECIAL03:
	case MI_SPECIAL04:
		return GetSpecialCharacterTalkSfx(ped->GetModelIndex(), sound);
	case MI_MALE02:
		return GetMaleNo2TalkSfx(sound);
	case MI_MALE03:
	case MI_P_MAN1:
	case MI_P_MAN2:
		return GetBlackProjectMaleTalkSfx(sound, ped->GetModelIndex());
	case MI_FATMALE01:
		return GetWhiteFatMaleTalkSfx(sound);
	case MI_FATMALE02:
		return GetBlackFatMaleTalkSfx(sound);
	case MI_FEMALE01:
		return GetBlackCasualFemaleTalkSfx(sound);
	case MI_FEMALE02:
	case MI_CAS_WOM:
		return GetWhiteCasualFemaleTalkSfx(sound);
	case MI_FEMALE03:
		return GetFemaleNo3TalkSfx(sound);
	case MI_FATFEMALE01:
		return GetBlackFatFemaleTalkSfx(sound);
	case MI_FATFEMALE02:
		return GetWhiteFatFemaleTalkSfx(sound);
	case MI_PROSTITUTE:
		return GetBlackFemaleProstituteTalkSfx(sound);
	case MI_PROSTITUTE2:
		return GetWhiteFemaleProstituteTalkSfx(sound);
	case MI_P_WOM1:
		return GetBlackProjectFemaleOldTalkSfx(sound);
	case MI_P_WOM2:
		return GetBlackProjectFemaleYoungTalkSfx(sound);
	case MI_CT_MAN1:
		return GetChinatownMaleOldTalkSfx(sound);
	case MI_CT_MAN2:
		return GetChinatownMaleYoungTalkSfx(sound);
	case MI_CT_WOM1:
		return GetChinatownFemaleOldTalkSfx(sound);
	case MI_CT_WOM2:
		return GetChinatownFemaleYoungTalkSfx(sound);
	case MI_LI_MAN1:
	case MI_LI_MAN2:
		return GetLittleItalyMaleTalkSfx(sound);
	case MI_LI_WOM1:
		return GetLittleItalyFemaleOldTalkSfx(sound);
	case MI_LI_WOM2:
		return GetLittleItalyFemaleYoungTalkSfx(sound);
	case MI_DOCKER1:
		return GetWhiteDockerMaleTalkSfx(sound);
	case MI_DOCKER2:
		return GetBlackDockerMaleTalkSfx(sound);
	case MI_SCUM_MAN:
		return GetScumMaleTalkSfx(sound);
	case MI_SCUM_WOM:
		return GetScumFemaleTalkSfx(sound);
	case MI_WORKER1:
		return GetWhiteWorkerMaleTalkSfx(sound);
	case MI_WORKER2:
		return GetBlackWorkerMaleTalkSfx(sound);
	case MI_B_MAN1:
	case MI_B_MAN3:
		return GetBusinessMaleYoungTalkSfx(sound, ped->GetModelIndex());
	case MI_B_MAN2:
		return GetBusinessMaleOldTalkSfx(sound);
	case MI_B_WOM1:
	case MI_B_WOM2:
		return GetWhiteBusinessFemaleTalkSfx(sound, ped->GetModelIndex());
	case MI_B_WOM3:
		return GetBlackBusinessFemaleTalkSfx(sound);
	case MI_MOD_MAN:
		return GetSupermodelMaleTalkSfx(sound);
	case MI_MOD_WOM:
		return GetSupermodelFemaleTalkSfx(sound);
	case MI_ST_MAN:
		return GetStewardMaleTalkSfx(sound);
	case MI_ST_WOM:
		return GetStewardFemaleTalkSfx(sound);
	case MI_FAN_MAN1:
	case MI_FAN_MAN2:
		return GetFanMaleTalkSfx(sound, ped->GetModelIndex());
	case MI_FAN_WOM:
		return GetFanFemaleTalkSfx(sound);
	case MI_HOS_MAN:
		return GetHospitalMaleTalkSfx(sound);
	case MI_HOS_WOM:
		return GetHospitalFemaleTalkSfx(sound);
	case MI_CONST1:
		return GetWhiteConstructionWorkerTalkSfx(sound);
	case MI_CONST2:
		return GetBlackConstructionWorkerTalkSfx(sound);
	case MI_SHOPPER1:
	case MI_SHOPPER2:
	case MI_SHOPPER3:
		return GetShopperFemaleTalkSfx(sound, ped->GetModelIndex());
	case MI_STUD_MAN:
		return GetStudentMaleTalkSfx(sound);
	case MI_STUD_WOM:
		return GetStudentFemaleTalkSfx(sound);
	case MI_CAS_MAN:
		return GetCasualMaleOldTalkSfx(sound);
	default:
		return GetGenericMaleTalkSfx(sound);
	}
}

void
cAudioManager::GetPhrase(uint32 &phrase, uint32 &prevPhrase, uint32 sample, uint32 maxOffset) const
{
	phrase = sample + m_anRandomTable[m_sQueueSample.m_nEntityIndex & 3] % maxOffset;

	// check if the same sfx like last time, if yes, then try use next one,
	// if exceeded range, then choose first available sample
	if (phrase == prevPhrase && ++phrase >= sample + maxOffset)
		phrase = sample;
	prevPhrase = phrase;
}

#pragma region PED_COMMENTS

uint32
cAudioManager::GetPlayerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_DAMAGE:
		GetPhrase(sfx, lastSfx, SFX_CLAUDE_HIGH_DAMAGE_GRUNT_1, 11);
		break;
	case SOUND_PED_HIT:
		GetPhrase(sfx, lastSfx, SFX_CLAUDE_LOW_DAMAGE_GRUNT_1, 10);
		break;
	case SOUND_PED_LAND:
		GetPhrase(sfx, lastSfx, SFX_CLAUDE_HIT_GROUND_GRUNT_1, 6);
		break;
	default:
		sfx = NO_SAMPLE;
		break;
	}
	return sfx;
}

uint32
cAudioManager::GetCopTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_ARREST_COP:
		GetPhrase(sfx, lastSfx, SFX_COP_VOICE_1_ARREST_1, 6);
		break;
	case SOUND_PED_PURSUIT_COP:
		pedState = FindPlayerPed()->m_nPedState;
		if (pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(sfx, lastSfx, SFX_COP_VOICE_1_CHASE_1, 7);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}

	return (SFX_COP_VOICE_2_ARREST_1 - SFX_COP_VOICE_1_ARREST_1) * (m_sQueueSample.m_nEntityIndex % 5) + sfx;
}

uint32
cAudioManager::GetSwatTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_ARREST_SWAT:
		GetPhrase(sfx, lastSfx, SFX_SWAT_VOICE_1_CHASE_1, 6);
		break;
	case SOUND_PED_PURSUIT_SWAT:
		pedState = FindPlayerPed()->m_nPedState;
		if (pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(sfx, lastSfx, SFX_SWAT_VOICE_1_CHASE_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}

	return (SFX_SWAT_VOICE_2_CHASE_1 - SFX_SWAT_VOICE_1_CHASE_1) * (m_sQueueSample.m_nEntityIndex % 4) + sfx;
}

uint32
cAudioManager::GetFBITalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_ARREST_FBI:
		GetPhrase(sfx, lastSfx, SFX_FBI_VOICE_1_CHASE_1, 6);
		break;
	case SOUND_PED_PURSUIT_FBI:
		pedState = FindPlayerPed()->m_nPedState;
		if (pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(sfx, lastSfx, SFX_FBI_VOICE_1_CHASE_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}

	return (SFX_FBI_VOICE_2_CHASE_1 - SFX_FBI_VOICE_1_CHASE_1) * (m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32
cAudioManager::GetArmyTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_PURSUIT_ARMY:
		pedState = FindPlayerPed()->m_nPedState;
		if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE) return NO_SAMPLE;
		GetPhrase(sfx, lastSfx, SFX_ARMY_VOICE_1_CHASE_1, 15);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}

	return (SFX_ARMY_VOICE_2_CHASE_1 - SFX_ARMY_VOICE_1_CHASE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetMedicTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_MEDIC_VOICE_1_GUN_PANIC_1, 5);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_MEDIC_VOICE_1_CARJACKED_1, 5);
		break;
	case SOUND_PED_HEALING:
		GetPhrase(sfx, lastSfx, SFX_MEDIC_VOICE_1_AT_VICTIM_1, 12);
		break;
	case SOUND_PED_LEAVE_VEHICLE:
		GetPhrase(sfx, lastSfx, SFX_MEDIC_VOICE_1_GET_OUT_VAN_CHAT_1, 9);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_MEDIC_VOICE_1_RUN_FROM_FIGHT_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return (SFX_MEDIC_VOICE_2_GUN_PANIC_1 - SFX_MEDIC_VOICE_1_GUN_PANIC_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetFiremanTalkSfx(int16 sound)
{
	return GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetNormalMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_NORMAL_MALE_GUN_PANIC_1, 7);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_NORMAL_MALE_CARJACKED_1, 7);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_NORMAL_MALE_DODGE_1, 9);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_NORMAL_MALE_RUN_FROM_FIGHT_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_NORMAL_MALE_DRIVER_ABUSE_1, 12);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_NORMAL_MALE_EYING_1, 8);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_NORMAL_MALE_SHOCKED_1, 10);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_NORMAL_MALE_CHAT_1, 25);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetTaxiDriverTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_ASIAN_TAXI_DRIVER_VOICE_1_CARJACKED_1, 7);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_ASIAN_TAXI_DRIVER_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}

	return (SFX_ASIAN_TAXI_DRIVER_VOICE_2_DRIVER_ABUSE_1 - SFX_ASIAN_TAXI_DRIVER_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetPimpTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_PIMP_GUN_COOL_1, 7);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_PIMP_CARJACKED_1, 4);
		break;
	case SOUND_PED_DEFEND:
		GetPhrase(sfx, lastSfx, SFX_PIMP_FIGHT_1, 9);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_PIMP_DODGE_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_PIMP_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_PIMP_SHOCKED_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_PIMP_CHAT_1, 17);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetMafiaTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(sfx, lastSfx, SFX_MAFIA_MALE_VOICE_1_CARJACKING_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_MAFIA_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_MAFIA_MALE_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_MAFIA_MALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_MAFIA_MALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_MAFIA_MALE_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_MAFIA_MALE_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return (SFX_MAFIA_MALE_VOICE_2_DRIVER_ABUSE_1 - SFX_MAFIA_MALE_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32
cAudioManager::GetTriadTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_TRIAD_MALE_VOICE_1_GUN_COOL_1, 3);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(sfx, lastSfx, SFX_TRIAD_MALE_VOICE_1_CARJACKING_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_TRIAD_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_TRIAD_MALE_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_TRIAD_MALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_TRIAD_MALE_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_TRIAD_MALE_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_TRIAD_MALE_VOICE_1_CHAT_1, 8);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetDiabloTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_DIABLO_MALE_VOICE_1_GUN_COOL_1, 4);
		break;
	case SOUND_PED_HANDS_COWER:
		sound = SOUND_PED_FLEE_SPRINT;
		return GetGenericMaleTalkSfx(sound);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(sfx, lastSfx, SFX_DIABLO_MALE_VOICE_1_CARJACKING_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_DIABLO_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_DIABLO_MALE_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_DIABLO_MALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_DIABLO_MALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_DIABLO_MALE_VOICE_1_EYING_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_DIABLO_MALE_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return (SFX_DIABLO_MALE_VOICE_2_CHAT_1 - SFX_DIABLO_MALE_VOICE_1_CHAT_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetYakuzaTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(sfx, lastSfx, SFX_YAKUZA_MALE_VOICE_1_CARJACKING_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_YAKUZA_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_YAKUZA_MALE_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_YAKUZA_MALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_YAKUZA_MALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_YAKUZA_MALE_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return (SFX_YAKUZA_MALE_VOICE_2_DRIVER_ABUSE_1 - SFX_YAKUZA_MALE_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetYardieTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		sfx = SFX_YARDIE_MALE_VOICE_1_GUN_COOL_1;
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(sfx, lastSfx, SFX_YARDIE_MALE_VOICE_1_CARJACKING_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		sfx = SFX_YARDIE_MALE_VOICE_1_CARJACKED_1;
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_YARDIE_MALE_VOICE_1_FIGHT_1, 6);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_YARDIE_MALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_YARDIE_MALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_YARDIE_MALE_VOICE_1_EYING_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_YARDIE_MALE_VOICE_1_CHAT_1, 8);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return (SFX_YARDIE_MALE_VOICE_2_DRIVER_ABUSE_1 - SFX_YARDIE_MALE_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetColumbianTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(sfx, lastSfx, SFX_COLUMBIAN_MALE_VOICE_1_CARJACKING_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_COLUMBIAN_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_COLUMBIAN_MALE_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_COLUMBIAN_MALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_COLUMBIAN_MALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_COLUMBIAN_MALE_VOICE_1_EYING_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_COLUMBIAN_MALE_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return (SFX_COLUMBIAN_MALE_VOICE_2_DRIVER_ABUSE_1 - SFX_COLUMBIAN_MALE_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetHoodTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_HOOD_MALE_VOICE_1_GUN_COOL_1, 5);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(sfx, lastSfx, SFX_HOOD_MALE_VOICE_1_CARJACKING_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_HOOD_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_HOOD_MALE_VOICE_1_FIGHT_1, 6);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_HOOD_MALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_HOOD_MALE_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_HOOD_MALE_VOICE_1_EYING_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_HOOD_MALE_VOICE_1_CHAT_1, 6);
		break;

	default:
		return GetGenericMaleTalkSfx(sound);
		break;
	}
	return (SFX_HOOD_MALE_VOICE_2_DRIVER_ABUSE_1 - SFX_HOOD_MALE_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetBlackCriminalTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CRIMINAL_VOICE_1_GUN_COOL_1, 4);
		break;
	case SOUND_PED_CAR_JACKING:
		sfx = SFX_BLACK_CRIMINAL_VOICE_1_CARJACKING_1;
		break;
	case SOUND_PED_MUGGING:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CRIMINAL_VOICE_1_MUGGING_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CRIMINAL_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CRIMINAL_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CRIMINAL_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
		break;
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteCriminalTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CRIMINAL_VOICE_1_GUN_COOL_1, 3);
		break;
	case SOUND_PED_CAR_JACKING:
		sfx = SFX_WHITE_CRIMINAL_VOICE_1_CARJACKING_1;
		break;
	case SOUND_PED_MUGGING:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CRIMINAL_VOICE_1_MUGGING_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CRIMINAL_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CRIMINAL_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CRIMINAL_VOICE_1_DRIVER_ABUSE_1, 4);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
		break;
	}
	return sfx;
}

uint32
cAudioManager::GetMaleNo2TalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_CASUAL_MALE_OLD_VOICE_1_CARJACKED_1, 3);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_CASUAL_MALE_OLD_VOICE_1_MUGGED_1, 4);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_CASUAL_MALE_OLD_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_CASUAL_MALE_OLD_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_CASUAL_MALE_OLD_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_CASUAL_MALE_OLD_VOICE_1_EYING_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_CASUAL_MALE_OLD_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackProjectMaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP: GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_MALE_VOICE_1_GUN_COOL_1, 3); break;
	case SOUND_PED_CAR_JACKED: GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_MALE_VOICE_1_CARJACKED_1, 2); break;
	case SOUND_PED_ROBBED: GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_MALE_VOICE_1_MUGGED_1, 2); break;
	case SOUND_PED_ATTACK: GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_MALE_VOICE_1_FIGHT_1, 6); break;
	case SOUND_PED_EVADE: GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_MALE_VOICE_1_DODGE_1, 5); break;
	case SOUND_PED_ANNOYED_DRIVER: GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_MALE_VOICE_1_DRIVER_ABUSE_1, 7); break;
	case SOUND_PED_CHAT_SEXY: GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_MALE_VOICE_1_EYING_1, 3); break;
	case SOUND_PED_CHAT: GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_MALE_VOICE_1_CHAT_1, 6); break;
	default: return GetGenericMaleTalkSfx(sound);
	}

	if (model == MI_P_MAN2)
		sfx += (SFX_BLACK_PROJECT_MALE_VOICE_2_DRIVER_ABUSE_1 - SFX_BLACK_PROJECT_MALE_VOICE_1_DRIVER_ABUSE_1);
	return sfx;
}

uint32
cAudioManager::GetWhiteFatMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED: GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_MALE_VOICE_1_CARJACKED_1, 3); break;
	case SOUND_PED_ROBBED: GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_MALE_VOICE_1_MUGGED_1, 3); break;
	case SOUND_PED_EVADE: GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_MALE_VOICE_1_DODGE_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_MALE_VOICE_1_DRIVER_ABUSE_1, 9); break;
	case SOUND_PED_WAIT_DOUBLEBACK: GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_MALE_VOICE_1_LOST_1, 2); break;
	case SOUND_PED_CHAT: GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_MALE_VOICE_1_CHAT_1, 9); break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFatMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_MALE_VOICE_1_CARJACKED_1, 4);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_MALE_VOICE_1_MUGGED_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_MALE_VOICE_1_DODGE_1, 7);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_MALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_MALE_VOICE_1_LOST_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_MALE_VOICE_1_CHAT_1, 8);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackCasualFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_1_VOICE_1_GUN_PANIC_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_1_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_1_VOICE_1_MUGGED_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_1_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_1_VOICE_1_RUN_FROM_FIGHT_1, 2);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_1_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_1_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_1_VOICE_1_CHAT_1, 8);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteCasualFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CASUAL_FEMALE_VOICE_1_GUN_PANIC_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CASUAL_FEMALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		sfx = SFX_WHITE_CASUAL_FEMALE_VOICE_1_MUGGED_1;
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CASUAL_FEMALE_VOICE_1_DODGE_1, 3);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CASUAL_FEMALE_VOICE_1_RUN_FROM_FIGHT_1, 2);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CASUAL_FEMALE_VOICE_1_DRIVER_ABUSE_1, 8);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CASUAL_FEMALE_VOICE_1_SHOCKED_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_CASUAL_FEMALE_VOICE_1_CHAT_1, 4);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetFemaleNo3TalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_3_VOICE_1_GUN_PANIC_1, 5);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_3_VOICE_1_CARJACKED_1, 3);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_3_VOICE_1_MUGGED_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_3_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_3_VOICE_1_RUN_FROM_FIGHT_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_3_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_3_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_FEMALE_3_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFatFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_FEMALE_VOICE_1_GUN_PANIC_1, 4);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_FEMALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_FEMALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_FEMALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_FEMALE_VOICE_1_SHOCKED_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_FAT_FEMALE_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteFatFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_FEMALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_FEMALE_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_FEMALE_VOICE_1_DRIVER_ABUSE_1, 8);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_FEMALE_VOICE_1_LOST_1, 2);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_FEMALE_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_FAT_FEMALE_VOICE_1_CHAT_1, 8);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFemaleProstituteTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROSTITUTE_VOICE_1_GUN_COOL_1, 4);
		break;
	case SOUND_PED_ROBBED:
		sfx = SFX_BLACK_PROSTITUTE_VOICE_1_MUGGED_1;
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROSTITUTE_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROSTITUTE_VOICE_1_DODGE_1, 3);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROSTITUTE_VOICE_1_DRIVER_ABUSE_1, 4);
		break;
	case SOUND_PED_SOLICIT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROSTITUTE_VOICE_1_SOLICIT_1, 8);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROSTITUTE_VOICE_1_CHAT_1, 4);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return (SFX_BLACK_PROSTITUTE_VOICE_2_CHAT_1 - SFX_BLACK_PROSTITUTE_VOICE_1_CHAT_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetWhiteFemaleProstituteTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_WHITE_PROSTITUTE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_WHITE_PROSTITUTE_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_WHITE_PROSTITUTE_VOICE_1_DODGE_1, 3);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_PROSTITUTE_VOICE_1_DRIVER_ABUSE_1, 4);
		break;
	case SOUND_PED_SOLICIT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_PROSTITUTE_VOICE_1_SOLICIT_1, 8);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_PROSTITUTE_VOICE_1_CHAT_1, 4);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return (SFX_WHITE_PROSTITUTE_VOICE_2_CHAT_1 - SFX_WHITE_PROSTITUTE_VOICE_1_CHAT_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetBlackProjectFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_OLD_VOICE_1_CARJACKED_1, 6);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_OLD_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_OLD_VOICE_1_DODGE_1, 10);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_OLD_VOICE_1_RUN_FROM_FIGHT_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_OLD_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_OLD_VOICE_1_SHOCKED_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_OLD_VOICE_1_CHAT_1, 10);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackProjectFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_YOUNG_VOICE_1_GUN_PANIC_1, 4);
		break;
	case SOUND_PED_CAR_JACKED:
		sfx = SFX_BLACK_PROJECT_FEMALE_YOUNG_VOICE_1_CARJACKED_1;
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_YOUNG_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_YOUNG_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_YOUNG_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_YOUNG_VOICE_1_SHOCKED_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_PROJECT_FEMALE_YOUNG_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownMaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_OLD_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_OLD_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_OLD_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_OLD_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_OLD_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_OLD_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_OLD_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownMaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_YOUNG_VOICE_1_GUN_PANIC_1, 2);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_YOUNG_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_YOUNG_VOICE_1_FIGHT_1, 6);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_YOUNG_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_YOUNG_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_YOUNG_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_MALE_YOUNG_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_OLD_FEMALE_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_OLD_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_OLD_FEMALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_OLD_FEMALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT_EVENT:
		sfx = SFX_CHINATOWN_OLD_FEMALE_VOICE_1_SHOCKED_1;
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_OLD_FEMALE_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_YOUNG_FEMALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_YOUNG_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_YOUNG_FEMALE_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_YOUNG_FEMALE_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_YOUNG_FEMALE_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_CHINATOWN_YOUNG_FEMALE_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetLittleItalyMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_MALE_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_MALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_MALE_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_MALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_MALE_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_MALE_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return (SFX_LITTLE_ITALY_MALE_VOICE_2_DRIVER_ABUSE_1 - SFX_LITTLE_ITALY_MALE_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetLittleItalyFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_OLD_FEMALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_OLD_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_OLD_FEMALE_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_OLD_FEMALE_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_OLD_FEMALE_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_OLD_FEMALE_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetLittleItalyFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_YOUNG_FEMALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_YOUNG_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_YOUNG_FEMALE_VOICE_1_DODGE_1, 7);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_YOUNG_FEMALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_YOUNG_FEMALE_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_LITTLE_ITALY_YOUNG_FEMALE_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteDockerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_DOCKER_MALE_VOICE_1_GUN_PANIC_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_WHITE_DOCKER_MALE_VOICE_1_FIGHT_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_WHITE_DOCKER_MALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_DOCKER_MALE_VOICE_1_DRIVER_ABUSE_1, 4);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_WHITE_DOCKER_MALE_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_DOCKER_MALE_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackDockerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_DOCKER_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_BLACK_DOCKER_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_DOCKER_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_DOCKER_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_BLACK_DOCKER_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_DOCKER_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetScumMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_SCUM_MALE_VOICE_1_GUN_PANIC_1, 5);
		break;
	case SOUND_PED_ROBBED:
		sfx = SFX_SCUM_MALE_VOICE_1_MUGGED_1;
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_SCUM_MALE_VOICE_1_FIGHT_1, 10);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_SCUM_MALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_SCUM_MALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(sfx, lastSfx, SFX_SCUM_MALE_VOICE_1_LOST_1, 3);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_SCUM_MALE_VOICE_1_EYING_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_SCUM_MALE_VOICE_1_CHAT_1, 9);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetScumFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_SCUM_FEMALE_VOICE_1_GUN_PANIC_1, 4);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_SCUM_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_SCUM_FEMALE_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_SCUM_FEMALE_VOICE_1_DODGE_1, 8);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_SCUM_FEMALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_SCUM_FEMALE_VOICE_1_CHAT_1, 13);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteWorkerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_WORKER_MALE_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_WHITE_WORKER_MALE_VOICE_1_FIGHT_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_WHITE_WORKER_MALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_WORKER_MALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_WHITE_WORKER_MALE_VOICE_1_EYING_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_WORKER_MALE_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackWorkerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_WORKER_MALE_VOICE_1_GUN_PANIC_1, 4);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_BLACK_WORKER_MALE_VOICE_1_FIGHT_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_WORKER_MALE_VOICE_1_DODGE_1, 3);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_WORKER_MALE_VOICE_1_DRIVER_ABUSE_1, 4);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_BLACK_WORKER_MALE_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_WORKER_MALE_VOICE_1_CHAT_1, 4);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBusinessMaleYoungTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_YOUNG_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_YOUNG_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_YOUNG_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_YOUNG_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_YOUNG_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_YOUNG_VOICE_1_RUN_FROM_FIGHT_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_YOUNG_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_YOUNG_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}

	if (model == MI_B_MAN3)
		sfx += (SFX_BUSINESS_MALE_YOUNG_VOICE_2_DRIVER_ABUSE_1 - SFX_BUSINESS_MALE_YOUNG_VOICE_1_DRIVER_ABUSE_1);
	return sfx;
}

uint32
cAudioManager::GetBusinessMaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_OLD_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_OLD_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_OLD_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_OLD_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_OLD_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_OLD_VOICE_1_MRUN_FROM_FIGHT_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_OLD_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BUSINESS_MALE_OLD_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteBusinessFemaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_BUSINESS_FEMALE_VOICE_1_GUN_PANIC_1, 4);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_WHITE_BUSINESS_FEMALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_WHITE_BUSINESS_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_WHITE_BUSINESS_FEMALE_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_WHITE_BUSINESS_FEMALE_VOICE_1_RUN_FROM_FIGHT_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_BUSINESS_FEMALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_BUSINESS_FEMALE_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_BUSINESS_FEMALE_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}

	if (model == MI_B_WOM2)
		sfx += (SFX_WHITE_BUSINESS_FEMALE_VOICE_2_DRIVER_ABUSE_1 - SFX_WHITE_BUSINESS_FEMALE_VOICE_1_DRIVER_ABUSE_1);
	return sfx;
}

uint32
cAudioManager::GetBlackBusinessFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_BUSINESS_FEMALE_VOICE_1_GUN_PANIC_1, 5);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_BUSINESS_FEMALE_VOICE_1_CARAJACKED_1, 4);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_BUSINESS_FEMALE_VOICE_1_MUGGED_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_BUSINESS_FEMALE_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_BLACK_BUSINESS_FEMALE_VOICE_1_RUN_FROM_FIGHT_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_BUSINESS_FEMALE_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_BUSINESS_FEMALE_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_BUSINESS_FEMALE_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetSupermodelMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_MODEL_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_MODEL_MALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_MODEL_MALE_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_MODEL_MALE_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_MODEL_MALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_MODEL_MALE_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_MODEL_MALE_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetSupermodelFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_MODEL_FEMALE_VOICE_1_GUN_PANIC_1, 4);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_MODEL_FEMALE_VOICE_1_MUGGED_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_MODEL_FEMALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_MODEL_FEMALE_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_MODEL_FEMALE_VOICE_1_SHOCKED_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_MODEL_FEMALE_VOICE_1_CHAT_1, 8);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStewardMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_MALE_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_MALE_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_MALE_VOICE_1_DODGE_1, 3);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_MALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_MALE_VOICE_1_CHAT_1, 4);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStewardFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_FEMALE_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_FEMALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_FEMALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_STEWARD_FEMALE_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return (SFX_STEWARD_FEMALE_VOICE_2_DRIVER_ABUSE_1 - SFX_STEWARD_FEMALE_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetFanMaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_MALE_VOICE_1_FIGHT_1, 3);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_MALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_MALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_MALE_VOICE_1_SHOCKED_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_MALE_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}

	if (model == MI_FAN_MAN2)
		sfx += (SFX_FOOTBALL_MALE_VOICE_2_DRIVER_ABUSE_1 - SFX_FOOTBALL_MALE_VOICE_1_DRIVER_ABUSE_1);
	return sfx;
}

uint32
cAudioManager::GetFanFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_ROBBED:
		sfx = SFX_FOOTBALL_FEMALE_VOICE_1_MUGGED_1;
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_FEMALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_FEMALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_FEMALE_VOICE_1_SHOCKED_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_FOOTBALL_FEMALE_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return (SFX_FOOTBALL_FEMALE_VOICE_2_DRIVER_ABUSE_1 - SFX_FOOTBALL_FEMALE_VOICE_1_DRIVER_ABUSE_1) * (m_sQueueSample.m_nEntityIndex % 2) + sfx;
}

uint32
cAudioManager::GetHospitalMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_HOSPITAL_MALE_VOICE_1_GUN_PANIC_1, 4);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_HOSPITAL_MALE_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_HOSPITAL_MALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_HOSPITAL_MALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_HOSPITAL_MALE_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetHospitalFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_HOSPITAL_FEMALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_HOSPITAL_FEMALE_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_HOSPITAL_FEMALE_VOICE_1_CHAT_1, 6);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteConstructionWorkerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_MALE_CONSTRUCTION_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_CAR_JACKED:
		sfx = SFX_WHITE_MALE_CONSTRUCTION_VOICE_1_CARJACKED_1;
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_WHITE_MALE_CONSTRUCTION_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_WHITE_MALE_CONSTRUCTION_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_WHITE_MALE_CONSTRUCTION_VOICE_1_DRIVER_ABUSE_1, 4);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_WHITE_MALE_CONSTRUCTION_VOICE_1_EYING_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_WHITE_MALE_CONSTRUCTION_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackConstructionWorkerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CONSTRUCTION_MALE_VOICE_1_GUN_PANIC_1, 3);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CONSTRUCTION_MALE_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CONSTRUCTION_MALE_VOICE_1_FIGHT_1, 5);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CONSTRUCTION_MALE_VOICE_1_DODGE_1, 5);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CONSTRUCTION_MALE_VOICE_1_DRIVER_ABUSE_1, 5);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CONSTRUCTION_MALE_VOICE_1_EYING_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_BLACK_CONSTRUCTION_MALE_VOICE_1_CHAT_1, 4);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetShopperFemaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(sfx, lastSfx, SFX_SHOPPER_VOICE_1_CARJACKED_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_SHOPPER_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_SHOPPER_VOICE_1_DODGE_1, 6);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_SHOPPER_VOICE_1_DRIVER_ABUSE_1, 7);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_SHOPPER_VOICE_1_SHOCKED_1, 4);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_SHOPPER_VOICE_1_CHAT_1, 7);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}

	if (model == MI_SHOPPER2) {
		sfx += (SFX_SHOPPER_VOICE_2_DRIVER_ABUSE_1 - SFX_SHOPPER_VOICE_1_DRIVER_ABUSE_1);
	} else if (model == MI_SHOPPER3) {
		sfx += (SFX_SHOPPER_VOICE_3_DRIVER_ABUSE_1 - SFX_SHOPPER_VOICE_1_DRIVER_ABUSE_1);
	}
	return sfx;
}

uint32
cAudioManager::GetStudentMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_MALE_VOICE_1_GUN_PANIC_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_MALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_MALE_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_MALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_MALE_VOICE_1_DRIVER_ABUSE_1, 4);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_MALE_VOICE_1_SHOCKED_1, 3);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_MALE_VOICE_1_CHAT_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStudentFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_FEMALE_VOICE_1_GUN_PANIC_1, 4);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_FEMALE_VOICE_1_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_FEMALE_VOICE_1_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_FEMALE_VOICE_1_DODGE_1, 4);
		break;
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_FEMALE_VOICE_1_DRIVER_ABUSE_1, 4);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_FEMALE_VOICE_1_SHOCKED_1, 2);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(sfx, lastSfx, SFX_STUDENT_FEMALE_VOICE_1_CHAT_1, 4);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetCasualMaleOldTalkSfx(int16 sound)
{
	return GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetSpecialCharacterTalkSfx(int32 modelIndex, int32 sound)
{
	char *modelName = CModelInfo::GetModelInfo(modelIndex)->GetModelName();
	if (!CGeneral::faststricmp(modelName, "eight") || !CGeneral::faststricmp(modelName, "eight2")) {
		return GetEightTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "frankie")) {
		return GetFrankieTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "misty")) {
		return GetMistyTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "ojg") || !CGeneral::faststricmp(modelName, "ojg_p")) {
		return GetOJGTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "cat")) {
		return GetCatatalinaTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "bomber")) {
		return GetBomberTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "s_guard")) {
		return GetSecurityGuardTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "chunky")) {
		return GetChunkyTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "asuka")) {
		return GetGenericFemaleTalkSfx(sound);
	}
	if (!CGeneral::faststricmp(modelName, "maria")) {
		return GetGenericFemaleTalkSfx(sound);
	}

	return GetGenericMaleTalkSfx(sound);
}
uint32
cAudioManager::GetEightTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_8BALL_GUN_COOL_1, 2);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_8BALL_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_8BALL_FIGHT_1, 6);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_8BALL_DODGE_1, 7);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetFrankieTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_SALVATORE_GUN_COOL_1, 4);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_SALVATORE_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_SALVATORE_FIGHT_1, 6);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_SALVATORE_DODGE_1, 3);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetMistyTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_MISTY_GUN_COOL_1, 5);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(sfx, lastSfx, SFX_MISTY_MUGGED_1, 2);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_MISTY_FIGHT_1, 4);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(sfx, lastSfx, SFX_MISTY_DODGE_1, 5);
		break;
	case SOUND_PED_TAXI_CALL:
		GetPhrase(sfx, lastSfx, SFX_MISTY_HERE_1, 4);
		break;
	default:
		return GetGenericFemaleTalkSfx(sound);
		break;
	}
	return sfx;
}

uint32
cAudioManager::GetOJGTalkSfx(int16 sound)
{
	return GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetCatatalinaTalkSfx(int16 sound)
{
	return GetGenericFemaleTalkSfx(sound);
}

uint32
cAudioManager::GetBomberTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound)
	{
	case SOUND_PED_BOMBER:
		GetPhrase(sfx, lastSfx, SFX_BOMBERMAN_1, 7);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetSecurityGuardTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(sfx, lastSfx, SFX_SECURITY_GUARD_VOICE_1_GUN_COOL_1, 2);
		break;
	case SOUND_PED_HANDS_COWER:
		sfx = SFX_SECURITY_GUARD_VOICE_1_GUN_PANIC_1;
		break;
	case SOUND_PED_CAR_JACKED:
	case SOUND_PED_ANNOYED_DRIVER:
		GetPhrase(sfx, lastSfx, SFX_SECURITY_GUARD_VOICE_1_DRIVER_ABUSE_1, 6);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(sfx, lastSfx, SFX_SECURITY_GUARD_VOICE_1_FIGHT_1, 2);
		break;
	case SOUND_PED_FLEE_RUN:
#ifdef FIX_BUGS
		sfx = SFX_SECURITY_GUARD_VOICE_1_RUN_FROM_FIGHT_1;
#else
		GetPhrase(sfx, lastSfx, SFX_SECURITY_GUARD_VOICE_1_DRIVER_ABUSE_1, 12);
#endif
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChunkyTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound)
	{
	case SOUND_PED_DEATH:
		return SFX_CHUNKY_DEATH;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(sfx, lastSfx, SFX_CHUNKY_RUN_1, 5);
		break;
	default:
		return GetGenericMaleTalkSfx(sound);
	}

	return sfx;
}

uint32
cAudioManager::GetGenericMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_DEATH:
		GetPhrase(sfx, lastSfx, SFX_GENERIC_MALE_DEATH_1, 8);
		break;
	case SOUND_PED_BULLET_HIT:
	case SOUND_PED_DEFEND:
		GetPhrase(sfx, lastSfx, SFX_GENERIC_MALE_GRUNT_1, 15);
		break;
	case SOUND_PED_BURNING:
		GetPhrase(sfx, lastSfx, SFX_GENERIC_MALE_FIRE_1, 8);
		break;
	case SOUND_PED_FLEE_SPRINT:
		GetPhrase(sfx, lastSfx, SFX_GENERIC_MALE_PANIC_1, 6);
		break;
	default:
		return NO_SAMPLE;
	}
	return sfx;
}

uint32
cAudioManager::GetGenericFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch (sound) {
	case SOUND_PED_DEATH:
		GetPhrase(sfx, lastSfx, SFX_GENERIC_FEMALE_DEATH_1, 10);
		break;
	case SOUND_PED_BULLET_HIT:
	case SOUND_PED_DEFEND:
		GetPhrase(sfx, lastSfx, SFX_GENERIC_FEMALE_GRUNT_1, 11);
		break;
	case SOUND_PED_BURNING:
		GetPhrase(sfx, lastSfx, SFX_GENERIC_FEMALE_FIRE_1, 9);
		break;
	case SOUND_PED_FLEE_SPRINT:
		GetPhrase(sfx, lastSfx, SFX_GENERIC_FEMALE_PANIC_1, 8);
		break;
	default:
		return NO_SAMPLE;
	}
	return sfx;
}

void
cPedComments::Add(tPedComment *com)
{
	uint8 index;

	if (m_nCommentsInBank[m_nActiveBank] >= NUM_PED_COMMENTS_SLOTS) {
		index = m_nIndexMap[m_nActiveBank][NUM_PED_COMMENTS_SLOTS - 1];
		if (m_asPedComments[m_nActiveBank][index].m_bVolume > com->m_bVolume)
			return;
	} else {
		index = m_nCommentsInBank[m_nActiveBank]++;
	}

	m_asPedComments[m_nActiveBank][index] = *com;

	uint32 i = 0;
	if (index != 0) {
		for (i = 0; i < index; i++) {
			if (m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][i]].m_bVolume < m_asPedComments[m_nActiveBank][index].m_bVolume) {
				break;
			}
		}

		if (i < index)
			memmove(&m_nIndexMap[m_nActiveBank][i + 1], &m_nIndexMap[m_nActiveBank][i], NUM_PED_COMMENTS_SLOTS - 1 - i);
	}

	m_nIndexMap[m_nActiveBank][i] = index;
}

void
cPedComments::Process()
{
	int sampleIndex;
	uint8 actualUsedBank;
	tPedComment *comment;

	if (AudioManager.m_nUserPause != 0) return;

	if (m_nCommentsInBank[m_nActiveBank]) {
		sampleIndex = m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_nSampleIndex;
		if (!SampleManager.IsPedCommentLoaded(sampleIndex))
			SampleManager.LoadPedComment(sampleIndex);

		AudioManager.m_sQueueSample.m_nEntityIndex = m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_nEntityIndex;
		AudioManager.m_sQueueSample.m_nCounter = 0;
		AudioManager.m_sQueueSample.m_nSampleIndex = sampleIndex;
		AudioManager.m_sQueueSample.m_nBankIndex = SFX_BANK_PED_COMMENTS;
		AudioManager.m_sQueueSample.m_nReleasingVolumeModificator = 3;
		AudioManager.m_sQueueSample.m_nVolume = m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_bVolume;
		AudioManager.m_sQueueSample.m_fDistance = m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_fDistance;
		AudioManager.m_sQueueSample.m_nLoopCount = 1;
		AudioManager.m_sQueueSample.m_nLoopStart = 0;
		AudioManager.m_sQueueSample.m_nLoopEnd = -1;
		AudioManager.m_sQueueSample.m_nEmittingVolume = MAX_VOLUME;
		AudioManager.m_sQueueSample.m_fSpeedMultiplier = 3.0f;
		switch (sampleIndex) {
		case SFX_POLICE_HELI_1:
		case SFX_POLICE_HELI_2:
		case SFX_POLICE_HELI_3:
			AudioManager.m_sQueueSample.m_fSoundIntensity = 400.0f;
			break;
		default:
			AudioManager.m_sQueueSample.m_fSoundIntensity = 50.0f;
			break;
		}
		AudioManager.m_sQueueSample.m_bReleasingSoundFlag = true;
		AudioManager.m_sQueueSample.m_vecPos = m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_vecPos;

		if (sampleIndex >= SFX_AMMU_D && sampleIndex <= SFX_AMMU_F) {
			AudioManager.m_sQueueSample.m_bReverbFlag = false;
			AudioManager.m_sQueueSample.m_bRequireReflection = false;
		} else {
			AudioManager.m_sQueueSample.m_bReverbFlag = true;
			AudioManager.m_sQueueSample.m_bRequireReflection = true;
		}

		AudioManager.m_sQueueSample.m_bIs2D = false;
		AudioManager.m_sQueueSample.m_nFrequency =
			SampleManager.GetSampleBaseFrequency(AudioManager.m_sQueueSample.m_nSampleIndex) + AudioManager.RandomDisplacement(750);
		if (CTimer::GetIsSlowMotionActive())
			AudioManager.m_sQueueSample.m_nFrequency /= 2;
		m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_nProcess = -1;
		AudioManager.AddSampleToRequestedQueue();
	}

	// Switch bank
	if (m_nActiveBank) {
		actualUsedBank = SFX_BANK_PED_COMMENTS;
		m_nActiveBank = SFX_BANK_0;
	} else {
		actualUsedBank = SFX_BANK_0;
		m_nActiveBank = SFX_BANK_PED_COMMENTS;
	}
	comment = m_asPedComments[actualUsedBank];
	for (uint32 i = 0; i < m_nCommentsInBank[actualUsedBank]; i++) {
		if (m_asPedComments[actualUsedBank][m_nIndexMap[actualUsedBank][i]].m_nProcess > 0) {
			--m_asPedComments[actualUsedBank][m_nIndexMap[actualUsedBank][i]].m_nProcess;
			Add(&comment[m_nIndexMap[actualUsedBank][i]]);
		}
	}

	for (uint32 i = 0; i < NUM_PED_COMMENTS_SLOTS; i++) {
		m_nIndexMap[actualUsedBank][i] = NUM_PED_COMMENTS_SLOTS;
	}
	m_nCommentsInBank[actualUsedBank] = 0;
}

#pragma endregion

#pragma endregion All the ped audio code

void
cAudioManager::ProcessExplosions(int32 explosion)
{
	uint8 type;
	CVector *pos;
	float distSquared;

	for (uint8 i = 0; i < ARRAY_SIZE(gaExplosion); i++) {
		if (CExplosion::GetExplosionActiveCounter(i) == 1) {
			CExplosion::ResetExplosionActiveCounter(i);
			type = CExplosion::GetExplosionType(i);
			switch (type) {
			case EXPLOSION_GRENADE:
			case EXPLOSION_ROCKET:
			case EXPLOSION_BARREL:
			case EXPLOSION_TANK_GRENADE:
				m_sQueueSample.m_fSoundIntensity = 400.0f;
				m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_2;
				m_sQueueSample.m_nFrequency = RandomDisplacement(2000) + 38000;
				m_sQueueSample.m_nReleasingVolumeModificator = 0;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				break;
			case EXPLOSION_MOLOTOV:
				m_sQueueSample.m_fSoundIntensity = 200.0f;
				m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_3;
				m_sQueueSample.m_nFrequency = RandomDisplacement(1000) + 19000;
				m_sQueueSample.m_nReleasingVolumeModificator = 0;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				break;
			case EXPLOSION_MINE:
			case EXPLOSION_HELI_BOMB:
				m_sQueueSample.m_fSoundIntensity = 300.0f;
				m_sQueueSample.m_nSampleIndex = SFX_ROCKET_LEFT;
				m_sQueueSample.m_nFrequency = RandomDisplacement(1000) + 12347;
				m_sQueueSample.m_nReleasingVolumeModificator = 0;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				break;
			default:
				m_sQueueSample.m_fSoundIntensity = 400.0f;
				m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_1;
				m_sQueueSample.m_nFrequency = RandomDisplacement(2000) + 38000;
				if (type == EXPLOSION_HELI)
					m_sQueueSample.m_nFrequency = 8 * m_sQueueSample.m_nFrequency / 10;
				m_sQueueSample.m_nReleasingVolumeModificator = 0;
				m_sQueueSample.m_nBankIndex = SFX_BANK_GENERIC_EXTRA;
				break;
			}
			pos = CExplosion::GetExplosionPosition(i);
			m_sQueueSample.m_vecPos = *pos;
			distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
			if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
				m_sQueueSample.m_fDistance = Sqrt(distSquared);
				m_sQueueSample.m_nVolume = ComputeVolume(MAX_VOLUME, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = i;
					m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nLoopCount = 1;
					m_sQueueSample.m_bReleasingSoundFlag = true;
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_nEmittingVolume = MAX_VOLUME;
					m_sQueueSample.m_nLoopStart = 0;
					m_sQueueSample.m_nLoopEnd = -1;
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_bRequireReflection = true;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::ProcessFires(int32)
{
	CEntity *entity;
	uint8 emittingVol;
	float distSquared;

	for (uint8 i = 0; i < NUM_FIRES; i++) {
		if (gFireManager.m_aFires[i].m_bIsOngoing && gFireManager.m_aFires[i].m_bAudioSet) {
			entity = gFireManager.m_aFires[i].m_pEntity;
			if (entity) {
				switch (entity->GetType()) {
				case ENTITY_TYPE_BUILDING:
					m_sQueueSample.m_fSoundIntensity = 50.0f;
					m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
					emittingVol = 100;
					m_sQueueSample.m_nFrequency = 8 * SampleManager.GetSampleBaseFrequency(SFX_CAR_ON_FIRE) / 10;
					m_sQueueSample.m_nFrequency += i * (m_sQueueSample.m_nFrequency / 64);
					m_sQueueSample.m_nReleasingVolumeModificator = 6;
					break;
				case ENTITY_TYPE_PED:
					m_sQueueSample.m_fSoundIntensity = 25.0f;
					m_sQueueSample.m_nSampleIndex = SFX_PED_ON_FIRE;
					m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_PED_ON_FIRE);
					emittingVol = 60;
					m_sQueueSample.m_nFrequency += i * (m_sQueueSample.m_nFrequency / 64);
					m_sQueueSample.m_nReleasingVolumeModificator = 10;
					break;
				default:
					m_sQueueSample.m_fSoundIntensity = 50.0f;
					m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
					m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CAR_ON_FIRE);
					m_sQueueSample.m_nFrequency += i * (m_sQueueSample.m_nFrequency / 64);
					emittingVol = 80;
					m_sQueueSample.m_nReleasingVolumeModificator = 8;
				}
			} else {
				m_sQueueSample.m_fSoundIntensity = 50.0f;
				m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CAR_ON_FIRE);
				m_sQueueSample.m_nFrequency += i * (m_sQueueSample.m_nFrequency / 64);
				emittingVol = 80;
				m_sQueueSample.m_nReleasingVolumeModificator = 8;
			}
			m_sQueueSample.m_vecPos = gFireManager.m_aFires[i].m_vecPos;
			distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
			if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
				m_sQueueSample.m_fDistance = Sqrt(distSquared);
				m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = i;
					m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					m_sQueueSample.m_nReleasingVolumeDivider = 10;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_bReleasingSoundFlag = false;
					m_sQueueSample.m_nEmittingVolume = emittingVol;
					m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_bRequireReflection = false;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::ProcessWaterCannon(int32)
{
	const float SOUND_INTENSITY = 30.0f;

	for (int32 i = 0; i < NUM_WATERCANNONS; i++) {
		if (CWaterCannons::aCannons[i].m_nId) {
			m_sQueueSample.m_vecPos = CWaterCannons::aCannons[0].m_avecPos[CWaterCannons::aCannons[i].m_nCur];
			float distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
			if (distSquared < SQR(SOUND_INTENSITY)) {
				m_sQueueSample.m_fDistance = Sqrt(distSquared);
#ifdef FIX_BUGS
				m_sQueueSample.m_nVolume = ComputeVolume(50, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
#else
				m_sQueueSample.m_nVolume = ComputeVolume(50, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
#endif
				if (m_sQueueSample.m_nVolume != 0) {
#ifdef FIX_BUGS
					m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
#else
					m_sQueueSample.m_fSoundIntensity = SQR(SOUND_INTENSITY);
#endif
					m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
					m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					m_sQueueSample.m_nFrequency = 15591;
					m_sQueueSample.m_nReleasingVolumeModificator = 5;
					m_sQueueSample.m_nCounter = i;
					m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					m_sQueueSample.m_nReleasingVolumeDivider = 8;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_bReleasingSoundFlag = false;
					m_sQueueSample.m_nEmittingVolume = 50;
					m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_bRequireReflection = false;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

#pragma region SCRIPT_OBJECTS
const int SCRIPT_OBJECT_INTENSITY_S = 30;
const int SCRIPT_OBJECT_INTENSITY_L = 80;

void
cAudioManager::ProcessScriptObject(int32 id)
{
	cAudioScriptObject *entity = (cAudioScriptObject *)m_asAudioEntities[id].m_pEntity;
	if (entity != nil) {
		m_sQueueSample.m_vecPos = entity->Posn;
		if (m_asAudioEntities[id].m_AudioEvents == 1)
			ProcessOneShotScriptObject(m_asAudioEntities[id].m_awAudioEvent[0]);
		else
			ProcessLoopingScriptObject(entity->AudioId);
	}
}

void
cAudioManager::ProcessOneShotScriptObject(uint8 sound)
{
	CPlayerPed *playerPed;
	uint8 emittingVolume;
	float distSquared;

	static uint8 iSound = 0;

	switch (sound) {
	case SCRIPT_SOUND_INJURED_PED_MALE_OUCH_S:
	case SCRIPT_SOUND_INJURED_PED_MALE_OUCH_L:
	{
		cPedParams pedParams;
		pedParams.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
		SetupPedComments(pedParams, SOUND_INJURED_PED_MALE_OUCH);
		return;
	}
	case SCRIPT_SOUND_INJURED_PED_FEMALE_OUCH_S:
	case SCRIPT_SOUND_INJURED_PED_FEMALE_OUCH_L:
	{
		cPedParams pedParams;
		pedParams.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
		SetupPedComments(pedParams, SOUND_INJURED_PED_FEMALE);
		return;
	}
	case SCRIPT_SOUND_GATE_START_CLUNK:
	case SCRIPT_SOUND_GATE_STOP_CLUNK:
		m_sQueueSample.m_fSoundIntensity = 40.0f;
		m_sQueueSample.m_nSampleIndex = SFX_COL_GATE;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		if (sound == SCRIPT_SOUND_GATE_START_CLUNK)
			m_sQueueSample.m_nFrequency = 10600;
		else
			m_sQueueSample.m_nFrequency = 9000;
		m_sQueueSample.m_nReleasingVolumeModificator = 1;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_bRequireReflection = true;
		emittingVolume = RandomDisplacement(10) + 50;
		break;
	case SCRIPT_SOUND_BULLET_HIT_GROUND_1:
	case SCRIPT_SOUND_BULLET_HIT_GROUND_2:
	case SCRIPT_SOUND_BULLET_HIT_GROUND_3:
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.m_nSampleIndex = m_anRandomTable[iSound % 5] % 3 + SFX_BULLET_WALL_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
		m_sQueueSample.m_nReleasingVolumeModificator = 9;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		emittingVolume = m_anRandomTable[2] % 20 + 90;
		break;
	case SCRIPT_SOUND_TRAIN_ANNOUNCEMENT_1:
	case SCRIPT_SOUND_TRAIN_ANNOUNCEMENT_2:
		if (!SampleManager.IsSampleBankLoaded(SFX_BANK_TRAIN))
			return;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = SFX_TRAIN_STATION_ANNOUNCE;
		m_sQueueSample.m_nBankIndex = SFX_BANK_TRAIN;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_TRAIN_STATION_ANNOUNCE);
		m_sQueueSample.m_nReleasingVolumeModificator = 0;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		m_sQueueSample.m_bIs2D = false;
		break;
	case SCRIPT_SOUND_PAYPHONE_RINGING:
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = SFX_PHONE_RING;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		emittingVolume = 80;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_PHONE_RING);
		m_sQueueSample.m_nReleasingVolumeModificator = 1;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_bRequireReflection = false;
		break;
	case SCRIPT_SOUND_GLASS_BREAK_L:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_GLASS_SMASH;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		emittingVolume = 70;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_GLASS_SMASH);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		break;
	case SCRIPT_SOUND_GLASS_BREAK_S:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_GLASS_SMASH;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		emittingVolume = 60;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_GLASS_SMASH);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		break;
	case SCRIPT_SOUND_GLASS_CRACK:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_GLASS_CRACK;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		emittingVolume = 70;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_GLASS_CRACK);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_bRequireReflection = true;
		break;
	case SCRIPT_SOUND_GLASS_LIGHT_BREAK:
		m_sQueueSample.m_fSoundIntensity = 55.0f;
		m_sQueueSample.m_nSampleIndex = (m_anRandomTable[4] & 3) + SFX_GLASS_SHARD_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_nFrequency = RandomDisplacement(2000) + 19000;
		m_sQueueSample.m_nReleasingVolumeModificator = 9;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		emittingVolume = RandomDisplacement(11) + 25;
		break;
	case SCRIPT_SOUND_BOX_DESTROYED_1:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_WOODEN_BOX_SMASH;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_nFrequency = RandomDisplacement(1500) + 18600;
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_bRequireReflection = true;
		emittingVolume = m_anRandomTable[2] % 20 + 80;
		break;
	case SCRIPT_SOUND_BOX_DESTROYED_2:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_CARDBOARD_BOX_SMASH;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_nFrequency = RandomDisplacement(1500) + 18600;
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_bRequireReflection = true;
		emittingVolume = m_anRandomTable[2] % 20 + 80;
		break;
	case SCRIPT_SOUND_METAL_COLLISION:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = m_anRandomTable[3] % 5 + SFX_COL_CAR_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_bRequireReflection = true;
		emittingVolume = m_anRandomTable[2] % 30 + 70;
		break;
	case SCRIPT_SOUND_TIRE_COLLISION:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_TYRE_BUMP;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		m_sQueueSample.m_bRequireReflection = true;
		emittingVolume = m_anRandomTable[2] % 30 + 60;
		break;
	case SCRIPT_SOUND_GUNSHELL_DROP:
		playerPed = FindPlayerPed();
		if (playerPed) {
			switch (playerPed->m_nSurfaceTouched) {
			case SURFACE_GRASS:
			case SURFACE_GRAVEL:
			case SURFACE_MUD_DRY:
			case SURFACE_TRANSPARENT_CLOTH:
			case SURFACE_PED:
			case SURFACE_SAND:
			case SURFACE_RUBBER:
			case SURFACE_HEDGE:
				m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_2;
				m_sQueueSample.m_nFrequency = RandomDisplacement(500) + 11000;
				m_sQueueSample.m_nReleasingVolumeModificator = 18;
				break;
			case SURFACE_WATER:
				return;
			default:
				m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_1;
				m_sQueueSample.m_nFrequency = RandomDisplacement(750) + 18000;
				m_sQueueSample.m_nReleasingVolumeModificator = 15;
				break;
			}
		} else {
			m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_1;
			m_sQueueSample.m_nFrequency = RandomDisplacement(750) + 18000;
			m_sQueueSample.m_nReleasingVolumeModificator = 15;
		}
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		emittingVolume = m_anRandomTable[2] % 20 + 30;
		break;
	case SCRIPT_SOUND_GUNSHELL_DROP_SOFT:
		m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_2;
		m_sQueueSample.m_nFrequency = RandomDisplacement(500) + 11000;
		m_sQueueSample.m_nReleasingVolumeModificator = 18;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		emittingVolume = m_anRandomTable[2] % 20 + 30;
		break;
	default:
		return;
	}

	distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		m_sQueueSample.m_nVolume = ComputeVolume(emittingVolume, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = iSound++;
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			m_sQueueSample.m_nEmittingVolume = emittingVolume;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_bReverbFlag = true;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessLoopingScriptObject(uint8 sound)
{
	uint8 emittingVolume;
	float distSquared;

	switch (sound) {
	case SCRIPT_SOUND_PARTY_1_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_1;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_1_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_1;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_2_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_2;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_2;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_2);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_2_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_2;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_2;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_2);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_3_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_3;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_3;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_3);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_3_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_3;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_3;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_3);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_4_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_4;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_4;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_4);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_4_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_4;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_4;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_4);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_5_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_5;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_5;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_5);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_5_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_5;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_5;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_5);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_6_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_6;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_6;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_6);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_6_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_6;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_6;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_6);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_7_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_7;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_7;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_7);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_7_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_7;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_7;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_7);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_8_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_8;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_8;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_8);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_8_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_8;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_8;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_8);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_9_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_9;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_9;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_9);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_9_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_9;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_9;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_9);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_10_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_10;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_10;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_10);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_10_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_10;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_10;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_10);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_11_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_11;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_11;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_11);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_11_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_11;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_11;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_11);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_12_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_12;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_12;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_12);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_12_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_12;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_12;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_12);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_13_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_RAGGA;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_RAGGA;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_RAGGA);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_13_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_RAGGA;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_RAGGA;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_RAGGA);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_STRIP_CLUB_LOOP_1_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_STRIP_CLUB_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_STRIP_CLUB_1;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_STRIP_CLUB_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_STRIP_CLUB_LOOP_1_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_STRIP_CLUB_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_STRIP_CLUB_1;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_STRIP_CLUB_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_STRIP_CLUB_LOOP_2_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_STRIP_CLUB_2;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_STRIP_CLUB_2;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_STRIP_CLUB_2);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_STRIP_CLUB_LOOP_2_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_STRIP_CLUB_2;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_STRIP_CLUB_2;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_STRIP_CLUB_2);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_WORK_SHOP_LOOP_S:
	case SCRIPT_SOUND_WORK_SHOP_LOOP_L:
		ProcessWorkShopScriptObject(sound);
		return;
	case SCRIPT_SOUND_SAWMILL_LOOP_S:
	case SCRIPT_SOUND_SAWMILL_LOOP_L:
		ProcessSawMillScriptObject(sound);
		return;
	case SCRIPT_SOUND_38:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_DOG_FOOD_FACTORY;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_DOG_FOOD_FACTORY;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_DOG_FOOD_FACTORY);
		m_sQueueSample.m_nReleasingVolumeModificator = 6;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_39:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_DOG_FOOD_FACTORY;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_DOG_FOOD_FACTORY;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_DOG_FOOD_FACTORY);
		m_sQueueSample.m_nReleasingVolumeModificator = 6;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_LAUNDERETTE_LOOP_S:
	case SCRIPT_SOUND_LAUNDERETTE_LOOP_L:
		ProcessLaunderetteScriptObject(sound);
		return;
	case SCRIPT_SOUND_CHINATOWN_RESTAURANT_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_RESTAURANT_CHINATOWN;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RESTAURANT_CHINATOWN;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RESTAURANT_CHINATOWN);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_CHINATOWN_RESTAURANT_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_RESTAURANT_CHINATOWN;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RESTAURANT_CHINATOWN;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RESTAURANT_CHINATOWN);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_CIPRIANI_RESAURANT_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_RESTAURANT_ITALY;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RESTAURANT_ITALY;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RESTAURANT_ITALY);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_CIPRIANI_RESAURANT_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_RESTAURANT_ITALY;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RESTAURANT_ITALY;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RESTAURANT_ITALY);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_46_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_RESTAURANT_GENERIC_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RESTAURANT_GENERIC_1;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RESTAURANT_GENERIC_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_47_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_RESTAURANT_GENERIC_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RESTAURANT_GENERIC_1;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RESTAURANT_GENERIC_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_MARCO_BISTRO_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_RESTAURANT_GENERIC_2;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RESTAURANT_GENERIC_2;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RESTAURANT_GENERIC_2);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_MARCO_BISTRO_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_RESTAURANT_GENERIC_2;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RESTAURANT_GENERIC_2;
		emittingVolume = 110;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RESTAURANT_GENERIC_2);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_AIRPORT_LOOP_S:
	case SCRIPT_SOUND_AIRPORT_LOOP_L:
		ProcessAirportScriptObject(sound);
		return;
	case SCRIPT_SOUND_SHOP_LOOP_S:
	case SCRIPT_SOUND_SHOP_LOOP_L:
		ProcessShopScriptObject(sound);
		return;
	case SCRIPT_SOUND_CINEMA_LOOP_S:
	case SCRIPT_SOUND_CINEMA_LOOP_L:
		ProcessCinemaScriptObject(sound);
		return;
	case SCRIPT_SOUND_DOCKS_LOOP_S:
	case SCRIPT_SOUND_DOCKS_LOOP_L:
		ProcessDocksScriptObject(sound);
		return;
	case SCRIPT_SOUND_HOME_LOOP_S:
	case SCRIPT_SOUND_HOME_LOOP_L:
		ProcessHomeScriptObject(sound);
		return;
	case SCRIPT_SOUND_FRANKIE_PIANO:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_PIANO_BAR_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_PIANO_BAR;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_PIANO_BAR_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_1_LOOP:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CLUB_1;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_1_S:
	case SCRIPT_SOUND_PORN_CINEMA_1_L:
	case SCRIPT_SOUND_PORN_CINEMA_2_S:
	case SCRIPT_SOUND_PORN_CINEMA_2_L:
	case SCRIPT_SOUND_PORN_CINEMA_3_S:
	case SCRIPT_SOUND_PORN_CINEMA_3_L:
	case SCRIPT_SOUND_MISTY_SEX_S:
	case SCRIPT_SOUND_MISTY_SEX_L:
		ProcessPornCinema(sound);
		return;
	case SCRIPT_SOUND_BANK_ALARM_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_BANK_ALARM_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_BANK_ALARM;
		emittingVolume = 90;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_BANK_ALARM_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 2;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_BANK_ALARM_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_BANK_ALARM_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_BANK_ALARM;
		emittingVolume = 90;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_BANK_ALARM_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 2;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_POLICE_BALL_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_POLICE_BALL_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_POLICE_BALL;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_POLICE_BALL_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 2;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_POLICE_BALL_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_POLICE_BALL_1;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_POLICE_BALL;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_POLICE_BALL_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 2;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_LOOP_INDUSTRIAL_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_RAVE_INDUSTRIAL;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RAVE_INDUSTRIAL;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RAVE_INDUSTRIAL);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_LOOP_INDUSTRIAL_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_RAVE_INDUSTRIAL;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RAVE_INDUSTRIAL;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RAVE_INDUSTRIAL);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_POLICE_CELL_BEATING_LOOP_S:
	case SCRIPT_SOUND_POLICE_CELL_BEATING_LOOP_L:
		ProcessPoliceCellBeatingScriptObject(sound);
		return;
	case SCRIPT_SOUND_RAVE_1_LOOP_S:
	case SCRIPT_SOUND_RAVE_2_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_RAVE_COMMERCIAL;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RAVE_COMMERCIAL;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_1_LOOP_L:
	case SCRIPT_SOUND_RAVE_2_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_RAVE_COMMERCIAL;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RAVE_COMMERCIAL;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_3_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_RAVE_SUBURBAN;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RAVE_SUBURBAN;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RAVE_SUBURBAN);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_RAVE_3_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_RAVE_SUBURBAN;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_RAVE_SUBURBAN;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RAVE_SUBURBAN);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PRETEND_FIRE_LOOP:
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		emittingVolume = 80;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CAR_ON_FIRE);
		m_sQueueSample.m_nReleasingVolumeModificator = 8;
		m_sQueueSample.m_nReleasingVolumeDivider = 10;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	default:
		return;
	}

	distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		m_sQueueSample.m_nVolume = ComputeVolume(emittingVolume, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_nEmittingVolume = emittingVolume;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessPornCinema(uint8 sound)
{

	eSfxSample sample;
	uint32 time;
	int32 rand;
	float distSquared;

	switch (sound) {
	case SCRIPT_SOUND_PORN_CINEMA_1_S:
	case SCRIPT_SOUND_MISTY_SEX_S:
		m_sQueueSample.m_nSampleIndex = SFX_PORN_1_LOOP;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_PORN_1;
		sample = SFX_PORN_1_GROAN_1;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_1_L:
	case SCRIPT_SOUND_MISTY_SEX_L:
		m_sQueueSample.m_nSampleIndex = SFX_PORN_1_LOOP;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_PORN_1;
		sample = SFX_PORN_1_GROAN_1;
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_2_S:
		m_sQueueSample.m_nSampleIndex = SFX_PORN_2_LOOP;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_PORN_2;
		sample = SFX_PORN_2_GROAN_1;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_2_L:
		m_sQueueSample.m_nSampleIndex = SFX_PORN_2_LOOP;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_PORN_2;
		sample = SFX_PORN_2_GROAN_1;
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_3_S:
		m_sQueueSample.m_nSampleIndex = SFX_PORN_3_LOOP;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_PORN_3;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		sample = SFX_PORN_3_GROAN_1;
		break;
	case SCRIPT_SOUND_PORN_CINEMA_3_L:
		m_sQueueSample.m_nSampleIndex = SFX_PORN_3_LOOP;
		m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_PORN_3;
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		sample = SFX_PORN_3_GROAN_1;
		break;
	default:
		return;
	}
	distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		if (sound != SCRIPT_SOUND_MISTY_SEX_S && sound != SCRIPT_SOUND_MISTY_SEX_L) {
			m_sQueueSample.m_nVolume = ComputeVolume(MAX_VOLUME, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nCounter = 0;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_bReleasingSoundFlag = false;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				m_sQueueSample.m_nEmittingVolume = MAX_VOLUME;
				m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
		}

		time = CTimer::GetTimeInMilliseconds();
		if (time > gPornNextTime) {
			m_sQueueSample.m_nVolume = ComputeVolume(90, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				rand = m_anRandomTable[1] & 1;
				m_sQueueSample.m_nSampleIndex = rand + sample;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
				m_sQueueSample.m_nCounter = rand + 1;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_nReleasingVolumeModificator = 6;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
				gPornNextTime = time + 2000 + m_anRandomTable[3] % 6000;
			}
		}
	}
}

void
cAudioManager::ProcessWorkShopScriptObject(uint8 sound)
{
	float distSquared;

	switch (sound) {
	case SCRIPT_SOUND_WORK_SHOP_LOOP_S:
	case SCRIPT_SOUND_WORK_SHOP_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		break;
	default:
		return;
	}
	distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		m_sQueueSample.m_nVolume = ComputeVolume(30, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nSampleIndex = SFX_WORKSHOP_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_WORKSHOP;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_WORKSHOP_1);
			m_sQueueSample.m_nCounter = 0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_nEmittingVolume = 30;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessSawMillScriptObject(uint8 sound)
{

	uint32 time;
	float distSquared;

	switch (sound) {
	case SCRIPT_SOUND_SAWMILL_LOOP_S:
	case SCRIPT_SOUND_SAWMILL_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		break;
	default:
		return;
	}
	distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		m_sQueueSample.m_nVolume = ComputeVolume(30, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nSampleIndex = SFX_SAWMILL_LOOP;
			m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_SAWMILL;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SAWMILL_LOOP);
			m_sQueueSample.m_nCounter = 0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_nEmittingVolume = 30;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
		time = CTimer::GetTimeInMilliseconds();
		if (time > gSawMillNextTime) {
			m_sQueueSample.m_nVolume = ComputeVolume(70, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nSampleIndex = SFX_SAWMILL_CUT_WOOD;
				m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_SAWMILL;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nCounter = 1;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
				gSawMillNextTime = time + 2000 + m_anRandomTable[3] % 4000;
			}
		}
	}
}

void
cAudioManager::ProcessLaunderetteScriptObject(uint8 sound)
{
	switch (sound) {
	case SCRIPT_SOUND_LAUNDERETTE_LOOP_S:
	case SCRIPT_SOUND_LAUNDERETTE_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		break;
	default:
		return;
	}
	float distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		m_sQueueSample.m_nVolume = ComputeVolume(45, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nSampleIndex = SFX_LAUNDERETTE_LOOP;
			m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_LAUNDERETTE;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_LAUNDERETTE_LOOP);
			m_sQueueSample.m_nCounter = 0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_nEmittingVolume = 45;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
		m_sQueueSample.m_nVolume = ComputeVolume(110, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nSampleIndex = SFX_LAUNDERETTE_SONG_LOOP;
			m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_LAUNDERETTE;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_LAUNDERETTE_SONG_LOOP);
			m_sQueueSample.m_nCounter = 1;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 3;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_nEmittingVolume = 110;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessShopScriptObject(uint8 sound)
{
	uint32 time;
	int32 rand;
	float distSquared;

	switch (sound) {
	case SCRIPT_SOUND_SHOP_LOOP_S:
	case SCRIPT_SOUND_SHOP_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		break;
	default:
		return;
	}
	distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
		m_sQueueSample.m_fDistance = Sqrt(distSquared);
		m_sQueueSample.m_nVolume = ComputeVolume(30, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nSampleIndex = SFX_SHOP_LOOP;
			m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_SHOP;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SHOP_LOOP);
			m_sQueueSample.m_nCounter = 0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 5;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_nEmittingVolume = 30;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
		time = CTimer::GetTimeInMilliseconds();
		if (time > gShopNextTime) {
			m_sQueueSample.m_nVolume = ComputeVolume(70, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				rand = m_anRandomTable[1] & 1;
				m_sQueueSample.m_nSampleIndex = rand + SFX_SHOP_TILL_1;
				m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_SHOP;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nCounter = rand + 1;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				m_sQueueSample.m_nEmittingVolume = 70;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
				gShopNextTime = time + 3000 + m_anRandomTable[3] % 7000;
			}
		}
	}
}

void
cAudioManager::ProcessAirportScriptObject(uint8 sound)
{
	static uint8 iSound = 0;

	uint32 time = CTimer::GetTimeInMilliseconds();
	if (time > gAirportNextTime) {
		switch (sound) {
		case SCRIPT_SOUND_AIRPORT_LOOP_S:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
			break;
		case SCRIPT_SOUND_AIRPORT_LOOP_L:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
			break;
		default:
			return;
		}
		float distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
		if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
			m_sQueueSample.m_fDistance = Sqrt(distSquared);
			m_sQueueSample.m_nVolume = ComputeVolume(110, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nSampleIndex = (m_anRandomTable[1] & 3) + SFX_AIRPORT_ANNOUNCEMENT_1;
				m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_AIRPORT;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nCounter = iSound++;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				m_sQueueSample.m_nEmittingVolume = 110;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
				gAirportNextTime = time + 10000 + m_anRandomTable[3] % 20000;
			}
		}
	}
}

void
cAudioManager::ProcessCinemaScriptObject(uint8 sound)
{
	uint8 rand;

	static uint8 iSound = 0;

	uint32 time = CTimer::GetTimeInMilliseconds();
	if (time > gCinemaNextTime) {
		switch (sound) {
		case SCRIPT_SOUND_CINEMA_LOOP_S:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
			break;
		case SCRIPT_SOUND_CINEMA_LOOP_L:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
			break;
		default:
			return;
		}
		float distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
		if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
			m_sQueueSample.m_fDistance = Sqrt(distSquared);
			rand = m_anRandomTable[0] % 90 + 30;
			m_sQueueSample.m_nVolume = ComputeVolume(rand, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nSampleIndex = iSound % 3 + SFX_CINEMA_BASS_1;
				m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CINEMA;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 4);
				m_sQueueSample.m_nCounter = iSound++;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				m_sQueueSample.m_nEmittingVolume = rand;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
				gCinemaNextTime = time + 1000 + m_anRandomTable[3] % 4000;
			}
		}
	}
}

void
cAudioManager::ProcessDocksScriptObject(uint8 sound)
{
	uint32 time;
	uint8 rand;
	float distSquared;

	static uint8 iSound = 0;

	time = CTimer::GetTimeInMilliseconds();
	if (time > gDocksNextTime) {
		switch (sound) {
		case SCRIPT_SOUND_DOCKS_LOOP_S:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
			break;
		case SCRIPT_SOUND_DOCKS_LOOP_L:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
			break;
		default:
			return;
		}
		distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
		if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
			m_sQueueSample.m_fDistance = Sqrt(distSquared);
			rand = m_anRandomTable[0] % 60 + 40;
			m_sQueueSample.m_nVolume = ComputeVolume(rand, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nSampleIndex = SFX_DOCKS_FOGHORN;
				m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_DOCKS;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_DOCKS_FOGHORN);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 8);
				m_sQueueSample.m_nCounter = iSound++;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				m_sQueueSample.m_nEmittingVolume = rand;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
				gDocksNextTime = time + 10000 + m_anRandomTable[3] % 40000;
			}
		}
	}
}
void
cAudioManager::ProcessHomeScriptObject(uint8 sound)
{
	uint32 time;
	uint8 rand;
	float dist;

	static uint8 iSound = 0;

	time = CTimer::GetTimeInMilliseconds();
	if (time > gHomeNextTime) {
		switch (sound) {
		case SCRIPT_SOUND_HOME_LOOP_S:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
			break;
		case SCRIPT_SOUND_HOME_LOOP_L:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
			break;
		default:
			return;
		}
		dist = GetDistanceSquared(m_sQueueSample.m_vecPos);
		if (dist < SQR(m_sQueueSample.m_fSoundIntensity)) {
			m_sQueueSample.m_fDistance = Sqrt(dist);
			rand = m_anRandomTable[0] % 30 + 40;
			m_sQueueSample.m_nVolume = ComputeVolume(rand, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nSampleIndex = m_anRandomTable[0] % 5 + SFX_HOME_1;
				m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_HOME;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
				m_sQueueSample.m_nCounter = iSound++;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_nEmittingVolume = rand;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = true;
				AddSampleToRequestedQueue();
				gHomeNextTime = time + 1000 + m_anRandomTable[3] % 4000;
			}
		}
	}
}
void
cAudioManager::ProcessPoliceCellBeatingScriptObject(uint8 sound)
{
	uint32 time = CTimer::GetTimeInMilliseconds();
	int32 sampleIndex;
	uint8 emittingVol;
	float distSquared;

	static uint8 iSound = 0;

	if (time > gCellNextTime) {
		switch (sound) {
		case SCRIPT_SOUND_POLICE_CELL_BEATING_LOOP_S:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
			break;
		case SCRIPT_SOUND_POLICE_CELL_BEATING_LOOP_L:
			m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
			break;
		default:
			return;
		}
		distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
		if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
			m_sQueueSample.m_fDistance = Sqrt(distSquared);
			if (m_FrameCounter & 1)
				sampleIndex = (m_anRandomTable[1] & 3) + SFX_FIGHT_1;
			else
				sampleIndex = (m_anRandomTable[3] & 1) + SFX_BAT_HIT_LEFT;
			m_sQueueSample.m_nSampleIndex = sampleIndex;
			emittingVol = m_anRandomTable[0] % 50 + 55;
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
				m_sQueueSample.m_nCounter = iSound++;
				m_sQueueSample.m_bIs2D = false;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bReleasingSoundFlag = true;
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_nEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
				cPedParams params;
				params.m_bDistanceCalculated = true;
				params.m_fDistance = distSquared;
				SetupPedComments(params, SOUND_INJURED_PED_MALE_PRISON);
			}
			gCellNextTime = time + 500 + m_anRandomTable[3] % 1500;
		}
	}
}
#pragma endregion All the code for script object audio on the map

void
cAudioManager::ProcessWeather(int32 id)
{
	uint8 vol;
	static uint8 iSound = 0;

	if (m_asAudioEntities[id].m_AudioEvents && m_asAudioEntities[id].m_awAudioEvent[0] == SOUND_LIGHTNING) {
		if (m_asAudioEntities[id].m_afVolume[0] >= 10.f) {
			m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_1;
			m_sQueueSample.m_nBankIndex = SFX_BANK_GENERIC_EXTRA;
			m_sQueueSample.m_nFrequency = RandomDisplacement(500) + 4000;
			vol = (m_asAudioEntities[id].m_afVolume[0] - 10.f) + 40;
		} else {
			m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_2;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_nFrequency = RandomDisplacement(500) + 4000;
			vol = (m_asAudioEntities[id].m_afVolume[0]) + 35;
		}
		m_sQueueSample.m_nVolume = vol;
		if (TheCamera.SoundDistUp < 20.f)
			m_sQueueSample.m_nVolume /= 2;
		if (iSound == 4)
			iSound = 0;
		m_sQueueSample.m_nCounter = iSound++;
		m_sQueueSample.m_nReleasingVolumeModificator = 0;
		m_sQueueSample.m_nOffset = (m_anRandomTable[2] & 15) + 55;
		m_sQueueSample.m_bIs2D = true;
		m_sQueueSample.m_nLoopCount = 1;
		m_sQueueSample.m_bReleasingSoundFlag = true;
		m_sQueueSample.m_nEmittingVolume = m_sQueueSample.m_nVolume;
		m_sQueueSample.m_nLoopStart = 0;
		m_sQueueSample.m_nLoopEnd = -1;
		m_sQueueSample.m_bReverbFlag = false;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
	}
	if (CWeather::Rain > 0.0f && (!CCullZones::CamNoRain() || !CCullZones::PlayerNoRain())) {
		m_sQueueSample.m_nSampleIndex = SFX_RAIN;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_RAIN);
		m_sQueueSample.m_nVolume = (int32)(25.f * CWeather::Rain);
		m_sQueueSample.m_nCounter = 4;
		m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		m_sQueueSample.m_nReleasingVolumeModificator = 0;
		m_sQueueSample.m_nOffset = 63;
		m_sQueueSample.m_bIs2D = true;
		m_sQueueSample.m_nLoopCount = 0;
		m_sQueueSample.m_bReleasingSoundFlag = false;
		m_sQueueSample.m_nReleasingVolumeDivider = 30;
		m_sQueueSample.m_bReverbFlag = false;
		m_sQueueSample.m_nEmittingVolume = m_sQueueSample.m_nVolume;
		m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
	}
}

void
cAudioManager::ProcessFrontEnd()
{
	bool stereo;
	bool processedPickup;
	bool processedMission;
	bool frontendBank;
	int16 sample;

	static uint8 iSound = 0;
	static uint32 cPickupNextFrame = 0;
	static uint32 cPartMisComNextFrame = 0;

	for (uint32 i = 0; i < m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_AudioEvents; i++) {
		processedPickup = false;
		stereo = false;
		processedMission = false;
		frontendBank = false;
		switch (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i]) {
		case SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM:
			m_sQueueSample.m_nSampleIndex = SFX_ERROR_FIRE_RIFLE;
			break;
		case SOUND_WEAPON_ROCKET_SHOT_NO_ZOOM:
			m_sQueueSample.m_nSampleIndex = SFX_ERROR_FIRE_ROCKET_LAUNCHER;
			break;
		case SOUND_GARAGE_NO_MONEY:
		case SOUND_GARAGE_BAD_VEHICLE:
		case SOUND_GARAGE_BOMB_ALREADY_SET:
			m_sQueueSample.m_nSampleIndex = SFX_PICKUP_ERROR_LEFT;
			stereo = true;
			break;
		case SOUND_GARAGE_OPENING:
		case SOUND_GARAGE_BOMB1_SET:
		case SOUND_GARAGE_BOMB2_SET:
		case SOUND_GARAGE_BOMB3_SET:
		case SOUND_41:
		case SOUND_GARAGE_VEHICLE_DECLINED:
		case SOUND_GARAGE_VEHICLE_ACCEPTED:
		case SOUND_PICKUP_HEALTH:
		case SOUND_4B:
		case SOUND_PICKUP_ADRENALINE:
		case SOUND_PICKUP_ARMOUR:
		case SOUND_EVIDENCE_PICKUP:
		case SOUND_UNLOAD_GOLD:
			m_sQueueSample.m_nSampleIndex = SFX_PICKUP_2_LEFT;
			processedPickup = true;
			stereo = true;
			break;
		case SOUND_PICKUP_WEAPON_BOUGHT:
		case SOUND_PICKUP_WEAPON:
			m_sQueueSample.m_nSampleIndex = SFX_PICKUP_1_LEFT;
			processedPickup = true;
			stereo = true;
			break;
		case SOUND_PICKUP_ERROR:
			m_sQueueSample.m_nSampleIndex = SFX_PICKUP_ERROR_LEFT;
			processedPickup = true;
			stereo = true;
			break;
		case SOUND_PICKUP_BONUS:
		case SOUND_PICKUP_MONEY:
		case SOUND_PICKUP_HIDDEN_PACKAGE:
		case SOUND_PICKUP_PACMAN_PILL:
		case SOUND_PICKUP_PACMAN_PACKAGE:
		case SOUND_PICKUP_FLOAT_PACKAGE:
			m_sQueueSample.m_nSampleIndex = SFX_PICKUP_3_LEFT;
			processedPickup = true;
			stereo = true;
			break;
		case SOUND_PAGER:
			// TODO: ps2 code
			m_sQueueSample.m_nSampleIndex = SFX_PAGER;
			break;
		case SOUND_RACE_START_3:
		case SOUND_RACE_START_2:
		case SOUND_RACE_START_1:
		case SOUND_CLOCK_TICK:
			m_sQueueSample.m_nSampleIndex = SFX_TIMER_BEEP;
			break;
		case SOUND_RACE_START_GO:
			m_sQueueSample.m_nSampleIndex = SFX_PART_MISSION_COMPLETE;
			break;
		case SOUND_PART_MISSION_COMPLETE:
			m_sQueueSample.m_nSampleIndex = SFX_PART_MISSION_COMPLETE;
			processedMission = true;
			break;
		case SOUND_FRONTEND_MENU_STARTING:
			m_sQueueSample.m_nSampleIndex = SFX_START_BUTTON_LEFT;
			stereo = true;
			break;
		case SOUND_FRONTEND_MENU_NEW_PAGE:
			m_sQueueSample.m_nSampleIndex = SFX_PAGE_CHANGE_AND_BACK_LEFT;
			stereo = true;
			frontendBank = true;
			break;
		case SOUND_FRONTEND_MENU_NAVIGATION:
			m_sQueueSample.m_nSampleIndex = SFX_HIGHLIGHT_LEFT;
			stereo = true;
			frontendBank = true;
			break;
		case SOUND_FRONTEND_MENU_SETTING_CHANGE:
			m_sQueueSample.m_nSampleIndex = SFX_SELECT_LEFT;
			stereo = true;
			frontendBank = true;
			break;
		case SOUND_FRONTEND_MENU_BACK:
			m_sQueueSample.m_nSampleIndex = SFX_SUB_MENU_BACK_LEFT;
			stereo = true;
			frontendBank = true;
			break;
		case SOUND_FRONTEND_STEREO:
			m_sQueueSample.m_nSampleIndex = SFX_STEREO_LEFT;
			stereo = true;
			frontendBank = true;
			break;
		case SOUND_FRONTEND_MONO:
			m_sQueueSample.m_nSampleIndex = SFX_MONO;
			frontendBank = true;
			break;
		case SOUND_FRONTEND_AUDIO_TEST:
			m_sQueueSample.m_nSampleIndex = m_anRandomTable[0] % 3 + SFX_NOISE_BURST_1;
			frontendBank = true;
			break;
		case SOUND_FRONTEND_FAIL:
			m_sQueueSample.m_nSampleIndex = SFX_ERROR_LEFT;
			frontendBank = true;
			stereo = true;
			break;
		case SOUND_FRONTEND_RADIO_TURN_OFF:
		case SOUND_FRONTEND_RADIO_CHANGE:
			m_sQueueSample.m_nSampleIndex = SFX_RADIO_CLICK;
			break;
		case SOUND_HUD:
			m_sQueueSample.m_nSampleIndex = SFX_INFO;
			break;
		default:
			continue;
		}

		if (processedPickup) {
			if (m_FrameCounter <= cPickupNextFrame)
				continue;
			cPickupNextFrame = m_FrameCounter + 5;
		} else if (processedMission) {
			if (m_FrameCounter <= cPartMisComNextFrame)
				continue;
			cPartMisComNextFrame = m_FrameCounter + 5;
		}

		sample = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
		if (sample == SFX_RAIN) {
			m_sQueueSample.m_nFrequency = 28509;
		} else if (sample == SFX_PICKUP_1_LEFT) {
			if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i] == 1.0f)
				m_sQueueSample.m_nFrequency = 32000;
			else
				m_sQueueSample.m_nFrequency = 48000;
		} else {
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		}
		m_sQueueSample.m_nVolume = 110;
		m_sQueueSample.m_nCounter = iSound++;
		m_sQueueSample.m_nLoopCount = 1;
		m_sQueueSample.m_bReleasingSoundFlag = true;
		m_sQueueSample.m_nBankIndex = frontendBank ? SFX_BANK_FRONT_END_MENU : SFX_BANK_0;
		m_sQueueSample.m_nReleasingVolumeModificator = 0;
		m_sQueueSample.m_bIs2D = true;
		m_sQueueSample.m_nEmittingVolume = m_sQueueSample.m_nVolume;
		m_sQueueSample.m_nLoopStart = 0;
		m_sQueueSample.m_nLoopEnd = -1;
		if (stereo)
			m_sQueueSample.m_nOffset = m_anRandomTable[0] & 31;
		else
			m_sQueueSample.m_nOffset = 63;
		m_sQueueSample.m_bReverbFlag = false;
		m_sQueueSample.m_bRequireReflection = false;
		AddSampleToRequestedQueue();
		if (stereo) {
			++m_sQueueSample.m_nSampleIndex;
			m_sQueueSample.m_nCounter = iSound++;
			m_sQueueSample.m_nOffset = MAX_VOLUME - m_sQueueSample.m_nOffset;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessCrane()
{
	CCrane *crane = (CCrane *)m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_pEntity;
	float distSquared;
	bool distCalculated = false;
	static const int intensity = 80;

	if (crane) {
		if (crane->m_nCraneStatus == CCrane::ACTIVATED) {
			if (crane->m_nCraneState != CCrane::IDLE) {
				m_sQueueSample.m_vecPos = crane->m_pCraneEntity->GetPosition();
				distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
				if (distSquared < SQR(intensity)) {
					CalculateDistance(distCalculated, distSquared);
					m_sQueueSample.m_nVolume = ComputeVolume(100, 80.f, m_sQueueSample.m_fDistance);
					if (m_sQueueSample.m_nVolume != 0) {
						m_sQueueSample.m_nCounter = 0;
						m_sQueueSample.m_nSampleIndex = SFX_CRANE_MAGNET;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_bIs2D = false;
						m_sQueueSample.m_nReleasingVolumeModificator = 2;
						m_sQueueSample.m_nFrequency = 6000;
						m_sQueueSample.m_nLoopCount = 0;
						m_sQueueSample.m_nEmittingVolume = 100;
						m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
						m_sQueueSample.m_fSpeedMultiplier = 4.0f;
						m_sQueueSample.m_fSoundIntensity = intensity;
						m_sQueueSample.m_bReleasingSoundFlag = false;
						m_sQueueSample.m_nReleasingVolumeDivider = 3;
						m_sQueueSample.m_bReverbFlag = true;
						m_sQueueSample.m_bRequireReflection = false;
						AddSampleToRequestedQueue();
					}
					if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_AudioEvents) {
						m_sQueueSample.m_nCounter = 1;
						m_sQueueSample.m_nSampleIndex = SFX_COL_CAR_2;
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_COL_CAR_2);
						m_sQueueSample.m_nLoopCount = 1;
						m_sQueueSample.m_bReleasingSoundFlag = true;
						m_sQueueSample.m_bReverbFlag = true;
						m_sQueueSample.m_bRequireReflection = true;
						AddSampleToRequestedQueue();
					}
				}
			}
		}
	}
}

void
cAudioManager::ProcessProjectiles()
{
	const int rocketLauncherIntensity = 90;
	const int molotovIntensity = 30;
	const int molotovVolume = 50;
	uint8 emittingVol;

	for (int32 i = 0; i < NUM_PROJECTILES; i++) {
		if (CProjectileInfo::GetProjectileInfo(i)->m_bInUse) {
			switch (CProjectileInfo::GetProjectileInfo(i)->m_eWeaponType) {
			case WEAPONTYPE_ROCKETLAUNCHER:
				emittingVol = MAX_VOLUME;
				m_sQueueSample.m_fSoundIntensity = rocketLauncherIntensity;
				m_sQueueSample.m_nSampleIndex = SFX_ROCKET_FLY;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_ROCKET_FLY);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				break;
			case WEAPONTYPE_MOLOTOV:
				emittingVol = molotovVolume;
				m_sQueueSample.m_fSoundIntensity = molotovIntensity;
				m_sQueueSample.m_nSampleIndex = SFX_PED_ON_FIRE;
				m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				m_sQueueSample.m_nFrequency = 32 * SampleManager.GetSampleBaseFrequency(SFX_PED_ON_FIRE) / 25;
				m_sQueueSample.m_nReleasingVolumeModificator = 7;
				break;
			default:
				return;
			}
			m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_vecPos = CProjectileInfo::ms_apProjectile[i]->GetPosition();
			float distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
			if (distSquared < SQR(m_sQueueSample.m_fSoundIntensity)) {
				m_sQueueSample.m_fDistance = Sqrt(distSquared);
				m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = i;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nLoopCount = 0;
					m_sQueueSample.m_nEmittingVolume = emittingVol;
					m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_bReleasingSoundFlag = false;
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_bRequireReflection = false;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

void
cAudioManager::ProcessGarages()
{
	const float SOUND_INTENSITY = 80.0f;

	CEntity *entity;
	uint8 state;
	uint32 sampleIndex;
	uint8 j;
	float distSquared;
	bool distCalculated;

	static uint8 iSound = 32;

	for (uint32 i = 0; i < CGarages::NumGarages; ++i) {
		if (CGarages::aGarages[i].m_eGarageType == GARAGE_NONE)
			continue;
		entity = CGarages::aGarages[i].m_pDoor1;
		if (entity == nil)
			continue;
		m_sQueueSample.m_vecPos = entity->GetPosition();
		distCalculated = false;
		distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
		if (distSquared < SQR(SOUND_INTENSITY)) {
			state = CGarages::aGarages[i].m_eGarageState;
			// while is here just to exit prematurely and avoid goto
			while (state == GS_OPENING || state == GS_CLOSING || state == GS_AFTERDROPOFF) {
				CalculateDistance(distCalculated, distSquared);
				m_sQueueSample.m_nVolume = ComputeVolume(90, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					if (CGarages::aGarages[i].m_eGarageType == GARAGE_CRUSHER) {
						if (CGarages::aGarages[i].m_eGarageState == GS_AFTERDROPOFF) {
							if (m_FrameCounter & 1) {
								if (m_anRandomTable[1] & 1)
									sampleIndex = m_anRandomTable[2] % 5 + SFX_COL_CAR_1;
								else
									sampleIndex = m_anRandomTable[2] % 6 + SFX_COL_CAR_PANEL_1;
								m_sQueueSample.m_nSampleIndex = sampleIndex;
								m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex) / 2;
								m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
								m_sQueueSample.m_nLoopCount = 1;
								m_sQueueSample.m_bReleasingSoundFlag = true;
								m_sQueueSample.m_nCounter = iSound++;
								if (iSound < 32)
									iSound = 32;
							} else break; // premature exit to go straight to the for loop
						} else {
							m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
							m_sQueueSample.m_nFrequency = 6543;

							m_sQueueSample.m_nCounter = i;
							m_sQueueSample.m_nLoopCount = 0;
							m_sQueueSample.m_nReleasingVolumeDivider = 3;
							m_sQueueSample.m_bReleasingSoundFlag = false;
						}
					} else {
						m_sQueueSample.m_nSampleIndex = SFX_GARAGE_DOOR_LOOP;
						m_sQueueSample.m_nFrequency = 13961;

						m_sQueueSample.m_nCounter = i;
						m_sQueueSample.m_nLoopCount = 0;
						m_sQueueSample.m_nReleasingVolumeDivider = 3;
						m_sQueueSample.m_bReleasingSoundFlag = false;
					}

					m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nReleasingVolumeModificator = 3;
					m_sQueueSample.m_nEmittingVolume = 90;
					m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
					m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
					m_sQueueSample.m_bReverbFlag = true;
					m_sQueueSample.m_bRequireReflection = false;
					AddSampleToRequestedQueue();
				}
				break;
			}
		}
		for (j = 0; j < m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_AudioEvents; ++j) {
			switch (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[j]) {
			case SOUND_GARAGE_DOOR_CLOSED:
			case SOUND_GARAGE_DOOR_OPENED:
				if (distSquared < SQR(SOUND_INTENSITY)) {
					CalculateDistance(distCalculated, distSquared);
					m_sQueueSample.m_nVolume = ComputeVolume(60, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
					if (m_sQueueSample.m_nVolume != 0) {
						if (CGarages::aGarages[i].m_eGarageType == GARAGE_CRUSHER) {
							m_sQueueSample.m_nSampleIndex = SFX_COL_CAR_PANEL_2;
							m_sQueueSample.m_nFrequency = 6735;
						} else if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[j] == SOUND_GARAGE_DOOR_OPENED) {
							m_sQueueSample.m_nSampleIndex = SFX_COL_CAR_PANEL_2;
							m_sQueueSample.m_nFrequency = 22000;
						} else {
							m_sQueueSample.m_nSampleIndex = SFX_COL_GARAGE_DOOR_1;
							m_sQueueSample.m_nFrequency = 18000;
						}
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_nReleasingVolumeModificator = 4;
						m_sQueueSample.m_nEmittingVolume = 60;
						m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
						m_sQueueSample.m_bReverbFlag = true; 
						m_sQueueSample.m_bIs2D = false;
						m_sQueueSample.m_bReleasingSoundFlag = true;
						m_sQueueSample.m_nLoopCount = 1;
						m_sQueueSample.m_nLoopStart = 0;
						m_sQueueSample.m_nLoopEnd = -1;
						m_sQueueSample.m_nCounter = iSound++;
						if (iSound < 32)
							iSound = 32;
						m_sQueueSample.m_bRequireReflection = true;
						AddSampleToRequestedQueue();
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void
cAudioManager::ProcessFireHydrant()
{
	float distSquared;
	bool distCalculated = false;
	static const int intensity = 35;

	m_sQueueSample.m_vecPos = ((CEntity *)m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_pEntity)->GetPosition();
	distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
	if (distSquared < SQR(intensity)) {
		CalculateDistance(distCalculated, distSquared);
		m_sQueueSample.m_nVolume = ComputeVolume(40, 35.f, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 0;
			m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 4;
			m_sQueueSample.m_nFrequency = 15591;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = 40;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = intensity;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_bReverbFlag = true;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
}

#pragma region BRIDGE
const int bridgeIntensity = 400;

void
cAudioManager::ProcessBridge()
{
	float dist;
	bool distCalculated = false;

	if (CBridge::pLiftRoad) {
		m_sQueueSample.m_vecPos = CBridge::pLiftRoad->GetPosition();
		dist = GetDistanceSquared(m_sQueueSample.m_vecPos);
		if (dist < SQR(450.0f)) {
			CalculateDistance(distCalculated, dist);
			switch (CBridge::State) {
			case STATE_BRIDGE_LOCKED:
			case STATE_LIFT_PART_IS_UP:
			case STATE_LIFT_PART_ABOUT_TO_MOVE_UP:
				ProcessBridgeWarning();
				break;
			case STATE_LIFT_PART_MOVING_DOWN:
			case STATE_LIFT_PART_MOVING_UP:
				ProcessBridgeWarning();
				ProcessBridgeMotor();
				break;
			default:
				break;
			}
			ProcessBridgeOneShots();
		}
	}
}

void
cAudioManager::ProcessBridgeWarning()
{
	if (CStats::CommercialPassed && m_sQueueSample.m_fDistance < 450.f) {
		m_sQueueSample.m_nVolume = ComputeVolume(100, 450.f, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 0;
			m_sQueueSample.m_nSampleIndex = SFX_BRIDGE_OPEN_WARNING;
			m_sQueueSample.m_nBankIndex = SFX_BANK_GENERIC_EXTRA;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_BRIDGE_OPEN_WARNING);
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = 100;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = 450.0f;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 8;
			m_sQueueSample.m_bReverbFlag = false;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessBridgeMotor()
{
	if (m_sQueueSample.m_fDistance < bridgeIntensity) {
		m_sQueueSample.m_nVolume = ComputeVolume(MAX_VOLUME, bridgeIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 1;
			m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE; // todo check sfx name
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_nFrequency = 5500;
			m_sQueueSample.m_nLoopCount = 0;
			m_sQueueSample.m_nEmittingVolume = MAX_VOLUME;
			m_sQueueSample.m_nLoopStart = SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopEnd = SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = bridgeIntensity;
			m_sQueueSample.m_bReleasingSoundFlag = false;
			m_sQueueSample.m_nReleasingVolumeDivider = 3;
			m_sQueueSample.m_bReverbFlag = false;
			AddSampleToRequestedQueue();
		}
	}
}

void
cAudioManager::ProcessBridgeOneShots()
{
	if (CBridge::State == STATE_LIFT_PART_IS_UP && CBridge::OldState == STATE_LIFT_PART_MOVING_UP)
		m_sQueueSample.m_nSampleIndex = SFX_COL_CONTAINER_1;
	else if (CBridge::State == STATE_LIFT_PART_IS_DOWN && CBridge::OldState == STATE_LIFT_PART_MOVING_DOWN)
		m_sQueueSample.m_nSampleIndex = SFX_COL_CONTAINER_1;
	else if (CBridge::State == STATE_LIFT_PART_MOVING_UP && CBridge::OldState == STATE_LIFT_PART_ABOUT_TO_MOVE_UP)
		m_sQueueSample.m_nSampleIndex = SFX_COL_CONTAINER_1;
	else if (CBridge::State == STATE_LIFT_PART_MOVING_DOWN && CBridge::OldState == STATE_LIFT_PART_IS_UP)
		m_sQueueSample.m_nSampleIndex = SFX_COL_CONTAINER_1;
	else return;

	if (m_sQueueSample.m_fDistance < bridgeIntensity) {
		m_sQueueSample.m_nVolume = ComputeVolume(MAX_VOLUME, bridgeIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 2;
			m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			m_sQueueSample.m_bIs2D = false;
			m_sQueueSample.m_nReleasingVolumeModificator = 1;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
			m_sQueueSample.m_nLoopCount = 1;
			m_sQueueSample.m_nEmittingVolume = MAX_VOLUME;
			m_sQueueSample.m_nLoopStart = 0;
			m_sQueueSample.m_nLoopEnd = -1;
			m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			m_sQueueSample.m_fSoundIntensity = bridgeIntensity;
			m_sQueueSample.m_bReleasingSoundFlag = true;
			m_sQueueSample.m_bReverbFlag = false;
			m_sQueueSample.m_bRequireReflection = false;
			AddSampleToRequestedQueue();
		}
	}
}
#pragma endregion

#pragma region MISSION_AUDIO
bool g_bMissionAudioLoadFailed;

struct MissionAudioData {
	const char *m_pName;
	int32 m_nId;
};

const MissionAudioData MissionAudioNameSfxAssoc[] = {
    {"lib_a1", STREAMED_SOUND_MISSION_LIB_A1}, {"lib_a2", STREAMED_SOUND_MISSION_LIB_A2},   {"lib_a", STREAMED_SOUND_MISSION_LIB_A},
    {"lib_b", STREAMED_SOUND_MISSION_LIB_B},   {"lib_c", STREAMED_SOUND_MISSION_LIB_C},     {"lib_d", STREAMED_SOUND_MISSION_LIB_D},
    {"l2_a", STREAMED_SOUND_MISSION_L2_A},     {"j4t_1", STREAMED_SOUND_MISSION_J4T_1},     {"j4t_2", STREAMED_SOUND_MISSION_J4T_2},
    {"j4t_3", STREAMED_SOUND_MISSION_J4T_3},   {"j4t_4", STREAMED_SOUND_MISSION_J4T_4},     {"j4_a", STREAMED_SOUND_MISSION_J4_A},
    {"j4_b", STREAMED_SOUND_MISSION_J4_B},     {"j4_c", STREAMED_SOUND_MISSION_J4_C},       {"j4_d", STREAMED_SOUND_MISSION_J4_D},
    {"j4_e", STREAMED_SOUND_MISSION_J4_E},     {"j4_f", STREAMED_SOUND_MISSION_J4_F},       {"j6_1", STREAMED_SOUND_MISSION_J6_1},
    {"j6_a", STREAMED_SOUND_MISSION_J6_A},     {"j6_b", STREAMED_SOUND_MISSION_J6_B},       {"j6_c", STREAMED_SOUND_MISSION_J6_C},
    {"j6_d", STREAMED_SOUND_MISSION_J6_D},     {"t4_a", STREAMED_SOUND_MISSION_T4_A},       {"s1_a", STREAMED_SOUND_MISSION_S1_A},
    {"s1_a1", STREAMED_SOUND_MISSION_S1_A1},   {"s1_b", STREAMED_SOUND_MISSION_S1_B},       {"s1_c", STREAMED_SOUND_MISSION_S1_C},
    {"s1_c1", STREAMED_SOUND_MISSION_S1_C1},   {"s1_d", STREAMED_SOUND_MISSION_S1_D},       {"s1_e", STREAMED_SOUND_MISSION_S1_E},
    {"s1_f", STREAMED_SOUND_MISSION_S1_F},     {"s1_g", STREAMED_SOUND_MISSION_S1_G},       {"s1_h", STREAMED_SOUND_MISSION_S1_H},
    {"s1_i", STREAMED_SOUND_MISSION_S1_I},     {"s1_j", STREAMED_SOUND_MISSION_S1_J},       {"s1_k", STREAMED_SOUND_MISSION_S1_K},
    {"s1_l", STREAMED_SOUND_MISSION_S1_L},     {"s3_a", STREAMED_SOUND_MISSION_S3_A},       {"s3_b", STREAMED_SOUND_MISSION_S3_B},
    {"el3_a", STREAMED_SOUND_MISSION_EL3_A},   {"mf1_a", STREAMED_SOUND_MISSION_MF1_A},     {"mf2_a", STREAMED_SOUND_MISSION_MF2_A},
    {"mf3_a", STREAMED_SOUND_MISSION_MF3_A},   {"mf3_b", STREAMED_SOUND_MISSION_MF3_B},     {"mf3_b1", STREAMED_SOUND_MISSION_MF3_B1},
    {"mf3_c", STREAMED_SOUND_MISSION_MF3_C},   {"mf4_a", STREAMED_SOUND_MISSION_MF4_A},     {"mf4_b", STREAMED_SOUND_MISSION_MF4_B},
    {"mf4_c", STREAMED_SOUND_MISSION_MF4_C},   {"a1_a", STREAMED_SOUND_MISSION_A1_A},       {"a3_a", STREAMED_SOUND_MISSION_A3_A},
    {"a5_a", STREAMED_SOUND_MISSION_A5_A},     {"a4_a", STREAMED_SOUND_MISSION_A4_A},       {"a4_b", STREAMED_SOUND_MISSION_A4_B},
    {"a4_c", STREAMED_SOUND_MISSION_A4_C},     {"a4_d", STREAMED_SOUND_MISSION_A4_D},       {"k1_a", STREAMED_SOUND_MISSION_K1_A},
    {"k3_a", STREAMED_SOUND_MISSION_K3_A},     {"r1_a", STREAMED_SOUND_MISSION_R1_A},       {"r2_a", STREAMED_SOUND_MISSION_R2_A},
    {"r2_b", STREAMED_SOUND_MISSION_R2_B},     {"r2_c", STREAMED_SOUND_MISSION_R2_C},       {"r2_d", STREAMED_SOUND_MISSION_R2_D},
    {"r2_e", STREAMED_SOUND_MISSION_R2_E},     {"r2_f", STREAMED_SOUND_MISSION_R2_F},       {"r2_g", STREAMED_SOUND_MISSION_R2_G},
    {"r2_h", STREAMED_SOUND_MISSION_R2_H},     {"r5_a", STREAMED_SOUND_MISSION_R5_A},       {"r6_a", STREAMED_SOUND_MISSION_R6_A},
    {"r6_a1", STREAMED_SOUND_MISSION_R6_A1},   {"r6_b", STREAMED_SOUND_MISSION_R6_B},       {"lo2_a", STREAMED_SOUND_MISSION_LO2_A},
    {"lo6_a", STREAMED_SOUND_MISSION_LO6_A},   {"yd2_a", STREAMED_SOUND_MISSION_YD2_A},     {"yd2_b", STREAMED_SOUND_MISSION_YD2_B},
    {"yd2_c", STREAMED_SOUND_MISSION_YD2_C},   {"yd2_c1", STREAMED_SOUND_MISSION_YD2_C1},   {"yd2_d", STREAMED_SOUND_MISSION_YD2_D},
    {"yd2_e", STREAMED_SOUND_MISSION_YD2_E},   {"yd2_f", STREAMED_SOUND_MISSION_YD2_F},     {"yd2_g", STREAMED_SOUND_MISSION_YD2_G},
    {"yd2_h", STREAMED_SOUND_MISSION_YD2_H},   {"yd2_ass", STREAMED_SOUND_MISSION_YD2_ASS}, {"yd2_ok", STREAMED_SOUND_MISSION_YD2_OK},
    {"h5_a", STREAMED_SOUND_MISSION_H5_A},     {"h5_b", STREAMED_SOUND_MISSION_H5_B},       {"h5_c", STREAMED_SOUND_MISSION_H5_C},
    {"ammu_a", STREAMED_SOUND_MISSION_AMMU_A}, {"ammu_b", STREAMED_SOUND_MISSION_AMMU_B},   {"ammu_c", STREAMED_SOUND_MISSION_AMMU_C},
    {"door_1", STREAMED_SOUND_MISSION_DOOR_1}, {"door_2", STREAMED_SOUND_MISSION_DOOR_2},   {"door_3", STREAMED_SOUND_MISSION_DOOR_3},
    {"door_4", STREAMED_SOUND_MISSION_DOOR_4}, {"door_5", STREAMED_SOUND_MISSION_DOOR_5},   {"door_6", STREAMED_SOUND_MISSION_DOOR_6},
    {"t3_a", STREAMED_SOUND_MISSION_T3_A},     {"t3_b", STREAMED_SOUND_MISSION_T3_B},       {"t3_c", STREAMED_SOUND_MISSION_T3_C},
    {"k1_b", STREAMED_SOUND_MISSION_K1_B},     {"c_1", STREAMED_SOUND_MISSION_CAT1},        {nil, 0}};

int32
FindMissionAudioSfx(const char *name)
{
	for (uint32 i = 0; MissionAudioNameSfxAssoc[i].m_pName != nil; ++i) {
		if (!CGeneral::faststricmp(MissionAudioNameSfxAssoc[i].m_pName, name))
			return MissionAudioNameSfxAssoc[i].m_nId;
	}
	debug("Can't find mission audio %s", name);
	return NO_SAMPLE;
}

bool
cAudioManager::MissionScriptAudioUsesPoliceChannel(int32 soundMission) const
{
	switch (soundMission) {
	case STREAMED_SOUND_MISSION_J6_D:
	case STREAMED_SOUND_MISSION_T4_A:
	case STREAMED_SOUND_MISSION_S1_H:
	case STREAMED_SOUND_MISSION_S3_B:
	case STREAMED_SOUND_MISSION_EL3_A:
	case STREAMED_SOUND_MISSION_A3_A:
	case STREAMED_SOUND_MISSION_A5_A:
	case STREAMED_SOUND_MISSION_K1_A:
	case STREAMED_SOUND_MISSION_R1_A:
	case STREAMED_SOUND_MISSION_R5_A:
	case STREAMED_SOUND_MISSION_LO2_A:
	case STREAMED_SOUND_MISSION_LO6_A:
		return true;
	default:
		return false;
	}
}

void
cAudioManager::PreloadMissionAudio(Const char *name)
{
	if (m_bIsInitialised) {
		int32 missionAudioSfx = FindMissionAudioSfx(name);
		if (missionAudioSfx != NO_SAMPLE) {
			m_sMissionAudio.m_nSampleIndex = missionAudioSfx;
			m_sMissionAudio.m_nLoadingStatus = LOADING_STATUS_NOT_LOADED;
			m_sMissionAudio.m_nPlayStatus = PLAY_STATUS_STOPPED;
			m_sMissionAudio.m_bIsPlaying = false;
			m_sMissionAudio.m_nMissionAudioCounter = m_nTimeSpent * SampleManager.GetStreamedFileLength(missionAudioSfx) / 1000;
			m_sMissionAudio.m_nMissionAudioCounter *= 4;
			m_sMissionAudio.m_bIsPlayed = false;
			m_sMissionAudio.m_bPredefinedProperties = true;
			g_bMissionAudioLoadFailed = false;
		}
	}
}

uint8
cAudioManager::GetMissionAudioLoadingStatus() const
{
	if (m_bIsInitialised)
		return m_sMissionAudio.m_nLoadingStatus;

	return LOADING_STATUS_LOADED;
}

void
cAudioManager::SetMissionAudioLocation(float x, float y, float z)
{
	if (m_bIsInitialised) {
		m_sMissionAudio.m_bPredefinedProperties = false;
		m_sMissionAudio.m_vecPos = CVector(x, y, z);
	}
}

void
cAudioManager::PlayLoadedMissionAudio()
{
	if (m_bIsInitialised && m_sMissionAudio.m_nSampleIndex != NO_SAMPLE && m_sMissionAudio.m_nLoadingStatus == LOADING_STATUS_LOADED &&
	    m_sMissionAudio.m_nPlayStatus == PLAY_STATUS_STOPPED)
		m_sMissionAudio.m_bIsPlayed = true;
}

bool
cAudioManager::IsMissionAudioSampleFinished()
{
	if (m_bIsInitialised)
		return m_sMissionAudio.m_nPlayStatus == PLAY_STATUS_FINISHED;

	static int32 cPretendFrame = 1;

	return (cPretendFrame++ & 63) == 0;
}

void
cAudioManager::ClearMissionAudio()
{
	if (m_bIsInitialised) {
		m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
		m_sMissionAudio.m_nLoadingStatus = LOADING_STATUS_NOT_LOADED;
		m_sMissionAudio.m_nPlayStatus = PLAY_STATUS_STOPPED;
		m_sMissionAudio.m_bIsPlaying = false;
		m_sMissionAudio.m_bIsPlayed = false;
		m_sMissionAudio.m_bPredefinedProperties = true;
		m_sMissionAudio.m_nMissionAudioCounter = 0;
	}
}

void
cAudioManager::ProcessMissionAudio()
{
	float dist;
	uint8 emittingVol;
	uint8 pan;
	float distSquared;
	CVector vec;

	static uint8 nCheckPlayingDelay = 0;
	static uint8 nFramesUntilFailedLoad = 0;
	static uint8 nFramesForPretendPlaying = 0;

	if (!m_bIsInitialised) return;
	if (m_sMissionAudio.m_nSampleIndex == NO_SAMPLE) return;

	switch (m_sMissionAudio.m_nLoadingStatus) {
	case LOADING_STATUS_NOT_LOADED:
		SampleManager.PreloadStreamedFile(m_sMissionAudio.m_nSampleIndex, 1);
		m_sMissionAudio.m_nLoadingStatus = LOADING_STATUS_LOADED;
		nFramesUntilFailedLoad = 0;
		break;
	case LOADING_STATUS_LOADED:
		if (!m_sMissionAudio.m_bIsPlayed)
			return;
		if (g_bMissionAudioLoadFailed) {
			if (m_bTimerJustReset) {
				ClearMissionAudio();
				SampleManager.StopStreamedFile(1);
				nFramesForPretendPlaying = 0;
				nCheckPlayingDelay = 0;
				nFramesUntilFailedLoad = 0;
			} else if (!m_nUserPause) {
				if (++nFramesForPretendPlaying < 120) {
					m_sMissionAudio.m_nPlayStatus = PLAY_STATUS_PLAYING;
				} else {
					m_sMissionAudio.m_nPlayStatus = PLAY_STATUS_FINISHED;
					m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
				}
			}
			break;
		}
		switch (m_sMissionAudio.m_nPlayStatus) {
		case PLAY_STATUS_STOPPED:
			if (MissionScriptAudioUsesPoliceChannel(m_sMissionAudio.m_nSampleIndex)) {
				SetMissionScriptPoliceAudio(m_sMissionAudio.m_nSampleIndex);
			} else {
				if (m_nUserPause)
					SampleManager.PauseStream(1, 1);
				if (m_sMissionAudio.m_bPredefinedProperties) {
					SampleManager.SetStreamedVolumeAndPan(80, 63, 1, 1);
				} else {
					distSquared = GetDistanceSquared(m_sMissionAudio.m_vecPos);
					if (distSquared >= SQR(50.0f)) {
						emittingVol = 0;
						pan = 63;
					} else {
						dist = Sqrt(distSquared);
						emittingVol = ComputeVolume(80, 50.0f, dist);
						TranslateEntity(&m_sMissionAudio.m_vecPos, &vec);
						pan = ComputePan(50.f, &vec);
					}
					SampleManager.SetStreamedVolumeAndPan(emittingVol, pan, 1, 1);
				}
				SampleManager.StartPreloadedStreamedFile(1);
			}
			m_sMissionAudio.m_nPlayStatus = PLAY_STATUS_PLAYING;
			nCheckPlayingDelay = 30;
			break;
		case PLAY_STATUS_PLAYING:
			if (m_bTimerJustReset) {
				ClearMissionAudio();
				SampleManager.StopStreamedFile(1);
				break;
			}
			if (MissionScriptAudioUsesPoliceChannel(m_sMissionAudio.m_nSampleIndex)) {
				if (!m_nUserPause) {
					if (nCheckPlayingDelay) {
						--nCheckPlayingDelay;
					} else if (GetMissionScriptPoliceAudioPlayingStatus() == PLAY_STATUS_FINISHED || m_sMissionAudio.m_nMissionAudioCounter-- == 0) {
						m_sMissionAudio.m_nPlayStatus = PLAY_STATUS_FINISHED;
						m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
						SampleManager.StopStreamedFile(1);
						m_sMissionAudio.m_nMissionAudioCounter = 0;
					}
				}
			} else if (m_sMissionAudio.m_bIsPlaying) {
				if (SampleManager.IsStreamPlaying(1) || m_nUserPause || m_nPreviousUserPause) {
					if (m_nUserPause)
						SampleManager.PauseStream(1, 1);
					else
						SampleManager.PauseStream(0, 1);
				} else {
					m_sMissionAudio.m_nPlayStatus = PLAY_STATUS_FINISHED;
					m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
					SampleManager.StopStreamedFile(1);
					m_sMissionAudio.m_nMissionAudioCounter = 0;
				}
			} else {
				if (m_nUserPause)
					break;
				if (nCheckPlayingDelay--) {
					if (!SampleManager.IsStreamPlaying(1))
						break;
					nCheckPlayingDelay = 0;
				}
				m_sMissionAudio.m_bIsPlaying = true;
			}
			break;
		default:
			break;
		}
		break;
	case LOADING_STATUS_FAILED:
		if (++nFramesUntilFailedLoad >= 90) {
			nFramesForPretendPlaying = 0;
			g_bMissionAudioLoadFailed = true;
			nFramesUntilFailedLoad = 0;
			m_sMissionAudio.m_nLoadingStatus = LOADING_STATUS_LOADED;
		}
		break;
	default:
		break;
	}
}
#pragma endregion All the mission audio stuff
