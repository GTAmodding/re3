#include "common.h"

#include "General.h"
#include "Camera.h"
#include "Renderer.h"
#include "ModelInfo.h"
#include "custompipes.h"

void
CSimpleModelInfo::DeleteRwObject(void)
{
	int i;
	RwFrame *f;
	for(i = 0; i < m_numAtomics; i++)
		if(m_atomics[i]){
			f = RpAtomicGetFrame(m_atomics[i]);
			RpAtomicDestroy(m_atomics[i]);
			RwFrameDestroy(f);
			m_atomics[i] = nil;
			RemoveTexDictionaryRef();
		}
}

RwObject*
CSimpleModelInfo::CreateInstance(void)
{
	RpAtomic *atomic;
	if(m_atomics[0] == nil)
		return nil;
	atomic = RpAtomicClone(m_atomics[0]);
	RpAtomicSetFrame(atomic, RwFrameCreate());
	return (RwObject*)atomic;
}

RwObject*
CSimpleModelInfo::CreateInstance(RwMatrix *matrix)
{
	RpAtomic *atomic;
	RwFrame *frame;

	if(m_atomics[0] == nil)
		return nil;
	atomic = RpAtomicClone(m_atomics[0]);
	frame = RwFrameCreate();
	*RwFrameGetMatrix(frame) = *matrix;
	RpAtomicSetFrame(atomic, frame);
	return (RwObject*)atomic;
}

void
CSimpleModelInfo::Init(void)
{
	m_atomics[0] = nil;
	m_atomics[1] = nil;
	m_atomics[2] = nil;
	m_numAtomics = 0;
	m_firstDamaged  = 0;
	m_normalCull    = 0;
	m_isDamaged     = 0;
	m_isBigBuilding = 0;
	m_noFade        = 0;
	m_drawLast      = 0;
	m_additive      = 0;
	m_isSubway      = 0;
	m_ignoreLight   = 0;
	m_noZwrite      = 0;
}

void
CSimpleModelInfo::SetAtomic(int n, RpAtomic *atomic)
{
	AddTexDictionaryRef();
	m_atomics[n] = atomic;
	if(m_ignoreLight){
		RpGeometry *geo = RpAtomicGetGeometry(atomic);
		RpGeometrySetFlags(geo, RpGeometryGetFlags(geo) & ~rpGEOMETRYLIGHT);
	}

#ifdef EXTENDED_PIPELINES
	CustomPipes::AttachWorldPipe(atomic);
#endif
}

void
CSimpleModelInfo::SetLodDistances(float *dist)
{
	m_lodDistances[0] = dist[0];
	m_lodDistances[1] = dist[1];
	m_lodDistances[2] = dist[2];
}

void
CSimpleModelInfo::IncreaseAlpha(void)
{
	if(m_alpha >= 0xEF)
		m_alpha = 0xFF;
	else
		m_alpha += 0x10;
}

float
CSimpleModelInfo::GetLodDistance(int i)
{
	return m_lodDistances[i] * TheCamera.LODDistMultiplier;
}

float
CSimpleModelInfo::GetNearDistance(void)
{
	return m_lodDistances[2] * TheCamera.LODDistMultiplier;
}

float
CSimpleModelInfo::GetLargestLodDistance(void)
{
	float d;
	if(m_firstDamaged == 0 || m_isDamaged)
		d = m_lodDistances[m_numAtomics-1];
	else
		d = m_lodDistances[m_firstDamaged-1];
	return d * TheCamera.LODDistMultiplier;
}

RpAtomic*
CSimpleModelInfo::GetAtomicFromDistance(float dist)
{
	int i;
	i = 0;
	if(m_isDamaged)
		i = m_firstDamaged;
	for(; i < m_numAtomics; i++)
		if(dist < m_lodDistances[i] * TheCamera.LODDistMultiplier)
			return m_atomics[i];
	return nil;
}

void
CSimpleModelInfo::FindRelatedModel(void)
{
	int i;
	CBaseModelInfo *mi;
	for(i = 0; i < MODELINFOSIZE; i++){
		mi = CModelInfo::GetModelInfo(i);
		if(mi && mi != this &&
		   !CGeneral::faststrcmp(GetModelName()+3, mi->GetModelName()+3)){
			assert(mi->IsSimple());
			this->SetRelatedModel((CSimpleModelInfo*)mi);
			return;
		}
	}
}

void
CSimpleModelInfo::SetupBigBuilding(void)
{
	CSimpleModelInfo *related;
	if(m_lodDistances[0] > LOD_DISTANCE && GetRelatedModel() == nil){
		m_isBigBuilding = 1;
		FindRelatedModel();
		related = GetRelatedModel();
		if(related)
			m_lodDistances[2] = related->GetLargestLodDistance()/TheCamera.LODDistMultiplier;
		else
#ifdef FIX_BUGS
		if(toupper(m_name[0]) == 'L' && toupper(m_name[1]) == 'O' && toupper(m_name[2]) == 'D')
			m_lodDistances[2] = 100.0f;
		else
			m_lodDistances[2] = 0.0f;
#else
			m_lodDistances[2] = 100.0f;
#endif
	}
}
