#include "common.h"
#include "Placeable.h"


CPlaceable::CPlaceable(void)
{
	m_matrix.SetScale(1.0f);
}

void
CPlaceable::SetHeading(float angle)
{
	CVector pos = GetMatrix().GetPosition();
	m_matrix.SetRotateZ(angle);
	GetMatrix().Translate(pos);
}

bool
CPlaceable::IsWithinArea(float x1, float y1, float x2, float y2)
{
	float tmp;

	if(x1 > x2){
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if(y1 > y2){
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	return x1 <= GetPosition().x && GetPosition().x <= x2 &&
	       y1 <= GetPosition().y && GetPosition().y <= y2;
}

bool
CPlaceable::IsWithinArea(float x1, float y1, float z1, float x2, float y2, float z2)
{
	float tmp;

	if(x1 > x2){
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if(y1 > y2){
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	if(z1 > z2){
		tmp = z1;
		z1 = z2;
		z2 = tmp;
	}

	return x1 <= GetPosition().x && GetPosition().x <= x2 &&
	       y1 <= GetPosition().y && GetPosition().y <= y2 &&
	       z1 <= GetPosition().z && GetPosition().z <= z2;
}
