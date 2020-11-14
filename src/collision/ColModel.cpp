#include "common.h"
#include "ColModel.h"
#include "Game.h"

CColModel::CColModel(void)
{
	numSpheres = 0;
	spheres = nil;
	numLines = 0;
	lines = nil;
	numBoxes = 0;
	boxes = nil;
	numTriangles = 0;
	vertices = nil;
	triangles = nil;
	trianglePlanes = nil;
	level = CGame::currLevel;
	ownsCollisionVolumes = true;
}

CColModel::~CColModel(void)
{
	RemoveCollisionVolumes();
	RemoveTrianglePlanes();
}

void
CColModel::RemoveCollisionVolumes(void)
{
	if(ownsCollisionVolumes){
		RwFree(spheres);
		RwFree(lines);
		RwFree(boxes);
		RwFree(vertices);
		RwFree(triangles);
	}
	numSpheres = 0;
	numLines = 0;
	numBoxes = 0;
	numTriangles = 0;
	spheres = nil;
	lines = nil;
	boxes = nil;
	vertices = nil;
	triangles = nil;
}

void
CColModel::CalculateTrianglePlanes(void)
{
	// HACK: allocate space for one more element to stuff the link pointer into
	trianglePlanes = (CColTrianglePlane*)RwMalloc(sizeof(CColTrianglePlane) * (numTriangles+1));
	for(int i = 0; i < numTriangles; i++)
		trianglePlanes[i].Set(vertices, triangles[i]);
}

void
CColModel::RemoveTrianglePlanes(void)
{
	RwFree(trianglePlanes);
	trianglePlanes = nil;
}

void
CColModel::SetLinkPtr(CLink<CColModel*> *lptr)
{
	assert(trianglePlanes);
	*(CLink<CColModel*>**)ALIGNPTR(&trianglePlanes[numTriangles]) = lptr;
}

CLink<CColModel*>*
CColModel::GetLinkPtr(void)
{
	assert(trianglePlanes);
	return *(CLink<CColModel*>**)ALIGNPTR(&trianglePlanes[numTriangles]);
}

void
CColModel::GetTrianglePoint(CVector &v, int i) const
{
	v = vertices[i].Get();
}

CColModel&
CColModel::operator=(const CColModel &other)
{
	int i;
	int numVerts;

	boundingSphere = other.boundingSphere;
	boundingBox = other.boundingBox;

	// copy spheres
	if(other.numSpheres){
		if(numSpheres != other.numSpheres){
			numSpheres = other.numSpheres;
			if(spheres)
				RwFree(spheres);
			spheres = (CColSphere*)RwMalloc(numSpheres*sizeof(CColSphere));
		}
		for(i = 0; i < numSpheres; i++)
			spheres[i] = other.spheres[i];
	}else{
		numSpheres = 0;
		if(spheres)
			RwFree(spheres);
		spheres = nil;
	}

	// copy lines
	if(other.numLines){
		if(numLines != other.numLines){
			numLines = other.numLines;
			if(lines)
				RwFree(lines);
			lines = (CColLine*)RwMalloc(numLines*sizeof(CColLine));
		}
		for(i = 0; i < numLines; i++)
			lines[i] = other.lines[i];
	}else{
		numLines = 0;
		if(lines)
			RwFree(lines);
		lines = nil;
	}

	// copy boxes
	if(other.numBoxes){
		if(numBoxes != other.numBoxes){
			numBoxes = other.numBoxes;
			if(boxes)
				RwFree(boxes);
			boxes = (CColBox*)RwMalloc(numBoxes*sizeof(CColBox));
		}
		for(i = 0; i < numBoxes; i++)
			boxes[i] = other.boxes[i];
	}else{
		numBoxes = 0;
		if(boxes)
			RwFree(boxes);
		boxes = nil;
	}

	// copy mesh
	if(other.numTriangles){
		// copy vertices
		numVerts = 0;
		for(i = 0; i < other.numTriangles; i++){
			if(other.triangles[i].a > numVerts)
				numVerts = other.triangles[i].a;
			if(other.triangles[i].b > numVerts)
				numVerts = other.triangles[i].b;
			if(other.triangles[i].c > numVerts)
				numVerts = other.triangles[i].c;
		}
		numVerts++;
		if(vertices)
			RwFree(vertices);
		if(numVerts){
			vertices = (CompressedVector*)RwMalloc(numVerts*sizeof(CompressedVector));
			for(i = 0; i < numVerts; i++)
				vertices[i] = other.vertices[i];
		}

		// copy triangles
		if(numTriangles != other.numTriangles){
			numTriangles = other.numTriangles;
			if(triangles)
				RwFree(triangles);
			triangles = (CColTriangle*)RwMalloc(numTriangles*sizeof(CColTriangle));
		}
		for(i = 0; i < numTriangles; i++)
			triangles[i] = other.triangles[i];
	}else{
		numTriangles = 0;
		if(triangles)
			RwFree(triangles);
		triangles = nil;
		if(vertices)
			RwFree(vertices);
		vertices = nil;
	}
	return *this;
}
