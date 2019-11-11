#include "common.h"
#include "patcher.h"
#include "TempColModels.h"
#include "SurfaceTable.h"

CColModel &CTempColModels::ms_colModelPed1 = *(CColModel*)0x726CB0;
CColModel &CTempColModels::ms_colModelPed2 = *(CColModel*)0x726D08;
CColModel &CTempColModels::ms_colModelBBox = *(CColModel*)0x727FE0;
CColModel &CTempColModels::ms_colModelBumper1 = *(CColModel*)0x86BE88;
CColModel &CTempColModels::ms_colModelWheel1 = *(CColModel*)0x878C40;
CColModel &CTempColModels::ms_colModelPanel1 = *(CColModel*)0x87BDD8;
CColModel &CTempColModels::ms_colModelBodyPart2 = *(CColModel*)0x87BE30;
CColModel &CTempColModels::ms_colModelBodyPart1 = *(CColModel*)0x87BE88;
CColModel (&CTempColModels::ms_colModelCutObj)[5] = *(CColModel(*)[5]) *(uintptr*)0x87C960;
CColModel &CTempColModels::ms_colModelPedGroundHit = *(CColModel*)0x880480;
CColModel &CTempColModels::ms_colModelBoot1 = *(CColModel*)0x880670;
CColModel &CTempColModels::ms_colModelDoor1 = *(CColModel*)0x880850;
CColModel &CTempColModels::ms_colModelBonnet1 = *(CColModel*)0x8808A8;


CColSphere s_aPedSpheres[3];
CColSphere s_aPed2Spheres[3];
CColSphere s_aPedGSpheres[4];
CColSphere s_aDoorSpheres[4];
CColSphere s_aBumperSpheres[4];
CColSphere s_aPanelSpheres[4];
CColSphere s_aBonnetSpheres[4];
CColSphere s_aBootSpheres[4];
CColSphere s_aWheelSpheres[2];
CColSphere s_aBodyPartSpheres1[2];
CColSphere s_aBodyPartSpheres2[2];

void
CTempColModels::Initialise(void)
{
#define SET_COLMODEL_SPHERES(colmodel, sphrs)\
	colmodel.numSpheres = ARRAYSIZE(sphrs);\
	colmodel.spheres = sphrs;\
	colmodel.level = LEVEL_NONE;\
	colmodel.ownsCollisionVolumes = false;\

	int i;

	ms_colModelBBox.boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelBBox.boundingBox.Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f), SURFACE_DEFAULT, 0);
	ms_colModelBBox.level = LEVEL_NONE;

	for (i = 0; i < ARRAYSIZE(ms_colModelCutObj); i++) {
		ms_colModelCutObj[i].boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
		ms_colModelCutObj[i].boundingBox.Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f), SURFACE_DEFAULT, 0);
		ms_colModelCutObj[i].level = LEVEL_NONE;
	}

	for (i = 0; i < 3; i++)
		s_aPedSpheres[0].radius = 0.35f;

	s_aPedSpheres[0].center = CVector(0.0f, 0.0f, -0.25f);
	s_aPedSpheres[1].center = CVector(0.0f, 0.0f, 0.15f);
	s_aPedSpheres[2].center = CVector(0.0f, 0.0f, 0.55f);

#ifdef FIX_BUGS
	for (i = 0; i < 3; i++) {
#else
	for (i = 0; i < 4; i++) {
#endif
		s_aPedSpheres[i].surface = SURFACE_FLESH;
		s_aPedSpheres[i].piece = 0;
	}

	ms_colModelPed1.boundingSphere.Set(1.25f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelPed1.boundingBox.Set(CVector(-0.35f, -0.35f, -1.0f), CVector(0.35f, 0.35f, 0.9f), SURFACE_DEFAULT, 0);
	SET_COLMODEL_SPHERES(ms_colModelPed1, s_aPedSpheres);

	s_aPed2Spheres[0].radius = 0.3f;
	s_aPed2Spheres[1].radius = 0.4f;
	s_aPed2Spheres[2].radius = 0.3f;

	s_aPed2Spheres[0].center = CVector(0.0f, 0.35f, -0.9f);
	s_aPed2Spheres[1].center = CVector(0.0f, 0.0f, -0.9f);
	s_aPed2Spheres[2].center = CVector(0.0f, -0.35f, -0.9f);

	for (i = 0; i < 3; i++) {
		s_aPed2Spheres[i].surface = SURFACE_FLESH;
		s_aPed2Spheres[i].piece = 0;
	}

	ms_colModelPed2.boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelPed2.boundingBox.Set(CVector(-0.7f, -0.7f, -1.2f), CVector(0.7f, 0.7f, 0.0f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelPed2, s_aPed2Spheres);

	// Ped ground collision
	
	s_aPedGSpheres[0].radius = 0.35f;
	s_aPedGSpheres[1].radius = 0.35f;
	s_aPedGSpheres[2].radius = 0.35f;
	s_aPedGSpheres[3].radius = 0.3f;

	s_aPedGSpheres[0].center = CVector(0.0f, -0.4f, -0.9f);
	s_aPedGSpheres[1].center = CVector(0.0f, -0.1f, -0.9f);
	s_aPedGSpheres[2].center = CVector(0.0f, 0.25f, -0.9f);
	s_aPedGSpheres[3].center = CVector(0.0f, 0.65f, -0.9f);

	s_aPedGSpheres[0].surface = SURFACE_FLESH;
	s_aPedGSpheres[1].surface = SURFACE_FLESH;
	s_aPedGSpheres[2].surface = SURFACE_FLESH;
	s_aPedGSpheres[3].surface = SURFACE_FLESH;
	s_aPedGSpheres[0].piece = 4;
	s_aPedGSpheres[1].piece = 1;
	s_aPedGSpheres[2].piece = 0;
	s_aPedGSpheres[3].piece = 6;

	ms_colModelPedGroundHit.boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelPedGroundHit.boundingBox.Set(CVector(-0.4f, -1.0f, -1.25f), CVector(0.4f, 1.2f, -0.5f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelPedGroundHit, s_aPedGSpheres);

	// Door Spheres

	s_aDoorSpheres[0].radius = 0.15f;
	s_aDoorSpheres[1].radius = 0.15f;
	s_aDoorSpheres[2].radius = 0.25f;

	s_aDoorSpheres[0].center = CVector(0.0f, -0.25f, -0.35f);
	s_aDoorSpheres[1].center = CVector(0.0f, -0.95f, -0.35f);
	s_aDoorSpheres[2].center = CVector(0.0f, -0.6f, 0.25f);

	for (i = 0; i < 3; i++) {
		s_aDoorSpheres[i].surface = SURFACE_BILLBOARD;
		s_aDoorSpheres[i].piece = 0;
	}

	ms_colModelDoor1.boundingSphere.Set(1.5f, CVector(0.0f, -0.6f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelDoor1.boundingBox.Set(CVector(-0.3f, 0.0f, -0.6f), CVector(0.3f, -1.2f, 0.6f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelDoor1, s_aDoorSpheres);

	// Bumper Spheres

	for (i = 0; i < 4; i++)
		s_aBumperSpheres[i].radius = 0.15f;

	s_aBumperSpheres[0].center = CVector(0.85f, -0.05f, 0.0f);
	s_aBumperSpheres[1].center = CVector(0.4f, 0.05f, 0.0f);
	s_aBumperSpheres[2].center = CVector(-0.4f, 0.05f, 0.0f);
	s_aBumperSpheres[3].center = CVector(-0.85f, -0.05f, 0.0f);

	for (i = 0; i < 4; i++) {
		s_aBumperSpheres[i].surface = SURFACE_BILLBOARD;
		s_aBumperSpheres[i].piece = 0;
	}

	ms_colModelBumper1.boundingSphere.Set(2.2f, CVector(0.0f, -0.6f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelBumper1.boundingBox.Set(CVector(-1.2f, -0.3f, -0.2f), CVector(1.2f, 0.3f, -0.2f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelBumper1, s_aBumperSpheres);

	// Panel Spheres

	for (i = 0; i < 4; i++)
		s_aPanelSpheres[i].radius = 0.15f;

	s_aPanelSpheres[0].center = CVector(0.15f, 0.45f, 0.0f);
	s_aPanelSpheres[1].center = CVector(0.15f, -0.45f, 0.0f);
	s_aPanelSpheres[2].center = CVector(-0.15f, -0.45f, 0.0f);
	s_aPanelSpheres[3].center = CVector(-0.15f, 0.45f, 0.0f);

	for (i = 0; i < 4; i++) {
		s_aPanelSpheres[i].surface = SURFACE_BILLBOARD;
		s_aPanelSpheres[i].piece = 0;
	}

	ms_colModelPanel1.boundingSphere.Set(1.4f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelPanel1.boundingBox.Set(CVector(-0.3f, -0.6f, -0.15f), CVector(0.3f, 0.6f, 0.15f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelPanel1, s_aPanelSpheres);

	for (i = 0; i < 4; i++)
		s_aBonnetSpheres[i].radius = 0.2f;

	s_aBonnetSpheres[0].center = CVector(-0.4f, 0.1f, 0.0f);
	s_aBonnetSpheres[1].center = CVector(-0.4f, 0.9f, 0.0f);
	s_aBonnetSpheres[2].center = CVector(0.4f, 0.1f, 0.0f);
	s_aBonnetSpheres[3].center = CVector(0.4f, 0.9f, 0.0f);

	for (i = 0; i < 4; i++) {
		s_aBonnetSpheres[i].surface = SURFACE_BILLBOARD;
		s_aBonnetSpheres[i].piece = 0;
	}

	ms_colModelBonnet1.boundingSphere.Set(1.7f, CVector(0.0f, 0.5f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelBonnet1.boundingBox.Set(CVector(-0.7f, -0.2f, -0.3f), CVector(0.7f, 1.2f, 0.3f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelBonnet1, s_aBonnetSpheres);

	for (i = 0; i < 4; i++)
		s_aBootSpheres[i].radius = 0.2f;

	s_aBootSpheres[0].center = CVector(-0.4f, -0.1f, 0.0f);
	s_aBootSpheres[1].center = CVector(-0.4f, -0.6f, 0.0f);
	s_aBootSpheres[2].center = CVector(0.4f, -0.1f, 0.0f);
	s_aBootSpheres[3].center = CVector(0.4f, -0.6f, 0.0f);

	for (i = 0; i < 4; i++) {
		s_aBootSpheres[i].surface = SURFACE_BILLBOARD;
		s_aBootSpheres[i].piece = 0;
	}


	ms_colModelBoot1.boundingSphere.Set(1.4f, CVector(0.0f, -0.4f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelBoot1.boundingBox.Set(CVector(-0.7f, -0.9f, -0.3f), CVector(0.7f, 0.2f, 0.3f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelBoot1, s_aBootSpheres);

	s_aWheelSpheres[0].radius = 0.35f;
	s_aWheelSpheres[1].radius = 0.35f;

	s_aWheelSpheres[0].center = CVector(-0.3f, 0.0f, 0.0f);
	s_aWheelSpheres[1].center = CVector(0.3f, 0.0f, 0.0f);

#ifdef FIX_BUGS
	for (i = 0; i < 2; i++) {
#else
	for (i = 0; i < 4; i++) {
#endif
		s_aWheelSpheres[i].surface = SURFACE_RUBBER29;
		s_aWheelSpheres[i].piece = 0;
	}


	ms_colModelWheel1.boundingSphere.Set(1.4f, CVector(0.0f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelWheel1.boundingBox.Set(CVector(-0.7f, -0.4f, -0.4f), CVector(0.7f, 0.4f, 0.4f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelWheel1, s_aWheelSpheres);

	s_aBodyPartSpheres1[0].radius = 0.2f;
	s_aBodyPartSpheres1[1].radius = 0.2f;

	s_aBodyPartSpheres1[0].center = CVector(0.0f, 0.0f, 0.0f);
	s_aBodyPartSpheres1[1].center = CVector(0.8f, 0.0f, 0.0f);

#ifdef FIX_BUGS
	for (i = 0; i < 2; i++) {
#else
	for (i = 0; i < 4; i++) {
#endif
		s_aBodyPartSpheres1[i].surface = SURFACE_FLESH;
		s_aBodyPartSpheres1[i].piece = 0;
	}

	ms_colModelBodyPart1.boundingSphere.Set(0.7f, CVector(0.4f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelBodyPart1.boundingBox.Set(CVector(-0.3f, -0.3f, -0.3f), CVector(1.1f, 0.3f, 0.3f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelBodyPart1, s_aBodyPartSpheres1);

	s_aBodyPartSpheres2[0].radius = 0.15f;
	s_aBodyPartSpheres2[1].radius = 0.15f;

	s_aBodyPartSpheres2[0].center = CVector(0.0f, 0.0f, 0.0f);
	s_aBodyPartSpheres2[1].center = CVector(0.5f, 0.0f, 0.0f);

#ifdef FIX_BUGS
	for (i = 0; i < 2; i++) {
#else
	for (i = 0; i < 4; i++) {
#endif
		s_aBodyPartSpheres2[i].surface = SURFACE_FLESH;
		s_aBodyPartSpheres2[i].piece = 0;
	}

	ms_colModelBodyPart2.boundingSphere.Set(0.5f, CVector(0.25f, 0.0f, 0.0f), SURFACE_DEFAULT, 0);
	ms_colModelBodyPart2.boundingBox.Set(CVector(-0.2f, -0.2f, -0.2f), CVector(0.7f, 0.2f, 0.2f), SURFACE_DEFAULT, 0);

	SET_COLMODEL_SPHERES(ms_colModelBodyPart2, s_aBodyPartSpheres2);

#undef SET_COLMODEL_SPHERES
}

STARTPATCHES
	InjectHook(0x412160, CTempColModels::Initialise, PATCH_JUMP);
ENDPATCHES
