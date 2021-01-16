#include "common.h"

#include "TempColModels.h"
#include "Game.h"

CColModel CTempColModels::ms_colModelPed1;
CColModel CTempColModels::ms_colModelPed2;
CColModel CTempColModels::ms_colModelBBox;
CColModel CTempColModels::ms_colModelBumper1;
CColModel CTempColModels::ms_colModelWheel1;
CColModel CTempColModels::ms_colModelPanel1;
CColModel CTempColModels::ms_colModelBodyPart2;
CColModel CTempColModels::ms_colModelBodyPart1;
CColModel CTempColModels::ms_colModelCutObj[5];
CColModel CTempColModels::ms_colModelPedGroundHit;
CColModel CTempColModels::ms_colModelBoot1;
CColModel CTempColModels::ms_colModelDoor1;
CColModel CTempColModels::ms_colModelBonnet1;


CColSphere s_aPedSpheres[3];
CColSphere s_aPed2Spheres[3];
CColSphere s_aPedGSpheres[4];
#ifdef FIX_BUGS
CColSphere s_aDoorSpheres[3];
#else
CColSphere s_aDoorSpheres[4];
#endif
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
	colmodel.numSpheres = ARRAY_SIZE(sphrs);\
	colmodel.spheres = sphrs;\
	colmodel.level = LEVEL_GENERIC;\
	colmodel.ownsCollisionVolumes = false;

	int i;

	ms_colModelBBox.boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelBBox.boundingBox.Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f));
	ms_colModelBBox.level = LEVEL_GENERIC;

	for (i = 0; i < ARRAY_SIZE(ms_colModelCutObj); i++) {
		ms_colModelCutObj[i].boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
		ms_colModelCutObj[i].boundingBox.Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f));
		ms_colModelCutObj[i].level = LEVEL_GENERIC;
	}

	// Ped Spheres

	for (i = 0; i < ARRAY_SIZE(s_aPedSpheres); i++)
		s_aPedSpheres[i].radius = 0.35f;

	s_aPedSpheres[0].center = CVector(0.0f, 0.0f, -0.25f);
	s_aPedSpheres[1].center = CVector(0.0f, 0.0f, 0.15f);
	s_aPedSpheres[2].center = CVector(0.0f, 0.0f, 0.55f);

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aPedSpheres); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aPedGSpheres); i++) {
#endif
		s_aPedSpheres[i].surface = SURFACE_PED;
		s_aPedSpheres[i].piece = 0;
	}

	ms_colModelPed1.boundingSphere.Set(1.25f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelPed1.boundingBox.Set(CVector(-0.35f, -0.35f, -1.0f), CVector(0.35f, 0.35f, 0.9f));
	SET_COLMODEL_SPHERES(ms_colModelPed1, s_aPedSpheres);

	// Ped 2 Spheres

	s_aPed2Spheres[0].radius = 0.3f;
	s_aPed2Spheres[1].radius = 0.4f;
	s_aPed2Spheres[2].radius = 0.3f;

	s_aPed2Spheres[0].center = CVector(0.0f, 0.35f, -0.9f);
	s_aPed2Spheres[1].center = CVector(0.0f, 0.0f, -0.9f);
	s_aPed2Spheres[2].center = CVector(0.0f, -0.35f, -0.9f);

	for (i = 0; i < ARRAY_SIZE(s_aPed2Spheres); i++) {
		s_aPed2Spheres[i].surface = SURFACE_PED;
		s_aPed2Spheres[i].piece = 0;
	}

	ms_colModelPed2.boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelPed2.boundingBox.Set(CVector(-0.7f, -0.7f, -1.2f), CVector(0.7f, 0.7f, 0.0f));

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

	s_aPedGSpheres[0].surface = SURFACE_PED;
	s_aPedGSpheres[1].surface = SURFACE_PED;
	s_aPedGSpheres[2].surface = SURFACE_PED;
	s_aPedGSpheres[3].surface = SURFACE_PED;
	s_aPedGSpheres[0].piece = 4;
	s_aPedGSpheres[1].piece = 1;
	s_aPedGSpheres[2].piece = 0;
	s_aPedGSpheres[3].piece = 6;

	ms_colModelPedGroundHit.boundingSphere.Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelPedGroundHit.boundingBox.Set(CVector(-0.4f, -1.0f, -1.25f), CVector(0.4f, 1.2f, -0.5f));

	SET_COLMODEL_SPHERES(ms_colModelPedGroundHit, s_aPedGSpheres);

	// Door Spheres

	s_aDoorSpheres[0].radius = 0.15f;
	s_aDoorSpheres[1].radius = 0.15f;
	s_aDoorSpheres[2].radius = 0.25f;

	s_aDoorSpheres[0].center = CVector(0.0f, -0.25f, -0.35f);
	s_aDoorSpheres[1].center = CVector(0.0f, -0.95f, -0.35f);
	s_aDoorSpheres[2].center = CVector(0.0f, -0.6f, 0.25f);

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aDoorSpheres); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aPed2Spheres); i++) {
#endif
		s_aDoorSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aDoorSpheres[i].piece = 0;
	}

	ms_colModelDoor1.boundingSphere.Set(1.5f, CVector(0.0f, -0.6f, 0.0f));
	ms_colModelDoor1.boundingBox.Set(CVector(-0.3f, 0.0f, -0.6f), CVector(0.3f, -1.2f, 0.6f));

	SET_COLMODEL_SPHERES(ms_colModelDoor1, s_aDoorSpheres);

	// Bumper Spheres

	for (i = 0; i < ARRAY_SIZE(s_aBumperSpheres); i++)
		s_aBumperSpheres[i].radius = 0.15f;

	s_aBumperSpheres[0].center = CVector(0.85f, -0.05f, 0.0f);
	s_aBumperSpheres[1].center = CVector(0.4f, 0.05f, 0.0f);
	s_aBumperSpheres[2].center = CVector(-0.4f, 0.05f, 0.0f);
	s_aBumperSpheres[3].center = CVector(-0.85f, -0.05f, 0.0f);

	for (i = 0; i < ARRAY_SIZE(s_aBumperSpheres); i++) {
		s_aBumperSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aBumperSpheres[i].piece = 0;
	}

	ms_colModelBumper1.boundingSphere.Set(2.2f, CVector(0.0f, -0.6f, 0.0f));
	ms_colModelBumper1.boundingBox.Set(CVector(-1.2f, -0.3f, -0.2f), CVector(1.2f, 0.3f, 0.2f));

	SET_COLMODEL_SPHERES(ms_colModelBumper1, s_aBumperSpheres);

	// Panel Spheres

	for (i = 0; i < ARRAY_SIZE(s_aPanelSpheres); i++)
		s_aPanelSpheres[i].radius = 0.15f;

	s_aPanelSpheres[0].center = CVector(0.15f, 0.45f, 0.0f);
	s_aPanelSpheres[1].center = CVector(0.15f, -0.45f, 0.0f);
	s_aPanelSpheres[2].center = CVector(-0.15f, -0.45f, 0.0f);
	s_aPanelSpheres[3].center = CVector(-0.15f, 0.45f, 0.0f);

	for (i = 0; i < ARRAY_SIZE(s_aPanelSpheres); i++) {
		s_aPanelSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aPanelSpheres[i].piece = 0;
	}

	ms_colModelPanel1.boundingSphere.Set(1.4f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelPanel1.boundingBox.Set(CVector(-0.3f, -0.6f, -0.15f), CVector(0.3f, 0.6f, 0.15f));

	SET_COLMODEL_SPHERES(ms_colModelPanel1, s_aPanelSpheres);

	// Bonnet Spheres

	for (i = 0; i < ARRAY_SIZE(s_aBonnetSpheres); i++)
		s_aBonnetSpheres[i].radius = 0.2f;

	s_aBonnetSpheres[0].center = CVector(-0.4f, 0.1f, 0.0f);
	s_aBonnetSpheres[1].center = CVector(-0.4f, 0.9f, 0.0f);
	s_aBonnetSpheres[2].center = CVector(0.4f, 0.1f, 0.0f);
	s_aBonnetSpheres[3].center = CVector(0.4f, 0.9f, 0.0f);

	for (i = 0; i < ARRAY_SIZE(s_aBonnetSpheres); i++) {
		s_aBonnetSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aBonnetSpheres[i].piece = 0;
	}

	ms_colModelBonnet1.boundingSphere.Set(1.7f, CVector(0.0f, 0.5f, 0.0f));
	ms_colModelBonnet1.boundingBox.Set(CVector(-0.7f, -0.2f, -0.3f), CVector(0.7f, 1.2f, 0.3f));

	SET_COLMODEL_SPHERES(ms_colModelBonnet1, s_aBonnetSpheres);

	// Boot Spheres

	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++)
		s_aBootSpheres[i].radius = 0.2f;

	s_aBootSpheres[0].center = CVector(-0.4f, -0.1f, 0.0f);
	s_aBootSpheres[1].center = CVector(-0.4f, -0.6f, 0.0f);
	s_aBootSpheres[2].center = CVector(0.4f, -0.1f, 0.0f);
	s_aBootSpheres[3].center = CVector(0.4f, -0.6f, 0.0f);

	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++) {
		s_aBootSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aBootSpheres[i].piece = 0;
	}

	ms_colModelBoot1.boundingSphere.Set(1.4f, CVector(0.0f, -0.4f, 0.0f));
	ms_colModelBoot1.boundingBox.Set(CVector(-0.7f, -0.9f, -0.3f), CVector(0.7f, 0.2f, 0.3f));

	SET_COLMODEL_SPHERES(ms_colModelBoot1, s_aBootSpheres);

	// Wheel Spheres

	s_aWheelSpheres[0].radius = 0.35f;
	s_aWheelSpheres[1].radius = 0.35f;

	s_aWheelSpheres[0].center = CVector(-0.3f, 0.0f, 0.0f);
	s_aWheelSpheres[1].center = CVector(0.3f, 0.0f, 0.0f);

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aWheelSpheres); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++) {
#endif
		s_aWheelSpheres[i].surface = SURFACE_WHEELBASE;
		s_aWheelSpheres[i].piece = 0;
	}

	ms_colModelWheel1.boundingSphere.Set(1.4f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelWheel1.boundingBox.Set(CVector(-0.7f, -0.4f, -0.4f), CVector(0.7f, 0.4f, 0.4f));

	SET_COLMODEL_SPHERES(ms_colModelWheel1, s_aWheelSpheres);

	// Body Part Spheres 1

	s_aBodyPartSpheres1[0].radius = 0.2f;
	s_aBodyPartSpheres1[1].radius = 0.2f;

	s_aBodyPartSpheres1[0].center = CVector(0.0f, 0.0f, 0.0f);
	s_aBodyPartSpheres1[1].center = CVector(0.8f, 0.0f, 0.0f);

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aBodyPartSpheres1); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++) {
#endif
		s_aBodyPartSpheres1[i].surface = SURFACE_PED;
		s_aBodyPartSpheres1[i].piece = 0;
	}

	ms_colModelBodyPart1.boundingSphere.Set(0.7f, CVector(0.4f, 0.0f, 0.0f));
	ms_colModelBodyPart1.boundingBox.Set(CVector(-0.3f, -0.3f, -0.3f), CVector(1.1f, 0.3f, 0.3f));

	SET_COLMODEL_SPHERES(ms_colModelBodyPart1, s_aBodyPartSpheres1);

	// Body Part Spheres 2

	s_aBodyPartSpheres2[0].radius = 0.15f;
	s_aBodyPartSpheres2[1].radius = 0.15f;

	s_aBodyPartSpheres2[0].center = CVector(0.0f, 0.0f, 0.0f);
	s_aBodyPartSpheres2[1].center = CVector(0.5f, 0.0f, 0.0f);

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aBodyPartSpheres2); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++) {
#endif
		s_aBodyPartSpheres2[i].surface = SURFACE_PED;
		s_aBodyPartSpheres2[i].piece = 0;
	}

	ms_colModelBodyPart2.boundingSphere.Set(0.5f, CVector(0.25f, 0.0f, 0.0f));
	ms_colModelBodyPart2.boundingBox.Set(CVector(-0.2f, -0.2f, -0.2f), CVector(0.7f, 0.2f, 0.2f));

	SET_COLMODEL_SPHERES(ms_colModelBodyPart2, s_aBodyPartSpheres2);

#undef SET_COLMODEL_SPHERES
}
