#pragma once

class CSceneEdit
{
public:
	static int32 &m_bCameraFollowActor;
	static bool &m_bRecording;
	static CVector &m_vecCurrentPosition;
	static CVector &m_vecCamHeading;

	static void Update(void);
	static void Init(void);
};
