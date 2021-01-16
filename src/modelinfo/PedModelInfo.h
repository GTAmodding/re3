#pragma once

#include "ClumpModelInfo.h"
#include "ColModel.h"
#include "PedType.h"

enum PedNode {
	PED_TORSO,
	PED_MID,	// Smid on PS2/PC, Storso on mobile/xbox
	PED_HEAD,
	PED_UPPERARML,
	PED_UPPERARMR,
	PED_HANDL,
	PED_HANDR,
	PED_UPPERLEGL,
	PED_UPPERLEGR,
	PED_FOOTL,
	PED_FOOTR,
	PED_LOWERLEGR,
	PED_NODE_MAX// Not valid: PED_LOWERLEGL
};

class CPedModelInfo : public CClumpModelInfo
{
public:
	uint32 m_animGroup;
	ePedType m_pedType;
	ePedStats m_pedStatType;
	uint32 m_carsCanDrive;
	CColModel *m_hitColModel;
#ifdef PED_SKIN
	RpAtomic *m_head;
	RpAtomic *m_lhand;
	RpAtomic *m_rhand;
#endif

	static RwObjectNameIdAssocation m_pPedIds[PED_NODE_MAX];

	CPedModelInfo(void) : CClumpModelInfo(MITYPE_PED) {
		m_hitColModel = nil;
#ifdef PED_SKIN
		m_head = nil;
		m_lhand = nil;
		m_rhand = nil;
#endif
	}
	~CPedModelInfo(void) { delete m_hitColModel; }
	void DeleteRwObject(void);
	void SetClump(RpClump *);

	void SetLowDetailClump(RpClump*);
	void CreateHitColModel(void);
	void CreateHitColModelSkinned(RpClump *clump);
	CColModel *GetHitColModel(void) { return m_hitColModel; }
	static CColModel *AnimatePedColModel(CColModel* colmodel, RwFrame* frame);
	CColModel *AnimatePedColModelSkinned(RpClump *clump);

#ifdef PED_SKIN
	static RpAtomic *findLimbsCb(RpAtomic *atomic, void *data);
	RpAtomic *getHead(void) { return m_head; }
	RpAtomic *getLeftHand(void) { return m_lhand; }
	RpAtomic *getRightHand(void) { return m_rhand; }
#endif
};
#ifndef PED_SKIN
VALIDATE_SIZE(CPedModelInfo, 0x48);
#endif