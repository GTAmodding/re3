#include "common.h"
#include "patcher.h"
#include "templates.h"
#include "Streaming.h"
#include "RwHelper.h"
#include "TxdStore.h"

CPool<TxdDef,TxdDef> *&CTxdStore::ms_pTxdPool = *(CPool<TxdDef,TxdDef>**)0x8F5FB8;
RwTexDictionary *&CTxdStore::ms_pStoredTxd = *(RwTexDictionary**)0x9405BC;

void
CTxdStore::Initialize(void)
{
	if(ms_pTxdPool == nil)
		ms_pTxdPool = new CPool<TxdDef,TxdDef>(TXDSTORESIZE);
}

void
CTxdStore::Shutdown(void)
{
	if(ms_pTxdPool)
		delete ms_pTxdPool;
}

void
CTxdStore::GameShutdown(void)
{
	int i;

	for(i = 0; i < TXDSTORESIZE; i++){
		TxdDef *def = GetSlot(i);
		if(def && GetNumRefs(i) == 0)
			RemoveTxdSlot(i);
	}
}

int
CTxdStore::AddTxdSlot(const char *name)
{
	TxdDef *def = ms_pTxdPool->New();
	assert(def);
	def->texDict = nil;
	def->refCount = 0;
	strcpy(def->name, name);
	return ms_pTxdPool->GetJustIndex(def);
}

void
CTxdStore::RemoveTxdSlot(int slot)
{
	TxdDef *def = GetSlot(slot);
	if(def->texDict)
		RwTexDictionaryDestroy(def->texDict);
	ms_pTxdPool->Delete(def);
}

int
CTxdStore::FindTxdSlot(const char *name)
{
	char *defname;
	int size = ms_pTxdPool->GetSize();
	for(int i = 0; i < size; i++){
		defname = GetTxdName(i);
		if(defname && _strcmpi(defname, name) == 0)
			return i;
	}
	return -1;
}

char*
CTxdStore::GetTxdName(int slot)
{
	TxdDef *def = GetSlot(slot);
	return def ? def->name : nil;
}

void
CTxdStore::PushCurrentTxd(void)
{
	ms_pStoredTxd = RwTexDictionaryGetCurrent();
}

void
CTxdStore::PopCurrentTxd(void)
{
	RwTexDictionarySetCurrent(ms_pStoredTxd);
	ms_pStoredTxd = nil;
}

void
CTxdStore::SetCurrentTxd(int slot)
{
	TxdDef *def = GetSlot(slot);
	if(def)
		RwTexDictionarySetCurrent(def->texDict);
}

void
CTxdStore::Create(int slot)
{
	GetSlot(slot)->texDict = RwTexDictionaryCreate();
}

int
CTxdStore::GetNumRefs(int slot)
{
	return GetSlot(slot)->refCount;
}

void
CTxdStore::AddRef(int slot)
{
	GetSlot(slot)->refCount++;
}

void
CTxdStore::RemoveRef(int slot)
{
	if(--GetSlot(slot)->refCount <= 0)
		CStreaming::RemoveModel(slot + STREAM_OFFSET_TXD);
}

void
CTxdStore::RemoveRefWithoutDelete(int slot)
{
	GetSlot(slot)->refCount--;
}

bool
CTxdStore::LoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);

	if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil)){
		def->texDict = RwTexDictionaryGtaStreamRead(stream);
		return def->texDict != nil;
	}
	printf("Failed to load TXD\n");
	return false;
}

bool
CTxdStore::LoadTxd(int slot, const char *filename)
{
	RwStream *stream;
	bool ret;

	ret = false;
	_rwD3D8TexDictionaryEnableRasterFormatConversion(true);
	do
		stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	while(stream == nil);
	ret = LoadTxd(slot, stream);
	RwStreamClose(stream, nil);
	return ret;
}

bool
CTxdStore::StartLoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);
	if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil)){
		def->texDict = RwTexDictionaryGtaStreamRead1(stream);
		return def->texDict != nil;
	}else{
		printf("Failed to load TXD\n");
		return nil;
	}
}

bool
CTxdStore::FinishLoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);
	def->texDict = RwTexDictionaryGtaStreamRead2(stream, def->texDict);
	return def->texDict != nil;
}

void
CTxdStore::RemoveTxd(int slot)
{
	TxdDef *def = GetSlot(slot);
	if(def->texDict)
		RwTexDictionaryDestroy(def->texDict);
	def->texDict = nil;
}

STARTPATCHES
	InjectHook(0x527440, CTxdStore::Initialize, PATCH_JUMP);
	InjectHook(0x527470, CTxdStore::Shutdown, PATCH_JUMP);
	InjectHook(0x527490, CTxdStore::GameShutdown, PATCH_JUMP);
	InjectHook(0x5274E0, CTxdStore::AddTxdSlot, PATCH_JUMP);
	InjectHook(0x5275D0, CTxdStore::FindTxdSlot, PATCH_JUMP);
	InjectHook(0x527590, CTxdStore::GetTxdName, PATCH_JUMP);
	InjectHook(0x527900, CTxdStore::PushCurrentTxd, PATCH_JUMP);
	InjectHook(0x527910, CTxdStore::PopCurrentTxd, PATCH_JUMP);
	InjectHook(0x5278C0, CTxdStore::SetCurrentTxd, PATCH_JUMP);
	InjectHook(0x527830, CTxdStore::Create, PATCH_JUMP);
	InjectHook(0x527A00, CTxdStore::GetNumRefs, PATCH_JUMP);
	InjectHook(0x527930, CTxdStore::AddRef, PATCH_JUMP);
	InjectHook(0x527970, CTxdStore::RemoveRef, PATCH_JUMP);
	InjectHook(0x5279C0, CTxdStore::RemoveRefWithoutDelete, PATCH_JUMP);
	InjectHook(0x527700, (bool (*)(int, RwStream*))CTxdStore::LoadTxd, PATCH_JUMP);
	InjectHook(0x5276B0, (bool (*)(int, const char*))CTxdStore::LoadTxd, PATCH_JUMP);
	InjectHook(0x527770, CTxdStore::StartLoadTxd, PATCH_JUMP);
	InjectHook(0x5277E0, CTxdStore::FinishLoadTxd, PATCH_JUMP);
	InjectHook(0x527870, CTxdStore::RemoveTxd, PATCH_JUMP);
ENDPATCHES
