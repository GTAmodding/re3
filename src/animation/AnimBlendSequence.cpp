#include "common.h"

#include "AnimBlendSequence.h"
#include "MemoryHeap.h"

CAnimBlendSequence::CAnimBlendSequence(void)
{
	type = 0;
	numFrames = 0;
	keyFrames = nil;
	keyFramesCompressed = nil;
#ifdef PED_SKIN
	boneTag = -1;
#endif
}

CAnimBlendSequence::~CAnimBlendSequence(void)
{
	if(keyFrames)
		RwFree(keyFrames);
	if(keyFramesCompressed)
		RwFree(keyFramesCompressed);
}

void
CAnimBlendSequence::SetName(char *name)
{
	strncpy(this->name, name, 24);
}

void
CAnimBlendSequence::SetNumFrames(int numFrames, bool translation)
{
	int sz;

	if(translation){
		sz = sizeof(KeyFrameTrans);
		type |= KF_ROT | KF_TRANS;
	}else{
		sz = sizeof(KeyFrame);
		type |= KF_ROT;
	}
	keyFrames = RwMalloc(sz * numFrames);
	this->numFrames = numFrames;
}

void
CAnimBlendSequence::RemoveQuaternionFlips(void)
{
	int i;
	CQuaternion last;
	KeyFrame *frame;

	if(numFrames < 2)
		return;

	frame = GetKeyFrame(0);
	last = frame->rotation;
	for(i = 1; i < numFrames; i++){
		frame = GetKeyFrame(i);
		if(DotProduct(last, frame->rotation) < 0.0f)
			frame->rotation = -frame->rotation;
		last = frame->rotation;
	}
}

void
CAnimBlendSequence::Uncompress(void)
{
	int i;

	if(numFrames == 0)
		return;

	PUSH_MEMID(MEMID_ANIMATION);

	float rotScale = 1.0f/4096.0f;
	float timeScale = 1.0f/60.0f;
	float transScale = 1.0f/128.0f;
	if(type & KF_TRANS){
		void *newKfs = RwMalloc(numFrames * sizeof(KeyFrameTrans));
		KeyFrameTransCompressed *ckf = (KeyFrameTransCompressed*)keyFramesCompressed;
		KeyFrameTrans *kf = (KeyFrameTrans*)newKfs;
		for(i = 0; i < numFrames; i++){
			kf->rotation.x = ckf->rot[0]*rotScale;
			kf->rotation.y = ckf->rot[1]*rotScale;
			kf->rotation.z = ckf->rot[2]*rotScale;
			kf->rotation.w = ckf->rot[3]*rotScale;
			kf->deltaTime = ckf->deltaTime*timeScale;
			kf->translation.x = ckf->trans[0]*transScale;
			kf->translation.y = ckf->trans[1]*transScale;
			kf->translation.z = ckf->trans[2]*transScale;
			kf++;
			ckf++;
		}
		keyFrames = newKfs;
	}else{
		void *newKfs = RwMalloc(numFrames * sizeof(KeyFrame));
		KeyFrameCompressed *ckf = (KeyFrameCompressed*)keyFramesCompressed;
		KeyFrame *kf = (KeyFrame*)newKfs;
		for(i = 0; i < numFrames; i++){
			kf->rotation.x = ckf->rot[0]*rotScale;
			kf->rotation.y = ckf->rot[1]*rotScale;
			kf->rotation.z = ckf->rot[2]*rotScale;
			kf->rotation.w = ckf->rot[3]*rotScale;
			kf->deltaTime = ckf->deltaTime*timeScale;
			kf++;
			ckf++;
		}
		keyFrames = newKfs;
	}
	REGISTER_MEMPTR(&keyFrames);

	RwFree(keyFramesCompressed);
	keyFramesCompressed = nil;

	POP_MEMID();
}

void
CAnimBlendSequence::CompressKeyframes(void)
{
	int i;

	if(numFrames == 0)
		return;

	PUSH_MEMID(MEMID_ANIMATION);

	float rotScale = 4096.0f;
	float timeScale = 60.0f;
	float transScale = 128.0f;
	if(type & KF_TRANS){
		void *newKfs = RwMalloc(numFrames * sizeof(KeyFrameTransCompressed));
		KeyFrameTransCompressed *ckf = (KeyFrameTransCompressed*)newKfs;
		KeyFrameTrans *kf = (KeyFrameTrans*)keyFrames;
		for(i = 0; i < numFrames; i++){
			ckf->rot[0] = kf->rotation.x*rotScale;
			ckf->rot[1] = kf->rotation.y*rotScale;
			ckf->rot[2] = kf->rotation.z*rotScale;
			ckf->rot[3] = kf->rotation.w*rotScale;
			ckf->deltaTime = kf->deltaTime*timeScale + 0.5f;
			ckf->trans[0] = kf->translation.x*transScale;
			ckf->trans[1] = kf->translation.y*transScale;
			ckf->trans[2] = kf->translation.z*transScale;
			kf++;
			ckf++;
		}
		keyFramesCompressed = newKfs;
	}else{
		void *newKfs = RwMalloc(numFrames * sizeof(KeyFrameCompressed));
		KeyFrameCompressed *ckf = (KeyFrameCompressed*)newKfs;
		KeyFrame *kf = (KeyFrame*)keyFrames;
		for(i = 0; i < numFrames; i++){
			ckf->rot[0] = kf->rotation.x*rotScale;
			ckf->rot[1] = kf->rotation.y*rotScale;
			ckf->rot[2] = kf->rotation.z*rotScale;
			ckf->rot[3] = kf->rotation.w*rotScale;
			ckf->deltaTime = kf->deltaTime*timeScale + 0.5f;
			kf++;
			ckf++;
		}
		keyFramesCompressed = newKfs;
	}
	REGISTER_MEMPTR(&keyFramesCompressed);

	POP_MEMID();
}

void
CAnimBlendSequence::RemoveUncompressedData(void)
{
	if(numFrames == 0)
		return;
	CompressKeyframes();
	RwFree(keyFrames);
	keyFrames = nil;
}

#ifdef USE_CUSTOM_ALLOCATOR
bool
CAnimBlendSequence::MoveMemory(void)
{
	if(keyFrames){
		void *newaddr = gMainHeap.MoveMemory(keyFrames);
		if(newaddr != keyFrames){
			keyFrames = newaddr;
			return true;
		}
	}else if(keyFramesCompressed){
		void *newaddr = gMainHeap.MoveMemory(keyFramesCompressed);
		if(newaddr != keyFramesCompressed){
			keyFramesCompressed = newaddr;
			return true;
		}
	}
	return false;
}
#endif

