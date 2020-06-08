#include "common.h"

#include "World.h"
#include "General.h"
#include "Timer.h"
#include "Stats.h"
#include "ModelIndices.h"
#include "Treadable.h"
#include "Vehicle.h"
#include "Ped.h"
#include "Object.h"
#include "Glass.h"
#include "ParticleObject.h"
#include "Particle.h"
#include "SurfaceTable.h"
#include "PathFind.h"
#include "CarCtrl.h"
#include "DMAudio.h"
#include "Automobile.h"
#include "Bike.h"
#include "Pickups.h"
#include "Physical.h"

//--MIAMI: file done

CPhysical::CPhysical(void)
{
	int i;

#ifdef FIX_BUGS
	m_nLastTimeCollided = 0;
#endif

	m_fForceMultiplier = 1.0f;
	m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
	m_vecMoveSpeedAvg = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnSpeedAvg = CVector(0.0f, 0.0f, 0.0f);

	m_movingListNode = nil;
	m_nStaticFrames = 0;

	m_nCollisionRecords = 0;
	for(i = 0; i < 6; i++)
		m_aCollisionRecords[i] = nil;

	m_bIsVehicleBeingShifted = false;
	bJustCheckCollision = false;

	m_nDamagePieceType = 0;
	m_fDamageImpulse = 0.0f;
	m_pDamageEntity = nil;
	m_vecDamageNormal = CVector(0.0f, 0.0f, 0.0f);

	bUsesCollision = true;
	m_audioEntityId = -5;
	m_phys_unused1 = 100.0f;
	m_vecCentreOfMass = CVector(0.0f, 0.0f, 0.0f);
	m_phys_unused2 = 0;

	bIsHeavy = false;
	bAffectedByGravity = true;
	bInfiniteMass = false;
	m_phy_flagA08 = false;
	bIsInWater = false;
	bHitByTrain = false;
	bSkipLineCol = false;

	m_fDistanceTravelled = 0.0f;

	m_phy_flagA20 = false;

#ifdef FIX_BUGS
	m_nSurfaceTouched = SURFACE_DEFAULT;
#endif
	m_nZoneLevel = LEVEL_NONE;

	bIsFrozen = false;
	bDontLoadCollision = false;
}

CPhysical::~CPhysical(void)
{
	m_entryInfoList.Flush();
}

void
CPhysical::Add(void)
{
	int x, xstart, xmid, xend;
	int y, ystart, ymid, yend;
	CSector *s;
	CPtrList *list;

	CRect bounds = GetBoundRect();
	xstart = CWorld::GetSectorIndexX(bounds.left);
	xend   = CWorld::GetSectorIndexX(bounds.right);
	xmid   = CWorld::GetSectorIndexX((bounds.left + bounds.right)/2.0f);
	ystart = CWorld::GetSectorIndexY(bounds.top);
	yend   = CWorld::GetSectorIndexY(bounds.bottom);
	ymid   = CWorld::GetSectorIndexY((bounds.top + bounds.bottom)/2.0f);
	assert(xstart >= 0);
	assert(xend < NUMSECTORS_X);
	assert(ystart >= 0);
	assert(yend < NUMSECTORS_Y);

	for(y = ystart; y <= yend; y++)
		for(x = xstart; x <= xend; x++){
			s = CWorld::GetSector(x, y);
			if(x == xmid && y == ymid) switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS];
				break;
			default:
				assert(0);
			}else switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES_OVERLAP];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS_OVERLAP];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS_OVERLAP];
				break;
			default:
				assert(0);
			}
			CPtrNode *node = list->InsertItem(this);
			assert(node);
			m_entryInfoList.InsertItem(list, node, s);
		}
}

void
CPhysical::Remove(void)
{
	CEntryInfoNode *node, *next;
	for(node = m_entryInfoList.first; node; node = next){
		next = node->next;
		node->list->DeleteNode(node->listnode);
		m_entryInfoList.DeleteNode(node);
	}
}

void
CPhysical::RemoveAndAdd(void)
{
	int x, xstart, xmid, xend;
	int y, ystart, ymid, yend;
	CSector *s;
	CPtrList *list;

	CRect bounds = GetBoundRect();
	xstart = CWorld::GetSectorIndexX(bounds.left);
	xend   = CWorld::GetSectorIndexX(bounds.right);
	xmid   = CWorld::GetSectorIndexX((bounds.left + bounds.right)/2.0f);
	ystart = CWorld::GetSectorIndexY(bounds.top);
	yend   = CWorld::GetSectorIndexY(bounds.bottom);
	ymid   = CWorld::GetSectorIndexY((bounds.top + bounds.bottom)/2.0f);
	assert(xstart >= 0);
	assert(xend < NUMSECTORS_X);
	assert(ystart >= 0);
	assert(yend < NUMSECTORS_Y);

	// we'll try to recycle nodes from here
	CEntryInfoNode *next = m_entryInfoList.first;

	for(y = ystart; y <= yend; y++)
		for(x = xstart; x <= xend; x++){
			s = CWorld::GetSector(x, y);
			if(x == xmid && y == ymid) switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS];
				break;
			}else switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES_OVERLAP];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS_OVERLAP];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS_OVERLAP];
				break;
			}
			if(next){
				// If we still have old nodes, use them
				next->list->RemoveNode(next->listnode);
				list->InsertNode(next->listnode);
				next->list = list;
				next->sector = s;
				next = next->next;
			}else{
				CPtrNode *node = list->InsertItem(this);
				m_entryInfoList.InsertItem(list, node, s);
			}
		}

	// Remove old nodes we no longer need
	CEntryInfoNode *node;
	for(node = next; node; node = next){
		next = node->next;
		node->list->DeleteNode(node->listnode);
		m_entryInfoList.DeleteNode(node);
	}
}

CRect
CPhysical::GetBoundRect(void)
{
	CVector center;
	float radius;
	GetBoundCentre(center);
	radius = GetBoundRadius();
	return CRect(center.x-radius, center.y-radius, center.x+radius, center.y+radius);
}

void
CPhysical::AddToMovingList(void)
{
	if (!bIsStaticWaitingForCollision)
		m_movingListNode = CWorld::GetMovingEntityList().InsertItem(this);
}

void
CPhysical::RemoveFromMovingList(void)
{
	if(m_movingListNode){
		CWorld::GetMovingEntityList().DeleteNode(m_movingListNode);
		m_movingListNode = nil;
	}
}

void
CPhysical::SetDamagedPieceRecord(uint16 piece, float impulse, CEntity *entity, CVector dir)
{
	m_nDamagePieceType = piece;
	m_fDamageImpulse = impulse;
	m_pDamageEntity = entity;
	entity->RegisterReference(&m_pDamageEntity);
	m_vecDamageNormal = dir;
}

void
CPhysical::AddCollisionRecord(CEntity *ent)
{
	AddCollisionRecord_Treadable(ent);
	this->bHasCollided = true;
	ent->bHasCollided = true;
	if(IsVehicle() && ent->IsVehicle()){
		if(((CVehicle*)this)->m_nAlarmState == -1)
			((CVehicle*)this)->m_nAlarmState = 15000;
		if(((CVehicle*)ent)->m_nAlarmState == -1)
			((CVehicle*)ent)->m_nAlarmState = 15000;
	}
	if(bUseCollisionRecords){
		int i;
		for(i = 0; i < m_nCollisionRecords; i++)
			if(m_aCollisionRecords[i] == ent)
				return;
		if(m_nCollisionRecords < PHYSICAL_MAX_COLLISIONRECORDS)
			m_aCollisionRecords[m_nCollisionRecords++] = ent;
		m_nLastTimeCollided = CTimer::GetTimeInMilliseconds();
	}
}

void
CPhysical::AddCollisionRecord_Treadable(CEntity *ent)
{
	if(ent->IsBuilding() && ((CBuilding*)ent)->GetIsATreadable()){
	}
}

bool
CPhysical::GetHasCollidedWith(CEntity *ent)
{
	int i;
	if(bUseCollisionRecords)
		for(i = 0; i < m_nCollisionRecords; i++)
			if(m_aCollisionRecords[i] == ent)
				return true;
	return false;
}

void
CPhysical::RemoveRefsToEntity(CEntity *ent)
{
	int i, j;

	for(i = 0; i < m_nCollisionRecords; i++){
		if(m_aCollisionRecords[i] == ent){
			for(j = i; j < m_nCollisionRecords-1; j++)
				m_aCollisionRecords[j] = m_aCollisionRecords[j+1];
			m_nCollisionRecords--;
		}
	}
}

void
CPhysical::PlacePhysicalRelativeToOtherPhysical(CPhysical *other, CPhysical *phys, CVector localPos)
{
	CVector worldPos = other->GetMatrix() * localPos;
	float step = 0.9f * CTimer::GetTimeStep();
	CVector pos = other->m_vecMoveSpeed*step + worldPos;

	CWorld::Remove(phys);
	phys->GetMatrix() = other->GetMatrix();
	phys->SetPosition(pos);
	phys->m_vecMoveSpeed = other->m_vecMoveSpeed;
	phys->GetMatrix().UpdateRW();
	phys->UpdateRwFrame();
	CWorld::Add(phys);
}

int32
CPhysical::ProcessEntityCollision(CEntity *ent, CColPoint *colpoints)
{
	int32 numSpheres = CCollision::ProcessColModels(
		GetMatrix(), *GetColModel(),
		ent->GetMatrix(), *ent->GetColModel(),
		colpoints,
		nil, nil);	// No Lines allowed!
	if(numSpheres > 0){
		AddCollisionRecord(ent);
		if(!ent->IsBuilding())	// Can't this catch dummies too?
			((CPhysical*)ent)->AddCollisionRecord(this);
		if(ent->IsBuilding() || ent->IsStatic())
			this->bHasHitWall = true;
	}
	return numSpheres;
}

void
CPhysical::ProcessControl(void)
{
	if(!IsPed())
		bIsInWater = false;
	bHasContacted = false;
	bIsInSafePosition = false;
	bWasPostponed = false;
	bHasHitWall = false;

	if(GetStatus() == STATUS_SIMPLE)
		return;

	m_nCollisionRecords = 0;
	bHasCollided = false;
	m_nDamagePieceType = 0;
	m_fDamageImpulse = 0.0f;
	m_pDamageEntity = nil;

	if(!bIsStuck){
		if(IsObject() ||
		   IsPed() && !bPedPhysics){
			m_vecMoveSpeedAvg = (m_vecMoveSpeedAvg + m_vecMoveSpeed)/2.0f;
			m_vecTurnSpeedAvg = (m_vecTurnSpeedAvg + m_vecTurnSpeed)/2.0f;
			float step = CTimer::GetTimeStep() * 0.003f;
			if(m_vecMoveSpeedAvg.MagnitudeSqr() < step*step &&
			   m_vecTurnSpeedAvg.MagnitudeSqr() < step*step){
				m_nStaticFrames++;
				if(m_nStaticFrames > 10){
					m_nStaticFrames = 10;
					bIsStatic = true;
					m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
					m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
					m_vecMoveFriction = m_vecMoveSpeed;
					m_vecTurnFriction = m_vecTurnSpeed;
					return;
				}
			}else 
				m_nStaticFrames = 0;
		}
	}
	ApplyGravity();
	ApplyFriction();
	ApplyAirResistance();
}

/*
 * Some quantities (german in parens):
 *
 * acceleration: distance/time^2: a
 * velocity: distance/time: v	(GTA: speed)
 * momentum (impuls): velocity*mass: p
 * impulse (kraftstoss): delta momentum, force*time: J
 *
 * angular equivalents:
 * velocity -> angular velocity	(GTA: turn speed)
 * momentum -> angular momentum (drehimpuls): L = r cross p
 * force -> torque (drehmoment): tau = r cross F
 * mass -> moment of inertia, angular mass (drehmoment, drehmasse): I = L/omega	(GTA: turn mass)
 */

CVector
CPhysical::GetSpeed(const CVector &r)
{
	return m_vecMoveSpeed + m_vecMoveFriction + CrossProduct(m_vecTurnFriction + m_vecTurnSpeed, r);
}

void
CPhysical::ApplyMoveSpeed(void)
{
	if(bIsFrozen)
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	else
		GetMatrix().Translate(m_vecMoveSpeed * CTimer::GetTimeStep());
}


void
CPhysical::ApplyTurnSpeed(void)
{
	if(bIsFrozen){
		m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
	}else{
		// Move the coordinate axes by their speed
		// Note that this denormalizes the matrix
		CVector turnvec = m_vecTurnSpeed*CTimer::GetTimeStep();
		GetRight() += CrossProduct(turnvec, GetRight());
		GetForward() += CrossProduct(turnvec, GetForward());
		GetUp() += CrossProduct(turnvec, GetUp());
	}
}

void
CPhysical::ApplyMoveForce(float jx, float jy, float jz)
{
	m_vecMoveSpeed += CVector(jx, jy, jz)*(1.0f/m_fMass);
}

void
CPhysical::ApplyTurnForce(float jx, float jy, float jz, float px, float py, float pz)
{
	CVector com = Multiply3x3(m_matrix, m_vecCentreOfMass);
	CVector turnimpulse = CrossProduct(CVector(px, py, pz)-com, CVector(jx, jy, jz));
	m_vecTurnSpeed += turnimpulse*(1.0f/m_fTurnMass);
}

void
CPhysical::ApplyFrictionMoveForce(float jx, float jy, float jz)
{
	m_vecMoveFriction += CVector(jx, jy, jz)*(1.0f/m_fMass);
}

void
CPhysical::ApplyFrictionTurnForce(float jx, float jy, float jz, float px, float py, float pz)
{
	CVector com = Multiply3x3(m_matrix, m_vecCentreOfMass);
	CVector turnimpulse = CrossProduct(CVector(px, py, pz)-com, CVector(jx, jy, jz));
	m_vecTurnFriction += turnimpulse*(1.0f/m_fTurnMass);
}

bool
CPhysical::ApplySpringCollision(float springConst, CVector &springDir, CVector &point, float springRatio, float bias)
{
	float compression = 1.0f - springRatio;
	if(compression > 0.0f){
		float step = Min(CTimer::GetTimeStep(), 3.0f);
		float impulse = -GRAVITY*m_fMass*step * springConst * compression * bias*2.0f;
		ApplyMoveForce(springDir*impulse);
		ApplyTurnForce(springDir*impulse, point);
	}
	return true;
}

bool
CPhysical::ApplySpringCollisionAlt(float springConst, CVector &springDir, CVector &point, float springRatio, float bias, CVector &forceDir)
{
	float compression = 1.0f - springRatio;
	if(compression > 0.0f){
		if(DotProduct(springDir, forceDir) > 0.0f)
			forceDir *= -1.0f;
		float step = Min(CTimer::GetTimeStep(), 3.0f);
		float impulse = GRAVITY*m_fMass*step * springConst * compression * bias*2.0f;
		if(bIsHeavy)
			impulse *= 0.75f;
		ApplyMoveForce(forceDir*impulse);
		ApplyTurnForce(forceDir*impulse, point);
	}
	return true;
}

// What exactly is speed?
bool
CPhysical::ApplySpringDampening(float damping, CVector &springDir, CVector &point, CVector &speed)
{
	float speedA = DotProduct(speed, springDir);
	float speedB = DotProduct(GetSpeed(point), springDir);
	float step = Min(CTimer::GetTimeStep(), 3.0f);
	float impulse = -damping * (speedA + speedB)/2.0f * m_fMass * step * 0.53f;
	if(bIsHeavy)
		impulse *= 2.0f;

	// what is this?
	float a = m_fTurnMass / ((point.MagnitudeSqr() + 1.0f) * 2.0f * m_fMass);
	a = Min(a, 1.0f);
	float b = Abs(impulse / (speedB * m_fMass));
	if(a < b)
		impulse *= a/b;

	ApplyMoveForce(springDir*impulse);
	ApplyTurnForce(springDir*impulse, point);
	return true;
}

void
CPhysical::ApplyGravity(void)
{
	if(bAffectedByGravity)
		m_vecMoveSpeed.z -= GRAVITY * CTimer::GetTimeStep();
}

void
CPhysical::ApplyFriction(void)
{
	m_vecMoveSpeed += m_vecMoveFriction;
	m_vecTurnSpeed += m_vecTurnFriction;
	m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
}

void
CPhysical::ApplyAirResistance(void)
{
	if(m_fAirResistance > 0.1f){
		if(GetStatus() != STATUS_GHOST){ 
			float f = Pow(m_fAirResistance, CTimer::GetTimeStep());
			m_vecMoveSpeed *= f;
			m_vecTurnSpeed *= f;
		}
	}else{
		float f = Pow(1.0f/(m_fAirResistance*0.5f*m_vecMoveSpeed.MagnitudeSqr() + 1.0f), CTimer::GetTimeStep());
		m_vecMoveSpeed *= f;
		m_vecTurnSpeed *= 0.99f;
	}
}

bool
CPhysical::ApplyCollision(CPhysical *B, CColPoint &colpoint, float &impulseA, float &impulseB)
{
	float eA, eB;
	CPhysical *A = this;
	CObject *Bobj = (CObject*)B;

	bool foo = false;	// TODO: what does this mean?
	bool ispedcontactA = false;
	bool ispedcontactB = false;

	float massFactorA;
	if(B->bPedPhysics){
		massFactorA = 10.0f;
		if(B->IsPed() && ((CPed*)B)->m_pCurrentPhysSurface == A)
			ispedcontactA = true;
	}else
		massFactorA = A->bIsHeavy ? 2.0f : 1.0f;

	float massFactorB;
	if(A->bPedPhysics){
		if(A->IsPed() && ((CPed*)A)->IsPlayer() && B->IsVehicle() &&
		   (B->GetStatus() == STATUS_ABANDONED || B->GetStatus() == STATUS_WRECKED || A->bHasHitWall))
			massFactorB = 1.0f/(Max(B->m_fMass - 2000.0f, 0.0f)/5000.0f + 1.0f);
		else
			massFactorB = 10.0f;

		if(A->IsPed() && ((CPed*)A)->m_pCurrentPhysSurface == B)
			ispedcontactB = true;
	}else
		massFactorB = B->bIsHeavy ? 2.0f : 1.0f;

	if(B->bInfiniteMass && !B->m_phy_flagA08){
		ispedcontactB = false;
		foo = true;
	}

	float speedA, speedB;
	if(B->IsStatic() && !foo){
		if(A->bPedPhysics){
			speedA = DotProduct(A->m_vecMoveSpeed, colpoint.normal);
			if(speedA < 0.0f){
				if(B->IsObject()){
					impulseA = -speedA * A->m_fMass;
					impulseB = impulseA;
					if(impulseA > Bobj->m_fUprootLimit){
						if(IsGlass(B->GetModelIndex()))
							CGlass::WindowRespondsToCollision(B, impulseA, A->m_vecMoveSpeed, colpoint.point, false);
						else if(!B->bInfiniteMass){
							B->bIsStatic = false;
							CWorld::Players[CWorld::PlayerInFocus].m_nHavocLevel += 2;
							CStats::PropertyDestroyed += CGeneral::GetRandomNumberInRange(30, 60);
						}
					}else{
						if(IsGlass(B->GetModelIndex()))
							CGlass::WindowRespondsToSoftCollision(B, impulseA);
						if(!A->bInfiniteMass)
							A->ApplyMoveForce(colpoint.normal*(1.0f + A->m_fElasticity)*impulseA);
						return true;
					}
				}else if(!B->bInfiniteMass)
					B->bIsStatic = false;
	
				if(B->bInfiniteMass){
					impulseA = -speedA * A->m_fMass;
					impulseB = 0.0f;
					if(!A->bInfiniteMass)
						A->ApplyMoveForce(colpoint.normal*(1.0f + A->m_fElasticity)*impulseA);
					return true;
				}
			}
		}else{
			CVector pointposA = colpoint.point - A->GetPosition();
			speedA = DotProduct(A->GetSpeed(pointposA), colpoint.normal);
			if(speedA < 0.0f){
				if(B->IsObject()){
					if(A->bHasHitWall)
						eA = -1.0f;
					else
						eA = -(1.0f + A->m_fElasticity);
					impulseA = eA * speedA * A->GetMass(pointposA, colpoint.normal);
					impulseB = impulseA;

					if(Bobj->m_nCollisionDamageEffect && impulseA > 20.0f){
						Bobj->ObjectDamage(impulseA);
						if(!B->bUsesCollision){
							if(!A->bInfiniteMass){
								A->ApplyMoveForce(colpoint.normal*0.2f*impulseA);
								A->ApplyTurnForce(colpoint.normal*0.2f*impulseA, pointposA);
							}
							return false;
						}
					}

					if((impulseA > Bobj->m_fUprootLimit || A->bIsStuck) &&
					   !B->bInfiniteMass){
						if(IsGlass(B->GetModelIndex()))
							CGlass::WindowRespondsToCollision(B, impulseA, A->m_vecMoveSpeed, colpoint.point, false);
						else
							B->bIsStatic = false;
						int16 model = B->GetModelIndex();
						if(model == MI_FIRE_HYDRANT && !Bobj->bHasBeenDamaged){
							CParticleObject::AddObject(POBJECT_FIRE_HYDRANT, B->GetPosition() - CVector(0.0f, 0.0f, 0.5f), true);
							Bobj->bHasBeenDamaged = true;
						}else if(model == MI_PARKINGMETER || model == MI_PARKINGMETER2){
							CPickups::CreateSomeMoney(GetPosition(), CGeneral::GetRandomNumber()%100);
							Bobj->bHasBeenDamaged = true;
						}else if(B->IsObject() && !IsExplosiveThingModel(model))
							Bobj->bHasBeenDamaged = true;
					}else{
						if(IsGlass(B->GetModelIndex()))
							CGlass::WindowRespondsToSoftCollision(B, impulseA);
						CVector f = colpoint.normal * impulseA;
						if(A->IsVehicle() && colpoint.normal.z < 0.7f)
							f.z *= 0.3f;
						if(!A->bInfiniteMass){
							A->ApplyMoveForce(f);
							if(!A->IsVehicle() || !CWorld::bNoMoreCollisionTorque)
								A->ApplyTurnForce(f, pointposA);
						}
						return true;
					}
				}else if(!B->bInfiniteMass)
					B->bIsStatic = false;
			}
		}
	
		if(B->IsStatic())
			return false;
		if(!B->bInfiniteMass && !B->m_phy_flagA08)
			B->AddToMovingList();
	}

	// B is not static

	if(A->bPedPhysics && B->bPedPhysics){
		// negative if A is moving towards B
		speedA = DotProduct(A->m_vecMoveSpeed, colpoint.normal);
		// positive if B is moving towards A
		float speedB = DotProduct(B->m_vecMoveSpeed, colpoint.normal);

		bool affectB = false;
		float mA = A->m_fMass;;
		float mB = B->m_fMass;;
		float speedSum;
		if(((CPed*)A)->GetPedState() == PED_FOLLOW_PATH){
			affectB = true;	
			speedSum = (2.0f*mA*speedA + mB*speedB)/(2.0f*mA + mB);
		}else{
			speedSum = Max(speedB, 0.0f);
		}

		if(speedA < speedSum){
			if(A->bHasHitWall)
				eA = speedSum;
			else
				eA = speedSum - (speedA - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			impulseA = (eA-speedA) * mA;
			if(!A->bInfiniteMass)
				A->ApplyMoveForce(colpoint.normal*impulseA);
			if(affectB && speedB < speedSum){
				if(B->bHasHitWall)
					eB = speedSum;
				else
					eB = speedSum - (speedB - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
				impulseB = -(eB-speedB) * mB;
				if(!B->bInfiniteMass)
					B->ApplyMoveForce(colpoint.normal*-impulseB);
			}
			return true;
		}
	}else if(A->bPedPhysics){
		CVector pointposB = colpoint.point - B->GetPosition();
		speedA = DotProduct(A->m_vecMoveSpeed, colpoint.normal);
		speedB = DotProduct(B->GetSpeed(pointposB), colpoint.normal);

		float mA = A->m_fMass*massFactorA;
		float mB = B->GetMassTweak(pointposB, colpoint.normal, massFactorB);
		float speedSum;
		if(foo)
			speedSum = speedB;
		else
			speedSum = (mB*speedB + mA*speedA)/(mA + mB);
		if(speedA < speedSum){
			if(A->bHasHitWall)
				eA = speedSum;
			else
				eA = speedSum - (speedA - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			if(B->bHasHitWall)
				eB = speedSum;
			else
				eB = speedSum - (speedB - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			impulseA = (eA - speedA) * mA;
			impulseB = -(eB - speedB) * mB;
			CVector fA = colpoint.normal*(impulseA/massFactorA);
			CVector fB = colpoint.normal*(-impulseB/massFactorB);
			if(!A->bInfiniteMass){
				if(fA.z < 0.0f) fA.z = 0.0f;
				if(ispedcontactB){
					fA.x *= 2.0f;
					fA.y *= 2.0f;
				}
				A->ApplyMoveForce(fA);
			}
			if(!B->bInfiniteMass && !ispedcontactB){
				B->ApplyMoveForce(fB);
				B->ApplyTurnForce(fB, pointposB);
			}
			return true;
		}
	}else if(B->bPedPhysics){
		CVector pointposA = colpoint.point - A->GetPosition();
		speedA = DotProduct(A->GetSpeed(pointposA), colpoint.normal);
		speedB = DotProduct(B->m_vecMoveSpeed, colpoint.normal);

		float mA = A->GetMassTweak(pointposA, colpoint.normal, massFactorA);
		float mB = B->m_fMass*massFactorB;
		float speedSum = (mB*speedB + mA*speedA)/(mA + mB);
		if(speedA < speedSum){
			if(A->bHasHitWall)
				eA = speedSum;
			else
				eA = speedSum - (speedA - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			if(B->bHasHitWall)
				eB = speedSum;
			else
				eB = speedSum - (speedB - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			impulseA = (eA - speedA) * mA;
			impulseB = -(eB - speedB) * mB;
			CVector fA = colpoint.normal*(impulseA/massFactorA);
			CVector fB = colpoint.normal*(-impulseB/massFactorB);
			if(!A->bInfiniteMass && !ispedcontactA){
				if(fA.z < 0.0f) fA.z = 0.0f;
				A->ApplyMoveForce(fA);
				A->ApplyTurnForce(fA, pointposA);
			}
			if(!B->bInfiniteMass){
				if(fB.z < 0.0f){
					fB.z = 0.0f;
					if(Abs(speedA) < 0.01f)
						fB *= 0.5f;
				}
				if(ispedcontactA){
					fB.x *= 2.0f;
					fB.y *= 2.0f;
				}
				B->ApplyMoveForce(fB);
			}
			return true;
		}
	}else{
		CVector pointposA = colpoint.point - A->GetPosition();
		CVector pointposB = colpoint.point - B->GetPosition();
		speedA = DotProduct(A->GetSpeed(pointposA), colpoint.normal);
		speedB = DotProduct(B->GetSpeed(pointposB), colpoint.normal);
		float mA = A->GetMassTweak(pointposA, colpoint.normal, massFactorA);
		float mB = B->GetMassTweak(pointposB, colpoint.normal, massFactorB);
		float speedSum = (mB*speedB + mA*speedA)/(mA + mB);
		if(speedA < speedSum){
			if(A->bHasHitWall)
				eA = speedSum;
			else
				eA = speedSum - (speedA - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			if(B->bHasHitWall)
				eB = speedSum;
			else
				eB = speedSum - (speedB - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			impulseA = (eA - speedA) * mA;
			impulseB = -(eB - speedB) * mB;
			CVector fA = colpoint.normal*(impulseA/massFactorA);
			CVector fB = colpoint.normal*(-impulseB/massFactorB);
			if(A->IsVehicle() && !A->bHasHitWall){
				fA.x *= 1.4f;
				fA.y *= 1.4f;
				if(colpoint.normal.z < 0.7f)
					fA.z *= 0.3f;
				if(A->GetStatus() == STATUS_PLAYER)
					pointposA *= 0.8f;
				if(CWorld::bNoMoreCollisionTorque){
					A->ApplyFrictionMoveForce(fA*-0.3f);
					A->ApplyFrictionTurnForce(fA*-0.3f, pointposA);
				}
			}
			if(B->IsVehicle() && !B->bHasHitWall){
				fB.x *= 1.4f;
				fB.y *= 1.4f;
				if(-colpoint.normal.z < 0.7f)
					fB.z *= 0.3f;
				if(B->GetStatus() == STATUS_PLAYER)
					pointposB *= 0.8f;
				if(CWorld::bNoMoreCollisionTorque){
					// BUG: the game actually uses A here, but this can't be right
					B->ApplyFrictionMoveForce(fB*-0.3f);
					B->ApplyFrictionTurnForce(fB*-0.3f, pointposB);
				}
			}
			if(!A->bInfiniteMass){
				A->ApplyMoveForce(fA);
				A->ApplyTurnForce(fA, pointposA);
			}
			if(!B->bInfiniteMass){
				if(B->bIsInSafePosition)
					B->UnsetIsInSafePosition();
				B->ApplyMoveForce(fB);
				B->ApplyTurnForce(fB, pointposB);
			}
			return true;
		}
	}
	return false;
}

bool
CPhysical::ApplyCollision(CColPoint &colpoint, float &impulse)
{
	float speed;
	if(bPedPhysics){
		speed = DotProduct(m_vecMoveSpeed, colpoint.normal);
		if(speed < 0.0f){
			impulse = -speed * m_fMass;
			ApplyMoveForce(colpoint.normal*impulse);
			return true;
		}
	}else{
		CVector pointpos = colpoint.point - GetPosition();
		speed = DotProduct(GetSpeed(pointpos), colpoint.normal);

		if(speed < 0.0f){
			float mass = GetMass(pointpos, colpoint.normal);
			impulse = -(m_fElasticity + 1.0f) * speed * mass;
			CVector f = colpoint.normal*impulse;
			if(IsVehicle()){
				f.x *= 1.4f;
				f.y *= 1.4f;
				if(colpoint.normal.z < 0.7f)
					f.z *= 0.3f;
			}
			if(!bInfiniteMass){
				ApplyMoveForce(f);
				if(!IsVehicle() || !CWorld::bNoMoreCollisionTorque)
					ApplyTurnForce(f, pointpos);
			}
			return true;
		}
	}

	return false;
}

bool
CPhysical::ApplyCollisionAlt(CEntity *B, CColPoint &colpoint, float &impulse, CVector &moveSpeed, CVector &turnSpeed)
{
	float normalSpeed;
	CVector speed;
	CVector vImpulse;

	if(GetModelIndex() == MI_BEACHBALL && B != (CEntity*)FindPlayerPed())
		((CObject*)this)->m_nBeachballBounces = 0;

	if(bPedPhysics){
		normalSpeed = DotProduct(m_vecMoveSpeed, colpoint.normal);
		if(normalSpeed < 0.0f){
			impulse = -normalSpeed * m_fMass;
			ApplyMoveForce(colpoint.normal * impulse);
			return true;
		}
	}else{
		CVector pointpos = colpoint.point - GetPosition();
		speed = GetSpeed(pointpos);
		normalSpeed = DotProduct(speed, colpoint.normal);
		if(normalSpeed < 0.0f){
			int16 elasticityType = 0;
			float mass = GetMass(pointpos, colpoint.normal);
			float minspeed = GRAVITY * CTimer::GetTimeStep();

			if(IsObject())
				elasticityType = 1;
			else if(IsVehicle() && !bIsInWater){
				if(((CVehicle*)this)->IsBike() && (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED)){
					minspeed *= 1.3f;
					elasticityType = 3;
				}else if(((CVehicle*)this)->IsBoat()){
					minspeed *= 1.2f;
					elasticityType = 4;
				}else if(GetUp().z < -0.3f){
					minspeed *= 1.1f;
					elasticityType = 2;
				}
			}

			if(elasticityType == 1 && !bHasContacted &&
			   Abs(m_vecMoveSpeed.x) < minspeed &&
			   Abs(m_vecMoveSpeed.y) < minspeed &&
			   Abs(m_vecMoveSpeed.z) < minspeed*2.0f)
				impulse = -0.98f * normalSpeed * mass;
			if(elasticityType == 3 &&
			   Abs(m_vecMoveSpeed.x) < minspeed &&
			   Abs(m_vecMoveSpeed.y) < minspeed &&
			   Abs(m_vecMoveSpeed.z) < minspeed*2.0f)
				impulse = -0.8f * normalSpeed * mass;
			else if(elasticityType == 2 &&
			   Abs(m_vecMoveSpeed.x) < minspeed &&
			   Abs(m_vecMoveSpeed.y) < minspeed &&
			   Abs(m_vecMoveSpeed.z) < minspeed*2.0f)
				impulse = -0.92f * normalSpeed * mass;
			else if(elasticityType == 4 &&
			   Abs(m_vecMoveSpeed.x) < minspeed &&
			   Abs(m_vecMoveSpeed.y) < minspeed &&
			   Abs(m_vecMoveSpeed.z) < minspeed*2.0f)
				impulse = -0.8f * normalSpeed * mass;
			else if(IsVehicle() && ((CVehicle*)this)->IsBoat() &&
			   colpoint.surfaceB == SURFACE_WOOD_SOLID && colpoint.normal.z < 0.5f)
				impulse = -(2.0f * m_fElasticity + 1.0f) * normalSpeed * mass;
			else
				impulse = -(m_fElasticity + 1.0f) * normalSpeed * mass;

			// ApplyMoveForce
			vImpulse = colpoint.normal*impulse;
			if(IsVehicle()){
				if(!bHasHitWall ||
				   !(m_vecMoveSpeed.MagnitudeSqr() > 0.1 || !(B->IsBuilding() || ((CPhysical*)B)->bInfiniteMass)))
					moveSpeed += vImpulse * 1.2f * (1.0f/m_fMass);
				else
					moveSpeed += vImpulse * (1.0f/m_fMass);
				vImpulse *= 0.8f;
			}else
				moveSpeed += vImpulse * (1.0f/m_fMass);

			// ApplyTurnForce
			CVector com = Multiply3x3(m_matrix, m_vecCentreOfMass);
			CVector turnimpulse = CrossProduct(pointpos-com, vImpulse);
			turnSpeed += turnimpulse*(1.0f/m_fTurnMass);

			return true;
		}
	}
	return false;
}

bool
CPhysical::ApplyFriction(CPhysical *B, float adhesiveLimit, CColPoint &colpoint)
{
	CVector speedA, speedB;
	float normalSpeedA, normalSpeedB;
	CVector vOtherSpeedA, vOtherSpeedB;
	float fOtherSpeedA, fOtherSpeedB;
	float speedSum;
	CVector frictionDir;
	float impulseA, impulseB;
	float impulseLimit;
	CPhysical *A = this;

	if(A->bPedPhysics && B->bPedPhysics){
		normalSpeedA = DotProduct(A->m_vecMoveSpeed, colpoint.normal);
		normalSpeedB = DotProduct(B->m_vecMoveSpeed, colpoint.normal);
		vOtherSpeedA = A->m_vecMoveSpeed - colpoint.normal*normalSpeedA;
		vOtherSpeedB = B->m_vecMoveSpeed - colpoint.normal*normalSpeedB;

		fOtherSpeedA = vOtherSpeedA.Magnitude();
		fOtherSpeedB = vOtherSpeedB.Magnitude();

		frictionDir = vOtherSpeedA * (1.0f/fOtherSpeedA);
		speedSum = (B->m_fMass*fOtherSpeedB + A->m_fMass*fOtherSpeedA)/(B->m_fMass + A->m_fMass);
		if(fOtherSpeedA > speedSum){
			impulseA = (speedSum - fOtherSpeedA) * A->m_fMass;
			impulseB = (speedSum - fOtherSpeedB) * B->m_fMass;
			impulseLimit = adhesiveLimit*CTimer::GetTimeStep();
			if(impulseA < -impulseLimit) impulseA = -impulseLimit;
			if(impulseB > impulseLimit) impulseB = impulseLimit;		// BUG: game has A's clamp again here, but this can't be right
			A->ApplyFrictionMoveForce(frictionDir*impulseA);
			B->ApplyFrictionMoveForce(frictionDir*impulseB);
			return true;
		}
	}else if(A->bPedPhysics){
		if(B->IsVehicle())
			return false;
		CVector pointposB = colpoint.point - B->GetPosition();
		speedB = B->GetSpeed(pointposB);

		normalSpeedA = DotProduct(A->m_vecMoveSpeed, colpoint.normal);
		normalSpeedB = DotProduct(speedB, colpoint.normal);
		vOtherSpeedA = A->m_vecMoveSpeed - colpoint.normal*normalSpeedA;
		vOtherSpeedB = speedB - colpoint.normal*normalSpeedB;

		fOtherSpeedA = vOtherSpeedA.Magnitude();
		fOtherSpeedB = vOtherSpeedB.Magnitude();

		frictionDir = vOtherSpeedA * (1.0f/fOtherSpeedA);
		float massB = B->GetMass(pointposB, frictionDir);
		speedSum = (massB*fOtherSpeedB + A->m_fMass*fOtherSpeedA)/(massB + A->m_fMass);
		if(fOtherSpeedA > speedSum){
			impulseA = (speedSum - fOtherSpeedA) * A->m_fMass;
			impulseB = (speedSum - fOtherSpeedB) * massB;
			impulseLimit = adhesiveLimit*CTimer::GetTimeStep();
			if(impulseA < -impulseLimit) impulseA = -impulseLimit;
			if(impulseB > impulseLimit) impulseB = impulseLimit;
			A->ApplyFrictionMoveForce(frictionDir*impulseA);
			B->ApplyFrictionMoveForce(frictionDir*impulseB);
			B->ApplyFrictionTurnForce(frictionDir*impulseB, pointposB);
			return true;
		}
	}else if(B->bPedPhysics){
		if(A->IsVehicle())
			return false;
		CVector pointposA = colpoint.point - A->GetPosition();
		speedA = A->GetSpeed(pointposA);

		normalSpeedA = DotProduct(speedA, colpoint.normal);
		normalSpeedB = DotProduct(B->m_vecMoveSpeed, colpoint.normal);
		vOtherSpeedA = speedA - colpoint.normal*normalSpeedA;
		vOtherSpeedB = B->m_vecMoveSpeed - colpoint.normal*normalSpeedB;

		fOtherSpeedA = vOtherSpeedA.Magnitude();
		fOtherSpeedB = vOtherSpeedB.Magnitude();

		frictionDir = vOtherSpeedA * (1.0f/fOtherSpeedA);
		float massA = A->GetMass(pointposA, frictionDir);
		speedSum = (B->m_fMass*fOtherSpeedB + massA*fOtherSpeedA)/(B->m_fMass + massA);
		if(fOtherSpeedA > speedSum){
			impulseA = (speedSum - fOtherSpeedA) * massA;
			impulseB = (speedSum - fOtherSpeedB) * B->m_fMass;
			impulseLimit = adhesiveLimit*CTimer::GetTimeStep();
			if(impulseA < -impulseLimit) impulseA = -impulseLimit;
			if(impulseB > impulseLimit) impulseB = impulseLimit;
			A->ApplyFrictionMoveForce(frictionDir*impulseA);
			A->ApplyFrictionTurnForce(frictionDir*impulseA, pointposA);
			B->ApplyFrictionMoveForce(frictionDir*impulseB);
			return true;
		}
	}else{
		CVector pointposA = colpoint.point - A->GetPosition();
		CVector pointposB = colpoint.point - B->GetPosition();
		speedA = A->GetSpeed(pointposA);
		speedB = B->GetSpeed(pointposB);

		normalSpeedA = DotProduct(speedA, colpoint.normal);
		normalSpeedB = DotProduct(speedB, colpoint.normal);
		vOtherSpeedA = speedA - colpoint.normal*normalSpeedA;
		vOtherSpeedB = speedB - colpoint.normal*normalSpeedB;

		fOtherSpeedA = vOtherSpeedA.Magnitude();
		fOtherSpeedB = vOtherSpeedB.Magnitude();

		frictionDir = vOtherSpeedA * (1.0f/fOtherSpeedA);
		float massA = A->GetMass(pointposA, frictionDir);
		float massB = B->GetMass(pointposB, frictionDir);
		speedSum = (massB*fOtherSpeedB + massA*fOtherSpeedA)/(massB + massA);
		if(fOtherSpeedA > speedSum){
			impulseA = (speedSum - fOtherSpeedA) * massA;
			impulseB = (speedSum - fOtherSpeedB) * massB;
			impulseLimit = adhesiveLimit*CTimer::GetTimeStep();
			if(impulseA < -impulseLimit) impulseA = -impulseLimit;
			if(impulseB > impulseLimit) impulseB = impulseLimit;
			A->ApplyFrictionMoveForce(frictionDir*impulseA);
			A->ApplyFrictionTurnForce(frictionDir*impulseA, pointposA);
			B->ApplyFrictionMoveForce(frictionDir*impulseB);
			B->ApplyFrictionTurnForce(frictionDir*impulseB, pointposB);
			return true;
		}
	}
	return false;
}

bool
CPhysical::ApplyFriction(float adhesiveLimit, CColPoint &colpoint)
{
	CVector speed;
	float normalSpeed;
	CVector vOtherSpeed;
	float fOtherSpeed;
	CVector frictionDir;
	float fImpulse;
	float impulseLimit;

	if(bPedPhysics){
		normalSpeed = DotProduct(m_vecMoveSpeed, colpoint.normal);
		vOtherSpeed = m_vecMoveSpeed - colpoint.normal*normalSpeed;

		fOtherSpeed = vOtherSpeed.Magnitude();
		if(fOtherSpeed > 0.0f){
			frictionDir = vOtherSpeed * (1.0f/fOtherSpeed);
			// not really impulse but speed
			// maybe use ApplyFrictionMoveForce instead?
			fImpulse = -fOtherSpeed;
			impulseLimit = adhesiveLimit*CTimer::GetTimeStep() / m_fMass;
			if(fImpulse < -impulseLimit) fImpulse = -impulseLimit;
			CVector vImpulse = frictionDir*fImpulse;
			m_vecMoveFriction += CVector(vImpulse.x, vImpulse.y, 0.0f);
			return true;
		}
	}else{
		CVector pointpos = colpoint.point - GetPosition();
		speed = GetSpeed(pointpos);
		normalSpeed = DotProduct(speed, colpoint.normal);
		vOtherSpeed = speed - colpoint.normal*normalSpeed;

		fOtherSpeed = vOtherSpeed.Magnitude();
		if(fOtherSpeed > 0.0f){
			frictionDir = vOtherSpeed * (1.0f/fOtherSpeed);
			fImpulse = -fOtherSpeed * m_fMass;
			impulseLimit = adhesiveLimit*CTimer::GetTimeStep() * 1.5f;
			if(fImpulse < -impulseLimit) fImpulse = -impulseLimit;
			ApplyFrictionMoveForce(frictionDir*fImpulse);
			ApplyFrictionTurnForce(frictionDir*fImpulse, pointpos);

			if(fOtherSpeed > 0.1f &&
			   colpoint.surfaceB != SURFACE_GRASS && colpoint.surfaceB != SURFACE_MUD_DRY &&
			   CSurfaceTable::GetAdhesionGroup(colpoint.surfaceA) == ADHESIVE_HARD){
				CVector v = frictionDir * fOtherSpeed * 0.25f;
				for(int i = 0; i < 4; i++)
					CParticle::AddParticle(PARTICLE_SPARK_SMALL, colpoint.point, v);
			}
			return true;
		}
	}
	return false;
}

bool
CPhysical::ProcessShiftSectorList(CPtrList *lists)
{
	int i, j;
	CPtrList *list;
	CPtrNode *node;
	CPhysical *A, *B;
	CObject *Bobj;
	bool canshift;
	CVector center;
	float radius;

	int numCollisions;
	int mostColliding;
	CColPoint colpoints[MAX_COLLISION_POINTS];
	CVector shift = { 0.0f, 0.0f, 0.0f };
	bool doShift = false;
	CEntity *boat = nil;

	bool skipShift;

	A = this;

	A->GetBoundCentre(center);
	radius = A->GetBoundRadius();
	for(i = 0; i <= ENTITYLIST_PEDS_OVERLAP; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			B = (CPhysical*)node->item;
			Bobj = (CObject*)B;
			skipShift = false;

			if(B->IsBuilding() ||
			   B->IsObject() && B->bInfiniteMass)
				canshift = true;
			else
				canshift = A->IsPed() &&
					B->IsObject() && B->IsStatic() && !Bobj->bHasBeenDamaged;
			if(B == A ||
			   B->m_scanCode == CWorld::GetCurrentScanCode() ||
			   !B->bUsesCollision ||
			   (A->bHasHitWall && !canshift) ||
			   !B->GetIsTouching(center, radius))
				continue;

			// This could perhaps be done a bit nicer

			if(B->IsBuilding())
				skipShift = false;
			else if(IsLightWithoutShift(A->GetModelIndex()) &&
			  (B->IsVehicle() || B->IsPed()) &&
			  A->GetUp().z < 0.66f)
				skipShift = true;
			else if((A->IsVehicle() || A->IsPed()) &&
			  B->GetUp().z < 0.66f &&
			  IsLightWithoutShift(B->GetModelIndex()))
				skipShift = true;
			else if(A->IsObject() && B->IsVehicle()){
				CObject *Aobj = (CObject*)A;
				if(Aobj->ObjectCreatedBy != TEMP_OBJECT &&
				   !Aobj->bHasBeenDamaged &&
				   Aobj->IsStatic()){
					if(Aobj->m_pCollidingEntity == B)
						Aobj->m_pCollidingEntity = nil;
				}else if(Aobj->m_pCollidingEntity != B){
					CMatrix inv;
					CVector size = CModelInfo::GetModelInfo(A->GetModelIndex())->GetColModel()->boundingBox.GetSize();
					size = A->GetMatrix() * size;
					if(size.z < B->GetPosition().z ||
					   (Invert(B->GetMatrix(), inv) * size).z < 0.0f){
						skipShift = true;
						Aobj->m_pCollidingEntity = B;
					}
				}
			}else if(B->IsObject() && A->IsVehicle()){
				CObject *Bobj = (CObject*)B;
				if(Bobj->ObjectCreatedBy != TEMP_OBJECT &&
				   !Bobj->bHasBeenDamaged &&
				   Bobj->IsStatic()){
					if(Bobj->m_pCollidingEntity == A)
						Bobj->m_pCollidingEntity = nil;
				}else if(Bobj->m_pCollidingEntity != A){
					CMatrix inv;
					CVector size = CModelInfo::GetModelInfo(B->GetModelIndex())->GetColModel()->boundingBox.GetSize();
					size = B->GetMatrix() * size;
					if(size.z < A->GetPosition().z ||
					   (Invert(A->GetMatrix(), inv) * size).z < 0.0f)
						skipShift = true;
				}
			}else if(IsBodyPart(A->GetModelIndex()) && B->IsPed())
				skipShift = true;
			else if(A->IsPed() && IsBodyPart(B->GetModelIndex()))
				skipShift = true;
			else if(A->IsPed() && ((CPed*)A)->m_pCollidingEntity == B ||
			  B->IsPed() && ((CPed*)B)->m_pCollidingEntity == A ||
			  A->GetModelIndex() == MI_RCBANDIT && B->IsVehicle() ||
			  B->GetModelIndex() == MI_RCBANDIT && (A->IsPed() || A->IsVehicle()))
				skipShift = true;

			if(skipShift)
				continue;

			B->m_scanCode = CWorld::GetCurrentScanCode();
			numCollisions = A->ProcessEntityCollision(B, colpoints);
			if(numCollisions <= 0)
				continue;

			mostColliding = 0;
			for(j = 1; j < numCollisions; j++)
				if(colpoints[j].depth > colpoints[mostColliding].depth)
					mostColliding = j;

			if(CWorld::bSecondShift)
				for(j = 0; j < numCollisions; j++)
					shift += colpoints[j].normal * colpoints[j].depth * 1.5f/numCollisions;
			else
				for(j = 0; j < numCollisions; j++)
					shift += colpoints[j].normal * colpoints[j].depth * 1.2f/numCollisions;

			if(A->IsVehicle() && B->IsVehicle()){
				CVector dir = A->GetPosition() - B->GetPosition();
				dir.Normalise();
				if(dir.z < 0.0f && dir.z < A->GetForward().z && dir.z < A->GetRight().z)
					dir.z = Min(0.0f, Min(A->GetForward().z, A->GetRight().z));
				shift += dir * colpoints[mostColliding].depth * 0.5f;
			}else if(A->IsPed() && B->IsVehicle() && ((CVehicle*)B)->IsBoat()){
				CVector dir = colpoints[mostColliding].normal;
				float f = Min(Abs(dir.z), 0.9f);
				dir.z = 0.0f;
				dir.Normalise();
				shift += dir * colpoints[mostColliding].depth / (1.0f - f);
				boat = B;
			}else if(B->IsPed() && A->IsVehicle() && ((CVehicle*)A)->IsBoat()){
				CVector dir = colpoints[mostColliding].normal * -1.0f;
				float f = Min(Abs(dir.z), 0.9f);
				dir.z = 0.0f;
				dir.Normalise();
				B->GetMatrix().Translate(dir * colpoints[mostColliding].depth / (1.0f - f));
				// BUG? how can that ever happen? A is a Ped
				if(B->IsVehicle())
					B->ProcessEntityCollision(A, colpoints);
			}else{
				if(CWorld::bSecondShift)
					shift += colpoints[mostColliding].normal * colpoints[mostColliding].depth * 0.4f;
				else
					shift += colpoints[mostColliding].normal * colpoints[mostColliding].depth * 0.2f;
			}

			doShift = true;
		}
	}

	if(!doShift)
		return false;
	GetMatrix().Translate(shift);
	if(boat)
		ProcessEntityCollision(boat, colpoints);
	return true;
}

bool
CPhysical::ProcessCollisionSectorList_SimpleCar(CPtrList *lists)
{
	static CColPoint aColPoints[MAX_COLLISION_POINTS];
	float radius;
	CVector center;
	int listtype;
	CPhysical *A, *B;
	int numCollisions;
	int i;
	float impulseA = -1.0f;
	float impulseB = -1.0f;

	A = (CPhysical*)this;

	if(!A->bUsesCollision)
		return false;

	radius = A->GetBoundRadius();
	A->GetBoundCentre(center);

	for(listtype = 3; listtype >= 0; listtype--){
		// Go through vehicles and objects
		CPtrList *list;
		switch(listtype){
		case 0:	list = &lists[ENTITYLIST_VEHICLES]; break;
		case 1:	list = &lists[ENTITYLIST_VEHICLES_OVERLAP]; break;
		case 2:	list = &lists[ENTITYLIST_OBJECTS]; break;
		case 3:	list = &lists[ENTITYLIST_OBJECTS_OVERLAP]; break;
		}

		// Find first collision in list
		CPtrNode *listnode;
		for(listnode = list->first; listnode; listnode = listnode->next){
			B = (CPhysical*)listnode->item;
			if(B != A &&
			   !(B->IsObject() && ((CObject*)B)->bIsStreetLight && B->GetUp().z < 0.66f) &&
			   B->m_scanCode != CWorld::GetCurrentScanCode() &&
			   B->bUsesCollision &&
			   B->GetIsTouching(center, radius)){
				B->m_scanCode = CWorld::GetCurrentScanCode();
				numCollisions = A->ProcessEntityCollision(B, aColPoints);
				if(numCollisions > 0)
					goto collision;
			}
		}
	}
	// no collision
	return false;

collision:

	if(A->bHasContacted && B->bHasContacted){
		for(i = 0; i < numCollisions; i++){
			if(!A->ApplyCollision(B, aColPoints[i], impulseA, impulseB))
				continue;

			if(impulseA > A->m_fDamageImpulse)
				A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

			if(impulseB > B->m_fDamageImpulse)
				B->SetDamagedPieceRecord(aColPoints[i].pieceB, impulseB, A, aColPoints[i].normal);

			float turnSpeedDiff = (B->m_vecTurnSpeed - A->m_vecTurnSpeed).MagnitudeSqr();
			float moveSpeedDiff = (B->m_vecMoveSpeed - A->m_vecMoveSpeed).MagnitudeSqr();

			DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));
		}
	}else if(A->bHasContacted){
		CVector savedMoveFriction = A->m_vecMoveFriction;
		CVector savedTurnFriction = A->m_vecTurnFriction;
		A->m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
		A->m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
		A->bHasContacted = false;

		for(i = 0; i < numCollisions; i++){
			if(!A->ApplyCollision(B, aColPoints[i], impulseA, impulseB))
				continue;

			if(impulseA > A->m_fDamageImpulse)
				A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

			if(impulseB > B->m_fDamageImpulse)
				B->SetDamagedPieceRecord(aColPoints[i].pieceB, impulseB, A, aColPoints[i].normal);

			float turnSpeedDiff = (B->m_vecTurnSpeed - A->m_vecTurnSpeed).MagnitudeSqr();
			float moveSpeedDiff = (B->m_vecMoveSpeed - A->m_vecMoveSpeed).MagnitudeSqr();

			DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));

			if(A->ApplyFriction(B, CSurfaceTable::GetAdhesiveLimit(aColPoints[i])/numCollisions, aColPoints[i])){
				A->bHasContacted = true;
				B->bHasContacted = true;
			}
		}

		if(!A->bHasContacted){
			A->bHasContacted = true;
			A->m_vecMoveFriction = savedMoveFriction;
			A->m_vecTurnFriction = savedTurnFriction;
		}
	}else if(B->bHasContacted){
		CVector savedMoveFriction = B->m_vecMoveFriction;
		CVector savedTurnFriction = B->m_vecTurnFriction;
		B->m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
		B->m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
		B->bHasContacted = false;

		for(i = 0; i < numCollisions; i++){
			if(!A->ApplyCollision(B, aColPoints[i], impulseA, impulseB))
				continue;

			if(impulseA > A->m_fDamageImpulse)
				A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

			if(impulseB > B->m_fDamageImpulse)
				B->SetDamagedPieceRecord(aColPoints[i].pieceB, impulseB, A, aColPoints[i].normal);

			float turnSpeedDiff = (B->m_vecTurnSpeed - A->m_vecTurnSpeed).MagnitudeSqr();
			float moveSpeedDiff = (B->m_vecMoveSpeed - A->m_vecMoveSpeed).MagnitudeSqr();

			DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));

			if(A->ApplyFriction(B, CSurfaceTable::GetAdhesiveLimit(aColPoints[i])/numCollisions, aColPoints[i])){
				A->bHasContacted = true;
				B->bHasContacted = true;
			}
		}

		if(!B->bHasContacted){
			B->bHasContacted = true;
			B->m_vecMoveFriction = savedMoveFriction;
			B->m_vecTurnFriction = savedTurnFriction;
		}
	}else{
		for(i = 0; i < numCollisions; i++){
			if(!A->ApplyCollision(B, aColPoints[i], impulseA, impulseB))
				continue;

			if(impulseA > A->m_fDamageImpulse)
				A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

			if(impulseB > B->m_fDamageImpulse)
				B->SetDamagedPieceRecord(aColPoints[i].pieceB, impulseB, A, aColPoints[i].normal);

			float turnSpeedDiff = (B->m_vecTurnSpeed - A->m_vecTurnSpeed).MagnitudeSqr();
			float moveSpeedDiff = (B->m_vecMoveSpeed - A->m_vecMoveSpeed).MagnitudeSqr();

			DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));

			if(A->ApplyFriction(B, CSurfaceTable::GetAdhesiveLimit(aColPoints[i])/numCollisions, aColPoints[i])){
				A->bHasContacted = true;
				B->bHasContacted = true;
			}
		}
	}

	if(B->GetStatus() == STATUS_SIMPLE){
		B->SetStatus(STATUS_PHYSICS);
		if(B->IsVehicle())
			CCarCtrl::SwitchVehicleToRealPhysics((CVehicle*)B);
	}

	return true;
}

bool
CPhysical::ProcessCollisionSectorList(CPtrList *lists)
{
	static CColPoint aColPoints[MAX_COLLISION_POINTS];
	float radius;
	CVector center;
	CPtrList *list;
	CPhysical *A, *B;
	CObject *Aobj, *Bobj;
	CPed *Aped, *Bped;
	int numCollisions;
	int numResponses;
	int i, j;
	bool skipCollision, altcollision;
	bool ret = false;
	float impulseA = -1.0f;
	float impulseB = -1.0f;

	A = (CPhysical*)this;
	Aobj = (CObject*)A;
	Aped = (CPed*)A;

	radius = A->GetBoundRadius();
	A->GetBoundCentre(center);

	for(j = 0; j <= ENTITYLIST_PEDS_OVERLAP; j++){
		list = &lists[j];

		CPtrNode *listnode;
		for(listnode = list->first; listnode; listnode = listnode->next){
			B = (CPhysical*)listnode->item;
			Bobj = (CObject*)B;
			Bped = (CPed*)B;

			bool isTouching = true;
			if(!B->bUsesCollision ||
			   B->m_scanCode == CWorld::GetCurrentScanCode() ||
			   B == A ||			   
			   !(isTouching = B->GetIsTouching(center, radius))){
				if(!isTouching){
					if(A->IsObject() && Aobj->m_pCollidingEntity == B)
						Aobj->m_pCollidingEntity = nil;
					else if(B->IsObject() && Bobj->m_pCollidingEntity == A)
						Bobj->m_pCollidingEntity = nil;
					else if(A->IsPed() && Aped->m_pCollidingEntity == B)
						Aped->m_pCollidingEntity = nil;
					else if(B->IsPed() && Bped->m_pCollidingEntity == A)
						Bped->m_pCollidingEntity = nil;
				}
				continue;
			}

			A->bSkipLineCol = false;
			skipCollision = false;
			altcollision = false;

			if(B->IsBuilding())
				skipCollision = false;
			else if(A->IsObject() && Aobj->bIsStreetLight &&
			  (B->IsVehicle() || B->IsPed()) &&
			  A->GetUp().z < 0.66f){
				skipCollision = true;
				A->bSkipLineCol = true;
				Aobj->m_pCollidingEntity = B;
			}else if(B->IsObject() && Bobj->bIsStreetLight &&
			  (A->IsVehicle() || A->IsPed()) &&
			  B->GetUp().z < 0.66f){
				skipCollision = true;
				A->bSkipLineCol = true;
				Bobj->m_pCollidingEntity = A;
			}else if(A->IsObject() && B->IsVehicle()){
				if(A->GetModelIndex() == MI_CAR_BUMPER)
					skipCollision = true;
				else if(Aobj->ObjectCreatedBy == TEMP_OBJECT ||
				   Aobj->bHasBeenDamaged ||
				   !Aobj->IsStatic()){
					if(Aobj->m_pCollidingEntity == B)
						skipCollision = true;
					else if(Aobj->m_nCollisionDamageEffect < DAMAGE_EFFECT_SMASH_COMPLETELY){
						CMatrix inv;
						CVector size = CModelInfo::GetModelInfo(A->GetModelIndex())->GetColModel()->boundingBox.GetSize();
						size = A->GetMatrix() * size;
						if(size.z < B->GetPosition().z ||
						   (Invert(B->GetMatrix(), inv) * size).z < 0.0f){
							skipCollision = true;
							Aobj->m_pCollidingEntity = B;
						}
					}
				}
			}else if(B->IsObject() && A->IsVehicle()){
				if(B->GetModelIndex() == MI_CAR_BUMPER)
					skipCollision = true;
				else if(Bobj->ObjectCreatedBy == TEMP_OBJECT ||
				   Bobj->bHasBeenDamaged ||
				   !Bobj->IsStatic()){
					if(Bobj->m_pCollidingEntity == A)
						skipCollision = true;
					else if(Bobj->m_nCollisionDamageEffect < DAMAGE_EFFECT_SMASH_COMPLETELY){
						CMatrix inv;
						CVector size = CModelInfo::GetModelInfo(B->GetModelIndex())->GetColModel()->boundingBox.GetSize();
						size = B->GetMatrix() * size;
						if(size.z < A->GetPosition().z ||
						   (Invert(A->GetMatrix(), inv) * size).z < 0.0f){
							skipCollision = true;
						}
					}
				}
			}else if(A->GetModelIndex() == MI_GRENADE && B->IsPed() &&
			  A->GetPosition().z < B->GetPosition().z){
				skipCollision = true;
			}else if(B->GetModelIndex() == MI_GRENADE && A->IsPed() &&
			  B->GetPosition().z < A->GetPosition().z){
				skipCollision = true;
				A->bSkipLineCol = true;
			}else if(A->IsPed() && Aped->m_pCollidingEntity == B){
				skipCollision = true;
				if(!Aped->bKnockedUpIntoAir || Aped->bKnockedOffBike)
					A->bSkipLineCol = true;
			}else if(B->IsPed() && Bped->m_pCollidingEntity == A){
				skipCollision = true;
				A->bSkipLineCol = true;
			}else if(A->GetModelIndex() == MI_RCBANDIT && (B->IsPed() || B->IsVehicle()) ||
			         B->GetModelIndex() == MI_RCBANDIT && (A->IsPed() || A->IsVehicle())){
				skipCollision = true;
				A->bSkipLineCol = true;
			}else if(A->IsPed() && B->IsObject() && Bobj->m_fUprootLimit > 0.0f)
				altcollision = true;


			if(!A->bUsesCollision || skipCollision){
				B->m_scanCode = CWorld::GetCurrentScanCode();
				numCollisions = A->ProcessEntityCollision(B, aColPoints);
				if(A->bJustCheckCollision && numCollisions > 0)
					return true;
				if(numCollisions == 0 && A == (CEntity*)FindPlayerPed() && Aped->m_pCollidingEntity == B)
					Aped->m_pCollidingEntity = nil;
			}else if(B->IsBuilding() || B->bIsStuck || B->bInfiniteMass || altcollision){

				// This is the case where B doesn't move

				B->m_scanCode = CWorld::GetCurrentScanCode();
				numCollisions = A->ProcessEntityCollision(B, aColPoints);
				if(numCollisions <= 0)
					continue;

				CVector moveSpeed = { 0.0f, 0.0f, 0.0f };
				CVector turnSpeed = { 0.0f, 0.0f, 0.0f };
				float maxImpulseA = 0.0f;
				numResponses = 0;
				if(A->bHasContacted){
					for(i = 0; i < numCollisions; i++){
						if(!A->ApplyCollisionAlt(B, aColPoints[i], impulseA, moveSpeed, turnSpeed))
							continue;

						numResponses++;
						if(impulseA > maxImpulseA) maxImpulseA = impulseA;

						if(A->IsVehicle()){
							if(!(((CVehicle*)A)->IsBoat() && aColPoints[i].surfaceB == SURFACE_WOOD_SOLID) &&
							   impulseA > A->m_fDamageImpulse)
								A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

							if(CSurfaceTable::GetAdhesionGroup(aColPoints[i].surfaceB) == ADHESIVE_SAND)
								aColPoints[i].surfaceB = SURFACE_SAND;

							float turnSpeedDiff = A->m_vecTurnSpeed.MagnitudeSqr();
							float moveSpeedDiff = A->m_vecMoveSpeed.MagnitudeSqr();

							if(A->GetUp().z < -0.6f &&
							   Abs(A->m_vecMoveSpeed.x) < 0.05f &&
							   Abs(A->m_vecMoveSpeed.y) < 0.05f)
								DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, 0.1f*impulseA, Max(turnSpeedDiff, moveSpeedDiff));
							else
								DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));

						}else{
							if(impulseA > A->m_fDamageImpulse)
								A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

							float turnSpeedDiff = A->m_vecTurnSpeed.MagnitudeSqr();
							float moveSpeedDiff = A->m_vecMoveSpeed.MagnitudeSqr();

							DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));
						}
					}
				}else{
					for(i = 0; i < numCollisions; i++){
						if(!A->ApplyCollisionAlt(B, aColPoints[i], impulseA, moveSpeed, turnSpeed))
							continue;

						numResponses++;
						if(impulseA > maxImpulseA) maxImpulseA = impulseA;
						float adhesion = CSurfaceTable::GetAdhesiveLimit(aColPoints[i]) / numCollisions;

						if(A->IsVehicle()){
							if(((CVehicle*)A)->IsBoat() && aColPoints[i].surfaceB == SURFACE_WOOD_SOLID)
								adhesion = 0.0f;
							else if(impulseA > A->m_fDamageImpulse)
								A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

							if(CSurfaceTable::GetAdhesionGroup(aColPoints[i].surfaceB) == ADHESIVE_SAND)
								aColPoints[i].surfaceB = SURFACE_SAND;

							float turnSpeedDiff = A->m_vecTurnSpeed.MagnitudeSqr();
							float moveSpeedDiff = A->m_vecMoveSpeed.MagnitudeSqr();

							if(A->GetUp().z < -0.6f &&
							   Abs(A->m_vecMoveSpeed.x) < 0.05f &&
							   Abs(A->m_vecMoveSpeed.y) < 0.05f)
								DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, 0.1f*impulseA, Max(turnSpeedDiff, moveSpeedDiff));
							else
								DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));


							if(A->GetModelIndex() == MI_RCBANDIT)
								adhesion *= 0.2f;
							else if(((CVehicle*)A)->IsBoat()){
								if(aColPoints[i].normal.z > 0.6f){
									if(CSurfaceTable::GetAdhesionGroup(aColPoints[i].surfaceB) == ADHESIVE_LOOSE ||
									   CSurfaceTable::GetAdhesionGroup(aColPoints[i].surfaceB) == ADHESIVE_SAND)
										adhesion *= 3.0f;
								}else
									adhesion = 0.0f;
							}else if(A->GetStatus() == STATUS_WRECKED)
								adhesion *= 3.0f;
							else if(A->GetUp().z > 0.3f)
								adhesion = 0.0f;
							else
								adhesion *= Min(5.0f, 0.03f*impulseA + 1.0f);
						}else{
							if(impulseA > A->m_fDamageImpulse)
								A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

							float turnSpeedDiff = A->m_vecTurnSpeed.MagnitudeSqr();
							float moveSpeedDiff = A->m_vecMoveSpeed.MagnitudeSqr();

							DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));
						}

						if(A->ApplyFriction(adhesion, aColPoints[i]))
							A->bHasContacted = true;
					}
				}

				if(numResponses){
					m_vecMoveSpeed += moveSpeed / numResponses;
					m_vecTurnSpeed += turnSpeed / numResponses;
					if(!CWorld::bNoMoreCollisionTorque &&
					   A->GetStatus() == STATUS_PLAYER && A->IsVehicle() &&
					   Abs(A->m_vecMoveSpeed.x) > 0.2f &&
					   Abs(A->m_vecMoveSpeed.y) > 0.2f){
						A->m_vecMoveFriction.x += moveSpeed.x * -0.3f / numCollisions;
						A->m_vecMoveFriction.y += moveSpeed.y * -0.3f / numCollisions;
						A->m_vecTurnFriction += turnSpeed * -0.3f / numCollisions;
					}

					if(B->IsObject() && Bobj->m_nCollisionDamageEffect && maxImpulseA > 20.0f)
						Bobj->ObjectDamage(maxImpulseA);

					if(!CWorld::bSecondShift)
						return true;
					ret = true;
				}
			}else{

				// B can move

				B->m_scanCode = CWorld::GetCurrentScanCode();
				numCollisions = A->ProcessEntityCollision(B, aColPoints);
				if(numCollisions <= 0)
					continue;

				float maxImpulseA = 0.0f;
				float maxImpulseB = 0.0f;
				if(A->bHasContacted && B->bHasContacted){
					for(i = 0; i < numCollisions; i++){
						if(!A->ApplyCollision(B, aColPoints[i], impulseA, impulseB))
							continue;

						if(impulseA > maxImpulseA) maxImpulseA = impulseA;
						if(impulseB > maxImpulseB) maxImpulseB = impulseB;

						if(impulseA > A->m_fDamageImpulse)
							A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

						if(impulseB > B->m_fDamageImpulse)
							B->SetDamagedPieceRecord(aColPoints[i].pieceB, impulseB, A, aColPoints[i].normal);

						float turnSpeedDiff = (B->m_vecTurnSpeed - A->m_vecTurnSpeed).MagnitudeSqr();
						float moveSpeedDiff = (B->m_vecMoveSpeed - A->m_vecMoveSpeed).MagnitudeSqr();

						DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));
					}
				}else if(A->bHasContacted){
					CVector savedMoveFriction = A->m_vecMoveFriction;
					CVector savedTurnFriction = A->m_vecTurnFriction;
					A->m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
					A->m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
					A->bHasContacted = false;

					for(i = 0; i < numCollisions; i++){
						if(!A->ApplyCollision(B, aColPoints[i], impulseA, impulseB))
							continue;

						if(impulseA > maxImpulseA) maxImpulseA = impulseA;
						if(impulseB > maxImpulseB) maxImpulseB = impulseB;

						if(impulseA > A->m_fDamageImpulse)
							A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

						if(impulseB > B->m_fDamageImpulse)
							B->SetDamagedPieceRecord(aColPoints[i].pieceB, impulseB, A, aColPoints[i].normal);

						float turnSpeedDiff = (B->m_vecTurnSpeed - A->m_vecTurnSpeed).MagnitudeSqr();
						float moveSpeedDiff = (B->m_vecMoveSpeed - A->m_vecMoveSpeed).MagnitudeSqr();

						DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));

						if(A->ApplyFriction(B, CSurfaceTable::GetAdhesiveLimit(aColPoints[i])/numCollisions, aColPoints[i])){
							A->bHasContacted = true;
							B->bHasContacted = true;
						}
					}

					if(!A->bHasContacted){
						A->bHasContacted = true;
						A->m_vecMoveFriction = savedMoveFriction;
						A->m_vecTurnFriction = savedTurnFriction;
					}
				}else if(B->bHasContacted){
					CVector savedMoveFriction = B->m_vecMoveFriction;
					CVector savedTurnFriction = B->m_vecTurnFriction;
					B->m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
					B->m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
					B->bHasContacted = false;

					for(i = 0; i < numCollisions; i++){
						if(!A->ApplyCollision(B, aColPoints[i], impulseA, impulseB))
							continue;

						if(impulseA > maxImpulseA) maxImpulseA = impulseA;
						if(impulseB > maxImpulseB) maxImpulseB = impulseB;

						if(impulseA > A->m_fDamageImpulse)
							A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

						if(impulseB > B->m_fDamageImpulse)
							B->SetDamagedPieceRecord(aColPoints[i].pieceB, impulseB, A, aColPoints[i].normal);

						float turnSpeedDiff = (B->m_vecTurnSpeed - A->m_vecTurnSpeed).MagnitudeSqr();
						float moveSpeedDiff = (B->m_vecMoveSpeed - A->m_vecMoveSpeed).MagnitudeSqr();

						DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));

						if(A->ApplyFriction(B, CSurfaceTable::GetAdhesiveLimit(aColPoints[i])/numCollisions, aColPoints[i])){
							A->bHasContacted = true;
							B->bHasContacted = true;
						}
					}

					if(!B->bHasContacted){
						B->bHasContacted = true;
						B->m_vecMoveFriction = savedMoveFriction;
						B->m_vecTurnFriction = savedTurnFriction;
					}
				}else{
					for(i = 0; i < numCollisions; i++){
						if(!A->ApplyCollision(B, aColPoints[i], impulseA, impulseB))
							continue;

						if(impulseA > maxImpulseA) maxImpulseA = impulseA;
						if(impulseB > maxImpulseB) maxImpulseB = impulseB;

						if(impulseA > A->m_fDamageImpulse)
							A->SetDamagedPieceRecord(aColPoints[i].pieceA, impulseA, B, aColPoints[i].normal);

						if(impulseB > B->m_fDamageImpulse)
							B->SetDamagedPieceRecord(aColPoints[i].pieceB, impulseB, A, aColPoints[i].normal);

						float turnSpeedDiff = (B->m_vecTurnSpeed - A->m_vecTurnSpeed).MagnitudeSqr();
						float moveSpeedDiff = (B->m_vecMoveSpeed - A->m_vecMoveSpeed).MagnitudeSqr();

						DMAudio.ReportCollision(A, B, aColPoints[i].surfaceA, aColPoints[i].surfaceB, impulseA, Max(turnSpeedDiff, moveSpeedDiff));

						if(A->ApplyFriction(B, CSurfaceTable::GetAdhesiveLimit(aColPoints[i])/numCollisions, aColPoints[i])){
							A->bHasContacted = true;
							B->bHasContacted = true;
						}
					}
				}

				if(B->IsPed() && A->IsVehicle() &&
				   (!Bped->IsPlayer() || B->bHasHitWall && A->m_vecMoveSpeed.MagnitudeSqr() > SQR(0.05f)))
					Bped->KillPedWithCar((CVehicle*)A, maxImpulseB);
				else if(B->GetModelIndex() == MI_TRAIN && A->IsPed() &&
				  (!Aped->IsPlayer() || A->bHasHitWall))
					Aped->KillPedWithCar((CVehicle*)B, maxImpulseA*2.0f);
				else if(B->IsObject() && B->bUsesCollision && A->IsVehicle()){
					// BUG? not impulseA?
					if(Bobj->m_nCollisionDamageEffect && maxImpulseB > 20.0f)
						Bobj->ObjectDamage(maxImpulseB);
					else if(Bobj->m_nCollisionDamageEffect >= DAMAGE_EFFECT_SMASH_COMPLETELY){
						CMatrix inv;
						CVector size = CModelInfo::GetModelInfo(B->GetModelIndex())->GetColModel()->boundingBox.GetSize();
						size = B->GetMatrix() * size;
						if(size.z < B->GetPosition().z ||
						   (Invert(A->GetMatrix(), inv) * size).z < 0.0f)
							Bobj->ObjectDamage(50.0f);
					}
				}else if(A->IsObject() && A->bUsesCollision && B->IsVehicle()){
					if(Aobj->m_nCollisionDamageEffect && maxImpulseB > 20.0f)
						Aobj->ObjectDamage(maxImpulseB);
					else if(Aobj->m_nCollisionDamageEffect >= DAMAGE_EFFECT_SMASH_COMPLETELY){
						CMatrix inv;
						CVector size = CModelInfo::GetModelInfo(A->GetModelIndex())->GetColModel()->boundingBox.GetSize();
						size = A->GetMatrix() * size;
						if(size.z < A->GetPosition().z ||
						   (Invert(B->GetMatrix(), inv) * size).z < 0.0f)
							Aobj->ObjectDamage(50.0f);
					}
				}

				if(B->GetStatus() == STATUS_SIMPLE){
					B->SetStatus(STATUS_PHYSICS);
					if(B->IsVehicle())
						CCarCtrl::SwitchVehicleToRealPhysics((CVehicle*)B);
				}

				if(!CWorld::bSecondShift)
					return true;
				ret = true;
			}
			
		}
	}

	return ret;
}

bool
CPhysical::CheckCollision(void)
{
	CEntryInfoNode *node;

	bCollisionProcessed = false;
	CWorld::AdvanceCurrentScanCode();
	for(node = m_entryInfoList.first; node; node = node->next)
		if(ProcessCollisionSectorList(node->sector->m_lists))
			return true;
	return false;
}

bool
CPhysical::CheckCollision_SimpleCar(void)
{
	CEntryInfoNode *node;

	bCollisionProcessed = false;
	CWorld::AdvanceCurrentScanCode();
	for(node = m_entryInfoList.first; node; node = node->next)
		if(ProcessCollisionSectorList_SimpleCar(node->sector->m_lists))
			return true;
	return false;
}

float PHYSICAL_SHIFT_SPEED_DAMP = 0.707f;

void
CPhysical::ProcessShift(void)
{
	m_fDistanceTravelled = 0.0f;
	if(GetStatus() == STATUS_SIMPLE){
		bIsStuck = false;
		bIsInSafePosition = true;
		RemoveAndAdd();
	}else{
		CPhysical *surf;
		if(bHasHitWall && IsPed() && (surf = ((CPed*)this)->m_pCurrentPhysSurface, surf == nil || !surf->bInfiniteMass || surf->m_phy_flagA08) ||
		   CWorld::bSecondShift){
			m_vecMoveSpeed *= Pow(PHYSICAL_SHIFT_SPEED_DAMP, CTimer::GetTimeStep());
			m_vecTurnSpeed *= Pow(PHYSICAL_SHIFT_SPEED_DAMP, CTimer::GetTimeStep());
		}

		CMatrix matrix(GetMatrix());
		ApplyMoveSpeed();
		ApplyTurnSpeed();
		GetMatrix().Reorthogonalise();

		CWorld::AdvanceCurrentScanCode();

		if(IsVehicle())
			m_bIsVehicleBeingShifted = true;

		CEntryInfoNode *node;
		bool hasshifted = false;
		for(node = m_entryInfoList.first; node; node = node->next)
			hasshifted |= ProcessShiftSectorList(node->sector->m_lists);
		m_bIsVehicleBeingShifted = false;
		if(hasshifted){
			CWorld::AdvanceCurrentScanCode();
			bool hadCollision = false;
			for(node = m_entryInfoList.first; node; node = node->next)
				if(ProcessCollisionSectorList(node->sector->m_lists)){
					if(!CWorld::bSecondShift){
						GetMatrix() = matrix;
						return;
					}
					hadCollision = true;
				}
			if(hadCollision){
				GetMatrix() = matrix;
				return;
			}
		}
		bIsStuck = false;
		bIsInSafePosition = true;
		m_fDistanceTravelled = (GetPosition() - matrix.GetPosition()).Magnitude();
		RemoveAndAdd();
	}
}

// x is the number of units (m) we would like to step
#define NUMSTEPS(x) ceil(Sqrt(distSq) * (1.0f/(x)))

float HIGHSPEED_ELASTICITY_MULT_PED = 2.0f;
float HIGHSPEED_ELASTICITY_MULT_COPCAR = 2.0f;

void
CPhysical::ProcessCollision(void)
{
	int i;
	CPed *ped = (CPed*)this;

	m_fDistanceTravelled = 0.0f;
	m_bIsVehicleBeingShifted = false;
	bSkipLineCol = false;

	if(!bUsesCollision){
		bIsStuck = false;
		bIsInSafePosition = true;
		RemoveAndAdd();
		return;
	}

	if(GetStatus() == STATUS_SIMPLE){
		if(CheckCollision_SimpleCar() && GetStatus() == STATUS_SIMPLE){
			SetStatus(STATUS_PHYSICS);
			if(IsVehicle())
				CCarCtrl::SwitchVehicleToRealPhysics((CVehicle*)this);
		}
		bIsStuck = false;
		bIsInSafePosition = true;
		RemoveAndAdd();
		return;
	}

	// Save current state
	CMatrix savedMatrix(GetMatrix());
	float savedElasticity = m_fElasticity;
	CVector savedMoveSpeed = m_vecMoveSpeed;
	float savedTimeStep = CTimer::GetTimeStep();

	int8 n = 1;	// The number of steps we divide the time step into
	float step = 0.0f;	// divided time step
	float distSq = m_vecMoveSpeed.MagnitudeSqr() * sq(CTimer::GetTimeStep());

	if(IsPed() && (distSq >= sq(0.3f) || ped->IsPlayer())){
		if(ped->IsPlayer()){
			if(ped->m_pCurrentPhysSurface)
				n = Max(NUMSTEPS(0.15f), 4.0f);
			else
				n = Max(NUMSTEPS(0.3f), 2.0f);
		}else
			n = NUMSTEPS(0.45f);
		step = savedTimeStep / n;
		if(!ped->IsPlayer())
			ped->m_fElasticity *= HIGHSPEED_ELASTICITY_MULT_PED;
	}else if(IsVehicle() && distSq >= sq(0.4f)){
		if(GetStatus() == STATUS_PLAYER)
			n = NUMSTEPS(0.2f);
		else
			n = distSq > 0.32f ? NUMSTEPS(0.3f) : NUMSTEPS(0.4f);
		step = savedTimeStep / n;

		CVector bbox = GetColModel()->boundingBox.GetSize();
		float relDistX = Abs(DotProduct(m_vecMoveSpeed, GetRight())) * CTimer::GetTimeStep() / bbox.x;
		float relDistY = Abs(DotProduct(m_vecMoveSpeed, GetForward())) * CTimer::GetTimeStep() / bbox.y;
		float relDistZ = Abs(DotProduct(m_vecMoveSpeed, GetUp())) * CTimer::GetTimeStep() / bbox.z;
		if(Max(relDistX, Max(relDistY, relDistZ)) < 1.0f){
			// check if we can get away with simplified processing

			ApplyMoveSpeed();
			ApplyTurnSpeed();
			GetMatrix().Reorthogonalise();
			bSkipLineCol = false;
			m_bIsVehicleBeingShifted = false;

			bJustCheckCollision = true;
			bUsesCollision = false;
			if(!CheckCollision()){
				bJustCheckCollision = false;
				bUsesCollision = true;
				if(IsVehicle())
					((CVehicle*)this)->bVehicleColProcessed = true;

				bHitByTrain = false;
				m_fDistanceTravelled = (GetPosition() - savedMatrix.GetPosition()).Magnitude();
				bSkipLineCol = false;

				bIsStuck = false;
				bIsInSafePosition = true;
				m_fElasticity = savedElasticity;
				RemoveAndAdd();
				return;
			}
			bJustCheckCollision = false;
			bUsesCollision = true;
			GetMatrix() = savedMatrix;
			m_vecMoveSpeed = savedMoveSpeed;
			if(IsVehicle() && ((CVehicle*)this)->bIsLawEnforcer)
				m_fElasticity *= HIGHSPEED_ELASTICITY_MULT_COPCAR;
		}
	}else if(IsObject() && ((CObject*)this)->ObjectCreatedBy != TEMP_OBJECT){
		int responsecase = ((CObject*)this)->m_nSpecialCollisionResponseCases;
		if(responsecase == COLLRESPONSE_LAMPOST){
			CVector speedUp = { 0.0f, 0.0f, 0.0f };
			CVector speedDown = { 0.0f, 0.0f, 0.0f };
			CColModel *colModel = GetColModel();
			speedUp.z = colModel->boundingBox.max.z;
			speedDown.z = colModel->boundingBox.min.z;
			speedUp = Multiply3x3(GetMatrix(), speedUp);
			speedDown = Multiply3x3(GetMatrix(), speedDown);
			speedUp = GetSpeed(speedUp);
			speedDown = GetSpeed(speedDown);
			distSq = Max(speedUp.MagnitudeSqr(), speedDown.MagnitudeSqr()) * sq(CTimer::GetTimeStep());
			if(distSq >= sq(0.3f)){
				n = NUMSTEPS(0.3f);
				step = savedTimeStep / n;
			}
		}else if(responsecase == COLLRESPONSE_UNKNOWN5){
			if(distSq >= 0.009f){
				n = NUMSTEPS(0.09f);
				step = savedTimeStep / n;
			}
		}else if(responsecase == COLLRESPONSE_SMALLBOX || responsecase == COLLRESPONSE_FENCEPART){
			if(distSq >= sq(0.15f)){
				n = NUMSTEPS(0.15f);
				step = savedTimeStep / n;
			}
		}else{
			if(distSq >= sq(0.3f)){
				n = NUMSTEPS(0.3f);
				step = savedTimeStep / n;
			}
		}
	}

	for(i = 1; i < n; i++){
		CTimer::SetTimeStep(i * step);
		ApplyMoveSpeed();
		ApplyTurnSpeed();
		// TODO: get rid of copy paste?
		if(CheckCollision()){
			if(IsPed() && m_vecMoveSpeed.z == 0.0f &&
			   !ped->bWasStanding &&
			   ped->bIsStanding)
				savedMatrix.GetPosition().z = GetPosition().z;
			GetMatrix() = savedMatrix;
			CTimer::SetTimeStep(savedTimeStep);
			m_fElasticity = savedElasticity;
			return;
		}
		if(IsPed() && m_vecMoveSpeed.z == 0.0f &&
		   !ped->bWasStanding &&
		   ped->bIsStanding)
			savedMatrix.GetPosition().z = GetPosition().z;
		GetMatrix() = savedMatrix;
		CTimer::SetTimeStep(savedTimeStep);
		if(IsVehicle()){
			CVehicle *veh = (CVehicle*)this;
			if(veh->m_vehType == VEHICLE_TYPE_CAR){
				CAutomobile *car = (CAutomobile*)this;
				car->m_aSuspensionSpringRatio[0] = 1.0f;
				car->m_aSuspensionSpringRatio[1] = 1.0f;
				car->m_aSuspensionSpringRatio[2] = 1.0f;
				car->m_aSuspensionSpringRatio[3] = 1.0f;
			}else if(veh->m_vehType == VEHICLE_TYPE_BIKE){
				CBike *bike = (CBike*)this;
				bike->m_aSuspensionSpringRatio[0] = 1.0f;
				bike->m_aSuspensionSpringRatio[1] = 1.0f;
				bike->m_aSuspensionSpringRatio[2] = 1.0f;
				bike->m_aSuspensionSpringRatio[3] = 1.0f;
			}
		}
	}
	
	ApplyMoveSpeed();
	ApplyTurnSpeed();
	GetMatrix().Reorthogonalise();
	m_bIsVehicleBeingShifted = false;
	bSkipLineCol = false;
	if(!m_vecMoveSpeed.IsZero() ||
	   !m_vecTurnSpeed.IsZero() ||
	   bHitByTrain ||
	   GetStatus() == STATUS_PLAYER ||
	   IsVehicle() && ((CVehicle*)this)->bRestingOnPhysical ||
	   IsPed() && ped->IsPlayer()){
		if(IsVehicle())
			((CVehicle*)this)->bVehicleColProcessed = true;
		if(CheckCollision()){
			GetMatrix() = savedMatrix;
			m_fElasticity = savedElasticity;
			return;
		}
	}
	bHitByTrain = false;
	m_fDistanceTravelled = (GetPosition() - savedMatrix.GetPosition()).Magnitude();
	bSkipLineCol = false;

	bIsStuck = false;
	bIsInSafePosition = true;
	m_fElasticity = savedElasticity;
	RemoveAndAdd();
}
