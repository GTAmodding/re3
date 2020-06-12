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

const int channels = ARRAY_SIZE(cAudioManager::m_asActiveSamples);
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

enum PLAY_STATUS : uint8 { PLAY_STATUS_STOPPED = 0, PLAY_STATUS_PLAYING, PLAY_STATUS_FINISHED };
enum LOADING_STATUS : uint8 { LOADING_STATUS_NOT_LOADED = 0, LOADING_STATUS_LOADED, LOADING_STATUS_FAILED };

void
cAudioManager::PreInitialiseGameSpecificSetup() const
{
	BankStartOffset[SAMPLEBANK_MAIN] = SAMPLEBANK_START;
#ifdef GTA_PS2
	BankStartOffset[SAMPLEBANK_CAR_PACARD] = SFX_CAR_ACCEL_1;
	BankStartOffset[SAMPLEBANK_CAR_PATHFINDER] = SFX_CAR_ACCEL_2;
	BankStartOffset[SAMPLEBANK_CAR_PORSCHE] = SFX_CAR_ACCEL_3;
	BankStartOffset[SAMPLEBANK_CAR_SPIDER] = SFX_CAR_ACCEL_4;
	BankStartOffset[SAMPLEBANK_CAR_MERC] = SFX_CAR_ACCEL_5;
	BankStartOffset[SAMPLEBANK_CAR_MACKTRUCK] = SFX_CAR_ACCEL_6;
	BankStartOffset[SAMPLEBANK_CAR_HOTROD] = SFX_CAR_ACCEL_7;
	BankStartOffset[SAMPLEBANK_CAR_COBRA] = SFX_CAR_ACCEL_8;
	BankStartOffset[SAMPLEBANK_CAR_NONE] = SFX_CAR_ACCEL_9;
	BankStartOffset[SAMPLEBANK_FRONTEND] = SFX_PAGE_CHANGE_AND_BACK_LEFT;
	BankStartOffset[SAMPLEBANK_TRAIN] = SFX_TRAIN_STATION_AMBIENCE_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_1] = SFX_CLUB_1;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_2] = SFX_CLUB_2;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_3] = SFX_CLUB_3;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_4] = SFX_CLUB_4;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_5] = SFX_CLUB_5;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_6] = SFX_CLUB_6;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_7] = SFX_CLUB_7;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_8] = SFX_CLUB_8;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_9] = SFX_CLUB_9;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_10] = SFX_CLUB_10;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_11] = SFX_CLUB_11;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_12] = SFX_CLUB_12;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_RAGGA] = SFX_CLUB_RAGGA;
	BankStartOffset[SAMPLEBANK_BUILDING_STRIP_CLUB_1] = SFX_STRIP_CLUB_1;
	BankStartOffset[SAMPLEBANK_BUILDING_STRIP_CLUB_2] = SFX_STRIP_CLUB_2;
	BankStartOffset[SAMPLEBANK_BUILDING_WORKSHOP] = SFX_WORKSHOP_1;
	BankStartOffset[SAMPLEBANK_BUILDING_PIANO_BAR] = SFX_PIANO_BAR_1;
	BankStartOffset[SAMPLEBANK_BUILDING_SAWMILL] = SFX_SAWMILL_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_DOG_FOOD_FACTORY] = SFX_DOG_FOOD_FACTORY;
	BankStartOffset[SAMPLEBANK_BUILDING_LAUNDERETTE] = SFX_LAUNDERETTE_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_RESTAURANT_CHINATOWN] = SFX_RESTAURANT_CHINATOWN;
	BankStartOffset[SAMPLEBANK_BUILDING_RESTAURANT_ITALY] = SFX_RESTAURANT_ITALY;
	BankStartOffset[SAMPLEBANK_BUILDING_RESTAURANT_GENERIC_1] = SFX_RESTAURANT_GENERIC_1;
	BankStartOffset[SAMPLEBANK_BUILDING_RESTAURANT_GENERIC_2] = SFX_RESTAURANT_GENERIC_2;
	BankStartOffset[SAMPLEBANK_BUILDING_AIRPORT] = SFX_AIRPORT_ANNOUNCEMENT_1;
	BankStartOffset[SAMPLEBANK_BUILDING_SHOP] = SFX_SHOP_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_CINEMA] = SFX_CINEMA_BASS_1;
	BankStartOffset[SAMPLEBANK_BUILDING_DOCKS] = SFX_DOCKS_FOGHORN;
	BankStartOffset[SAMPLEBANK_BUILDING_HOME] = SFX_HOME_1;
	BankStartOffset[SAMPLEBANK_BUILDING_PORN_1] = SFX_PORN_1_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_PORN_2] = SFX_PORN_2_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_PORN_3] = SFX_PORN_3_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_POLICE_BALL] = SFX_POLICE_BALL_1;
	BankStartOffset[SAMPLEBANK_BUILDING_BANK_ALARM] = SFX_BANK_ALARM_1;
	BankStartOffset[SAMPLEBANK_BUILDING_RAVE_INDUSTRIAL] = SFX_RAVE_INDUSTRIAL;
	BankStartOffset[SAMPLEBANK_BUILDING_RAVE_COMMERCIAL] = SFX_RAVE_COMMERCIAL;
	BankStartOffset[SAMPLEBANK_BUILDING_RAVE_SUBURBAN] = SFX_RAVE_SUBURBAN;
	BankStartOffset[SAMPLEBANK_BUILDING_RAVE_COMMERCIAL_2] = SFX_RAVE_COMMERCIAL_2;
	BankStartOffset[SAMPLEBANK_BUILDING_39] = SFX_CLUB_1_1;
	BankStartOffset[SAMPLEBANK_BUILDING_40] = SFX_CLUB_1_2;
	BankStartOffset[SAMPLEBANK_BUILDING_41] = SFX_CLUB_1_3;
	BankStartOffset[SAMPLEBANK_BUILDING_42] = SFX_CLUB_1_4;
	BankStartOffset[SAMPLEBANK_BUILDING_43] = SFX_CLUB_1_5;
	BankStartOffset[SAMPLEBANK_BUILDING_44] = SFX_CLUB_1_6;
	BankStartOffset[SAMPLEBANK_BUILDING_45] = SFX_CLUB_1_7;
	BankStartOffset[SAMPLEBANK_BUILDING_46] = SFX_CLUB_1_8;
	BankStartOffset[SAMPLEBANK_BUILDING_47] = SFX_CLUB_1_9;
	BankStartOffset[SAMPLEBANK_EXTRAS] = SFX_EXPLOSION_1;
#endif // GTA_PS2
	BankStartOffset[SAMPLEBANK_PED] = SAMPLEBANK_PED_START;
}

void
cAudioManager::PostInitialiseGameSpecificSetup()
{
	m_nFireAudioEntity = CreateEntity(AUDIOTYPE_FIRE, &gFireManager);
	if (m_nFireAudioEntity >= 0)
		SetEntityStatus(m_nFireAudioEntity, 1);

	m_nCollisionEntity = CreateEntity(AUDIOTYPE_COLLISION, (void *)1);
	if (m_nCollisionEntity >= 0)
		SetEntityStatus(m_nCollisionEntity, 1);

	m_nFrontEndEntity = CreateEntity(AUDIOTYPE_FRONTEND, (void *)1);
	if (m_nFrontEndEntity >= 0)
		SetEntityStatus(m_nFrontEndEntity, 1);

	m_nProjectileEntity = CreateEntity(AUDIOTYPE_PROJECTILE, (void *)1);
	if (m_nProjectileEntity >= 0)
		SetEntityStatus(m_nProjectileEntity, 1);

	m_nWaterCannonEntity = CreateEntity(AUDIOTYPE_WATERCANNON, (void *)1);
	if (m_nWaterCannonEntity >= 0)
		SetEntityStatus(m_nWaterCannonEntity, 1);

	m_nPoliceChannelEntity = CreateEntity(AUDIOTYPE_POLICERADIO, (void *)1);
	if (m_nPoliceChannelEntity >= 0)
		SetEntityStatus(m_nPoliceChannelEntity, 1);
#ifdef GTA_BRIDGE
	m_nBridgeEntity = CreateEntity(AUDIOTYPE_BRIDGE, (void*)1);
	if (m_nBridgeEntity >= 0)
		SetEntityStatus(m_nBridgeEntity, 1);
#endif // GTA_BRIDGE
	m_nEscalatorEntity = CreateEntity(AUDIOTYPE_ESCALATOR, (void*)1);
	if (m_nEscalatorEntity >= 0)
		SetEntityStatus(m_nEscalatorEntity, 1);

	m_nExtraSoundsEntity = CreateEntity(AUDIOTYPE_EXTRA_SOUNDS, (void*)1);
	if (m_nExtraSoundsEntity >= 0)
		SetEntityStatus(m_nExtraSoundsEntity, 1);


	m_sMissionAudio.m_nSampleIndex[0] = NO_SAMPLE;
	m_sMissionAudio.m_nLoadingStatus[0] = LOADING_STATUS_NOT_LOADED;
	m_sMissionAudio.m_nPlayStatus[0] = PLAY_STATUS_STOPPED;
	m_sMissionAudio.field_22[0] = 0;
	m_sMissionAudio.m_bIsPlayed[0] = false;
	m_sMissionAudio.m_bPredefinedProperties[0] = true;
	m_sMissionAudio.m_nMissionAudioCounter[0] = 0;
	m_sMissionAudio.m_bIsMobile[0] = false;
	field_5538 = 127;
	m_sMissionAudio.m_nSampleIndex[1] = NO_SAMPLE;
	m_sMissionAudio.m_nLoadingStatus[1] = LOADING_STATUS_NOT_LOADED;
	m_sMissionAudio.m_nPlayStatus[1] = PLAY_STATUS_STOPPED;
	m_sMissionAudio.field_22[1] = 0;
	m_sMissionAudio.m_bIsPlayed[1] = false;
	m_sMissionAudio.m_bPredefinedProperties[1] = true;
	m_sMissionAudio.m_nMissionAudioCounter[1] = 0;
	m_sMissionAudio.m_bIsMobile[1] = false;
	field_5538 = 127;

	ResetAudioLogicTimers(CTimer::GetTimeInMilliseconds());
}
void
cAudioManager::PreTerminateGameSpecificShutdown()
{
#ifdef GTA_BRIDGE
	if (m_nBridgeEntity >= 0) {
		DestroyEntity(m_nBridgeEntity);
		m_nBridgeEntity = AEHANDLE_NONE;
	}
#endif
	if (m_nEscalatorEntity >= 0) {
		DestroyEntity(m_nEscalatorEntity);
		m_nEscalatorEntity = AEHANDLE_NONE;
	}
	if (m_nExtraSoundsEntity >= 0) {
		DestroyEntity(m_nExtraSoundsEntity);
		m_nExtraSoundsEntity = AEHANDLE_NONE;
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
	ClearMissionAudio(0);
	ClearMissionAudio(1);
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
				SampleManager.SetChannelReverbFlag(i, 1);
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
			const PedState &state = playerPed->m_nPedState;
			if (state != PED_ENTER_CAR && state != PED_STEAL_CAR && !playerPed->bInVehicle)
				SampleManager.StopChannel(m_nActiveSamples);
		}
	}
}

void
cAudioManager::ProcessEntity(int32 id)
{
	if (m_asAudioEntities[id].m_nStatus) {
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
/*		case AUDIOTYPE_CRANE:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessCrane();
			}
			break;*/
		case AUDIOTYPE_SCRIPTOBJECT:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessScriptObject(id);
			}
			break;
#ifdef GTA_BRIDGE
		case AUDIOTYPE_BRIDGE:
			if (!m_nUserPause) {
				m_sQueueSample.m_bReverbFlag = true;
				ProcessBridge();
			}
			break;
#endif
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
	RIO,
	FIRETRUK,
	TRASH,
	STRETCH,
	MANANA,
	INFERNUS,
	VOODOO,
	PONY,
	MULE,
	CHEETAH,
	AMBULAN,
	FBICAR,
	MOONBEAM,
	ESPERANT,
	TAXI,
	WASHING,
	BOBCAT,
	MRWHOOP,
	BFINJECT,
	HUNTER,
	POLICE,
	ENFORCER,
	SECURICA,
	BANSHEE,
	PREDATOR,
	BUS,
	RHINO,
	BARRACKS,
	CUBAN,
	CHOPPER,
	ANGEL,
	COACH,
	CABBIE,
	STALLION,
	RUMPO,
	RCBANDIT,
	ROMERO,
	PACKER,
	SENTXS,
	ADMIRAL,
	SQUALO,
	SEASPAR,
	PIZZABOY,
	GANGBUR,
	AIRTRAIN,
	DEADDODO,
	SPEEDER,
	REEFER,
	TROPIC,
	FLATBED,
	YANKEE,
	CADDY,
	ZEBRA,
	TOPFUN,
	SKIMMER,
	PCJ600,
	FAGGIO,
	FREEWAY,
	RCBARON,
	RCRAIDER,
	GLENDALE,
	OCEANIC,
	SANCHEZ,
	SPARROW,
	PATRIOT,
	LOVEFIST,
	COASTG,
	DINGHY,
	HERMES,
	SABRE,
	SABRETUR,
	PHEONIX,
	WALTON,
	REGINA,
	COMET,
	DELUXO,
	BURRITO,
	SPAND,
	MARQUIS,
	BAGGAGE,
	KAUFMAN,
	MAVERICK,
	VCNMAV,
	RANCHER,
	FBIRANCH,
	VIRGO,
	GREENWOO,
	JETMAX,
	HOTRING,
	SANDKING,
	BLISTAC,
	POLMAV,
	BOXVILLE,
	BENSON,
	MESA,
	RCGOBLIN,
	HOTRINA,
	HOTRINB,
	BLOODRA,
	BLOODRB,
	VICECHEE,

	// HACK so this compiles
	// TODO(MIAMI): check it out
	DODO = -1
};

enum
{
	DOOR_TYPE_OLD = 0,
	DOOR_TYPE_NEW,
	DOOR_TYPE_TRUCK,
	DOOR_TYPE_AIRBREAK,
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

const tVehicleSampleData aVehicleSettings[110] = { {SFX_CAR_REV_2, 2, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9935, 1},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_56CHEV, 11487, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_8, 8, SFX_CAR_HORN_PORSCHE, 11025, SFX_CAR_HORN_JEEP, 10928, 1},
												 {SFX_CAR_REV_6, 6, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_HORN_JEEP, 9935, 2},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_56CHEV, 12893, SFX_CAR_HORN_JEEP, 8941, 0},
												 {SFX_CAR_REV_5, 5, SFX_CAR_HORN_BMW328, 10706, SFX_CAR_HORN_JEEP, 11922, 1},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_HORN_JEEP, 7948, 2},
												 {SFX_CAR_REV_6, 6, SFX_CAR_HORN_TRUCK, 29711, SFX_POLICE_SIREN_SLOW, 11556, 2},
												 {SFX_CAR_REV_6, 6, SFX_CAR_HORN_TRUCK, 31478, SFX_CAR_HORN_JEEP, 8941, 2},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_BMW328, 9538, SFX_CAR_HORN_JEEP, 12220, 1},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_3, 3, SFX_CAR_HORN_BMW328, 12017, SFX_CAR_HORN_JEEP, 9935, 1},
												 {SFX_CAR_REV_2, 2, SFX_CAR_HORN_JEEP, 22295, SFX_CAR_HORN_JEEP, 12200, 1},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_BUS2, 18000, SFX_CAR_HORN_JEEP, 13400, 1},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_BUS, 18286, SFX_CAR_HORN_JEEP, 9935, 2},
												 {SFX_CAR_REV_3, 3, SFX_CAR_HORN_PORSCHE, 11025, SFX_CAR_HORN_JEEP, 13600, 1},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_JEEP, 22295, SFX_AMBULANCE_SIREN_SLOW, 8795, 2},
												 {SFX_CAR_REV_5, 5, SFX_CAR_HORN_PORSCHE, 9271, SFX_POLICE_SIREN_SLOW, 16168, 1},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_56CHEV, 12170, SFX_CAR_HORN_JEEP, 8000, 1},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_BUS2, 12345, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_2, 2, SFX_CAR_HORN_BMW328, 10796, SFX_CAR_HORN_JEEP, 8543, 1},
												 {SFX_CAR_REV_5, 5, SFX_CAR_HORN_PORSCHE, 9271, SFX_CAR_HORN_JEEP, 9935, 1},
												 {SFX_CAR_REV_2, 2, SFX_CAR_HORN_PICKUP, 10924, SFX_CAR_HORN_JEEP, 9935, 1},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_PICKUP, 11025, SFX_ICE_CREAM_TUNE, 11025, 0},
												 {SFX_CAR_REV_7, 7, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9935, 1},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 10000, 0},
												 {SFX_CAR_REV_5, 5, SFX_CAR_HORN_BMW328, 10706, SFX_POLICE_SIREN_SLOW, 13596, 1},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_BUS, 17260, SFX_POLICE_SIREN_SLOW, 13000, 2},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_PICKUP, 8670, SFX_CAR_HORN_JEEP, 9935, 2},
												 {SFX_CAR_REV_8, 8, SFX_CAR_HORN_PORSCHE, 10400, SFX_CAR_HORN_JEEP, 10123, 1},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 26513, SFX_POLICE_SIREN_SLOW, 13596, 0},
												 {SFX_CAR_REV_6, 6, SFX_CAR_HORN_BUS2, 11652, SFX_CAR_HORN_JEEP, 10554, 3},
												 {SFX_CAR_REV_6, 6, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_HORN_JEEP, 8000, 2},
												 {SFX_CAR_REV_6, 6, SFX_CAR_HORN_TRUCK, 28043, SFX_CAR_HORN_JEEP, 9935, 2},
												 {SFX_CAR_REV_1, 0, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_HORN_JEEP, 9935, 3},
												 {SFX_CAR_REV_1, 0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CESNA_IDLE, 0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_6, 6, SFX_CAR_HORN_BUS, 16291, SFX_CAR_HORN_JEEP, 7500, 3},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_56CHEV, 10233, SFX_CAR_HORN_JEEP, 8935, 0},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_PICKUP, 8670, SFX_CAR_HORN_JEEP, 8935, 0},
												 {SFX_CAR_REV_1, 0, SFX_CAR_HORN_PICKUP, 2000, SFX_CAR_HORN_JEEP, 17000, 0},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_5, 5, SFX_CAR_HORN_BMW328, 9003, SFX_CAR_HORN_JEEP, 9935, 1},
												 {SFX_CAR_REV_2, 2, SFX_CAR_HORN_PORSCHE, 12375, SFX_CAR_HORN_JEEP, 9935, 1},
												 {SFX_CAR_REV_5, 5, SFX_CAR_HORN_BUS2, 15554, SFX_CAR_HORN_JEEP, 9935, 1},
												 {SFX_CAR_REV_7, 7, SFX_CAR_HORN_BUS2, 13857, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_7, 7, SFX_CAR_HORN_PICKUP, 10924, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 2},
												 {SFX_CAR_REV_1, 0, SFX_CAR_HORN_JEEP, 20143, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 0, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9000, 0},
												 {SFX_CAR_REV_6, 6, SFX_CAR_HORN_TRUCK, 28043, SFX_CAR_HORN_JEEP, 9935, 2},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_BUS, 18286, SFX_CAR_HORN_JEEP, 9935, 2},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_4, 4, SFX_CAR_HORN_BUS2, 18000, SFX_CAR_HORN_JEEP, 13400, 1},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0},
												 {SFX_CAR_REV_1, 1, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9935, 0} };


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
	params.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
	params.m_pVehicle = veh;
	params.m_pTransmission = nil;
	params.m_nIndex = 0;
	params.m_fVelocityChange = 0.0f;

	if (handling != nil)
		params.m_pTransmission = &handling->Transmission;

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
			ProcessModelCarEngine(&params);
			ProcessVehicleOneShots(&params);
			((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
			break;
		}
		if (params.m_nIndex == DODO) {
			if (!ProcessVehicleRoadNoise(&params)) {
				ProcessVehicleOneShots(&params);
				((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
				break;
			}
			if (CWeather::WetRoads > 0.f)
				ProcessWetRoadNoise(&params);
			ProcessVehicleSkidding(&params);
		} else {
			if (!ProcessVehicleRoadNoise(&params)) {
				ProcessVehicleOneShots(&params);
				((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
				break;
			}
			ProcessReverseGear(&params);
			if (CWeather::WetRoads > 0.f)
				ProcessWetRoadNoise(&params);
			ProcessVehicleSkidding(&params);
			ProcessVehicleHorn(&params);
			ProcessVehicleSirenOrAlarm(&params);
			if (UsesReverseWarning(params.m_nIndex))
				ProcessVehicleReverseWarning(&params);
			if (HasAirBrakes(params.m_nIndex))
				ProcessAirBrakes(&params);
		}
		ProcessCarBombTick(&params);
		ProcessVehicleEngine(&params);
		ProcessEngineDamage(&params);
		ProcessVehicleDoors(&params);

		ProcessVehicleOneShots(&params);
		((CAutomobile *)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange;
		break;
	case VEHICLE_TYPE_BOAT:
		ProcessBoatEngine(&params);
		ProcessBoatMovingOverWater(&params);
		ProcessVehicleOneShots(&params);
		break;
#ifdef GTA_TRAIN
	case VEHICLE_TYPE_TRAIN:
		ProcessTrainNoise(&params);
		ProcessVehicleOneShots(&params);
		break;
#endif
	case VEHICLE_TYPE_HELI:
		ProcessHelicopter(&params);
		ProcessVehicleOneShots(&params);
		break;
	case VEHICLE_TYPE_PLANE:
		ProcessPlane(&params);
		ProcessVehicleOneShots(&params);
		break;
	default:
		break;
	}
	ProcessRainOnVehicle(&params);
}

void
cAudioManager::ProcessRainOnVehicle(cVehicleParams *params)
{
	const int rainOnVehicleIntensity = 22;
	if (params->m_fDistance < SQR(rainOnVehicleIntensity) && CWeather::Rain > 0.01f && (!CCullZones::CamNoRain() || !CCullZones::PlayerNoRain())) {
		CVehicle *veh = params->m_pVehicle;
		++veh->m_bRainAudioCounter;
		if (veh->m_bRainAudioCounter >= 2) {
			veh->m_bRainAudioCounter = 0;
			CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
			float emittingVol = 30.f * CWeather::Rain;
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, rainOnVehicleIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = veh->m_bRainSamplesCounter++;
				if (veh->m_bRainSamplesCounter > 4)
					veh->m_bRainSamplesCounter = 68;
				m_sQueueSample.m_nSampleIndex = (m_anRandomTable[1] & 3) + SFX_CAR_RAIN_1;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessReverseGear(cVehicleParams *params)
{
	const int reverseGearIntensity = 30;

	CVehicle *veh;
	CAutomobile *automobile;
	int32 emittingVol;
	float modificator;

	if (params->m_fDistance >= SQR(reverseGearIntensity))
		return false;
	veh = params->m_pVehicle;
	if (veh->bEngineOn && (veh->m_fGasPedal < 0.0f || veh->m_nCurrentGear == 0)) {
		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		automobile = (CAutomobile *)params->m_pVehicle;
		if (automobile->m_nWheelsOnGround != 0) {
			modificator = params->m_fVelocityChange / params->m_pTransmission->fMaxReverseVelocity;
		} else {
			if (automobile->m_nDriveWheelsOnGround != 0)
				automobile->m_fGasPedalAudio *= 0.4f;
			modificator = automobile->m_fGasPedalAudio;
		}
		modificator = Abs(modificator);
		emittingVol = (24.f * modificator);
		m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, reverseGearIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			if (params->m_pVehicle->m_fGasPedal >= 0.0f) {
				m_sQueueSample.m_nCounter = 62;
				m_sQueueSample.m_nSampleIndex = SFX_REVERSE_GEAR_2;
			} else {
				m_sQueueSample.m_nCounter = 61;
				m_sQueueSample.m_nSampleIndex = SFX_REVERSE_GEAR;
			}
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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

// TODO(Miami): this becomes ProcessModelVehicle
void
cAudioManager::ProcessModelCarEngine(cVehicleParams *params)
{
/*	const float SOUND_INTENSITY = 30.0f;
	CAutomobile *automobile;
	float allowedVelocity;
	int32 emittingVol;
	float velocityChange;

	if (params->m_fDistance < SQR(SOUND_INTENSITY)) {
		automobile = (CAutomobile *)params->m_pVehicle;
		if (automobile->bEngineOn) {
			if (automobile->m_nWheelsOnGround == 0) {
				if (automobile->m_nDriveWheelsOnGround != 0)
					automobile->m_fGasPedalAudio *= 0.4f;
				velocityChange = automobile->m_fGasPedalAudio * params->m_pTransmission->fMaxVelocity;
			} else {
				velocityChange = Abs(params->m_fVelocityChange);
			}
			if (velocityChange > 0.001f) {
				allowedVelocity = 0.5f * params->m_pTransmission->fMaxVelocity;
				if (velocityChange < allowedVelocity)
					emittingVol = (90.f * velocityChange / allowedVelocity);
				else
					emittingVol = 90;
				if (emittingVol) {
					CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
					m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, 30.f, m_sQueueSample.m_fDistance);
					if (m_sQueueSample.m_nVolume != 0) {
						m_sQueueSample.m_nCounter = 2;
						m_sQueueSample.m_nSampleIndex = SFX_REMOTE_CONTROLLED_CAR;
						m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
						m_sQueueSample.m_bIs2D = false;
						m_sQueueSample.m_nReleasingVolumeModificator = 1;
						m_sQueueSample.m_nFrequency = (11025.f * velocityChange / params->m_pTransmission->fMaxVelocity + 11025.f);
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
	}*/
}



bool
cAudioManager::ProcessVehicleRoadNoise(cVehicleParams *params)
{
	const float SOUND_INTENSITY = 95.0f;

	int32 emittingVol;
	uint32 freq;
	float modificator;
	int sampleFreq;
	float velocity;

	if (params->m_fDistance >= SQR(SOUND_INTENSITY))
		return false;
	if (params->m_pTransmission != nil) {
		if (((CAutomobile*)params->m_pVehicle)->m_nDriveWheelsOnGround != 0) {
			velocity = Abs(params->m_fVelocityChange);
			if (velocity > 0.0f) {
				CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
				emittingVol = 30.f * Min(1.f, velocity / (0.5f * params->m_pTransmission->fMaxVelocity));
				m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = 0;
					m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nReleasingVolumeModificator = 3;
					if (params->m_pVehicle->m_nSurfaceTouched == SURFACE_WATER) {
						m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
						freq = 6050 * emittingVol / 30 + 16000;
					} else {
						m_sQueueSample.m_nSampleIndex = SFX_ROAD_NOISE;
						modificator = m_sQueueSample.m_fDistance / 190.f;
						sampleFreq = SampleManager.GetSampleBaseFrequency(SFX_ROAD_NOISE);
						freq = (sampleFreq * modificator) + ((3 * sampleFreq) / 4);
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
cAudioManager::ProcessWetRoadNoise(cVehicleParams *params)
{
	const float SOUND_INTENSITY = 30.0f;

	float relativeVelocity;
	int32 emittingVol;
	float modificator;
	int freq;
	float velChange;

	if (params->m_fDistance >= SQR(SOUND_INTENSITY))
		return false;
	if (params->m_pTransmission != nil) {
		if (((CAutomobile *)params->m_pVehicle)->m_nDriveWheelsOnGround != 0) {
			velChange = Abs(params->m_fVelocityChange);
			if (velChange > 0.f) {
				CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
				relativeVelocity = Min(1.0f, velChange / (0.5f * params->m_pTransmission->fMaxVelocity));
				emittingVol = 23.0f * relativeVelocity * CWeather::WetRoads;
				m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = 1;
					m_sQueueSample.m_nSampleIndex = SFX_ROAD_NOISE;
					m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nReleasingVolumeModificator = 3;
					modificator = m_sQueueSample.m_fDistance / 6.f;
					freq = SampleManager.GetSampleBaseFrequency(SFX_ROAD_NOISE);
					m_sQueueSample.m_nFrequency = freq + freq * modificator;
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
cAudioManager::ProcessVehicleEngine(cVehicleParams *params)
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

	if (params->m_fDistance < SQR(SOUND_INTENSITY)) {
		playerVeh = FindPlayerVehicle();
		veh = params->m_pVehicle;
		if (playerVeh == veh && veh->GetStatus() == STATUS_WRECKED) {
			SampleManager.StopChannel(m_nActiveSamples);
			return;
		}
		if (veh->bEngineOn) {
			CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
			automobile = (CAutomobile *)params->m_pVehicle;
			if (params->m_nIndex == DODO) {
				ProcessCesna(params);
				return;
			}
			if (FindPlayerVehicle() == veh) {
				ProcessPlayersVehicleEngine(params, automobile);
				return;
			}
			transmission = params->m_pTransmission;
			if (transmission != nil) {
				currentGear = params->m_pVehicle->m_nCurrentGear;
				if (automobile->m_nWheelsOnGround != 0) {
					if (automobile->bIsHandbrakeOn) {
						if (params->m_fVelocityChange == 0.0f)
							traction = 0.9f;
					} else if (params->m_pVehicle->GetStatus() == STATUS_SIMPLE) {
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
							if (automobile->m_aWheelState[VEHWHEEL_FRONT_LEFT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							if (automobile->m_aWheelState[VEHWHEEL_REAR_LEFT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							break;
						case 'R':
							if (automobile->m_aWheelState[VEHWHEEL_FRONT_RIGHT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							if (automobile->m_aWheelState[VEHWHEEL_REAR_RIGHT] == WHEEL_STATE_SPINNING)
								traction += 0.1f;
							break;
						}
					}
					if (transmission->fMaxVelocity <= 0.f) {
						relativeChange = 0.f;
					} else if (currentGear != 0) {
						relativeGearChange =
						    Min(1.0f, (params->m_fVelocityChange - transmission->Gears[currentGear].fShiftDownVelocity) / transmission->fMaxVelocity * 2.5f);
						if (traction == 0.0f && automobile->GetStatus() != STATUS_SIMPLE &&
						    params->m_fVelocityChange >= transmission->Gears[1].fShiftUpVelocity) {
							traction = 0.7f;
						}
						relativeChange = traction * automobile->m_fGasPedalAudio * 0.95f + (1.0f - traction) * relativeGearChange;
					} else
						relativeChange =
						    Min(1.0f, 1.0f - Abs((params->m_fVelocityChange - transmission->Gears[0].fShiftDownVelocity) / transmission->fMaxReverseVelocity));
				} else {
					if (automobile->m_nDriveWheelsOnGround != 0)
						automobile->m_fGasPedalAudio *= 0.4f;
					relativeChange = automobile->m_fGasPedalAudio;
				}
				modificator = relativeChange;
				if (currentGear || !automobile->m_nWheelsOnGround)
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
						m_sQueueSample.m_nSampleIndex = aVehicleSettings[params->m_nIndex].m_nBank - CAR_SAMPLEBANKS_OFFSET + SFX_CAR_IDLE_1;
						freq = modificator * 10000 + 22050;
						m_sQueueSample.m_nCounter = 52;
					} else {
						m_sQueueSample.m_nSampleIndex = aVehicleSettings[params->m_nIndex].m_nAccelerationSampleIndex;
						m_sQueueSample.m_nCounter = 2;
					}
				} else {
					if (automobile->m_fGasPedal < 0.05f) {
						m_sQueueSample.m_nSampleIndex = aVehicleSettings[params->m_nIndex].m_nBank - CAR_SAMPLEBANKS_OFFSET + SFX_CAR_IDLE_1;
						freq = modificator * 10000 + 22050;
						m_sQueueSample.m_nCounter = 52;
					} else {
						m_sQueueSample.m_nSampleIndex = aVehicleSettings[params->m_nIndex].m_nAccelerationSampleIndex;
						m_sQueueSample.m_nCounter = 2;
					}
				}
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessCesna(cVehicleParams *params)
{
	static uint8 nAccel = 0;

	//((CAutomobile *)params->m_pVehicle)->Damage.GetEngineStatus();

	if (FindPlayerVehicle() == params->m_pVehicle) {
		if (params->m_nIndex == DODO) {
			if (Pads[0].GetAccelerate() <= 0) {
				if (nAccel != 0)
					--nAccel;
			} else if (nAccel < 60) {
				++nAccel;
			}
			AddPlayerCarSample(85 * (60 - nAccel) / 60 + 20, 8500 * nAccel / 60 + 17000, SFX_CESNA_IDLE, SAMPLEBANK_MAIN, 52, true);
			AddPlayerCarSample(85 * nAccel / 60 + 20, 8500 * nAccel / 60 + 17000, SFX_CESNA_REV, SAMPLEBANK_MAIN, 2, true);
		}
	} else if (params->m_nIndex == DODO) {
		AddPlayerCarSample(105, 17000, SFX_CESNA_IDLE, SAMPLEBANK_MAIN, 52, true);
	} else if (params->m_fDistance < SQR(200)) {
		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(80, 200.f, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 52;
			m_sQueueSample.m_nSampleIndex = SFX_CESNA_IDLE;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		if (params->m_fDistance < SQR(90)) {
			m_sQueueSample.m_nVolume = ComputeVolume(80, 90.f, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 2;
				m_sQueueSample.m_nSampleIndex = SFX_CESNA_REV;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessPlayersVehicleEngine(cVehicleParams *params, CAutomobile *automobile)
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
	transmission = params->m_pTransmission;
	velocityChange = params->m_fVelocityChange;
	relativeVelocityChange = 2.0f * velocityChange / transmission->fMaxVelocity;

	accelerationMultipler = clamp(relativeVelocityChange, 0.0f, 1.0f);
	gasPedalAudio = accelerationMultipler;
	currentGear = params->m_pVehicle->m_nCurrentGear;

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
		if ((automobile->m_aWheelState[VEHWHEEL_FRONT_LEFT] != WHEEL_STATE_NORMAL || automobile->m_aWheelState[VEHWHEEL_REAR_LEFT] != WHEEL_STATE_NORMAL) &&
		    (automobile->m_aWheelState[VEHWHEEL_FRONT_RIGHT] != WHEEL_STATE_NORMAL || automobile->m_aWheelState[VEHWHEEL_REAR_RIGHT] != WHEEL_STATE_NORMAL))
			lostTraction = true;
		break;
	case 'R':
		if ((automobile->m_aWheelState[VEHWHEEL_FRONT_RIGHT] != WHEEL_STATE_NORMAL) || (automobile->m_aWheelState[VEHWHEEL_REAR_RIGHT] != WHEEL_STATE_NORMAL))
			lostTraction = true;
		break;
	}

	if (velocityChange != 0.0f) {
		time = params->m_pVehicle->m_vecMoveSpeed.z / velocityChange;
		if (time > 0.0f)
			freqModifier = -(Min(0.2f, time) * 3000.0f * 5.0f);
		else
			freqModifier = -(Max(-0.2f, time) * 3000.0f * 5.0f);
		if (params->m_fVelocityChange < -0.001f)
			freqModifier = -freqModifier;
	} else
		freqModifier = 0;

	engineSoundType = aVehicleSettings[params->m_nIndex].m_nBank;
	soundOffset = 3 * (engineSoundType - CAR_SAMPLEBANKS_OFFSET);
	if (accelerateState <= 0) {
		if (params->m_fVelocityChange < -0.001f) {
			if (channelUsed) {
				SampleManager.StopChannel(m_nActiveSamples);
				bAccelSampleStopped = true;
			}
			if (!automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn || lostTraction)
				gasPedalAudio = automobile->m_fGasPedalAudio;
			else
				gasPedalAudio = Min(1.0f, params->m_fVelocityChange / params->m_pTransmission->fMaxReverseVelocity);

			gasPedalAudio = Max(0.0f, gasPedalAudio);
			automobile->m_fGasPedalAudio = gasPedalAudio;
		} else if (LastAccel > 0) {
			if (channelUsed) {
				SampleManager.StopChannel(m_nActiveSamples);
				bAccelSampleStopped = true;
			}
			nCruising = 0;
			if (!automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn || lostTraction ||
			    params->m_fVelocityChange >= 0.01f && automobile->m_fGasPedalAudio > 0.2f) {
				automobile->m_fGasPedalAudio *= 0.6f;
				gasPedalAudio = automobile->m_fGasPedalAudio;
			}
			if (gasPedalAudio > 0.05f) {
				freq = (5000.f * (gasPedalAudio - 0.05f) * 20.f / 19) + 19000;
				if (engineSoundType == SAMPLEBANK_CAR_MACKTRUCK)
					freq /= 2;
				AddPlayerCarSample((25.f * (gasPedalAudio - 0.05f) * 20.f / 19) + 40, freq, (soundOffset + SFX_CAR_FINGER_OFF_ACCEL_1), engineSoundType, 63,
				                   false);
			}
		}
		freq = (10000.f * gasPedalAudio) + 22050;
		if (engineSoundType == SAMPLEBANK_CAR_MACKTRUCK)
			freq /= 2;
		AddPlayerCarSample(110 - (40.f * gasPedalAudio), freq, (engineSoundType - CAR_SAMPLEBANKS_OFFSET + SFX_CAR_IDLE_1), SAMPLEBANK_MAIN, 52, true);

		CurrentPretendGear = Max(1, currentGear);
	} else {
		while (nCruising == 0) {
			if (accelerateState < 150 || !automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn || lostTraction ||
			    currentGear < 2 && velocityChange - automobile->m_fVelocityChangeForAudio < 0.01f) { // here could be used abs
				if (!automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn || lostTraction) {
					if (!automobile->m_nWheelsOnGround && automobile->m_nDriveWheelsOnGround ||
					    (automobile->bIsHandbrakeOn && !bHandbrakeOnLastFrame || lostTraction && !bLostTractionLastFrame) && automobile->m_nWheelsOnGround) {
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
				if (engineSoundType == SAMPLEBANK_CAR_MACKTRUCK)
					freq /= 2;
				if (channelUsed) {
					SampleManager.StopChannel(m_nActiveSamples);
					bAccelSampleStopped = true;
				}
				AddPlayerCarSample(vol, freq, (engineSoundType - CAR_SAMPLEBANKS_OFFSET + SFX_CAR_REV_1), SAMPLEBANK_MAIN, 2, true);
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
						if (CurrentPretendGear < params->m_pTransmission->nNumberOfGears - 1)
							++CurrentPretendGear;
						else {
							nCruising = 1;
							break; // while was used just for this fucking place
						}
					}

					if (!SampleManager.InitialiseChannel(m_nActiveSamples, soundOffset + SFX_CAR_ACCEL_1, SAMPLEBANK_MAIN))
						return;
					SampleManager.SetChannelLoopCount(m_nActiveSamples, 1);
					SampleManager.SetChannelLoopPoints(m_nActiveSamples, 0, -1);
				}

				SampleManager.SetChannelEmittingVolume(m_nActiveSamples, 85);
				SampleManager.SetChannel3DPosition(m_nActiveSamples, pos.x, pos.y, pos.z);
				SampleManager.SetChannel3DDistances(m_nActiveSamples, 50.f, 12.5f);
				freq = GearFreqAdj[CurrentPretendGear] + freqModifier + 22050;
				if (engineSoundType == SAMPLEBANK_CAR_MACKTRUCK)
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
			if (accelerateState < 150 || !automobile->m_nWheelsOnGround || automobile->bIsHandbrakeOn || lostTraction ||
			    currentGear < params->m_pTransmission->nNumberOfGears - 1) {
				nCruising = 0;
			} else {
				if (accelerateState >= 220 && 0.001f + params->m_fVelocityChange < automobile->m_fVelocityChangeForAudio) {
					if (nCruising < 800)
						++nCruising;
				} else if (nCruising > 3) {
					--nCruising;
				}
				freq = 27 * nCruising + freqModifier + 22050;
				if (engineSoundType == SAMPLEBANK_CAR_MACKTRUCK)
					freq /= 2;
				AddPlayerCarSample(85, freq, (soundOffset + SFX_CAR_AFTER_ACCEL_1), engineSoundType, 64, true);
			}
		}
	}
	LastAccel = accelerateState;

	bHandbrakeOnLastFrame = !!automobile->bIsHandbrakeOn;
	bLostTractionLastFrame = lostTraction;
}

void
cAudioManager::ProcessVehicleSkidding(cVehicleParams *params)
{
	const float SOUND_INTENSITY = 40.0f;

	CAutomobile *automobile;
	cTransmission *transmission;
	int32 emittingVol;
	float newSkidVal = 0.0f;
	float skidVal = 0.0f;

	if (params->m_fDistance >= SQR(SOUND_INTENSITY))
		return;
	automobile = (CAutomobile *)params->m_pVehicle;
	if (!automobile->m_nWheelsOnGround)
		return;
	CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
	for (int32 i = 0; i < ARRAY_SIZE(automobile->m_aWheelState); i++) {
		if (automobile->m_aWheelState[i] == WHEEL_STATE_NORMAL || automobile->Damage.GetWheelStatus(i) == WHEEL_STATUS_MISSING)
			continue;
		transmission = params->m_pTransmission;
		switch (transmission->nDriveType) {
		case '4':
			newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission, params->m_fVelocityChange);
			break;
		case 'F':
			if (i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)
				newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission, params->m_fVelocityChange);
			else
				newSkidVal = GetVehicleNonDriveWheelSkidValue(i, automobile, transmission, params->m_fVelocityChange);
			break;
		case 'R':
			if (i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT)
				newSkidVal = GetVehicleDriveWheelSkidValue(i, automobile, transmission, params->m_fVelocityChange);
			else
				newSkidVal = GetVehicleNonDriveWheelSkidValue(i, automobile, transmission, params->m_fVelocityChange);
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
			switch (params->m_pVehicle->m_nSurfaceTouched) {
			case SURFACE_GRASS:
			case SURFACE_HEDGE:
				m_sQueueSample.m_nSampleIndex = SFX_RAIN;
				emittingVol /= 4;
				m_sQueueSample.m_nFrequency = 13000.f * skidVal + 35000.f;
				m_sQueueSample.m_nVolume /= 4;
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

			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
}

float
cAudioManager::GetVehicleDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile, cTransmission *transmission, float velocityChange)
{
	tWheelState wheelState;
	float relativeVelChange;
	float gasPedalAudio = automobile->m_fGasPedalAudio;
	float modificator;
	float velChange;
	float relativeVel;

	wheelState = automobile->m_aWheelState[wheel];
	if (wheelState == WHEEL_STATE_SPINNING && gasPedalAudio > 0.4f) {
		relativeVelChange = (gasPedalAudio - 0.4f) * 1.25f;

	} else if (wheelState == WHEEL_STATE_SKIDDING) {
		relativeVelChange = Min(1.0f, Abs(velocityChange) / transmission->fMaxVelocity);
	} else if (wheelState == WHEEL_STATE_FIXED) {
		modificator = 0.4f;
		relativeVelChange = gasPedalAudio;
		if (relativeVelChange > 0.4f) {
			relativeVelChange = relativeVelChange - 0.4f;
			modificator = 5.f / 3.f;
		}
		velChange = Abs(velocityChange);
		if (relativeVelChange > 0.4f)
			relativeVelChange = relativeVelChange * modificator;
		if (velChange > 0.04f) {
			relativeVel = Min(1.0f, velChange / transmission->fMaxVelocity);
		} else {
			relativeVel = 0.0f;
		}
		if (relativeVel >= relativeVelChange)
			relativeVelChange = relativeVel;
	} else {
		relativeVelChange = 0.0f;
	}

	return Max(relativeVelChange, Min(1.0f, Abs(automobile->m_vecTurnSpeed.z) * 20.0f));
}

float
cAudioManager::GetVehicleNonDriveWheelSkidValue(uint8 wheel, CAutomobile *automobile, cTransmission *transmission, float velocityChange)
{
	float relativeVelChange;

	if (automobile->m_aWheelState[wheel] == WHEEL_STATE_SKIDDING)
		relativeVelChange = Min(1.0f, Abs(velocityChange) / transmission->fMaxVelocity);
	else
		relativeVelChange = 0.0f;

	return Max(relativeVelChange, Min(1.0f, Abs(automobile->m_vecTurnSpeed.z) * 20.0f));
}

void
cAudioManager::ProcessVehicleHorn(cVehicleParams *params)
{
	const float SOUND_INTENSITY = 40.0f;

	CAutomobile *automobile;

	if (params->m_fDistance < SQR(SOUND_INTENSITY)) {
		automobile = (CAutomobile *)params->m_pVehicle;
		if ((!automobile->m_bSirenOrAlarm || !UsesSirenSwitching(params->m_nIndex)) && automobile->GetModelIndex() != MI_MRWHOOP) {
			if (automobile->m_nCarHornTimer) {
				if (params->m_pVehicle->GetStatus() != STATUS_PLAYER) {
					automobile->m_nCarHornTimer = Min(44, automobile->m_nCarHornTimer);
					if (automobile->m_nCarHornTimer == 44)
						automobile->m_nCarHornPattern = (m_FrameCounter + m_sQueueSample.m_nEntityIndex) & 7;
					if (!hornPatternsArray[automobile->m_nCarHornPattern][44 - automobile->m_nCarHornTimer])
						return;
				}

				CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
				m_sQueueSample.m_nVolume = ComputeVolume(80, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_nCounter = 4;
					m_sQueueSample.m_nSampleIndex = aVehicleSettings[params->m_nIndex].m_nHornSample;
					m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
					m_sQueueSample.m_bIs2D = false;
					m_sQueueSample.m_nReleasingVolumeModificator = 2;
					m_sQueueSample.m_nFrequency = aVehicleSettings[params->m_nIndex].m_nHornFrequency;
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
cAudioManager::ProcessVehicleSirenOrAlarm(cVehicleParams *params)
{
	const float SOUND_INTENSITY = 110.0f;

	if (params->m_fDistance < SQR(SOUND_INTENSITY)) {
		CVehicle *veh = params->m_pVehicle;
		if (veh->m_bSirenOrAlarm == false && !veh->IsAlarmOn())
			return true;

		if (veh->IsAlarmOn()) {
			if (CTimer::GetTimeInMilliseconds() > veh->m_bRainAudioCounter)
				veh->m_bRainAudioCounter = CTimer::GetTimeInMilliseconds() + 750;

			if (veh->m_bRainAudioCounter < CTimer::GetTimeInMilliseconds() + 375)
				return true;
		}

		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(veh->bIsDrowning ? 20 : 80, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 5;
			if (UsesSiren(params->m_nIndex)) {
				if (params->m_pVehicle->GetStatus() == STATUS_ABANDONED)
					return true;
				if (veh->m_nCarHornTimer && params->m_nIndex != FIRETRUK) {
					m_sQueueSample.m_nSampleIndex = SFX_SIREN_FAST;
					if (params->m_nIndex == FBICAR)
						m_sQueueSample.m_nFrequency = 16113;
					else
						m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_SIREN_FAST);
					m_sQueueSample.m_nCounter = 60;
				} else {
					m_sQueueSample.m_nSampleIndex = aVehicleSettings[params->m_nIndex].m_nSirenOrAlarmSample;
					m_sQueueSample.m_nFrequency = aVehicleSettings[params->m_nIndex].m_nSirenOrAlarmFrequency;
				}
			} else {
				m_sQueueSample.m_nSampleIndex = aVehicleSettings[params->m_nIndex].m_nSirenOrAlarmSample;
				m_sQueueSample.m_nFrequency = aVehicleSettings[params->m_nIndex].m_nSirenOrAlarmFrequency;
			}
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessVehicleReverseWarning(cVehicleParams *params)
{
	const float SOUND_INTENSITY = 50.0f;

	CVehicle *veh = params->m_pVehicle;

	if (params->m_fDistance >= SQR(SOUND_INTENSITY))
		return false;

	if (veh->bEngineOn && veh->m_fGasPedal < 0.0f) {
		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(60, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 12;
			m_sQueueSample.m_nSampleIndex = SFX_REVERSE_WARNING;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessVehicleDoors(cVehicleParams *params)
{
	const float SOUND_INTENSITY = 40.0f;

	CAutomobile *automobile;
	int8 doorState;
	int32 emittingVol;
	float velocity;

	if (params->m_fDistance >= SQR(SOUND_INTENSITY))
		return false;

	automobile = (CAutomobile *)params->m_pVehicle;
	CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
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
						m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessAirBrakes(cVehicleParams *params)
{
	CAutomobile *automobile;
	uint8 rand;

	if (params->m_fDistance > SQR(30))
		return false;
	automobile = (CAutomobile *)params->m_pVehicle;
	if (!automobile->bEngineOn)
		return true;

	if ((automobile->m_fVelocityChangeForAudio < 0.025f || params->m_fVelocityChange >= 0.025f) &&
	    (automobile->m_fVelocityChangeForAudio > -0.025f || params->m_fVelocityChange <= 0.025f))
		return true;

	CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
	rand = m_anRandomTable[0] % 10 + 70;
	m_sQueueSample.m_nVolume = ComputeVolume(rand, 30.0f, m_sQueueSample.m_fDistance);
	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 13;
		m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_AIR_BRAKES);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessEngineDamage(cVehicleParams *params)
{
	const int engineDamageIntensity = 40;

	CAutomobile *veh;
	uint8 engineStatus;
	uint8 emittingVolume;

	if (params->m_fDistance >= SQR(engineDamageIntensity))
		return false;
	veh = (CAutomobile *)params->m_pVehicle;
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
		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(emittingVolume, engineDamageIntensity, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 28;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessCarBombTick(cVehicleParams *params)
{
	CAutomobile *automobile;

	if (params->m_fDistance >= SQR(40.f))
		return false;
	automobile = (CAutomobile *)params->m_pVehicle;
	if (automobile->bEngineOn && automobile->m_bombType == CARBOMB_TIMEDACTIVE) {
		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		m_sQueueSample.m_nVolume = ComputeVolume(60, 40.f, m_sQueueSample.m_fDistance);
		if (m_sQueueSample.m_nVolume != 0) {
			m_sQueueSample.m_nCounter = 35;
			m_sQueueSample.m_nSampleIndex = SFX_COUNTDOWN;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessVehicleOneShots(cVehicleParams *params)
{
	int16 event;
	uint8 emittingVol;
	float relVol;
	float vol;
	bool noReflections;
	float maxDist;
	cPedParams pedParams;

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
			switch (aVehicleSettings[params->m_nIndex].m_bDoorType) {
			case DOOR_TYPE_OLD:
				m_sQueueSample.m_nSampleIndex = SFX_OLD_CAR_DOOR_CLOSE;
				break;
			case DOOR_TYPE_NEW:
			default:
				m_sQueueSample.m_nSampleIndex = SFX_NEW_CAR_DOOR_CLOSE;
				break;
			case DOOR_TYPE_TRUCK:
				m_sQueueSample.m_nSampleIndex = SFX_TRUCK_DOOR_CLOSE;
				break;
			case DOOR_TYPE_AIRBREAK:
				m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
				break;
			}
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] + 22;
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
			switch (aVehicleSettings[params->m_nIndex].m_bDoorType) {
			case DOOR_TYPE_OLD:
				m_sQueueSample.m_nSampleIndex = SFX_OLD_CAR_DOOR_OPEN;
				break;
			case DOOR_TYPE_NEW:
			default:
				m_sQueueSample.m_nSampleIndex = SFX_NEW_CAR_DOOR_OPEN;
				break;
			case DOOR_TYPE_TRUCK:
				m_sQueueSample.m_nSampleIndex = SFX_TRUCK_DOOR_OPEN;
				break;
			case DOOR_TYPE_AIRBREAK:
				m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
				break;
			}
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] + 10;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iWheelIndex++;
			if (iWheelIndex > 85)
				iWheelIndex = 82;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_TYRE_BUMP);
			m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
			if (params->m_nIndex == RCBANDIT) {
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		/*case SOUND_17: {
			const float SOUND_INTENSITY = 50.0f;
			m_sQueueSample.m_nSampleIndex = SFX_POLICE_BOAT_THUMB_OFF;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = 47;
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_POLICE_BOAT_THUMB_OFF) + RandomDisplacement(600);
			m_sQueueSample.m_nReleasingVolumeModificator = 2;
			m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
			emittingVol = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			maxDist = SQR(SOUND_INTENSITY);
			break;
		}*/
		case SOUND_TRAIN_DOOR_CLOSE:
		case SOUND_TRAIN_DOOR_OPEN: {
			const float SOUND_INTENSITY = 35.0f;
			m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		case SOUND_91:
		case SOUND_BOMB_ONIGNITION_ACTIVATED:
		case SOUND_BOMB_TICK: {
			const float SOUND_INTENSITY = 50.0f;
			m_sQueueSample.m_nSampleIndex = SFX_ARM_BOMB;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			pedParams.m_pPed = nil;
			pedParams.m_bDistanceCalculated = false;
			pedParams.m_fDistance = 0.0f;
			pedParams.m_bDistanceCalculated = params->m_bDistanceCalculated;
			pedParams.m_fDistance = params->m_fDistance;
			SetupPedComments(&pedParams, SOUND_PED_HELI_PLAYER_FOUND);
			continue;
		/* case SOUND_PED_BODYCAST_HIT:
			pedParams.m_pPed = nil;
			pedParams.m_bDistanceCalculated = false;
			pedParams.m_fDistance = 0.0f;
			pedParams.m_bDistanceCalculated = params->m_bDistanceCalculated;
			pedParams.m_fDistance = params->m_fDistance;
			SetupPedComments(&pedParams, SOUND_PED_BODYCAST_HIT);
			continue; */
		case SOUND_WATER_FALL: {
			const float SOUND_INTENSITY = 40.0f;
			m_sQueueSample.m_nSampleIndex = SFX_SPLASH_1;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		if (params->m_fDistance < maxDist) {
			CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
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
#ifdef GTA_TRAIN
bool
cAudioManager::ProcessTrainNoise(cVehicleParams *params)
{
	const float SOUND_INTENSITY = 300.0f;

	CTrain *train;
	uint8 emittingVol;
	float speedMultipler;

	if (params->m_fDistance >= SQR(SOUND_INTENSITY))
		return false;

	if (params->m_fVelocityChange > 0.0f) {
		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		train = (CTrain *)params->m_pVehicle;
		speedMultipler = Min(1.0f, train->m_fSpeed * 250.f / 51.f);
		emittingVol = (75.f * speedMultipler);
		if (train->m_fWagonPosition == 0.0f) {
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 32;
				m_sQueueSample.m_nSampleIndex = SFX_TRAIN_FAR;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		if (params->m_fDistance < SQR(SOUND_INTENSITY)) {
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 33;
				m_sQueueSample.m_nSampleIndex = SFX_TRAIN_NEAR;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
#endif
bool
cAudioManager::ProcessBoatEngine(cVehicleParams *params)
{
	CBoat *boat;
	float padRelativeAccerate;
	float gasPedal;
	int32 padAccelerate;
	uint8 emittingVol;
	float oneShotVol;

	static uint16 LastAccel = 0;
	static uint8 LastVol = 0;

	static const int intensity = 50;

	if (params->m_fDistance < SQR(intensity)) {
		boat = (CBoat *)params->m_pVehicle;
		if (params->m_nIndex == REEFER) {
			CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
			m_sQueueSample.m_nVolume = ComputeVolume(80, 50.f, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				m_sQueueSample.m_nCounter = 39;
				m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
				m_sQueueSample.m_nFrequency = 10386;
				m_sQueueSample.m_nFrequency += (m_sQueueSample.m_nEntityIndex * 65536) % 1000;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			if (FindPlayerVehicle() == params->m_pVehicle) {
				if (Pads[0].GetAccelerate() <= Pads[0].GetBrake())
					padAccelerate = Pads[0].GetBrake();
				else
					padAccelerate = Pads[0].GetAccelerate();
				padRelativeAccerate = padAccelerate / 255;
				emittingVol = (100.f * padRelativeAccerate) + 15;
				m_sQueueSample.m_nFrequency = (3000.f * padRelativeAccerate) + 6000;
				if (!boat->m_bIsAnchored)
					m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
			} else {
				gasPedal = Abs(boat->m_fGasPedal);
				if (gasPedal > 0.0f) {
					m_sQueueSample.m_nFrequency = 6000;
					emittingVol = 15;
				} else {
					emittingVol = (100.f * gasPedal) + 15;
					m_sQueueSample.m_nFrequency = (3000.f * gasPedal) + 6000;
					if (!boat->m_bIsAnchored)
						m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
				}
			}
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, 50.f, m_sQueueSample.m_fDistance);
			if (!m_sQueueSample.m_nVolume)
				return true;
			m_sQueueSample.m_nCounter = 40;
			m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
			m_sQueueSample.m_nFrequency += (m_sQueueSample.m_nEntityIndex * 65536) % 1000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			if (FindPlayerVehicle() == params->m_pVehicle) {
				if (Pads[0].GetAccelerate() <= Pads[0].GetBrake())
					padAccelerate = Pads[0].GetBrake();
				else
					padAccelerate = Pads[0].GetAccelerate();
				if (padAccelerate <= 20) {
					emittingVol = 45 - 45 * padAccelerate / 40;
					m_sQueueSample.m_nFrequency = 100 * padAccelerate + 11025;
					m_sQueueSample.m_nCounter = 39;
					m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
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
					m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
				}
				LastVol = emittingVol;
				LastAccel = padAccelerate;
			} else {
				gasPedal = Abs(boat->m_fGasPedal);
				if (gasPedal > 0.0f) {
					m_sQueueSample.m_nFrequency = 11025;
					emittingVol = 45;
					m_sQueueSample.m_nCounter = 39;
					m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
				} else {
					emittingVol = (105.f * gasPedal) + 15;
					m_sQueueSample.m_nFrequency = (4000.f * gasPedal) + 8000;
					if (!boat->m_bIsAnchored)
						m_sQueueSample.m_nFrequency = 11 * m_sQueueSample.m_nFrequency / 10;
					m_sQueueSample.m_nCounter = 40;
					m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
				}
			}
			CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, 50.f, m_sQueueSample.m_fDistance);
			if (!m_sQueueSample.m_nVolume)
				return true;
			m_sQueueSample.m_nFrequency += (m_sQueueSample.m_nEntityIndex * 65536) % 1000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessBoatMovingOverWater(cVehicleParams *params)
{
	float velocityChange;
	int32 vol;
	float multiplier;

	if (params->m_fDistance > SQR(50))
		return false;

	velocityChange = Abs(params->m_fVelocityChange);
	if (velocityChange <= 0.0005f && ((CBoat*)params->m_pVehicle)->bBoatInWater)
		return true;

	velocityChange = Min(0.75f, velocityChange);
	multiplier = (velocityChange - 0.0005f) * 1.3342f;
	CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
	vol = (30.f * multiplier);
	m_sQueueSample.m_nVolume = ComputeVolume(vol, 50.f, m_sQueueSample.m_fDistance);
	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 38;
		m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessHelicopter(cVehicleParams *params)
{
	CHeli *heli;
	float MaxDist;
	float dist;
	float baseDist;
	int32 emittingVol;
	static const tHelicopterSampleData gHeliSfxRanges[3] = {{400.f, 380.f, 100}, {100.f, 70.f, MAX_VOLUME}, {60.f, 30.f, MAX_VOLUME}};

	if (SQR(gHeliSfxRanges[0].m_fMaxDistance) <= params->m_fDistance)
		return false;

	CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
	heli = (CHeli *)params->m_pVehicle;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::ProcessPlane(cVehicleParams *params)
{
	switch (params->m_nIndex) {
	case AIRTRAIN:
		ProcessJumbo(params);
		break;
	case DEADDODO:
		ProcessCesna(params);
		break;
	default:
		debug("Plane Model Id is %d\n, ", params->m_pVehicle->GetModelIndex());
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
cAudioManager::ProcessJumbo(cVehicleParams *params)
{
	CPlane *plane;
	float position;

	if (params->m_fDistance < SQR(440)) {
		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		plane = (CPlane *)params->m_pVehicle;
		DoJumboVolOffset();
		position = PlanePathPosition[plane->m_nPlaneId];
		if (position <= TakeOffPoint) {
			if (plane->m_fSpeed <= 0.10334f) {
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
				if (plane->m_fSpeed > 0.10334f) {
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
		modificator = (plane->m_fSpeed - 0.10334f) * 1.676f;
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
		const float modificator = Min(1.f, (plane->m_fSpeed - 0.10334f) * 1.676f);
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::SetupJumboWhineSound(uint8 emittingVol, int32 freq)
{
	const float SOUND_INTENSITY = 170.0f;

	if (m_sQueueSample.m_fDistance >= SOUND_INTENSITY)
		return false;

	m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);

	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 2;
		m_sQueueSample.m_nSampleIndex = SFX_JUMBO_WHINE;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
cAudioManager::SetupJumboEngineSound(uint8 vol, int32 freq)
{
	const float SOUND_INTENSITY = 180.0f;
	if (m_sQueueSample.m_fDistance >= SOUND_INTENSITY)
		return false;

	uint8 emittingVol = vol - gJumboVolOffsetPercentage / 100;
	m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, SOUND_INTENSITY, m_sQueueSample.m_fDistance);
	if (m_sQueueSample.m_nVolume != 0) {
		m_sQueueSample.m_nCounter = 3;
		m_sQueueSample.m_nSampleIndex = SFX_JUMBO_ENGINE;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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

	params.m_pPed = nil;
	params.m_bDistanceCalculated = false;
	params.m_fDistance = 0.0f;

	m_sQueueSample.m_vecPos = ped->GetPosition();

	// params.m_bDistanceCalculated = false;
	params.m_pPed = (CPed *)ped;
	params.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
	ProcessPedOneShots(&params);
}

void
cAudioManager::ProcessPedOneShots(cPedParams *params)
{
	uint8 emittingVol;
	int32 sampleIndex;

	CPed *ped = params->m_pPed;

	bool stereo;
	int16 sound;
	bool noReflection;
	CWeapon *weapon;
	float maxDist = 0.f; // uninitialized variable

	static uint8 iSound = 21;

	weapon = params->m_pPed->GetWeapon();
	for (uint32 i = 0; i < m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_AudioEvents; i++) {
		noReflection = false;
		stereo = false;
		m_sQueueSample.m_bRequireReflection = false;
		sound = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
		switch (sound) {
		case SOUND_STEP_START:
		case SOUND_STEP_END:
			if (!params->m_pPed->bIsLooking) {
				emittingVol = m_anRandomTable[3] % 15 + 45;
				if (FindPlayerPed() != m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_pEntity)
					emittingVol /= 2;
				maxDist = 400.f;
				switch (params->m_pPed->m_nSurfaceTouched) {
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
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nCounter = m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] - 28;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 17);
				switch (params->m_pPed->m_nMoveState) {
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
			}
			break;
		case SOUND_FALL_LAND:
		case SOUND_FALL_COLLAPSE:
			if (!ped->bIsLooking) {
				maxDist = SQR(30);
				emittingVol = m_anRandomTable[3] % 20 + 80;
				if (ped->m_nSurfaceTouched == SURFACE_WATER) {
					m_sQueueSample.m_nSampleIndex = (m_anRandomTable[3] & 3) + SFX_FOOTSTEP_WATER_1;
				} else if (sound == SOUND_FALL_LAND) {
					m_sQueueSample.m_nSampleIndex = SFX_BODY_LAND;
				} else {
					m_sQueueSample.m_nSampleIndex = SFX_BODY_LAND_AND_FALL;
				}
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			}
			break;
		case SOUND_FIGHT_PUNCH_33:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			m_sQueueSample.m_nFrequency = 18000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_KICK_34:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			m_sQueueSample.m_nFrequency = 16500;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_HEADBUTT_35:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			m_sQueueSample.m_nFrequency = 20000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_PUNCH_36:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			m_sQueueSample.m_nFrequency = 18000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_PUNCH_37:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			m_sQueueSample.m_nFrequency = 16500;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_CLOSE_PUNCH_38:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			m_sQueueSample.m_nFrequency = 20000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_PUNCH_39:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			m_sQueueSample.m_nFrequency = 18000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_PUNCH_OR_KICK_BELOW_40:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			m_sQueueSample.m_nFrequency = 16500;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_PUNCH_41:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			m_sQueueSample.m_nFrequency = 20000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_PUNCH_FROM_BEHIND_42:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			m_sQueueSample.m_nFrequency = 18000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_KNEE_OR_KICK_43:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			m_sQueueSample.m_nFrequency = 16500;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
		case SOUND_FIGHT_KICK_44:
			m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			m_sQueueSample.m_nFrequency = 20000;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound;
			stereo = true;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound++;
			stereo = true;
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
				noReflection = true;
			break;
		case SOUND_WEAPON_SHOT_FIRED:
			weapon = ped->GetWeapon();
			switch (weapon->m_eWeaponType) {
			case WEAPONTYPE_COLT45:
				m_sQueueSample.m_nSampleIndex = SFX_COLT45_LEFT;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nCounter = iSound++;
				stereo = true;
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
					noReflection = true;
				break;
			case WEAPONTYPE_UZI:
				m_sQueueSample.m_nSampleIndex = SFX_UZI_LEFT;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nCounter = iSound++;
				stereo = true;
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
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nCounter = iSound++;
				stereo = true;
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
					noReflection = true;
				break;
			case WEAPONTYPE_RUGER:
				m_sQueueSample.m_nSampleIndex = SFX_AK47_LEFT;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nCounter = iSound++;
				stereo = true;
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
			case WEAPONTYPE_M4:
				m_sQueueSample.m_nSampleIndex = SFX_M60_LEFT;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nCounter = iSound++;
				stereo = true;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_M60_LEFT);
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
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nCounter = iSound++;
				stereo = true;
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
					noReflection = true;
				break;
			case WEAPONTYPE_ROCKETLAUNCHER:
				m_sQueueSample.m_nSampleIndex = SFX_ROCKET_LEFT;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nCounter = iSound++;
				stereo = true;
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
					noReflection = true;
				break;
			case WEAPONTYPE_FLAMETHROWER:
				m_sQueueSample.m_nSampleIndex = SFX_FLAMETHROWER_LEFT;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
					noReflection = true;
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
				m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
				m_sQueueSample.m_nFrequency = 39243;
				break;
			case WEAPONTYPE_SHOTGUN:
				m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
				m_sQueueSample.m_nFrequency = 30290;
				break;
			case WEAPONTYPE_RUGER:
				m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_AK47_RELOAD);
				break;
			case WEAPONTYPE_M4:
				m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_AK47_RELOAD);
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
			stereo = true;
			m_sQueueSample.m_nFrequency += RandomDisplacement(300);
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nSampleIndex = SFX_UZI_END_LEFT;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound++;
			stereo = true;
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
				noReflection = true;
			break;
		case SOUND_WEAPON_FLAMETHROWER_FIRE:
			m_sQueueSample.m_nSampleIndex = SFX_FLAMETHROWER_START_LEFT;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound++;
			stereo = true;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
			m_sQueueSample.m_nCounter = iSound++;
			stereo = true;
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

		if (stereo && iSound > 60)
			iSound = 21;
		if (params->m_fDistance < maxDist) {
			CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
			m_sQueueSample.m_nVolume = ComputeVolume(emittingVol, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
			if (m_sQueueSample.m_nVolume != 0) {
				if (noReflection) {
					if (m_sQueueSample.m_fDistance < 0.2f * m_sQueueSample.m_fSoundIntensity) {
						m_sQueueSample.m_bIs2D = true;
						m_sQueueSample.m_nOffset = 0;
					} else {
						noReflection = false;
					}
				}
				m_sQueueSample.m_bReverbFlag = true;
				AddSampleToRequestedQueue();
				if (noReflection) {
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
cAudioManager::SetupPedComments(cPedParams *params, uint32 sound)
{
	CPed *ped = params->m_pPed;
	uint8 emittingVol;
	float soundIntensity;
	tPedComment pedComment;

	if (ped != nil) {
		switch (sound) {
		/*case SOUND_AMMUNATION_WELCOME_1:
			pedComment.m_nSampleIndex = SFX_AMMU_D;
			break;
		case SOUND_AMMUNATION_WELCOME_2:
			pedComment.m_nSampleIndex = SFX_AMMU_E;
			break;
		case SOUND_AMMUNATION_WELCOME_3:
			pedComment.m_nSampleIndex = SFX_AMMU_F;
			break;*/
		default:
			pedComment.m_nSampleIndex = GetPedCommentSfx(ped, sound);
			if (pedComment.m_nSampleIndex == NO_SAMPLE)
				return;
			break;
		}

		soundIntensity = 50.0f;
	} else {
		switch (sound) {
		/*case SOUND_PED_HELI_PLAYER_FOUND:
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
			break;*/
		default:
			return;
		}
	}

	if (params->m_fDistance < SQR(soundIntensity)) {
		CalculateDistance(params->m_bDistanceCalculated, params->m_fDistance);
		if (sound != SOUND_PAGER) {
			switch (sound) {
			/*case SOUND_AMMUNATION_WELCOME_1:
			case SOUND_AMMUNATION_WELCOME_2:
			case SOUND_AMMUNATION_WELCOME_3:
				emittingVol = MAX_VOLUME;
				break; */
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
	//if (ped->IsPlayer())
	//	return GetPlayerTalkSfx(sound);

	// TODO(Miami): ped comments

	return NO_SAMPLE;
}

void
cAudioManager::GetPhrase(uint32 *phrase, uint32 *prevPhrase, uint32 sample, uint32 maxOffset) const
{
	*phrase = sample + m_anRandomTable[m_sQueueSample.m_nEntityIndex & 3] % maxOffset;

	// check if the same sfx like last time, if yes, then try use next one,
	// if exceeded range, then choose first available sample
	if (*phrase == *prevPhrase && ++*phrase >= sample + maxOffset)
		*phrase = sample;
	*prevPhrase = *phrase;
}

#pragma region PED_COMMENTS
// TODO: all the ped comment funcs should follow here
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
		AudioManager.m_sQueueSample.m_nBankIndex = SAMPLEBANK_PED;
		AudioManager.m_sQueueSample.m_nReleasingVolumeModificator = 3;
		AudioManager.m_sQueueSample.m_nVolume = m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_bVolume;
		AudioManager.m_sQueueSample.m_fDistance = m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_fDistance;
		AudioManager.m_sQueueSample.m_nLoopCount = 1;
		AudioManager.m_sQueueSample.m_nLoopStart = 0;
		AudioManager.m_sQueueSample.m_nLoopEnd = -1;
		AudioManager.m_sQueueSample.m_nEmittingVolume = MAX_VOLUME;
		AudioManager.m_sQueueSample.m_fSpeedMultiplier = 3.0f;
		switch (sampleIndex) {
		//case SFX_POLICE_HELI_1:
		//case SFX_POLICE_HELI_2:
		//case SFX_POLICE_HELI_3:
		//	AudioManager.m_sQueueSample.m_fSoundIntensity = 400.0f;
		//	break;
		default:
			AudioManager.m_sQueueSample.m_fSoundIntensity = 50.0f;
			break;
		}
		AudioManager.m_sQueueSample.m_bReleasingSoundFlag = true;
		AudioManager.m_sQueueSample.m_vecPos = m_asPedComments[m_nActiveBank][m_nIndexMap[m_nActiveBank][0]].m_vecPos;

		/*if (sampleIndex >= SFX_AMMU_D && sampleIndex <= SFX_AMMU_F) {
			AudioManager.m_sQueueSample.m_bReverbFlag = false;
			AudioManager.m_sQueueSample.m_bRequireReflection = false;
		} else*/ {
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
		actualUsedBank = SAMPLEBANK_PED;
		m_nActiveBank = SAMPLEBANK_MAIN;
	} else {
		actualUsedBank = SAMPLEBANK_MAIN;
		m_nActiveBank = SAMPLEBANK_PED;
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
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				break;
			case EXPLOSION_MOLOTOV:
				m_sQueueSample.m_fSoundIntensity = 200.0f;
				m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_3;
				m_sQueueSample.m_nFrequency = RandomDisplacement(1000) + 19000;
				m_sQueueSample.m_nReleasingVolumeModificator = 0;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				break;
			case EXPLOSION_MINE:
			case EXPLOSION_HELI_BOMB:
				m_sQueueSample.m_fSoundIntensity = 300.0f;
				m_sQueueSample.m_nSampleIndex = SFX_ROCKET_LEFT;
				m_sQueueSample.m_nFrequency = RandomDisplacement(1000) + 12347;
				m_sQueueSample.m_nReleasingVolumeModificator = 0;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				break;
			default:
				m_sQueueSample.m_fSoundIntensity = 400.0f;
				m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_1;
				m_sQueueSample.m_nFrequency = RandomDisplacement(2000) + 38000;
				if (type == EXPLOSION_HELI)
					m_sQueueSample.m_nFrequency = 8 * m_sQueueSample.m_nFrequency / 10;
				m_sQueueSample.m_nReleasingVolumeModificator = 0;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_EXTRAS;
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
					m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
	const float SOUND_INTENSITY = 900.0f;

	for (int32 i = 0; i < NUM_WATERCANNONS; i++) {
		if (CWaterCannons::aCannons[i].m_nId) {
			m_sQueueSample.m_vecPos = CWaterCannons::aCannons[0].m_avecPos[CWaterCannons::aCannons[i].m_nCur];
			float distSquared = GetDistanceSquared(m_sQueueSample.m_vecPos);
			if (distSquared < SOUND_INTENSITY) {
				m_sQueueSample.m_fDistance = Sqrt(distSquared);
				m_sQueueSample.m_nVolume = ComputeVolume(50, m_sQueueSample.m_fSoundIntensity, m_sQueueSample.m_fDistance);
				if (m_sQueueSample.m_nVolume != 0) {
					m_sQueueSample.m_fSoundIntensity = SOUND_INTENSITY;
					m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
					m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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

	cPedParams male;
	cPedParams female;

	static uint8 iSound = 0;

	switch (sound) {
	case SCRIPT_SOUND_INJURED_PED_MALE_OUCH_S:
	case SCRIPT_SOUND_INJURED_PED_MALE_OUCH_L:
		male.m_pPed = nil;
		male.m_bDistanceCalculated = false;
		male.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
		SetupPedComments(&male, SOUND_INJURED_PED_MALE_OUCH);
		return;
	case SCRIPT_SOUND_INJURED_PED_FEMALE_OUCH_S:
	case SCRIPT_SOUND_INJURED_PED_FEMALE_OUCH_L:
		female.m_pPed = nil;
		female.m_bDistanceCalculated = false;
		female.m_fDistance = GetDistanceSquared(m_sQueueSample.m_vecPos);
		SetupPedComments(&female, SOUND_INJURED_PED_FEMALE);
		return;
	case SCRIPT_SOUND_GATE_START_CLUNK:
	case SCRIPT_SOUND_GATE_STOP_CLUNK:
		m_sQueueSample.m_fSoundIntensity = 40.0f;
		m_sQueueSample.m_nSampleIndex = SFX_COL_GATE;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 32);
		m_sQueueSample.m_nReleasingVolumeModificator = 9;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		emittingVolume = m_anRandomTable[2] % 20 + 90;
		break;
	/*
	case SCRIPT_SOUND_TRAIN_ANNOUNCEMENT_1:
	case SCRIPT_SOUND_TRAIN_ANNOUNCEMENT_2:
		if (!SampleManager.IsSampleBankLoaded(SAMPLEBANK_TRAIN))
			return;
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = SFX_TRAIN_STATION_ANNOUNCE;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_TRAIN;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_TRAIN_STATION_ANNOUNCE);
		m_sQueueSample.m_nReleasingVolumeModificator = 0;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		m_sQueueSample.m_bIs2D = false;
		break;*/
	case SCRIPT_SOUND_PAYPHONE_RINGING:
		m_sQueueSample.m_fSoundIntensity = 80.0f;
		m_sQueueSample.m_nSampleIndex = SFX_PHONE_RING;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
		emittingVolume = 70;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_GLASS_SMASH);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		break;
	case SCRIPT_SOUND_GLASS_BREAK_S:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_GLASS_SMASH;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
		emittingVolume = 60;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_GLASS_SMASH);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		break;
	case SCRIPT_SOUND_GLASS_CRACK:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_GLASS_CRACK;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
		m_sQueueSample.m_nFrequency = RandomDisplacement(2000) + 19000;
		m_sQueueSample.m_nReleasingVolumeModificator = 9;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		emittingVolume = RandomDisplacement(11) + 25;
		break;
	case SCRIPT_SOUND_BOX_DESTROYED_1:
		m_sQueueSample.m_fSoundIntensity = 60.0f;
		m_sQueueSample.m_nSampleIndex = SFX_WOODEN_BOX_SMASH;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
				m_sQueueSample.m_fSoundIntensity = 20.0f;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				m_sQueueSample.m_bIs2D = false;
				emittingVolume = m_anRandomTable[2] % 20 + 30;
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
				return;
			case SURFACE_WATER:
				return;
			default:
				break;
			}
		}
		m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_1;
		m_sQueueSample.m_nFrequency = RandomDisplacement(750) + 18000;
		m_sQueueSample.m_nReleasingVolumeModificator = 15;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
		m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		m_sQueueSample.m_bIs2D = false;
		emittingVolume = m_anRandomTable[2] % 20 + 30;
		break;
	case SCRIPT_SOUND_GUNSHELL_DROP_SOFT:
		m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_2;
		m_sQueueSample.m_nFrequency = RandomDisplacement(500) + 11000;
		m_sQueueSample.m_nReleasingVolumeModificator = 18;
		m_sQueueSample.m_fSoundIntensity = 20.0f;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_1;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_1_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_1;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_1;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_2_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_2;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_2;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_2);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_2_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_2;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_2;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_2);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_3_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_3;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_3;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_3);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_3_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_3;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_3;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_3);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_4_LOOP_S:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_S;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_4;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_4;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_4);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_4_LOOP_L:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_4;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_4;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_4);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PARTY_1_LOOP:
		m_sQueueSample.m_fSoundIntensity = SCRIPT_OBJECT_INTENSITY_L;
		m_sQueueSample.m_nSampleIndex = SFX_CLUB_1;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_BUILDING_CLUB_1;
		emittingVolume = MAX_VOLUME;
		m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_CLUB_1);
		m_sQueueSample.m_nReleasingVolumeModificator = 3;
		m_sQueueSample.m_nReleasingVolumeDivider = 3;
		m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		break;
	case SCRIPT_SOUND_PRETEND_FIRE_LOOP:
		m_sQueueSample.m_fSoundIntensity = 50.0f;
		m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
#pragma endregion All the code for script object audio on the map

void
cAudioManager::ProcessWeather(int32 id)
{
	uint8 vol;
	static uint8 iSound = 0;

	if (m_asAudioEntities[id].m_AudioEvents && m_asAudioEntities[id].m_awAudioEvent[0] == SOUND_LIGHTNING) {
		if (m_asAudioEntities[id].m_afVolume[0] >= 10.f) {
			m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_1;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_EXTRAS;
			m_sQueueSample.m_nFrequency = RandomDisplacement(500) + 4000;
			vol = (m_asAudioEntities[id].m_afVolume[0] - 10.f) + 40;
		} else {
			m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_2;
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
		//case SOUND_GARAGE_NO_MONEY:
		//case SOUND_GARAGE_BAD_VEHICLE:
		//case SOUND_GARAGE_BOMB_ALREADY_SET:
		//	m_sQueueSample.m_nSampleIndex = SFX_PICKUP_ERROR_LEFT;
		//	stereo = true;
		//	break;
		//case SOUND_GARAGE_OPENING:
		//case SOUND_GARAGE_BOMB1_SET:
		//case SOUND_GARAGE_BOMB2_SET:
		//case SOUND_GARAGE_BOMB3_SET:
		//case SOUND_41:
		//case SOUND_GARAGE_VEHICLE_DECLINED:
		//case SOUND_GARAGE_VEHICLE_ACCEPTED:
		//case SOUND_PICKUP_HEALTH:
		//case SOUND_4B:
		//case SOUND_PICKUP_ADRENALINE:
		//case SOUND_PICKUP_ARMOUR:
		//case SOUND_EVIDENCE_PICKUP:
		//case SOUND_UNLOAD_GOLD:
		//	m_sQueueSample.m_nSampleIndex = SFX_PICKUP_2_LEFT;
		//	processedPickup = true;
		//	stereo = true;
		//	break;
		//case SOUND_PICKUP_WEAPON_BOUGHT:
		//case SOUND_PICKUP_WEAPON:
		//	m_sQueueSample.m_nSampleIndex = SFX_PICKUP_1_LEFT;
		//	processedPickup = true;
		//	stereo = true;
		//	break;
		//case SOUND_PICKUP_ERROR:
		//	m_sQueueSample.m_nSampleIndex = SFX_PICKUP_ERROR_LEFT;
		//	processedPickup = true;
		//	stereo = true;
		//	break;
		//case SOUND_PICKUP_BONUS:
		//case SOUND_PICKUP_MONEY:
		//case SOUND_PICKUP_HIDDEN_PACKAGE:
		//case SOUND_PICKUP_PACMAN_PILL:
		//case SOUND_PICKUP_PACMAN_PACKAGE:
		//case SOUND_PICKUP_FLOAT_PACKAGE:
		//	m_sQueueSample.m_nSampleIndex = SFX_PICKUP_3_LEFT;
		//	processedPickup = true;
		//	stereo = true;
		//	break;
		//case SOUND_PAGER:
		//	m_sQueueSample.m_nSampleIndex = SFX_PAGER;
		//	break;
		case SOUND_RACE_START_3:
		case SOUND_RACE_START_2:
		case SOUND_RACE_START_1:
		case SOUND_CLOCK_TICK:
			m_sQueueSample.m_nSampleIndex = SFX_TIMER_BEEP;
			break;
		case SOUND_RACE_START_GO:
			m_sQueueSample.m_nSampleIndex = SFX_PART_MISSION_COMPLETE_LEFT;
			break;
		case SOUND_PART_MISSION_COMPLETE:
			m_sQueueSample.m_nSampleIndex = SFX_PART_MISSION_COMPLETE_LEFT;
			processedMission = true;
			break;
		case SOUND_FRONTEND_MENU_STARTING:
			m_sQueueSample.m_nSampleIndex = SFX_FE_HIGHLIGHT_LEFT;
			stereo = true;
			break;
		case SOUND_FRONTEND_MENU_NEW_PAGE:
			m_sQueueSample.m_nSampleIndex = SFX_FE_HIGHLIGHT_LEFT;
			stereo = true;
			frontendBank = true;
			frontendBank = true;
			break;
		case SOUND_FRONTEND_MENU_SETTING_CHANGE:
			m_sQueueSample.m_nSampleIndex = SFX_FE_SELECT_LEFT;
			stereo = true;
			frontendBank = true;
			frontendBank = true;
			frontendBank = true;
			break;
		//case SOUND_FRONTEND_EXIT:
		//	m_sQueueSample.m_nSampleIndex = SFX_SUB_MENU_BACK_LEFT;
		//	stereo = true;
		//	break;
		//case SOUND_FRONTEND_AUDIO_TEST:
		//	m_sQueueSample.m_nSampleIndex = m_anRandomTable[0] % 3 + SFX_NOISE_BURST_1;
		//	break;
		case SOUND_FRONTEND_FAIL:
			m_sQueueSample.m_nSampleIndex = SFX_FE_ERROR_LEFT;
			frontendBank = true;
			stereo = true;
			break;
		case SOUND_FRONTEND_NO_RADIO:
		case SOUND_FRONTEND_RADIO_CHANGE:
			m_sQueueSample.m_nSampleIndex = SFX_RADIO_CLICK;
			break;
		//case SOUND_HUD_SOUND:
		//	m_sQueueSample.m_nSampleIndex = SFX_INFO;
		//	break;
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
		/*} else if (sample == SFX_PICKUP_1_LEFT) {
			if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_afVolume[i] == 1.0f)
				m_sQueueSample.m_nFrequency = 32000;
			else
				m_sQueueSample.m_nFrequency = 48000;*/
		} else {
			m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
		}
		m_sQueueSample.m_nVolume = 110;
		m_sQueueSample.m_nCounter = iSound++;
		m_sQueueSample.m_nLoopCount = 1;
		m_sQueueSample.m_bReleasingSoundFlag = true;
		m_sQueueSample.m_nBankIndex = frontendBank ? SAMPLEBANK_FRONTEND : SAMPLEBANK_MAIN;
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

/*void
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
						m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
}*/

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
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_nFrequency = SampleManager.GetSampleBaseFrequency(SFX_ROCKET_FLY);
				m_sQueueSample.m_nReleasingVolumeModificator = 3;
				break;
			case WEAPONTYPE_MOLOTOV:
				emittingVol = molotovVolume;
				m_sQueueSample.m_fSoundIntensity = molotovIntensity;
				m_sQueueSample.m_nSampleIndex = SFX_PED_ON_FIRE;
				m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
	eGarageState state;
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

					m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
						m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
#ifdef GTA_BRIDGE
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_EXTRAS;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
			m_sQueueSample.m_nBankIndex = SAMPLEBANK_MAIN;
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
#endif

#pragma region MISSION_AUDIO
bool g_bMissionAudioLoadFailed[MISSION_AUDIO_SLOTS];

struct MissionAudioData {
	const char *m_pName;
	int32 m_nId;
};


const MissionAudioData MissionAudioNameSfxAssoc[] = {
	{"mobring", STREAMED_SOUND_MISSION_MOBR1},    {"pagring", STREAMED_SOUND_MISSION_PAGER},    {"carrev", STREAMED_SOUND_MISSION_CARREV},
	{"bikerev", STREAMED_SOUND_MISSION_BIKEREV},  {"liftop", STREAMED_SOUND_MISSION_LIFTOP},    {"liftcl", STREAMED_SOUND_MISSION_LIFTCL},
	{"liftrun", STREAMED_SOUND_MISSION_LIFTRUN},  {"liftbel", STREAMED_SOUND_MISSION_LIFTBEL},  {"inlift", STREAMED_SOUND_MISSION_INLIFT},
	{"caml", STREAMED_SOUND_MISSION_CAMERAL},     {"camr", STREAMED_SOUND_MISSION_CAMERAR},     {"cheer1", STREAMED_SOUND_MISSION_CHEER1},
	{"cheer2", STREAMED_SOUND_MISSION_CHEER2},    {"cheer3", STREAMED_SOUND_MISSION_CHEER3},    {"cheer4", STREAMED_SOUND_MISSION_CHEER4},
	{"ooh1", STREAMED_SOUND_MISSION_OOH1},        {"ooh2", STREAMED_SOUND_MISSION_OOH2},        {"race1", STREAMED_SOUND_MISSION_RACE1},
	{"race2", STREAMED_SOUND_MISSION_RACE2},      {"race3", STREAMED_SOUND_MISSION_RACE3},      {"race4", STREAMED_SOUND_MISSION_RACE4},
	{"race5", STREAMED_SOUND_MISSION_RACE5},      {"race6", STREAMED_SOUND_MISSION_RACE6},      {"race7", STREAMED_SOUND_MISSION_RACE7},
	{"race8", STREAMED_SOUND_MISSION_RACE8},      {"race9", STREAMED_SOUND_MISSION_RACE9},      {"race10", STREAMED_SOUND_MISSION_RACE10},
	{"race11", STREAMED_SOUND_MISSION_RACE11},    {"race12", STREAMED_SOUND_MISSION_RACE12},    {"race13", STREAMED_SOUND_MISSION_RACE13},
	{"race14", STREAMED_SOUND_MISSION_RACE14},    {"race15", STREAMED_SOUND_MISSION_RACE15},    {"hot1", STREAMED_SOUND_MISSION_HOT1},
	{"hot2", STREAMED_SOUND_MISSION_HOT2},        {"hot3", STREAMED_SOUND_MISSION_HOT3},        {"hot4", STREAMED_SOUND_MISSION_HOT4},
	{"hot5", STREAMED_SOUND_MISSION_HOT5},        {"hot6", STREAMED_SOUND_MISSION_HOT6},        {"hot7", STREAMED_SOUND_MISSION_HOT7},
	{"hot8", STREAMED_SOUND_MISSION_HOT8},        {"hot9", STREAMED_SOUND_MISSION_HOT9},        {"hot10", STREAMED_SOUND_MISSION_HOT10},
	{"hot11", STREAMED_SOUND_MISSION_HOT11},      {"hot12", STREAMED_SOUND_MISSION_HOT12},      {"hot13", STREAMED_SOUND_MISSION_HOT13},
	{"hot14", STREAMED_SOUND_MISSION_HOT14},      {"hot15", STREAMED_SOUND_MISSION_HOT15},      {"lanstp1", STREAMED_SOUND_MISSION_LANSTP1},
	{"lanstp2", STREAMED_SOUND_MISSION_LANSTP2},  {"lanamu1", STREAMED_SOUND_MISSION_LANAMU1},  {"lanamu2", STREAMED_SOUND_MISSION_LANAMU2},
	{"airhrnl", STREAMED_SOUND_MISSION_AIRHORNL}, {"airhrnr", STREAMED_SOUND_MISSION_AIRHORNR}, {"sniper", STREAMED_SOUND_MISSION_SNIPSCRL},
	{"snipsh", STREAMED_SOUND_MISSION_SNIPSHORT}, {"bloroof", STREAMED_SOUND_MISSION_BLOWROOF}, {"sfx_01", STREAMED_SOUND_MISSION_SFX_01},
	{"sfx_02", STREAMED_SOUND_MISSION_SFX_02},    {"LAW1_1", STREAMED_SOUND_MISSION_LAW1_1},    {"LAW1_2", STREAMED_SOUND_MISSION_LAW1_2},
	{"LAW1_3", STREAMED_SOUND_MISSION_LAW1_3},    {"LAW1_4", STREAMED_SOUND_MISSION_LAW1_4},    {"LAW1_5", STREAMED_SOUND_MISSION_LAW1_5},
	{"LAW1_6", STREAMED_SOUND_MISSION_LAW1_6},    {"LAW1_7", STREAMED_SOUND_MISSION_LAW1_7},    {"LAW1_8", STREAMED_SOUND_MISSION_LAW1_8},
	{"LAW1_9", STREAMED_SOUND_MISSION_LAW1_9},    {"LAW1_10", STREAMED_SOUND_MISSION_LAW1_10},  {"LAW2_1", STREAMED_SOUND_MISSION_LAW2_1},
	{"LAW2_2", STREAMED_SOUND_MISSION_LAW2_2},    {"LAW2_3", STREAMED_SOUND_MISSION_LAW2_3},    {"LAW2_4", STREAMED_SOUND_MISSION_LAW2_4},
	{"LAW2_5", STREAMED_SOUND_MISSION_LAW2_5},    {"LAW2_6", STREAMED_SOUND_MISSION_LAW2_6},    {"LAW2_7", STREAMED_SOUND_MISSION_LAW2_7},
	{"LAW2_8", STREAMED_SOUND_MISSION_LAW2_8},    {"LAW2_9", STREAMED_SOUND_MISSION_LAW2_9},    {"LAW2_10", STREAMED_SOUND_MISSION_LAW2_10},
	{"LAW3_1", STREAMED_SOUND_MISSION_LAW3_1},    {"LAW3_2", STREAMED_SOUND_MISSION_LAW3_2},    {"LAW3_3", STREAMED_SOUND_MISSION_LAW3_3},
	{"LAW3_4", STREAMED_SOUND_MISSION_LAW3_4},    {"LAW3_5", STREAMED_SOUND_MISSION_LAW3_5},    {"LAW3_6", STREAMED_SOUND_MISSION_LAW3_6},
	{"LAW3_10", STREAMED_SOUND_MISSION_LAW3_10},  {"LAW3_11", STREAMED_SOUND_MISSION_LAW3_11},  {"LAW3_12", STREAMED_SOUND_MISSION_LAW3_12},
	{"LAW3_13", STREAMED_SOUND_MISSION_LAW3_13},  {"LAW3_14", STREAMED_SOUND_MISSION_LAW3_14},  {"LAW3_16", STREAMED_SOUND_MISSION_LAW3_16},
	{"LAW3_17", STREAMED_SOUND_MISSION_LAW3_17},  {"LAW3_18", STREAMED_SOUND_MISSION_LAW3_18},  {"LAW3_19", STREAMED_SOUND_MISSION_LAW3_19},
	{"LAW3_20", STREAMED_SOUND_MISSION_LAW3_20},  {"LAW3_21", STREAMED_SOUND_MISSION_LAW3_21},  {"LAW3_22", STREAMED_SOUND_MISSION_LAW3_22},
	{"LAW3_23", STREAMED_SOUND_MISSION_LAW3_23},  {"LAW3_24", STREAMED_SOUND_MISSION_LAW3_24},  {"LAW3_25", STREAMED_SOUND_MISSION_LAW3_25},
	{"LAW4_1a", STREAMED_SOUND_MISSION_LAW4_1A},  {"LAW4_1b", STREAMED_SOUND_MISSION_LAW4_1B},  {"LAW4_1c", STREAMED_SOUND_MISSION_LAW4_1C},
	{"LAW4_1d", STREAMED_SOUND_MISSION_LAW4_1D},  {"LAW4_10", STREAMED_SOUND_MISSION_LAW4_10},  {"LAW4_3", STREAMED_SOUND_MISSION_LAW4_3},
	{"LAW4_4", STREAMED_SOUND_MISSION_LAW4_4},    {"LAW4_5", STREAMED_SOUND_MISSION_LAW4_5},    {"LAW4_6", STREAMED_SOUND_MISSION_LAW4_6},
	{"LAW4_7", STREAMED_SOUND_MISSION_LAW4_7},    {"LAW4_8", STREAMED_SOUND_MISSION_LAW4_8},    {"LAW4_9", STREAMED_SOUND_MISSION_LAW4_9},
	{"COL1_1", STREAMED_SOUND_MISSION_COL1_1},    {"COL1_2", STREAMED_SOUND_MISSION_COL1_2},    {"COL1_3", STREAMED_SOUND_MISSION_COL1_3},
	{"COL1_4", STREAMED_SOUND_MISSION_COL1_4},    {"COL1_5", STREAMED_SOUND_MISSION_COL1_5},    {"COL1_6", STREAMED_SOUND_MISSION_COL1_6},
	{"COL1_7", STREAMED_SOUND_MISSION_COL1_7},    {"COL1_8", STREAMED_SOUND_MISSION_COL1_8},    {"COL2_1", STREAMED_SOUND_MISSION_COL2_1},
	{"COL2_2", STREAMED_SOUND_MISSION_COL2_2},    {"COL2_3", STREAMED_SOUND_MISSION_COL2_3},    {"COL2_4", STREAMED_SOUND_MISSION_COL2_4},
	{"COL2_5", STREAMED_SOUND_MISSION_COL2_5},    {"COL2_6a", STREAMED_SOUND_MISSION_COL2_6A},  {"COL2_7", STREAMED_SOUND_MISSION_COL2_7},
	{"COL2_8", STREAMED_SOUND_MISSION_COL2_8},    {"COL2_9", STREAMED_SOUND_MISSION_COL2_9},    {"COL2_10", STREAMED_SOUND_MISSION_COL2_10},
	{"COL2_11", STREAMED_SOUND_MISSION_COL2_11},  {"COL2_12", STREAMED_SOUND_MISSION_COL2_12},  {"COL2_13", STREAMED_SOUND_MISSION_COL2_13},
	{"COL2_14", STREAMED_SOUND_MISSION_COL2_14},  {"COL2_15", STREAMED_SOUND_MISSION_COL2_15},  {"COL2_16", STREAMED_SOUND_MISSION_COL2_16},
	{"COL3_1", STREAMED_SOUND_MISSION_COL3_1},    {"COL3_2", STREAMED_SOUND_MISSION_COL3_2},    {"COL3_2a", STREAMED_SOUND_MISSION_COL3_2A},
	{"COL3_2b", STREAMED_SOUND_MISSION_COL3_2B},  {"COL3_3", STREAMED_SOUND_MISSION_COL3_3},    {"COL3_4", STREAMED_SOUND_MISSION_COL3_4},
	{"COL3_5", STREAMED_SOUND_MISSION_COL3_5},    {"COL3_6", STREAMED_SOUND_MISSION_COL3_6},    {"COL3_7", STREAMED_SOUND_MISSION_COL3_7},
	{"COL3_8", STREAMED_SOUND_MISSION_COL3_8},    {"COL3_9", STREAMED_SOUND_MISSION_COL3_9},    {"COL3_10", STREAMED_SOUND_MISSION_COL3_10},
	{"COL3_11", STREAMED_SOUND_MISSION_COL3_11},  {"COL3_12", STREAMED_SOUND_MISSION_COL3_12},  {"COL3_13", STREAMED_SOUND_MISSION_COL3_13},
	{"COL3_14", STREAMED_SOUND_MISSION_COL3_14},  {"COL3_15", STREAMED_SOUND_MISSION_COL3_15},  {"COL3_16", STREAMED_SOUND_MISSION_COL3_16},
	{"COL3_17", STREAMED_SOUND_MISSION_COL3_17},  {"COL3_18", STREAMED_SOUND_MISSION_COL3_18},  {"COL3_19", STREAMED_SOUND_MISSION_COL3_19},
	{"COL3_20", STREAMED_SOUND_MISSION_COL3_20},  {"COL3_21", STREAMED_SOUND_MISSION_COL3_21},  {"COL3_23", STREAMED_SOUND_MISSION_COL3_23},
	{"COL3_24", STREAMED_SOUND_MISSION_COL3_24},  {"COL3_25", STREAMED_SOUND_MISSION_COL3_25},  {"COL4_1", STREAMED_SOUND_MISSION_COL4_1},
	{"COL4_2", STREAMED_SOUND_MISSION_COL4_2},    {"COL4_3", STREAMED_SOUND_MISSION_COL4_3},    {"COL4_4", STREAMED_SOUND_MISSION_COL4_4},
	{"COL4_5", STREAMED_SOUND_MISSION_COL4_5},    {"COL4_6", STREAMED_SOUND_MISSION_COL4_6},    {"COL4_7", STREAMED_SOUND_MISSION_COL4_7},
	{"COL4_8", STREAMED_SOUND_MISSION_COL4_8},    {"COL4_9", STREAMED_SOUND_MISSION_COL4_9},    {"COL4_10", STREAMED_SOUND_MISSION_COL4_10},
	{"COL4_11", STREAMED_SOUND_MISSION_COL4_11},  {"COL4_12", STREAMED_SOUND_MISSION_COL4_12},  {"COL4_13", STREAMED_SOUND_MISSION_COL4_13},
	{"COL4_14", STREAMED_SOUND_MISSION_COL4_14},  {"COL4_15", STREAMED_SOUND_MISSION_COL4_15},  {"COL4_16", STREAMED_SOUND_MISSION_COL4_16},
	{"COL4_17", STREAMED_SOUND_MISSION_COL4_17},  {"COL4_18", STREAMED_SOUND_MISSION_COL4_18},  {"COL4_19", STREAMED_SOUND_MISSION_COL4_19},
	{"COL4_20", STREAMED_SOUND_MISSION_COL4_20},  {"COL4_21", STREAMED_SOUND_MISSION_COL4_21},  {"COL4_22", STREAMED_SOUND_MISSION_COL4_22},
	{"COL4_23", STREAMED_SOUND_MISSION_COL4_23},  {"COL4_24", STREAMED_SOUND_MISSION_COL4_24},  {"COL4_25", STREAMED_SOUND_MISSION_COL4_25},
	{"COL4_26", STREAMED_SOUND_MISSION_COL4_26},  {"COL5_1", STREAMED_SOUND_MISSION_COL5_1},    {"COL5_2", STREAMED_SOUND_MISSION_COL5_2},
	{"COL5_3", STREAMED_SOUND_MISSION_COL5_3},    {"COL5_4", STREAMED_SOUND_MISSION_COL5_4},    {"COL5_5", STREAMED_SOUND_MISSION_COL5_5},
	{"COL5_6", STREAMED_SOUND_MISSION_COL5_6},    {"COL5_7", STREAMED_SOUND_MISSION_COL5_7},    {"COL5_8", STREAMED_SOUND_MISSION_COL5_8},
	{"COL5_9", STREAMED_SOUND_MISSION_COL5_9},    {"COL5_10", STREAMED_SOUND_MISSION_COL5_10},  {"COL5_11", STREAMED_SOUND_MISSION_COL5_11},
	{"COL5_12", STREAMED_SOUND_MISSION_COL5_12},  {"COL5_13", STREAMED_SOUND_MISSION_COL5_13},  {"COL5_14", STREAMED_SOUND_MISSION_COL5_14},
	{"COL5_15", STREAMED_SOUND_MISSION_COL5_15},  {"COL5_16", STREAMED_SOUND_MISSION_COL5_16},  {"COL5_17", STREAMED_SOUND_MISSION_COL5_17},
	{"COL5_18", STREAMED_SOUND_MISSION_COL5_18},  {"COL5_19", STREAMED_SOUND_MISSION_COL5_19},  {"COL5_20", STREAMED_SOUND_MISSION_COL5_20},
	{"COL5_21", STREAMED_SOUND_MISSION_COL5_21},  {"COL5_22", STREAMED_SOUND_MISSION_COL5_22},  {"COK1_1", STREAMED_SOUND_MISSION_COK1_1},
	{"COK1_2", STREAMED_SOUND_MISSION_COK1_2},    {"COK1_3", STREAMED_SOUND_MISSION_COK1_3},    {"COK1_4", STREAMED_SOUND_MISSION_COK1_4},
	{"COK1_5", STREAMED_SOUND_MISSION_COK1_5},    {"COK1_6", STREAMED_SOUND_MISSION_COK1_6},    {"COK2_1", STREAMED_SOUND_MISSION_COK2_1},
	{"COK2_2", STREAMED_SOUND_MISSION_COK2_2},    {"COK2_3", STREAMED_SOUND_MISSION_COK2_3},    {"COK2_4", STREAMED_SOUND_MISSION_COK2_4},
	{"COK2_5", STREAMED_SOUND_MISSION_COK2_5},    {"COK2_6", STREAMED_SOUND_MISSION_COK2_6},    {"COK2_7a", STREAMED_SOUND_MISSION_COK2_7A},
	{"COK2_7b", STREAMED_SOUND_MISSION_COK2_7B},  {"COK2_7c", STREAMED_SOUND_MISSION_COK2_7C},  {"COK2_8a", STREAMED_SOUND_MISSION_COK2_8A},
	{"COK2_8b", STREAMED_SOUND_MISSION_COK2_8B},  {"COK2_8c", STREAMED_SOUND_MISSION_COK2_8C},  {"COK2_8d", STREAMED_SOUND_MISSION_COK2_8D},
	{"COK2_9", STREAMED_SOUND_MISSION_COK2_9},    {"COK210a", STREAMED_SOUND_MISSION_COK210A},  {"COK210b", STREAMED_SOUND_MISSION_COK210B},
	{"COK210c", STREAMED_SOUND_MISSION_COK210C},  {"COK212a", STREAMED_SOUND_MISSION_COK212A},  {"COK212b", STREAMED_SOUND_MISSION_COK212B},
	{"COK2_13", STREAMED_SOUND_MISSION_COK2_13},  {"COK2_14", STREAMED_SOUND_MISSION_COK2_14},  {"COK2_15", STREAMED_SOUND_MISSION_COK2_15},
	{"COK2_16", STREAMED_SOUND_MISSION_COK2_16},  {"COK2_20", STREAMED_SOUND_MISSION_COK2_20},  {"COK2_21", STREAMED_SOUND_MISSION_COK2_21},
	{"COK2_22", STREAMED_SOUND_MISSION_COK2_22},  {"COK3_1", STREAMED_SOUND_MISSION_COK3_1},    {"COK3_2", STREAMED_SOUND_MISSION_COK3_2},
	{"COK3_3", STREAMED_SOUND_MISSION_COK3_3},    {"COK3_4", STREAMED_SOUND_MISSION_COK3_4},    {"COK4_1", STREAMED_SOUND_MISSION_COK4_1},
	{"COK4_2", STREAMED_SOUND_MISSION_COK4_2},    {"COK4_3", STREAMED_SOUND_MISSION_COK4_3},    {"COK4_4", STREAMED_SOUND_MISSION_COK4_4},
	{"COK4_5", STREAMED_SOUND_MISSION_COK4_5},    {"COK4_6", STREAMED_SOUND_MISSION_COK4_6},    {"COK4_7", STREAMED_SOUND_MISSION_COK4_7},
	{"COK4_8", STREAMED_SOUND_MISSION_COK4_8},    {"COK4_9", STREAMED_SOUND_MISSION_COK4_9},    {"COK4_9A", STREAMED_SOUND_MISSION_COK4_9A},
	{"COK4_10", STREAMED_SOUND_MISSION_COK4_10},  {"COK4_11", STREAMED_SOUND_MISSION_COK4_11},  {"COK4_12", STREAMED_SOUND_MISSION_COK4_12},
	{"COK4_13", STREAMED_SOUND_MISSION_COK4_13},  {"COK4_14", STREAMED_SOUND_MISSION_COK4_14},  {"COK4_15", STREAMED_SOUND_MISSION_COK4_15},
	{"COK4_16", STREAMED_SOUND_MISSION_COK4_16},  {"COK4_17", STREAMED_SOUND_MISSION_COK4_17},  {"COK4_18", STREAMED_SOUND_MISSION_COK4_18},
	{"COK4_19", STREAMED_SOUND_MISSION_COK4_19},  {"COK4_20", STREAMED_SOUND_MISSION_COK4_20},  {"COK4_21", STREAMED_SOUND_MISSION_COK4_21},
	{"COK4_22", STREAMED_SOUND_MISSION_COK4_22},  {"COK4_23", STREAMED_SOUND_MISSION_COK4_23},  {"COK4_24", STREAMED_SOUND_MISSION_COK4_24},
	{"COK4_25", STREAMED_SOUND_MISSION_COK4_25},  {"COK4_26", STREAMED_SOUND_MISSION_COK4_26},  {"COK4_27", STREAMED_SOUND_MISSION_COK4_27},
	{"RESC_1", STREAMED_SOUND_MISSION_RESC_1},    {"RESC_2", STREAMED_SOUND_MISSION_RESC_2},    {"RESC_3", STREAMED_SOUND_MISSION_RESC_3},
	{"RESC_4", STREAMED_SOUND_MISSION_RESC_4},    {"RESC_5", STREAMED_SOUND_MISSION_RESC_5},    {"RESC_6", STREAMED_SOUND_MISSION_RESC_6},
	{"RESC_7", STREAMED_SOUND_MISSION_RESC_7},    {"RESC_8", STREAMED_SOUND_MISSION_RESC_8},    {"RESC_9", STREAMED_SOUND_MISSION_RESC_9},
	{"RESC_10", STREAMED_SOUND_MISSION_RESC_10},  {"ASS_1", STREAMED_SOUND_MISSION_ASS_1},      {"ASS_2", STREAMED_SOUND_MISSION_ASS_2},
	{"ASS_3", STREAMED_SOUND_MISSION_ASS_3},      {"ASS_4", STREAMED_SOUND_MISSION_ASS_4},      {"ASS_5", STREAMED_SOUND_MISSION_ASS_5},
	{"ASS_6", STREAMED_SOUND_MISSION_ASS_6},      {"ASS_7", STREAMED_SOUND_MISSION_ASS_7},      {"ASS_8", STREAMED_SOUND_MISSION_ASS_8},
	{"ASS_9", STREAMED_SOUND_MISSION_ASS_9},      {"ASS_10", STREAMED_SOUND_MISSION_ASS_10},    {"ASS_11", STREAMED_SOUND_MISSION_ASS_11},
	{"ASS_12", STREAMED_SOUND_MISSION_ASS_12},    {"ASS_13", STREAMED_SOUND_MISSION_ASS_13},    {"ASS_14", STREAMED_SOUND_MISSION_ASS_14},
	{"BUD1_1", STREAMED_SOUND_MISSION_BUD1_1},    {"BUD1_2", STREAMED_SOUND_MISSION_BUD1_2},    {"BUD1_3", STREAMED_SOUND_MISSION_BUD1_3},
	{"BUD1_4", STREAMED_SOUND_MISSION_BUD1_4},    {"BUD1_5", STREAMED_SOUND_MISSION_BUD1_5},    {"BUD1_9", STREAMED_SOUND_MISSION_BUD1_9},
	{"BUD1_10", STREAMED_SOUND_MISSION_BUD1_10},  {"BUD2_1", STREAMED_SOUND_MISSION_BUD2_1},    {"BUD2_2", STREAMED_SOUND_MISSION_BUD2_2},
	{"BUD2_3", STREAMED_SOUND_MISSION_BUD2_3},    {"BUD2_4", STREAMED_SOUND_MISSION_BUD2_4},    {"BUD2_5", STREAMED_SOUND_MISSION_BUD2_5},
	{"BUD2_6", STREAMED_SOUND_MISSION_BUD2_6},    {"BUD2_7", STREAMED_SOUND_MISSION_BUD2_7},    {"BUD3_1a", STREAMED_SOUND_MISSION_BUD3_1A},
	{"BUD3_1b", STREAMED_SOUND_MISSION_BUD3_1B},  {"BUD3_1", STREAMED_SOUND_MISSION_BUD3_1},    {"BUD3_2", STREAMED_SOUND_MISSION_BUD3_2},
	{"BUD3_3", STREAMED_SOUND_MISSION_BUD3_3},    {"BUD3_4", STREAMED_SOUND_MISSION_BUD3_4},    {"BUD3_1c", STREAMED_SOUND_MISSION_BUD3_1C},
	{"BUD3_5", STREAMED_SOUND_MISSION_BUD3_5},    {"BUD3_6", STREAMED_SOUND_MISSION_BUD3_6},    {"BUD3_7", STREAMED_SOUND_MISSION_BUD3_7},
	{"BUD3_8a", STREAMED_SOUND_MISSION_BUD3_8A},  {"BUD3_8b", STREAMED_SOUND_MISSION_BUD3_8B},  {"BUD3_8c", STREAMED_SOUND_MISSION_BUD3_8C},
	{"BUD3_9a", STREAMED_SOUND_MISSION_BUD3_9A},  {"BUD3_9b", STREAMED_SOUND_MISSION_BUD3_9B},  {"BUD3_9c", STREAMED_SOUND_MISSION_BUD3_9C},
	{"CAP1_2", STREAMED_SOUND_MISSION_CAP1_2},    {"CAP1_3", STREAMED_SOUND_MISSION_CAP1_3},    {"CAP1_4", STREAMED_SOUND_MISSION_CAP1_4},
	{"CAP1_5", STREAMED_SOUND_MISSION_CAP1_5},    {"CAP1_6", STREAMED_SOUND_MISSION_CAP1_6},    {"CAP1_7", STREAMED_SOUND_MISSION_CAP1_7},
	{"CAP1_8", STREAMED_SOUND_MISSION_CAP1_8},    {"CAP1_9", STREAMED_SOUND_MISSION_CAP1_9},    {"CAP1_10", STREAMED_SOUND_MISSION_CAP1_10},
	{"CAP1_11", STREAMED_SOUND_MISSION_CAP1_11},  {"CAP1_12", STREAMED_SOUND_MISSION_CAP1_12},  {"FINKILL", STREAMED_SOUND_MISSION_FINKILL},
	{"FIN_1a", STREAMED_SOUND_MISSION_FIN_1A},    {"FIN_1b", STREAMED_SOUND_MISSION_FIN_1B},    {"FIN_1c", STREAMED_SOUND_MISSION_FIN_1C},
	{"FIN_2b", STREAMED_SOUND_MISSION_FIN_2B},    {"FIN_2c", STREAMED_SOUND_MISSION_FIN_2C},    {"FIN_3", STREAMED_SOUND_MISSION_FIN_3},
	{"FIN_4", STREAMED_SOUND_MISSION_FIN_4},      {"FIN_5", STREAMED_SOUND_MISSION_FIN_5},      {"FIN_6", STREAMED_SOUND_MISSION_FIN_6},
	{"FIN_10", STREAMED_SOUND_MISSION_FIN_10},    {"FIN_11a", STREAMED_SOUND_MISSION_FIN_11A},  {"FIN_11b", STREAMED_SOUND_MISSION_FIN_11B},
	{"FIN_12a", STREAMED_SOUND_MISSION_FIN_12A},  {"FIN_12b", STREAMED_SOUND_MISSION_FIN_12B},  {"FIN_12c", STREAMED_SOUND_MISSION_FIN_12C},
	{"FIN_13", STREAMED_SOUND_MISSION_FIN_13},    {"BNK1_1", STREAMED_SOUND_MISSION_BNK1_1},    {"BNK1_2", STREAMED_SOUND_MISSION_BNK1_2},
	{"BNK1_3", STREAMED_SOUND_MISSION_BNK1_3},    {"BNK1_4", STREAMED_SOUND_MISSION_BNK1_4},    {"BNK1_5", STREAMED_SOUND_MISSION_BNK1_5},
	{"BNK1_6", STREAMED_SOUND_MISSION_BNK1_6},    {"BNK1_7", STREAMED_SOUND_MISSION_BNK1_7},    {"BNK1_8", STREAMED_SOUND_MISSION_BNK1_8},
	{"BNK1_10", STREAMED_SOUND_MISSION_BNK1_10},  {"BNK1_11", STREAMED_SOUND_MISSION_BNK1_11},  {"BNK1_12", STREAMED_SOUND_MISSION_BNK1_12},
	{"BNK1_13", STREAMED_SOUND_MISSION_BNK1_13},  {"BNK1_14", STREAMED_SOUND_MISSION_BNK1_14},  {"BNK2_1", STREAMED_SOUND_MISSION_BNK2_1},
	{"BNK2_2", STREAMED_SOUND_MISSION_BNK2_2},    {"BNK2_3", STREAMED_SOUND_MISSION_BNK2_3},    {"BNK2_4", STREAMED_SOUND_MISSION_BNK2_4},
	{"BNK2_5", STREAMED_SOUND_MISSION_BNK2_5},    {"BNK2_6", STREAMED_SOUND_MISSION_BNK2_6},    {"BNK2_7", STREAMED_SOUND_MISSION_BNK2_7},
	{"BNK2_8", STREAMED_SOUND_MISSION_BNK2_8},    {"BNK2_9", STREAMED_SOUND_MISSION_BNK2_9},    {"BNK3_1", STREAMED_SOUND_MISSION_BNK3_1},
	{"BNK3_2", STREAMED_SOUND_MISSION_BNK3_2},    {"BNK3_3a", STREAMED_SOUND_MISSION_BNK3_3A},  {"BNK3_3b", STREAMED_SOUND_MISSION_BNK3_3B},
	{"BNK3_3c", STREAMED_SOUND_MISSION_BNK3_3C},  {"BNK3_4a", STREAMED_SOUND_MISSION_BNK3_4A},  {"BNK3_4b", STREAMED_SOUND_MISSION_BNK3_4B},
	{"BNK3_4c", STREAMED_SOUND_MISSION_BNK3_4C},  {"BNK4_1", STREAMED_SOUND_MISSION_BNK4_1},    {"BNK4_2", STREAMED_SOUND_MISSION_BNK4_2},
	{"BNK4_3A", STREAMED_SOUND_MISSION_BNK4_3A},  {"BNK4_3B", STREAMED_SOUND_MISSION_BNK4_3B},  {"BNK4_3C", STREAMED_SOUND_MISSION_BNK4_3C},
	{"BNK4_3D", STREAMED_SOUND_MISSION_BNK4_3D},  {"BNK4_3E", STREAMED_SOUND_MISSION_BNK4_3E},  {"BNK4_3F", STREAMED_SOUND_MISSION_BNK4_3F},
	{"BNK4_3G", STREAMED_SOUND_MISSION_BNK4_3G},  {"BNK4_3H", STREAMED_SOUND_MISSION_BNK4_3H},  {"BNK4_3I", STREAMED_SOUND_MISSION_BNK4_3I},
	{"BNK4_3J", STREAMED_SOUND_MISSION_BNK4_3J},  {"BNK4_3K", STREAMED_SOUND_MISSION_BNK4_3K},  {"BNK4_3M", STREAMED_SOUND_MISSION_BNK4_3M},
	{"BNK4_3O", STREAMED_SOUND_MISSION_BNK4_3O},  {"BNK4_3P", STREAMED_SOUND_MISSION_BNK4_3P},  {"BNK4_3Q", STREAMED_SOUND_MISSION_BNK4_3Q},
	{"BNK4_3R", STREAMED_SOUND_MISSION_BNK4_3R},  {"BNK4_3S", STREAMED_SOUND_MISSION_BNK4_3S},  {"BNK4_3T", STREAMED_SOUND_MISSION_BNK4_3T},
	{"BNK4_3U", STREAMED_SOUND_MISSION_BNK4_3U},  {"BNK4_3V", STREAMED_SOUND_MISSION_BNK4_3V},  {"BNK4_4a", STREAMED_SOUND_MISSION_BNK4_4A},
	{"BNK4_4b", STREAMED_SOUND_MISSION_BNK4_4B},  {"BNK4_5", STREAMED_SOUND_MISSION_BNK4_5},    {"BNK4_6", STREAMED_SOUND_MISSION_BNK4_6},
	{"BNK4_7", STREAMED_SOUND_MISSION_BNK4_7},    {"BNK4_8", STREAMED_SOUND_MISSION_BNK4_8},    {"BNK4_9", STREAMED_SOUND_MISSION_BNK4_9},
	{"BNK4_10", STREAMED_SOUND_MISSION_BNK4_10},  {"BNK4_11", STREAMED_SOUND_MISSION_BNK4_11},  {"BK4_12a", STREAMED_SOUND_MISSION_BK4_12A},
	{"BK4_12b", STREAMED_SOUND_MISSION_BK4_12B},  {"BK4_12c", STREAMED_SOUND_MISSION_BK4_12C},  {"BNK4_13", STREAMED_SOUND_MISSION_BNK4_13},
	{"BK4_14a", STREAMED_SOUND_MISSION_BK4_14A},  {"BK4_14b", STREAMED_SOUND_MISSION_BK4_14B},  {"BNK4_15", STREAMED_SOUND_MISSION_BNK4_15},
	{"BNK4_16", STREAMED_SOUND_MISSION_BNK4_16},  {"BNK4_17", STREAMED_SOUND_MISSION_BNK4_17},  {"BNK4_18", STREAMED_SOUND_MISSION_BNK4_18},
	{"BK4_19a", STREAMED_SOUND_MISSION_BK4_19A},  {"BK4_19b", STREAMED_SOUND_MISSION_BK4_19B},  {"BK4_20a", STREAMED_SOUND_MISSION_BK4_20A},
	{"BK4_20b", STREAMED_SOUND_MISSION_BK4_20B},  {"BNK4_21", STREAMED_SOUND_MISSION_BNK4_21},  {"BNK422a", STREAMED_SOUND_MISSION_BNK422A},
	{"BNK422b", STREAMED_SOUND_MISSION_BNK422B},  {"BK4_23a", STREAMED_SOUND_MISSION_BK4_23A},  {"BK4_23b", STREAMED_SOUND_MISSION_BK4_23B},
	{"BK4_23c", STREAMED_SOUND_MISSION_BK4_23C},  {"BK4_23d", STREAMED_SOUND_MISSION_BK4_23D},  {"BK4_24a", STREAMED_SOUND_MISSION_BK4_24A},
	{"BK4_24b", STREAMED_SOUND_MISSION_BK4_24B},  {"BNK4_25", STREAMED_SOUND_MISSION_BNK4_25},  {"BNK4_26", STREAMED_SOUND_MISSION_BNK4_26},
	{"BNK4_27", STREAMED_SOUND_MISSION_BNK4_27},  {"BNK4_28", STREAMED_SOUND_MISSION_BNK4_28},  {"BNK4_29", STREAMED_SOUND_MISSION_BNK4_29},
	{"BNK4_30", STREAMED_SOUND_MISSION_BNK4_30},  {"BK4_31a", STREAMED_SOUND_MISSION_BK4_31A},  {"BK4_31b", STREAMED_SOUND_MISSION_BK4_31B},
	{"BNK4_32", STREAMED_SOUND_MISSION_BNK4_32},  {"BK4_34a", STREAMED_SOUND_MISSION_BK4_34A},  {"BK4_34b", STREAMED_SOUND_MISSION_BK4_34B},
	{"BK4_35a", STREAMED_SOUND_MISSION_BK4_35A},  {"BK4_35b", STREAMED_SOUND_MISSION_BK4_35B},  {"BNK4_36", STREAMED_SOUND_MISSION_BNK4_36},
	{"BNK4_37", STREAMED_SOUND_MISSION_BNK4_37},  {"BNK4_38", STREAMED_SOUND_MISSION_BNK4_38},  {"BNK_39", STREAMED_SOUND_MISSION_BNK4_39},
	{"BK4_40a", STREAMED_SOUND_MISSION_BK4_40A},  {"BK4_40b", STREAMED_SOUND_MISSION_BK4_40B},  {"BNK4_41", STREAMED_SOUND_MISSION_BNK4_41},
	{"BNK4_42", STREAMED_SOUND_MISSION_BNK4_42},  {"BNK4_43", STREAMED_SOUND_MISSION_BNK4_43},  {"BNK4_44", STREAMED_SOUND_MISSION_BNK4_44},
	{"BNK4_45", STREAMED_SOUND_MISSION_BNK4_45},  {"BNK4_46", STREAMED_SOUND_MISSION_BNK4_46},  {"BNK4_47", STREAMED_SOUND_MISSION_BNK4_47},
	{"BNK4_48", STREAMED_SOUND_MISSION_BNK4_48},  {"BNK4_49", STREAMED_SOUND_MISSION_BNK4_49},  {"BNK450A", STREAMED_SOUND_MISSION_BNK450A},
	{"BNK450B", STREAMED_SOUND_MISSION_BNK450B},  {"BNK4_51", STREAMED_SOUND_MISSION_BNK4_51},  {"BNK4_94", STREAMED_SOUND_MISSION_BNK4_94},
	{"BNK4_95", STREAMED_SOUND_MISSION_BNK4_95},  {"BNK4_96", STREAMED_SOUND_MISSION_BNK4_96},  {"BNK4_97", STREAMED_SOUND_MISSION_BNK4_97},
	{"BNK4_98", STREAMED_SOUND_MISSION_BNK4_98},  {"BNK4_99", STREAMED_SOUND_MISSION_BNK4_99},  {"CNT1_1", STREAMED_SOUND_MISSION_CNT1_1},
	{"CNT1_2", STREAMED_SOUND_MISSION_CNT1_2},    {"CNT1_3", STREAMED_SOUND_MISSION_CNT1_3},    {"CNT1_4", STREAMED_SOUND_MISSION_CNT1_4},
	{"CNT1_5", STREAMED_SOUND_MISSION_CNT1_5},    {"CNT2_1", STREAMED_SOUND_MISSION_CNT2_1},    {"CNT2_2", STREAMED_SOUND_MISSION_CNT2_2},
	{"CNT2_3", STREAMED_SOUND_MISSION_CNT2_3},    {"CNT2_4", STREAMED_SOUND_MISSION_CNT2_4},    {"PORN1_1", STREAMED_SOUND_MISSION_PORN1_1},
	{"PORN1_2", STREAMED_SOUND_MISSION_PORN1_2},  {"PORN1_3", STREAMED_SOUND_MISSION_PORN1_3},  {"PRN1_3A", STREAMED_SOUND_MISSION_PRN1_3A},
	{"PORN1_4", STREAMED_SOUND_MISSION_PORN1_4},  {"PORN1_5", STREAMED_SOUND_MISSION_PORN1_5},  {"PORN1_6", STREAMED_SOUND_MISSION_PORN1_6},
	{"PORN1_7", STREAMED_SOUND_MISSION_PORN1_7},  {"PORN1_8", STREAMED_SOUND_MISSION_PORN1_8},  {"PORN1_9", STREAMED_SOUND_MISSION_PORN1_9},
	{"PRN1_10", STREAMED_SOUND_MISSION_PRN1_10},  {"PRN1_11", STREAMED_SOUND_MISSION_PRN1_11},  {"PRN1_12", STREAMED_SOUND_MISSION_PRN1_12},
	{"PRN1_13", STREAMED_SOUND_MISSION_PRN1_13},  {"PRN1_14", STREAMED_SOUND_MISSION_PRN1_14},  {"PRN1_15", STREAMED_SOUND_MISSION_PRN1_15},
	{"PRN1_16", STREAMED_SOUND_MISSION_PRN1_16},  {"PRN1_17", STREAMED_SOUND_MISSION_PRN1_17},  {"PRN1_18", STREAMED_SOUND_MISSION_PRN1_18},
	{"PRN1_19", STREAMED_SOUND_MISSION_PRN1_19},  {"PRN1_20", STREAMED_SOUND_MISSION_PRN1_20},  {"PRN1_21", STREAMED_SOUND_MISSION_PRN1_21},
	{"PORN3_1", STREAMED_SOUND_MISSION_PORN3_1},  {"PORN3_2", STREAMED_SOUND_MISSION_PORN3_2},  {"PORN3_3", STREAMED_SOUND_MISSION_PORN3_3},
	{"PORN3_4", STREAMED_SOUND_MISSION_PORN3_4},  {"TAX1_1", STREAMED_SOUND_MISSION_TAX1_1},    {"TAX1_2", STREAMED_SOUND_MISSION_TAX1_2},
	{"TAX1_3", STREAMED_SOUND_MISSION_TAX1_3},    {"TAX1_4", STREAMED_SOUND_MISSION_TAX1_4},    {"TAX1_5", STREAMED_SOUND_MISSION_TAX1_5},
	{"TAX2_1", STREAMED_SOUND_MISSION_TAX2_1},    {"TAX2_2", STREAMED_SOUND_MISSION_TAX2_2},    {"TAX2_3", STREAMED_SOUND_MISSION_TAX2_3},
	{"TAX2_4", STREAMED_SOUND_MISSION_TAX2_4},    {"TAX2_5", STREAMED_SOUND_MISSION_TAX2_5},    {"TAX2_6", STREAMED_SOUND_MISSION_TAX2_6},
	{"TAX2_7", STREAMED_SOUND_MISSION_TAX2_7},    {"TAX3_1", STREAMED_SOUND_MISSION_TAX3_1},    {"TAX3_2", STREAMED_SOUND_MISSION_TAX3_2},
	{"TAX3_3", STREAMED_SOUND_MISSION_TAX3_3},    {"TAX3_4", STREAMED_SOUND_MISSION_TAX3_4},    {"TAX3_5", STREAMED_SOUND_MISSION_TAX3_5},
	{"TEX1_1", STREAMED_SOUND_MISSION_TEX1_1},    {"TEX1_2", STREAMED_SOUND_MISSION_TEX1_2},    {"TEX1_3", STREAMED_SOUND_MISSION_TEX1_3},
	{"TEX1_4", STREAMED_SOUND_MISSION_TEX1_4},    {"TEX1_5", STREAMED_SOUND_MISSION_TEX1_5},    {"TEX1_6", STREAMED_SOUND_MISSION_TEX1_6},
	{"TEX2_1", STREAMED_SOUND_MISSION_TEX2_1},    {"TEX3_1", STREAMED_SOUND_MISSION_TEX3_1},    {"TEX3_2", STREAMED_SOUND_MISSION_TEX3_2},
	{"TEX3_3", STREAMED_SOUND_MISSION_TEX3_3},    {"TEX3_4", STREAMED_SOUND_MISSION_TEX3_4},    {"TEX3_5", STREAMED_SOUND_MISSION_TEX3_5},
	{"TEX3_6", STREAMED_SOUND_MISSION_TEX3_6},    {"TEX3_7", STREAMED_SOUND_MISSION_TEX3_7},    {"TEX3_8", STREAMED_SOUND_MISSION_TEX3_8},
	{"PHIL1_2", STREAMED_SOUND_MISSION_PHIL1_2},  {"PHIL1_3", STREAMED_SOUND_MISSION_PHIL1_3},  {"PHIL2_1", STREAMED_SOUND_MISSION_PHIL2_1},
	{"PHIL2_2", STREAMED_SOUND_MISSION_PHIL2_2},  {"PHIL2_3", STREAMED_SOUND_MISSION_PHIL2_3},  {"PHIL2_4", STREAMED_SOUND_MISSION_PHIL2_4},
	{"PHIL2_5", STREAMED_SOUND_MISSION_PHIL2_5},  {"PHIL2_6", STREAMED_SOUND_MISSION_PHIL2_6},  {"PHIL2_7", STREAMED_SOUND_MISSION_PHIL2_7},
	{"PHIL2_8", STREAMED_SOUND_MISSION_PHIL2_8},  {"PHIL2_9", STREAMED_SOUND_MISSION_PHIL2_9},  {"PHIL210", STREAMED_SOUND_MISSION_PHIL210},
	{"PHIL211", STREAMED_SOUND_MISSION_PHIL211},  {"BIKE1_1", STREAMED_SOUND_MISSION_BIKE1_1},  {"BIKE1_2", STREAMED_SOUND_MISSION_BIKE1_2},
	{"BIKE1_3", STREAMED_SOUND_MISSION_BIKE1_3},  {"ROK1_1a", STREAMED_SOUND_MISSION_ROK1_1A},  {"ROK1_1b", STREAMED_SOUND_MISSION_ROK1_1B},
	{"ROK1_5", STREAMED_SOUND_MISSION_ROK1_5},    {"ROK1_6", STREAMED_SOUND_MISSION_ROK1_6},    {"ROK1_7", STREAMED_SOUND_MISSION_ROK1_7},
	{"ROK1_8", STREAMED_SOUND_MISSION_ROK1_8},    {"ROK1_9", STREAMED_SOUND_MISSION_ROK1_9},    {"PSYCH_1", STREAMED_SOUND_MISSION_PSYCH_1},
	{"PSYCH_2", STREAMED_SOUND_MISSION_PSYCH_2},  {"ROK2_01", STREAMED_SOUND_MISSION_ROK2_01},  {"ROK3_1", STREAMED_SOUND_MISSION_ROK3_1},
	{"ROK3_2", STREAMED_SOUND_MISSION_ROK3_2},    {"ROK3_3", STREAMED_SOUND_MISSION_ROK3_3},    {"ROK3_4", STREAMED_SOUND_MISSION_ROK3_4},
	{"ROK3_5", STREAMED_SOUND_MISSION_ROK3_5},    {"ROK3_6", STREAMED_SOUND_MISSION_ROK3_6},    {"ROK3_7", STREAMED_SOUND_MISSION_ROK3_7},
	{"ROK3_8", STREAMED_SOUND_MISSION_ROK3_8},    {"ROK3_9", STREAMED_SOUND_MISSION_ROK3_9},    {"ROK3_10", STREAMED_SOUND_MISSION_ROK3_10},
	{"ROK3_11", STREAMED_SOUND_MISSION_ROK3_11},  {"ROK3_12", STREAMED_SOUND_MISSION_ROK3_12},  {"ROK3_13", STREAMED_SOUND_MISSION_ROK3_13},
	{"ROK3_14", STREAMED_SOUND_MISSION_ROK3_14},  {"ROK3_15", STREAMED_SOUND_MISSION_ROK3_15},  {"ROK3_16", STREAMED_SOUND_MISSION_ROK3_16},
	{"ROK3_17", STREAMED_SOUND_MISSION_ROK3_17},  {"ROK3_18", STREAMED_SOUND_MISSION_ROK3_18},  {"ROK3_19", STREAMED_SOUND_MISSION_ROK3_19},
	{"ROK3_20", STREAMED_SOUND_MISSION_ROK3_20},  {"ROK3_21", STREAMED_SOUND_MISSION_ROK3_21},  {"ROK3_22", STREAMED_SOUND_MISSION_ROK3_22},
	{"ROK3_23", STREAMED_SOUND_MISSION_ROK3_23},  {"ROK3_24", STREAMED_SOUND_MISSION_ROK3_24},  {"ROK3_25", STREAMED_SOUND_MISSION_ROK3_25},
	{"ROK3_26", STREAMED_SOUND_MISSION_ROK3_26},  {"ROK3_27", STREAMED_SOUND_MISSION_ROK3_27},  {"ROK3_62", STREAMED_SOUND_MISSION_ROK3_62},
	{"ROK3_63", STREAMED_SOUND_MISSION_ROK3_63},  {"ROK3_64", STREAMED_SOUND_MISSION_ROK3_64},  {"ROK3_65", STREAMED_SOUND_MISSION_ROK3_65},
	{"ROK3_66", STREAMED_SOUND_MISSION_ROK3_66},  {"ROK3_67", STREAMED_SOUND_MISSION_ROK3_67},  {"ROK3_68", STREAMED_SOUND_MISSION_ROK3_68},
	{"ROK3_69", STREAMED_SOUND_MISSION_ROK3_69},  {"ROK3_70", STREAMED_SOUND_MISSION_ROK3_70},  {"ROK3_71", STREAMED_SOUND_MISSION_ROK3_71},
	{"ROK3_73", STREAMED_SOUND_MISSION_ROK3_73},  {"HAT_1a", STREAMED_SOUND_MISSION_HAT_1A},    {"intro1", STREAMED_SOUND_MISSION_INTRO1},
	{"intro2", STREAMED_SOUND_MISSION_INTRO2},    {"intro3", STREAMED_SOUND_MISSION_INTRO3},    {"intro4", STREAMED_SOUND_MISSION_INTRO4},
	{"CUB1_1", STREAMED_SOUND_MISSION_CUB1_1},    {"CUB1_2", STREAMED_SOUND_MISSION_CUB1_2},    {"CUB1_3", STREAMED_SOUND_MISSION_CUB1_3},
	{"CUB1_4", STREAMED_SOUND_MISSION_CUB1_4},    {"CUB1_5", STREAMED_SOUND_MISSION_CUB1_5},    {"CUB1_6", STREAMED_SOUND_MISSION_CUB1_6},
	{"CUB1_7", STREAMED_SOUND_MISSION_CUB1_7},    {"CUB1_8", STREAMED_SOUND_MISSION_CUB1_8},    {"CUB1_9", STREAMED_SOUND_MISSION_CUB1_9},
	{"CUB1_10", STREAMED_SOUND_MISSION_CUB1_10},  {"CUB2_1", STREAMED_SOUND_MISSION_CUB2_1},    {"CUB2_2", STREAMED_SOUND_MISSION_CUB2_2},
	{"CUB2_3a", STREAMED_SOUND_MISSION_CUB2_3A},  {"CUB2_3b", STREAMED_SOUND_MISSION_CUB2_3B},  {"CUB2_3c", STREAMED_SOUND_MISSION_CUB2_3C},
	{"CUB2_4a", STREAMED_SOUND_MISSION_CUB2_4A},  {"CUB2_5", STREAMED_SOUND_MISSION_CUB2_5},    {"CUB2_6", STREAMED_SOUND_MISSION_CUB2_6},
	{"CUB2_7", STREAMED_SOUND_MISSION_CUB2_7},    {"CUB2_8", STREAMED_SOUND_MISSION_CUB2_8},    {"CUB2_9", STREAMED_SOUND_MISSION_CUB2_9},
	{"CUB2_10", STREAMED_SOUND_MISSION_CUB2_10},  {"CUB2_11", STREAMED_SOUND_MISSION_CUB2_11},  {"CUB3_1", STREAMED_SOUND_MISSION_CUB3_1},
	{"CUB3_2", STREAMED_SOUND_MISSION_CUB3_2},    {"CUB3_3", STREAMED_SOUND_MISSION_CUB3_3},    {"CUB3_4", STREAMED_SOUND_MISSION_CUB3_4},
	{"CUB4_1", STREAMED_SOUND_MISSION_CUB4_1},    {"CUB4_2", STREAMED_SOUND_MISSION_CUB4_2},    {"CUB4_3", STREAMED_SOUND_MISSION_CUB4_3},
	{"CUB4_4", STREAMED_SOUND_MISSION_CUB4_4},    {"CUB4_5", STREAMED_SOUND_MISSION_CUB4_5},    {"CUB4_5A", STREAMED_SOUND_MISSION_CUB4_5A},
	{"CUB4_6", STREAMED_SOUND_MISSION_CUB4_6},    {"CUB4_7", STREAMED_SOUND_MISSION_CUB4_7},    {"CUB4_8", STREAMED_SOUND_MISSION_CUB4_8},
	{"CUB4_9", STREAMED_SOUND_MISSION_CUB4_9},    {"CUB4_10", STREAMED_SOUND_MISSION_CUB4_10},  {"CUB4_11", STREAMED_SOUND_MISSION_CUB4_11},
	{"CUB4_12", STREAMED_SOUND_MISSION_CUB4_12},  {"CUB4_13", STREAMED_SOUND_MISSION_CUB4_13},  {"CUB4_14", STREAMED_SOUND_MISSION_CUB4_14},
	{"CUB4_15", STREAMED_SOUND_MISSION_CUB4_15},  {"CUB4_16", STREAMED_SOUND_MISSION_CUB4_16},  {"golf_1", STREAMED_SOUND_MISSION_GOLF_1},
	{"golf_2", STREAMED_SOUND_MISSION_GOLF_2},    {"golf_3", STREAMED_SOUND_MISSION_GOLF_3},    {"bar_1", STREAMED_SOUND_MISSION_BAR_1},
	{"bar_2", STREAMED_SOUND_MISSION_BAR_2},      {"bar_3", STREAMED_SOUND_MISSION_BAR_3},      {"bar_4", STREAMED_SOUND_MISSION_BAR_4},
	{"bar_5", STREAMED_SOUND_MISSION_BAR_5},      {"bar_6", STREAMED_SOUND_MISSION_BAR_6},      {"bar_7", STREAMED_SOUND_MISSION_BAR_7},
	{"bar_8", STREAMED_SOUND_MISSION_BAR_8},      {"strip_1", STREAMED_SOUND_MISSION_STRIP_1},  {"strip_2", STREAMED_SOUND_MISSION_STRIP_2},
	{"strip_3", STREAMED_SOUND_MISSION_STRIP_3},  {"strip_4", STREAMED_SOUND_MISSION_STRIP_4},  {"strip_5", STREAMED_SOUND_MISSION_STRIP_5},
	{"strip_6", STREAMED_SOUND_MISSION_STRIP_6},  {"strip_7", STREAMED_SOUND_MISSION_STRIP_7},  {"strip_8", STREAMED_SOUND_MISSION_STRIP_8},
	{"strip_9", STREAMED_SOUND_MISSION_STRIP_9},  {"star_1", STREAMED_SOUND_MISSION_STAR_1},    {"star_2", STREAMED_SOUND_MISSION_STAR_2},
	{"star_3", STREAMED_SOUND_MISSION_STAR_3},    {"star_4", STREAMED_SOUND_MISSION_STAR_4},    {"mob_01a", STREAMED_SOUND_MISSION_MOB_01A},
	{"mob_01b", STREAMED_SOUND_MISSION_MOB_01B},  {"mob_01c", STREAMED_SOUND_MISSION_MOB_01C},  {"mob_02a", STREAMED_SOUND_MISSION_MOB_02A},
	{"mob_02b", STREAMED_SOUND_MISSION_MOB_02B},  {"mob_02c", STREAMED_SOUND_MISSION_MOB_02C},  {"mob_03a", STREAMED_SOUND_MISSION_MOB_03A},
	{"mob_03b", STREAMED_SOUND_MISSION_MOB_03B},  {"mob_03c", STREAMED_SOUND_MISSION_MOB_03C},  {"mob_03d", STREAMED_SOUND_MISSION_MOB_03D},
	{"mob_03e", STREAMED_SOUND_MISSION_MOB_03E},  {"shark_1", STREAMED_SOUND_MISSION_SHARK_1},  {"shark_2", STREAMED_SOUND_MISSION_SHARK_2},
	{"shark_3", STREAMED_SOUND_MISSION_SHARK_3},  {"shark_4", STREAMED_SOUND_MISSION_SHARK_4},  {"shark_5", STREAMED_SOUND_MISSION_SHARK_5},
	{"mob_04a", STREAMED_SOUND_MISSION_MOB_04A},  {"mob_04b", STREAMED_SOUND_MISSION_MOB_04B},  {"mob_04c", STREAMED_SOUND_MISSION_MOB_04C},
	{"mob_04d", STREAMED_SOUND_MISSION_MOB_04D},  {"mob_05a", STREAMED_SOUND_MISSION_MOB_05A},  {"mob_05b", STREAMED_SOUND_MISSION_MOB_05B},
	{"mob_05c", STREAMED_SOUND_MISSION_MOB_05C},  {"mob_05d", STREAMED_SOUND_MISSION_MOB_05D},  {"mob_06a", STREAMED_SOUND_MISSION_MOB_06A},
	{"mob_06b", STREAMED_SOUND_MISSION_MOB_06B},  {"mob_06c", STREAMED_SOUND_MISSION_MOB_06C},  {"mob_07a", STREAMED_SOUND_MISSION_MOB_07A},
	{"mob_07b", STREAMED_SOUND_MISSION_MOB_07B},  {"mob_08a", STREAMED_SOUND_MISSION_MOB_08A},  {"mob_08b", STREAMED_SOUND_MISSION_MOB_08B},
	{"mob_08c", STREAMED_SOUND_MISSION_MOB_08C},  {"mob_08d", STREAMED_SOUND_MISSION_MOB_08D},  {"mob_08e", STREAMED_SOUND_MISSION_MOB_08E},
	{"mob_08f", STREAMED_SOUND_MISSION_MOB_08F},  {"mob_08g", STREAMED_SOUND_MISSION_MOB_08G},  {"mob_09a", STREAMED_SOUND_MISSION_MOB_09A},
	{"mob_09b", STREAMED_SOUND_MISSION_MOB_09B},  {"mob_09c", STREAMED_SOUND_MISSION_MOB_09C},  {"mob_09d", STREAMED_SOUND_MISSION_MOB_09D},
	{"mob_09e", STREAMED_SOUND_MISSION_MOB_09E},  {"mob_09f", STREAMED_SOUND_MISSION_MOB_09F},  {"mob_10a", STREAMED_SOUND_MISSION_MOB_10A},
	{"mob_10b", STREAMED_SOUND_MISSION_MOB_10B},  {"mob_10c", STREAMED_SOUND_MISSION_MOB_10C},  {"mob_10d", STREAMED_SOUND_MISSION_MOB_10D},
	{"mob_10e", STREAMED_SOUND_MISSION_MOB_10E},  {"mob_11a", STREAMED_SOUND_MISSION_MOB_11A},  {"mob_11b", STREAMED_SOUND_MISSION_MOB_11B},
	{"mob_11c", STREAMED_SOUND_MISSION_MOB_11C},  {"mob_11d", STREAMED_SOUND_MISSION_MOB_11D},  {"mob_11e", STREAMED_SOUND_MISSION_MOB_11E},
	{"mob_11f", STREAMED_SOUND_MISSION_MOB_11F},  {"mob_14a", STREAMED_SOUND_MISSION_MOB_14A},  {"mob_14b", STREAMED_SOUND_MISSION_MOB_14B},
	{"mob_14c", STREAMED_SOUND_MISSION_MOB_14C},  {"mob_14d", STREAMED_SOUND_MISSION_MOB_14D},  {"mob_14e", STREAMED_SOUND_MISSION_MOB_14E},
	{"mob_14f", STREAMED_SOUND_MISSION_MOB_14F},  {"mob_14g", STREAMED_SOUND_MISSION_MOB_14G},  {"mob_14h", STREAMED_SOUND_MISSION_MOB_14H},
	{"mob_16a", STREAMED_SOUND_MISSION_MOB_16A},  {"mob_16b", STREAMED_SOUND_MISSION_MOB_16B},  {"mob_16c", STREAMED_SOUND_MISSION_MOB_16C},
	{"mob_16d", STREAMED_SOUND_MISSION_MOB_16D},  {"mob_16e", STREAMED_SOUND_MISSION_MOB_16E},  {"mob_16f", STREAMED_SOUND_MISSION_MOB_16F},
	{"mob_16g", STREAMED_SOUND_MISSION_MOB_16G},  {"mob_17a", STREAMED_SOUND_MISSION_MOB_17A},  {"mob_17b", STREAMED_SOUND_MISSION_MOB_17B},
	{"mob_17c", STREAMED_SOUND_MISSION_MOB_17C},  {"mob_17d", STREAMED_SOUND_MISSION_MOB_17D},  {"mob_17e", STREAMED_SOUND_MISSION_MOB_17E},
	{"mob_17g", STREAMED_SOUND_MISSION_MOB_17G},  {"mob_17h", STREAMED_SOUND_MISSION_MOB_17H},  {"mob_17i", STREAMED_SOUND_MISSION_MOB_17I},
	{"mob_17j", STREAMED_SOUND_MISSION_MOB_17J},  {"mob_17k", STREAMED_SOUND_MISSION_MOB_17K},  {"mob_17l", STREAMED_SOUND_MISSION_MOB_17L},
	{"mob_18a", STREAMED_SOUND_MISSION_MOB_18A},  {"mob_18b", STREAMED_SOUND_MISSION_MOB_18B},  {"mob_18c", STREAMED_SOUND_MISSION_MOB_18C},
	{"mob_18d", STREAMED_SOUND_MISSION_MOB_18D},  {"mob_18e", STREAMED_SOUND_MISSION_MOB_18E},  {"mob_18f", STREAMED_SOUND_MISSION_MOB_18F},
	{"mob_18g", STREAMED_SOUND_MISSION_MOB_18G},  {"mob_20a", STREAMED_SOUND_MISSION_MOB_20A},  {"mob_20b", STREAMED_SOUND_MISSION_MOB_20B},
	{"mob_20c", STREAMED_SOUND_MISSION_MOB_20C},  {"mob_20d", STREAMED_SOUND_MISSION_MOB_20D},  {"mob_20e", STREAMED_SOUND_MISSION_MOB_20E},
	{"mob_24a", STREAMED_SOUND_MISSION_MOB_24A},  {"mob_24b", STREAMED_SOUND_MISSION_MOB_24B},  {"mob_24c", STREAMED_SOUND_MISSION_MOB_24C},
	{"mob_24d", STREAMED_SOUND_MISSION_MOB_24D},  {"mob_24e", STREAMED_SOUND_MISSION_MOB_24E},  {"mob_24f", STREAMED_SOUND_MISSION_MOB_24F},
	{"mob_24g", STREAMED_SOUND_MISSION_MOB_24G},  {"mob_24h", STREAMED_SOUND_MISSION_MOB_24H},  {"mob_25a", STREAMED_SOUND_MISSION_MOB_25A},
	{"mob_25b", STREAMED_SOUND_MISSION_MOB_25B},  {"mob_25c", STREAMED_SOUND_MISSION_MOB_25C},  {"mob_25d", STREAMED_SOUND_MISSION_MOB_25D},
	{"mob_26a", STREAMED_SOUND_MISSION_MOB_26A},  {"mob_26b", STREAMED_SOUND_MISSION_MOB_26B},  {"mob_26c", STREAMED_SOUND_MISSION_MOB_26C},
	{"mob_26d", STREAMED_SOUND_MISSION_MOB_26D},  {"mob_26e", STREAMED_SOUND_MISSION_MOB_26E},  {"mob_29a", STREAMED_SOUND_MISSION_MOB_29A},
	{"mob_29b", STREAMED_SOUND_MISSION_MOB_29B},  {"mob_29c", STREAMED_SOUND_MISSION_MOB_29C},  {"mob_29d", STREAMED_SOUND_MISSION_MOB_29D},
	{"mob_29e", STREAMED_SOUND_MISSION_MOB_29E},  {"mob_29f", STREAMED_SOUND_MISSION_MOB_29F},  {"mob_29g", STREAMED_SOUND_MISSION_MOB_29G},
	{"mob_30a", STREAMED_SOUND_MISSION_MOB_30A},  {"mob_30b", STREAMED_SOUND_MISSION_MOB_30B},  {"mob_30c", STREAMED_SOUND_MISSION_MOB_30C},
	{"mob_30d", STREAMED_SOUND_MISSION_MOB_30D},  {"mob_30e", STREAMED_SOUND_MISSION_MOB_30E},  {"mob_30f", STREAMED_SOUND_MISSION_MOB_30F},
	{"mob_33a", STREAMED_SOUND_MISSION_MOB_33A},  {"mob_33b", STREAMED_SOUND_MISSION_MOB_33B},  {"mob_33c", STREAMED_SOUND_MISSION_MOB_33C},
	{"mob_33d", STREAMED_SOUND_MISSION_MOB_33D},  {"mob_34a", STREAMED_SOUND_MISSION_MOB_34A},  {"mob_34b", STREAMED_SOUND_MISSION_MOB_34B},
	{"mob_34c", STREAMED_SOUND_MISSION_MOB_34C},  {"mob_34d", STREAMED_SOUND_MISSION_MOB_34D},  {"mob_35a", STREAMED_SOUND_MISSION_MOB_35A},
	{"mob_35b", STREAMED_SOUND_MISSION_MOB_35B},  {"mob_35c", STREAMED_SOUND_MISSION_MOB_35C},  {"mob_35d", STREAMED_SOUND_MISSION_MOB_35D},
	{"mob_36a", STREAMED_SOUND_MISSION_MOB_36A},  {"mob_36b", STREAMED_SOUND_MISSION_MOB_36B},  {"mob_36c", STREAMED_SOUND_MISSION_MOB_36C},
	{"mob_40a", STREAMED_SOUND_MISSION_MOB_40A},  {"mob_40b", STREAMED_SOUND_MISSION_MOB_40B},  {"mob_40c", STREAMED_SOUND_MISSION_MOB_40C},
	{"mob_40d", STREAMED_SOUND_MISSION_MOB_40D},  {"mob_40e", STREAMED_SOUND_MISSION_MOB_40E},  {"mob_40f", STREAMED_SOUND_MISSION_MOB_40F},
	{"mob_40g", STREAMED_SOUND_MISSION_MOB_40G},  {"mob_40h", STREAMED_SOUND_MISSION_MOB_40H},  {"mob_40i", STREAMED_SOUND_MISSION_MOB_40I},
	{"mob_41a", STREAMED_SOUND_MISSION_MOB_41A},  {"mob_41b", STREAMED_SOUND_MISSION_MOB_41B},  {"mob_41c", STREAMED_SOUND_MISSION_MOB_41C},
	{"mob_41d", STREAMED_SOUND_MISSION_MOB_41D},  {"mob_41e", STREAMED_SOUND_MISSION_MOB_41E},  {"mob_41f", STREAMED_SOUND_MISSION_MOB_41F},
	{"mob_41g", STREAMED_SOUND_MISSION_MOB_41G},  {"mob_41h", STREAMED_SOUND_MISSION_MOB_41H},  {"mob_42a", STREAMED_SOUND_MISSION_MOB_42A},
	{"mob_42b", STREAMED_SOUND_MISSION_MOB_42B},  {"mob_42c", STREAMED_SOUND_MISSION_MOB_42C},  {"mob_42d", STREAMED_SOUND_MISSION_MOB_42D},
	{"mob_42e", STREAMED_SOUND_MISSION_MOB_42E},  {"mob_43a", STREAMED_SOUND_MISSION_MOB_43A},  {"mob_43b", STREAMED_SOUND_MISSION_MOB_43B},
	{"mob_43c", STREAMED_SOUND_MISSION_MOB_43C},  {"mob_43d", STREAMED_SOUND_MISSION_MOB_43D},  {"mob_43e", STREAMED_SOUND_MISSION_MOB_43E},
	{"mob_43f", STREAMED_SOUND_MISSION_MOB_43F},  {"mob_43g", STREAMED_SOUND_MISSION_MOB_43G},  {"mob_43h", STREAMED_SOUND_MISSION_MOB_43H},
	{"mob_45a", STREAMED_SOUND_MISSION_MOB_45A},  {"mob_45b", STREAMED_SOUND_MISSION_MOB_45B},  {"mob_45c", STREAMED_SOUND_MISSION_MOB_45C},
	{"mob_45d", STREAMED_SOUND_MISSION_MOB_45D},  {"mob_45e", STREAMED_SOUND_MISSION_MOB_45E},  {"mob_45f", STREAMED_SOUND_MISSION_MOB_45F},
	{"mob_45g", STREAMED_SOUND_MISSION_MOB_45G},  {"mob_45h", STREAMED_SOUND_MISSION_MOB_45H},  {"mob_45i", STREAMED_SOUND_MISSION_MOB_45I},
	{"mob_45j", STREAMED_SOUND_MISSION_MOB_45J},  {"mob_45k", STREAMED_SOUND_MISSION_MOB_45K},  {"mob_45l", STREAMED_SOUND_MISSION_MOB_45L},
	{"mob_45m", STREAMED_SOUND_MISSION_MOB_45M},  {"mob_45n", STREAMED_SOUND_MISSION_MOB_45N},  {"mob_46a", STREAMED_SOUND_MISSION_MOB_46A},
	{"mob_46b", STREAMED_SOUND_MISSION_MOB_46B},  {"mob_46c", STREAMED_SOUND_MISSION_MOB_46C},  {"mob_46d", STREAMED_SOUND_MISSION_MOB_46D},
	{"mob_46e", STREAMED_SOUND_MISSION_MOB_46E},  {"mob_46f", STREAMED_SOUND_MISSION_MOB_46F},  {"mob_46g", STREAMED_SOUND_MISSION_MOB_46G},
	{"mob_46h", STREAMED_SOUND_MISSION_MOB_46H},  {"mob_47a", STREAMED_SOUND_MISSION_MOB_47A},  {"mob_52a", STREAMED_SOUND_MISSION_MOB_52A},
	{"mob_52b", STREAMED_SOUND_MISSION_MOB_52B},  {"mob_52c", STREAMED_SOUND_MISSION_MOB_52C},  {"mob_52d", STREAMED_SOUND_MISSION_MOB_52D},
	{"mob_52e", STREAMED_SOUND_MISSION_MOB_52E},  {"mob_52f", STREAMED_SOUND_MISSION_MOB_52F},  {"mob_52g", STREAMED_SOUND_MISSION_MOB_52G},
	{"mob_52h", STREAMED_SOUND_MISSION_MOB_52H},  {"mob_54a", STREAMED_SOUND_MISSION_MOB_54A},  {"mob_54b", STREAMED_SOUND_MISSION_MOB_54B},
	{"mob_54c", STREAMED_SOUND_MISSION_MOB_54C},  {"mob_54d", STREAMED_SOUND_MISSION_MOB_54D},  {"mob_54e", STREAMED_SOUND_MISSION_MOB_54E},
	{"mob_55a", STREAMED_SOUND_MISSION_MOB_55A},  {"mob_55b", STREAMED_SOUND_MISSION_MOB_55B},  {"mob_55c", STREAMED_SOUND_MISSION_MOB_55C},
	{"mob_55d", STREAMED_SOUND_MISSION_MOB_55D},  {"mob_55e", STREAMED_SOUND_MISSION_MOB_55E},  {"mob_55f", STREAMED_SOUND_MISSION_MOB_55F},
	{"mob_56a", STREAMED_SOUND_MISSION_MOB_56A},  {"mob_56b", STREAMED_SOUND_MISSION_MOB_56B},  {"mob_56c", STREAMED_SOUND_MISSION_MOB_56C},
	{"mob_56d", STREAMED_SOUND_MISSION_MOB_56D},  {"mob_56e", STREAMED_SOUND_MISSION_MOB_56E},  {"mob_56f", STREAMED_SOUND_MISSION_MOB_56F},
	{"mob_57a", STREAMED_SOUND_MISSION_MOB_57A},  {"mob_57b", STREAMED_SOUND_MISSION_MOB_57B},  {"mob_57c", STREAMED_SOUND_MISSION_MOB_57C},
	{"mob_57d", STREAMED_SOUND_MISSION_MOB_57D},  {"mob_57e", STREAMED_SOUND_MISSION_MOB_57E},  {"mob_58a", STREAMED_SOUND_MISSION_MOB_58A},
	{"mob_58b", STREAMED_SOUND_MISSION_MOB_58B},  {"mob_58c", STREAMED_SOUND_MISSION_MOB_58C},  {"mob_58d", STREAMED_SOUND_MISSION_MOB_58D},
	{"mob_58e", STREAMED_SOUND_MISSION_MOB_58E},  {"mob_58f", STREAMED_SOUND_MISSION_MOB_58F},  {"mob_58g", STREAMED_SOUND_MISSION_MOB_58G},
	{"mob_61a", STREAMED_SOUND_MISSION_MOB_61A},  {"mob_61b", STREAMED_SOUND_MISSION_MOB_61B},  {"mob_62a", STREAMED_SOUND_MISSION_MOB_62A},
	{"mob_62b", STREAMED_SOUND_MISSION_MOB_62B},  {"mob_62c", STREAMED_SOUND_MISSION_MOB_62C},  {"mob_62d", STREAMED_SOUND_MISSION_MOB_62D},
	{"mob_63a", STREAMED_SOUND_MISSION_MOB_63A},  {"mob_63b", STREAMED_SOUND_MISSION_MOB_63B},  {"mob_63c", STREAMED_SOUND_MISSION_MOB_63C},
	{"mob_63d", STREAMED_SOUND_MISSION_MOB_63D},  {"mob_63e", STREAMED_SOUND_MISSION_MOB_63E},  {"mob_63f", STREAMED_SOUND_MISSION_MOB_63F},
	{"mob_63g", STREAMED_SOUND_MISSION_MOB_63G},  {"mob_63h", STREAMED_SOUND_MISSION_MOB_63H},  {"mob_63i", STREAMED_SOUND_MISSION_MOB_63I},
	{"mob_63j", STREAMED_SOUND_MISSION_MOB_63J},  {"mob_66a", STREAMED_SOUND_MISSION_MOB_66A},  {"mob_66b", STREAMED_SOUND_MISSION_MOB_66B},
	{"mob_68a", STREAMED_SOUND_MISSION_MOB_68A},  {"mob_68b", STREAMED_SOUND_MISSION_MOB_68B},  {"mob_68c", STREAMED_SOUND_MISSION_MOB_68C},
	{"mob_68d", STREAMED_SOUND_MISSION_MOB_68D},  {"mob_70a", STREAMED_SOUND_MISSION_MOB_70A},  {"mob_70b", STREAMED_SOUND_MISSION_MOB_70B},
	{"mob_71a", STREAMED_SOUND_MISSION_MOB_71A},  {"mob_71b", STREAMED_SOUND_MISSION_MOB_71B},  {"mob_71c", STREAMED_SOUND_MISSION_MOB_71C},
	{"mob_71d", STREAMED_SOUND_MISSION_MOB_71D},  {"mob_71e", STREAMED_SOUND_MISSION_MOB_71E},  {"mob_71f", STREAMED_SOUND_MISSION_MOB_71F},
	{"mob_71g", STREAMED_SOUND_MISSION_MOB_71G},  {"mob_71h", STREAMED_SOUND_MISSION_MOB_71H},  {"mob_71i", STREAMED_SOUND_MISSION_MOB_71I},
	{"mob_71j", STREAMED_SOUND_MISSION_MOB_71J},  {"mob_71k", STREAMED_SOUND_MISSION_MOB_71K},  {"mob_71l", STREAMED_SOUND_MISSION_MOB_71L},
	{"mob_71m", STREAMED_SOUND_MISSION_MOB_71M},  {"mob_71n", STREAMED_SOUND_MISSION_MOB_71N},  {"mob_72a", STREAMED_SOUND_MISSION_MOB_72A},
	{"mob_72b", STREAMED_SOUND_MISSION_MOB_72B},  {"mob_72c", STREAMED_SOUND_MISSION_MOB_72C},  {"mob_72d", STREAMED_SOUND_MISSION_MOB_72D},
	{"mob_72e", STREAMED_SOUND_MISSION_MOB_72E},  {"mob_72f", STREAMED_SOUND_MISSION_MOB_72F},  {"mob_72g", STREAMED_SOUND_MISSION_MOB_72G},
	{"mob_73a", STREAMED_SOUND_MISSION_MOB_73A},  {"mob_73c", STREAMED_SOUND_MISSION_MOB_73C},  {"mob_73d", STREAMED_SOUND_MISSION_MOB_73D},
	{"mob_73f", STREAMED_SOUND_MISSION_MOB_73F},  {"mob_73g", STREAMED_SOUND_MISSION_MOB_73G},  {"mob_73i", STREAMED_SOUND_MISSION_MOB_73I},
	{"mob_95a", STREAMED_SOUND_MISSION_MOB_95A},  {"mob_96a", STREAMED_SOUND_MISSION_MOB_96A},  {"mob_98a", STREAMED_SOUND_MISSION_MOB_98A},
	{"mob_99a", STREAMED_SOUND_MISSION_MOB_99A},  {"job1_1b", STREAMED_SOUND_MISSION_JOB1_1B},  {"job1_1c", STREAMED_SOUND_MISSION_JOB1_1C},
	{"job1_1d", STREAMED_SOUND_MISSION_JOB1_1D},  {"job2_1b", STREAMED_SOUND_MISSION_JOB2_1B},  {"job2_2", STREAMED_SOUND_MISSION_JOB2_2},
	{"job2_3", STREAMED_SOUND_MISSION_JOB2_3},    {"job2_4", STREAMED_SOUND_MISSION_JOB2_4},    {"job2_5", STREAMED_SOUND_MISSION_JOB2_5},
	{"job2_6", STREAMED_SOUND_MISSION_JOB2_6},    {"job2_7", STREAMED_SOUND_MISSION_JOB2_7},    {"job2_8", STREAMED_SOUND_MISSION_JOB2_8},
	{"job2_9", STREAMED_SOUND_MISSION_JOB2_9},    {"job3_1", STREAMED_SOUND_MISSION_JOB3_1},    {"job3_2", STREAMED_SOUND_MISSION_JOB3_2},
	{"job3_3", STREAMED_SOUND_MISSION_JOB3_3},    {"job4_1", STREAMED_SOUND_MISSION_JOB4_1},    {"job4_2", STREAMED_SOUND_MISSION_JOB4_2},
	{"job4_3", STREAMED_SOUND_MISSION_JOB4_3},    {"job5_1", STREAMED_SOUND_MISSION_JOB5_1},    {"job5_2", STREAMED_SOUND_MISSION_JOB5_2},
	{"job5_3", STREAMED_SOUND_MISSION_JOB5_3},    {"bjm1_20", STREAMED_SOUND_MISSION_BJM1_20},  {"bjm1_4", STREAMED_SOUND_MISSION_BJM1_4},
	{"bjm1_5", STREAMED_SOUND_MISSION_BJM1_5},    {"merc_39", STREAMED_SOUND_MISSION_MERC_39},  {"mono_1", STREAMED_SOUND_MISSION_MONO_1},
	{"mono_2", STREAMED_SOUND_MISSION_MONO_2},    {"mono_3", STREAMED_SOUND_MISSION_MONO_3},    {"mono_4", STREAMED_SOUND_MISSION_MONO_4},
	{"mono_5", STREAMED_SOUND_MISSION_MONO_5},    {"mono_6", STREAMED_SOUND_MISSION_MONO_6},    {"mono_7", STREAMED_SOUND_MISSION_MONO_7},
	{"mono_8", STREAMED_SOUND_MISSION_MONO_8},    {"mono_9", STREAMED_SOUND_MISSION_MONO_9},    {"mono10", STREAMED_SOUND_MISSION_MONO10},
	{"mono11", STREAMED_SOUND_MISSION_MONO11},    {"mono12", STREAMED_SOUND_MISSION_MONO12},    {"mono13", STREAMED_SOUND_MISSION_MONO13},
	{"mono14", STREAMED_SOUND_MISSION_MONO14},    {"mono15", STREAMED_SOUND_MISSION_MONO15},    {"mono16", STREAMED_SOUND_MISSION_MONO16},
	{"fud_01", STREAMED_SOUND_MISSION_FUD_01},    {"fud_02", STREAMED_SOUND_MISSION_FUD_02},    {"fud_03", STREAMED_SOUND_MISSION_FUD_03},
	{"fud_04", STREAMED_SOUND_MISSION_FUD_04},    {"fud_05", STREAMED_SOUND_MISSION_FUD_05},    {"fud_06", STREAMED_SOUND_MISSION_FUD_06},
	{"fud_07", STREAMED_SOUND_MISSION_FUD_07},    {"fud_08", STREAMED_SOUND_MISSION_FUD_08},    {"fud_09", STREAMED_SOUND_MISSION_FUD_09},
	{"fud_10", STREAMED_SOUND_MISSION_FUD_10},    {"fud_11", STREAMED_SOUND_MISSION_FUD_11},    {"fud_12", STREAMED_SOUND_MISSION_FUD_12},
	{"fud_13", STREAMED_SOUND_MISSION_FUD_13},    {"fud_14", STREAMED_SOUND_MISSION_FUD_14},    {"fud_15", STREAMED_SOUND_MISSION_FUD_15},
	{"fud_16", STREAMED_SOUND_MISSION_FUD_16},    {"fud_17", STREAMED_SOUND_MISSION_FUD_17},    {"fud_18", STREAMED_SOUND_MISSION_FUD_18},
	{"fud_19", STREAMED_SOUND_MISSION_FUD_19},    {"fud_20", STREAMED_SOUND_MISSION_FUD_20},    {"burg_01", STREAMED_SOUND_MISSION_BURG_01},
	{"burg_02", STREAMED_SOUND_MISSION_BURG_02},  {"burg_03", STREAMED_SOUND_MISSION_BURG_03},  {"burg_04", STREAMED_SOUND_MISSION_BURG_04},
	{"burg_05", STREAMED_SOUND_MISSION_BURG_05},  {"burg_06", STREAMED_SOUND_MISSION_BURG_06},  {"burg_07", STREAMED_SOUND_MISSION_BURG_07},
	{"burg_08", STREAMED_SOUND_MISSION_BURG_08},  {"burg_09", STREAMED_SOUND_MISSION_BURG_09},  {"burg_10", STREAMED_SOUND_MISSION_BURG_10},
	{"burg_11", STREAMED_SOUND_MISSION_BURG_11},  {"burg_12", STREAMED_SOUND_MISSION_BURG_12},  {"crust01", STREAMED_SOUND_MISSION_CRUST01},
	{"crust02", STREAMED_SOUND_MISSION_CRUST02},  {"crust03", STREAMED_SOUND_MISSION_CRUST03},  {"crust04", STREAMED_SOUND_MISSION_CRUST04},
	{"crust05", STREAMED_SOUND_MISSION_CRUST05},  {"crust06", STREAMED_SOUND_MISSION_CRUST06},  {"crust07", STREAMED_SOUND_MISSION_CRUST07},
	{"crust08", STREAMED_SOUND_MISSION_CRUST08},  {"crust09", STREAMED_SOUND_MISSION_CRUST09},  {"band_01", STREAMED_SOUND_MISSION_BAND_01},
	{"band_02", STREAMED_SOUND_MISSION_BAND_02},  {"band_03", STREAMED_SOUND_MISSION_BAND_03},  {"band_04", STREAMED_SOUND_MISSION_BAND_04},
	{"band_05", STREAMED_SOUND_MISSION_BAND_05},  {"band_06", STREAMED_SOUND_MISSION_BAND_06},  {"band_07", STREAMED_SOUND_MISSION_BAND_07},
	{"band_08", STREAMED_SOUND_MISSION_BAND_08},  {"shaft01", STREAMED_SOUND_MISSION_SHAFT01},  {"shaft02", STREAMED_SOUND_MISSION_SHAFT02},
	{"shaft03", STREAMED_SOUND_MISSION_SHAFT03},  {"shaft04", STREAMED_SOUND_MISSION_SHAFT04},  {"shaft05", STREAMED_SOUND_MISSION_SHAFT05},
	{"shaft06", STREAMED_SOUND_MISSION_SHAFT06},  {"shaft07", STREAMED_SOUND_MISSION_SHAFT07},  {"shaft08", STREAMED_SOUND_MISSION_SHAFT08},
	{"piss_01", STREAMED_SOUND_MISSION_PISS_01},  {"piss_02", STREAMED_SOUND_MISSION_PISS_02},  {"piss_03", STREAMED_SOUND_MISSION_PISS_03},
	{"piss_04", STREAMED_SOUND_MISSION_PISS_04},  {"piss_05", STREAMED_SOUND_MISSION_PISS_05},  {"piss_06", STREAMED_SOUND_MISSION_PISS_06},
	{"piss_07", STREAMED_SOUND_MISSION_PISS_07},  {"piss_08", STREAMED_SOUND_MISSION_PISS_08},  {"piss_09", STREAMED_SOUND_MISSION_PISS_09},
	{"piss_10", STREAMED_SOUND_MISSION_PISS_10},  {"piss_11", STREAMED_SOUND_MISSION_PISS_11},  {"piss_12", STREAMED_SOUND_MISSION_PISS_12},
	{"piss_13", STREAMED_SOUND_MISSION_PISS_13},  {"piss_14", STREAMED_SOUND_MISSION_PISS_14},  {"piss_15", STREAMED_SOUND_MISSION_PISS_15},
	{"piss_16", STREAMED_SOUND_MISSION_PISS_16},  {"piss_17", STREAMED_SOUND_MISSION_PISS_17},  {"piss_18", STREAMED_SOUND_MISSION_PISS_18},
	{"piss_19", STREAMED_SOUND_MISSION_PISS_19},  {"gimme01", STREAMED_SOUND_MISSION_GIMME01},  {"gimme02", STREAMED_SOUND_MISSION_GIMME02},
	{"gimme03", STREAMED_SOUND_MISSION_GIMME03},  {"gimme04", STREAMED_SOUND_MISSION_GIMME04},  {"gimme05", STREAMED_SOUND_MISSION_GIMME05},
	{"gimme06", STREAMED_SOUND_MISSION_GIMME06},  {"gimme07", STREAMED_SOUND_MISSION_GIMME07},  {"gimme08", STREAMED_SOUND_MISSION_GIMME08},
	{"gimme09", STREAMED_SOUND_MISSION_GIMME09},  {"gimme10", STREAMED_SOUND_MISSION_GIMME10},  {"gimme11", STREAMED_SOUND_MISSION_GIMME11},
	{"gimme12", STREAMED_SOUND_MISSION_GIMME12},  {"gimme13", STREAMED_SOUND_MISSION_GIMME13},  {"gimme14", STREAMED_SOUND_MISSION_GIMME14},
	{"gimme15", STREAMED_SOUND_MISSION_GIMME15},  {"bust_01", STREAMED_SOUND_MISSION_BUST_01},  {"bust_02", STREAMED_SOUND_MISSION_BUST_02},
	{"bust_03", STREAMED_SOUND_MISSION_BUST_03},  {"bust_04", STREAMED_SOUND_MISSION_BUST_04},  {"bust_05", STREAMED_SOUND_MISSION_BUST_05},
	{"bust_06", STREAMED_SOUND_MISSION_BUST_06},  {"bust_07", STREAMED_SOUND_MISSION_BUST_07},  {"bust_08", STREAMED_SOUND_MISSION_BUST_08},
	{"bust_09", STREAMED_SOUND_MISSION_BUST_09},  {"bust_10", STREAMED_SOUND_MISSION_BUST_10},  {"bust_11", STREAMED_SOUND_MISSION_BUST_11},
	{"bust_12", STREAMED_SOUND_MISSION_BUST_12},  {"bust_13", STREAMED_SOUND_MISSION_BUST_13},  {"bust_14", STREAMED_SOUND_MISSION_BUST_14},
	{"bust_15", STREAMED_SOUND_MISSION_BUST_15},  {"bust_16", STREAMED_SOUND_MISSION_BUST_16},  {"bust_17", STREAMED_SOUND_MISSION_BUST_17},
	{"bust_18", STREAMED_SOUND_MISSION_BUST_18},  {"bust_19", STREAMED_SOUND_MISSION_BUST_19},  {"bust_20", STREAMED_SOUND_MISSION_BUST_20},
	{"bust_21", STREAMED_SOUND_MISSION_BUST_21},  {"bust_22", STREAMED_SOUND_MISSION_BUST_22},  {"bust_23", STREAMED_SOUND_MISSION_BUST_23},
	{"bust_24", STREAMED_SOUND_MISSION_BUST_24},  {"bust_25", STREAMED_SOUND_MISSION_BUST_25},  {"bust_26", STREAMED_SOUND_MISSION_BUST_26},
	{"bust_27", STREAMED_SOUND_MISSION_BUST_27},  {"bust_28", STREAMED_SOUND_MISSION_BUST_28},  {nil, 0} };

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
	return false;
}

void
cAudioManager::PreloadMissionAudio(uint8 slot, Const char *name)
{
	if (m_bIsInitialised && slot < MISSION_AUDIO_SLOTS) {
		int32 missionAudioSfx = FindMissionAudioSfx(name);
		if (missionAudioSfx != NO_SAMPLE) {
			m_sMissionAudio.m_nSampleIndex[slot] = missionAudioSfx;
			m_sMissionAudio.m_nLoadingStatus[slot] = LOADING_STATUS_NOT_LOADED;
			m_sMissionAudio.m_nPlayStatus[slot] = PLAY_STATUS_STOPPED;
			m_sMissionAudio.field_22[slot] = 0;
			m_sMissionAudio.m_nMissionAudioCounter[slot] = m_nTimeSpent * SampleManager.GetStreamedFileLength(missionAudioSfx) / 1000;
			m_sMissionAudio.m_nMissionAudioCounter[slot] *= 4;
			m_sMissionAudio.m_bIsPlayed[slot] = false;
			m_sMissionAudio.m_bPredefinedProperties[slot] = true;
			g_bMissionAudioLoadFailed[slot] = false;
		}
	}
}

uint8
cAudioManager::GetMissionAudioLoadingStatus(uint8 slot) const
{
	if (m_bIsInitialised && slot < MISSION_AUDIO_SLOTS)
		return m_sMissionAudio.m_nLoadingStatus[slot];

	return LOADING_STATUS_LOADED;
}

void
cAudioManager::SetMissionAudioLocation(uint8 slot, float x, float y, float z)
{
	if (m_bIsInitialised && slot < MISSION_AUDIO_SLOTS) {
		m_sMissionAudio.m_bPredefinedProperties[slot] = false;
		m_sMissionAudio.m_vecPos[slot] = CVector(x, y, z);
	}
}

void
cAudioManager::PlayLoadedMissionAudio(uint8 slot)
{
	if (m_bIsInitialised && slot < MISSION_AUDIO_SLOTS && m_sMissionAudio.m_nSampleIndex[slot] != NO_SAMPLE && m_sMissionAudio.m_nLoadingStatus[slot] == LOADING_STATUS_LOADED &&
	    m_sMissionAudio.m_nPlayStatus[slot] == PLAY_STATUS_STOPPED)
		m_sMissionAudio.m_bIsPlayed[slot] = true;
}

bool
cAudioManager::ShouldDuckMissionAudio(uint8 slot) const
{
	if (IsMissionAudioSamplePlaying(slot))
		return m_sMissionAudio.m_nSampleIndex[slot] != STREAMED_SOUND_MISSION_ROK2_01;
	return false;
}

bool
cAudioManager::IsMissionAudioSamplePlaying(uint8 slot) const
{
	if (m_bIsInitialised) {
		if (slot < MISSION_AUDIO_SLOTS)
			return m_sMissionAudio.m_nPlayStatus[slot] == PLAY_STATUS_PLAYING;
		else
			return true;
	} else {
		static int32 cPretendFrame[MISSION_AUDIO_SLOTS] = { 1, 1 };

		return (cPretendFrame[slot]++ % 64) != 0;
	}
}

bool
cAudioManager::IsMissionAudioSampleFinished(uint8 slot)
{
	if (m_bIsInitialised) {
		if (slot < MISSION_AUDIO_SLOTS)
			return m_sMissionAudio.m_nPlayStatus[slot] == PLAY_STATUS_FINISHED;
		else
			return true;
	}

	static int32 cPretendFrame[MISSION_AUDIO_SLOTS] = { 1, 1 };

	return (cPretendFrame[slot]++ % 64) == 0;
}

void
cAudioManager::ClearMissionAudio(uint8 slot)
{
	if (m_bIsInitialised && slot < MISSION_AUDIO_SLOTS) {
		m_sMissionAudio.m_nSampleIndex[slot] = NO_SAMPLE;
		m_sMissionAudio.m_nLoadingStatus[slot] = LOADING_STATUS_NOT_LOADED;
		m_sMissionAudio.m_nPlayStatus[slot] = PLAY_STATUS_STOPPED;
		m_sMissionAudio.field_22[slot] = 0;
		m_sMissionAudio.m_bIsPlayed[slot] = false;
		m_sMissionAudio.m_bPredefinedProperties[slot] = true;
		m_sMissionAudio.m_nMissionAudioCounter[slot] = 0;
	}
}

void
cAudioManager::ProcessMissionAudioSlot(uint8 slot)
{
	float dist;
	uint8 emittingVol;
	uint8 pan;
	float distSquared;
	CVector vec;

	static uint8 nCheckPlayingDelay[MISSION_AUDIO_SLOTS] = { 0, 0 };
	static uint8 nFramesUntilFailedLoad[MISSION_AUDIO_SLOTS] = { 0, 0 };
	static uint8 nFramesForPretendPlaying[MISSION_AUDIO_SLOTS] = { 0, 0 };

	if (m_sMissionAudio.m_nSampleIndex[slot] == NO_SAMPLE) return;

	switch (m_sMissionAudio.m_nLoadingStatus[slot]) {
	case LOADING_STATUS_NOT_LOADED:
		SampleManager.PreloadStreamedFile(m_sMissionAudio.m_nSampleIndex[slot], slot + 1);
		m_sMissionAudio.m_nLoadingStatus[slot] = LOADING_STATUS_LOADED;
		nFramesUntilFailedLoad[slot] = 0;
		break;
	case LOADING_STATUS_LOADED:
		if (!m_sMissionAudio.m_bIsPlayed[slot])
			return;
		if (g_bMissionAudioLoadFailed[slot]) {
			if (m_bTimerJustReset) {
				ClearMissionAudio(slot);
				SampleManager.StopStreamedFile(slot + 1);
				nFramesForPretendPlaying[slot] = 0;
				nCheckPlayingDelay[slot] = 0;
				nFramesUntilFailedLoad[slot] = 0;
			} else if (!m_nUserPause) {
				if (++nFramesForPretendPlaying[slot] < 120) {
					m_sMissionAudio.m_nPlayStatus[slot] = PLAY_STATUS_PLAYING;
				} else {
					m_sMissionAudio.m_nPlayStatus[slot] = PLAY_STATUS_FINISHED;
					m_sMissionAudio.m_nSampleIndex[slot] = NO_SAMPLE;
				}
			}
			break;
		}
		switch (m_sMissionAudio.m_nPlayStatus[slot]) {
		case PLAY_STATUS_STOPPED:
			if (MissionScriptAudioUsesPoliceChannel(m_sMissionAudio.m_nSampleIndex[slot])) {
				SetMissionScriptPoliceAudio(m_sMissionAudio.m_nSampleIndex[slot]);
			} else {
				if (m_nUserPause)
					SampleManager.PauseStream(1, slot + 1);
				if (m_sMissionAudio.m_bPredefinedProperties) {
					if (m_sMissionAudio.m_nSampleIndex[slot] == STREAMED_SOUND_MISSION_CAMERAL)
						SampleManager.SetStreamedVolumeAndPan(80, 0, 1, slot + 1);
					else if (m_sMissionAudio.m_nSampleIndex[slot] == STREAMED_SOUND_MISSION_CAMERAR)
						SampleManager.SetStreamedVolumeAndPan(80, 127, 1, slot + 1);
					else
						SampleManager.SetStreamedVolumeAndPan(80, 63, 1, slot + 1);
				} else {
					distSquared = GetDistanceSquared(m_sMissionAudio.m_vecPos[slot]);
					if (distSquared >= SQR(80.0f)) {
						emittingVol = 0;
						pan = 63;
					} else {
						emittingVol = 80;
						if (distSquared > 0.0f) {
							dist = Sqrt(distSquared);
							emittingVol = ComputeVolume(80, 80.0f, dist);
						}
						TranslateEntity(&m_sMissionAudio.m_vecPos[slot], &vec);
						pan = ComputePan(80.f, &vec);
					}
					SampleManager.SetStreamedVolumeAndPan(emittingVol, pan, 1, slot + 1);
				}
				SampleManager.StartPreloadedStreamedFile(slot + 1);
			}
			m_sMissionAudio.m_nPlayStatus[slot] = PLAY_STATUS_PLAYING;
			nCheckPlayingDelay[slot] = 30;
			if (m_sMissionAudio.m_nSampleIndex[slot] >= STREAMED_SOUND_MISSION_MOB_01A && m_sMissionAudio.m_nSampleIndex[slot] <= STREAMED_SOUND_MISSION_MOB_99A)
				m_sMissionAudio.m_bIsMobile[slot] = true;
			break;
		case PLAY_STATUS_PLAYING:
			if (m_bTimerJustReset) {
				ClearMissionAudio(slot);
				SampleManager.StopStreamedFile(slot + 1);
				break;
			}
			if (MissionScriptAudioUsesPoliceChannel(m_sMissionAudio.m_nSampleIndex[slot])) {
				if (!m_nUserPause) {
					if (nCheckPlayingDelay[slot]) {
						--nCheckPlayingDelay[slot];
					} else if (GetMissionScriptPoliceAudioPlayingStatus() == PLAY_STATUS_FINISHED || m_sMissionAudio.m_nMissionAudioCounter[slot]-- == 0) {
						m_sMissionAudio.m_nPlayStatus[slot] = PLAY_STATUS_FINISHED;
						m_sMissionAudio.m_nSampleIndex[slot] = NO_SAMPLE;
						SampleManager.StopStreamedFile(slot + 1);
						m_sMissionAudio.m_nMissionAudioCounter[slot] = 0;
					}
				}
			} else if (m_sMissionAudio.field_22[slot]) {
				if (SampleManager.IsStreamPlaying(slot + 1) || m_nUserPause || m_nPreviousUserPause) {
					if (m_nUserPause)
						SampleManager.PauseStream(1, slot + 1);
					else
					{
						SampleManager.PauseStream(0, slot + 1);
						if (!m_sMissionAudio.m_bPredefinedProperties) {
							distSquared = GetDistanceSquared(m_sMissionAudio.m_vecPos[slot]);
							if (distSquared >= SQR(80.0f)) {
								emittingVol = 0;
								pan = 63;
							} else {
								emittingVol = 127;
								if (distSquared > 0.0f) {
									dist = Sqrt(distSquared);
									emittingVol = ComputeVolume(127, 80.0f, dist);
								}
								TranslateEntity(&m_sMissionAudio.m_vecPos[slot], &vec);
								pan = ComputePan(80.f, &vec);
							}
							SampleManager.SetStreamedVolumeAndPan(emittingVol, pan, 1, slot + 1);
						}
					}
				} else if (m_sMissionAudio.m_nSampleIndex[slot] == STREAMED_SOUND_MISSION_ROK2_01) {
					m_sMissionAudio.m_nPlayStatus[slot] = PLAY_STATUS_STOPPED;
				} else {
					m_sMissionAudio.m_nPlayStatus[slot] = PLAY_STATUS_FINISHED;
					if (m_sMissionAudio.m_nSampleIndex[slot] >= STREAMED_SOUND_MISSION_MOB_01A && m_sMissionAudio.m_nSampleIndex[slot] <= STREAMED_SOUND_MISSION_MOB_99A)
						m_sMissionAudio.m_bIsMobile[slot] = false;
					m_sMissionAudio.m_nSampleIndex[slot] = NO_SAMPLE;
					SampleManager.StopStreamedFile(slot + 1);
					m_sMissionAudio.m_nMissionAudioCounter[slot] = 0;
				}
			} else {
				if (m_nUserPause)
					break;
				if (nCheckPlayingDelay[slot]--) {
					if (!SampleManager.IsStreamPlaying(slot + 1))
						break;
					nCheckPlayingDelay[slot] = 0;
				}
				m_sMissionAudio.field_22[slot] = 1;
			}
			break;
		default:
			break;
		}
		break;
	case LOADING_STATUS_FAILED:
		if (++nFramesUntilFailedLoad[slot] >= 90) {
			nFramesForPretendPlaying[slot] = 0;
			g_bMissionAudioLoadFailed[slot] = true;
			nFramesUntilFailedLoad[slot] = 0;
			m_sMissionAudio.m_nLoadingStatus[slot] = LOADING_STATUS_LOADED;
		}
		break;
	default:
		break;
	}
}

void
cAudioManager::ProcessMissionAudio()
{
	if (!m_bIsInitialised) return;
	
	for (int i = 0; i < MISSION_AUDIO_SLOTS; i++)
		ProcessMissionAudioSlot(i);

	if (m_sMissionAudio.m_bIsMobile[0] || m_sMissionAudio.m_bIsMobile[1])
		field_5538 = 64;
	else if (field_5538 < 127) {
		field_5538 += 5;
		if (field_5538 > 127)
			field_5538 = 127;
	}
}
#pragma endregion All the mission audio stuff