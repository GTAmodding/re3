#pragma once

#include "AnimBlendList.h"

// TODO: put somewhere else
struct AnimBlendFrameData
{
	enum {
		IGNORE_ROTATION = 2,
		IGNORE_TRANSLATION = 4,
		VELOCITY_EXTRACTION = 8,
		VELOCITY_EXTRACTION_3D = 0x10,
		UPDATE_KEYFRAMES = 0x20,
		COMPRESSED = 0x40
	};

	uint8 flag;
	RwV3d resetPos;
	union {
		RwFrame *frame;
		RpHAnimStdInterpFrame *hanimFrame;
	};
	int32 nodeID;
};
VALIDATE_SIZE(AnimBlendFrameData, 0x18);


class CAnimBlendClumpData
{
public:
	CAnimBlendLink link;
	int32 numFrames;
	CVector *velocity;
	// order of frames is determined by RW hierarchy
	AnimBlendFrameData *frames;

	CAnimBlendClumpData(void);
	~CAnimBlendClumpData(void);
	void SetNumberOfFrames(int n);
	void SetNumberOfBones(int n) { SetNumberOfFrames(n); }
	void ForAllFrames(void (*cb)(AnimBlendFrameData*, void*), void *arg);
};
