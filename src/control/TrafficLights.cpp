#include "common.h"

#include "vehicle_enums.h" //needs to be before Vehicle.h

#include "Camera.h"
#include "Clock.h"
#include "Coronas.h"
#include "General.h"
#include "PathFind.h"
#include "PointLights.h"
#include "Shadows.h"
#include "SpecialFX.h"
#include "Timecycle.h"
#include "Timer.h"
#include "TrafficLights.h"
#include "Vehicle.h"
#include "Weather.h"
#include "World.h"

//--MIAMI: file done

bool CTrafficLights::bGreenLightsCheat;

void
CTrafficLights::DisplayActualLight(CEntity *ent)
{
	if(ent->GetUp().z < 0.96f || ent->bRenderDamaged)
		return;

	int phase;
	if(FindTrafficLightType(ent) == 1)
		phase = LightForCars1_Visual();
	else
		phase = LightForCars2_Visual();

	int i, m = ent->GetModelIndex();
	if (MI_TRAFFICLIGHTS == m) {
		CBaseModelInfo* mi = CModelInfo::GetModelInfo(ent->GetModelIndex());
		float x = mi->Get2dEffect(0)->pos.x;
		float yMin = mi->Get2dEffect(0)->pos.y;
		float yMax = mi->Get2dEffect(0)->pos.y;
		float zMin = mi->Get2dEffect(0)->pos.z;
		float zMax = mi->Get2dEffect(0)->pos.z;
		for (i = 1; i < 6; i++) {
			assert(mi->Get2dEffect(i));
			yMin = Min(yMin, mi->Get2dEffect(i)->pos.y);
			yMax = Max(yMax, mi->Get2dEffect(i)->pos.y);
			zMin = Min(zMin, mi->Get2dEffect(i)->pos.z);
			zMax = Max(zMax, mi->Get2dEffect(i)->pos.z);
		}

		CVector pos1, pos2;
		uint8 r, g;
		int id;
		switch (phase) {
		case CAR_LIGHTS_GREEN:
			r = 0;
			g = 255;
			pos1 = ent->GetMatrix() * CVector(x, yMax, zMin);
			pos2 = ent->GetMatrix() * CVector(x, yMin, zMin);
			id = 0;
			break;
		case CAR_LIGHTS_YELLOW:
			r = 255;
			g = 128;
			pos1 = ent->GetMatrix() * CVector(x, yMax, (zMin + zMax) / 2.0f);
			pos2 = ent->GetMatrix() * CVector(x, yMin, (zMin + zMax) / 2.0f);
			id = 1;
			break;
		case CAR_LIGHTS_RED:
			r = 255;
			g = 0;
			pos1 = ent->GetMatrix() * CVector(x, yMax, zMax);
			pos2 = ent->GetMatrix() * CVector(x, yMin, zMax);
			id = 2;
			break;
		default:
			r = 0;
			g = 0;
			pos1 = ent->GetMatrix() * CVector(x, yMax, (zMin + zMax) / 2.0f);
			pos2 = ent->GetMatrix() * CVector(x, yMin, (zMin + zMax) / 2.0f);
			id = -1;
			break;
		}

		if (CWeather::TrafficLightBrightness > 0.5f)
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
				pos1, CVector(0.0f, 0.0f, 0.0f), 8.0f,
				r / 255.0f, g / 255.0f, 0 / 255.0f, CPointLights::FOG_NORMAL, true);

		if (CWeather::TrafficLightBrightness > 0.05f)
			CShadows::StoreStaticShadow((uintptr)ent,
				SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos1,
				8.0f, 0.0f, 0.0f, -8.0f, 128,
				r * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				g * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				0 * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				12.0f, 1.0f, 40.0f, false, 0.0f);

		if (DotProduct(TheCamera.GetForward(), ent->GetForward()) < 0.0f)
			CCoronas::RegisterCorona((uintptr)ent + id,
				r * CTimeCycle::GetSpriteBrightness() * 0.7f,
				g * CTimeCycle::GetSpriteBrightness() * 0.7f,
				0 * CTimeCycle::GetSpriteBrightness() * 0.7f,
				255,
				pos1, 1.75f * CTimeCycle::GetSpriteSize(), 50.0f,
				CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		else
			CCoronas::RegisterCorona((uintptr)ent + id + 3,
				r * CTimeCycle::GetSpriteBrightness() * 0.7f,
				g * CTimeCycle::GetSpriteBrightness() * 0.7f,
				0 * CTimeCycle::GetSpriteBrightness() * 0.7f,
				255,
				pos2, 1.75f * CTimeCycle::GetSpriteSize(), 50.0f,
				CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);

		CBrightLights::RegisterOne(pos1, ent->GetUp(), ent->GetRight(), CVector(0.0f, 0.0f, 0.0f), id + BRIGHTLIGHT_TRAFFIC_GREEN);
		CBrightLights::RegisterOne(pos2, ent->GetUp(), -ent->GetRight(), CVector(0.0f, 0.0f, 0.0f), id + BRIGHTLIGHT_TRAFFIC_GREEN);
	}
	else if (MI_TRAFFICLIGHTS_VERTICAL == m) {
		CBaseModelInfo* mi = CModelInfo::GetModelInfo(ent->GetModelIndex());
		float x = mi->Get2dEffect(0)->pos.x;
		float yMin = mi->Get2dEffect(0)->pos.y;
		float yMax = mi->Get2dEffect(0)->pos.y;
		float zMin = mi->Get2dEffect(0)->pos.z;
		float zMax = mi->Get2dEffect(0)->pos.z;
		for (i = 1; i < 6; i++) {
			assert(mi->Get2dEffect(i));
			yMin = Min(yMin, mi->Get2dEffect(i)->pos.y);
			yMax = Max(yMax, mi->Get2dEffect(i)->pos.y);
			zMin = Min(zMin, mi->Get2dEffect(i)->pos.z);
			zMax = Max(zMax, mi->Get2dEffect(i)->pos.z);
		}

		CVector pos1;
		uint8 r, g;
		int id;
		switch (phase) {
		case CAR_LIGHTS_GREEN:
			r = 0;
			g = 255;
			pos1 = ent->GetMatrix() * mi->Get2dEffect(2)->pos;
			id = 0;
			break;
		case CAR_LIGHTS_YELLOW:
			r = 255;
			g = 128;
			pos1 = ent->GetMatrix() * mi->Get2dEffect(1)->pos;
			id = 1;
			break;
		case CAR_LIGHTS_RED:
			r = 255;
			g = 0;
			pos1 = ent->GetMatrix() * mi->Get2dEffect(0)->pos;
			id = 2;
			break;
		default:
			r = 0;
			g = 0;
			pos1 = ent->GetMatrix() * mi->Get2dEffect(1)->pos;
			id = -1;
			break;
		}

		CBrightLights::RegisterOne(pos1, ent->GetUp(), ent->GetRight(), CVector(0.0f, 0.0f, 0.0f), id + BRIGHTLIGHT_TRAFFIC_GREEN);

		if (CWeather::TrafficLightBrightness > 0.5f)
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
				pos1, CVector(0.0f, 0.0f, 0.0f), 8.0f,
				r / 255.0f, g / 255.0f, 0 / 255.0f, CPointLights::FOG_NORMAL, true);

		if (CWeather::TrafficLightBrightness > 0.05f)
			CShadows::StoreStaticShadow((uintptr)ent,
				SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos1,
				8.0f, 0.0f, 0.0f, -8.0f, 128,
				r * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				g * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				0 * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				12.0f, 1.0f, 40.0f, false, 0.0f);

		if (DotProduct(TheCamera.GetForward(), ent->GetForward()) < 0.0f)
			CCoronas::RegisterCorona((uintptr)ent + id,
				r * CTimeCycle::GetSpriteBrightness() * 0.7f,
				g * CTimeCycle::GetSpriteBrightness() * 0.7f,
				0 * CTimeCycle::GetSpriteBrightness() * 0.7f,
				255,
				pos1, 1.75f * CTimeCycle::GetSpriteSize(), 50.0f,
				CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}
	else if (MI_TRAFFICLIGHTS_MIAMI == m || MI_TRAFFICLIGHTS_TWOVERTICAL == m) {
		CBaseModelInfo* mi = CModelInfo::GetModelInfo(ent->GetModelIndex());
		CVector pos1, pos2;
		uint8 r, g;
		int id;
		if (MI_TRAFFICLIGHTS_MIAMI == m) {
			switch (phase) {
			case CAR_LIGHTS_GREEN:
				r = 0;
				g = 255;
				pos1 = ent->GetMatrix() * mi->Get2dEffect(4)->pos;
				pos2 = ent->GetMatrix() * mi->Get2dEffect(5)->pos;
				id = 0;
				break;
			case CAR_LIGHTS_YELLOW:
				r = 255;
				g = 128;
				pos1 = ent->GetMatrix() * mi->Get2dEffect(2)->pos;
				pos2 = ent->GetMatrix() * mi->Get2dEffect(3)->pos;
				id = 1;
				break;
			case CAR_LIGHTS_RED:
				r = 255;
				g = 0;
				pos1 = ent->GetMatrix() * mi->Get2dEffect(0)->pos;
				pos2 = ent->GetMatrix() * mi->Get2dEffect(1)->pos;
				id = 2;
				break;
			default:
				r = 0;
				g = 0;
				pos1 = ent->GetMatrix() * mi->Get2dEffect(2)->pos;
				pos2 = ent->GetMatrix() * mi->Get2dEffect(3)->pos;
				id = -1;
				break;
			}
		}
		else {
			switch (phase) {
			case CAR_LIGHTS_GREEN:
				r = 0;
				g = 255;
				pos1 = ent->GetMatrix() * mi->Get2dEffect(2)->pos;
				pos2 = ent->GetMatrix() * mi->Get2dEffect(5)->pos;
				id = 0;
				break;
			case CAR_LIGHTS_YELLOW:
				r = 255;
				g = 128;
				pos1 = ent->GetMatrix() * mi->Get2dEffect(1)->pos;
				pos2 = ent->GetMatrix() * mi->Get2dEffect(4)->pos;
				id = 1;
				break;
			case CAR_LIGHTS_RED:
				r = 255;
				g = 0;
				pos1 = ent->GetMatrix() * mi->Get2dEffect(0)->pos;
				pos2 = ent->GetMatrix() * mi->Get2dEffect(3)->pos;
				id = 2;
				break;
			default:
				r = 0;
				g = 0;
				pos1 = ent->GetMatrix() * mi->Get2dEffect(1)->pos;
				pos2 = ent->GetMatrix() * mi->Get2dEffect(4)->pos;
				id = -1;
				break;
			}
		}

		CVector pos = (pos1 + pos2) / 2;
		if (id >= 0) {
			CBrightLights::RegisterOne(pos1, ent->GetUp(), ent->GetRight(), CVector(0.0f, 0.0f, 0.0f), id + BRIGHTLIGHT_TRAFFIC_GREEN);
			CBrightLights::RegisterOne(pos2, ent->GetUp(), ent->GetRight(), CVector(0.0f, 0.0f, 0.0f), id + BRIGHTLIGHT_TRAFFIC_GREEN);
		}

		if (CWeather::TrafficLightBrightness > 0.5f)
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
				pos, CVector(0.0f, 0.0f, 0.0f), 8.0f,
				r / 255.0f, g / 255.0f, 0 / 255.0f, CPointLights::FOG_NORMAL, true);

		if (CWeather::TrafficLightBrightness > 0.05f)
			CShadows::StoreStaticShadow((uintptr)ent,
				SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos,
				8.0f, 0.0f, 0.0f, -8.0f, 128,
				r * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				g * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				0 * CTimeCycle::GetLightOnGroundBrightness() * CWeather::TrafficLightBrightness / 8.0f,
				12.0f, 1.0f, 40.0f, false, 0.0f);

		if (id >= 0) {
			if (DotProduct(TheCamera.GetForward(), ent->GetForward()) < 0.0f)
				CCoronas::RegisterCorona((uintptr)ent + id,
					r * CTimeCycle::GetSpriteBrightness() * 0.7f,
					g * CTimeCycle::GetSpriteBrightness() * 0.7f,
					0 * CTimeCycle::GetSpriteBrightness() * 0.7f,
					255,
					pos1, 1.75f * CTimeCycle::GetSpriteSize(), 50.0f,
					CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
					CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
			else
				CCoronas::RegisterCorona((uintptr)ent + id,
					r * CTimeCycle::GetSpriteBrightness() * 0.7f,
					g * CTimeCycle::GetSpriteBrightness() * 0.7f,
					0 * CTimeCycle::GetSpriteBrightness() * 0.7f,
					255,
					pos2, 1.75f * CTimeCycle::GetSpriteSize(), 50.0f,
					CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
					CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		}
	}
}

bool DoesLineSegmentIntersect(float l1x1, float l1y1, float l1x2, float l1y2, float l2x1, float l2y1, float l2x2, float l2y2)
{
	return ((l2y2 - l1y1) * (l1x2 - l1x1) + (l1x1 - l2x2) * (l1y2 - l1y1)) *
		((l2y1 - l1y1) * (l1x2 - l1x1) + (l1x1 - l2x1) * (l1y2 - l1y1)) <= 0.0f &&
		((l1y2 - l2y1) * (l2x2 - l2x1) + (l2y2 - l2y1) * (l2x1 - l1x2)) *
		((l1y1 - l2y1) * (l2x2 - l2x1) + (l2y2 - l2y1) * (l2x1 - l1x1)) <= 0.0f;
}

void
CTrafficLights::ScanForLightsOnMap(void)
{
	int x, y;
	int i, j, k, l;
	CPtrNode *node;

	for(x = 0; x < NUMSECTORS_X; x++)
	for(y = 0; y < NUMSECTORS_Y; y++){
		CPtrList &list = CWorld::GetSector(x, y)->m_lists[ENTITYLIST_DUMMIES];
		for(node = list.first; node; node = node->next){
			CEntity *light = (CEntity*)node->item;
			if (!IsTrafficLight(light->GetModelIndex()))
				continue;

			CVector pos1 = light->GetMatrix() * CVector(17.0f, 0.0f, 0.0f);
			CVector pos2 = light->GetMatrix() * CVector(-15.0f, 0.0f, 0.0f);

			// Check cars
			for(i = 0; i < ThePaths.m_numCarPathNodes; i++){
				if ((ThePaths.m_pathNodes[i].GetPosition() - pos1).MagnitudeSqr() >= SQR(100.0f))
					continue;
				for (j = 0; j < ThePaths.m_pathNodes[i].numLinks; j++){
					int con = ThePaths.ConnectedNode(ThePaths.m_pathNodes[i].firstLink + j);
					if (i < con) {
						CVector i_pos = ThePaths.m_pathNodes[i].GetPosition();
						CVector con_pos = ThePaths.m_pathNodes[con].GetPosition();
						if (Abs(pos1.z - (i_pos.z + con_pos.z) / 2) < 10.0f &&
							DoesLineSegmentIntersect(pos1.x, pos1.y, pos2.x, pos2.y, i_pos.x, i_pos.y, con_pos.x, con_pos.y)) {
							//debug("Setting up light: nodes %f %f %f - %f %f %f, light %f %f %f - %f %f %f\n", i_pos.x, i_pos.y, i_pos.z, con_pos.x, con_pos.y, con_pos.z, pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z);
							int link = ThePaths.m_carPathConnections[ThePaths.m_pathNodes[i].firstLink + j];
							ThePaths.m_carPathLinks[link].trafficLightType = FindTrafficLightType(light);
							if (ThePaths.m_pathNodes[i].numLinks > ThePaths.m_pathNodes[con].numLinks)
								con = i;
							if (ThePaths.m_carPathLinks[link].pathNodeIndex != con)
								ThePaths.m_carPathLinks[link].trafficLightDirection = true;
						}
					}
				}
			}

			// Check peds
			for(i = ThePaths.m_numCarPathNodes; i < ThePaths.m_numPathNodes; i++){
				float dist1, dist2;
				dist1 = Abs(ThePaths.m_pathNodes[i].GetX() - light->GetPosition().x) +
					Abs(ThePaths.m_pathNodes[i].GetY() - light->GetPosition().y);
				if(dist1 < 50.0f){
					for(l = 0; l < ThePaths.m_pathNodes[i].numLinks; l++){
						j = ThePaths.m_pathNodes[i].firstLink + l;
						if(ThePaths.ConnectionCrossesRoad(j)){
							k = ThePaths.ConnectedNode(j);
							dist2 = Abs(ThePaths.m_pathNodes[k].GetX() - light->GetPosition().x) +
								Abs(ThePaths.m_pathNodes[k].GetY() - light->GetPosition().y);
							if(dist1 < 15.0f || dist2 < 15.0f)
								ThePaths.ConnectionSetTrafficLight(j);
						}
					}
				}
			}
		}
	}
}

bool
CTrafficLights::ShouldCarStopForLight(CVehicle *vehicle, bool alwaysStop)
{
	int node, type;
	bool direction;

	node = vehicle->AutoPilot.m_nNextPathNodeInfo;
	type = ThePaths.m_carPathLinks[node].trafficLightType;
	direction = ThePaths.m_carPathLinks[node].trafficLightDirection;

	if(type){
		if((direction || ThePaths.m_carPathLinks[node].pathNodeIndex == vehicle->AutoPilot.m_nNextRouteNode) &&
		   (!direction || ThePaths.m_carPathLinks[node].pathNodeIndex != vehicle->AutoPilot.m_nNextRouteNode))
			if(alwaysStop ||
			   type == 1 && LightForCars1() != CAR_LIGHTS_GREEN ||
			   type == 2 && LightForCars2() != CAR_LIGHTS_GREEN){
				float dist = DotProduct2D(CVector2D(vehicle->GetPosition()) - ThePaths.m_carPathLinks[node].GetPosition(),
						ThePaths.m_carPathLinks[node].GetDirection());
				if(vehicle->AutoPilot.m_nNextDirection == -1){
					if(dist > 0.0f && dist < 8.0f)
						return true;
				}else{
					if(dist < 0.0f && dist > -8.0f)
						return true;
				}
			}
	}

	node = vehicle->AutoPilot.m_nCurrentPathNodeInfo;
	type = ThePaths.m_carPathLinks[node].trafficLightType;
	direction = ThePaths.m_carPathLinks[node].trafficLightDirection;
	if(type){
		if((direction || ThePaths.m_carPathLinks[node].pathNodeIndex == vehicle->AutoPilot.m_nCurrentRouteNode) &&
		   (!direction || ThePaths.m_carPathLinks[node].pathNodeIndex != vehicle->AutoPilot.m_nCurrentRouteNode))
			if(alwaysStop ||
			   type == 1 && LightForCars1() != CAR_LIGHTS_GREEN ||
			   type == 2 && LightForCars2() != CAR_LIGHTS_GREEN){
				float dist = DotProduct2D(CVector2D(vehicle->GetPosition()) - ThePaths.m_carPathLinks[node].GetPosition(),
						ThePaths.m_carPathLinks[node].GetDirection());
				if(vehicle->AutoPilot.m_nCurrentDirection == -1){
					if(dist > 0.0f && dist < 8.0f)
						return true;
				}else{
					if(dist < 0.0f && dist > -8.0f)
						return true;
				}
			}
	}

	if(vehicle->GetStatus() == STATUS_PHYSICS){
		node = vehicle->AutoPilot.m_nPreviousPathNodeInfo;
		type = ThePaths.m_carPathLinks[node].trafficLightType;
		direction = ThePaths.m_carPathLinks[node].trafficLightDirection;
		if(type){
			if((direction || ThePaths.m_carPathLinks[node].pathNodeIndex == vehicle->AutoPilot.m_nPrevRouteNode) &&
			   (!direction || ThePaths.m_carPathLinks[node].pathNodeIndex != vehicle->AutoPilot.m_nPrevRouteNode))
				if(alwaysStop ||
				   type == 1 && LightForCars1() != CAR_LIGHTS_GREEN ||
				   type == 2 && LightForCars2() != CAR_LIGHTS_GREEN){
					float dist = DotProduct2D(CVector2D(vehicle->GetPosition()) - ThePaths.m_carPathLinks[node].GetPosition(),
							ThePaths.m_carPathLinks[node].GetDirection());
					if(vehicle->AutoPilot.m_nPreviousDirection == -1){
						if(dist > 0.0f && dist < 6.0f)
							return true;
					}else{
						if(dist < 0.0f && dist > -6.0f)
							return true;
					}
				}
		}
	}

	return false;
}

bool
CTrafficLights::ShouldCarStopForBridge(CVehicle *vehicle)
{
#ifdef GTA_BRIDGE
	return ThePaths.m_carPathLinks[vehicle->AutoPilot.m_nNextPathNodeInfo].bBridgeLights &&
		!ThePaths.m_carPathLinks[vehicle->AutoPilot.m_nCurrentPathNodeInfo].bBridgeLights;
#else
	return false;
#endif
}

int
CTrafficLights::FindTrafficLightType(CEntity *light)
{
	float orientation = RADTODEG(CGeneral::GetATanOfXY(light->GetForward().x, light->GetForward().y));
	if((orientation > 60.0f && orientation < 60.0f + 90.0f) ||
	   (orientation > 240.0f && orientation < 240.0f + 90.0f))
		return 1;
	return 2;
}

uint8
CTrafficLights::LightForPeds(void)
{
	uint32 period = CTimer::GetTimeInMilliseconds() % 16384;

	if(period < 12000)
		return PED_LIGHTS_DONT_WALK;
	else if(period < 16384 - 1000)
		return PED_LIGHTS_WALK;
	else
		return PED_LIGHTS_WALK_BLINK;
}

uint8
CTrafficLights::LightForCars1(void)
{
	if (CWeather::Wind > 1.1f)
		return CAR_LIGHTS_GREEN;

	if (bGreenLightsCheat)
		return CAR_LIGHTS_GREEN;

	uint32 period = CTimer::GetTimeInMilliseconds() % 16384;

	if(period < 5000)
		return CAR_LIGHTS_GREEN;
	else if(period < 5000 + 1000)
		return CAR_LIGHTS_YELLOW;
	else
		return CAR_LIGHTS_RED;
}

uint8
CTrafficLights::LightForCars2(void)
{
	if (CWeather::Wind > 1.1f)
		return CAR_LIGHTS_GREEN;

	if (bGreenLightsCheat)
		return CAR_LIGHTS_GREEN;

	uint32 period = CTimer::GetTimeInMilliseconds() % 16384;

	if(period < 6000)
		return CAR_LIGHTS_RED;
	else if(period < 12000 - 1000)
		return CAR_LIGHTS_GREEN;
	else if(period < 12000)
		return CAR_LIGHTS_YELLOW;
	else
		return CAR_LIGHTS_RED;
}

uint8
CTrafficLights::LightForCars1_Visual(void)
{
	if (CWeather::Wind <= 1.1f)
		return LightForCars1();
	return (CTimer::GetTimeInMilliseconds() & 0x400 ? CAR_LIGHTS_NONE : CAR_LIGHTS_YELLOW);
}

uint8
CTrafficLights::LightForCars2_Visual(void)
{
	if (CWeather::Wind <= 1.1f)
		return LightForCars2();
	return (CTimer::GetTimeInMilliseconds() & 0x400 ? CAR_LIGHTS_NONE : CAR_LIGHTS_YELLOW);
}
