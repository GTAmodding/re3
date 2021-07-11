#include "common.h"

#include "AnimBlendClumpData.h"
#include "MemoryMgr.h"

CAnimBlendClumpData::CAnimBlendClumpData(void)
{
	numFrames = 0;
	velocity2d = nil;
	frames = nil;
	link.Init();
}

CAnimBlendClumpData::~CAnimBlendClumpData(void)
{
	link.Remove();
	if(frames)
		RwFreeAlign(frames);
}

void
CAnimBlendClumpData::SetNumberOfFrames(int n)
{
	if(frames)
		RwFreeAlign(frames);
	numFrames = n;
	frames = (AnimBlendFrameData*)RwMallocAlign(numFrames * sizeof(AnimBlendFrameData), 64);
}

void
CAnimBlendClumpData::ForAllFrames(void (*cb)(AnimBlendFrameData*, void*), void *arg)
{
	int i;
	for(i = 0; i < numFrames; i++)
		cb(&frames[i], arg);
}
