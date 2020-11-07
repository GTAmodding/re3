#pragma once

class CMatrix
{
public:
	RwMatrix m_matrix;
	RwMatrix m_matrix_old;
	RwMatrix *m_attachment;
	bool m_hasRwMatrix;	// are we the owner?

	CMatrix(void);
	CMatrix(CMatrix const &m);
	CMatrix(RwMatrix *matrix, bool owner = false);
	CMatrix(float scale){
		m_attachment = nil;
		m_hasRwMatrix = false;
		SetScale(scale);
	}
	~CMatrix(void);
	void Attach(RwMatrix *matrix, bool owner = false);
	void AttachRW(RwMatrix *matrix, bool owner = false);
	void Detach(void);
	void Update(void);
	void UpdateRW(void);
	void UpdateRWInterPolated();
	void operator=(CMatrix const &rhs);
	void save(CMatrix const &rhs);
	CMatrix &operator+=(CMatrix const &rhs);
	CMatrix &operator*=(CMatrix const &rhs);

	void SaveOldMatrix();
	RwMatrix CreateMatrixInterpolated();

	CVector &GetPosition(void) { return *(CVector *)&m_matrix.pos; }
	CVector &GetRight(void) { return *(CVector *)&m_matrix.right; }
	CVector &GetForward(void) { return *(CVector *)&m_matrix.up; }
	CVector &GetUp(void) { return *(CVector *)&m_matrix.at; }

    
	CVector &GetPositionInterpolated(void); // we assume that matrix was interpolated and updated earlier
	CVector &GetRightInterpolated(void);    // also here
	CVector &GetForwardInterpolated(void);  // also here
	CVector &GetUpInterpolated(void);       // also here
    RwMatrix &GetMatrixInterpolated(void);       // also here

	void SetTranslate(float x, float y, float z);
	void SetTranslate(const CVector &trans){ SetTranslate(trans.x, trans.y, trans.z); }
	void Translate(float x, float y, float z){
		m_matrix.pos.x += x;
		m_matrix.pos.y += y;
		m_matrix.pos.z += z;
	}
	void Translate(const CVector &trans){ Translate(trans.x, trans.y, trans.z); }

	void SetScale(float s);
	void Scale(float scale)
	{
		float *pFloatMatrix = (float*)&m_matrix;
		for (int i = 0; i < 3; i++)
#ifdef FIX_BUGS // BUGFIX from VC
			for (int j = 0; j < 3; j++)
#else
			for (int j = 0; j < 4; j++)
#endif
				pFloatMatrix[i * 4 + j] *= scale;
	}


	void SetRotateXOnly(float angle);
	void SetRotateYOnly(float angle);
	void SetRotateZOnly(float angle);
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
	void SetRotateX(float angle);
	void SetRotateY(float angle);
	void SetRotateZ(float angle);
	void SetRotate(float xAngle, float yAngle, float zAngle);
	void Rotate(float x, float y, float z);
	void RotateX(float x);
	void RotateY(float y);
	void RotateZ(float z);

	void Reorthogonalise(void) const;
	void CopyOnlyMatrix(CMatrix *other);
	void SetUnity(void);
	void ResetOrientation(void);
	void SetTranslateOnly(float x, float y, float z) {
		m_matrix.pos.x = x;
		m_matrix.pos.y = y;
		m_matrix.pos.z = z;
	}
	void SetTranslateOnly(const CVector& pos) {
		SetTranslateOnly(pos.x, pos.y, pos.z);
	}
	void CheckIntegrity(){}
};


CMatrix &Invert(const CMatrix &src, CMatrix &dst);
CMatrix Invert(const CMatrix &matrix);
CMatrix operator*(const CMatrix &m1, const CMatrix &m2);
inline CVector MultiplyInverse(const CMatrix &mat, const CVector &vec)
{
	CVector v(vec.x - mat.m_matrix.pos.x, vec.y - mat.m_matrix.pos.y, vec.z - mat.m_matrix.pos.z);
	return CVector(
		mat.m_matrix.right.x * v.x + mat.m_matrix.right.y * v.y + mat.m_matrix.right.z * v.z,
		mat.m_matrix.up.x * v.x + mat.m_matrix.up.y * v.y + mat.m_matrix.up.z * v.z,
		mat.m_matrix.at.x * v.x + mat.m_matrix.at.y * v.y + mat.m_matrix.at.z * v.z);
}

RwMatrix Mult(const RwMatrix &src1_, const RwMatrix &src2_);
RwMatrix CreateMatrixWithScale(float s);
RwMatrix Add(const RwMatrix &m1, const RwMatrix &m2);
RwMatrix InterpolateMatrix(RwMatrix &prev, RwMatrix &now, float t);

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
	void CompressFromFullMatrix(CMatrix other);
	void DecompressIntoFullMatrix(CMatrix &other);
};

class CCompressedMatrix : public CCompressedMatrixNotAligned
{
	int _alignment; // no clue what would this align to
};