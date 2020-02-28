#include "common.h"
#include "patcher.h"
#include "DMAudio.h"
#include "Entity.h"
#include "AudioCollision.h"
#include "AudioManager.h"
#include "AudioSamples.h"
#include "SurfaceTable.h"
#include "sampman.h"

constexpr int CollisionSoundIntensity = 60;

void
cAudioCollisionManager::AddCollisionToRequestedQueue()
{
	int32 collisionsIndex;
	int32 i;


	if (m_bCollisionsInQueue < NUMAUDIOCOLLISIONS)
		collisionsIndex = m_bCollisionsInQueue++;
	else {
		collisionsIndex = m_bIndicesTable[NUMAUDIOCOLLISIONS - 1];
		if (m_sQueue.m_fDistance >= m_asCollisions1[collisionsIndex].m_fDistance) return;
	}

	m_asCollisions1[collisionsIndex] = m_sQueue;

	i = 0;
	if(collisionsIndex) {
		while(m_asCollisions1[m_bIndicesTable[i]].m_fDistance <= m_asCollisions1[collisionsIndex].m_fDistance) {
			if(++i >= collisionsIndex) {
				m_bIndicesTable[i] = collisionsIndex;
				return;
			}
		}
		memmove(&m_bIndicesTable[i + 1], &m_bIndicesTable[i], NUMAUDIOCOLLISIONS - 1 - i);
	}
	m_bIndicesTable[i] = collisionsIndex;
}

float
cAudioManager::GetCollisionLoopingRatio(uint32 a, uint32 b, float c) const
{
	return GetCollisionRatio(c, 0.0f, 0.02f, 0.02f);
}

float
cAudioManager::GetCollisionOneShotRatio(int32 a, float b) const
{
	float result;

	switch(a) {
	case SURFACE_DEFAULT:
	case SURFACE_TARMAC:
	case SURFACE_PAVEMENT:
	case SURFACE_STONE:
	case SURFACE_BOLLARD: result = GetCollisionRatio(b, 10.f, 60.f, 50.f); break;
	case SURFACE_GRASS:
	case SURFACE_LOOSE30: result = GetCollisionRatio(b, 0.f, 2.f, 2.f); break;
	case SURFACE_DIRT: result = GetCollisionRatio(b, 0.f, 2.f, 2.f); break;
	case SURFACE_DIRTTRACK: result = GetCollisionRatio(b, 0.f, 2.f, 2.f); break;
	case SURFACE_METAL6: result = GetCollisionRatio(b, 6.f, 50.f, 44.f); break;
	case SURFACE_GLASS: result = GetCollisionRatio(b, 0.1f, 10.f, 9.9f); break;
	case SURFACE_SCAFFOLD:
	case SURFACE_STEEL: result = GetCollisionRatio(b, 30.f, 130.f, 100.f); break;
	case SURFACE_METAL_DOOR: result = GetCollisionRatio(b, 20.f, 100.f, 80.f); break;
	case SURFACE_BILLBOARD: result = GetCollisionRatio(b, 0.f, 4.f, 4.f); break;
	case SURFACE_METAL_POLE:
	case SURFACE_GATE: result = GetCollisionRatio(b, 1.f, 10.f, 9.f); break;
	case SURFACE_STREET_LIGHT: result = GetCollisionRatio(b, 1.f, 10.f, 9.f); break;
	case SURFACE_METAL14: result = GetCollisionRatio(b, 1.f, 15.f, 14.f); break;
	case SURFACE_METAL15: result = GetCollisionRatio(b, 8.f, 50.f, 42.f); break;
	case SURFACE_METAL_FENCE: result = GetCollisionRatio(b, 0.1f, 10.f, 9.9f); break;
	case SURFACE_FLESH: result = GetCollisionRatio(b, 0.f, 20.f, 20.f); break;
	case SURFACE_SAND: result = GetCollisionRatio(b, 0.f, 10.f, 10.f); break;
	case SURFACE_PUDDLE: result = GetCollisionRatio(b, 0.f, 10.f, 10.f); break;
	case SURFACE_WOOD: result = GetCollisionRatio(b, 1.f, 4.f, 3.f); break;
	case SURFACE_WOOD_BOX: result = GetCollisionRatio(b, 0.1f, 5.f, 4.9f); break;
	case SURFACE_WOOD_PLANK: result = GetCollisionRatio(b, 0.1f, 40.f, 39.9f); break;
	case SURFACE_TIRE:
	case SURFACE_RUBBER29: result = GetCollisionRatio(b, 0.f, 10.f, 10.f); break;
	case SURFACE_HARD24: result = GetCollisionRatio(b, 0.1f, 4.f, 3.9f); break;
	case SURFACE_HEDGE: result = GetCollisionRatio(b, 0.f, 0.5f, 0.5f); break;
	case SURFACE_METAL27: result = GetCollisionRatio(b, 4.f, 40.f, 36.f); break;
	case SURFACE_METAL28: result = GetCollisionRatio(b, 0.f, 5.f, 5.f); break;
	default: result = 0.f; break;
	}

	return result;
}

float
cAudioManager::GetCollisionRatio(float a, float b, float c, float d) const
{
	float e;
	e = a;
	if(a <= b) return 0.0f;
	if(c <= a) e = c;
	return (e - b) / d;
}

uint32
cAudioManager::SetLoopingCollisionRequestedSfxFreqAndGetVol(cAudioCollision *audioCollision)
{
	uint8 surface1 = audioCollision->m_bSurface1;
	uint8 surface2 = audioCollision->m_bSurface2;
	int32 vol;
	float ratio;

	if(surface1 == SURFACE_GRASS || surface2 == SURFACE_GRASS || surface1 == SURFACE_HEDGE ||
	   surface2 == SURFACE_HEDGE) {
		ratio = GetCollisionRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
		m_sQueueSample.m_nSampleIndex = SFX_RAIN;
		m_sQueueSample.m_nFrequency = 13000.f * ratio + 35000;
		vol = 50.f * ratio;
	} else {
		if(surface1 == SURFACE_PUDDLE || surface2 == SURFACE_PUDDLE) {
			ratio = GetCollisionRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
			m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
			m_sQueueSample.m_nFrequency = 6050.f * ratio + 16000;
			vol = 30.f * ratio;

		} else {
			if(surface1 == SURFACE_DIRT || surface2 == SURFACE_DIRT || surface1 == SURFACE_DIRTTRACK ||
			   surface2 == SURFACE_DIRTTRACK || surface1 == SURFACE_SAND || surface2 == SURFACE_SAND) {
				ratio = GetCollisionRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
				m_sQueueSample.m_nSampleIndex = SFX_GRAVEL_SKID;
				m_sQueueSample.m_nFrequency = 6000.f * ratio + 10000;
				vol = 50.f * ratio;
			} else {
				if(surface1 == SURFACE_FLESH || surface2 == SURFACE_FLESH) { return 0; }
				ratio = GetCollisionRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
				m_sQueueSample.m_nSampleIndex = SFX_SCRAPE_CAR_1;
				m_sQueueSample.m_nFrequency = 10000.f * ratio + 10000;
				vol = 40.f * ratio;
			}
		}
	}
	if(audioCollision->m_nBaseVolume < 2) vol = audioCollision->m_nBaseVolume * vol / 2;
	return vol;
}

void
cAudioManager::SetUpLoopingCollisionSound(cAudioCollision *col, uint8 counter)
{
	if(col->m_fIntensity2 > 0.0016f) {
		uint8 emittingVol = SetLoopingCollisionRequestedSfxFreqAndGetVol(col);
		if(emittingVol) {
			m_sQueueSample.m_fDistance = Sqrt(col->m_fDistance);
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, CollisionSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_counter = counter;
				m_sQueueSample.m_vecPos = col->m_vecPosition;
				m_sQueueSample.m_bBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_bIsDistant = false;
				m_sQueueSample.field_16 = 7;
				m_sQueueSample.m_nLoopCount = 0;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart =
				    SampleManager.GetSampleLoopStartOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.m_nLoopEnd =
				    SampleManager.GetSampleLoopEndOffset(m_sQueueSample.m_nSampleIndex);
				m_sQueueSample.field_48 = 4.0f;
				m_sQueueSample.m_fSoundIntensity = CollisionSoundIntensity;
				m_sQueueSample.field_56 = 0;
				m_sQueueSample.field_76 = 5;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
		}
	}
}

void
cAudioManager::SetUpOneShotCollisionSound(cAudioCollision *col)
{
	static const int32 gOneShotCol[] = {
	    SFX_COL_TARMAC_1,   SFX_COL_TARMAC_1,   SFX_COL_GRASS_1,
	    SFX_COL_GRAVEL_1,       SFX_COL_MUD_1,        SFX_COL_TARMAC_1,
	    SFX_COL_CAR_1,      SFX_COL_GRASS_1,    SFX_COL_SCAFFOLD_POLE_1,
	    SFX_COL_GARAGE_DOOR_1, SFX_COL_CAR_PANEL_1,  SFX_COL_THICK_METAL_PLATE_1,
	    SFX_COL_SCAFFOLD_POLE_1,       SFX_COL_LAMP_POST_1,   SFX_COL_HYDRANT_1,
	    SFX_COL_HYDRANT_1,   SFX_COL_METAL_CHAIN_FENCE_1,      SFX_COL_PED_1,
	    SFX_COL_SAND_1,       SFX_SPLASH_1,      SFX_COL_WOOD_CRATES_1,
	    SFX_COL_WOOD_BENCH_1,   SFX_COL_WOOD_SOLID_1, SFX_COL_GRASS_1,
	    SFX_COL_GRASS_1,    SFX_COL_VEG_1,      SFX_COL_TARMAC_1,
	    SFX_COL_CONTAINER_1,   SFX_COL_NEWS_VENDOR_1,   SFX_TYRE_BUMP,
	    SFX_COL_CARDBOARD_1,      SFX_COL_TARMAC_1,   SFX_COL_GATE};

	int16 s1;
	int16 s2;

	int32 emittingVol;
	float ratio;

	static uint16 counter = 28;

	for(int32 i = 0; i < 2; i++) {
		if(i) {
			s1 = col->m_bSurface2;
			s2 = col->m_bSurface1;
		} else {
			s1 = col->m_bSurface1;
			s2 = col->m_bSurface2;
		}
		ratio = GetCollisionOneShotRatio(s1, col->m_fIntensity1);
		if(s1 == SURFACE_METAL6 && s2 == SURFACE_FLESH) ratio = 0.25f * ratio;
		if(s1 == SURFACE_METAL6 && ratio < 0.6f) {
			s1 = SURFACE_BILLBOARD;
			ratio = min(1.f, 2.f * ratio);
		}
		emittingVol = 40.f * ratio;
		if(emittingVol) {
			m_sQueueSample.m_fDistance = Sqrt(col->m_fDistance);
			m_sQueueSample.m_bVolume =
			    ComputeVolume(emittingVol, CollisionSoundIntensity, m_sQueueSample.m_fDistance);
			if(m_sQueueSample.m_bVolume) {
				m_sQueueSample.m_nSampleIndex = gOneShotCol[s1];
				switch(m_sQueueSample.m_nSampleIndex) {
				case SFX_COL_TARMAC_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[3] % 5;
					break;
				case SFX_COL_CAR_PANEL_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[0] % 6;
					break;
				case SFX_COL_LAMP_POST_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[1] % 2;
					break;
				case SFX_COL_METAL_CHAIN_FENCE_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[3] % 4;
					break;
				case SFX_COL_PED_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[4] % 5;
					break;
				case SFX_COL_WOOD_CRATES_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[4] % 4;
					break;
				case SFX_COL_WOOD_BENCH_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[1] % 4;
					break;
				case SFX_COL_VEG_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[2] % 5;
					break;
				case SFX_COL_NEWS_VENDOR_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[2] % 3;
					break;
				case SFX_COL_CAR_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[1] % 5;
					break;
				case SFX_COL_CARDBOARD_1:
					m_sQueueSample.m_nSampleIndex += m_anRandomTable[3] % 2;
					break;
				default: break;
				}
				switch(s1) {
				case SURFACE_GLASS: m_sQueueSample.m_nFrequency = 13500; break;
				case SURFACE_METAL15: m_sQueueSample.m_nFrequency = 8819; break;
				case SURFACE_PUDDLE:
					m_sQueueSample.m_nFrequency =
					    2 * SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
					break;
				case SURFACE_TIRE: m_sQueueSample.m_nFrequency = 6000; break;
				case SURFACE_HARD24: m_sQueueSample.m_nFrequency = 8000; break;
				default:
					m_sQueueSample.m_nFrequency =
					    SampleManager.GetSampleBaseFrequency(m_sQueueSample.m_nSampleIndex);
					break;
				}
				m_sQueueSample.m_nFrequency += RandomDisplacement(m_sQueueSample.m_nFrequency / 16);
				m_sQueueSample.m_counter = counter++;
				if(counter >= 255) counter = 28;
				m_sQueueSample.m_vecPos = col->m_vecPosition;
				m_sQueueSample.m_bBankIndex = SAMPLEBANK_MAIN;
				m_sQueueSample.m_bIsDistant = false;
				m_sQueueSample.field_16 = 11;
				m_sQueueSample.m_nLoopCount = 1;
				m_sQueueSample.m_bEmittingVolume = emittingVol;
				m_sQueueSample.m_nLoopStart = 0;
				m_sQueueSample.m_nLoopEnd = -1;
				m_sQueueSample.field_48 = 4.0f;
				m_sQueueSample.m_fSoundIntensity = CollisionSoundIntensity;
				m_sQueueSample.field_56 = 1;
				m_sQueueSample.m_bReverbFlag = true;
				m_sQueueSample.m_bRequireReflection = false;
				AddSampleToRequestedQueue();
			}
		}
	}
}

void
cAudioManager::ServiceCollisions()
{
	int i, j;
	bool someArr1[NUMAUDIOCOLLISIONS];
	bool someArr2[NUMAUDIOCOLLISIONS];

	m_sQueueSample.m_nEntityIndex = m_nCollisionEntity;

	for (int i = 0; i < NUMAUDIOCOLLISIONS; i++)
		someArr1[i] = someArr2[i] = false;

	for (i = 0; i < m_sCollisionManager.m_bCollisionsInQueue; i++) {
		for (j = 0; j < NUMAUDIOCOLLISIONS; j++) {
			int index = m_sCollisionManager.m_bIndicesTable[i];
			if ((m_sCollisionManager.m_asCollisions1[index].m_pEntity1 == m_sCollisionManager.m_asCollisions2[j].m_pEntity1)
				&& (m_sCollisionManager.m_asCollisions1[index].m_pEntity2 == m_sCollisionManager.m_asCollisions2[j].m_pEntity2)
				&& (m_sCollisionManager.m_asCollisions1[index].m_bSurface1 == m_sCollisionManager.m_asCollisions2[j].m_bSurface1)
				&& (m_sCollisionManager.m_asCollisions1[index].m_bSurface2 == m_sCollisionManager.m_asCollisions2[j].m_bSurface2)
				) {
				someArr1[index] = true;
				someArr2[j] = true;
				m_sCollisionManager.m_asCollisions1[index].m_nBaseVolume = ++m_sCollisionManager.m_asCollisions2[j].m_nBaseVolume;
				SetUpLoopingCollisionSound(&m_sCollisionManager.m_asCollisions1[index], j);
				break;
			}
		}
	}

	for (i = 0; i < NUMAUDIOCOLLISIONS; i++) {
		if (!someArr2[i]) {
			m_sCollisionManager.m_asCollisions2[i].m_pEntity1 = nil;
			m_sCollisionManager.m_asCollisions2[i].m_pEntity2 = nil;
			m_sCollisionManager.m_asCollisions2[i].m_bSurface1 = SURFACE_DEFAULT;
			m_sCollisionManager.m_asCollisions2[i].m_bSurface2 = SURFACE_DEFAULT;
			m_sCollisionManager.m_asCollisions2[i].m_fIntensity2 = 0.0f;
			m_sCollisionManager.m_asCollisions2[i].m_fIntensity1 = 0.0f;
			m_sCollisionManager.m_asCollisions2[i].m_vecPosition = CVector(0.0f, 0.0f, 0.0f);
			m_sCollisionManager.m_asCollisions2[i].m_fDistance = 0.0f;
		}
	}

	for (i = 0; i < m_sCollisionManager.m_bCollisionsInQueue; i++) {
		int index = m_sCollisionManager.m_bIndicesTable[i];
		if (!someArr1[index]) {
			for (j = 0; j < NUMAUDIOCOLLISIONS; j++) {
				if (someArr2[j]) {
					m_sCollisionManager.m_asCollisions2[j].m_nBaseVolume = 1;
					m_sCollisionManager.m_asCollisions2[j].m_pEntity1 = m_sCollisionManager.m_asCollisions1[index].m_pEntity1;
					m_sCollisionManager.m_asCollisions2[j].m_pEntity2 = m_sCollisionManager.m_asCollisions1[index].m_pEntity2;
					m_sCollisionManager.m_asCollisions2[j].m_bSurface1 = m_sCollisionManager.m_asCollisions1[index].m_bSurface1;
					m_sCollisionManager.m_asCollisions2[j].m_bSurface2 = m_sCollisionManager.m_asCollisions1[index].m_bSurface2;
					break;
				}
			}
			SetUpOneShotCollisionSound(&m_sCollisionManager.m_asCollisions1[index]);
			SetUpLoopingCollisionSound(&m_sCollisionManager.m_asCollisions1[index], j);
		}
	}

	for (int i = 0; i < NUMAUDIOCOLLISIONS; i++)
		m_sCollisionManager.m_bIndicesTable[i] = NUMAUDIOCOLLISIONS;
	m_sCollisionManager.m_bCollisionsInQueue = 0;
}

void
cAudioManager::ReportCollision(CEntity *entity1, CEntity *entity2, uint8 surface1, uint8 surface2, float collisionPower,
                               float velocity)
{
	float distSquared;
	CVector v1;
	CVector v2;

	if(!m_bIsInitialised || m_nCollisionEntity < 0 || m_bUserPause ||
	   (velocity < 0.0016f && collisionPower < 0.01f))
		return;

	if(entity1->IsBuilding()) {
		v1 = v2 = entity2->GetPosition();
	} else if(entity2->IsBuilding()) {
		v1 = v2 = entity1->GetPosition();
	} else {
		v1 = entity1->GetPosition();
		v2 = entity2->GetPosition();
	}
	CVector pos = (v1 + v2) * 0.5f;
	distSquared = GetDistanceSquared(&pos);
	if(distSquared < SQR(CollisionSoundIntensity)) {
		m_sCollisionManager.m_sQueue.m_pEntity1 = entity1;
		m_sCollisionManager.m_sQueue.m_pEntity2 = entity2;
		m_sCollisionManager.m_sQueue.m_bSurface1 = surface1;
		m_sCollisionManager.m_sQueue.m_bSurface2 = surface2;
		m_sCollisionManager.m_sQueue.m_fIntensity1 = collisionPower;
		m_sCollisionManager.m_sQueue.m_fIntensity2 = velocity;
		m_sCollisionManager.m_sQueue.m_vecPosition = pos;
		m_sCollisionManager.m_sQueue.m_fDistance = distSquared;
		m_sCollisionManager.AddCollisionToRequestedQueue();
	}
}

STARTPATCHES
InjectHook(0x5685E0, &cAudioCollisionManager::AddCollisionToRequestedQueue, PATCH_JUMP);
InjectHook(0x569060, &cAudioManager::GetCollisionOneShotRatio, PATCH_JUMP);
InjectHook(0x5693B0, &cAudioManager::GetCollisionRatio, PATCH_JUMP);
InjectHook(0x568410, &cAudioManager::ReportCollision, PATCH_JUMP);
InjectHook(0x5686D0, &cAudioManager::ServiceCollisions, PATCH_JUMP);
InjectHook(0x568E20, &cAudioManager::SetLoopingCollisionRequestedSfxFreqAndGetVol, PATCH_JUMP);
InjectHook(0x568D30, &cAudioManager::SetUpLoopingCollisionSound, PATCH_JUMP);
InjectHook(0x5689D0, &cAudioManager::SetUpOneShotCollisionSound, PATCH_JUMP);
ENDPATCHES
