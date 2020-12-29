#include "common.h"

#include "AnimBlendSequence.h"
#include "AnimBlendHierarchy.h"
#include "AnimManager.h"

//--MIAMI: file done

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
	CAnimManager::RemoveFromUncompressedCache(this);
	RemoveAnimSequences();
	totalLength = 0.0f;
	compressed = 0;
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
#ifdef FIX_BUGS
		if(sequences[i].numFrames == 0)
			continue;
#endif

		totalLength = Max(totalLength, sequences[i].GetKeyFrame(sequences[i].numFrames-1)->deltaTime);
		for(j = sequences[i].numFrames-1; j >= 1; j--){
			KeyFrame *kf1 = sequences[i].GetKeyFrame(j);
			KeyFrame *kf2 = sequences[i].GetKeyFrame(j-1);
			kf1->deltaTime -= kf2->deltaTime;
		}
	}
}

void
CAnimBlendHierarchy::CalcTotalTimeCompressed(void)
{
	int i, j;

	totalLength = 0.0f;

	for(i = 0; i < numSequences; i++){
#ifdef FIX_BUGS
		if(sequences[i].numFrames == 0)
			continue;
#endif

		totalLength = Max(totalLength, sequences[i].GetKeyFrameCompressed(sequences[i].numFrames-1)->GetDeltaTime());
		for(j = sequences[i].numFrames-1; j >= 1; j--){
			KeyFrameCompressed *kf1 = sequences[i].GetKeyFrameCompressed(j);
			KeyFrameCompressed *kf2 = sequences[i].GetKeyFrameCompressed(j-1);
			kf1->deltaTime -= kf2->deltaTime;
		}
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
	sequences = nil;
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
	compressed = 0;
	if(totalLength == 0.0f){
		RemoveQuaternionFlips();
		CalcTotalTime();
	}
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

bool
CAnimBlendHierarchy::IsCompressed()
{
	return !!compressed;
}
