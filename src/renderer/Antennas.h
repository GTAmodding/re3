#pragma once

class CAntenna
{
public:
	bool active;
	bool updatedLastFrame;
	uint32 id;
	float segmentLength;
	CVector pos[6];
	CVector speed[6];

	void Update(CVector dir, CVector pos);
};

class CAntennas
{
	// no need to use game's array
	static CAntenna aAntennas[NUMANTENNAS];
public:
	static void Init(void);
	static void Update(void);
	static void RegisterOne(uint32 id, CVector dir, CVector position, float length);
	static void Render(void);
};
