#include "common.h"
#include "patcher.h"
#include "General.h"
#include "NodeName.h"
#include "VisibilityPlugins.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendHierarchy.h"
#include "AnimBlendAssociation.h"
#include "RpAnimBlend.h"

RwInt32 &ClumpOffset = *(RwInt32*)0x8F1B84;

enum
{
	ID_RPANIMBLEND = MAKECHUNKID(rwVENDORID_ROCKSTAR, 0xFD),
};

void*
AnimBlendClumpCreate(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	*RWPLUGINOFFSET(CAnimBlendClumpData*, object, offsetInObject) = nil;
	return object;
}

void*
AnimBlendClumpDestroy(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	CAnimBlendClumpData *data;
	data = *RPANIMBLENDCLUMPDATA(object);
	if(data){
		RpAnimBlendClumpRemoveAllAssociations((RpClump*)object);
		delete data;
		*RPANIMBLENDCLUMPDATA(object) = nil;
	}
	return object;
}

void *AnimBlendClumpCopy(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject) { return nil; }

bool
RpAnimBlendPluginAttach(void)
{
	ClumpOffset = RpClumpRegisterPlugin(sizeof(CAnimBlendClumpData*), ID_RPANIMBLEND,
		AnimBlendClumpCreate, AnimBlendClumpDestroy, AnimBlendClumpCopy);
	return ClumpOffset >= 0;
}

CAnimBlendAssociation*
RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc)
{
	if(assoc->link.next)
		return CAnimBlendAssociation::FromLink(assoc->link.next);
	return nil;
}

CAnimBlendAssociation*
RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc, uint32 mask)
{
	CAnimBlendLink *link;
	for(link = assoc->link.next; link; link = link->next){
		assoc = CAnimBlendAssociation::FromLink(link);
		if(assoc->flags & mask)
			return assoc;
	}
	return nil;
}

void
RpAnimBlendAllocateData(RpClump *clump)
{
	*RPANIMBLENDCLUMPDATA(clump) = new CAnimBlendClumpData;
}


void
RpAnimBlendClumpSetBlendDeltas(RpClump *clump, uint32 mask, float delta)
{
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);
		if(mask == 0 || (assoc->flags & mask))
			assoc->blendDelta = delta;
	}
}

void
RpAnimBlendClumpRemoveAllAssociations(RpClump *clump)
{
	RpAnimBlendClumpRemoveAssociations(clump, 0);
}

void
RpAnimBlendClumpRemoveAssociations(RpClump *clump, uint32 mask)
{
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	CAnimBlendLink *next;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = next){
		next = link->next;
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);
		if(mask == 0 || (assoc->flags & mask))
			if(assoc)
				delete assoc;
	}
}

RwFrame*
FrameForAllChildrenCountCallBack(RwFrame *frame, void *data)
{
	int *numFrames = (int*)data;
	(*numFrames)++;
	RwFrameForAllChildren(frame, FrameForAllChildrenCountCallBack, data);
	return frame;
}

RwFrame*
FrameForAllChildrenFillFrameArrayCallBack(RwFrame *frame, void *data)
{
	AnimBlendFrameData **frames = (AnimBlendFrameData**)data;
	(*frames)->frame = frame;
	(*frames)++;
	RwFrameForAllChildren(frame, FrameForAllChildrenFillFrameArrayCallBack, frames);
	return frame;
}

void
FrameInitCallBack(AnimBlendFrameData *frameData, void*)
{
	frameData->flag = 0;
	frameData->resetPos = *RwMatrixGetPos(RwFrameGetMatrix(frameData->frame));
}

void
RpAnimBlendClumpInit(RpClump *clump)
{
#ifdef PED_SKIN
	TODO
#else
	int numFrames = 0;
	CAnimBlendClumpData *clumpData;
	RwFrame *root;
	AnimBlendFrameData *frames;

	RpAnimBlendAllocateData(clump);
	clumpData = *RPANIMBLENDCLUMPDATA(clump);
	root = RpClumpGetFrame(clump);
	RwFrameForAllChildren(root, FrameForAllChildrenCountCallBack, &numFrames);
	clumpData->SetNumberOfFrames(numFrames);
	frames = clumpData->frames;
	RwFrameForAllChildren(root, FrameForAllChildrenFillFrameArrayCallBack, &frames);
	clumpData->ForAllFrames(FrameInitCallBack, nil);
	clumpData->frames[0].flag |= AnimBlendFrameData::VELOCITY_EXTRACTION;
#endif
}

bool
RpAnimBlendClumpIsInitialized(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	return clumpData && clumpData->numFrames != 0;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetAssociation(RpClump *clump, uint32 id)
{
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == nil) return nil;

	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);
		if(assoc->animId == id)
			return assoc;
	}
	return nil;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainAssociation(RpClump *clump, CAnimBlendAssociation **assocRet, float *blendRet)
{
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == nil) return nil;

	CAnimBlendAssociation *mainAssoc = nil;
	CAnimBlendAssociation *secondAssoc = nil;
	float mainBlend = 0.0f;
	float secondBlend = 0.0f;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);

		if(assoc->IsPartial())
			continue;

		if(assoc->blendAmount > mainBlend){
			secondBlend = mainBlend;
			mainBlend = assoc->blendAmount;

			secondAssoc = mainAssoc;
			mainAssoc = assoc;
		}else if(assoc->blendAmount > secondBlend){
			secondBlend = assoc->blendAmount;
			secondAssoc = assoc;
		}
	}
	if(assocRet) *assocRet = secondAssoc;
	if(blendRet) *blendRet = secondBlend;
	return mainAssoc;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainPartialAssociation(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == nil) return nil;

	CAnimBlendAssociation *mainAssoc = nil;
	float mainBlend = 0.0f;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);

		if(!assoc->IsPartial())
			continue;

		if(assoc->blendAmount > mainBlend){
			mainBlend = assoc->blendAmount;
			mainAssoc = assoc;
		}
	}
	return mainAssoc;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainAssociation_N(RpClump *clump, int n)
{
	int i;
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == nil) return nil;

	i = 0;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);

		if(assoc->IsPartial())
			continue;

		if(i == n)
			return assoc;
		i++;
	}
	return nil;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainPartialAssociation_N(RpClump *clump, int n)
{
	int i;
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == nil) return nil;

	i = 0;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);

		if(!assoc->IsPartial())
			continue;

		if(i == n)
			return assoc;
		i++;
	}
	return nil;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetFirstAssociation(RpClump *clump, uint32 mask)
{
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == nil) return nil;

	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);
		if(assoc->flags & mask)
			return assoc;
	}
	return nil;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetFirstAssociation(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	if(clumpData == nil) return nil;
	if(clumpData->link.next == nil) return nil;
	return CAnimBlendAssociation::FromLink(clumpData->link.next); 
}

void
FillFrameArrayCallBack(AnimBlendFrameData *frame, void *arg)
{
	AnimBlendFrameData **frames = (AnimBlendFrameData**)arg;
	frames[CVisibilityPlugins::GetFrameHierarchyId(frame->frame)] = frame;
}

void
RpAnimBlendClumpFillFrameArray(RpClump *clump, AnimBlendFrameData **frames)
{
#ifdef PED_SKIN
	TODO
#else
	(*RPANIMBLENDCLUMPDATA(clump))->ForAllFrames(FillFrameArrayCallBack, frames);
#endif
}

AnimBlendFrameData *pFrameDataFound;

void
FrameFindCallBack(AnimBlendFrameData *frame, void *arg)
{
	char *nodename = GetFrameNodeName(frame->frame);
	if(!CGeneral::faststricmp(nodename, (char*)arg))
		pFrameDataFound = frame;
}

AnimBlendFrameData*
RpAnimBlendClumpFindFrame(RpClump *clump, const char *name)
{
	pFrameDataFound = nil;
#ifdef PED_SKIN
	TODO
#else
	(*RPANIMBLENDCLUMPDATA(clump))->ForAllFrames(FrameFindCallBack, (void*)name);
#endif
	return pFrameDataFound;
}

void
RpAnimBlendClumpUpdateAnimations(RpClump *clump, float timeDelta)
{
	int i;
	AnimBlendFrameUpdateData updateData;
	float totalLength = 0.0f;
	float totalBlend = 0.0f;
	CAnimBlendLink *link, *next;
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	gpAnimBlendClump = clumpData;

	if(clumpData->link.next == nil)
		return;

	// Update blend and get node array
	i = 0;
	updateData.foobar = 0;
	for(link = clumpData->link.next; link; link = next){
		next = link->next;
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);
		if(assoc->UpdateBlend(timeDelta)){
			// CAnimManager::UncompressAnimation(v6->hierarchy)
			updateData.nodes[i++] = assoc->GetNode(0);
			if(assoc->flags & ASSOC_MOVEMENT){
				totalLength += assoc->hierarchy->totalLength/assoc->speed * assoc->blendAmount;
				totalBlend += assoc->blendAmount;
			}else
				updateData.foobar = 1;
		}
	}
	updateData.nodes[i] = nil;

	clumpData->ForAllFrames(FrameUpdateCallBack, &updateData);

	for(link = clumpData->link.next; link; link = link->next){
		CAnimBlendAssociation *assoc = CAnimBlendAssociation::FromLink(link);
		float relSpeed = totalLength == 0.0f ? 1.0f : totalBlend/totalLength;
		assoc->UpdateTime(timeDelta, relSpeed);
	}
	RwFrameUpdateObjects(RpClumpGetFrame(clump));
}


STARTPATCHES
	InjectHook(0x4052D0, RpAnimBlendPluginAttach, PATCH_JUMP);
	InjectHook(0x4052A0, RpAnimBlendAllocateData, PATCH_JUMP);
	InjectHook(0x405780, (CAnimBlendAssociation *(*)(CAnimBlendAssociation*))RpAnimBlendGetNextAssociation, PATCH_JUMP);
	InjectHook(0x4057A0, (CAnimBlendAssociation *(*)(CAnimBlendAssociation*,uint32))RpAnimBlendGetNextAssociation, PATCH_JUMP);

	InjectHook(0x405520, RpAnimBlendClumpSetBlendDeltas, PATCH_JUMP);
	InjectHook(0x405560, RpAnimBlendClumpRemoveAllAssociations, PATCH_JUMP);
	InjectHook(0x405570, RpAnimBlendClumpRemoveAssociations, PATCH_JUMP);
	InjectHook(0x405480, RpAnimBlendClumpInit, PATCH_JUMP);
	InjectHook(0x405500, RpAnimBlendClumpIsInitialized, PATCH_JUMP);
	InjectHook(0x4055C0, RpAnimBlendClumpGetAssociation, PATCH_JUMP);
	InjectHook(0x4055F0, RpAnimBlendClumpGetMainAssociation, PATCH_JUMP);
	InjectHook(0x405680, RpAnimBlendClumpGetMainPartialAssociation, PATCH_JUMP);
	InjectHook(0x4056D0, RpAnimBlendClumpGetMainAssociation_N, PATCH_JUMP);
	InjectHook(0x405710, RpAnimBlendClumpGetMainPartialAssociation_N, PATCH_JUMP);
	InjectHook(0x405750, (CAnimBlendAssociation *(*)(RpClump*, uint32))RpAnimBlendClumpGetFirstAssociation, PATCH_JUMP);
	InjectHook(0x4031B0, (CAnimBlendAssociation *(*)(RpClump*))RpAnimBlendClumpGetFirstAssociation, PATCH_JUMP);
	InjectHook(0x405460, RpAnimBlendClumpFillFrameArray, PATCH_JUMP);
	InjectHook(0x4024B0, RpAnimBlendClumpUpdateAnimations, PATCH_JUMP);
ENDPATCHES
