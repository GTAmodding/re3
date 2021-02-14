#include "common.h"

#include "rpworld.h"
#include "Treadable.h"
#include "Pools.h"

void *CTreadable::operator new(size_t sz) { return CPools::GetTreadablePool()->New();  }
void CTreadable::operator delete(void *p, size_t sz) { CPools::GetTreadablePool()->Delete((CTreadable*)p); }
