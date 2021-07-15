#pragma once
#include "common.h"
#include "Vector.h"

class CMovingThing
{
public:
	CMovingThing *m_pNext;
	CMovingThing *m_pPrev;
	int16 m_nType;
	int16 m_nHidden;
	CVector m_vecPosn;
	CEntity* m_pEntity;

	void Update();
	void AddToList(CMovingThing *pThing);
	void RemoveFromList();
	int16 SizeList();
};

#define NUMMOVINGTHINGS 128

class CMovingThings
{
public:
	static CMovingThing StartCloseList;
	static CMovingThing EndCloseList;
	static int16 Num;
	static CMovingThing aMovingThings[NUMMOVINGTHINGS];

	static void Init();
	static void Shutdown();
	static void Update();
	static void Render();
};

class CScrollBar
{
private:
	uint8       m_Counter;
	const char* m_pMessage;
	CVector     m_Position;
	uint32      m_MessageCurrentChar;
	uint32      m_MessageLength;
	CVector     m_Size;
	float       m_fIntensity;
	uint8       m_MessageBar[40];
	uint8       m_Type;
	bool        m_bVisible;
	uint8       m_uRed;
	uint8       m_uGreen;
	uint8       m_uBlue;
	float       m_fScale;

public:
	void SetVisibility(bool visible) { m_bVisible = visible; }
	bool IsVisible() { return m_bVisible; }

	void Init(CVector, uint8, float, float, float, uint8, uint8, uint8, float);
	void Update();
	void Render();
};

class CTowerClock
{
private:
	CVector m_Position;
	CVector m_Size;
	float   m_fDrawDistance;
	float   m_fScale;
	uint8   m_uRed;
	uint8   m_uGreen;
	uint8   m_uBlue;
	bool    m_bVisible;
	float   m_fIntensity;

public:
	void SetVisibility(bool visible) { m_bVisible = visible; }
	bool IsVisible() { return m_bVisible; }

	void Init(CVector, float, float, uint8, uint8, uint8, float, float);
	void Update();
	void Render();
};

class CDigitalClock
{
private:
	CVector m_Position;
	CVector m_Size;
	float   m_fDrawDistance;
	float   m_fScale;
	uint8   m_uRed;
	uint8   m_uGreen;
	uint8   m_uBlue;
	bool    m_bVisible;
	float   m_fIntensity;

public:
	void SetVisibility(bool visible) { m_bVisible = visible; }
	bool IsVisible() { return m_bVisible; }

	void Init(CVector, float, float, uint8, uint8, uint8, float, float);
	void Update();
	void Render();
};