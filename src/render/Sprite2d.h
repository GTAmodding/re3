#pragma once

class CSprite2d
{
	static float RecipNearClip;
	static float NearScreenZ;
	static int nextBufferVertex;
	static int nextBufferIndex;
	static RwIm2DVertex maVertices[8];
public:
	RwTexture *m_pTexture;

	static void SetRecipNearClip(void);
	static void InitPerFrame(void);

	CSprite2d(void) : m_pTexture(nil) {};
	~CSprite2d(void) { Delete(); };
	void Delete(void);
	void SetRenderState(void);
	void SetTexture(const char *name);
	void SetTexture(const char *name, const char *mask);
	void SetAddressing(RwTextureAddressMode addr);
	void Draw(float x, float y, float w, float h, const CRGBA &col);
	void Draw(const CRect &rect, const CRGBA &col);
	void Draw(const CRect &rect, const CRGBA &col,
		float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2);
	void Draw(const CRect &rect, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3);
	void Draw(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, const CRGBA &col);

	static void SetVertices(const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3, uint32 far);
	static void SetVertices(const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3,
		float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2);
	static void SetVertices(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
		const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3);
	static void SetVertices(int n, float *positions, float *uvs, const CRGBA &col);
	static void SetMaskVertices(int n, float *positions);
	static void SetVertices(RwIm2DVertex *verts, const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3,
		float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2);

	static void DrawRect(const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3);
	static void DrawRect(const CRect &r, const CRGBA &col);
	static void DrawRectXLU(const CRect &r, const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3);
	static void DrawAnyRect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
		const CRGBA &c0, const CRGBA &c1, const CRGBA &c2, const CRGBA &c3);

	static void Draw2DPolygon(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, const CRGBA &color);

	static RwIm2DVertex* GetVertices() { return maVertices; };

	static bool IsVertexBufferFull();
	static void AddToBuffer(const CRect &a1, const CRGBA &a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9, float a10);
	static void RenderVertexBuffer();
};
