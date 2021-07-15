#include "common.h"

#include "main.h"
#include "Lights.h"
#include "Camera.h"
#include "Weather.h"
#include "World.h"
#include "Collision.h"
#include "Sprite.h"
#include "Timer.h"
#include "PointLights.h"

int16 CPointLights::NumLights;
CRegisteredPointLight CPointLights::aLights[NUMPOINTLIGHTS];

void
CPointLights::InitPerFrame(void)
{
	NumLights = 0;
}

#define MAX_DIST 22.0f

void
CPointLights::AddLight(uint8 type, CVector coors, CVector dir, float radius, float red, float green, float blue, uint8 fogType, bool castExtraShadows)
{
	CVector dist;
	float distance;

	// The check is done in some weird way in the game
	// we're doing it a bit better here
	if(NumLights >= NUMPOINTLIGHTS)
		return;

	dist = coors - TheCamera.GetPosition();
	if(Abs(dist.x) < MAX_DIST && Abs(dist.y) < MAX_DIST){
		distance = dist.Magnitude();
		if(distance < MAX_DIST){
			aLights[NumLights].type = type;
			aLights[NumLights].fogType = fogType;
			aLights[NumLights].coors = coors;
			aLights[NumLights].dir = dir;
			aLights[NumLights].radius = radius;
			aLights[NumLights].castExtraShadows = castExtraShadows;
			if(distance < MAX_DIST*0.75f){
				aLights[NumLights].red = red;
				aLights[NumLights].green = green;
				aLights[NumLights].blue = blue;
			}else{
				float fade = 1.0f - (distance/MAX_DIST - 0.75f)*4.0f;
				aLights[NumLights].red = red * fade;
				aLights[NumLights].green = green * fade;
				aLights[NumLights].blue = blue * fade;
			}
			NumLights++;
		}
	}
}

float
CPointLights::GenerateLightsAffectingObject(Const CVector *objCoors)
{
	int i;
	float ret;
	CVector dist;
	float radius, distance;

	ret = 1.0f;
	for(i = 0; i < NumLights; i++){
		if(aLights[i].type == LIGHT_FOGONLY || aLights[i].type == LIGHT_FOGONLY_ALWAYS)
			continue;

		// same weird distance calculation. simplified here
		dist = aLights[i].coors - *objCoors;
		radius = aLights[i].radius;
		if(Abs(dist.x) < radius &&
		   Abs(dist.y) < radius &&
		   Abs(dist.z) < radius){

			distance = dist.Magnitude();
			if(distance < radius){

				float distNorm = distance/radius;
				if(aLights[i].type == LIGHT_DARKEN){
					// darken the object the closer it is
					ret *= distNorm;
				}else{
					float intensity;
					// distance fade
					if(distNorm < 0.5f)
						intensity = 1.0f;
					else
						intensity = 1.0f - (distNorm - 0.5f)/(1.0f - 0.5f);

					if(distance != 0.0f){
						CVector dir = dist / distance;

						if(aLights[i].type == LIGHT_DIRECTIONAL){
							float dot = -DotProduct(dir, aLights[i].dir);
							intensity *= Max((dot-0.5f)*2.0f, 0.0f);
						}

						if(intensity > 0.0f)
							AddAnExtraDirectionalLight(Scene.world,
								dir.x, dir.y, dir.z,
								aLights[i].red*intensity, aLights[i].green*intensity, aLights[i].blue*intensity);
					}
				}
			}
		}
	}

	return ret;
}

extern RwRaster *gpPointlightRaster;

void
CPointLights::RemoveLightsAffectingObject(void)
{
	RemoveExtraDirectionalLights(Scene.world);
}

// for directional fog
#define FOG_AREA_LENGTH 12.0f
#define FOG_AREA_WIDTH 5.0f
// for pointlight fog
#define FOG_AREA_RADIUS 9.0f

float FogSizes[8] = { 1.3f, 2.0f, 1.7f, 2.0f, 1.4f, 2.1f, 1.5f, 2.3f };

void
CPointLights::RenderFogEffect(void)
{
	int i;
	float fogginess;
	CColPoint point;
	CEntity *entity;
	float xmin, ymin;
	float xmax, ymax;
	int16 xi, yi;
	CVector spriteCoors;
	float spritew, spriteh;

	PUSH_RENDERGROUP("CPointLights::RenderFogEffect");

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpPointlightRaster);

	for(i = 0; i < NumLights; i++){
		if(aLights[i].fogType != FOG_NORMAL && aLights[i].fogType != FOG_ALWAYS)
			continue;

		fogginess = aLights[i].fogType == FOG_NORMAL ? CWeather::Foggyness : 1.0f;
		if(fogginess == 0.0f)
			continue;

		if(aLights[i].type == LIGHT_DIRECTIONAL){

			// TODO: test this. haven't found directional fog so far

			float coors2X = aLights[i].coors.x + FOG_AREA_LENGTH*aLights[i].dir.x;
			float coors2Y = aLights[i].coors.y + FOG_AREA_LENGTH*aLights[i].dir.y;

			if(coors2X < aLights[i].coors.x){
				xmin = coors2X;
				xmax = aLights[i].coors.x;
			}else{
				xmax = coors2X;
				xmin = aLights[i].coors.x;
			}
			if(coors2Y < aLights[i].coors.y){
				ymin = coors2Y;
				ymax = aLights[i].coors.y;
			}else{
				ymax = coors2Y;
				ymin = aLights[i].coors.y;
			}

			xmin -= 5.0f;
			ymin -= 5.0f;
			xmax += 5.0f;
			ymax += 5.0f;

			for(xi = (int16)xmin - (int16)xmin % 4; xi <= (int16)xmax + 4; xi += 4){
				for(yi = (int16)ymin - (int16)ymin % 4; yi <= (int16)ymax + 4; yi += 4){
					// Some kind of pseudo random number?
					int r = (xi ^ yi)>>2 & 0xF;
					if((r & 1) == 0)
						continue;

					// Check if fog effect is close enough to directional line in x and y
					float dx = xi - aLights[i].coors.x;
					float dy = yi - aLights[i].coors.y;
					float dot = dx*aLights[i].dir.x + dy*aLights[i].dir.y;
					float distsq = sq(dx) + sq(dy);
					float linedistsq = distsq - sq(dot);
					if(dot > 0.0f && dot < FOG_AREA_LENGTH && linedistsq < sq(FOG_AREA_WIDTH)){
						CVector fogcoors(xi, yi, aLights[i].coors.z + 10.0f);
						if(CWorld::ProcessVerticalLine(fogcoors, fogcoors.z - 20.0f,
								point, entity, true, false, false, false, true, false, nil)){
							// Now same check again in xyz
							fogcoors.z = point.point.z + 1.3f;
							// actually we don't have to recalculate x and y, but the game does it that way
							dx = xi - aLights[i].coors.x;
							dy = yi - aLights[i].coors.y;
							float dz = fogcoors.z - aLights[i].coors.z;
							dot = dx*aLights[i].dir.x + dy*aLights[i].dir.y + dz*aLights[i].dir.z;
							distsq = sq(dx) + sq(dy) + sq(dz);
							linedistsq = distsq - sq(dot);
							if(dot > 0.0f && dot < FOG_AREA_LENGTH && linedistsq < sq(FOG_AREA_WIDTH)){
								float intensity = 158.0f * fogginess;
								// more intensity the smaller the angle
								intensity *= dot/Sqrt(distsq);
								// more intensity the closer to light source
								intensity *= 1.0f - sq(dot/FOG_AREA_LENGTH);
								// more intensity the closer to line
								intensity *= 1.0f - sq(Sqrt(linedistsq) / FOG_AREA_WIDTH);

								if(CSprite::CalcScreenCoors(fogcoors, &spriteCoors, &spritew, &spriteh, true)){
									float rotation = (CTimer::GetTimeInMilliseconds()&0x1FFF) * 2*3.14f / 0x2000;
									float size = FogSizes[r>>1];
									CSprite::RenderOneXLUSprite_Rotate_Aspect(spriteCoors.x, spriteCoors.y, spriteCoors.z,
										spritew * size, spriteh * size,
										aLights[i].red * intensity, aLights[i].green * intensity, aLights[i].blue * intensity,
										intensity, 1/spriteCoors.z, rotation, 255);
								}
							}
						}
					}
				}
			}

		}else if(aLights[i].type == LIGHT_POINT || aLights[i].type == LIGHT_FOGONLY || aLights[i].type == LIGHT_FOGONLY_ALWAYS){
			if(CWorld::ProcessVerticalLine(aLights[i].coors, aLights[i].coors.z - 20.0f,
					point, entity, true, false, false, false, true, false, nil)){

				xmin = aLights[i].coors.x - FOG_AREA_RADIUS;
				ymin = aLights[i].coors.y - FOG_AREA_RADIUS;
				xmax = aLights[i].coors.x + FOG_AREA_RADIUS;
				ymax = aLights[i].coors.y + FOG_AREA_RADIUS;

				for(xi = (int16)xmin - (int16)xmin % 2; xi <= (int16)xmax + 2; xi += 2){
					for(yi = (int16)ymin - (int16)ymin % 2; yi <= (int16)ymax + 2; yi += 2){
						// Some kind of pseudo random number?
						int r = (xi ^ yi)>>1 & 0xF;
						if((r & 1) == 0)
							continue;

						float dx = xi - aLights[i].coors.x;
						float dy = yi - aLights[i].coors.y;
						float lightdist = Sqrt(sq(dx) + sq(dy));
						if(lightdist < FOG_AREA_RADIUS){
							dx = xi - TheCamera.GetPosition().x;
							dy = yi - TheCamera.GetPosition().y;
							float camdist = Sqrt(sq(dx) + sq(dy));
							if(camdist < MAX_DIST){
								float intensity;
								// distance fade
								if(camdist < MAX_DIST/2)
									intensity = 1.0f;
								else
									intensity = 1.0f - (camdist - MAX_DIST/2) / (MAX_DIST/2);
								intensity *= 132.0f * fogginess;
								// more intensity the closer to light source
								intensity *= 1.0f - sq(lightdist / FOG_AREA_RADIUS);

								CVector fogcoors(xi, yi, point.point.z + 1.6f);
								if(CSprite::CalcScreenCoors(fogcoors, &spriteCoors, &spritew, &spriteh, true)){
									float rotation = (CTimer::GetTimeInMilliseconds()&0x3FFF) * 2*3.14f / 0x4000;
									float size = FogSizes[r>>1];
									CSprite::RenderOneXLUSprite_Rotate_Aspect(spriteCoors.x, spriteCoors.y, spriteCoors.z,
										spritew * size, spriteh * size,
										aLights[i].red * intensity, aLights[i].green * intensity, aLights[i].blue * intensity,
										intensity, 1/spriteCoors.z, rotation, 255);
								}
							}
						}
					}
				}
			}
		}
	}

	POP_RENDERGROUP();
}
