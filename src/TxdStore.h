#pragma once

#include "templates.h"

struct TxdDef {
	RwTexDictionary *texDict;
	int refCount;
	char name[20];
};

class CTxdStore
{
	static CPool<TxdDef,TxdDef> *&ms_pTxdPool;
	static RwTexDictionary *&ms_pStoredTxd;
public:
	static void Initialize(void);
	static void Shutdown(void);
	static void GameShutdown(void);
	static int AddTxdSlot(const char *name);
	static void RemoveTxdSlot(int slot);
	static int FindTxdSlot(const char *name);
	static char *GetTxdName(int slot);
	static void PushCurrentTxd(void);
	static void PopCurrentTxd(void);
	static void SetCurrentTxd(int slot);
	static void Create(int slot);
	static int GetNumRefs(int slot);
	static void AddRef(int slot);
	static void RemoveRef(int slot);
	static void RemoveRefWithoutDelete(int slot);
	static bool LoadTxd(int slot, RwStream *stream);
	static bool LoadTxd(int slot, const char *filename);
	static bool StartLoadTxd(int slot, RwStream *stream);
	static bool FinishLoadTxd(int slot, RwStream *stream);
	static void RemoveTxd(int slot);

	static TxdDef *GetSlot(int slot) {
		assert(slot >= 0);
		assert(ms_pTxdPool);
		assert(slot < ms_pTxdPool->GetSize());
		return ms_pTxdPool->GetSlot(slot);
	}
	static bool isTxdLoaded(int slot);
};
