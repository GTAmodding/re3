class RenderBuffer
{
public:
	static int VerticesToBeStored;
	static int IndicesToBeStored;
	static void ClearRenderBuffer(void);
	static void StartStoring(int numIndices, int numVertices, RwImVertexIndex **indexStart, RwIm3DVertex **vertexStart);
	static void StopStoring(void);
	static void RenderStuffInBuffer(void);
};

#define TEMPBUFFERVERTSIZE 256
#define TEMPBUFFERINDEXSIZE 1024

extern int32 TempBufferVerticesStored;
extern int32 TempBufferIndicesStored;
extern RwIm3DVertex TempBufferRenderVertices[TEMPBUFFERVERTSIZE];
extern RwImVertexIndex TempBufferRenderIndexList[TEMPBUFFERINDEXSIZE];