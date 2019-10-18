#pragma once

class CSkidmarks
{
public:
	static void Clear(void);
	static void Update(void);
	static void Render(void);
	static void RegisterOne(uint32 id, CVector pos, float fwdx, float fwdY, bool *isMuddy, bool *isBloddy);
};
