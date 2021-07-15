#pragma once

class CVehicle;

enum {
	// NB: not all values are allowed, check the code
#ifdef SQUEEZE_PERFORMANCE
	NUM_RUBBISH_SHEETS = 32
#else
	NUM_RUBBISH_SHEETS = 64
#endif
};

class COneSheet
{
public:
	CVector m_basePos;
	CVector m_animatedPos;
	float m_targetZ;
	int8 m_state;
	int8 m_animationType;
	uint32 m_moveStart;
	uint32 m_moveDuration;
	float m_animHeight;
	float m_xDist;
	float m_yDist;
	float m_angle;
	bool m_isVisible;
	bool m_targetIsVisible;
	COneSheet *m_next;
	COneSheet *m_prev;

	void AddToList(COneSheet *list);
	void RemoveFromList(void);
};

class CRubbish
{
	static bool bRubbishInvisible;
	static int RubbishVisibility;
	static COneSheet aSheets[NUM_RUBBISH_SHEETS];
	static COneSheet StartEmptyList;
	static COneSheet EndEmptyList;
	static COneSheet StartStaticsList;
	static COneSheet EndStaticsList;
	static COneSheet StartMoversList;
	static COneSheet EndMoversList;
public:
	static void Render(void);
	static void StirUp(CVehicle *veh);	// CAutomobile on PS2
	static void Update(void);
	static void SetVisibility(bool visible);
	static void Init(void);
	static void Shutdown(void);
};
