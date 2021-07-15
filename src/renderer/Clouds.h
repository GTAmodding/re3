#pragma once

class CClouds
{
public:
	static float CloudRotation;
	static uint32 IndividualRotation;

	static float ms_cameraRoll;
	static float ms_horizonZ;
	static CRGBA ms_colourTop;
	static CRGBA ms_colourBottom;

	static void Init(void);
	static void Shutdown(void);
	static void Update(void);
	static void Render(void);
	static void RenderBackground(int16 topred, int16 topgreen, int16 topblue,
		int16 botred, int16 botgreen, int16 botblue, int16 alpha);
	static void RenderHorizon(void);
};
