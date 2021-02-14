#include "common.h"

#include "AnimBlendSequence.h"
#include "AnimBlendHierarchy.h"

CAnimBlendHierarchy::CAnimBlendHierarchy(void)
{
	sequences = nil;
	numSequences = 0;
	compressed = 0;
	totalLength = 0.0f;
	linkPtr = nil;
}

void
CAnimBlendHierarchy::Shutdown(void)
{
	RemoveAnimSequences();
	compressed = 0;
	linkPtr = nil;
}

void
CAnimBlendHierarchy::SetName(char *name)
{
	strncpy(this->name, name, 24);
}

void
CAnimBlendHierarchy::CalcTotalTime(void)
{
	int i, j;
	totalLength = 0.0f;

	for(i = 0; i < numSequences; i++){
		float seqTime = 0.0f;
		for(j = 0; j < sequences[i].numFrames; j++)
			seqTime += sequences[i].GetKeyFrame(j)->deltaTime;
		totalLength = Max(totalLength, seqTime);
	}
}

void
CAnimBlendHierarchy::RemoveQuaternionFlips(void)
{
	int i;

	for(i = 0; i < numSequences; i++)
		sequences[i].RemoveQuaternionFlips();
}

void
CAnimBlendHierarchy::RemoveAnimSequences(void)
{
	delete[] sequences;
	numSequences = 0;
}

void
CAnimBlendHierarchy::Uncompress(void)
{
#ifdef ANIM_COMPRESSION
	int i;
	assert(compressed);
	for(i = 0; i < numSequences; i++)
		sequences[i].Uncompress();
#endif
	if(totalLength == 0.0f)
		CalcTotalTime();
	compressed = 0;
}

void
CAnimBlendHierarchy::RemoveUncompressedData(void)
{
#ifdef ANIM_COMPRESSION
	int i;
	assert(!compressed);
	for(i = 0; i < numSequences; i++)
		sequences[i].RemoveUncompressedData();
#endif
	compressed = 1;
}

#ifdef USE_CUSTOM_ALLOCATOR
void
CAnimBlendHierarchy::MoveMemory(bool onlyone)
{
	int i;
	for(i = 0; i < numSequences; i++)
		if(sequences[i].MoveMemory() && onlyone)
			return;
}
#endif
