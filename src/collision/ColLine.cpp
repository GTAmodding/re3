#include "common.h"
#include "ColLine.h"

void
CColLine::Set(const CVector &p0, const CVector &p1)
{
	this->p0 = p0;
	this->p1 = p1;
}