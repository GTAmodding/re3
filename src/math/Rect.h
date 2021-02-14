#pragma once

class CRect
{
public:
	float left;     // x min
	float bottom;   // y max
	float right;    // x max
	float top;      // y min

	CRect(void);
	CRect(float l, float t, float r, float b);
	void ContainPoint(CVector const &v){
		if(v.x < left) left = v.x;
		if(v.x > right) right = v.x;
		if(v.y < top) top = v.y;
		if(v.y > bottom) bottom = v.y;
	}
	void ContainRect(const CRect &r){
		if(r.left < left) left = r.left;
		if(r.right > right) right = r.right;
		if(r.top < top) top = r.top;
		if(r.bottom > bottom) bottom = r.bottom;
	}

	bool IsPointInside(const CVector2D &p){
		return p.x >= left &&
			p.x <= right &&
			p.y >= top &&
			p.y <= bottom;
	}
	bool IsPointInside(const CVector2D &p, float extraRadius){
		return p.x >= left-extraRadius &&
			p.x <= right+extraRadius &&
			p.y >= top-extraRadius &&
			p.y <= bottom+extraRadius;
	}

	void Translate(float x, float y){
		left += x;
		right += x;
		bottom += y;
		top += y;
	}

	void Grow(float r) {
		left -= r;
		right += r;
		top -= r;
		bottom += r;
	}

	void Grow(float l, float r)
	{
		left -= l;
		top -= l;
		right += r;
		bottom += r;
	}

	void Grow(float l, float r, float t, float b)
	{
		left -= l;
		top -= t;
		right += r;
		bottom += b;
	}

	float GetWidth(void) { return right - left; }
	float GetHeight(void) { return bottom - top; }
};
