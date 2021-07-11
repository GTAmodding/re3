#include "common.h"

#include "AudioScriptObject.h"
#include "Pools.h"
#include "DMAudio.h"
#include "SaveBuf.h"

cAudioScriptObject::cAudioScriptObject()
{
	Reset();
};

cAudioScriptObject::~cAudioScriptObject()
{
	Reset();
};

void
cAudioScriptObject::Reset()
{
	AudioId = SCRIPT_SOUND_INVALID;
	Posn = CVector(0.0f, 0.0f, 0.0f);
	AudioEntity = AEHANDLE_NONE;
}

void *
cAudioScriptObject::operator new(size_t sz) throw()
{
	return CPools::GetAudioScriptObjectPool()->New();
}

void *
cAudioScriptObject::operator new(size_t sz, int handle) throw()
{
	return CPools::GetAudioScriptObjectPool()->New(handle);
}

void
cAudioScriptObject::operator delete(void *p, size_t sz) throw()
{
	CPools::GetAudioScriptObjectPool()->Delete((cAudioScriptObject *)p);
}

void
cAudioScriptObject::operator delete(void *p, int handle) throw()
{
	CPools::GetAudioScriptObjectPool()->Delete((cAudioScriptObject *)p);
}

void
cAudioScriptObject::LoadAllAudioScriptObjects(uint8 *buf, uint32 size)
{
	INITSAVEBUF

	CheckSaveHeader(buf, 'A', 'U', 'D', '\0', size - SAVE_HEADER_SIZE);

	int32 pool_size;
	ReadSaveBuf(&pool_size, buf);
	for (int32 i = 0; i < pool_size; i++) {
		int32 handle;
		ReadSaveBuf(&handle, buf);
		cAudioScriptObject *p = new(handle) cAudioScriptObject;
		assert(p != nil);
		ReadSaveBuf(p, buf);
		p->AudioEntity = DMAudio.CreateLoopingScriptObject(p);
	}

	VALIDATESAVEBUF(size);
}

void
cAudioScriptObject::SaveAllAudioScriptObjects(uint8 *buf, uint32 *size)
{
	INITSAVEBUF

	int32 pool_size = CPools::GetAudioScriptObjectPool()->GetNoOfUsedSpaces();
	*size = SAVE_HEADER_SIZE + sizeof(int32) + pool_size * (sizeof(cAudioScriptObject) + sizeof(int32));
	WriteSaveHeader(buf, 'A', 'U', 'D', '\0', *size - SAVE_HEADER_SIZE);
	WriteSaveBuf(buf, pool_size);

	int32 i = CPools::GetAudioScriptObjectPool()->GetSize();
	while (i--) {
		cAudioScriptObject *p = CPools::GetAudioScriptObjectPool()->GetSlot(i);
		if (p != nil) {
			WriteSaveBuf(buf, CPools::GetAudioScriptObjectPool()->GetIndex(p));
			WriteSaveBuf(buf, *p);
		}
	}

	VALIDATESAVEBUF(*size);
}

void
PlayOneShotScriptObject(uint8 id, CVector const &pos)
{
	if (!DMAudio.IsAudioInitialised()) return;

	cAudioScriptObject *audioScriptObject = new cAudioScriptObject();
	audioScriptObject->Posn = pos;
	audioScriptObject->AudioId = id;
	audioScriptObject->AudioEntity = AEHANDLE_NONE;
	DMAudio.CreateOneShotScriptObject(audioScriptObject);
}
