#pragma once

#include "Vehicle.h"

// some miami bike leftovers

enum eBikeNodes {
	BIKE_NODE_NONE,
	BIKE_CHASSIS,
	BIKE_FORKS_FRONT,
	BIKE_FORKS_REAR,
	BIKE_WHEEL_FRONT,
	BIKE_WHEEL_REAR,
	BIKE_MUDGUARD,
	BIKE_HANDLEBARS,
	BIKE_NUM_NODES
};

class CBike : public CVehicle
{
public:
	RwFrame *m_aBikeNodes[BIKE_NUM_NODES]; // assuming
	uint8 unk1[96];
	AnimationId m_bikeSitAnimation;
	uint8 unk2[180];
	float m_aSuspensionSpringRatio[4];

	/* copied from VC, one of the floats here is gone, assuming m_bike_unused1 */
	float m_aSuspensionSpringRatioPrev[4];
	float m_aWheelTimer[4];
	//float m_bike_unused1;
	int m_aWheelSkidmarkType[2];
	bool m_aWheelSkidmarkBloody[2];
	bool m_aWheelSkidmarkUnk[2];
	float m_aWheelRotation[2];
	float m_aWheelSpeed[2];
	float m_aWheelPosition[2];
	float m_aWheelBasePosition[2];
	float m_aSuspensionSpringLength[4];
	float m_aSuspensionLineLength[4];
	float m_fHeightAboveRoad;
	/**/

	float m_fTraction;
};