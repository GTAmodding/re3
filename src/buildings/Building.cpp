#include "common.h"

#include "Building.h"
#include "Streaming.h"
#include "Pools.h"

void *CBuilding::operator new(size_t sz) { return CPools::GetBuildingPool()->New();  }
void CBuilding::operator delete(void *p, size_t sz) { CPools::GetBuildingPool()->Delete((CBuilding*)p); }

void
CBuilding::ReplaceWithNewModel(int32 id)
{
	DeleteRwObject();

	if (CModelInfo::GetModelInfo(m_modelIndex)->GetNumRefs() == 0)
		CStreaming::RemoveModel(m_modelIndex);
	m_modelIndex = id;

	if(bIsBIGBuilding)
		if(m_level == LEVEL_GENERIC || m_level == CGame::currLevel)
			CStreaming::RequestModel(id, STREAMFLAGS_DONT_REMOVE);
}
