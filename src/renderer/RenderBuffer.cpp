#include "common.h"

#include "RenderBuffer.h"

int32 TempBufferVerticesStored;
int32 TempBufferIndicesStored;

RwIm3DVertex TempBufferRenderVertices[TEMPBUFFERVERTSIZE];
RwImVertexIndex TempBufferRenderIndexList[TEMPBUFFERINDEXSIZE];

int RenderBuffer::VerticesToBeStored;
int RenderBuffer::IndicesToBeStored;

void
RenderBuffer::ClearRenderBuffer(void)
{
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
}

void
RenderBuffer::StartStoring(int numIndices, int numVertices, RwImVertexIndex **indexStart, RwIm3DVertex **vertexStart)
{
	if(TempBufferIndicesStored + numIndices >= TEMPBUFFERINDEXSIZE)
		RenderStuffInBuffer();
	if(TempBufferVerticesStored + numVertices >= TEMPBUFFERVERTSIZE)
		RenderStuffInBuffer();
        *indexStart = &TempBufferRenderIndexList[TempBufferIndicesStored];
        *vertexStart = &TempBufferRenderVertices[TempBufferVerticesStored];
        IndicesToBeStored = numIndices;
        VerticesToBeStored = numVertices;
}

void
RenderBuffer::StopStoring(void)
{
	int i;
	for(i = TempBufferIndicesStored; i < TempBufferIndicesStored+IndicesToBeStored; i++)
		TempBufferRenderIndexList[i] += TempBufferVerticesStored;
	TempBufferIndicesStored += IndicesToBeStored;
	TempBufferVerticesStored += VerticesToBeStored;
}

void
RenderBuffer::RenderStuffInBuffer(void)
{
	if(TempBufferVerticesStored && RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, rwIM3D_VERTEXUV)){
		RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
		RwIm3DEnd();
	}
	ClearRenderBuffer();
}
