#include "common.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Floater.h"
#include "ModelIndices.h"
#include "Physical.h"
#include "Timer.h"
#include "Vehicle.h"
#include "WaterLevel.h"

//--MIAMI: done

cBuoyancy mod_Buoyancy;

float fVolMultiplier = 1.0f;
// amount of boat volume in bounding box
// 1.0-volume is the empty space in the bbox
float fBoatVolumeDistribution[9] = {
	// rear
	0.75f, 0.9f, 0.75f,
	0.95f, 1.0f, 0.95f,
	0.4f, 0.7f, 0.4f
	// bow
};
float fBoatVolumeDistributionCat[9] = {
	0.9f, 0.3f, 0.9f,
	1.0f, 0.5f, 1.0f,
	0.95f, 0.4f, 0.95f
};
float fBoatVolumeDistributionSail[9] = {
	0.55f, 0.95f, 0.55f,
	0.75f, 1.1f, 0.75f,
	0.3f, 0.8f, 0.3f
};
float fBoatVolumeDistributionDinghy[9] = {
	0.65f, 0.85f, 0.65f,
	0.85f, 1.1f, 0.85f,
	0.65f, 0.95f, 0.65f
};
float fBoatVolumeDistributionSpeed[9] = {
	0.7f, 0.9f, 0.7f,
	0.95f, 1.0f, 0.95f,
	0.6f, 0.7f, 0.6f
};

bool
cBuoyancy::ProcessBuoyancy(CPhysical *phys, float buoyancy, CVector *point, CVector *impulse)
{
	m_numSteps = 2.0f;

	if(!CWaterLevel::GetWaterLevel(phys->GetPosition(), &m_waterlevel, phys->bTouchingWater))
		return false;
	m_matrix = phys->GetMatrix();

	PreCalcSetup(phys, buoyancy);
	SimpleCalcBuoyancy();
	float f = CalcBuoyancyForce(phys, point, impulse);
	if(m_isBoat)
		return true;
	return f != 0.0f;
}

bool
cBuoyancy::ProcessBuoyancyBoat(CVehicle *veh, float buoyancy, CVector *point, CVector *impulse, bool bNoTurnForce)
{
	m_numSteps = 2.0f;

	if(!CWaterLevel::GetWaterLevel(veh->GetPosition(), &m_waterlevel, veh->bTouchingWater))
		return false;
	m_matrix = veh->GetMatrix();
	PreCalcSetup(veh, buoyancy);


	float x, y;
	int ix, i;
	tWaterLevel waterPosition;
	CVector waterNormal;

	// Floater is divided into 3x3 parts. Process and sum each of them
	float volDiv = 1.0f/((m_dimMax.z - m_dimMin.z)*sq(m_numSteps+1.0f));
	ix = 0;
	for(x = m_dimMin.x; x <= m_dimMax.x; x += m_step.x){
		i = ix;
		for(y = m_dimMin.y; y <= m_dimMax.y; y += m_step.y){
			CVector waterLevel(x, y, 0.0f);
			FindWaterLevelNorm(m_positionZ, &waterLevel, &waterPosition, &waterNormal);
			switch(veh->GetModelIndex()){
			case MI_RIO:
				fVolMultiplier = fBoatVolumeDistributionCat[i];
				break;
			case MI_SQUALO:
			case MI_SPEEDER:
			case MI_JETMAX:
				fVolMultiplier = fBoatVolumeDistributionSpeed[i];
				break;
			case MI_COASTG:
			case MI_DINGHY:
				fVolMultiplier = fBoatVolumeDistributionDinghy[i];
				break;
			case MI_MARQUIS:
				fVolMultiplier = fBoatVolumeDistributionSail[i];
				break;
			case MI_PREDATOR:
			case MI_SKIMMER:
			case MI_REEFER:
			case MI_TROPIC:
			default:
				fVolMultiplier = fBoatVolumeDistribution[i];
				break;
			}
			if(waterPosition != FLOATER_ABOVE_WATER){
				float volume = SimpleSumBuoyancyData(waterLevel, waterPosition);
				float upImpulse = volume * volDiv * buoyancy * CTimer::GetTimeStep();
				CVector speed = veh->GetSpeed(Multiply3x3(veh->GetMatrix(), CVector(x, y, 0.0f)));
				float damp = 1.0f - DotProduct(speed, waterNormal)*veh->pHandling->fSuspensionDampingLevel;
				float finalImpulse = upImpulse*Max(damp, 0.0f);
				impulse->z += finalImpulse;
				if(!bNoTurnForce)
					veh->ApplyTurnForce(finalImpulse*waterNormal, Multiply3x3(m_matrix, waterLevel));
			}
			i += 3;
		}
		ix++;
	}

	m_volumeUnderWater *= volDiv;

	*point = Multiply3x3(m_matrix, m_impulsePoint);
	return m_isBoat || m_haveVolume;

}

void
cBuoyancy::PreCalcSetup(CPhysical *phys, float buoyancy)
{
	CColModel *colModel;

	m_isBoat = phys->IsVehicle() && ((CVehicle*)phys)->IsBoat();
	colModel = phys->GetColModel();
	m_dimMin = colModel->boundingBox.min;
	m_dimMax = colModel->boundingBox.max;

	if(m_isBoat){
		switch(phys->GetModelIndex()){
		case MI_PREDATOR:
		default:
			m_dimMax.y *= 1.05f;
			m_dimMin.y *= 0.9f;
			break;
		case MI_SPEEDER:
			m_dimMax.y *= 1.25f;
			m_dimMin.y *= 0.83f;
			break;
		case MI_REEFER:
			m_dimMin.y *= 0.9f;
			break;
		case MI_RIO:
			m_dimMax.y *= 0.9f;
			m_dimMin.y *= 0.9f;
			m_dimMax.z += 0.25f;
			m_dimMin.z -= 0.2f;
			break;
		case MI_SQUALO:
			m_dimMax.y *= 0.9f;
			m_dimMin.y *= 0.9f;
			break;
		case MI_TROPIC:
			m_dimMax.y *= 1.3f;
			m_dimMin.y *= 0.82f;
			m_dimMin.z -= 0.2f;
			break;
		case MI_SKIMMER:
			m_dimMin.y = -m_dimMax.y;
			m_dimMax.y *= 1.2f;
			break;
		case MI_COASTG:
			m_dimMax.y *= 1.1f;
			m_dimMin.y *= 0.9f;
			m_dimMin.z -= 0.3f;
			break;
		case MI_DINGHY:
			m_dimMax.y *= 1.3f;
			m_dimMin.y *= 0.9f;
			m_dimMin.z -= 0.2f;
			break;
		case MI_MARQUIS:
			m_dimMax.y *= 1.3f;
			m_dimMin.y *= 0.9f;
			break;
		case MI_JETMAX:
			m_dimMin.y *= 0.9f;
			break;
		}
	}

	m_step = (m_dimMax - m_dimMin)/m_numSteps;

	if(m_step.z > m_step.x && m_step.z > m_step.y){
		m_stepRatio.x = m_step.x/m_step.z;
		m_stepRatio.y = m_step.y/m_step.z;
		m_stepRatio.z = 1.0f;
	}else if(m_step.y > m_step.x && m_step.y > m_step.z){
		m_stepRatio.x = m_step.x/m_step.y;
		m_stepRatio.y = 1.0f;
		m_stepRatio.z = m_step.z/m_step.y;
	}else{
		m_stepRatio.x = 1.0f;
		m_stepRatio.y = m_step.y/m_step.x;
		m_stepRatio.z = m_step.z/m_step.x;
	}

	m_haveVolume = false;
	m_numPartialVolumes = 1.0f;
	m_volumeUnderWater = 0.0f;
	m_impulsePoint = CVector(0.0f, 0.0f, 0.0f);
	m_position = phys->GetPosition();
	m_positionZ = CVector(0.0f, 0.0f, m_position.z);
	m_buoyancy = buoyancy;
	m_waterlevel += m_waterLevelInc;
}

void
cBuoyancy::SimpleCalcBuoyancy(void)
{
	float x, y;
	tWaterLevel waterPosition;

	// Floater is divided into 3x3 parts. Process and sum each of them
	for(x = m_dimMin.x; x <= m_dimMax.x; x += m_step.x){
		for(y = m_dimMin.y; y <= m_dimMax.y; y += m_step.y){
			CVector waterLevel(x, y, 0.0f);
			FindWaterLevel(m_positionZ, &waterLevel, &waterPosition);
			fVolMultiplier = 1.0f;
			if(waterPosition != FLOATER_ABOVE_WATER)
				SimpleSumBuoyancyData(waterLevel, waterPosition);
		}
	}

	m_volumeUnderWater /= (m_dimMax.z - m_dimMin.z)*sq(m_numSteps+1.0f);
}

float
cBuoyancy::SimpleSumBuoyancyData(CVector &waterLevel, tWaterLevel waterPosition)
{
	static float fThisVolume;
	static CVector AverageOfWaterLevel;
	static float fFraction;
	static float fRemainingSlice;

	float submerged = Abs(waterLevel.z - m_dimMin.z);
	// subtract empty space from submerged volume
	fThisVolume = submerged - (1.0f - fVolMultiplier);
	if(fThisVolume < 0.0f)
		return 0.0f;

	if(m_isBoat){
		fThisVolume *= fVolMultiplier;
		fThisVolume = sq(fThisVolume);
	}

	m_volumeUnderWater += fThisVolume;

	AverageOfWaterLevel.x = waterLevel.x * m_stepRatio.x;
	AverageOfWaterLevel.y = waterLevel.y * m_stepRatio.y;
	AverageOfWaterLevel.z = (waterLevel.z+m_dimMin.z)/2.0f * m_stepRatio.z;

	if(m_flipAverage)
		AverageOfWaterLevel = -AverageOfWaterLevel;

	fFraction = 1.0f/m_numPartialVolumes;
	fRemainingSlice = 1.0f - fFraction;
	m_impulsePoint = m_impulsePoint*fRemainingSlice + AverageOfWaterLevel*fThisVolume*fFraction;
	m_numPartialVolumes += 1.0f;
	m_haveVolume = true;
	return fThisVolume;
}

void
cBuoyancy::FindWaterLevel(const CVector &zpos, CVector *waterLevel, tWaterLevel *waterPosition)
{
	*waterPosition = FLOATER_IN_WATER;
	// waterLevel is a local x,y point
	// m_position is the global position of our floater
	// zpos is the global z coordinate of our floater
	CVector xWaterLevel = Multiply3x3(m_matrix, *waterLevel);
	CWaterLevel::GetWaterLevel(xWaterLevel.x + m_position.x, xWaterLevel.y + m_position.y, m_position.z,
		&waterLevel->z, true);
	waterLevel->z -= xWaterLevel.z + zpos.z;	// make local
	if(waterLevel->z > m_dimMax.z){
		waterLevel->z = m_dimMax.z;
		*waterPosition = FLOATER_UNDER_WATER;
	}else if(waterLevel->z < m_dimMin.z){
		waterLevel->z = m_dimMin.z;
		*waterPosition = FLOATER_ABOVE_WATER;
	}
}

// Same as above but also get normal
void
cBuoyancy::FindWaterLevelNorm(const CVector &zpos, CVector *waterLevel, tWaterLevel *waterPosition, CVector *normal)
{
	*waterPosition = FLOATER_IN_WATER;
	CVector xWaterLevel = Multiply3x3(m_matrix, *waterLevel);
	CWaterLevel::GetWaterLevel(xWaterLevel.x + m_position.x, xWaterLevel.y + m_position.y, m_position.z,
		&waterLevel->z, true);
	waterLevel->z -= xWaterLevel.z + zpos.z;	// make local
	if(waterLevel->z >= m_dimMin.z)
		*normal = CWaterLevel::GetWaterNormal(xWaterLevel.x + m_position.x, xWaterLevel.y + m_position.y);
	if(waterLevel->z > m_dimMax.z){
		waterLevel->z = m_dimMax.z;
		*waterPosition = FLOATER_UNDER_WATER;
	}else if(waterLevel->z < m_dimMin.z){
		waterLevel->z = m_dimMin.z;
		*waterPosition = FLOATER_ABOVE_WATER;
	}
}

bool
cBuoyancy::CalcBuoyancyForce(CPhysical *phys, CVector *point, CVector *impulse)
{
	if(!m_haveVolume)
		return false;

	*point = Multiply3x3(m_matrix, m_impulsePoint);
	*impulse = CVector(0.0f, 0.0f, m_volumeUnderWater*m_buoyancy*CTimer::GetTimeStep());
	return true;
}
