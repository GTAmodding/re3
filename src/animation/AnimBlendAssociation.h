#pragma once

#include "AnimBlendList.h"
#include "AnimBlendNode.h"
#include "AnimBlendHierarchy.h"

enum {
	ASSOC_RUNNING = 1,
	ASSOC_REPEAT = 2,
	ASSOC_DELETEFADEDOUT = 4,
	ASSOC_FADEOUTWHENDONE = 8,
	ASSOC_PARTIAL = 0x10,
	ASSOC_MOVEMENT = 0x20,	// ???
	ASSOC_HAS_TRANSLATION = 0x40,
	ASSOC_HAS_X_TRANSLATION = 0x80,	// for 2d velocity extraction
	ASSOC_WALK = 0x100,	// for CPed::PlayFootSteps(void)
	ASSOC_IDLE = 0x200,	// only xpress scratch has it by default, but game adds it to player's idle animations later
	ASSOC_NOWALK = 0x400,	// see CPed::PlayFootSteps(void)
	ASSOC_BLOCK = 0x800,	// unused in assoc description, blocks other anims from being played
	ASSOC_FRONTAL = 0x1000, // anims that we fall to front
	ASSOC_DRIVING = 0x2000,	// new in VC
};

// Anim hierarchy associated with a clump
// Holds the interpolated state of all nodes.
// Also used as template for other clumps.
class CAnimBlendAssociation
{
public:
	enum {
		// callbackType
		CB_NONE,
		CB_FINISH,
		CB_DELETE
	};

	CAnimBlendLink link;

	int16 numNodes;			// taken from CAnimBlendClumpData::numFrames
	int16 groupId;		// ID of CAnimBlendAssocGroup this is in
	// NB: Order of these depends on order of nodes in Clump this was built from
	CAnimBlendNode *nodes;
	CAnimBlendHierarchy *hierarchy;
	float blendAmount;
	float blendDelta;	// how much blendAmount changes over time
	float currentTime;
	float speed;
	float timeStep;
	int16 animId;
	int16 flags;
	int32 callbackType;
	void (*callback)(CAnimBlendAssociation*, void*);
	void *callbackArg;

	bool IsRunning(void) { return !!(flags & ASSOC_RUNNING); }
	bool IsRepeating(void) { return !!(flags & ASSOC_REPEAT); }
	bool IsPartial(void) { return !!(flags & ASSOC_PARTIAL); }
	bool IsMovement(void) { return !!(flags & ASSOC_MOVEMENT); }
	bool HasTranslation(void) { return !!(flags & ASSOC_HAS_TRANSLATION); }
	bool HasXTranslation(void) { return !!(flags & ASSOC_HAS_X_TRANSLATION); }

	float GetBlendAmount(float weight) { return IsPartial() ? blendAmount : blendAmount*weight; }
	CAnimBlendNode *GetNode(int i) { return &nodes[i]; }

	CAnimBlendAssociation(void);
	CAnimBlendAssociation(CAnimBlendAssociation &other);
#ifndef FIX_BUGS
	virtual
#endif
	~CAnimBlendAssociation(void);
	void AllocateAnimBlendNodeArray(int n);
	void FreeAnimBlendNodeArray(void);
	void Init(RpClump *clump, CAnimBlendHierarchy *hier);
	void Init(CAnimBlendAssociation &assoc);
	void SetBlend(float amount, float delta);
	void SetFinishCallback(void (*callback)(CAnimBlendAssociation*, void*), void *arg);
	void SetDeleteCallback(void (*callback)(CAnimBlendAssociation*, void*), void *arg);
	void SetCurrentTime(float time);
	void SyncAnimation(CAnimBlendAssociation *other);
	void Start(float time);
	void UpdateTimeStep(float timeDelta, float relSpeed);
	bool UpdateTime(float timeDelta, float relSpeed);
	bool UpdateBlend(float timeDelta);

	void SetRun(void) { flags |= ASSOC_RUNNING; }

	float GetTimeLeft() { return hierarchy->totalLength - currentTime; }
	float GetProgress() { return currentTime / hierarchy->totalLength; }

	static CAnimBlendAssociation *FromLink(CAnimBlendLink *l) {
		return (CAnimBlendAssociation*)((uint8*)l - offsetof(CAnimBlendAssociation, link));
	}
};
