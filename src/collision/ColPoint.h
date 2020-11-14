#pragma once

struct CColPoint
{
	CVector point;
	int pad1;
	// the surface normal on the surface of point
	CVector normal;
	int pad2;
	uint8 surfaceA;
	uint8 pieceA;
	uint8 surfaceB;
	uint8 pieceB;
	float depth;

	const CVector &GetNormal() { return normal; }
	float GetDepth() { return depth; }
	void Set(float depth, uint8 surfA, uint8 pieceA, uint8 surfB, uint8 pieceB) {
		this->depth = depth;
		this->surfaceA = surfA;
		this->pieceA = pieceA;
		this->surfaceB = surfB;
		this->pieceB = pieceB;
	}
	void Set(uint8 surfA, uint8 pieceA, uint8 surfB, uint8 pieceB) {
		this->surfaceA = surfA;
		this->pieceA = pieceA;
		this->surfaceB = surfB;
		this->pieceB = pieceB;
	}

	CColPoint &operator=(const CColPoint &other);
};

