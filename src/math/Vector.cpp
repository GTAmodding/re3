#include "common.h"

void
CVector::Normalise(void)
{
	float sq = MagnitudeSqr();
	if (sq > 0.0f) {
		float invsqrt = RecipSqrt(sq);
		x *= invsqrt;
		y *= invsqrt;
		z *= invsqrt;
	} else
		x = 1.0f;
}

CVector
CrossProduct(const CVector &v1, const CVector &v2)
{
	return CVector(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

CVector
Multiply3x3(const CMatrix &mat, const CVector &vec)
{
	// TODO: VU0 code
	return CVector(mat.m_matrix.right.x * vec.x + mat.m_matrix.up.x * vec.y + mat.m_matrix.at.x * vec.z,
	               mat.m_matrix.right.y * vec.x + mat.m_matrix.up.y * vec.y + mat.m_matrix.at.y * vec.z,
	               mat.m_matrix.right.z * vec.x + mat.m_matrix.up.z * vec.y + mat.m_matrix.at.z * vec.z);
}

CVector
Multiply3x3(const RwMatrix &mat, const CVector &vec)
{
	// TODO: VU0 code
	return CVector(mat.right.x * vec.x + mat.up.x * vec.y + mat.at.x * vec.z,
	               mat.right.y * vec.x + mat.up.y * vec.y + mat.at.y * vec.z,
	               mat.right.z * vec.x + mat.up.z * vec.y + mat.at.z * vec.z);
}

CVector
Multiply3x3(const CVector &vec, const CMatrix &mat)
{
	return CVector(mat.m_matrix.right.x * vec.x + mat.m_matrix.right.y * vec.y + mat.m_matrix.right.z * vec.z,
	               mat.m_matrix.up.x * vec.x + mat.m_matrix.up.y * vec.y + mat.m_matrix.up.z * vec.z,
	               mat.m_matrix.at.x * vec.x + mat.m_matrix.at.y * vec.y + mat.m_matrix.at.z * vec.z);
}

CVector
operator*(const CMatrix &mat, const CVector &vec)
{
	// TODO: VU0 code
	return CVector(mat.m_matrix.right.x * vec.x + mat.m_matrix.up.x * vec.y + mat.m_matrix.at.x * vec.z + mat.m_matrix.pos.x,
	               mat.m_matrix.right.y * vec.x + mat.m_matrix.up.y * vec.y + mat.m_matrix.at.y * vec.z + mat.m_matrix.pos.y,
	               mat.m_matrix.right.z * vec.x + mat.m_matrix.up.z * vec.y + mat.m_matrix.at.z * vec.z + mat.m_matrix.pos.z);
}

CVector
operator*(const RwMatrix &mat, const CVector &vec)
{
	// TODO: VU0 code
	return CVector(mat.right.x * vec.x + mat.up.x * vec.y + mat.at.x * vec.z + mat.pos.x,
	               mat.right.y * vec.x + mat.up.y * vec.y + mat.at.y * vec.z + mat.pos.y,
	               mat.right.z * vec.x + mat.up.z * vec.y + mat.at.z * vec.z + mat.pos.z);
}
