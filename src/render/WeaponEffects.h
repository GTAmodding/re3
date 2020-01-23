#pragma once

class CWeaponEffects
{
public:
	bool m_bCrosshair;
	int8 gap_1[3];
	CVector m_vecPos;
	uint8 m_red;
	uint8 m_green;
	uint8 m_blue;
	uint8 m_alpha;
	float m_size;
	int32 field_24;
	RwTexture *m_pTexture;
	RwRaster *m_pRaster;

public:
	static void Render(void);
	static void ClearCrossHair();
	static void MarkTarget(CVector, uint8, uint8, uint8, uint8, float);
};
