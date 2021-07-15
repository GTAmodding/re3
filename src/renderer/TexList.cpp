#include "common.h"
#include "TexList.h"
#include "rtbmp.h"
#include "FileMgr.h"

bool CTexList::ms_nTexUsed[MAX_TEXUSED];

void
CTexList::Initialise()
{}

void
CTexList::Shutdown()
{}

RwTexture *
CTexList::SetTexture(int32 slot, char *name)
{
	return nil;
}

int32
CTexList::GetFirstFreeTexture()
{
	for (int32 i = 0; i < MAX_TEXUSED; i++)
		if (!ms_nTexUsed[i])
			return i;
	return -1;
}

RwTexture *
CTexList::LoadFileNameTexture(char *name)
{
	return SetTexture(GetFirstFreeTexture(), name);
}

void
CTexList::LoadGlobalTextureList()
{
	CFileMgr::SetDir("TEXTURES");
}