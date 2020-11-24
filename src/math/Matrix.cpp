#include "common.h"

extern float timeAlpha;

CMatrix::CMatrix(void)
{
	m_attachment = nil;
	m_hasRwMatrix = false;
}

CMatrix::CMatrix(CMatrix const &m)
{
	m_attachment = nil;
	m_hasRwMatrix = false;
	*this = m;
}

CMatrix::CMatrix(RwMatrix *matrix, bool owner)
{
	m_attachment = nil;
	Attach(matrix, owner);
}

CMatrix::~CMatrix(void)
{
	if (m_hasRwMatrix && m_attachment)
		RwMatrixDestroy(m_attachment);
}

void
CMatrix::Attach(RwMatrix *matrix, bool owner)
{
#ifdef FIX_BUGS
	if (m_attachment && m_hasRwMatrix)
#else
	if (m_hasRwMatrix && m_attachment)
#endif
		RwMatrixDestroy(m_attachment);
	m_attachment = matrix;
	m_hasRwMatrix = owner;
	Update();
}

void
CMatrix::AttachRW(RwMatrix *matrix, bool owner)
{
	if (m_hasRwMatrix && m_attachment)
		RwMatrixDestroy(m_attachment);
	m_attachment = matrix;
	m_hasRwMatrix = owner;
	UpdateRW();
}

void
CMatrix::Detach(void)
{
	if (m_hasRwMatrix && m_attachment)
		RwMatrixDestroy(m_attachment);
	m_attachment = nil;
}

void
CMatrix::Update(void)
{
	m_matrix = *m_attachment;
}

void
CMatrix::UpdateRW(void)
{
	if(m_attachment) {
		*m_attachment = m_matrix;
		RwMatrixUpdate(m_attachment);
	}
}

void
CMatrix::UpdateRWInterPolated(void)
{
	if(m_attachment) {
		*m_attachment = CreateMatrixInterpolated();
		RwMatrixUpdate(m_attachment);
	}
}

void
CMatrix::operator=(CMatrix const &rhs)
{
	m_matrix = rhs.m_matrix;
}

void
CMatrix::save(CMatrix const &rhs)
{
	m_matrix = rhs.m_matrix;
}

void
CMatrix::CopyOnlyMatrix(CMatrix *other)
{
	m_matrix = other->m_matrix;
}

CMatrix &
CMatrix::operator+=(CMatrix const &rhs)
{
	m_matrix.right.x += rhs.m_matrix.right.x;
	m_matrix.up.x += rhs.m_matrix.up.x;
	m_matrix.at.x += rhs.m_matrix.at.x;
	m_matrix.right.y += rhs.m_matrix.right.y;
	m_matrix.up.y += rhs.m_matrix.up.y;
	m_matrix.at.y += rhs.m_matrix.at.y;
	m_matrix.right.z += rhs.m_matrix.right.z;
	m_matrix.up.z += rhs.m_matrix.up.z;
	m_matrix.at.z += rhs.m_matrix.at.z;
	m_matrix.pos.x += rhs.m_matrix.pos.x;
	m_matrix.pos.y += rhs.m_matrix.pos.y;
	m_matrix.pos.z += rhs.m_matrix.pos.z;
	return *this;
}

void
CMatrix::SetUnity(void)
{
	m_matrix.right.x = 1.0f;
	m_matrix.right.y = 0.0f;
	m_matrix.right.z = 0.0f;
	m_matrix.up.x = 0.0f;
	m_matrix.up.y = 1.0f;
	m_matrix.up.z = 0.0f;
	m_matrix.at.x = 0.0f;
	m_matrix.at.y = 0.0f;
	m_matrix.at.z = 1.0f;
	m_matrix.pos.x = 0.0f;
	m_matrix.pos.y = 0.0f;
	m_matrix.pos.z = 0.0f;
}

void
CMatrix::ResetOrientation(void)
{
	m_matrix.right.x = 1.0f;
	m_matrix.right.y = 0.0f;
	m_matrix.right.z = 0.0f;
	m_matrix.up.x = 0.0f;
	m_matrix.up.y = 1.0f;
	m_matrix.up.z = 0.0f;
	m_matrix.at.x = 0.0f;
	m_matrix.at.y = 0.0f;
	m_matrix.at.z = 1.0f;
}

void
CMatrix::SetScale(float s)
{
	m_matrix.right.x = s;
	m_matrix.right.y = 0.0f;
	m_matrix.right.z = 0.0f;

	m_matrix.up.x = 0.0f;
	m_matrix.up.y = s;
	m_matrix.up.z = 0.0f;

	m_matrix.at.x = 0.0f;
	m_matrix.at.y = 0.0f;
	m_matrix.at.z = s;

	m_matrix.pos.x = 0.0f;
	m_matrix.pos.y = 0.0f;
	m_matrix.pos.z = 0.0f;
}

void
CMatrix::SetTranslate(float x, float y, float z)
{
	m_matrix.right.x = 1.0f;
	m_matrix.right.y = 0.0f;
	m_matrix.right.z = 0.0f;

	m_matrix.up.x = 0.0f;
	m_matrix.up.y = 1.0f;
	m_matrix.up.z = 0.0f;

	m_matrix.at.x = 0.0f;
	m_matrix.at.y = 0.0f;
	m_matrix.at.z = 1.0f;

	m_matrix.pos.x = x;
	m_matrix.pos.y = y;
	m_matrix.pos.z = z;
}

void
CMatrix::SetRotateXOnly(float angle)
{
	float c = Cos(angle);
	float s = Sin(angle);

	m_matrix.right.x = 1.0f;
	m_matrix.right.y = 0.0f;
	m_matrix.right.z = 0.0f;

	m_matrix.up.x = 0.0f;
	m_matrix.up.y = c;
	m_matrix.up.z = s;

	m_matrix.at.x = 0.0f;
	m_matrix.at.y = -s;
	m_matrix.at.z = c;
}

void
CMatrix::SetRotateYOnly(float angle)
{
	float c = Cos(angle);
	float s = Sin(angle);

	m_matrix.right.x = c;
	m_matrix.right.y = 0.0f;
	m_matrix.right.z = -s;

	m_matrix.up.x = 0.0f;
	m_matrix.up.y = 1.0f;
	m_matrix.up.z = 0.0f;

	m_matrix.at.x = s;
	m_matrix.at.y = 0.0f;
	m_matrix.at.z = c;
}

void
CMatrix::SetRotateZOnly(float angle)
{
	float c = Cos(angle);
	float s = Sin(angle);

	m_matrix.right.x = c;
	m_matrix.right.y = s;
	m_matrix.right.z = 0.0f;

	m_matrix.up.x = -s;
	m_matrix.up.y = c;
	m_matrix.up.z = 0.0f;

	m_matrix.at.x = 0.0f;
	m_matrix.at.y = 0.0f;
	m_matrix.at.z = 1.0f;
}

void
CMatrix::SetRotateX(float angle)
{
	SetRotateXOnly(angle);
	m_matrix.pos.x = 0.0f;
	m_matrix.pos.y = 0.0f;
	m_matrix.pos.z = 0.0f;
}


void
CMatrix::SetRotateY(float angle)
{
	SetRotateYOnly(angle);
	m_matrix.pos.x = 0.0f;
	m_matrix.pos.y = 0.0f;
	m_matrix.pos.z = 0.0f;
}

void
CMatrix::SetRotateZ(float angle)
{
	SetRotateZOnly(angle);
	m_matrix.pos.x = 0.0f;
	m_matrix.pos.y = 0.0f;
	m_matrix.pos.z = 0.0f;
}

void
CMatrix::SetRotate(float xAngle, float yAngle, float zAngle)
{
	float cX = Cos(xAngle);
	float sX = Sin(xAngle);
	float cY = Cos(yAngle);
	float sY = Sin(yAngle);
	float cZ = Cos(zAngle);
	float sZ = Sin(zAngle);

	m_matrix.right.x = cZ * cY - (sZ * sX) * sY;
	m_matrix.right.y = (cZ * sX) * sY + sZ * cY;
	m_matrix.right.z = -cX * sY;

	m_matrix.up.x = -sZ * cX;
	m_matrix.up.y = cZ * cX;
	m_matrix.up.z = sX;

	m_matrix.at.x = (sZ * sX) * cY + cZ * sY;
	m_matrix.at.y = sZ * sY - (cZ * sX) * cY;
	m_matrix.at.z = cX * cY;

	m_matrix.pos.x = 0.0f;
	m_matrix.pos.y = 0.0f;
	m_matrix.pos.z = 0.0f;
}

void
CMatrix::RotateX(float x)
{
	float c = Cos(x);
	float s = Sin(x);

	float ry = m_matrix.right.y;
	float rz = m_matrix.right.z;
	float uy = m_matrix.up.y;
	float uz = m_matrix.up.z;
	float ay = m_matrix.at.y;
	float az = m_matrix.at.z;
	float py = m_matrix.pos.y;
	float pz = m_matrix.pos.z;

	m_matrix.right.y = c * ry - s * rz;
	m_matrix.right.z = c * rz + s * ry;
	m_matrix.up.y = c * uy - s * uz;
	m_matrix.up.z = c * uz + s * uy;
	m_matrix.at.y = c * ay - s * az;
	m_matrix.at.z = c * az + s * ay;
	m_matrix.pos.y = c * py - s * pz;
	m_matrix.pos.z = c * pz + s * py;
}

void
CMatrix::RotateY(float y)
{
	float c = Cos(y);
	float s = Sin(y);

	float rx = m_matrix.right.x;
	float rz = m_matrix.right.z;
	float ux = m_matrix.up.x;
	float uz = m_matrix.up.z;
	float ax = m_matrix.at.x;
	float az = m_matrix.at.z;
	float px = m_matrix.pos.x;
	float pz = m_matrix.pos.z;

	m_matrix.right.x = c * rx + s * rz;
	m_matrix.right.z = c * rz - s * rx;
	m_matrix.up.x = c * ux + s * uz;
	m_matrix.up.z = c * uz - s * ux;
	m_matrix.at.x = c * ax + s * az;
	m_matrix.at.z = c * az - s * ax;
	m_matrix.pos.x = c * px + s * pz;
	m_matrix.pos.z = c * pz - s * px;
}

void
CMatrix::RotateZ(float z)
{
	float c = Cos(z);
	float s = Sin(z);

	float ry = m_matrix.right.y;
	float rx = m_matrix.right.x;
	float uy = m_matrix.up.y;
	float ux = m_matrix.up.x;
	float ay = m_matrix.at.y;
	float ax = m_matrix.at.x;
	float py = m_matrix.pos.y;
	float px = m_matrix.pos.x;

	m_matrix.right.x = c * rx - s * ry;
	m_matrix.right.y = c * ry + s * rx;
	m_matrix.up.x = c * ux - s * uy;
	m_matrix.up.y = c * uy + s * ux;
	m_matrix.at.x = c * ax - s * ay;
	m_matrix.at.y = c * ay + s * ax;
	m_matrix.pos.x = c * px - s * py;
	m_matrix.pos.y = c * py + s * px;

}

void
CMatrix::Rotate(float x, float y, float z)
{
	float cX = Cos(x);
	float sX = Sin(x);
	float cY = Cos(y);
	float sY = Sin(y);
	float cZ = Cos(z);
	float sZ = Sin(z);
	
	float rx = m_matrix.right.x;
	float ry = m_matrix.right.y;
	float rz = m_matrix.right.z;
	float ux = m_matrix.up.x;
	float uy = m_matrix.up.y;
	float uz = m_matrix.up.z;
	float ax = m_matrix.at.x;
	float ay = m_matrix.at.y;
	float az = m_matrix.at.z;
	float px = m_matrix.pos.x;
	float py = m_matrix.pos.y;
	float pz = m_matrix.pos.z;

	float x1 = cZ * cY - (sZ * sX) * sY;
	float x2 = (cZ * sX) * sY + sZ * cY;
	float x3 = -cX * sY;
	float y1 = -sZ * cX;
	float y2 = cZ * cX;
	float y3 = sX;
	float z1 = (sZ * sX) * cY + cZ * sY;
	float z2 = sZ * sY - (cZ * sX) * cY;
	float z3 = cX * cY;

	m_matrix.right.x = x1 * rx + y1 * ry + z1 * rz;
	m_matrix.right.y = x2 * rx + y2 * ry + z2 * rz;
	m_matrix.right.z = x3 * rx + y3 * ry + z3 * rz;
	m_matrix.up.x = x1 * ux + y1 * uy + z1 * uz;
	m_matrix.up.y = x2 * ux + y2 * uy + z2 * uz;
	m_matrix.up.z = x3 * ux + y3 * uy + z3 * uz;
	m_matrix.at.x = x1 * ax + y1 * ay + z1 * az;
	m_matrix.at.y = x2 * ax + y2 * ay + z2 * az;
	m_matrix.at.z = x3 * ax + y3 * ay + z3 * az;
	m_matrix.pos.x = x1 * px + y1 * py + z1 * pz;
	m_matrix.pos.y = x2 * px + y2 * py + z2 * pz;
	m_matrix.pos.z = x3 * px + y3 * py + z3 * pz;
}

CMatrix &
CMatrix::operator*=(CMatrix const &rhs)
{
	// TODO: VU0 code
	*this = *this * rhs;
	return *this;
}

void
CMatrix::SaveOldMatrix()
{
	m_matrix_old = m_matrix;
}

RwMatrix
CMatrix::CreateMatrixInterpolated()
{
	RwMatrix m_matrix_interpolated;
	if(*(CVector *)&m_matrix_old.pos != CVector(0, 0, 0))
		m_matrix_interpolated = InterpolateMatrix(m_matrix_old, m_matrix, timeAlpha);
	else
		m_matrix_interpolated = m_matrix;
	return m_matrix_interpolated;
}

CVector &
CMatrix::GetPositionInterpolated()
{
	return *(CVector*)RwMatrixGetPos(m_attachment);
}

CVector &
CMatrix::GetRightInterpolated()
{
	return *(CVector *)RwMatrixGetRight(m_attachment);
}

CVector &
CMatrix::GetForwardInterpolated()
{
	return *(CVector *)RwMatrixGetUp(m_attachment);
}

CVector &
CMatrix::GetUpInterpolated()
{
	return *(CVector *)RwMatrixGetAt(m_attachment);
}

RwMatrix &
CMatrix::GetMatrixInterpolated()
{
	return *m_attachment;
}

void
CMatrix::Reorthogonalise(void) const
{
	CVector &r = *(CVector *)&m_matrix.right;
	CVector &f = *(CVector *)&m_matrix.up;
	CVector &u = *(CVector *)&m_matrix.at;
	u = CrossProduct(r, f);
	u.Normalise();
	r = CrossProduct(f, u);
	r.Normalise();
	f = CrossProduct(u, r);
}

CMatrix
operator*(const CMatrix &m1, const CMatrix &m2)
{
	// TODO: VU0 code
	CMatrix out;
	RwMatrix *dst = &out.m_matrix;
	const RwMatrix *src1 = &m1.m_matrix;
	const RwMatrix *src2 = &m2.m_matrix;
	dst->right.x = src1->right.x * src2->right.x + src1->up.x * src2->right.y + src1->at.x * src2->right.z;
	dst->right.y = src1->right.y * src2->right.x + src1->up.y * src2->right.y + src1->at.y * src2->right.z;
	dst->right.z = src1->right.z * src2->right.x + src1->up.z * src2->right.y + src1->at.z * src2->right.z;
	dst->up.x = src1->right.x * src2->up.x + src1->up.x * src2->up.y + src1->at.x * src2->up.z;
	dst->up.y = src1->right.y * src2->up.x + src1->up.y * src2->up.y + src1->at.y * src2->up.z;
	dst->up.z = src1->right.z * src2->up.x + src1->up.z * src2->up.y + src1->at.z * src2->up.z;
	dst->at.x = src1->right.x * src2->at.x + src1->up.x * src2->at.y + src1->at.x * src2->at.z;
	dst->at.y = src1->right.y * src2->at.x + src1->up.y * src2->at.y + src1->at.y * src2->at.z;
	dst->at.z = src1->right.z * src2->at.x + src1->up.z * src2->at.y + src1->at.z * src2->at.z;
	dst->pos.x = src1->right.x * src2->pos.x + src1->up.x * src2->pos.y + src1->at.x * src2->pos.z + src1->pos.x;
	dst->pos.y = src1->right.y * src2->pos.x + src1->up.y * src2->pos.y + src1->at.y * src2->pos.z + src1->pos.y;
	dst->pos.z = src1->right.z * src2->pos.x + src1->up.z * src2->pos.y + src1->at.z * src2->pos.z + src1->pos.z;
	return out;
}

CMatrix &
Invert(const CMatrix &src, CMatrix &dst)
{
	// TODO: VU0 code
	// GTA handles this as a raw 4x4 orthonormal matrix
	// and trashes the RW flags, let's not do that
	float (*scr_fm)[4] = (float (*)[4])&src.m_matrix;
	float (*dst_fm)[4] = (float (*)[4])&dst.m_matrix;

	dst_fm[3][0] = dst_fm[3][1] = dst_fm[3][2] = 0.0f;
#ifndef FIX_BUGS
	dst_fm[3][3] = scr_fm[3][3];
#endif

	dst_fm[0][0] = scr_fm[0][0];
	dst_fm[0][1] = scr_fm[1][0];
	dst_fm[0][2] = scr_fm[2][0];
#ifndef FIX_BUGS
	dst_fm[0][3] = scr_fm[3][0];
#endif
	dst_fm[1][0] = scr_fm[0][1];
	dst_fm[1][1] = scr_fm[1][1];
	dst_fm[1][2] = scr_fm[2][1];
#ifndef FIX_BUGS
	dst_fm[1][3] = scr_fm[3][1];
#endif
	dst_fm[2][0] = scr_fm[0][2];
	dst_fm[2][1] = scr_fm[1][2];
	dst_fm[2][2] = scr_fm[2][2];
#ifndef FIX_BUGS
	dst_fm[2][3] = scr_fm[3][2];
#endif

	dst_fm[3][0] += dst_fm[0][0] * scr_fm[3][0];
	dst_fm[3][1] += dst_fm[0][1] * scr_fm[3][0];
	dst_fm[3][2] += dst_fm[0][2] * scr_fm[3][0];
#ifndef FIX_BUGS
	dst_fm[3][3] += dst_fm[0][3] * scr_fm[3][0];
#endif

	dst_fm[3][0] += dst_fm[1][0] * scr_fm[3][1];
	dst_fm[3][1] += dst_fm[1][1] * scr_fm[3][1];
	dst_fm[3][2] += dst_fm[1][2] * scr_fm[3][1];
#ifndef FIX_BUGS
	dst_fm[3][3] += dst_fm[1][3] * scr_fm[3][1];
#endif

	dst_fm[3][0] += dst_fm[2][0] * scr_fm[3][2];
	dst_fm[3][1] += dst_fm[2][1] * scr_fm[3][2];
	dst_fm[3][2] += dst_fm[2][2] * scr_fm[3][2];
#ifndef FIX_BUGS
	dst_fm[3][3] += dst_fm[2][3] * scr_fm[3][2];
#endif

	dst_fm[3][0] = -dst_fm[3][0];
	dst_fm[3][1] = -dst_fm[3][1];
	dst_fm[3][2] = -dst_fm[3][2];
#ifndef FIX_BUGS
	dst_fm[3][3] = scr_fm[3][3] - dst_fm[3][3];
#endif

	return dst;
}

RwMatrix
Mult(const RwMatrix &src1_, const RwMatrix &src2_)
{
	RwMatrix result;
	RwMatrix *dst = &result;
	const RwMatrix *src1 = &src1_;
	const RwMatrix *src2 = &src2_;
	dst->right.x = src1->right.x * src2->right.x + src1->up.x * src2->right.y + src1->at.x * src2->right.z;
	dst->right.y = src1->right.y * src2->right.x + src1->up.y * src2->right.y + src1->at.y * src2->right.z;
	dst->right.z = src1->right.z * src2->right.x + src1->up.z * src2->right.y + src1->at.z * src2->right.z;
	dst->up.x = src1->right.x * src2->up.x + src1->up.x * src2->up.y + src1->at.x * src2->up.z;
	dst->up.y = src1->right.y * src2->up.x + src1->up.y * src2->up.y + src1->at.y * src2->up.z;
	dst->up.z = src1->right.z * src2->up.x + src1->up.z * src2->up.y + src1->at.z * src2->up.z;
	dst->at.x = src1->right.x * src2->at.x + src1->up.x * src2->at.y + src1->at.x * src2->at.z;
	dst->at.y = src1->right.y * src2->at.x + src1->up.y * src2->at.y + src1->at.y * src2->at.z;
	dst->at.z = src1->right.z * src2->at.x + src1->up.z * src2->at.y + src1->at.z * src2->at.z;
	dst->pos.x = src1->right.x * src2->pos.x + src1->up.x * src2->pos.y + src1->at.x * src2->pos.z + src1->pos.x;
	dst->pos.y = src1->right.y * src2->pos.x + src1->up.y * src2->pos.y + src1->at.y * src2->pos.z + src1->pos.y;
	dst->pos.z = src1->right.z * src2->pos.x + src1->up.z * src2->pos.y + src1->at.z * src2->pos.z + src1->pos.z;
	return result;
}

RwMatrix
CreateMatrixWithScale(float s)
{
	RwMatrix m_matrix;
	m_matrix.right.x = s;
	m_matrix.right.y = 0.0f;
	m_matrix.right.z = 0.0f;

	m_matrix.up.x = 0.0f;
	m_matrix.up.y = s;
	m_matrix.up.z = 0.0f;

	m_matrix.at.x = 0.0f;
	m_matrix.at.y = 0.0f;
	m_matrix.at.z = s;

	m_matrix.pos.x = 0.0f;
	m_matrix.pos.y = 0.0f;
	m_matrix.pos.z = 0.0f;
	return m_matrix;
}

RwMatrix
Add(const RwMatrix &m1, const RwMatrix &m2)
{
	RwMatrix o;
	o.right.x = m1.right.x + m2.right.x;
	o.up.x = m1.up.x + m2.up.x;
	o.at.x = m1.at.x + m2.at.x;
	o.right.y = m1.right.y + m2.right.y;
	o.up.y = m1.up.y + m2.up.y;
	o.at.y = m1.at.y + m2.at.y;
	o.right.z = m1.right.z + m2.right.z;
	o.up.z = m1.up.z + m2.up.z;
	o.at.z = m1.at.z + m2.at.z;
	o.pos.x = m1.pos.x + m2.pos.x;
	o.pos.y = m1.pos.y + m2.pos.y;
	o.pos.z = m1.pos.z + m2.pos.z;
	return o;
}

RwMatrix
InterpolateMatrix(RwMatrix &prev, RwMatrix &now, float interpolation)
{
	RwMatrix result;
	result = Mult(prev, CreateMatrixWithScale(1.0f - interpolation));
	result.pos = result.pos * (1.f - interpolation);
	result = Add(result, Mult(CreateMatrixWithScale(interpolation), now));
	return result;
}

CMatrix
Invert(const CMatrix &matrix)
{
	CMatrix inv;
	return Invert(matrix, inv);
}

void
CCompressedMatrixNotAligned::CompressFromFullMatrix(CMatrix& other)
{
	m_rightX = 127.0f * other.GetRight().x;
	m_rightY = 127.0f * other.GetRight().y;
	m_rightZ = 127.0f * other.GetRight().z;
	m_upX = 127.0f * other.GetForward().x;
	m_upY = 127.0f * other.GetForward().y;
	m_upZ = 127.0f * other.GetForward().z;
	m_vecPos = other.GetPosition();
}

void
CCompressedMatrixNotAligned::DecompressIntoFullMatrix(CMatrix &other)
{
	other.GetRight().x = m_rightX / 127.0f;
	other.GetRight().y = m_rightY / 127.0f;
	other.GetRight().z = m_rightZ / 127.0f;
	other.GetForward().x = m_upX / 127.0f;
	other.GetForward().y = m_upY / 127.0f;
	other.GetForward().z = m_upZ / 127.0f;
	other.GetUp() = CrossProduct(other.GetRight(), other.GetForward());
	other.GetPosition() = m_vecPos;
	other.Reorthogonalise();
}