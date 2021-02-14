#pragma once
#include "common.h"
#include "AnimBlendClumpData.h"

struct LimbOrientation
{
	float yaw;
	float pitch;
};

struct LimbMovementInfo {
	float maxYaw;
	float minYaw;
	float yawD;
	float maxPitch;
	float minPitch;
	float pitchD;
};

enum LimbMoveStatus {
	ANGLES_SET_TO_MAX, // because given angles were unreachable
	ONE_ANGLE_COULDNT_BE_SET_EXACTLY, // because it can't be reached in a jiffy
	ANGLES_SET_EXACTLY
};

class CPed;

class CPedIK
{
public:
	enum {
		GUN_POINTED_SUCCESSFULLY = 1, // set but unused
		LOOKAROUND_HEAD_ONLY = 2,
		AIMS_WITH_ARM = 4,
	};

	CPed *m_ped;
	LimbOrientation m_headOrient;
	LimbOrientation m_torsoOrient;
	LimbOrientation m_upperArmOrient;
	LimbOrientation m_lowerArmOrient;
	int32 m_flags;

	static LimbMovementInfo ms_torsoInfo;
	static LimbMovementInfo ms_headInfo;
	static LimbMovementInfo ms_headRestoreInfo;
	static LimbMovementInfo ms_upperArmInfo;
	static LimbMovementInfo ms_lowerArmInfo;

	CPedIK(CPed *ped);
	bool PointGunInDirection(float targetYaw, float targetPitch);
	bool PointGunInDirectionUsingArm(float targetYaw, float targetPitch);
	bool PointGunAtPosition(CVector const& position);
	void GetComponentPosition(RwV3d &pos, uint32 node);
	static RwMatrix *GetWorldMatrix(RwFrame *source, RwMatrix *destination);
	void RotateTorso(AnimBlendFrameData* animBlend, LimbOrientation* limb, bool changeRoll);
	void ExtractYawAndPitchLocal(RwMatrix *mat, float *yaw, float *pitch);
	void ExtractYawAndPitchLocalSkinned(AnimBlendFrameData *node, float *yaw, float *pitch);
	void ExtractYawAndPitchWorld(RwMatrix *mat, float *yaw, float *pitch);
	LimbMoveStatus MoveLimb(LimbOrientation &limb, float targetYaw, float targetPitch, LimbMovementInfo &moveInfo);
	bool RestoreGunPosn(void);
	void RotateHead(void);
	bool LookInDirection(float targetYaw, float targetPitch);
	bool LookAtPosition(CVector const& pos);
	bool RestoreLookAt(void);
};

VALIDATE_SIZE(CPedIK, 0x28);
