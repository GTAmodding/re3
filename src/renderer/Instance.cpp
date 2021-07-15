#include "common.h"

#include "Instance.h"

void
CInstance::Shutdown()
{
	GetMatrix().Detach();
}
