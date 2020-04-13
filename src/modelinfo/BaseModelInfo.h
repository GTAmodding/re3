#pragma once

#include "Collision.h"

#define MAX_MODEL_NAME (24)

enum ModeInfoType : uint8
{
	MITYPE_NA        = 0,
	MITYPE_SIMPLE    = 1,
	MITYPE_MLO       = 2,
	MITYPE_TIME      = 3,
	MITYPE_CLUMP     = 4,
	MITYPE_VEHICLE   = 5,
	MITYPE_PED       = 6,
	MITYPE_XTRACOMPS = 7,
};
static_assert(sizeof(ModeInfoType) == 1, "ModeInfoType: error");

class C2dEffect;

class CBaseModelInfo
{
protected:
	// TODO?: make more things protected
	char         m_name[MAX_MODEL_NAME];
	CColModel   *m_colModel;
	C2dEffect   *m_twodEffects;
	int16        m_objectId;
public:
	uint16       m_refCount;
	int16        m_txdSlot;
	ModeInfoType m_type;
	uint8        m_num2dEffects;
	bool         m_freeCol;

	CBaseModelInfo(ModeInfoType type);
	virtual ~CBaseModelInfo() {}
	virtual void Shutdown(void);
	virtual void DeleteRwObject(void) = 0;
	virtual RwObject *CreateInstance(RwMatrix *) = 0;
	virtual RwObject *CreateInstance(void) = 0;
	virtual RwObject *GetRwObject(void) = 0;

	bool IsSimple(void) { return m_type == MITYPE_SIMPLE || m_type == MITYPE_TIME; }
	bool IsClump(void) { return m_type == MITYPE_CLUMP || m_type == MITYPE_PED || m_type == MITYPE_VEHICLE ||
		m_type == MITYPE_MLO || m_type == MITYPE_XTRACOMPS;	// unused but what the heck
	}
	char *GetName(void) { return m_name; }
	void SetName(const char *name) { strncpy(m_name, name, 24); }
	void SetColModel(CColModel *col, bool free = false){
		m_colModel = col; m_freeCol = free; }
	CColModel *GetColModel(void) { return m_colModel; }
	void DeleteCollisionModel(void);
	void ClearTexDictionary(void) { m_txdSlot = -1; }
	short GetObjectID(void) { return m_objectId; }
	void SetObjectID(int16 id) { m_objectId = id; }
	short GetTxdSlot(void) { return m_txdSlot; }
	void AddRef(void);
	void RemoveRef(void);
	void SetTexDictionary(const char *name);
	void AddTexDictionaryRef(void);
	void RemoveTexDictionaryRef(void);
	void Init2dEffects(void);
	void Add2dEffect(C2dEffect *fx);
	C2dEffect *Get2dEffect(int n);
};

static_assert(sizeof(CBaseModelInfo) == 0x30, "CBaseModelInfo: error");
