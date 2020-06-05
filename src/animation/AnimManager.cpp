#include "common.h"

#include "General.h"
#include "RwHelper.h"
#include "ModelInfo.h"
#include "ModelIndices.h"
#include "FileMgr.h"
#include "RpAnimBlend.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendAssociation.h"
#include "AnimBlendAssocGroup.h"
#include "AnimManager.h"
#include "Streaming.h"

//--MIAMI: code done (except for pointless TODO)

CAnimBlock CAnimManager::ms_aAnimBlocks[NUMANIMBLOCKS];
CAnimBlendHierarchy CAnimManager::ms_aAnimations[NUMANIMATIONS];
int32 CAnimManager::ms_numAnimBlocks;
int32 CAnimManager::ms_numAnimations;
CAnimBlendAssocGroup *CAnimManager::ms_aAnimAssocGroups;
CLinkList<CAnimBlendHierarchy*> CAnimManager::ms_animCache;

AnimAssocDesc aStdAnimDescs[] = {
	{ ANIM_WALK, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_RUN, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_SPRINT, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_IDLE_STANCE, ASSOC_REPEAT },
	{ ANIM_WALK_START, ASSOC_HAS_TRANSLATION },
	{ ANIM_RUN_STOP, ASSOC_DELETEFADEDOUT | ASSOC_HAS_TRANSLATION },
	{ ANIM_RUN_STOP_R, ASSOC_DELETEFADEDOUT | ASSOC_HAS_TRANSLATION },
	{ ANIM_IDLE_CAM, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_IDLE_HBHB, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_IDLE_TIRED, ASSOC_REPEAT },
	{ ANIM_IDLE_ARMED, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_IDLE_CHAT, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_IDLE_TAXI, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_KO_SHOT_FRONT1, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_KO_SHOT_FRONT2, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_KO_SHOT_FRONT3, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_KO_SHOT_FRONT4, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_KO_SHOT_FACE, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_KO_SHOT_STOM, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_KO_SHOT_ARML, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_KO_SHOT_ARMR, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_KO_SHOT_LEGL, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_KO_SHOT_LEGR, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_KD_LEFT, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_KD_RIGHT, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_KO_SKID_FRONT, ASSOC_PARTIAL },
	{ ANIM_KO_SPIN_R, ASSOC_PARTIAL },
	{ ANIM_KO_SKID_BACK, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_KO_SPIN_L, ASSOC_PARTIAL },
	{ ANIM_SHOT_FRONT_PARTIAL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_SHOT_LEFT_PARTIAL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_SHOT_BACK_PARTIAL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_SHOT_RIGHT_PARTIAL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_HIT_FRONT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_HIT_LEFT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_HIT_BACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_HIT_RIGHT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FLOOR_HIT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_HIT_BODYBLOW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_HIT_CHEST, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_HIT_HEAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_HIT_WALK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_HIT_WALL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_FLOOR_HIT_F, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_HIT_BEHIND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_IDLE, ASSOC_REPEAT },
	{ ANIM_FIGHT2_IDLE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_SH_F, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_FIGHT_BODYBLOW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_HEAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_KICK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_KNEE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_LHOOK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_PUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_ROUNDHOUSE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_FIGHT_LONGKICK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_FIGHT_PPUNCH, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_FIGHT_JAB, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_ELBOW_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_ELBOW_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_BKICK_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_BKICK_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BOMBER, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_PUNCH_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_PPUNCH2, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_KICK_FLOOR, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_THROWU, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FIGHT_SH_BACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_CAR_JACKED_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_CAR_LJACKED_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_CAR_JACKED_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_CAR_LJACKED_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_CAR_QJACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_QJACKED, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_CAR_ALIGN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_ALIGNHI_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_OPEN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_DOORLOCKED_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_PULLOUT_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_PULLOUT_LOW_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_GETIN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_GETIN_LOW_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_CLOSEDOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_CLOSEDOOR_LOW_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_ROLLDOOR, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_ROLLDOOR_LOW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_JUMPIN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_GETOUT_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_GETOUT_LOW_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_CLOSE_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_ALIGN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_ALIGNHI_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_OPEN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_DOORLOCKED_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_PULLOUT_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_PULLOUT_LOW_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_GETIN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_GETIN_LOW_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_CLOSEDOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_CLOSEDOOR_LOW_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_SHUFFLE_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_LSHUFFLE_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_SIT, ASSOC_DELETEFADEDOUT},
	{ ANIM_CAR_LSIT, ASSOC_DELETEFADEDOUT},
	{ ANIM_CAR_SITP, ASSOC_DELETEFADEDOUT},
	{ ANIM_CAR_SITPLO, ASSOC_DELETEFADEDOUT},
	{ ANIM_DRIVE_L, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVE_R, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVE_LOW_L, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVE_LOW_R, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVEBY_L, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVEBY_R, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVEBY_LOW_L, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVEBY_LOW_R, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_CAR_LB, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVE_BOAT, ASSOC_DELETEFADEDOUT | ASSOC_DRIVING },
	{ ANIM_DRIVE_BOAT_L, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_DRIVE_BOAT_R, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BOAT_LB, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_PICKUP_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_PICKUP_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_PULLUP_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_PULLUP_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_ELBOW_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_ELBOW_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_FALL_OFF, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_BIKE_FALL_R, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_CAR_GETOUT_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_GETOUT_LOW_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_CLOSE_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_HOOKERTALK, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_IDLE_STANCE2, ASSOC_PARTIAL },
	{ ANIM_IDLE_STANCE3, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_CRAWLOUT_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_CRAWLOUT_RHS2, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_CAR_ROLLOUT_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_CAR_ROLLOUT_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_GETUP1, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_GETUP2, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_GETUP3, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_GETUP_FRONT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_JUMP_LAUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_JUMP_GLIDE, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_JUMP_LAND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_FALL_FALL, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_FALL_GLIDE, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_FALL_LAND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_FALL_COLLAPSE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_FALL_BACK, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_FALL_FRONT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_EV_STEP, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_EV_DIVE, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_XPRESS_SCRATCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_IDLE },
	{ ANIM_ROAD_CROSS, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_TURN_180, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_ARREST_GUN, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_DROWN, ASSOC_PARTIAL },
	{ ANIM_DUCK_DOWN, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_DUCK_LOW, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_WEAPON_CROUCH, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_RBLOCK_CSHOOT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_HANDSUP, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_HANDSCOWER, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_FUCKU, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_PHONE_IN, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_PHONE_OUT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_PHONE_TALK, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_SEAT_DOWN, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_SEAT_UP, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_SEAT_IDLE, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_SEAT_DOWN2, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_ATM, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_ABSEIL, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
};
AnimAssocDesc aVanAnimDescs[] = {
	{ ANIM_VAN_OPEN_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_VAN_GETIN_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_VAN_CLOSE_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_VAN_GETOUT_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_VAN_OPEN, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_VAN_GETIN, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_VAN_CLOSE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_VAN_GETOUT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aCoachAnimDescs[] = {
	{ ANIM_COACH_OPEN_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_COACH_OPEN_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_COACH_IN_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_COACH_IN_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_COACH_OUT_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aBikeAnimDescs[] = {
	{ ANIM_BIKE_RIDE, ASSOC_DELETEFADEDOUT},
	{ ANIM_BIKE_STILL, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_LEFT, ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_RIGHT, ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_BACK, ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_FWD, ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_PUSHES, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_JUMPON_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_JUMPON_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_KICK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_HIT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_GETOFF_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_GETOFF_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_GETOFF_BACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_BIKE_DRIVEBY_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_DRIVEBY_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_DRIVEBY_FT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_PASSENGER, ASSOC_DELETEFADEDOUT | ASSOC_DRIVING },
};
AnimAssocDesc aMeleeAnimDescs[] = {
	{ ANIM_MELEE_ATTACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_ATTACK_2ND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_ATTACK_START, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_WEAPON_CROUCHRELOAD, ASSOC_REPEAT }, // TODO(Miami): Overload that name for melee/swing
	{ ANIM_WEAPON_SPECIAL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION }, // TODO(Miami): Overload that name for melee/swing
};
AnimAssocDesc aSwingAnimDescs[] = {
	{ ANIM_MELEE_ATTACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_ATTACK_2ND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_ATTACK_START, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_CROUCHRELOAD, ASSOC_REPEAT }, // TODO(Miami): Overload that name for melee/swing
	{ ANIM_WEAPON_SPECIAL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL }, // TODO(Miami): Overload that name for melee/swing
};
AnimAssocDesc aWeaponAnimDescs[] = {
	{ ANIM_WEAPON_FIRE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_CROUCHFIRE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_RELOAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_CROUCHRELOAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_SPECIAL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aMedicAnimDescs[] = {
	{ ANIM_CPR, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aSunbatheAnimDescs[] = {
	{ ANIM_SUNBATHE, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_SUNBATHE_DOWN, ASSOC_REPEAT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_SUNBATHE_UP, ASSOC_REPEAT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_SUNBATHE_ESCAPE, ASSOC_REPEAT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
};
AnimAssocDesc aPlayerIdleAnimDescs[] = {
	{ ANIM_IDLE_STRETCH, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_IDLE_TIME, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_IDLE_SHOULDER, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_IDLE_STRETCH_LEG, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aRiotAnimDescs[] = {
	{ ANIM_RIOT_ANGRY, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_ANGRY_B, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_CHANT, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_PUNCHES, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_SHOUT, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_CHALLENGE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_FUKU, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aStripAnimDescs[] = {
	{ ANIM_STRIP_A, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_B, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_C, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_D, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_E, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_F, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_G, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aStdAnimDescsSide[] = {
	{ ANIM_WALK, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION | ASSOC_WALK },
	{ ANIM_RUN, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION | ASSOC_WALK },
	{ ANIM_SPRINT, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION | ASSOC_WALK },
	{ ANIM_IDLE_STANCE, ASSOC_REPEAT },
	{ ANIM_WALK_START, ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
};

char const* aStdAnimations[] = {
	"walk_civi",
	"run_civi",
	"sprint_panic",
	"idle_stance",
	"walk_start",
	"run_stop",
	"run_stopR",
	"idle_hbhb",
	"idle_hbhb",
	"idle_tired",
	"idle_armed",
	"idle_chat",
	"idle_taxi",
	"KO_shot_front",
	"KO_shot_front",
	"KO_shot_front",
	"KO_shot_front",
	"KO_shot_face",
	"KO_shot_stom",
	"KO_shot_arml",
	"KO_shot_armR",
	"KO_shot_legl",
	"KO_shot_legR",
	"KD_left",
	"KD_right",
	"KO_skid_front",
	"KO_spin_R",
	"KO_skid_back",
	"KO_spin_L",
	"SHOT_partial",
	"SHOT_leftP",
	"SHOT_partial",
	"SHOT_rightP",
	"HIT_front",
	"HIT_L",
	"HIT_back",
	"HIT_R",
	"FLOOR_hit",
	"HIT_bodyblow",
	"HIT_chest",
	"HIT_head",
	"HIT_walk",
	"HIT_wall",
	"FLOOR_hit_f",
	"HIT_behind",
	"FIGHTIDLE",
	"FIGHT2IDLE",
	"FIGHTsh_F",
	"FIGHTbodyblow",
	"FIGHThead",
	"FIGHTkick",
	"FIGHTknee",
	"FIGHTLhook",
	"FIGHTpunch",
	"FIGHTrndhse",
	"FIGHTlngkck",
	"FIGHTppunch",
	"FIGHTjab",
	"FIGHTelbowL",
	"FIGHTelbowR",
	"FIGHTbkickL",
	"FIGHTbkickR",
	"bomber",
	"punchR",
	"FIGHTppunch",
	"KICK_floor",
	"WEAPON_throwu",
	"FIGHTsh_back",
	"car_jackedRHS",
	"car_LjackedRHS",
	"car_jackedLHS",
	"car_LjackedLHS",
	"CAR_Qjack",
	"CAR_Qjacked",
	"CAR_align_LHS",
	"CAR_alignHI_LHS",
	"CAR_open_LHS",
	"CAR_doorlocked_LHS",
	"CAR_pullout_LHS",
	"CAR_pulloutL_LHS",
	"CAR_getin_LHS",
	"CAR_getinL_LHS",
	"CAR_closedoor_LHS",
	"CAR_closedoorL_LHS",
	"CAR_rolldoor",
	"CAR_rolldoorLO",
	"CAR_jumpin_LHS",
	"CAR_getout_LHS",
	"CAR_getoutL_LHS",
	"CAR_close_LHS",
	"CAR_align_RHS",
	"CAR_alignHI_RHS",
	"CAR_open_RHS",
	"CAR_doorlocked_RHS",
	"CAR_pullout_RHS",
	"CAR_pulloutL_RHS",
	"CAR_getin_RHS",
	"CAR_getinL_RHS",
	"CAR_closedoor_RHS",
	"CAR_closedoorL_RHS",
	"CAR_shuffle_RHS",
	"CAR_Lshuffle_RHS",
	"CAR_sit",
	"CAR_Lsit",
	"CAR_sitp",
	"CAR_sitpLO",
	"DRIVE_L",
	"Drive_R",
	"Drive_LO_l",
	"Drive_LO_R",
	"Driveby_L",
	"Driveby_R",
	"DrivebyL_L",
	"DrivebyL_R",
	"CAR_LB",
	"DRIVE_BOAT",
	"DRIVE_BOAT_L",
	"DRIVE_BOAT_R",
	"DRIVE_BOAT_back",
	"BIKE_pickupR",
	"BIKE_pickupL",
	"BIKE_pullupR",
	"BIKE_pullupL",
	"BIKE_elbowR",
	"BIKE_elbowL",
	"BIKE_fall_off",
	"BIKE_fallR",
	"CAR_getout_RHS",
	"CAR_getoutL_RHS",
	"CAR_close_RHS",
	"car_hookertalk",
	"idle_stance",
	"idle_stance",
	"CAR_crawloutRHS",
	"CAR_crawloutRHS",
	"CAR_rollout_LHS",
	"CAR_rollout_LHS",
	"Getup",
	"Getup",
	"Getup",
	"Getup_front",
	"JUMP_launch",
	"JUMP_glide",
	"JUMP_land",
	"FALL_fall",
	"FALL_glide",
	"FALL_land",
	"FALL_collapse",
	"FALL_back",
	"FALL_front",
	"EV_step",
	"EV_dive",
	"XPRESSscratch",
	"roadcross",
	"TURN_180",
	"ARRESTgun",
	"DROWN",
	"DUCK_down",
	"DUCK_low",
	"WEAPON_crouch",
	"RBLOCK_Cshoot",
	"handsup",
	"handsCOWER",
	"FUCKU",
	"PHONE_in",
	"PHONE_out",
	"PHONE_talk",
	"SEAT_down",
	"SEAT_up",
	"SEAT_idle",
	"SEAT_down",
	"ATM",
	"abseil",
};
char const* aVanAnimations[] = {
	"VAN_openL",
	"VAN_getinL",
	"VAN_closeL",
	"VAN_getoutL",
	"VAN_open",
	"VAN_getin",
	"VAN_close",
	"VAN_getout",
};
char const* aCoachAnimations[] = {
	"COACH_opnL",
	"COACH_opnL",
	"COACH_inL",
	"COACH_inL",
	"COACH_outL",
};
char const* aBikesAnimations[] = {
	"BIKEs_Ride",
	"BIKEs_Still",
	"BIKEs_Left",
	"BIKEs_Right",
	"BIKEs_Back",
	"BIKEs_Fwd",
	"BIKEs_pushes",
	"BIKEs_jumponR",
	"BIKEs_jumponL",
	"BIKEs_kick",
	"BIKEs_hit",
	"BIKEs_getoffRHS",
	"BIKEs_getoffLHS",
	"BIKEs_getoffBACK",
	"BIKEs_drivebyLHS",
	"BIKEs_drivebyRHS",
	"BIKEs_drivebyFT",
	"BIKEs_passenger",
};
char const* aBikevAnimations[] = {
	"BIKEv_Ride",
	"BIKEv_Still",
	"BIKEv_Left",
	"BIKEv_Right",
	"BIKEv_Back",
	"BIKEv_Fwd",
	"BIKEv_pushes",
	"BIKEv_jumponR",
	"BIKEv_jumponL",
	"BIKEv_kick",
	"BIKEv_hit",
	"BIKEv_getoffRHS",
	"BIKEv_getoffLHS",
	"BIKEv_getoffBACK",
	"BIKEv_drivebyLHS",
	"BIKEv_drivebyRHS",
	"BIKEv_drivebyFT",
	"BIKEv_passenger",
};
char const* aBikehAnimations[] = {
	"BIKEh_Ride",
	"BIKEh_Still",
	"BIKEh_Left",
	"BIKEh_Right",
	"BIKEh_Back",
	"BIKEh_Fwd",
	"BIKEh_pushes",
	"BIKEh_jumponR",
	"BIKEh_jumponL",
	"BIKEh_kick",
	"BIKEh_hit",
	"BIKEh_getoffRHS",
	"BIKEh_getoffLHS",
	"BIKEh_getoffBACK",
	"BIKEh_drivebyLHS",
	"BIKEh_drivebyRHS",
	"BIKEh_drivebyFT",
	"BIKEh_passenger",
};
char const* aBikedAnimations[] = {
	"BIKEd_Ride",
	"BIKEd_Still",
	"BIKEd_Left",
	"BIKEd_Right",
	"BIKEd_Back",
	"BIKEd_Fwd",
	"BIKEd_pushes",
	"BIKEd_jumponR",
	"BIKEd_jumponL",
	"BIKEd_kick",
	"BIKEd_hit",
	"BIKEd_getoffRHS",
	"BIKEd_getoffLHS",
	"BIKEd_getoffBACK",
	"BIKEd_drivebyLHS",
	"BIKEd_drivebyRHS",
	"BIKEd_drivebyFT",
	"BIKEd_passenger",
};
char const* aUnarmedAnimations[] = {
	"punchR",
	"KICK_floor",
	"FIGHTppunch",
};
char const* aScrewdriverAnimations[] = {
	"FIGHTbodyblow",
	"FIGHTbodyblow",
	"FIGHTppunch",
	"FIGHTIDLE",
	"FIGHTbodyblow",
};
char const* aKnifeAnimations[] = {
	"WEAPON_knife_1",
	"WEAPON_knife_2",
	"knife_part",
	"WEAPON_knifeidle",
	"WEAPON_knife_3",
};
char const* aBaseballbatAnimations[] = {
	"WEAPON_bat_h",
	"WEAPON_bat_v",
	"BAT_PART",
	"WEAPON_bat_h",
	"WEAPON_golfclub",
};
char const* aGolfclubAnimations[] = {
	"WEAPON_bat_h",
	"WEAPON_golfclub",
	"BAT_PART",
	"WEAPON_bat_h",
	"WEAPON_bat_v",
};
char const* aChainsawAnimations[] = {
	"WEAPON_csaw",
	"WEAPON_csawlo",
	"csaw_part",
};
char const* aPythonAnimations[] = {
	"python_fire",
	"python_crouchfire",
	"python_reload",
	"python_crouchreload",
};
char const* aColtAnimations[] = {
	"colt45_fire",
	"colt45_crouchfire",
	"colt45_reload",
	"colt45_crouchreload",
	"colt45_cop",
};
char const* aShotgunAnimations[] = {
	"shotgun_fire",
	"shotgun_crouchfire",
};
char const* aBuddyAnimations[] = {
	"buddy_fire",
	"buddy_crouchfire",
};
char const* aTecAnimations[] = {
	"TEC_fire",
	"TEC_crouchfire",
	"TEC_reload",
	"TEC_crouchreload",
};
char const* aUziAnimations[] = {
	"UZI_fire",
	"UZI_crouchfire",
	"UZI_reload",
	"UZI_crouchreload",
};
char const* aRifleAnimations[] = {
	"RIFLE_fire",
	"RIFLE_crouchfire",
	"RIFLE_load",
	"RIFLE_crouchload",
};
char const* aM60Animations[] = {
	"M60_fire",
	"M60_fire",
	"M60_reload",
};
char const* aSniperAnimations[] = {
	"WEAPON_sniper",
};
char const* aThrowAnimations[] = {
	"WEAPON_throw",
	"WEAPON_throwu",
	"WEAPON_start_throw",
};
char const* aFlamethrowerAnimations[] = {
	"FLAME_fire",
};
char const* aMedicAnimations[] = {
	"CPR",
};
char const* aSunbatheAnimations[] = {
	"bather",
	"batherdown",
	"batherup",
	"batherscape",
};
char const* aPlayerIdleAnimations[] = {
	"stretch",
	"time",
	"shldr",
	"strleg",
};
char const* aRiotAnimations[] = {
	"riot_angry",
	"riot_angry_b",
	"riot_chant",
	"riot_punches",
	"riot_shout",
	"riot_challenge",
	"riot_fuku",
};
char const* aStripAnimations[] = {
	"strip_A",
	"strip_B",
	"strip_C",
	"strip_D",
	"strip_E",
	"strip_F",
	"strip_G",
};
char const* aLanceAnimations[] = {
	"lance",
};
char const* aPlayerAnimations[] = {
	"walk_player",
	"run_player",
	"SPRINT_civi",
	"IDLE_STANCE",
	"walk_start",
};
char const* aPlayerWithRocketAnimations[] = {
	"walk_rocket",
	"run_rocket",
	"run_rocket",
	"idle_rocket",
	"walk_start_rocket",
};
char const* aPlayer1ArmedAnimations[] = {
	"walk_player",
	"run_1armed",
	"SPRINT_civi",
	"IDLE_STANCE",
	"walk_start",
};
char const* aPlayer2ArmedAnimations[] = {
	"walk_armed",
	"run_armed",
	"run_armed",
	"idle_armed",
	"walk_start_armed",
};
char const* aPlayerBBBatAnimations[] = {
	"walk_player",
	"run_player",
	"run_player",
	"IDLE_STANCE",
	"walk_start",
};
char const* aPlayerChainsawAnimations[] = {
	"walk_csaw",
	"run_csaw",
	"run_csaw",
	"IDLE_csaw",
	"walk_start_csaw",
};
char const* aShuffleAnimations[] = {
	"WALK_shuffle",
	"RUN_civi",
	"SPRINT_civi",
	"IDLE_STANCE",
};
char const* aOldAnimations[] = {
	"walk_old",
	"run_civi",
	"sprint_civi",
	"idle_stance",
};
char const* aGang1Animations[] = {
	"walk_gang1",
	"run_gang1",
	"sprint_civi",
	"idle_stance",
};
char const* aGang2Animations[] = {
	"walk_gang2",
	"run_gang1",
	"sprint_civi",
	"idle_stance",
};
char const* aFatAnimations[] = {
	"walk_fat",
	"run_civi",
	"woman_runpanic",
	"idle_stance",
};
char const* aOldFatAnimations[] = {
	"walk_fatold",
	"run_fatold",
	"woman_runpanic",
	"idle_stance",
};
char const* aJoggerAnimations[] = {
	"JOG_maleA",
	"run_civi",
	"sprint_civi",
	"idle_stance",
};
char const* aStdWomanAnimations[] = {
	"woman_walknorm",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aWomanShopAnimations[] = {
	"woman_walkshop",
	"woman_run",
	"woman_run",
	"woman_idlestance",
};
char const* aBusyWomanAnimations[] = {
	"woman_walkbusy",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aSexyWomanAnimations[] = {
	"woman_walksexy",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aFatWomanAnimations[] = {
	"walk_fat",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aOldWomanAnimations[] = {
	"woman_walkold",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aJoggerWomanAnimations[] = {
	"JOG_maleB",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aPanicChunkyAnimations[] = {
	"run_fatold",
	"woman_runpanic",
	"woman_runpanic",
	"idle_stance",
};
char const* aSkateAnimations[] = {
	"skate_run",
	"skate_sprint",
	"skate_sprint",
	"skate_idle",
};
char const* aPlayerStrafeBackAnimations[] = {
	"walk_back",
	"run_back",
	"run_back",
	"IDLE_STANCE",
	"walk_start_back",
};
char const* aPlayerStrafeLeftAnimations[] = {
	"walk_left",
	"run_left",
	"run_left",
	"IDLE_STANCE",
	"walk_start_left",
};
char const* aPlayerStrafeRightAnimations[] = {
	"walk_right",
	"run_right",
	"run_right",
	"IDLE_STANCE",
	"walk_start_right",
};
char const* aRocketStrafeBackAnimations[] = {
	"walk_rocket_back",
	"run_rocket_back",
	"run_rocket_back",
	"idle_rocket",
	"walkst_rocket_back",
};
char const* aRocketStrafeLeftAnimations[] = {
	"walk_rocket_left",
	"run_rocket_left",
	"run_rocket_left",
	"idle_rocket",
	"walkst_rocket_left",
};
char const* aRocketStrafeRightAnimations[] = {
	"walk_rocket_right",
	"run_rocket_right",
	"run_rocket_right",
	"idle_rocket",
	"walkst_rocket_right",
};
char const* aChainsawStrafeBackAnimations[] = {
	"walk_csaw_back",
	"run_csaw_back",
	"run_csaw_back",
	"idle_csaw",
	"walkst_csaw_back",
};
char const* aChainsawStrafeLeftAnimations[] = {
	"walk_csaw_left",
	"run_csaw_left",
	"run_csaw_left",
	"idle_csaw",
	"walkst_csaw_left",
};
char const* aChainsawStrafeRightAnimations[] = {
	"walk_csaw_right",
	"run_csaw_right",
	"run_csaw_right",
	"idle_csaw",
	"walkst_csaw_right",
};


#define awc(a) ARRAY_SIZE(a), a
const AnimAssocDefinition CAnimManager::ms_aAnimAssocDefinitions[NUM_ANIM_ASSOC_GROUPS] = {
	{ "man", "ped", MI_COP, awc(aStdAnimations), aStdAnimDescs },
	{ "van", "van", MI_COP, awc(aVanAnimations), aVanAnimDescs },
	{ "coach", "coach", MI_COP, awc(aCoachAnimations), aCoachAnimDescs },
	{ "bikes", "bikes", MI_COP, awc(aBikesAnimations), aBikeAnimDescs },
	{ "bikev", "bikev", MI_COP, awc(aBikevAnimations), aBikeAnimDescs },
	{ "bikeh", "bikeh", MI_COP, awc(aBikehAnimations), aBikeAnimDescs },
	{ "biked", "biked", MI_COP, awc(aBikedAnimations), aBikeAnimDescs },
	{ "unarmed", "ped", MI_COP, awc(aUnarmedAnimations), aMeleeAnimDescs },
	{ "screwdrv", "ped", MI_COP, awc(aScrewdriverAnimations), aMeleeAnimDescs },
	{ "knife", "knife", MI_COP, awc(aKnifeAnimations), aMeleeAnimDescs },
	{ "baseball", "baseball", MI_COP, awc(aBaseballbatAnimations), aSwingAnimDescs },
	{ "golfclub", "baseball", MI_COP, awc(aGolfclubAnimations), aSwingAnimDescs },
	{ "chainsaw", "chainsaw", MI_COP, awc(aChainsawAnimations), aMeleeAnimDescs },
	{ "python", "python", MI_COP, awc(aPythonAnimations), aWeaponAnimDescs },
	{ "colt45", "colt45", MI_COP, awc(aColtAnimations), aWeaponAnimDescs },
	{ "shotgun", "shotgun", MI_COP, awc(aShotgunAnimations), aWeaponAnimDescs },
	{ "buddy", "buddy", MI_COP, awc(aBuddyAnimations), aWeaponAnimDescs },
	{ "tec", "tec", MI_COP, awc(aTecAnimations), aWeaponAnimDescs },
	{ "uzi", "uzi", MI_COP, awc(aUziAnimations), aWeaponAnimDescs },
	{ "rifle", "rifle", MI_COP, awc(aRifleAnimations), aWeaponAnimDescs },
	{ "m60", "m60", MI_COP, awc(aM60Animations), aWeaponAnimDescs },
	{ "sniper", "sniper", MI_COP, awc(aSniperAnimations), aWeaponAnimDescs },
	{ "grenade", "grenade", MI_COP, awc(aThrowAnimations), aWeaponAnimDescs },
	{ "flame", "flame", MI_COP, awc(aFlamethrowerAnimations), aWeaponAnimDescs },
	{ "medic", "medic", MI_COP, awc(aMedicAnimations), aMedicAnimDescs },
	{ "sunbathe", "sunbathe", MI_COP, 1, aSunbatheAnimations, aSunbatheAnimDescs },	// NB: not using awc here!
	{ "playidles", "playidles", MI_COP, awc(aPlayerIdleAnimations), aPlayerIdleAnimDescs },
	{ "riot", "riot", MI_COP, awc(aRiotAnimations), aRiotAnimDescs },
	{ "strip", "strip", MI_COP, awc(aStripAnimations), aStripAnimDescs },
	{ "lance", "lance", MI_COP, awc(aLanceAnimations), aSunbatheAnimDescs },
	{ "player", "ped", MI_COP, awc(aPlayerAnimations), aStdAnimDescs },
	{ "playerrocket", "ped", MI_COP, awc(aPlayerWithRocketAnimations), aStdAnimDescs },
	{ "player1armed", "ped", MI_COP, awc(aPlayer1ArmedAnimations), aStdAnimDescs },
	{ "player2armed", "ped", MI_COP, awc(aPlayer2ArmedAnimations), aStdAnimDescs },
	{ "playerBBBat", "ped", MI_COP, awc(aPlayerBBBatAnimations), aStdAnimDescs },
	{ "playercsaw", "ped", MI_COP, awc(aPlayerChainsawAnimations), aStdAnimDescs },
	{ "shuffle", "ped", MI_COP, awc(aShuffleAnimations), aStdAnimDescs },
	{ "oldman", "ped", MI_COP, awc(aOldAnimations), aStdAnimDescs },
	{ "gang1", "ped", MI_COP, awc(aGang1Animations), aStdAnimDescs },
	{ "gang2", "ped", MI_COP, awc(aGang2Animations), aStdAnimDescs },
	{ "fatman", "ped", MI_COP, awc(aFatAnimations), aStdAnimDescs },
	{ "oldfatman", "ped", MI_COP, awc(aOldFatAnimations), aStdAnimDescs },
	{ "jogger", "ped", MI_COP, awc(aJoggerAnimations), aStdAnimDescs },
	{ "woman", "ped", MI_COP, awc(aStdWomanAnimations), aStdAnimDescs },
	{ "shopping", "ped", MI_COP, awc(aWomanShopAnimations), aStdAnimDescs },
	{ "busywoman", "ped", MI_COP, awc(aBusyWomanAnimations), aStdAnimDescs },
	{ "sexywoman", "ped", MI_COP, awc(aSexyWomanAnimations), aStdAnimDescs },
	{ "fatwoman", "ped", MI_COP, awc(aFatWomanAnimations), aStdAnimDescs },
	{ "oldwoman", "ped", MI_COP, awc(aOldWomanAnimations), aStdAnimDescs },
	{ "jogwoman", "ped", MI_COP, awc(aJoggerWomanAnimations), aStdAnimDescs },
	{ "panicchunky", "ped", MI_COP, awc(aPanicChunkyAnimations), aStdAnimDescs },
	{ "skate", "skate", MI_COP, awc(aSkateAnimations), aStdAnimDescs },
	{ "playerback", "ped", MI_COP, awc(aPlayerStrafeBackAnimations), aStdAnimDescs },
	{ "playerleft", "ped", MI_COP, awc(aPlayerStrafeLeftAnimations), aStdAnimDescsSide },
	{ "playerright", "ped", MI_COP, awc(aPlayerStrafeRightAnimations), aStdAnimDescsSide },
	{ "rocketback", "ped", MI_COP, awc(aRocketStrafeBackAnimations), aStdAnimDescs },
	{ "rocketleft", "ped", MI_COP, awc(aRocketStrafeLeftAnimations), aStdAnimDescsSide },
	{ "rocketright", "ped", MI_COP, awc(aRocketStrafeRightAnimations), aStdAnimDescsSide },
	{ "csawback", "ped", MI_COP, awc(aChainsawStrafeBackAnimations), aStdAnimDescs },
	{ "csawleft", "ped", MI_COP, awc(aChainsawStrafeLeftAnimations), aStdAnimDescsSide },
	{ "csawright", "ped", MI_COP, awc(aChainsawStrafeRightAnimations), aStdAnimDescsSide },
};
#undef awc

void
CAnimManager::Initialise(void)
{
	ms_numAnimations = 0;
	ms_numAnimBlocks = 0;
	ms_animCache.Init(25);
}

void
CAnimManager::Shutdown(void)
{
	int i;

	for(i = 0; i < NUMANIMBLOCKS; i++)
		CStreaming::RemoveAnim(i);

	for(i = 0; i < ms_numAnimations; i++)
		ms_aAnimations[i].Shutdown();

	ms_animCache.Shutdown();

	delete[] ms_aAnimAssocGroups;
}

void
CAnimManager::UncompressAnimation(CAnimBlendHierarchy *hier)
{
	if(hier->compressed2){
		if(hier->totalLength == 0.0f)
			hier->CalcTotalTimeCompressed();
	}else{
		if(!hier->compressed){
			if(hier->linkPtr){
				hier->linkPtr->Remove();
				ms_animCache.head.Insert(hier->linkPtr);
			}
		}else{
			CLink<CAnimBlendHierarchy*> *link = ms_animCache.Insert(hier);
			if(link == nil){
				ms_animCache.tail.prev->item->RemoveUncompressedData();
				ms_animCache.Remove(ms_animCache.tail.prev);
				link = ms_animCache.Insert(hier);
			}
			hier->linkPtr = link;
			hier->Uncompress();
		}
	}
}

void
CAnimManager::RemoveFromUncompressedCache(CAnimBlendHierarchy *hier)
{
	if(hier->linkPtr){
		ms_animCache.Remove(hier->linkPtr);
		hier->linkPtr = nil;
	}
}

CAnimBlock*
CAnimManager::GetAnimationBlock(const char *name)
{
	int i;

	for(i = 0; i < ms_numAnimBlocks; i++)
		if(strcasecmp(ms_aAnimBlocks[i].name, name) == 0)
			return &ms_aAnimBlocks[i];
	return nil;
}

int32
CAnimManager::GetAnimationBlockIndex(const char *name)
{
	int i;

	for(i = 0; i < ms_numAnimBlocks; i++)
		if(strcasecmp(ms_aAnimBlocks[i].name, name) == 0)
			return i;
	return -1;
}

int32
CAnimManager::RegisterAnimBlock(const char *name)
{
	CAnimBlock *animBlock = GetAnimationBlock(name);
	if(animBlock == nil){
		animBlock = &ms_aAnimBlocks[ms_numAnimBlocks++];
		strncpy(animBlock->name, name, MAX_ANIMBLOCK_NAME);
		animBlock->numAnims = 0;
		assert(animBlock->refCount == 0);
	}
	return animBlock - ms_aAnimBlocks;
}

int32
CAnimManager::GetNumRefsToAnimBlock(int32 block)
{
	return ms_aAnimBlocks[block].refCount;
}

void
CAnimManager::AddAnimBlockRef(int32 block)
{
	ms_aAnimBlocks[block].refCount++;
}

void
CAnimManager::RemoveAnimBlockRefWithoutDelete(int32 block)
{
	ms_aAnimBlocks[block].refCount--;
}

void
CAnimManager::RemoveAnimBlockRef(int32 block)
{
	ms_aAnimBlocks[block].refCount--;
	if(ms_aAnimBlocks[block].refCount == 0)
		CStreaming::RemoveAnim(block);
}

void
CAnimManager::RemoveAnimBlock(int32 block)
{
	int i;
	CAnimBlock *animblock;

	animblock = &ms_aAnimBlocks[block];
	debug("Removing ANIMS %s\n", animblock->name);
	for(i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++)
		if(ms_aAnimAssocGroups[i].animBlock == animblock)
			ms_aAnimAssocGroups[i].DestroyAssociations();
	for(i = 0; i < animblock->numAnims; i++)
		ms_aAnimations[animblock->firstIndex + i].Shutdown();
	animblock->isLoaded = false;
	animblock->refCount = 0;
}

CAnimBlendHierarchy*
CAnimManager::GetAnimation(const char *name, CAnimBlock *animBlock)
{
	int i;
	CAnimBlendHierarchy *hier = &ms_aAnimations[animBlock->firstIndex];

	for(i = 0; i < animBlock->numAnims; i++){
		if(strcasecmp(hier->name, name) == 0)
			return hier;
		hier++;
	}
	return nil;
}

const char*
CAnimManager::GetAnimGroupName(AssocGroupId groupId)
{
	return ms_aAnimAssocDefinitions[groupId].name;
}

CAnimBlendAssociation*
CAnimManager::CreateAnimAssociation(AssocGroupId groupId, AnimationId animId)
{
	return ms_aAnimAssocGroups[groupId].CopyAnimation(animId);
}

CAnimBlendAssociation*
CAnimManager::GetAnimAssociation(AssocGroupId groupId, AnimationId animId)
{
	return ms_aAnimAssocGroups[groupId].GetAnimation(animId);
}

CAnimBlendAssociation*
CAnimManager::GetAnimAssociation(AssocGroupId groupId, const char *name)
{
	return ms_aAnimAssocGroups[groupId].GetAnimation(name);
}

CAnimBlendAssociation*
CAnimManager::AddAnimation(RpClump *clump, AssocGroupId groupId, AnimationId animId)
{
	CAnimBlendAssociation *anim = CreateAnimAssociation(groupId, animId);
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	if(anim->IsMovement()){
		CAnimBlendAssociation *syncanim = nil;
		CAnimBlendLink *link;
		for(link = clumpData->link.next; link; link = link->next){
			syncanim = CAnimBlendAssociation::FromLink(link);
			if(syncanim->IsMovement())
				break;
		}
		if(link){
			anim->SyncAnimation(syncanim);
			anim->flags |= ASSOC_RUNNING;
		}else
			anim->Start(0.0f);
	}else
		anim->Start(0.0f);

	clumpData->link.Prepend(&anim->link);
	return anim;
}

CAnimBlendAssociation*
CAnimManager::AddAnimationAndSync(RpClump *clump, CAnimBlendAssociation *syncanim, AssocGroupId groupId, AnimationId animId)
{
	CAnimBlendAssociation *anim = CreateAnimAssociation(groupId, animId);
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	if (anim->IsMovement() && syncanim){
		anim->SyncAnimation(syncanim);
		anim->flags |= ASSOC_RUNNING;
	}else
		anim->Start(0.0f);

	clumpData->link.Prepend(&anim->link);
	return anim;
}

CAnimBlendAssociation*
CAnimManager::BlendAnimation(RpClump *clump, AssocGroupId groupId, AnimationId animId, float delta)
{
	int removePrevAnim = 0;
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	CAnimBlendAssociation *anim = GetAnimAssociation(groupId, animId);
	bool isMovement = anim->IsMovement();
	bool isPartial = anim->IsPartial();
	CAnimBlendLink *link;
	CAnimBlendAssociation *found = nil, *movementAnim = nil;
	for(link = clumpData->link.next; link; link = link->next){
		anim = CAnimBlendAssociation::FromLink(link);
		if(isMovement && anim->IsMovement())
			movementAnim = anim;
		if(anim->animId == animId)
			found = anim;
		else{
			if(isPartial == anim->IsPartial()){
				if(anim->blendAmount > 0.0f){
					float blendDelta = -delta*anim->blendAmount;
					if(blendDelta < anim->blendDelta || !isPartial)
						anim->blendDelta = blendDelta;
				}else{
					anim->blendDelta = -1.0f;
				}
				anim->flags |= ASSOC_DELETEFADEDOUT;
				removePrevAnim = 1;
			}
		}
	}
	if(found){
		found->blendDelta = (1.0f - found->blendAmount)*delta;
		if(!found->IsRunning() && found->currentTime == found->hierarchy->totalLength)
			found->Start(0.0f);
	}else{
		found = AddAnimationAndSync(clump, movementAnim, groupId, animId);
		if(!removePrevAnim && !isPartial){
			found->blendAmount = 1.0f;
			return found;
		}
		found->blendAmount = 0.0f;
		found->blendDelta = delta;
	}
	UncompressAnimation(found->hierarchy);
	return found;
}

void
CAnimManager::LoadAnimFiles(void)
{
	LoadAnimFile("ANIM\\PED.IFP");
	ms_aAnimAssocGroups = new CAnimBlendAssocGroup[NUM_ANIM_ASSOC_GROUPS];
	CreateAnimAssocGroups();
}

void
CAnimManager::CreateAnimAssocGroups(void)
{
	int i, j;

	for(i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++){
		CAnimBlock *block = GetAnimationBlock(ms_aAnimAssocDefinitions[i].blockName);
		if(block == nil || !block->isLoaded || ms_aAnimAssocGroups[i].assocList)
			continue;

		CBaseModelInfo *mi = CModelInfo::GetModelInfo(ms_aAnimAssocDefinitions[i].modelIndex);
		RpClump *clump = (RpClump*)mi->CreateInstance();
		RpAnimBlendClumpInit(clump);
		CAnimBlendAssocGroup *group = &ms_aAnimAssocGroups[i];
		const AnimAssocDefinition *def = &ms_aAnimAssocDefinitions[i];
		group->groupId = i;
		group->firstAnimId = def->animDescs[0].animId;
		group->CreateAssociations(def->blockName, clump, def->animNames, def->numAnims);
		for(j = 0; j < group->numAssociations; j++)

			// GetAnimation(i) in III (but it's in LoadAnimFiles), GetAnimation(group->animDesc[j].animId) in VC
			group->GetAnimation(def->animDescs[j].animId)->flags |= def->animDescs[j].flags;
#ifdef PED_SKIN
		if(IsClumpSkinned(clump))
			RpClumpForAllAtomics(clump, AtomicRemoveAnimFromSkinCB, nil);
#endif
		RpClumpDestroy(clump);
	}
}

void
CAnimManager::LoadAnimFile(const char *filename)
{
	RwStream *stream;
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	assert(stream);
	LoadAnimFile(stream, true);
	RwStreamClose(stream, nil);
}

//--MIAMI: done (except maybe implement some unimplemented compression?)
void
CAnimManager::LoadAnimFile(RwStream *stream, bool compress, char (*somename)[32])
{
	#define ROUNDSIZE(x) if((x) & 3) (x) += 4 - ((x)&3)
	struct IfpHeader {
		char ident[4];
		uint32 size;
	};
	IfpHeader anpk, info, name, dgan, cpan, anim;
	char buf[256];
	int j, k, l;
	float *fbuf = (float*)buf;

	// block name
	RwStreamRead(stream, &anpk, sizeof(IfpHeader));
	ROUNDSIZE(anpk.size);
	RwStreamRead(stream, &info, sizeof(IfpHeader));
	ROUNDSIZE(info.size);
	RwStreamRead(stream, buf, info.size);
	CAnimBlock *animBlock = GetAnimationBlock(buf+4);
	if(animBlock){
		if(animBlock->numAnims == 0){
			animBlock->numAnims = *(int*)buf;
			animBlock->firstIndex = ms_numAnimations;
		}
	}else{
		animBlock = &ms_aAnimBlocks[ms_numAnimBlocks++];
		strncpy(animBlock->name, buf+4, MAX_ANIMBLOCK_NAME);
		animBlock->numAnims = *(int*)buf;
		animBlock->firstIndex = ms_numAnimations;
	}

	debug("Loading ANIMS %s\n", animBlock->name);
	animBlock->isLoaded = true;

	int animIndex = animBlock->firstIndex;
	for(j = 0; j < animBlock->numAnims; j++){
		assert(animIndex < ARRAY_SIZE(ms_aAnimations));
		CAnimBlendHierarchy *hier = &ms_aAnimations[animIndex++];

		// animation name
		RwStreamRead(stream, &name, sizeof(IfpHeader));
		ROUNDSIZE(name.size);
		RwStreamRead(stream, buf, name.size);
		hier->SetName(buf);

		// TODO(MIAMI)? some unused crap here
		hier->compressed = false;
		hier->compressed2 = false;

		// DG info has number of nodes/sequences
		RwStreamRead(stream, (char*)&dgan, sizeof(IfpHeader));
		ROUNDSIZE(dgan.size);
		RwStreamRead(stream, (char*)&info, sizeof(IfpHeader));
		ROUNDSIZE(info.size);
		RwStreamRead(stream, buf, info.size);
		hier->numSequences = *(int*)buf;
		hier->sequences = new CAnimBlendSequence[hier->numSequences];

		CAnimBlendSequence *seq = hier->sequences;
		for(k = 0; k < hier->numSequences; k++, seq++){
			// Each node has a name and key frames
			RwStreamRead(stream, &cpan, sizeof(IfpHeader));
			ROUNDSIZE(dgan.size);
			RwStreamRead(stream, &anim, sizeof(IfpHeader));
			ROUNDSIZE(anim.size);
			RwStreamRead(stream, buf, anim.size);
			int numFrames = *(int*)(buf+28);
#ifdef PED_SKIN
			if(anim.size == 44)
				seq->SetBoneTag(*(int*)(buf+40));
#endif
			seq->SetName(buf);
			if(numFrames == 0)
				continue;

			RwStreamRead(stream, &info, sizeof(info));
			if(strncmp(info.ident, "KR00", 4) == 0){
				seq->SetNumFrames(numFrames, false, false);
				KeyFrame *kf = seq->GetKeyFrame(0);
				for(l = 0; l < numFrames; l++, kf++){
					RwStreamRead(stream, buf, 0x14);
					kf->rotation.x = -fbuf[0];
					kf->rotation.y = -fbuf[1];
					kf->rotation.z = -fbuf[2];
					kf->rotation.w = fbuf[3];
					kf->deltaTime = fbuf[4];	// absolute time here
				}
			}else if(strncmp(info.ident, "KRT0", 4) == 0){
				seq->SetNumFrames(numFrames, true, false);
				KeyFrameTrans *kf = (KeyFrameTrans*)seq->GetKeyFrame(0);
				for(l = 0; l < numFrames; l++, kf++){
					RwStreamRead(stream, buf, 0x20);
					kf->rotation.x = -fbuf[0];
					kf->rotation.y = -fbuf[1];
					kf->rotation.z = -fbuf[2];
					kf->rotation.w = fbuf[3];
					kf->translation.x = fbuf[4];
					kf->translation.y = fbuf[5];
					kf->translation.z = fbuf[6];
					kf->deltaTime = fbuf[7];	// absolute time here
				}
			}else if(strncmp(info.ident, "KRTS", 4) == 0){
				seq->SetNumFrames(numFrames, true, false);
				KeyFrameTrans *kf = (KeyFrameTrans*)seq->GetKeyFrame(0);
				for(l = 0; l < numFrames; l++, kf++){
					RwStreamRead(stream, buf, 0x2C);
					kf->rotation.x = -fbuf[0];
					kf->rotation.y = -fbuf[1];
					kf->rotation.z = -fbuf[2];
					kf->rotation.w = fbuf[3];
					kf->translation.x = fbuf[4];
					kf->translation.y = fbuf[5];
					kf->translation.z = fbuf[6];
					// scaling ignored
					kf->deltaTime = fbuf[10];	// absolute time here
				}
			}
		}

		hier->RemoveQuaternionFlips();
		hier->CalcTotalTime();
	}
	if(animIndex > ms_numAnimations)
		ms_numAnimations = animIndex;
}

void
CAnimManager::RemoveLastAnimFile(void)
{
	int i;
	ms_numAnimBlocks--;
	ms_numAnimations = ms_aAnimBlocks[ms_numAnimBlocks].firstIndex;
	for(i = 0; i < ms_aAnimBlocks[ms_numAnimBlocks].numAnims; i++)
		ms_aAnimations[ms_aAnimBlocks[ms_numAnimBlocks].firstIndex + i].Shutdown();
	ms_aAnimBlocks[ms_numAnimBlocks].isLoaded = false;
}
