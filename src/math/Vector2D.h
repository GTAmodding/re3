#pragma once

class CVector2D
{
public:
	float x, y;
	CVector2D(void) {}
	CVector2D(float x, float y) : x(x), y(y) {}
	CVector2D(const CVector &v) : x(v.x), y(v.y) {}
	float Heading(void) const { return Atan2(-x, y); }
	float Magnitude(void) const { return Sqrt(x*x + y*y); }
	float MagnitudeSqr(void) const { return x*x + y*y; }

	void Normalise(void) {
		float sq = MagnitudeSqr();
		if(sq > 0.0f){
			float invsqrt = RecipSqrt(sq);
			x *= invsqrt;
			y *= invsqrt;
		}else
			x = 1.0f;
	}

	const CVector2D &operator+=(CVector2D const &right) {
		x += right.x;
		y += right.y;
		return *this;
	}

	const CVector2D &operator-=(CVector2D const &right) {
		x -= right.x;
		y -= right.y;
		return *this;
	}

	const CVector2D &operator*=(float right) {
		x *= right;
		y *= right;
		return *this;
	}

	const CVector2D &operator/=(float right) {
		x /= right;
		y /= right;
		return *this;
	}
	CVector2D operator-(const CVector2D &rhs) const {
		return CVector2D(x-rhs.x, y-rhs.y);
	}
	CVector2D operator+(const CVector2D &rhs) const {
		return CVector2D(x+rhs.x, y+rhs.y);
	}
	CVector2D operator/(float t) const {
		return CVector2D(x/t, y/t);
	}
	CVector2D operator-() const {
		return CVector2D(-x, -y);
	}
};

inline float
DotProduct2D(const CVector2D &v1, const CVector2D &v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}

inline float
CrossProduct2D(const CVector2D &v1, const CVector2D &v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}

inline float
Distance2D(const CVector2D &v, float x, float y)
{
	return Sqrt((v.x-x)*(v.x-x) + (v.y-y)*(v.y-y));
}

inline float
DistanceSqr2D(const CVector2D &v, float x, float y)
{
	return (v.x-x)*(v.x-x) + (v.y-y)*(v.y-y);
}

inline void
NormalizeXY(float &x, float &y)
{
	float l  = Sqrt(x*x + y*y);
	if(l != 0.0f){
		x /= l;
		y /= l;
	}else
		x = 1.0f;
}

inline CVector2D operator*(const CVector2D &left, float right)
{
	return CVector2D(left.x * right, left.y * right);
}

inline CVector2D operator*(float left, const CVector2D &right)
{
	return CVector2D(left * right.x, left * right.y);
}
