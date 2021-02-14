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

CAnimBlock CAnimManager::ms_aAnimBlocks[NUMANIMBLOCKS];
CAnimBlendHierarchy CAnimManager::ms_aAnimations[NUMANIMATIONS];
int32 CAnimManager::ms_numAnimBlocks;
int32 CAnimManager::ms_numAnimations;
CAnimBlendAssocGroup *CAnimManager::ms_aAnimAssocGroups;
CLinkList<CAnimBlendHierarchy*> CAnimManager::ms_animCache;

AnimAssocDesc aStdAnimDescs[] = {
	{ ANIM_STD_WALK, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_RUN, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_RUNFAST, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_IDLE, ASSOC_REPEAT },
	{ ANIM_STD_STARTWALK, ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_RUNSTOP1, ASSOC_DELETEFADEDOUT | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_RUNSTOP2, ASSOC_DELETEFADEDOUT | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_IDLE_CAM, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_IDLE_HBHB, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_IDLE_TIRED, ASSOC_REPEAT },
	{ ANIM_STD_IDLE_BIGGUN, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_CHAT, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_HAILTAXI, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_KO_FRONT, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL  },
	{ ANIM_STD_KO_LEFT, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL  },
	{ ANIM_STD_KO_BACK, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL  },
	{ ANIM_STD_KO_RIGHT, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL  },
	{ ANIM_STD_KO_SHOT_FACE, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL  },
	{ ANIM_STD_KO_SHOT_STOMACH, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_KO_SHOT_ARM_L, ASSOC_PARTIAL | ASSOC_FRONTAL  },
	{ ANIM_STD_KO_SHOT_ARM_R, ASSOC_PARTIAL | ASSOC_FRONTAL  },
	{ ANIM_STD_KO_SHOT_LEG_L, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_KO_SHOT_LEG_R, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_SPINFORWARD_LEFT, ASSOC_PARTIAL | ASSOC_FRONTAL  },
	{ ANIM_STD_SPINFORWARD_RIGHT, ASSOC_PARTIAL | ASSOC_FRONTAL  },
	{ ANIM_STD_HIGHIMPACT_FRONT, ASSOC_PARTIAL },
	{ ANIM_STD_HIGHIMPACT_LEFT, ASSOC_PARTIAL },
	{ ANIM_STD_HIGHIMPACT_BACK, ASSOC_PARTIAL | ASSOC_FRONTAL  },
	{ ANIM_STD_HIGHIMPACT_RIGHT, ASSOC_PARTIAL },
	{ ANIM_STD_HITBYGUN_FRONT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_HITBYGUN_LEFT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_HITBYGUN_BACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_HITBYGUN_RIGHT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_HIT_FRONT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_LEFT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_HIT_BACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_RIGHT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_HIT_FLOOR, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
#if GTA_VERSION <= GTA3_PS2_160
	{ ANIM_STD_HIT_BODY, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
#endif
	{ ANIM_STD_HIT_BODYBLOW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_CHEST, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_HEAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_WALK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_WALL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_FLOOR_FRONT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_FRONTAL  },
	{ ANIM_STD_HIT_BEHIND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_PUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_KICKGROUND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_WEAPON_BAT_H, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_WEAPON_BAT_V, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_WEAPON_HGUN_BODY, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_WEAPON_AK_BODY, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_WEAPON_PUMP, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_WEAPON_SNIPER, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_WEAPON_THROW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_THROW_UNDER, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_START_THROW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_DETONATE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_HGUN_RELOAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_AK_RELOAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
#ifdef PC_PLAYER_CONTROLS
	// maybe wrong define, but unused anyway
	{ ANIM_FPS_PUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FPS_BAT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FPS_UZI, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FPS_PUMP, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FPS_AK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FPS_M16, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_FPS_ROCKET, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
#endif
	{ ANIM_STD_FIGHT_IDLE, ASSOC_REPEAT },
	{ ANIM_STD_FIGHT_2IDLE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_SHUFFLE_F, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FIGHT_BODYBLOW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_HEAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_KICK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_KNEE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_LHOOK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_PUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_ROUNDHOUSE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FIGHT_LONGKICK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_PARTIAL_PUNCH, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_JACKEDCAR_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_JACKEDCAR_LO_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_JACKEDCAR_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_JACKEDCAR_LO_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_QUICKJACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_QUICKJACKED, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_ALIGN_DOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_ALIGNHI_DOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_OPEN_DOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CARDOOR_LOCKED_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_PULL_OUT_PED_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_PULL_OUT_PED_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_GET_IN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_GET_IN_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_GETOUT_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_GETOUT_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_ALIGN_DOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_ALIGNHI_DOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_OPEN_DOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CARDOOR_LOCKED_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_PULL_OUT_PED_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_PULL_OUT_PED_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_GET_IN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_GET_IN_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_SHUFFLE_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_SHUFFLE_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_SIT, ASSOC_DELETEFADEDOUT },
	{ ANIM_STD_CAR_SIT_LO, ASSOC_DELETEFADEDOUT },
	{ ANIM_STD_CAR_SIT_P, ASSOC_DELETEFADEDOUT },
	{ ANIM_STD_CAR_SIT_P_LO, ASSOC_DELETEFADEDOUT },
	{ ANIM_STD_CAR_DRIVE_LEFT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_DRIVE_RIGHT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_DRIVE_LEFT_LO, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_DRIVE_RIGHT_LO, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_DRIVEBY_LEFT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_DRIVEBY_RIGHT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_LOOKBEHIND, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_BOAT_DRIVE, ASSOC_DELETEFADEDOUT },
	{ ANIM_STD_GETOUT_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_GETOUT_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_HOOKERTALK, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_OPEN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_OPEN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_GET_IN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_GET_IN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_GET_OUT_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_TRAIN_GETIN, ASSOC_PARTIAL },
	{ ANIM_STD_TRAIN_GETOUT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CRAWLOUT_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CRAWLOUT_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_OPEN_DOOR_REAR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_GET_IN_REAR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_GET_OUT_REAR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_OPEN_DOOR_REAR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_GET_IN_REAR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_GET_OUT_REAR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_GET_UP, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_GET_UP_LEFT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_GET_UP_RIGHT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_GET_UP_FRONT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_JUMP_LAUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_JUMP_GLIDE, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_JUMP_LAND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FALL, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_FALL_GLIDE, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_FALL_LAND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FALL_COLLAPSE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_EVADE_STEP, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_EVADE_DIVE, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL  },
	{ ANIM_STD_XPRESS_SCRATCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_IDLE  },
	{ ANIM_STD_ROADCROSS, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_TURN180, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_ARREST, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_DROWN, ASSOC_PARTIAL },
	{ ANIM_MEDIC_CPR, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_DUCK_DOWN, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_DUCK_LOW, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_RBLOCK_SHOOT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_THROW_UNDER2, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_HANDSUP, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HANDSCOWER, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_PARTIAL_FUCKU, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK  },
	{ ANIM_STD_PHONE_IN, ASSOC_PARTIAL },
	{ ANIM_STD_PHONE_OUT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_PHONE_TALK, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
};
#ifdef PC_PLAYER_CONTROLS
AnimAssocDesc aStdAnimDescsSide[] = {
	{ ANIM_STD_WALK, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_STD_RUN, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_STD_RUNFAST, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_STD_IDLE, ASSOC_REPEAT },
	{ ANIM_STD_STARTWALK, ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
};
#endif
char const *aStdAnimations[] = {
	"walk_civi",
	"run_civi",
	"sprint_panic",
	"idle_stance",
	"walk_start",
	"run_stop",
	"run_stopR",
	"idle_cam",
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
#if GTA_VERSION <= GTA3_PS2_160
	"HIT_body",
#endif
	"HIT_bodyblow",
	"HIT_chest",
	"HIT_head",
	"HIT_walk",
	"HIT_wall",
	"FLOOR_hit_f",
	"HIT_behind",
	"punchR",
	"KICK_floor",
	"WEAPON_bat_h",
	"WEAPON_bat_v",
	"WEAPON_hgun_body",
	"WEAPON_AK_body",
	"WEAPON_pump",
	"WEAPON_sniper",
	"WEAPON_throw",
	"WEAPON_throwu",
	"WEAPON_start_throw",
	"bomber",
	"WEAPON_hgun_rload",
	"WEAPON_AK_rload",
#ifdef PC_PLAYER_CONTROLS
	// maybe wrong define, but unused anyway
	"FPS_PUNCH",
	"FPS_BAT",
	"FPS_UZI",
	"FPS_PUMP",
	"FPS_AK",
	"FPS_M16",
	"FPS_ROCKET",
#endif
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
	"CAR_LB",
	"DRIVE_BOAT",
	"CAR_getout_RHS",
	"CAR_getoutL_RHS",
	"CAR_close_RHS",
	"car_hookertalk",
	"COACH_opnL",
	"COACH_opnR",
	"COACH_inL",
	"COACH_inR",
	"COACH_outL",
	"TRAIN_getin",
	"TRAIN_getout",
	"CAR_crawloutRHS",
	"CAR_crawloutRHS",
	"VAN_openL",
	"VAN_getinL",
	"VAN_closeL",
	"VAN_getoutL",
	"VAN_open",
	"VAN_getin",
	"VAN_close",
	"VAN_getout",
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
	"EV_step",
	"EV_dive",
	"XPRESSscratch",
	"roadcross",
	"TURN_180",
	"ARRESTgun",
	"DROWN",
	"CPR",
	"DUCK_down",
	"DUCK_low",
	"RBLOCK_Cshoot",
	"WEAPON_throwu",
	"handsup",
	"handsCOWER",
	"FUCKU",
	"PHONE_in",
	"PHONE_out",
	"PHONE_talk",
};
char const *aPlayerAnimations[] = {
	"walk_player",
	"run_player",
	"SPRINT_civi",
	"IDLE_STANCE",
	"walk_start",
};
char const *aPlayerWithRocketAnimations[] = {
	"walk_rocket",
	"run_rocket",
	"run_rocket",
	"idle_rocket",
	"walk_start_rocket",
};
char const *aPlayer1ArmedAnimations[] = {
	"walk_player",
	"run_1armed",
	"SPRINT_civi",
	"IDLE_STANCE",
	"walk_start",
};
char const *aPlayer2ArmedAnimations[] = {
	"walk_player",
	"run_armed",
	"run_armed",
	"idle_stance",
	"walk_start",
};
char const *aPlayerBBBatAnimations[] = {
	"walk_player",
	"run_player",
	"run_player",
	"IDLE_STANCE",
	"walk_start",
};
char const *aShuffleAnimations[] = {
	"WALK_shuffle",
	"RUN_civi",
	"SPRINT_civi",
	"IDLE_STANCE",
};
char const *aOldAnimations[] = {
	"walk_old",
	"run_civi",
	"sprint_civi",
	"idle_stance",
};
char const *aGang1Animations[] = {
	"walk_gang1",
	"run_gang1",
	"sprint_civi",
	"idle_stance",
};
char const *aGang2Animations[] = {
	"walk_gang2",
	"run_gang1",
	"sprint_civi",
	"idle_stance",
};
char const *aFatAnimations[] = {
	"walk_fat",
	"run_civi",
	"woman_runpanic",
	"idle_stance",
};
char const *aOldFatAnimations[] = {
	"walk_fatold",
	"run_fatold",
	"woman_runpanic",
	"idle_stance",
};
char const *aStdWomanAnimations[] = {
	"woman_walknorm",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const *aWomanShopAnimations[] = {
	"woman_walkshop",
	"woman_run",
	"woman_run",
	"woman_idlestance",
};
char const *aBusyWomanAnimations[] = {
	"woman_walkbusy",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const *aSexyWomanAnimations[] = {
	"woman_walksexy",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const *aOldWomanAnimations[] = {
	"woman_walkold",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const *aFatWomanAnimations[] = {
	"walk_fat",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const *aPanicChunkyAnimations[] = {
	"run_fatold",
	"woman_runpanic",
	"woman_runpanic",
	"idle_stance",
};
#ifdef PC_PLAYER_CONTROLS
char const *aPlayerStrafeBackAnimations[] = {
	"walk_player_back",
	"run_player_back",
	"run_player_back",
	"IDLE_STANCE",
	"walk_start_back",
};
char const *aPlayerStrafeLeftAnimations[] = {
	"walk_player_left",
	"run_left",
	"run_left",
	"IDLE_STANCE",
	"walk_start_left",
};
char const *aPlayerStrafeRightAnimations[] = {
	"walk_player_right",
	"run_right",
	"run_right",
	"IDLE_STANCE",
	"walk_start_right",
};
char const *aRocketStrafeBackAnimations[] = {
	"walk_rocket_back",
	"run_rocket_back",
	"run_rocket_back",
	"idle_rocket",
	"walkst_rocket_back",
};
char const *aRocketStrafeLeftAnimations[] = {
	"walk_rocket_left",
	"run_rocket_left",
	"run_rocket_left",
	"idle_rocket",
	"walkst_rocket_left",
};
char const *aRocketStrafeRightAnimations[] = {
	"walk_rocket_right",
	"run_rocket_right",
	"run_rocket_right",
	"idle_rocket",
	"walkst_rocket_right",
};
#endif

#define awc(a) ARRAY_SIZE(a), a
const AnimAssocDefinition CAnimManager::ms_aAnimAssocDefinitions[NUM_ANIM_ASSOC_GROUPS] = {
	{ "man", "ped", MI_COP, awc(aStdAnimations), aStdAnimDescs },
	{ "player", "ped", MI_COP, awc(aPlayerAnimations), aStdAnimDescs },
	{ "playerrocket", "ped", MI_COP, awc(aPlayerWithRocketAnimations), aStdAnimDescs },
	{ "player1armed", "ped", MI_COP, awc(aPlayer1ArmedAnimations), aStdAnimDescs },
	{ "player2armed", "ped", MI_COP, awc(aPlayer2ArmedAnimations), aStdAnimDescs },
	{ "playerBBBat", "ped", MI_COP, awc(aPlayerBBBatAnimations), aStdAnimDescs },
	{ "shuffle", "ped", MI_COP, awc(aShuffleAnimations), aStdAnimDescs },
	{ "oldman", "ped", MI_COP, awc(aOldAnimations), aStdAnimDescs },
	{ "gang1", "ped", MI_COP, awc(aGang1Animations), aStdAnimDescs },
	{ "gang2", "ped", MI_COP, awc(aGang2Animations), aStdAnimDescs },
	{ "fatman", "ped", MI_COP, awc(aFatAnimations), aStdAnimDescs },
	{ "oldfatman", "ped", MI_COP, awc(aOldFatAnimations), aStdAnimDescs },
	{ "woman", "ped", MI_COP, awc(aStdWomanAnimations), aStdAnimDescs },
	{ "shopping", "ped", MI_COP, awc(aWomanShopAnimations), aStdAnimDescs },
	{ "busywoman", "ped", MI_COP, awc(aBusyWomanAnimations), aStdAnimDescs },
	{ "sexywoman", "ped", MI_COP, awc(aSexyWomanAnimations), aStdAnimDescs },
	{ "oldwoman", "ped", MI_COP, awc(aOldWomanAnimations), aStdAnimDescs },
	{ "fatwoman", "ped", MI_COP, awc(aFatWomanAnimations), aStdAnimDescs },
	{ "panicchunky", "ped", MI_COP, awc(aPanicChunkyAnimations), aStdAnimDescs },
#ifdef PC_PLAYER_CONTROLS
	{ "playerback", "ped", MI_COP, awc(aPlayerStrafeBackAnimations), aStdAnimDescs },
	{ "playerleft", "ped", MI_COP, awc(aPlayerStrafeLeftAnimations), aStdAnimDescsSide },
	{ "playerright", "ped", MI_COP, awc(aPlayerStrafeRightAnimations), aStdAnimDescsSide },
	{ "rocketback", "ped", MI_COP, awc(aRocketStrafeBackAnimations), aStdAnimDescs },
	{ "rocketleft", "ped", MI_COP, awc(aRocketStrafeLeftAnimations), aStdAnimDescsSide },
	{ "rocketright", "ped", MI_COP, awc(aRocketStrafeRightAnimations), aStdAnimDescsSide },
#endif
};
#undef awc

void
CAnimManager::Initialise(void)
{
	ms_numAnimations = 0;
	ms_numAnimBlocks = 0;
	ms_animCache.Init(25);

//	dumpanimdata();
}

void
CAnimManager::Shutdown(void)
{
	int i;

	ms_animCache.Shutdown();

	for(i = 0; i < ms_numAnimations; i++)
		ms_aAnimations[i].Shutdown();

	delete[] ms_aAnimAssocGroups;
}

void
CAnimManager::UncompressAnimation(CAnimBlendHierarchy *hier)
{
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

CAnimBlock*
CAnimManager::GetAnimationBlock(const char *name)
{
	int i;

	for(i = 0; i < ms_numAnimBlocks; i++)
		if(strcasecmp(ms_aAnimBlocks[i].name, name) == 0)
			return &ms_aAnimBlocks[i];
	return nil;
}

CAnimBlendHierarchy*
CAnimManager::GetAnimation(const char *name, CAnimBlock *animBlock)
{
	int i;
	CAnimBlendHierarchy *hier = &ms_aAnimations[animBlock->firstIndex];

	for(i = 0; i < animBlock->numAnims; i++){
		if(!CGeneral::faststricmp(hier->name, name))
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
	int i, j;

	LoadAnimFile("ANIM\\PED.IFP");

	// Create all assoc groups
	ms_aAnimAssocGroups = new CAnimBlendAssocGroup[NUM_ANIM_ASSOC_GROUPS];
	for(i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++){
		CBaseModelInfo *mi = CModelInfo::GetModelInfo(ms_aAnimAssocDefinitions[i].modelIndex);
		RpClump *clump = (RpClump*)mi->CreateInstance();
		RpAnimBlendClumpInit(clump);
		CAnimBlendAssocGroup *group = &ms_aAnimAssocGroups[i];
		const AnimAssocDefinition *def = &ms_aAnimAssocDefinitions[i];
		group->CreateAssociations(def->blockName, clump, def->animNames, def->numAnims);
		for(j = 0; j < group->numAssociations; j++)
			group->GetAnimation(j)->flags |= def->animDescs[j].flags;
#ifdef PED_SKIN
		// forgot on xbox/android
		if(IsClumpSkinned(clump))
			RpClumpForAllAtomics(clump, AtomicRemoveAnimFromSkinCB, nil);
#endif
		RpClumpDestroy(clump);
	}
}

void
CAnimManager::LoadAnimFile(const char *filename)
{
	int fd;
	fd = CFileMgr::OpenFile(filename, "rb");
	assert(fd > 0);
	LoadAnimFile(fd, true);
	CFileMgr::CloseFile(fd);
}

void
CAnimManager::LoadAnimFile(int fd, bool compress)
{
	#define ROUNDSIZE(x) if((x) & 3) (x) += 4 - ((x)&3)
	struct IfpHeader {
		char ident[4];
		uint32 size;
	};
	IfpHeader anpk, info, name, dgan, cpan, anim;
	int numANPK;
	char buf[256];
	int i, j, k, l;
	float *fbuf = (float*)buf;

	CFileMgr::Read(fd, (char*)&anpk, sizeof(IfpHeader));
	if(!CGeneral::faststrncmp(anpk.ident, "ANLF", 4)) {
		ROUNDSIZE(anpk.size);
		CFileMgr::Read(fd, buf, anpk.size);
		numANPK = *(int*)buf;
	} else if(!CGeneral::faststrncmp(anpk.ident, "ANPK", 4)) {
		CFileMgr::Seek(fd, -8, 1);
		numANPK = 1;
	}

	for(i = 0; i < numANPK; i++){
		// block name
		CFileMgr::Read(fd, (char*)&anpk, sizeof(IfpHeader));
		ROUNDSIZE(anpk.size);
		CFileMgr::Read(fd, (char*)&info, sizeof(IfpHeader));
		ROUNDSIZE(info.size);
		CFileMgr::Read(fd, buf, info.size);
		CAnimBlock *animBlock = &ms_aAnimBlocks[ms_numAnimBlocks++];
		strncpy(animBlock->name, buf+4, 24);
		animBlock->numAnims = *(int*)buf;

		animBlock->firstIndex = ms_numAnimations;

		for(j = 0; j < animBlock->numAnims; j++){
			CAnimBlendHierarchy *hier = &ms_aAnimations[ms_numAnimations++];

			// animation name
			CFileMgr::Read(fd, (char*)&name, sizeof(IfpHeader));
			ROUNDSIZE(name.size);
			CFileMgr::Read(fd, buf, name.size);
			hier->SetName(buf);

			// DG info has number of nodes/sequences
			CFileMgr::Read(fd, (char*)&dgan, sizeof(IfpHeader));
			ROUNDSIZE(dgan.size);
			CFileMgr::Read(fd, (char*)&info, sizeof(IfpHeader));
			ROUNDSIZE(info.size);
			CFileMgr::Read(fd, buf, info.size);
			hier->numSequences = *(int*)buf;
			hier->sequences = new CAnimBlendSequence[hier->numSequences];

			CAnimBlendSequence *seq = hier->sequences;
			for(k = 0; k < hier->numSequences; k++, seq++){
				// Each node has a name and key frames
				CFileMgr::Read(fd, (char*)&cpan, sizeof(IfpHeader));
				ROUNDSIZE(dgan.size);
				CFileMgr::Read(fd, (char*)&anim, sizeof(IfpHeader));
				ROUNDSIZE(anim.size);
				CFileMgr::Read(fd, buf, anim.size);
				int numFrames = *(int*)(buf+28);
				seq->SetName(buf);
#ifdef PED_SKIN
				if(anim.size == 44)
					seq->SetBoneTag(*(int*)(buf+40));
#endif
				if(numFrames == 0)
					continue;

				bool hasScale = false;
				bool hasTranslation = false;
				CFileMgr::Read(fd, (char*)&info, sizeof(info));
				if(!CGeneral::faststrncmp(info.ident, "KRTS", 4)) {
					hasScale = true;
					seq->SetNumFrames(numFrames, true);
				}else if(!CGeneral::faststrncmp(info.ident, "KRT0", 4)) {
					hasTranslation = true;
					seq->SetNumFrames(numFrames, true);
				}else if(!CGeneral::faststrncmp(info.ident, "KR00", 4)){
					seq->SetNumFrames(numFrames, false);
				}

				for(l = 0; l < numFrames; l++){
					if(hasScale){
						CFileMgr::Read(fd, buf, 0x2C);
						CQuaternion rot(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
						rot.Invert();
						CVector trans(fbuf[4], fbuf[5], fbuf[6]);

						KeyFrameTrans *kf = (KeyFrameTrans*)seq->GetKeyFrame(l);
						kf->rotation = rot;
						kf->translation = trans;
						// scaling ignored
						kf->deltaTime = fbuf[10];	// absolute time here
					}else if(hasTranslation){
						CFileMgr::Read(fd, buf, 0x20);
						CQuaternion rot(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
						rot.Invert();
						CVector trans(fbuf[4], fbuf[5], fbuf[6]);

						KeyFrameTrans *kf = (KeyFrameTrans*)seq->GetKeyFrame(l);
						kf->rotation = rot;
						kf->translation = trans;
						kf->deltaTime = fbuf[7];	// absolute time here
					}else{
						CFileMgr::Read(fd, buf, 0x14);
						CQuaternion rot(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
						rot.Invert();

						KeyFrame *kf = (KeyFrame*)seq->GetKeyFrame(l);
						kf->rotation = rot;
						kf->deltaTime = fbuf[4];	// absolute time here
					}
				}

				// convert absolute time to deltas
				for(l = seq->numFrames-1; l > 0; l--){
					KeyFrame *kf1 = seq->GetKeyFrame(l);
					KeyFrame *kf2 = seq->GetKeyFrame(l-1);
					kf1->deltaTime -= kf2->deltaTime;
				}
			}

			hier->RemoveQuaternionFlips();
			if(compress)
				hier->RemoveUncompressedData();
			else
				hier->CalcTotalTime();
		}
	}
}

void
CAnimManager::RemoveLastAnimFile(void)
{
	int i;
	ms_numAnimBlocks--;
	ms_numAnimations = ms_aAnimBlocks[ms_numAnimBlocks].firstIndex;
	for(i = 0; i < ms_aAnimBlocks[ms_numAnimBlocks].numAnims; i++)
		ms_aAnimations[ms_aAnimBlocks[ms_numAnimBlocks].firstIndex + i].RemoveAnimSequences();
}
