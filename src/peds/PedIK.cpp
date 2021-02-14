#include "common.h"

#include "Bones.h"
#include "Camera.h"
#include "PedIK.h"
#include "Ped.h"
#include "General.h"
#include "RwHelper.h"

LimbMovementInfo CPedIK::ms_torsoInfo = { DEGTORAD(50.0f), DEGTORAD(-50.0f), DEGTORAD(15.0f), DEGTORAD(45.0f), DEGTORAD(-45.0f), DEGTORAD(7.0f) };
LimbMovementInfo CPedIK::ms_headInfo = { DEGTORAD(90.0f), DEGTORAD(-90.0f), DEGTORAD(10.0f), DEGTORAD(45.0f), DEGTORAD(-45.0f), DEGTORAD(5.0f) };
LimbMovementInfo CPedIK::ms_headRestoreInfo = { DEGTORAD(90.0f), DEGTORAD(-90.0f), DEGTORAD(10.0f), DEGTORAD(45.0f), DEGTORAD(-45.0f), DEGTORAD(5.0f) };
LimbMovementInfo CPedIK::ms_upperArmInfo = { DEGTORAD(20.0f), DEGTORAD(-100.0f), DEGTORAD(20.0f), DEGTORAD(70.0f), DEGTORAD(-70.0f), DEGTORAD(10.0f) };
LimbMovementInfo CPedIK::ms_lowerArmInfo = { DEGTORAD(80.0f), DEGTORAD(0.0f), DEGTORAD(20.0f), DEGTORAD(90.0f), DEGTORAD(-90.0f), DEGTORAD(5.0f) };

const RwV3d XaxisIK = { 1.0f, 0.0f, 0.0f};
const RwV3d YaxisIK = { 0.0f, 1.0f, 0.0f};
const RwV3d ZaxisIK = { 0.0f, 0.0f, 1.0f};

CPedIK::CPedIK(CPed *ped)
{
	m_ped = ped;
	m_flags = 0;
	m_headOrient.yaw = 0.0f;
	m_headOrient.pitch = 0.0f;
	m_torsoOrient.yaw = 0.0f;
	m_torsoOrient.pitch = 0.0f;
	m_upperArmOrient.yaw = 0.0f;
	m_upperArmOrient.pitch = 0.0f;
	m_lowerArmOrient.yaw = 0.0f;
	m_lowerArmOrient.pitch = 0.0f;
}

#ifdef PED_SKIN
inline RwMatrix*
GetBoneMatrix(CPed *ped, int32 bone)
{
	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(ped->GetClump());
	int idx = RpHAnimIDGetIndex(hier, bone);
	RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
	return &mats[idx];
}
inline RwMatrix*
GetComponentMatrix(CPed *ped, int32 node)
{
	return GetBoneMatrix(ped, ped->m_pFrames[node]->nodeID);
}
#endif

void
CPedIK::RotateTorso(AnimBlendFrameData *node, LimbOrientation *limb, bool changeRoll)
{
#ifdef PED_SKIN
	if(IsClumpSkinned(m_ped->GetClump())){
		RtQuat *q = &node->hanimFrame->q;
#ifndef FIX_BUGS
		// this is what the game does (also VC), but it does not look great
		RtQuatRotate(q, &XaxisIK, RADTODEG(limb->yaw), rwCOMBINEPRECONCAT);
		RtQuatRotate(q, &ZaxisIK, RADTODEG(limb->pitch), rwCOMBINEPRECONCAT);	// pitch
#else
		// copied the code from the non-skinned case
		// this seems to work ok

		// We can't get the parent matrix of an hanim frame but
		// this function is always called with PED_MID, so we know the parent frame.
		// Trouble is that PED_MID is "Smid" on PS2/PC but BONE_torso on mobile/xbox...
		// Assuming BONE_torso, the parent is BONE_mid, so let's use that:
		RwMatrix *mat = GetBoneMatrix(m_ped, BONE_mid);

		RwV3d vec1, vec2;
		vec1.x = mat->right.z;
		vec1.y = mat->up.z;
		vec1.z = mat->at.z;
		float c = Cos(m_ped->m_fRotationCur);
		float s = Sin(m_ped->m_fRotationCur);
		vec2.x = -(c*mat->right.x + s*mat->right.y);
		vec2.y = -(c*mat->up.x + s*mat->up.y);
		vec2.z = -(c*mat->at.x + s*mat->at.y);

		// Not sure what exactly to do here
		RtQuatRotate(q, &vec1, RADTODEG(limb->yaw), rwCOMBINEPRECONCAT);
		RtQuatRotate(q, &vec2, RADTODEG(limb->pitch), rwCOMBINEPRECONCAT);
#endif
		m_ped->bDontAcceptIKLookAts = true;
	}else
#endif
	{
		RwFrame *f = node->frame;
		RwMatrix *mat = GetWorldMatrix(RwFrameGetParent(f), RwMatrixCreate());

		RwV3d upVector = { mat->right.z, mat->up.z, mat->at.z };
		RwV3d rightVector;
		RwV3d pos = RwFrameGetMatrix(f)->pos;

		// rotation == 0 -> looking in y direction
		// left? vector
		float c = Cos(m_ped->m_fRotationCur);
		float s = Sin(m_ped->m_fRotationCur);
		rightVector.x = -(c*mat->right.x + s*mat->right.y);
		rightVector.y = -(c*mat->up.x + s*mat->up.y);
		rightVector.z = -(c*mat->at.x + s*mat->at.y);

		if(changeRoll){
			// Used when aiming only involves over the legs.(canAimWithArm)
			// Automatically changes roll(forward rotation) axis of the parts above upper legs while moving, based on position of upper legs.
			// Not noticeable in normal conditions...

			RwV3d forwardVector;
			CVector inversedForward = CrossProduct(CVector(0.0f, 0.0f, 1.0f), mat->up);
			inversedForward.Normalise();
			float dotProduct = DotProduct(mat->at, inversedForward);
			if(dotProduct > 1.0f) dotProduct = 1.0f;
			if(dotProduct < -1.0f) dotProduct = -1.0f;
			float alpha = Acos(dotProduct);

			if(mat->at.z < 0.0f)
				alpha = -alpha;

			forwardVector.x = s * mat->right.x - c * mat->right.y;
			forwardVector.y = s * mat->up.x - c * mat->up.y;
			forwardVector.z = s * mat->at.x - c * mat->at.y;

			float curYaw, curPitch;
			ExtractYawAndPitchWorld(mat, &curYaw, &curPitch);
			RwMatrixRotate(RwFrameGetMatrix(f), &rightVector, RADTODEG(limb->pitch), rwCOMBINEPOSTCONCAT);
			RwMatrixRotate(RwFrameGetMatrix(f), &upVector, RADTODEG(limb->yaw - (curYaw - m_ped->m_fRotationCur)), rwCOMBINEPOSTCONCAT);
			RwMatrixRotate(RwFrameGetMatrix(f), &forwardVector, RADTODEG(alpha), rwCOMBINEPOSTCONCAT);
		}else{
			// pitch
			RwMatrixRotate(RwFrameGetMatrix(f), &rightVector, RADTODEG(limb->pitch), rwCOMBINEPOSTCONCAT);
			// yaw
			RwMatrixRotate(RwFrameGetMatrix(f), &upVector, RADTODEG(limb->yaw), rwCOMBINEPOSTCONCAT);
		}
		RwFrameGetMatrix(f)->pos = pos;
		RwMatrixDestroy(mat);
	}
}

void
CPedIK::GetComponentPosition(RwV3d &pos, uint32 node)
{
	RwFrame *f;
	RwMatrix *mat;

#ifdef PED_SKIN
	if(IsClumpSkinned(m_ped->GetClump())){
		pos.x = 0.0f;
		pos.y = 0.0f;
		pos.z = 0.0f;
		mat = GetComponentMatrix(m_ped, node);
		// could just copy the position out of the matrix...
		RwV3dTransformPoints(&pos, &pos, 1, mat);
	}else
#endif
	{
		f = m_ped->m_pFrames[node]->frame;
		mat = RwFrameGetMatrix(f);
		pos = mat->pos;

		for (f = RwFrameGetParent(f); f; f = RwFrameGetParent(f))
			RwV3dTransformPoints(&pos, &pos, 1, RwFrameGetMatrix(f));
	}
}

RwMatrix*
CPedIK::GetWorldMatrix(RwFrame *source, RwMatrix *destination)
{
	RwFrame *i;

	*destination = *RwFrameGetMatrix(source);

	for (i = RwFrameGetParent(source); i; i = RwFrameGetParent(i))
		RwMatrixTransform(destination, RwFrameGetMatrix(i), rwCOMBINEPOSTCONCAT);

	return destination;
}

LimbMoveStatus
CPedIK::MoveLimb(LimbOrientation &limb, float targetYaw, float targetPitch, LimbMovementInfo &moveInfo)
{
	LimbMoveStatus result = ONE_ANGLE_COULDNT_BE_SET_EXACTLY;

	// yaw

	if (limb.yaw > targetYaw) {
		limb.yaw -= moveInfo.yawD;
	} else if (limb.yaw < targetYaw) {
		limb.yaw += moveInfo.yawD;
	}

	if (Abs(limb.yaw - targetYaw) < moveInfo.yawD) {
		limb.yaw = targetYaw;
		result = ANGLES_SET_EXACTLY;
	}

	if (limb.yaw > moveInfo.maxYaw || limb.yaw < moveInfo.minYaw) {
		limb.yaw = clamp(limb.yaw, moveInfo.minYaw, moveInfo.maxYaw);
		result = ANGLES_SET_TO_MAX;
	}

	// pitch

	if (limb.pitch > targetPitch) {
		limb.pitch -= moveInfo.pitchD;
	} else if (limb.pitch < targetPitch) {
		limb.pitch += moveInfo.pitchD;
	}

	if (Abs(limb.pitch - targetPitch) < moveInfo.pitchD)
		limb.pitch = targetPitch;
	else
		result = ONE_ANGLE_COULDNT_BE_SET_EXACTLY;

	if (limb.pitch > moveInfo.maxPitch || limb.pitch < moveInfo.minPitch) {
		limb.pitch = clamp(limb.pitch, moveInfo.minPitch, moveInfo.maxPitch);
		result = ANGLES_SET_TO_MAX;
	}
	return result;
}

bool
CPedIK::RestoreGunPosn(void)
{
	LimbMoveStatus limbStatus = MoveLimb(m_torsoOrient, 0.0f, 0.0f, ms_torsoInfo);
	RotateTorso(m_ped->m_pFrames[PED_MID], &m_torsoOrient, false);
	return limbStatus == ANGLES_SET_EXACTLY;
}

#ifdef PED_SKIN
void
CPedIK::RotateHead(void)
{
	RtQuat *q = &m_ped->m_pFrames[PED_HEAD]->hanimFrame->q;
	RtQuatRotate(q, &XaxisIK, RADTODEG(m_headOrient.yaw), rwCOMBINEREPLACE);
	RtQuatRotate(q, &ZaxisIK, RADTODEG(m_headOrient.pitch), rwCOMBINEPOSTCONCAT);
	m_ped->bDontAcceptIKLookAts = true;
}
#endif

bool
CPedIK::LookInDirection(float targetYaw, float targetPitch)
{
	bool success = true;
	float yaw, pitch;
#ifdef PED_SKIN
	if(IsClumpSkinned(m_ped->GetClump())){
		if (!(m_ped->m_pFrames[PED_HEAD]->flag & AnimBlendFrameData::IGNORE_ROTATION)) {
			m_ped->m_pFrames[PED_HEAD]->flag |= AnimBlendFrameData::IGNORE_ROTATION;
			ExtractYawAndPitchLocalSkinned(m_ped->m_pFrames[PED_HEAD], &m_headOrient.yaw, &m_headOrient.pitch);
		}

		// parent of head is torso
		RwMatrix worldMat = *GetBoneMatrix(m_ped, BONE_torso);
		ExtractYawAndPitchWorld(&worldMat, &yaw, &pitch);

		LimbMoveStatus headStatus = MoveLimb(m_headOrient, CGeneral::LimitRadianAngle(targetYaw - yaw),
			CGeneral::LimitRadianAngle(DEGTORAD(10.0f)), ms_headInfo);
		if (headStatus == ANGLES_SET_TO_MAX)
			success = false;

		if (headStatus != ANGLES_SET_EXACTLY){
			if (!(m_flags & LOOKAROUND_HEAD_ONLY)){
				if (MoveLimb(m_torsoOrient, CGeneral::LimitRadianAngle(targetYaw), targetPitch, ms_torsoInfo))
					success = true;
			}else{
				RotateHead();
				return success;
			}
		}

		if (!(m_flags & LOOKAROUND_HEAD_ONLY))
			RotateTorso(m_ped->m_pFrames[PED_MID], &m_torsoOrient, false);
		RotateHead();
	}else
#endif
	{
		RwFrame *frame = m_ped->m_pFrames[PED_HEAD]->frame;
		RwMatrix *frameMat = RwFrameGetMatrix(frame);

		if (!(m_ped->m_pFrames[PED_HEAD]->flag & AnimBlendFrameData::IGNORE_ROTATION)) {
			m_ped->m_pFrames[PED_HEAD]->flag |= AnimBlendFrameData::IGNORE_ROTATION;
			ExtractYawAndPitchLocal(frameMat, &m_headOrient.yaw, &m_headOrient.pitch);
		}

		RwMatrix *worldMat = RwMatrixCreate();
		worldMat = GetWorldMatrix(RwFrameGetParent(frame), worldMat);

		ExtractYawAndPitchWorld(worldMat, &yaw, &pitch);
		RwMatrixDestroy(worldMat);

		yaw += m_torsoOrient.yaw;
		float neededYawTurn = CGeneral::LimitRadianAngle(targetYaw - yaw);
		pitch *= Cos(neededYawTurn);

		float neededPitchTurn = CGeneral::LimitRadianAngle(targetPitch - pitch);
		LimbMoveStatus headStatus = MoveLimb(m_headOrient, neededYawTurn, neededPitchTurn, ms_headInfo);
		if (headStatus == ANGLES_SET_TO_MAX)
			success = false;

		if (headStatus != ANGLES_SET_EXACTLY && !(m_flags & LOOKAROUND_HEAD_ONLY)) {
			float remainingTurn = CGeneral::LimitRadianAngle(targetYaw - m_ped->m_fRotationCur);
			if (MoveLimb(m_torsoOrient, remainingTurn, targetPitch, ms_torsoInfo))
				success = true;
		}
		CMatrix nextFrame = CMatrix(frameMat);
		CVector framePos = nextFrame.GetPosition();

		nextFrame.SetRotateZ(m_headOrient.pitch);
		nextFrame.RotateX(m_headOrient.yaw);
		nextFrame.GetPosition() += framePos;
		nextFrame.UpdateRW();

		if (!(m_flags & LOOKAROUND_HEAD_ONLY))
			RotateTorso(m_ped->m_pFrames[PED_MID], &m_torsoOrient, false);

	}
	return success;
}

bool
CPedIK::LookAtPosition(CVector const &pos)
{
	float yawToFace = CGeneral::GetRadianAngleBetweenPoints(
		pos.x, pos.y,
		m_ped->GetPosition().x, m_ped->GetPosition().y);

	float pitchToFace = CGeneral::GetRadianAngleBetweenPoints(
		pos.z, (m_ped->GetPosition() - pos).Magnitude2D(),
		m_ped->GetPosition().z, 0.0f);

	return LookInDirection(yawToFace, pitchToFace);
}

bool
CPedIK::PointGunInDirection(float targetYaw, float targetPitch)
{
	bool result = true;
	bool armPointedToGun = false;
	float angle = CGeneral::LimitRadianAngle(targetYaw - m_ped->m_fRotationCur);
	m_flags &= (~GUN_POINTED_SUCCESSFULLY);
	m_flags |= LOOKAROUND_HEAD_ONLY;
	if (m_flags & AIMS_WITH_ARM) {
		armPointedToGun = PointGunInDirectionUsingArm(angle, targetPitch);
		angle = CGeneral::LimitRadianAngle(angle - m_upperArmOrient.yaw);
	}
	if (armPointedToGun) {
		if (m_flags & AIMS_WITH_ARM && m_torsoOrient.yaw * m_upperArmOrient.yaw < 0.0f)
			MoveLimb(m_torsoOrient, 0.0f, m_torsoOrient.pitch, ms_torsoInfo);
	} else {
		// Unused code
		RwMatrix *matrix;
		float yaw, pitch;
#ifdef PED_SKIN
		if(IsClumpSkinned(m_ped->GetClump())){
			matrix = RwMatrixCreate();
			*matrix = *GetComponentMatrix(m_ped, PED_UPPERARMR);
			ExtractYawAndPitchWorld(matrix, &yaw, &pitch);
			RwMatrixDestroy(matrix);
		}else
#endif
		{
			matrix = GetWorldMatrix(RwFrameGetParent(m_ped->m_pFrames[PED_UPPERARMR]->frame), RwMatrixCreate());
			ExtractYawAndPitchWorld(matrix, &yaw, &pitch);
			RwMatrixDestroy(matrix);
		}
		//

		LimbMoveStatus status = MoveLimb(m_torsoOrient, angle, targetPitch, ms_torsoInfo);
		if (status == ANGLES_SET_TO_MAX)
			result = false;
		else if (status == ANGLES_SET_EXACTLY)
			m_flags |= GUN_POINTED_SUCCESSFULLY;
	}
	if (TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam() && m_flags & AIMS_WITH_ARM)
		RotateTorso(m_ped->m_pFrames[PED_MID], &m_torsoOrient, true);
	else
		RotateTorso(m_ped->m_pFrames[PED_MID], &m_torsoOrient, false);
	return result;
}

bool
CPedIK::PointGunInDirectionUsingArm(float targetYaw, float targetPitch)
{
	bool result = false;

	RwV3d upVector;	// only for non-skinned
	RwMatrix *matrix;
	float yaw, pitch;
#ifdef PED_SKIN
	if(IsClumpSkinned(m_ped->GetClump())){
		matrix = RwMatrixCreate();
		*matrix = *GetComponentMatrix(m_ped, PED_UPPERARMR);
		ExtractYawAndPitchWorld(matrix, &yaw, &pitch);
		RwMatrixDestroy(matrix);
	}else
#endif
	{
		RwFrame *frame = m_ped->m_pFrames[PED_UPPERARMR]->frame;
		matrix = GetWorldMatrix(RwFrameGetParent(frame), RwMatrixCreate());

		// with PED_SKIN this is actually done below (with a memory leak)
		upVector.x = matrix->right.z;
		upVector.y = matrix->up.z;
		upVector.z = matrix->at.z;

		ExtractYawAndPitchWorld(matrix, &yaw, &pitch);
		RwMatrixDestroy(matrix);
	}

	RwV3d rightVector = { 0.0f, 0.0f, 1.0f };
	RwV3d forwardVector = { 1.0f, 0.0f, 0.0f };

	float uaYaw, uaPitch;
#ifdef PED_SKIN
	if(IsClumpSkinned(m_ped->GetClump())){
		uaYaw = targetYaw;
		uaPitch = targetPitch + DEGTORAD(10.0f);
	}else
#endif
	{
		uaYaw = targetYaw - m_torsoOrient.yaw - DEGTORAD(15.0f);
		uaPitch = CGeneral::LimitRadianAngle(targetPitch - pitch);
	}
	LimbMoveStatus uaStatus = MoveLimb(m_upperArmOrient, uaYaw, uaPitch, ms_upperArmInfo);
	if (uaStatus == ANGLES_SET_EXACTLY) {
		m_flags |= GUN_POINTED_SUCCESSFULLY;
		result = true;
	}

#ifdef PED_SKIN
	// this code is completely missing on xbox & android, but we can keep it with the check
	// TODO? implement it for skinned geometry?
	if(!IsClumpSkinned(m_ped->GetClump()))
#endif
	if (uaStatus == ANGLES_SET_TO_MAX) {
		float laYaw = uaYaw - m_upperArmOrient.yaw;

		LimbMoveStatus laStatus;
		if (laYaw > 0.0f)
			laStatus = MoveLimb(m_lowerArmOrient, laYaw, -DEGTORAD(45.0f), ms_lowerArmInfo);
		else
			laStatus = MoveLimb(m_lowerArmOrient, laYaw, 0.0f, ms_lowerArmInfo);

		if (laStatus == ANGLES_SET_EXACTLY) {
			m_flags |= GUN_POINTED_SUCCESSFULLY;
			result = true;
		}
		RwFrame *child = GetFirstChild(m_ped->m_pFrames[PED_UPPERARMR]->frame);
		RwV3d pos = RwFrameGetMatrix(child)->pos;
		RwMatrixRotate(RwFrameGetMatrix(child), &forwardVector, RADTODEG(m_lowerArmOrient.pitch), rwCOMBINEPOSTCONCAT);
		RwMatrixRotate(RwFrameGetMatrix(child), &rightVector, RADTODEG(-m_lowerArmOrient.yaw), rwCOMBINEPOSTCONCAT);
		RwFrameGetMatrix(child)->pos = pos;
	}

#ifdef PED_SKIN
	if(IsClumpSkinned(m_ped->GetClump())){
		RtQuat *q = &m_ped->m_pFrames[PED_UPPERARMR]->hanimFrame->q;
		RtQuatRotate(q, &XaxisIK, RADTODEG(m_upperArmOrient.yaw), rwCOMBINEPOSTCONCAT);
		RtQuatRotate(q, &ZaxisIK, RADTODEG(m_upperArmOrient.pitch), rwCOMBINEPOSTCONCAT);
		m_ped->bDontAcceptIKLookAts = true;
	}else
#endif
	{
		RwFrame *frame = m_ped->m_pFrames[PED_UPPERARMR]->frame;
		// with PED_SKIN we're also getting upVector here
		RwV3d pos = RwFrameGetMatrix(frame)->pos;
		RwMatrixRotate(RwFrameGetMatrix(frame), &rightVector, RADTODEG(m_upperArmOrient.pitch), rwCOMBINEPOSTCONCAT);
		RwMatrixRotate(RwFrameGetMatrix(frame), &upVector, RADTODEG(m_upperArmOrient.yaw), rwCOMBINEPOSTCONCAT);
		RwFrameGetMatrix(frame)->pos = pos;
	}
	return result;
}

bool
CPedIK::PointGunAtPosition(CVector const& position)
{
	return PointGunInDirection(
		CGeneral::GetRadianAngleBetweenPoints(position.x, position.y, m_ped->GetPosition().x, m_ped->GetPosition().y),
		CGeneral::GetRadianAngleBetweenPoints(position.z, Distance2D(m_ped->GetPosition(), position.x, position.y),
		m_ped->GetPosition().z,
		0.0f));
}

bool
CPedIK::RestoreLookAt(void)
{
	bool result = false;
	float yaw, pitch;

#ifdef PED_SKIN
	if(IsClumpSkinned(m_ped->GetClump())){
		if (m_ped->m_pFrames[PED_HEAD]->flag & AnimBlendFrameData::IGNORE_ROTATION) {
			m_ped->m_pFrames[PED_HEAD]->flag &= (~AnimBlendFrameData::IGNORE_ROTATION);
		} else {
			ExtractYawAndPitchLocalSkinned(m_ped->m_pFrames[PED_HEAD], &yaw, &pitch);
			if (MoveLimb(m_headOrient, yaw, pitch, ms_headRestoreInfo) == ANGLES_SET_EXACTLY)
				result = true;
		}
		RotateHead();
	}else
#endif
	{
		RwMatrix *mat = RwFrameGetMatrix(m_ped->m_pFrames[PED_HEAD]->frame);
		if (m_ped->m_pFrames[PED_HEAD]->flag & AnimBlendFrameData::IGNORE_ROTATION) {
			m_ped->m_pFrames[PED_HEAD]->flag &= (~AnimBlendFrameData::IGNORE_ROTATION);
		} else {
			ExtractYawAndPitchLocal(mat, &yaw, &pitch);
			if (MoveLimb(m_headOrient, yaw, pitch, ms_headRestoreInfo) == ANGLES_SET_EXACTLY)
				result = true;
		}

		CMatrix matrix(mat);
		CVector pos = matrix.GetPosition();
		matrix.SetRotateZ(m_headOrient.pitch);
		matrix.RotateX(m_headOrient.yaw);
		matrix.Translate(pos);
		matrix.UpdateRW();
	}
	if (!(m_flags & LOOKAROUND_HEAD_ONLY)){
		MoveLimb(m_torsoOrient, 0.0f, 0.0f, ms_torsoInfo);
		if (!(m_flags & LOOKAROUND_HEAD_ONLY))
			RotateTorso(m_ped->m_pFrames[PED_MID], &m_torsoOrient, false);
	}
	return result;
}

void
CPedIK::ExtractYawAndPitchWorld(RwMatrix *mat, float *yaw, float *pitch)
{
	float f = clamp(DotProduct(mat->up, CVector(0.0f, 1.0f, 0.0f)), -1.0f, 1.0f);
	*yaw = Acos(f);
	if (mat->up.x > 0.0f) *yaw = -*yaw;

	f = clamp(DotProduct(mat->right, CVector(0.0f, 0.0f, 1.0f)), -1.0f, 1.0f);
	*pitch = Acos(f);
	if (mat->up.z > 0.0f) *pitch = -*pitch;
}

void
CPedIK::ExtractYawAndPitchLocal(RwMatrix *mat, float *yaw, float *pitch)
{
	float f = clamp(DotProduct(mat->at, CVector(0.0f, 0.0f, 1.0f)), -1.0f, 1.0f);
	*yaw = Acos(f);
	if (mat->at.y > 0.0f) *yaw = -*yaw;

	f = clamp(DotProduct(mat->right, CVector(1.0f, 0.0f, 0.0f)), -1.0f, 1.0f);
	*pitch = Acos(f);
	if (mat->up.x > 0.0f) *pitch = -*pitch;
}

#ifdef PED_SKIN
void
CPedIK::ExtractYawAndPitchLocalSkinned(AnimBlendFrameData *node, float *yaw, float *pitch)
{
	RwMatrix *mat = RwMatrixCreate();
	RtQuatConvertToMatrix(&node->hanimFrame->q, mat);
	ExtractYawAndPitchLocal(mat, yaw, pitch);
	RwMatrixDestroy(mat);
}
#endif
