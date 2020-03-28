#include "common.h"
#include "patcher.h"
#include "DMAudio.h"
#include "AudioScriptObject.h"
#include "Pools.h"

WRAPPER void cAudioScriptObject::SaveAllAudioScriptObjects(uint8 *buf, uint32 *size) { EAXJMP(0x57c460); }

void
cAudioScriptObject::Reset()
{
	AudioId = 125;
	Posn = CVector(0.0f, 0.0f, 0.0f);
	AudioEntity = AEHANDLE_NONE;
}

void *
cAudioScriptObject::operator new(size_t sz)
{
	return CPools::GetAudioScriptObjectPool()->New();
}
void *
cAudioScriptObject::operator new(size_t sz, int handle)
{
	return CPools::GetAudioScriptObjectPool()->New(handle);
}
void
cAudioScriptObject::operator delete(void *p, size_t sz)
{
	CPools::GetAudioScriptObjectPool()->Delete((cAudioScriptObject *)p);
}
void
cAudioScriptObject::operator delete(void *p, int handle)
{
	CPools::GetAudioScriptObjectPool()->Delete((cAudioScriptObject *)p);
}

void
PlayOneShotScriptObject(uint8 id, CVector const &pos)
{
	cAudioScriptObject *audioScriptObject = new cAudioScriptObject();
	audioScriptObject->Posn = pos;
	audioScriptObject->AudioId = id;
	audioScriptObject->AudioEntity = AEHANDLE_NONE;
	DMAudio.CreateOneShotScriptObject(audioScriptObject);
}

STARTPATCHES
InjectHook(0x57C430, &cAudioScriptObject::Reset, PATCH_JUMP);
InjectHook(0x57C5F0, &PlayOneShotScriptObject, PATCH_JUMP);
ENDPATCHES