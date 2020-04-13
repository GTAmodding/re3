#pragma once

class CVector
{
public:
	float x, y, z;
	CVector(void) {}
	CVector(float x, float y, float z) : x(x), y(y), z(z) {}
#ifdef RWCORE_H
	CVector(const RwV3d &v) : x(v.x), y(v.y), z(v.z) {}

	operator RwV3d (void) const {
		RwV3d vecRw = { this->x, this->y, this->z };
		return vecRw;
	}
	
	operator RwV3d *(void) {
		return (RwV3d*)this;
	}
	
	operator RwV3d &(void) {
		return *((RwV3d*)this);
	}
#endif
	// (0,1,0) means no rotation. So get right vector and its atan
	float Heading(void) const { return Atan2(-x, y); }
	float Magnitude(void) const { return Sqrt(x*x + y*y + z*z); }
	float MagnitudeSqr(void) const { return x*x + y*y + z*z; }
	float Magnitude2D(void) const { return Sqrt(x*x + y*y); }
	float MagnitudeSqr2D(void) const { return x*x + y*y; }
	void Normalise(void) {
		float sq = MagnitudeSqr();
		if(sq > 0.0f){
			float invsqrt = RecipSqrt(sq);
			x *= invsqrt;
			y *= invsqrt;
			z *= invsqrt;
		}else
			x = 1.0f;
	}
	
	void Normalise(float norm) {
		float sq = MagnitudeSqr();
		float invsqrt = RecipSqrt(norm, sq);
		x *= invsqrt;
		y *= invsqrt;
		z *= invsqrt;
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

inline const CVector
CrossProduct(const CVector &v1, const CVector &v2)
{
	return CVector(
		v1.y*v2.z - v1.z*v2.y,
		v1.z*v2.x - v1.x*v2.z,
		v1.x*v2.y - v1.y*v2.x);
}

inline float
Distance(const CVector &v1, const CVector &v2)
{
	return (v2 - v1).Magnitude();
}