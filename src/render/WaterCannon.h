#pragma once

class CWaterCannon
{
public:
	int32 m_nId;
	int16 m_wIndex;
	char gap_6[2];
	int32 m_nTimeCreated;
	CVector m_avecPos[16];
	CVector m_avecVelocity[16];
	char m_abUsed[16];

	static void Update(void);
	static void UpdateOne(uint32 id, CVector *pos, CVector *dir);
	static void Render(void);
};

static_assert(sizeof(CWaterCannon) == 412, "CWaterCannon: error");

extern CWaterCannon* aCannons;
