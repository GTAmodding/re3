#pragma once

class CMatrix
{
public:
	RwMatrix m_matrix;
	RwMatrix *m_attachment;
	bool m_hasRwMatrix;	// are we the owner?

	CMatrix(void){
		m_attachment = nil;
		m_hasRwMatrix = false;
	}
	CMatrix(CMatrix const &m){
		m_attachment = nil;
		m_hasRwMatrix = false;
		*this = m;
	}
	CMatrix(RwMatrix *matrix, bool owner = false){
		m_attachment = nil;
		Attach(matrix, owner);
	}
	CMatrix(float scale){
		m_attachment = nil;
		m_hasRwMatrix = false;
		SetScale(scale);
	}
	~CMatrix(void){
		if(m_hasRwMatrix && m_attachment)
			RwMatrixDestroy(m_attachment);
	}
	void Attach(RwMatrix *matrix, bool owner = false){
		if(m_hasRwMatrix && m_attachment)
			RwMatrixDestroy(m_attachment);
		m_attachment = matrix;
		m_hasRwMatrix = owner;
		Update();
	}
	void AttachRW(RwMatrix *matrix, bool owner = false){
		if(m_hasRwMatrix && m_attachment)
			RwMatrixDestroy(m_attachment);
		m_attachment = matrix;
		m_hasRwMatrix = owner;
		UpdateRW();
	}
	void Detach(void){
		if(m_hasRwMatrix && m_attachment)
			RwMatrixDestroy(m_attachment);
		m_attachment = nil;
	}
	void Update(void){
		m_matrix = *m_attachment;
	}
	void UpdateRW(void){
		if(m_attachment){
			*m_attachment = m_matrix;
			RwMatrixUpdate(m_attachment);
		}
	}
	void operator=(CMatrix const &rhs){
		m_matrix = rhs.m_matrix;
		if(m_attachment)
			UpdateRW();
	}
	CMatrix& operator+=(CMatrix const &rhs){
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

	const CVector &GetPosition(void) const { return *(CVector*)&m_matrix.pos; }
	CVector& GetPosition(void) { return *(CVector*)&m_matrix.pos; }
	CVector &GetRight(void) { return *(CVector*)&m_matrix.right; }
	CVector &GetForward(void) { return *(CVector*)&m_matrix.up; }
	CVector &GetUp(void) { return *(CVector*)&m_matrix.at; }

	void SetTranslate(float x, float y, float z){
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
	void SetTranslate(const CVector &trans){ SetTranslate(trans.x, trans.y, trans.z); }
	void Translate(float x, float y, float z){
		m_matrix.pos.x += x;
		m_matrix.pos.y += y;
		m_matrix.pos.z += z;
	}
	void Translate(const CVector &trans){ Translate(trans.x, trans.y, trans.z); }

	void SetScale(float s){
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
	void Scale(float scale)
	{
		float *pFloatMatrix = (float*)&m_matrix;
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				pFloatMatrix[i * 4 + j] *= scale;
	}
	void Scale(float sx, float sy, float sz)
	{
		float *pFloatMatrix = (float*)&m_matrix;
		for (int i = 0; i < 3; i++){
			pFloatMatrix[i * 4 + 0] *= sx;
			pFloatMatrix[i * 4 + 1] *= sy;
			pFloatMatrix[i * 4 + 2] *= sz;
		}
	}


	void SetRotateXOnly(float angle){
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
	void SetRotateX(float angle){
		SetRotateXOnly(angle);
		m_matrix.pos.x = 0.0f;
		m_matrix.pos.y = 0.0f;
		m_matrix.pos.z = 0.0f;
	}
	void SetRotateYOnly(float angle){
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
	void SetRotateY(float angle){
		SetRotateYOnly(angle);
		m_matrix.pos.x = 0.0f;
		m_matrix.pos.y = 0.0f;
		m_matrix.pos.z = 0.0f;
	}
	void SetRotateZOnly(float angle){
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
	void SetRotateZOnlyScaled(float angle, float scale) {
		float c = Cos(angle);
		float s = Sin(angle);

		m_matrix.right.x = c * scale;
		m_matrix.right.y = s * scale;
		m_matrix.right.z = 0.0f;

		m_matrix.up.x = -s * scale;
		m_matrix.up.y = c * scale;
		m_matrix.up.z = 0.0f;

		m_matrix.at.x = 0.0f;
		m_matrix.at.y = 0.0f;
		m_matrix.at.z = scale;
	}
	void SetRotateZ(float angle){
		SetRotateZOnly(angle);
		m_matrix.pos.x = 0.0f;
		m_matrix.pos.y = 0.0f;
		m_matrix.pos.z = 0.0f;
	}
	void SetRotate(float xAngle, float yAngle, float zAngle);
	void Rotate(float x, float y, float z);
	void RotateX(float x);
	void RotateY(float y);
	void RotateZ(float z);

	void Reorthogonalise(void);
	void CopyOnlyMatrix(CMatrix *other){
		m_matrix = other->m_matrix;
	}
	void SetUnity(void) {
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
	void ResetOrientation(void) {
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
	void SetTranslateOnly(float x, float y, float z) {
		m_matrix.pos.x = x;
		m_matrix.pos.y = y;
		m_matrix.pos.z = z;
	}
	void SetTranslateOnly(const CVector& pos) {
		SetTranslateOnly(pos.x, pos.y, pos.z);
	}
};


CMatrix &Invert(const CMatrix &src, CMatrix &dst);
CVector operator*(const CMatrix &mat, const CVector &vec);
CMatrix operator*(const CMatrix &m1, const CMatrix &m2);
inline CVector MultiplyInverse(const CMatrix &mat, const CVector &vec)
{
	CVector v(vec.x - mat.m_matrix.pos.x, vec.y - mat.m_matrix.pos.y, vec.z - mat.m_matrix.pos.z);
	return CVector(
		mat.m_matrix.right.x * v.x + mat.m_matrix.right.y * v.y + mat.m_matrix.right.z * v.z,
		mat.m_matrix.up.x * v.x + mat.m_matrix.up.y * v.y + mat.m_matrix.up.z * v.z,
		mat.m_matrix.at.x * v.x + mat.m_matrix.at.y * v.y + mat.m_matrix.at.z * v.z);
}

const CVector Multiply3x3(const CMatrix &mat, const CVector &vec);
const CVector Multiply3x3(const CVector &vec, const CMatrix &mat);

inline CMatrix
Invert(const CMatrix &matrix)
{
	CMatrix inv;
	return Invert(matrix, inv);
}


class CCompressedMatrixNotAligned
{
	CVector m_vecPos;
	int8 m_rightX;
	int8 m_rightY;
	int8 m_rightZ;
	int8 m_upX;
	int8 m_upY;
	int8 m_upZ;
public:
	void CompressFromFullMatrix(CMatrix &other)
	{
		m_rightX = 127.0f * other.GetRight().x;
		m_rightY = 127.0f * other.GetRight().y;
		m_rightZ = 127.0f * other.GetRight().z;
		m_upX = 127.0f * other.GetForward().x;
		m_upY = 127.0f * other.GetForward().y;
		m_upZ = 127.0f * other.GetForward().z;
		m_vecPos = other.GetPosition();
	}
	void DecompressIntoFullMatrix(CMatrix &other)
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
};

class CCompressedMatrix : public CCompressedMatrixNotAligned
{
	int _alignment; // no clue what would this align to
};