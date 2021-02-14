#pragma once

class CVector : public RwV3d
{
public:
	CVector(void) {}
	CVector(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	CVector(const RwV3d &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
	// (0,1,0) means no rotation. So get right vector and its atan
	float Heading(void) const { return Atan2(-x, y); }
	float Magnitude(void) const { return Sqrt(x*x + y*y + z*z); }
	float MagnitudeSqr(void) const { return x*x + y*y + z*z; }
	float Magnitude2D(void) const { return Sqrt(x*x + y*y); }
	float MagnitudeSqr2D(void) const { return x*x + y*y; }
	void Normalise(void);
	
	void Normalise2D(void) {
		float sq = MagnitudeSqr2D();
		float invsqrt = RecipSqrt(sq);
		x *= invsqrt;
		y *= invsqrt;
	}

	const CVector &operator+=(CVector const &right) {
		x += right.x;
		y += right.y;
		z += right.z;
		return *this;
	}

	const CVector &operator-=(CVector const &right) {
		x -= right.x;
		y -= right.y;
		z -= right.z;
		return *this;
	}

	const CVector &operator*=(float right) {
		x *= right;
		y *= right;
		z *= right;
		return *this;
	}

	const CVector &operator/=(float right) {
		x /= right;
		y /= right;
		z /= right;
		return *this;
	}

	CVector operator-() const {
		return CVector(-x, -y, -z);
	}

	const bool operator==(CVector const &right) {
		return x == right.x && y == right.y && z == right.z;
	}

	const bool operator!=(CVector const &right) {
		return x != right.x || y != right.y || z != right.z;
	}

	bool IsZero(void) const { return x == 0.0f && y == 0.0f && z == 0.0f; }
};

inline CVector operator+(const CVector &left, const CVector &right)
{
	return CVector(left.x + right.x, left.y + right.y, left.z + right.z);
}

inline CVector operator-(const CVector &left, const CVector &right)
{
	return CVector(left.x - right.x, left.y - right.y, left.z - right.z);
}

inline CVector operator*(const CVector &left, float right)
{
	return CVector(left.x * right, left.y * right, left.z * right);
}

inline CVector operator*(float left, const CVector &right)
{
	return CVector(left * right.x, left * right.y, left * right.z);
}

inline CVector operator/(const CVector &left, float right)
{
	return CVector(left.x / right, left.y / right, left.z / right);
}

inline float
DotProduct(const CVector &v1, const CVector &v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

CVector CrossProduct(const CVector &v1, const CVector &v2);

inline float
Distance(const CVector &v1, const CVector &v2)
{
	return (v2 - v1).Magnitude();
}

inline float
Distance2D(const CVector &v1, const CVector &v2)
{
	float x = v2.x - v1.x;
	float y = v2.y - v1.y;
	return Sqrt(x*x + y*y);
}

class CMatrix;

CVector Multiply3x3(const CMatrix &mat, const CVector &vec);
CVector Multiply3x3(const CVector &vec, const CMatrix &mat);
CVector operator*(const CMatrix &mat, const CVector &vec);