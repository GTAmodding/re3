#pragma once

#include "templates.h"
#include "ColBox.h"
#include "ColSphere.h"
#include "ColLine.h"
#include "ColPoint.h"
#include "ColTriangle.h"

struct CColModel
{
	CColSphere boundingSphere;
	CColBox boundingBox;
	int16 numSpheres;
	int16 numLines;
	int16 numBoxes;
	int16 numTriangles;
	int32 level;
	bool ownsCollisionVolumes;	// missing on PS2
	CColSphere *spheres;
	CColLine *lines;
	CColBox *boxes;
	CompressedVector *vertices;
	CColTriangle *triangles;
	CColTrianglePlane *trianglePlanes;

	CColModel(void);
	~CColModel(void);
	void RemoveCollisionVolumes(void);
	void CalculateTrianglePlanes(void);
	void RemoveTrianglePlanes(void);
	CLink<CColModel*> *GetLinkPtr(void);
	void SetLinkPtr(CLink<CColModel*>*);
	void GetTrianglePoint(CVector &v, int i) const;

	CColModel& operator=(const CColModel& other);
};