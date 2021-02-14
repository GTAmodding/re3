#pragma once

struct CColModel;

#define MAX_MODEL_NAME (24)

enum ModelInfoType
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

class C2dEffect;

class CBaseModelInfo
{
protected:
	char         m_name[MAX_MODEL_NAME];
	CColModel   *m_colModel;
	C2dEffect   *m_twodEffects;
	int16        m_objectId;
	uint16       m_refCount;
	int16        m_txdSlot;
	uint8        m_type;
	uint8        m_num2dEffects;
	bool         m_bOwnsColModel;
#ifdef EXTRA_MODEL_FLAGS
public:
	// from mobile
	bool         m_bIsDoubleSided;
	bool         m_bIsTree;
	bool         m_bCanBeIgnored;	// for low-end devices
	bool RenderDoubleSided(void) { return m_bIsDoubleSided || m_bIsTree; }
#endif

public:
	CBaseModelInfo(ModelInfoType type);
	virtual ~CBaseModelInfo() {}
	virtual void Shutdown(void);
	virtual void DeleteRwObject(void) = 0;
	virtual RwObject *CreateInstance(void) = 0;
	virtual RwObject *CreateInstance(RwMatrix *) = 0;
	virtual RwObject *GetRwObject(void) = 0;

	// one day it becomes virtual
	uint8 GetModelType() const { return m_type; }
	bool IsSimple(void) { return m_type == MITYPE_SIMPLE || m_type == MITYPE_TIME; }
	bool IsClump(void) { return m_type == MITYPE_CLUMP || m_type == MITYPE_PED || m_type == MITYPE_VEHICLE ||
		m_type == MITYPE_MLO || m_type == MITYPE_XTRACOMPS;	// unused but what the heck
	}
	char *GetModelName(void) { return m_name; }
	void SetModelName(const char *name) { strncpy(m_name, name, MAX_MODEL_NAME); }
	void SetColModel(CColModel *col, bool owns = false){
		m_colModel = col; m_bOwnsColModel = owns; }
	CColModel *GetColModel(void) { return m_colModel; }
	bool DoesOwnColModel(void) { return m_bOwnsColModel; }
	void DeleteCollisionModel(void);
	void ClearTexDictionary(void) { m_txdSlot = -1; }
	int16 GetObjectID(void) { return m_objectId; }
	void SetObjectID(int16 id) { m_objectId = id; }
	int16 GetTxdSlot(void) { return m_txdSlot; }
	void AddRef(void);
	void RemoveRef(void);
	void SetTexDictionary(const char *name);
	void AddTexDictionaryRef(void);
	void RemoveTexDictionaryRef(void);
	void Init2dEffects(void);
	void Add2dEffect(C2dEffect *fx);
	C2dEffect *Get2dEffect(int n);
	uint8 GetNum2dEffects() const { return m_num2dEffects; }
	uint16 GetNumRefs() const { return m_refCount; }
};

VALIDATE_SIZE(CBaseModelInfo, 0x30);
