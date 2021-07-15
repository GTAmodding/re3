#pragma once

#include "ParticleType.h"

class CParticle;

enum
{
	ZCHECK_FIRST      = BIT(0),
	ZCHECK_STEP       = BIT(1),
	DRAW_OPAQUE       = BIT(2),
	SCREEN_TRAIL      = BIT(3),
	SPEED_TRAIL       = BIT(4),
	RAND_VERT_V       = BIT(5),
	CYCLE_ANIM        = BIT(6),
	DRAW_DARK         = BIT(7),
	VERT_TRAIL        = BIT(8),
	_FLAG9            = BIT(9),		// unused
	DRAWTOP2D         = BIT(10),
	CLIPOUT2D         = BIT(11),
	ZCHECK_BUMP       = BIT(12),
	ZCHECK_BUMP_FIRST = BIT(13)
};


struct tParticleSystemData
{
	tParticleType m_Type;
	char          m_aName[20];
	float         m_fCreateRange;
	float         m_fDefaultInitialRadius;
	float         m_fExpansionRate;
	uint16        m_nZRotationInitialAngle;
	int16         m_nZRotationAngleChangeAmount;
	uint16        m_nZRotationChangeTime;
	uint16        m_nZRadiusChangeTime;
	float         m_fInitialZRadius;
	float         m_fZRadiusChangeAmount;
	uint16        m_nFadeToBlackTime;
	int16         m_nFadeToBlackAmount;
	uint8         m_nFadeToBlackInitialIntensity;
	uint8         m_nFadeAlphaInitialIntensity;
	uint16        m_nFadeAlphaTime;
	int16         m_nFadeAlphaAmount;
	uint16        m_nStartAnimationFrame;
	uint16        m_nFinalAnimationFrame;
	uint16        m_nAnimationSpeed;
	uint16        m_nRotationSpeed;
	float         m_fGravitationalAcceleration;
	int32         m_nFrictionDecceleration;
	int32         m_nLifeSpan;
	float         m_fPositionRandomError;
	float         m_fVelocityRandomError;
	float         m_fExpansionRateError;
	int32         m_nRotationRateError;
	uint32        m_nLifeSpanErrorShape;
	float         m_fTrailLengthMultiplier;
	uint32        Flags;
	RwRGBA        m_RenderColouring;
	uint8         m_InitialColorVariation;
	RwRGBA        m_FadeDestinationColor;
	uint32        m_ColorFadeTime;

	RwRaster      **m_ppRaster;
	CParticle     *m_pParticles;
};

VALIDATE_SIZE(tParticleSystemData, 0x88);

class cParticleSystemMgr
{
	enum
	{
		CFG_PARAM_PARTICLE_TYPE_NAME = 0,
		CFG_PARAM_RENDER_COLOURING_R,
		CFG_PARAM_RENDER_COLOURING_G,
		CFG_PARAM_RENDER_COLOURING_B,
		CFG_PARAM_INITIAL_COLOR_VARIATION,
		CFG_PARAM_FADE_DESTINATION_COLOR_R,
		CFG_PARAM_FADE_DESTINATION_COLOR_G,
		CFG_PARAM_FADE_DESTINATION_COLOR_B,
		CFG_PARAM_COLOR_FADE_TIME,
		CFG_PARAM_DEFAULT_INITIAL_RADIUS,
		CFG_PARAM_EXPANSION_RATE,
		CFG_PARAM_INITIAL_INTENSITY,
		CFG_PARAM_FADE_TIME,
		CFG_PARAM_FADE_AMOUNT,
		CFG_PARAM_INITIAL_ALPHA_INTENSITY,
		CFG_PARAM_FADE_ALPHA_TIME,
		CFG_PARAM_FADE_ALPHA_AMOUNT,
		CFG_PARAM_INITIAL_ANGLE,
		CFG_PARAM_CHANGE_TIME,
		CFG_PARAM_ANGLE_CHANGE_AMOUNT,
		CFG_PARAM_INITIAL_Z_RADIUS,
		CFG_PARAM_Z_RADIUS_CHANGE_TIME,
		CFG_PARAM_Z_RADIUS_CHANGE_AMOUNT,
		CFG_PARAM_ANIMATION_SPEED,
		CFG_PARAM_START_ANIMATION_FRAME,
		CFG_PARAM_FINAL_ANIMATION_FRAME,
		CFG_PARAM_ROTATION_SPEED,
		CFG_PARAM_GRAVITATIONAL_ACCELERATION,
		CFG_PARAM_FRICTION_DECCELERATION,
		CFG_PARAM_LIFE_SPAN,
		CFG_PARAM_POSITION_RANDOM_ERROR,
		CFG_PARAM_VELOCITY_RANDOM_ERROR,
		CFG_PARAM_EXPANSION_RATE_ERROR,
		CFG_PARAM_ROTATION_RATE_ERROR,
		CFG_PARAM_LIFE_SPAN_ERROR_SHAPE,
		CFG_PARAM_TRAIL_LENGTH_MULTIPLIER,
		CFG_PARAM_PARTICLE_CREATE_RANGE,
		CFG_PARAM_FLAGS,

		MAX_CFG_PARAMS,
		CFG_PARAM_FIRST = CFG_PARAM_PARTICLE_TYPE_NAME,
		CFG_PARAM_LAST = CFG_PARAM_FLAGS
	};

public:
	tParticleSystemData m_aParticles[MAX_PARTICLES];

	cParticleSystemMgr();

	void Initialise();
	void LoadParticleData();
	void RangeCheck(tParticleSystemData *pData) { }
};

VALIDATE_SIZE(cParticleSystemMgr, 0x2420);

extern cParticleSystemMgr mod_ParticleSystemManager;
