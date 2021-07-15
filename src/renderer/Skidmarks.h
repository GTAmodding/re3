#pragma once

enum { SKIDMARK_LENGTH = 16 };

class CSkidmark
{
public:
	uint8 m_state;
	bool m_wasUpdated;
	bool m_isBloody;
	bool m_isMuddy;
	uintptr m_id;
	int16 m_last;
	uint32 m_lastUpdate;
	uint32 m_fadeStart;
	uint32 m_fadeEnd;
	CVector m_pos[SKIDMARK_LENGTH];
	CVector m_side[SKIDMARK_LENGTH];
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
	static void RegisterOne(uintptr id, CVector pos, float fwdX, float fwdY, bool *isMuddy, bool *isBloody);
};
