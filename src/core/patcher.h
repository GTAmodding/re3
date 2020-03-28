#pragma once

#define WRAPPER __declspec(naked)
#define DEPRECATED __declspec(deprecated)
#define EAXJMP(a) { _asm mov eax, a _asm jmp eax }
#define VARJMP(a) { _asm jmp a }
#define WRAPARG(a) UNREFERENCED_PARAMETER(a)

//#include <string.h>	//memset

enum
{
	PATCH_CALL,
	PATCH_JUMP,
	PATCH_NOTHING,
};

enum
{
	III_10 = 1,
	III_11,
	III_STEAM,
	VC_10,
	VC_11,
	VC_STEAM
};

extern int gtaversion;

class StaticPatcher
{
private:
	using Patcher = void(*)();

	Patcher		m_func;
	StaticPatcher	*m_next;
	static StaticPatcher	*ms_head;

	void Run() { m_func(); }
public:
	StaticPatcher(Patcher func);
	static void Apply();
};

template<typename T>
inline T AddressByVersion(uint32_t addressIII10, uint32_t addressIII11, uint32_t addressIIISteam, uint32_t addressvc10, uint32_t addressvc11, uint32_t addressvcSteam)
{
	if(gtaversion == -1){
		     if(*(uint32_t*)0x5C1E75 == 0xB85548EC) gtaversion = III_10;
		else if(*(uint32_t*)0x5C2135 == 0xB85548EC) gtaversion = III_11;
		else if(*(uint32_t*)0x5C6FD5 == 0xB85548EC) gtaversion = III_STEAM;
		else if(*(uint32_t*)0x667BF5 == 0xB85548EC) gtaversion = VC_10;
		else if(*(uint32_t*)0x667C45 == 0xB85548EC) gtaversion = VC_11;
		else if(*(uint32_t*)0x666BA5 == 0xB85548EC) gtaversion = VC_STEAM;
		else gtaversion = 0;
	}
	switch(gtaversion){
	case III_10:
		return (T)addressIII10;
	case III_11:
		return (T)addressIII11;
	case III_STEAM:
		return (T)addressIIISteam;
	case VC_10:
		return (T)addressvc10;
	case VC_11:
		return (T)addressvc11;
	case VC_STEAM:
		return (T)addressvcSteam;
	default:
		return (T)0;
	}
}

inline bool
is10(void)
{
	return gtaversion == III_10 || gtaversion == VC_10;
}

inline bool
isIII(void)
{
	return gtaversion >= III_10 && gtaversion <= III_STEAM;
}

inline bool
isVC(void)
{
	return gtaversion >= VC_10 && gtaversion <= VC_STEAM;
}

#define PTRFROMCALL(addr) (uint32_t)(*(uint32_t*)((uint32_t)addr+1) + (uint32_t)addr + 5)
#define INTERCEPT(saved, func, a) \
{ \
	saved = PTRFROMCALL(a); \
	InjectHook(a, func); \
}

void InjectHook_internal(uint32 address, uint32 hook, int type);
void Protect_internal(uint32 address, uint32 size);
void Unprotect_internal(void);

template<typename T, typename AT> inline void
Patch(AT address, T value)
{
	Protect_internal((uint32)address, sizeof(T));
	*(T*)address = value;
	Unprotect_internal();
}

template<typename AT> inline void
Nop(AT address, unsigned int nCount)
{
	Protect_internal((uint32)address, nCount);
	//memset((void*)address, 0x90, nCount);
	for (unsigned int i = 0; i < nCount; i++) *(uint8*)(address++) = 0x90;
	Unprotect_internal();
}

template<typename AT, typename HT> inline void
InjectHook(AT address, HT hook, unsigned int nType=PATCH_NOTHING)
{
	uint32		uiHook;
	_asm
	{
		mov		eax, hook
		mov		uiHook, eax
	}
	InjectHook_internal((uint32)address, uiHook, nType);
}

inline void ExtractCall(void *dst, uint32_t a)
{
	*(uint32_t*)dst = (uint32_t)(*(uint32_t*)(a+1) + a + 5);
}
template<typename T>
inline void InterceptCall(void *dst, T func, uint32_t a)
{
	ExtractCall(dst, a);
	InjectHook(a, func);
}
template<typename T>
inline void InterceptVmethod(void *dst, T func, uint32_t a)
{
	*(uint32_t*)dst = *(uint32_t*)a;
	Patch(a, func);
}

#define STARTPATCHES static StaticPatcher Patcher([](){
#define ENDPATCHES });
