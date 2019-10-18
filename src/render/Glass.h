#pragma once

class CEntity;

class CGlass
{
public:
	static void AskForObjectToBeRenderedInGlass(CEntity *ent);
	static void WindowRespondsToCollision(CEntity *ent, float amount, CVector speed, CVector point, bool foo);
	static void WindowRespondsToSoftCollision(CEntity *ent, float amount);
	static void Render(void);
	static void Update(void);
};
