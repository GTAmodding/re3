#include "common.h"

#include "audio_enums.h"   //needs to be before AudioManager.h
#include "vehicle_enums.h" //needs to be before AudioManager.h
#include "ParticleType.h"

#include "AudioManager.h"
#include "Camera.h"
#include "DMAudio.h"
#include "Game.h"
#include "General.h"
#include "Particle.h"
#include "ParticleMgr.h"
#include "ParticleObject.h"
#include "Placeable.h"
#include "Timer.h"
#include "screendroplets.h"

CParticleObject gPObjectArray[MAX_PARTICLEOBJECTS];

CParticleObject *CParticleObject::pCloseListHead;
CParticleObject *CParticleObject::pFarListHead;
CParticleObject *CParticleObject::pUnusedListHead;

CAudioHydrant List[MAX_AUDIOHYDRANTS];

CAudioHydrant *CAudioHydrant::Get(int n) { return &List[n]; }

CAudioHydrant::CAudioHydrant() : AudioEntity(AEHANDLE_NONE), pParticleObject(nil) {}

bool
CAudioHydrant::Add(CParticleObject *particleobject)
{
	for(int32 i = 0; i < MAX_AUDIOHYDRANTS; i++) {
		if ( List[i].AudioEntity == AEHANDLE_NONE )
		{
			List[i].AudioEntity = DMAudio.CreateEntity(AUDIOTYPE_FIREHYDRANT, particleobject);

			if ( AEHANDLE_IS_FAILED(List[i].AudioEntity) )
				return false;
			
			DMAudio.SetEntityStatus(List[i].AudioEntity, true);
			
			List[i].pParticleObject = particleobject;
			
			return true;
		}
	}

	return false;
}

void
CAudioHydrant::Remove(CParticleObject *particleobject)
{
	for ( int32 i = 0; i < MAX_AUDIOHYDRANTS; i++ )
	{
		if ( List[i].pParticleObject == particleobject )
		{
			DMAudio.DestroyEntity(List[i].AudioEntity);
			List[i].AudioEntity = AEHANDLE_NONE;
			List[i].pParticleObject = nil;
		}
	}
}

CParticleObject::CParticleObject() :
	CPlaceable(),
	m_nFrameCounter(0),
	m_nState(POBJECTSTATE_INITIALISED),
	m_pNext(nil),
	m_pPrev(nil),
	m_nRemoveTimer(0)
	
{
	;
}

CParticleObject::~CParticleObject()
{
	
}

void
CParticleObject::Initialise()
{
	pCloseListHead = nil;
	pFarListHead   = nil;
	
	pUnusedListHead = &gPObjectArray[0];
	
	for ( int32 i = 0; i < MAX_PARTICLEOBJECTS; i++ )
	{
		if ( i == 0 )
			gPObjectArray[i].m_pPrev = nil;
		else
			gPObjectArray[i].m_pPrev = &gPObjectArray[i - 1];
		
		if ( i == MAX_PARTICLEOBJECTS-1 )
			gPObjectArray[i].m_pNext = nil;
		else
			gPObjectArray[i].m_pNext = &gPObjectArray[i + 1];
		
		gPObjectArray[i].m_nState = POBJECTSTATE_FREE;
	}
}

CParticleObject *
CParticleObject::AddObject(uint16 type, CVector const &pos, uint8 remove)
{
	CRGBA color(0, 0, 0, 0);
	CVector target(0.0f, 0.0f, 0.0f);
	return AddObject(type, pos, target, 0.0f, 0, color, remove);
}

CParticleObject *
CParticleObject::AddObject(uint16 type, CVector const &pos, float size, uint8 remove)
{
	CRGBA color(0, 0, 0, 0);
	CVector target(0.0f, 0.0f, 0.0f);
	return AddObject(type, pos, target, size, 0, color, remove);
}

CParticleObject *
CParticleObject::AddObject(uint16 type, CVector const &pos, CVector const &target, float size, uint8 remove)
{
	CRGBA color(0, 0, 0, 0);
	return AddObject(type, pos, target, size, 0, color, remove);
}

CParticleObject *
CParticleObject::AddObject(uint16 type, CVector const &pos, CVector const &target, float size, uint32 lifeTime, RwRGBA const &color, uint8 remove)
{
	CParticleObject *pobj = pUnusedListHead;
	
	if ( pobj == nil )
	{
		printf("Error: No particle objects available!\n");
		return nil;
	}
	
	MoveToList(&pUnusedListHead, &pCloseListHead, pobj);
	
	pobj->m_nState           = POBJECTSTATE_UPDATE_CLOSE;
	pobj->m_Type             = (eParticleObjectType)type;
	
	pobj->SetPosition(pos);
	pobj->m_vecTarget        = target;
	
	pobj->m_nNumEffectCycles = 1;
	pobj->m_nSkipFrames      = 1;
	pobj->m_nCreationChance  = 0;
	pobj->m_nFrameCounter    = 0;
	
	pobj->m_bRemove          = remove;
	
	pobj->m_pParticle        = nil;
	
	if ( lifeTime != 0 )
		pobj->m_nRemoveTimer = CTimer::GetTimeInMilliseconds() + lifeTime;
	else
		pobj->m_nRemoveTimer = 0;
	
	if ( color.alpha != 0 )
		pobj->m_Color = color;
	else
		pobj->m_Color.alpha  = 0;
	
	pobj->m_fSize            = size;
	pobj->m_fRandVal         = 0.0f;
	
	switch ( type )
	{
		case POBJECT_PAVEMENT_STEAM:
		{
			pobj->m_ParticleType     = PARTICLE_STEAM_NY;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 3;
			pobj->m_nCreationChance  = 8;
			break;
		}
		
		case POBJECT_PAVEMENT_STEAM_SLOWMOTION:
		{
			pobj->m_ParticleType     = PARTICLE_STEAM_NY_SLOWMOTION;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 8;
			break;
		}
		
		case POBJECT_WALL_STEAM:
		{
			pobj->m_ParticleType     = PARTICLE_STEAM_NY;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 3;
			pobj->m_nCreationChance  = 8;
			break;
		}
		
		case POBJECT_WALL_STEAM_SLOWMOTION:
		{
			pobj->m_ParticleType     = PARTICLE_STEAM_NY_SLOWMOTION;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 8;
			break;
		}
		
		case POBJECT_DARK_SMOKE:
		{
			pobj->m_ParticleType     = PARTICLE_STEAM_NY;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 3;
			pobj->m_nCreationChance  = 8;
			pobj->m_Color            = CRGBA(16, 16, 16, 255);
			break;
		}
		
		case POBJECT_WATER_FOUNTAIN_VERT:
		{
			pobj->m_ParticleType     = PARTICLE_WATER_HYDRANT;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 0;
			pobj->m_vecTarget        = CVector(0.0f, 0.0f, 0.1f);
			break;
		}
		
		case POBJECT_WATER_FOUNTAIN_HORIZ:
		{
			pobj->m_ParticleType     = PARTICLE_WATER_HYDRANT;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 0;
			break;
		}
		
		case POBJECT_FIRE_HYDRANT:
		{
			pobj->m_ParticleType     = PARTICLE_WATER_HYDRANT;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 0;
			pobj->m_vecTarget        = CVector(0.0f, 0.0f, 0.3f);
			pobj->m_nRemoveTimer     = CTimer::GetTimeInMilliseconds() + 5000;
			CAudioHydrant::Add(pobj);
			break;
		}
		
		case POBJECT_CAR_WATER_SPLASH:
		case POBJECT_PED_WATER_SPLASH:
		{
			pobj->m_ParticleType     = PARTICLE_CAR_SPLASH;
			pobj->m_nNumEffectCycles = 0;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 0;
#ifdef SCREEN_DROPLETS
			ScreenDroplets::RegisterSplash(pobj);
#endif
			break;
		}
		
		case POBJECT_SPLASHES_AROUND:
		{
			pobj->m_ParticleType     = PARTICLE_SPLASH;
			pobj->m_nNumEffectCycles = 15;
			pobj->m_nSkipFrames      = 2;
			pobj->m_nCreationChance  = 0;
			break;
		}
		
		case POBJECT_SMALL_FIRE:
		{
			pobj->m_ParticleType     = PARTICLE_FLAME;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 2;
			pobj->m_nCreationChance  = 2;
			pobj->m_vecTarget        = CVector(0.0f, 0.0f, 0.0f);
			break;
		}
		
		case POBJECT_BIG_FIRE:
		{
			pobj->m_ParticleType     = PARTICLE_FLAME;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 2;
			pobj->m_nCreationChance  = 4;
			pobj->m_vecTarget        = CVector(0.0f, 0.0f, 0.0f);
			break;
		}
		
		case POBJECT_DRY_ICE:
		{
			pobj->m_ParticleType     = PARTICLE_SMOKE;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 0;
			pobj->m_vecTarget        = CVector(0.0f, 0.0f, 0.0f);
			break;
		}
		
		case POBJECT_DRY_ICE_SLOWMOTION:
		{
			pobj->m_ParticleType     = PARTICLE_SMOKE_SLOWMOTION;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 0;
			pobj->m_vecTarget        = CVector(0.0f, 0.0f, 0.0f);
			break;
		}
		
		case POBJECT_FIRE_TRAIL:
		{
			pobj->m_ParticleType     = PARTICLE_EXPLOSION_MEDIUM;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 3;
			pobj->m_nCreationChance  = 2;
			pobj->m_fRandVal         = 0.01f;
			break;
		}
		
		case POBJECT_SMOKE_TRAIL:
		{
			pobj->m_ParticleType     = PARTICLE_FIREBALL_SMOKE;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 2;
			pobj->m_fRandVal         = 0.02f;
			break;
		}
		
		case POBJECT_FIREBALL_AND_SMOKE:
		{
			pobj->m_ParticleType     = PARTICLE_FLAME;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 0;
			pobj->m_fRandVal         = 0.1f;
			break;
		}
		
		case POBJECT_ROCKET_TRAIL:
		{
			pobj->m_ParticleType     = PARTICLE_FLAME;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 2;
			pobj->m_nCreationChance  = 8;
			pobj->m_fRandVal         = 0.1f;
			break;
		}
		
		case POBJECT_EXPLOSION_ONCE:
		{
			pobj->m_ParticleType     = PARTICLE_EXPLOSION_LARGE;
			pobj->m_nNumEffectCycles = 1;
			pobj->m_nSkipFrames      = 1;
			pobj->m_nCreationChance  = 0;
			pobj->m_nRemoveTimer     = CTimer::GetTimeInMilliseconds();
			break;
		}
	}
	
	return pobj;
}

CParticleObject *
CParticleObject::AddObject(tParticleType type, CVector const &pos, CVector const &target, float size, uint32 lifeTime, uint8 numEffectCycles, uint8 skipFrames, uint16 creationChance, uint8 remove)
{
	CParticleObject *pobj = pUnusedListHead;
	
	ASSERT(pobj != nil);
	
	if ( pobj == nil )
	{
		printf("Error: No particle objects available!\n");
		return nil;
	}
	
	MoveToList(&pUnusedListHead, &pCloseListHead, pobj);
	
	pobj->m_nState           = POBJECTSTATE_UPDATE_CLOSE;
	pobj->m_Type             = (eParticleObjectType)-1;
	pobj->m_ParticleType     = type;
	
	pobj->SetPosition(pos);
	pobj->m_vecTarget        = target;
	
	pobj->m_nNumEffectCycles = numEffectCycles;
	pobj->m_nSkipFrames      = skipFrames;
	pobj->m_nCreationChance  = creationChance;
	pobj->m_nFrameCounter    = 0;
	
	pobj->m_bRemove          = remove;
	
	if ( lifeTime != 0 )
		pobj->m_nRemoveTimer = CTimer::GetTimeInMilliseconds() + lifeTime;
	else
		pobj->m_nRemoveTimer = 0;
	
	pobj->m_Color.alpha  = 0;
	
	pobj->m_fSize            = size;
	pobj->m_fRandVal         = 0.0f;

	return pobj;
}

void
CParticleObject::RemoveObject(void)
{
	switch ( this->m_nState )
	{
		case POBJECTSTATE_UPDATE_CLOSE:
		{
			MoveToList(&pCloseListHead, &pUnusedListHead, this);
			this->m_nState = POBJECTSTATE_FREE;
			break;
		}
		case POBJECTSTATE_UPDATE_FAR:
		{
			MoveToList(&pFarListHead, &pUnusedListHead, this);
			this->m_nState = POBJECTSTATE_FREE;
			break;
		}
	}
}

void
CParticleObject::UpdateAll(void)
{
	{
		CParticleObject *pobj = pCloseListHead;
		CParticleObject *nextpobj;
		if ( pobj != nil )
		{
			do
			{
				nextpobj = pobj->m_pNext;
				pobj->UpdateClose();
				pobj = nextpobj;
			}
			while ( nextpobj != nil );
		}
	}

	{
		int32 frame = CTimer::GetFrameCounter() & 31;
		int32 counter = 0;
	
		CParticleObject *pobj = pFarListHead;
		CParticleObject *nextpobj;
		if ( pobj != nil )
		{
			do
			{
				nextpobj = pobj->m_pNext;
				
				if ( counter == frame )
				{
					pobj->UpdateFar();
					frame += 32;
				}
				
				counter++;
				
				pobj = nextpobj;
			}
			while ( nextpobj != nil );
		}
	}
}

void CParticleObject::UpdateClose(void)
{
	if ( !CGame::playingIntro )
	{
		if ( (this->GetPosition() - TheCamera.GetPosition()).MagnitudeSqr2D() > SQR(100.0f) )
		{
			if ( this->m_bRemove )
			{
				if ( this->m_Type == POBJECT_FIRE_HYDRANT )
					CAudioHydrant::Remove(this);
				
				MoveToList(&pCloseListHead, &pUnusedListHead, this);
				this->m_nState = POBJECTSTATE_FREE;
			}
			else
			{
				MoveToList(&pCloseListHead, &pFarListHead, this);
				this->m_nState = POBJECTSTATE_UPDATE_FAR;
			}
			
			return;
		}
	}
	
	if ( ++this->m_nFrameCounter >= this->m_nSkipFrames )
    {
		this->m_nFrameCounter = 0;
		
		int32 randVal;
		if ( this->m_nCreationChance != 0 )
			randVal = CGeneral::GetRandomNumber() % this->m_nCreationChance;
		
		if (   this->m_nCreationChance == 0
			|| randVal == 0 && this->m_nCreationChance < 0
			|| randVal != 0 && this->m_nCreationChance > 0)
		{
			switch ( this->m_Type )
			{
				case POBJECT_SMALL_FIRE:
				{
					CVector pos = this->GetPosition();
					CVector vel = this->m_vecTarget;
					float size  = this->m_fSize;
					
					CVector flamevel;
					
					flamevel.x = vel.x;
					flamevel.y = vel.y;
					flamevel.z = CGeneral::GetRandomNumberInRange(0.0125f*size, 0.1f*size);
						
					CParticle::AddParticle(PARTICLE_FLAME, pos, flamevel, nil, size);
					
					
					CVector possmoke = pos;
					
					possmoke.x += CGeneral::GetRandomNumberInRange(0.625f*-size, size*0.625f);
					possmoke.y += CGeneral::GetRandomNumberInRange(0.625f*-size, size*0.625f);
					possmoke.z += CGeneral::GetRandomNumberInRange(0.625f* size, size*2.5f);
					
					CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE, possmoke, vel);
            
					break;
				}
				
				case POBJECT_BIG_FIRE:
				{
					CVector pos = this->GetPosition();
					CVector vel = this->m_vecTarget;
					float size  = this->m_fSize;
					
					
					float s = 0.7f*size;
					
					CVector flamevel;
					
					flamevel.x = vel.x;
					flamevel.y = vel.y;
					flamevel.z = CGeneral::GetRandomNumberInRange(0.0125f*s, 0.1f*s);
					
					float flamesize = 0.8f*size;
					
					CParticle::AddParticle(PARTICLE_FLAME, pos, flamevel, nil, flamesize);
					
					
					for ( int32 i = 0; i < 4; i++ )
					{
						CVector smokepos = pos;
						
						smokepos.x += CGeneral::GetRandomNumberInRange(0.625f*-size, 0.625f*size);
						smokepos.y += CGeneral::GetRandomNumberInRange(0.625f*-size, 0.625f*size);
						smokepos.z += CGeneral::GetRandomNumberInRange(0.625f* size, 3.5f  *size);
						
						CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE, smokepos, vel);
					}

					break;
				}
				
				case POBJECT_FIREBALL_AND_SMOKE:
				{
					if ( this->m_pParticle == nil )
					{
						CVector pos = this->GetPosition();
						CVector vel = this->m_vecTarget;
						float size  = this->m_fSize;
						
						CVector expvel = 1.2f*vel;
						float expsize  = 1.2f*size;
						this->m_pParticle = CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM, pos, expvel, nil, expsize);
					}
					else
					{
						CVector pos = this->GetPosition(); // this->m_pParticle->m_vecPosition ?
						CVector vel = this->m_vecTarget;
						float size  = this->m_fSize;
						
						CVector veloffset   = 0.35f*vel;
						CVector fireballvel = vel;
						
						for ( int32 i = 0; i < this->m_nNumEffectCycles; i++ )
						{
							fireballvel.x += CGeneral::GetRandomNumberInRange(-veloffset.x, veloffset.x);
							fireballvel.y += CGeneral::GetRandomNumberInRange(-veloffset.y, veloffset.y);
							fireballvel.z += CGeneral::GetRandomNumberInRange(-veloffset.z, veloffset.z);

							CParticle::AddParticle(PARTICLE_FIREBALL_SMOKE, pos, fireballvel, nil, size);
						}
					}
					
					break;
				}
				
				case POBJECT_ROCKET_TRAIL:
				{
					if ( this->m_pParticle == nil )
					{
						CVector pos = this->GetPosition();
						CVector vel = this->m_vecTarget;
						float size  = this->m_fSize;
						
						this->m_pParticle = CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM, pos, vel, nil, size);
					}
					else
					{
						CVector pos = this->m_pParticle->m_vecPosition;
						CVector vel = this->m_vecTarget;
						float size  = this->m_fSize;
						
						float fireballsize  = size * 1.5f;
						CVector fireballvel = vel * -0.8f;
						
						for ( int32 i = 0; i < this->m_nNumEffectCycles; i++ )
						{
							CParticle::AddParticle(PARTICLE_FIREBALL_SMOKE, pos, fireballvel, nil, fireballsize);
						}
					}
					
					break;
				}
				
				case POBJECT_FIRE_TRAIL:
				{
					for ( int32 i = 0; i < this->m_nNumEffectCycles; i++ )
					{
						CVector vel = this->m_vecTarget;

						if ( vel.x != 0.0f )
							vel.x += CGeneral::GetRandomNumberInRange(-this->m_fRandVal, this->m_fRandVal);
						
						if ( vel.y != 0.0f )
							vel.y += CGeneral::GetRandomNumberInRange(-this->m_fRandVal, this->m_fRandVal);
						
						if ( vel.z != 0.0f )
							vel.z += CGeneral::GetRandomNumberInRange(-this->m_fRandVal, this->m_fRandVal);
						
						CParticle::AddParticle(this->m_ParticleType, this->GetPosition(), vel, nil, this->m_fSize,
							CGeneral::GetRandomNumberInRange(-6.0f, 6.0f));
					}
					
					break;
				}
				
				case POBJECT_PED_WATER_SPLASH:
				{
					CRGBA colorsmoke(255, 255, 255, 196);
					
					CVector pos = this->GetPosition();
					CVector vel = this->m_vecTarget;
					
					for ( int32 i = 0; i < 3; i++ )
					{
						int32 angle = 90 * i;
						float fCos = CParticle::Cos(angle);
						float fSin = CParticle::Sin(angle);
						
						CVector splashpos;
						CVector splashvel;
												
						splashpos = pos + CVector(0.75f*fCos, 0.75f*fSin, 0.0f);
						splashvel = vel + CVector(0.05f*fCos, 0.05f*fSin, CGeneral::GetRandomNumberInRange(0.04f, 0.08f));
						
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.1f, 0.8f), colorsmoke);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.1f, 0.5f), this->m_Color);
						

						splashpos = pos + CVector(0.75f*fCos, 0.75f*-fSin, 0.0f);
						splashvel = vel + CVector(0.05f*fCos, 0.05f*-fSin, CGeneral::GetRandomNumberInRange(0.04f, 0.08f));
						
						
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.1f, 0.8f), colorsmoke);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.1f, 0.5f), this->m_Color);
						
						
						splashpos = pos + CVector(0.75f*-fCos, 0.75f*fSin, 0.0f);
						splashvel = vel + CVector(0.05f*-fCos, 0.05f*fSin, CGeneral::GetRandomNumberInRange(0.04f, 0.08f));
						
						
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.1f, 0.8f), colorsmoke);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.1f, 0.5f), this->m_Color);
						
						
						splashpos = pos + CVector(0.75f*-fCos, 0.75f*-fSin, 0.0f);
						splashvel = vel + CVector(0.05f*-fCos, 0.05f*-fSin, CGeneral::GetRandomNumberInRange(0.04f, 0.08f));
						
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.1f, 0.8f), colorsmoke);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.1f, 0.5f), this->m_Color);
					}
					
					for ( int32 i = 0; i < 1; i++ )
					{
						int32 angle = 180 * (i + 1);
						
						float fCos = CParticle::Cos(angle);
						float fSin = CParticle::Sin(angle);
						
						CVector splashpos;
						CVector splashvel;
						
						splashpos = pos + CVector(0.5f*fCos, 0.5f*fSin, 0.0f);
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.25f, 0.25f) * fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.25f, 0.25f) * fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.05f, 0.25f);
						
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.4f, 1.0f), this->m_Color);
						
						
						splashpos = pos + CVector(0.5f*fCos, 0.5f*-fSin, 0.0f);
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.25f, 0.25f) *  fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.25f, 0.25f) * -fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.05f, 0.25f);

						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.4f, 1.0f), this->m_Color);
						
						
						splashpos = pos + CVector(0.5f*-fCos, 0.5f*fSin, 0.0f);
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.25f, 0.25f) * -fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.25f, 0.25f) *  fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.05f, 0.25f);

						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.4f, 1.0f), this->m_Color);
						
						
						splashpos = pos + CVector(0.5f*-fCos, 0.5f*-fSin, 0.0f);
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.25f, 0.25f) * -fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.25f, 0.25f) * -fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.05f, 0.25f);

						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.4f, 1.0f), this->m_Color);
					}

					break;
				}
				
				case POBJECT_CAR_WATER_SPLASH:
				{
					CRGBA colorsmoke(255, 255, 255, 196);
					
					CVector pos = this->GetPosition();
					CVector vel = this->m_vecTarget;
					
					float size = CGeneral::GetRandomNumberInRange(1.0f, 2.5f);
					
					for ( int32 i = 0; i < 3; i++ )
					{
						int32 angle = 90 * i;
						
						float fCos = CParticle::Cos(angle);
						float fSin = CParticle::Sin(angle);
						
						CVector splashpos;
						CVector splashvel;
						
						splashpos = pos + CVector(2.0f*fCos, 2.0f*fSin, 0.0f);
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f) * fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f) * fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.01f, 0.03f);
						
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, splashpos, splashvel, nil, size, colorsmoke);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil, 0.0f, this->m_Color);
						

						splashpos = pos + CVector(2.0f*fCos, 2.0f*-fSin, 0.0f);						
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f) * fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f) * -fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.01f, 0.03f);
						
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, splashpos, splashvel, nil, size, colorsmoke);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil, 0.0f, this->m_Color);
						
						splashpos = pos + CVector(2.0f*-fCos, 2.0f*fSin, 0.0f);
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f) * -fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f) * fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.01f, 0.03f);
						
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, splashpos, splashvel, nil, size, colorsmoke);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil, 0.0f, this->m_Color);
						
						splashpos = pos + CVector(2.0f*-fCos, 2.0f*-fSin, 0.0f);
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f) * -fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f) * -fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.01f, 0.03f);
						
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, splashpos, splashvel, nil, size, colorsmoke);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil, 0.0f, this->m_Color);
					}
					
					for ( int32 i = 0; i < 1; i++ )
					{
						int32 angle = 180 * (i + 1);
						
						float fCos = CParticle::Cos(angle);
						float fSin = CParticle::Sin(angle);
						
						CVector splashpos;
						CVector splashvel;
						
						
						splashpos = pos + CVector(1.25f*fCos, 1.25f*fSin, 0.0f);
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.26f, 0.53f);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil, 0.0f, this->m_Color);
						
						splashpos = pos + CVector(1.25f*fCos, 1.25f*-fSin, 0.0f);						
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * -fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.26f, 0.53f);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil, 0.0f, this->m_Color);
						
						splashpos = pos + CVector(1.25f*-fCos, 1.25f*fSin, 0.0f);						
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * -fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.26f, 0.53f);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil, 0.0f, this->m_Color);

						splashpos = pos + CVector(1.25f*-fCos, 1.25f*-fSin, 0.0f);						
						splashvel = vel;
						splashvel.x += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * -fCos;
						splashvel.y += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * -fSin;
						splashvel.z += CGeneral::GetRandomNumberInRange(0.26f, 0.53f);
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil, 0.0f, this->m_Color);
					}

					break;
				}
				
				case POBJECT_SPLASHES_AROUND:
				{
					CVector pos = this->GetPosition();
					float size  = this->m_fSize;
					
					for ( int32 i = 0; i < this->m_nNumEffectCycles; i++ )
					{
						CVector splashpos = pos;
						
						splashpos.x += CGeneral::GetRandomNumberInRange(-size, size);
						splashpos.y += CGeneral::GetRandomNumberInRange(-size, size);

						if ( CGeneral::GetRandomNumber() & 1 )
						{
							CParticle::AddParticle(PARTICLE_RAIN_SPLASH,   splashpos, CVector(0.0f, 0.0f, 0.0f),
								nil, 0.1f,  this->m_Color);
						}
						else
						{
							CParticle::AddParticle(PARTICLE_RAIN_SPLASHUP, splashpos, CVector(0.0f, 0.0f, 0.0f),
								nil, 0.12f, this->m_Color);
						}
					}
			
					break;
				}
				
				case POBJECT_FIRE_HYDRANT:
				{
					CVector pos = this->GetPosition();
					CVector vel = this->m_vecTarget;
					
					if ( (TheCamera.GetPosition() - pos).Magnitude() > 5.0f )
					{
						for ( int32 i = 0; i < 1; i++ )
						{
							int32 angle = 180 * i;
						
							float fCos = CParticle::Cos(angle);
							float fSin = CParticle::Sin(angle);
							
							CVector splashpos, splashvel;
							
							splashpos = pos + CVector(0.01f*fCos, 0.01f*fSin, 0.0f);
							splashvel = vel + CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.004f, 0.008f));
							
							CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
								CGeneral::GetRandomNumberInRange(0.005f, 0.0075f), this->m_Color, 0, 0, 1, 300);
								
							splashpos = pos + CVector(0.01f*fCos, 0.01f*-fSin, 0.0f);
							splashvel = vel + CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.004f, 0.008f));
							
							CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
								CGeneral::GetRandomNumberInRange(0.005f, 0.0075f), this->m_Color, 0, 0, 1, 300);
								
							splashpos = pos + CVector(0.01f*-fCos, 0.01f*fSin, 0.0f);
							splashvel = vel + CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.004f, 0.008f));
							
							CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
								CGeneral::GetRandomNumberInRange(0.005f, 0.0075f), this->m_Color, 0, 0, 1, 300);
								
							splashpos = pos + CVector(0.01f*-fCos, 0.01f*-fSin, 0.0f);
							splashvel = vel + CVector(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(0.004f, 0.008f));
							
							CParticle::AddParticle(PARTICLE_CAR_SPLASH, splashpos, splashvel, nil,
								CGeneral::GetRandomNumberInRange(0.005f, 0.0075f), this->m_Color, 0, 0, 1, 300);
						}
						for ( int32 i = 0; i < this->m_nNumEffectCycles; i++ )
						{
							CParticle::AddParticle(this->m_ParticleType, pos, vel, nil, 0.0f, this->m_Color);
						}
					}
					
					break;
				}
				
				case POBJECT_WATER_FOUNTAIN_VERT:
				{
					CVector pos = this->GetPosition();
					CVector vel = this->m_vecTarget;
					
					for ( int32 i = 0; i < 2; i++ )
					{
						int32 angle = 180 * i;
						
						float fCos = CParticle::Cos(angle);
						float fSin = CParticle::Sin(angle);
						
						CVector splashpos, splashvel;
						
						splashpos = pos + CVector(0.015f*fCos, 0.015f*fSin, 0.0f);
						splashvel = vel + CVector(0.015f*fCos, 0.015f*fSin, CGeneral::GetRandomNumberInRange(0.004f, 0.008f));
							
						CParticle::AddParticle(PARTICLE_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.001f, 0.005f), this->m_Color, 0, 0, 1, 1000);
							
						splashpos = pos + CVector(0.015f*fCos, 0.015f*-fSin, 0.0f);
						splashvel = vel + CVector(0.015f*fCos, 0.015f*-fSin, CGeneral::GetRandomNumberInRange(0.004f, 0.008f));
							
						CParticle::AddParticle(PARTICLE_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.001f, 0.005f), this->m_Color, 0, 0, 1, 1000);
							
						splashpos = pos + CVector(0.015f*-fCos, 0.015f*fSin, 0.0f);
						splashvel = vel + CVector(0.015f*-fCos, 0.015f*fSin, CGeneral::GetRandomNumberInRange(0.004f, 0.008f));
							
						CParticle::AddParticle(PARTICLE_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.001f, 0.005f), this->m_Color, 0, 0, 1, 1000);
							
						splashpos = pos + CVector(0.015f*-fCos, 0.015f*-fSin, 0.0f);
						splashvel = vel + CVector(0.015f*-fCos, 0.015f*-fSin, CGeneral::GetRandomNumberInRange(0.004f, 0.008f));
							
						CParticle::AddParticle(PARTICLE_SPLASH, splashpos, splashvel, nil,
							CGeneral::GetRandomNumberInRange(0.001f, 0.005f), this->m_Color, 0, 0, 1, 1000);
					}
					
					break;
				}
				
				case POBJECT_WATER_FOUNTAIN_HORIZ:
				{
					CVector pos = this->GetPosition();
					CVector vel = this->m_vecTarget;
					
					for ( int32 i = 0; i < 3; i++ )
					{
						CParticle::AddParticle(PARTICLE_CAR_SPLASH, pos, vel, nil, 0.001f, this->m_Color, 0, 0, 1, 1000);
					}
					
					break;
				}
				
				default:
				{
					if ( this->m_fRandVal != 0.0f )
					{
						for ( int32 i = 0; i < this->m_nNumEffectCycles; i++ )
						{
							CVector vel = this->m_vecTarget;
							
							if ( vel.x != 0.0f )
								vel.x += CGeneral::GetRandomNumberInRange(-this->m_fRandVal, this->m_fRandVal);
							
							if ( vel.y != 0.0f )
								vel.y += CGeneral::GetRandomNumberInRange(-this->m_fRandVal, this->m_fRandVal);
							
							if ( vel.z != 0.0f )
								vel.z += CGeneral::GetRandomNumberInRange(-this->m_fRandVal, this->m_fRandVal);
							
							CParticle::AddParticle(this->m_ParticleType, this->GetPosition(), vel, nil,
								this->m_fSize, this->m_Color);
						}
					}
					else
					{
						for ( int32 i = 0; i < this->m_nNumEffectCycles; i++ )
						{
							CParticle::AddParticle(this->m_ParticleType, this->GetPosition(), this->m_vecTarget, nil,
								this->m_fSize, this->m_Color);
						}
					}
					
					break;
				}
			}
		}
	}
	
	if ( this->m_nRemoveTimer != 0 && this->m_nRemoveTimer < CTimer::GetTimeInMilliseconds() )
	{
		MoveToList(&pCloseListHead, &pUnusedListHead, this);
		this->m_nState = POBJECTSTATE_FREE;

		if ( this->m_Type == POBJECT_FIRE_HYDRANT )
			CAudioHydrant::Remove(this);
	}
}

void
CParticleObject::UpdateFar(void)
{
	if ( this->m_nRemoveTimer != 0 && this->m_nRemoveTimer < CTimer::GetTimeInMilliseconds() )
	{
		MoveToList(&pFarListHead, &pUnusedListHead, this);
		this->m_nState = POBJECTSTATE_FREE;
		
		if ( this->m_Type == POBJECT_FIRE_HYDRANT )
			CAudioHydrant::Remove(this);
	}
	
	CVector2D dist = this->GetPosition() - TheCamera.GetPosition();
	if ( dist.MagnitudeSqr() < SQR(100.0f)/*10000.0f*/ )
	{
		MoveToList(&pFarListHead, &pCloseListHead, this);
		this->m_nState = POBJECTSTATE_UPDATE_CLOSE;
	}
}

bool
CParticleObject::SaveParticle(uint8 *buffer, uint32 *length)
{
	ASSERT( buffer != nil );
	ASSERT( length != nil );
	
	int32 numObjects = 0;
	
	for ( CParticleObject *p = pCloseListHead; p != nil; p = p->m_pNext )
		++numObjects;
	
	for ( CParticleObject *p = pFarListHead; p != nil; p = p->m_pNext )
		++numObjects;
	
	*(int32 *)buffer = numObjects;
	buffer += sizeof(int32);
	
	int32 objectsLength = sizeof(CParticleObject) * (numObjects + 1);
	int32 dataLength = objectsLength + sizeof(int32);
	
	for ( CParticleObject *p = pCloseListHead; p != nil; p = p->m_pNext )
	{
#if 0 // todo better
		*(CParticleObject*)buffer = *p;
#else
		memcpy(buffer, p, sizeof(CParticleObject));
#endif
		buffer += sizeof(CParticleObject);
	}
	
	for ( CParticleObject *p = pFarListHead; p != nil; p = p->m_pNext )
	{
#if 0 // todo better
		*(CParticleObject*)buffer = *p;
#else
		memcpy(buffer, p, sizeof(CParticleObject));
#endif
		buffer += sizeof(CParticleObject);
	}
	
	*length = dataLength;
  
	return true;
}

bool
CParticleObject::LoadParticle(uint8 *buffer, uint32  length)
{
	ASSERT( buffer != nil );
	
	RemoveAllParticleObjects();
	
	int32 numObjects = *(int32 *)buffer;
	buffer += sizeof(int32);
	
	if ( length != sizeof(CParticleObject) * (numObjects + 1) + sizeof(int32) )
		return false;
	
	if ( numObjects == 0 )
		return true;
	

	int32 i = 0;
	while ( i < numObjects )
	{
		CParticleObject *dst = pUnusedListHead;
		CParticleObject *src = (CParticleObject *)buffer;
		buffer += sizeof(CParticleObject);
		
		if ( dst == nil )
			return false;
		
		MoveToList(&pUnusedListHead, &pCloseListHead, dst);
		
		dst->m_nState           = POBJECTSTATE_UPDATE_CLOSE;
		dst->m_Type             = src->m_Type;
		dst->m_ParticleType     = src->m_ParticleType;
		dst->SetPosition(src->GetPosition());
		dst->m_vecTarget        = src->m_vecTarget;
		dst->m_nFrameCounter    = src->m_nFrameCounter;
		dst->m_bRemove          = src->m_bRemove;
		dst->m_pParticle        = nil;
		dst->m_nRemoveTimer     = src->m_nRemoveTimer;
		dst->m_Color            = src->m_Color;
		dst->m_fSize            = src->m_fSize;
		dst->m_fRandVal         = src->m_fRandVal;
		dst->m_nNumEffectCycles = src->m_nNumEffectCycles;
		dst->m_nSkipFrames      = src->m_nSkipFrames;
		dst->m_nCreationChance  = src->m_nCreationChance;
		
		i++;
	}
	
	return true;
}

void
CParticleObject::RemoveAllExpireableParticleObjects(void)
{
	{
		CParticleObject *pobj = pCloseListHead;
		CParticleObject *nextpobj;
		if ( pobj != nil )
		{
			do
			{
				nextpobj = pobj->m_pNext;
				if ( pobj->m_nRemoveTimer != 0 )
				{
					MoveToList(&pCloseListHead, &pUnusedListHead, pobj);
					pobj->m_nState = POBJECTSTATE_FREE;
				}
				pobj = nextpobj;
			}
			while ( nextpobj != nil );
		}
	}
	
	{
		CParticleObject *pobj = pFarListHead;
		CParticleObject *nextpobj;
		if ( pobj != nil )
		{
			do
			{
				nextpobj = pobj->m_pNext;
				if ( pobj->m_nRemoveTimer != 0 )
				{
					MoveToList(&pFarListHead, &pUnusedListHead, pobj);
					pobj->m_nState = POBJECTSTATE_FREE;
				}
				pobj = nextpobj;
			}
			while ( nextpobj != nil );
		}
	}
}

void
CParticleObject::RemoveAllParticleObjects(void)
{
	pUnusedListHead = &gPObjectArray[0];
	
	pCloseListHead = nil;
	pFarListHead   = nil;
	
	for ( int32 i = 0; i < MAX_PARTICLEOBJECTS; i++ )
	{
		if ( i == 0 )
			gPObjectArray[i].m_pPrev = nil;
		else
			gPObjectArray[i].m_pPrev = &gPObjectArray[i - 1];
		
		if ( i == MAX_PARTICLEOBJECTS-1 )
			gPObjectArray[i].m_pNext = nil;
		else
			gPObjectArray[i].m_pNext = &gPObjectArray[i + 1];
		
		gPObjectArray[i].m_nState = POBJECTSTATE_FREE;
	}
}

void
CParticleObject::MoveToList(CParticleObject **from, CParticleObject **to, CParticleObject *obj)
{
	ASSERT( from != nil );
	ASSERT( to   != nil );
	ASSERT( obj  != nil );
	
	if ( obj->m_pPrev == nil )
	{
		*from = obj->m_pNext;
		if ( *from )
			(*from)->m_pPrev = nil;
	}
	else
	{
		if ( obj->m_pNext == nil )
			obj->m_pPrev->m_pNext = nil;
		else
		{
			obj->m_pNext->m_pPrev = obj->m_pPrev;
			obj->m_pPrev->m_pNext = obj->m_pNext;
		}
	}
	
	obj->m_pNext = *to;
	obj->m_pPrev = nil;
	*to = obj;

	if ( obj->m_pNext )
		obj->m_pNext->m_pPrev = obj;
}
