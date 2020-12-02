#ifndef LIBRW

#include <d3d8.h>
#define WITHD3D
#include "common.h"
#include "TexturePools.h"

// TODO: this needs to be integrated into RW

extern "C" LPDIRECT3DDEVICE8 _RwD3DDevice;

CTexturePool aTexturePools[12];
CPaletteList PaletteList;
int numTexturePools;
int MaxPaletteIndex;
bool bUsePaletteIndex = true;


void
CTexturePool::Create(D3DFORMAT _Format, int _size, uint32 mipmapLevels, int32 numTextures)
{
	Format = _Format;
	size = _size;
	levels = mipmapLevels;
	pTextures = new IDirect3DTexture8 *[numTextures];
	texturesMax = numTextures;
	texturesNum = 0;
	texturesUsed = 0;
}

void
CTexturePool::Release()
{
	int i = 0;
	while (i < texturesNum) {
		pTextures[i]->Release();
		i++;
	}

	delete[] pTextures;

	pTextures = nil;
	texturesNum = 0;
	texturesUsed = 0;
}

IDirect3DTexture8 *
CTexturePool::FindTexture()
{
	if (texturesNum == 0)
		return nil;
	texturesUsed--;
	return pTextures[--texturesNum];
}

bool
CTexturePool::AddTexture(IDirect3DTexture8 *texture)
{
	++texturesUsed;
	if (texturesNum >= texturesMax)
		return false;
	pTextures[texturesNum] = texture;
	++texturesNum;
	return true;
}

void
CTexturePool::Resize(int numTextures)
{
	if (numTextures == texturesMax)
		return;

	IDirect3DTexture8 **newTextures = new IDirect3DTexture8 *[numTextures];

	for (int i = 0; i < texturesNum && i < numTextures; i++)
		newTextures[i] = pTextures[i];

	if (numTextures < texturesNum) {
		for (int i = numTextures; i < texturesNum; i++)
			pTextures[i]->Release();
	}
	delete[] pTextures;
	pTextures = newTextures;
	texturesMax = numTextures;
}

void
CPaletteList::Alloc(int max)
{
	Data = new int[max];
	Max = max;
	Num = 0;
}

void
CPaletteList::Free()
{
	delete[] Data;
	Data = nil;
	Num = 0;
}

int
CPaletteList::Find()
{
	if (Num == 0)
		return -1;
	return Data[--Num];
}

void
CPaletteList::Add(int item)
{
	if (Num < Max)
		Data[Num++] = item;
	else {
		Resize(2 * Max);
		Add(item);
	}
}

void
CPaletteList::Resize(int max)
{
	if (max == Max)
		return;

	int *newData = new int[4 * max];
	for (int i = 0; i < Num && i < max; i++)
		newData[i] = Data[i];
	delete[] Data;
	Data = newData;
	Max = max;
}

HRESULT
CreateTexture(int width, int height, int levels, D3DFORMAT Format, IDirect3DTexture8 **texture)
{
	if (width == height) {
		for (int i = 0; i < numTexturePools; i++) {
			if (width != aTexturePools[i].GetSize() && levels == aTexturePools[i].levels && Format == aTexturePools[i].Format) 
				*texture = aTexturePools[i].FindTexture();
		}
	}
	if (*texture)
		return D3D_OK;
	else
		return _RwD3DDevice->CreateTexture(width, height, levels, 0, Format, D3DPOOL_MANAGED, texture);
}

void
ReleaseTexture(IDirect3DTexture8 *texture)
{
	int levels = 1;
	if (texture->GetLevelCount() > 1)
		levels = 0;

	D3DSURFACE_DESC SURFACE_DESC;

	texture->GetLevelDesc(0, &SURFACE_DESC);

	if (SURFACE_DESC.Width == SURFACE_DESC.Height) {
		for (int i = 0; i < numTexturePools; i++) {
			if (SURFACE_DESC.Width == aTexturePools[i].GetSize() && SURFACE_DESC.Format == aTexturePools[i].Format && levels == aTexturePools[i].levels) {
				if (!aTexturePools[i].AddTexture(texture)) {
					if (aTexturePools[i].texturesUsed > 3 * aTexturePools[i].texturesMax / 2) {
						aTexturePools[i].Resize(2 * aTexturePools[i].texturesMax);
						aTexturePools[i].texturesUsed--;
						aTexturePools[i].AddTexture(texture);
					} else {
						texture->Release();
					}
				}
				return;
			}
		}
	}
	if (numTexturePools < 12 && bUsePaletteIndex && levels != 0 && SURFACE_DESC.Width == SURFACE_DESC.Height &&
	    (SURFACE_DESC.Width == 64 || SURFACE_DESC.Width == 128 || SURFACE_DESC.Width == 256)) {
		aTexturePools[numTexturePools].Create(SURFACE_DESC.Format, SURFACE_DESC.Width, 1, 16);
		aTexturePools[numTexturePools].AddTexture(texture);
		numTexturePools++;
	} else
		texture->Release();
}

int
FindAvailablePaletteIndex()
{
	int index = PaletteList.Find();
	if (index == -1)
		index = MaxPaletteIndex++;
	return index;
}

void
AddAvailablePaletteIndex(int index)
{
	if (bUsePaletteIndex)
		PaletteList.Add(index);
}

void
_TexturePoolsInitialise()
{
	PaletteList.Alloc(100);
	MaxPaletteIndex = 0;
}

void
_TexturePoolsShutdown()
{
	for (int i = 0; i < numTexturePools; i++)
		aTexturePools[i].Release();

	numTexturePools = 0;
	bUsePaletteIndex = false;
	PaletteList.Free();
}

#endif // !LIBRW