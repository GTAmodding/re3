#pragma once

enum { SKIDMARK_LENGTH = 16 };

enum eSkidmarkType
{
	SKIDMARK_NORMAL,
	SKIDMARK_MUDDY,
	SKIDMARK_SANDY,
	SKIDMARK_BLOODY
};

class CSkidmark
{
public:
	CVector m_pos[SKIDMARK_LENGTH];
	float m_sideX[SKIDMARK_LENGTH];
	float m_sideY[SKIDMARK_LENGTH];
	uintptr m_id;
	uint32 m_lastUpdate;
	uint32 m_fadeStart;
	uint32 m_fadeEnd;
	uint32 m_type;
	int16 m_last;
	uint8 m_state;
	bool m_wasUpdated;
};

class CSkidmarks
{
	static CSkidmark aSkidmarks[NUMSKIDMARKS];
public:

	static void Init(void);
	static void Shutdown(void);
	static void Clear(void);
	static void Update(void);
	static void Render(void);
	static void RegisterOne(uintptr id, const CVector &pos, float fwdX, float fwdY, eSkidmarkType type, bool *isBloody);
	static void RegisterOne(uintptr id, const CVector &pos, float fwdX, float fwdY, bool *isMuddy, bool *isBloody);
};
