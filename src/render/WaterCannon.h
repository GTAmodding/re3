#pragma once

class CWaterCannons
{
public:
	static void Update(void);
	static void UpdateOne(uint32 id, CVector *pos, CVector *dir);
	static void Render(void);
};
