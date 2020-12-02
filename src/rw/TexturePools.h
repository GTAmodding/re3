#pragma once

class CTexturePool
{
public:
	D3DFORMAT Format;
	int size;
	uint32 levels;
	int32 texturesMax;
	int32 texturesUsed;
	int32 texturesNum;
	IDirect3DTexture8 **pTextures;

public:
	CTexturePool() {}
	void Create(D3DFORMAT _Format, int size, uint32 mipmapLevels, int32 numTextures);
	void Release();
	IDirect3DTexture8 *FindTexture();
	bool AddTexture(IDirect3DTexture8 *texture);
	void Resize(int numTextures);
#ifdef FIX_BUGS
	int GetSize() { return size; }
#else
	float GetSize() { return size; }
#endif
};

class CPaletteList
{
	int Max;
	int Num;
	int *Data;
public:
	void Alloc(int max);
	void Free();
	int Find();
	void Add(int item);
	void Resize(int max);
};

void _TexturePoolsInitialise();
void _TexturePoolsShutdown();