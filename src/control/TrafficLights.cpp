#include "common.h"

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

// TODO: figure out the meaning of this
enum { SOME_FLAG = 0x80 };

void
CTrafficLights::DisplayActualLight(CEntity *ent)
{
	if(ent->GetUp().z < 0.96f || ent->bRenderDamaged)
		return;

	int phase;
	if(FindTrafficLightType(ent) == 1)
		phase = LightForCars1();
	else
		phase = LightForCars2();

	int i;
	CBaseModelInfo *mi = CModelInfo::GetModelInfo(ent->GetModelIndex());
	float x = mi->Get2dEffect(0)->pos.x;
	float yMin = mi->Get2dEffect(0)->pos.y;
	float yMax = mi->Get2dEffect(0)->pos.y;
	float zMin = mi->Get2dEffect(0)->pos.z;
	float zMax = mi->Get2dEffect(0)->pos.z;
	for(i = 1; i < 6; i++){
		assert(mi->Get2dEffect(i));
		yMin = Min(yMin, mi->Get2dEffect(i)->pos.y);
		yMax = Max(yMax, mi->Get2dEffect(i)->pos.y);
		zMin = Min(zMin, mi->Get2dEffect(i)->pos.z);
		zMax = Max(zMax, mi->Get2dEffect(i)->pos.z);
	}

	CVector pos1, pos2;
	uint8 r, g;
	int id;
	switch(phase){
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
		pos1 = ent->GetMatrix() * CVector(x, yMax, (zMin+zMax)/2.0f);
		pos2 = ent->GetMatrix() * CVector(x, yMin, (zMin+zMax)/2.0f);
		id = 1;
		break;
	case CAR_LIGHTS_RED:
	default:
		r = 255;
		g = 0;
		pos1 = ent->GetMatrix() * CVector(x, yMax, zMax);
		pos2 = ent->GetMatrix() * CVector(x, yMin, zMax);
		id = 2;
		break;
	}

	if(CClock::GetHours() > 19 || CClock::GetHours() < 6 || CWeather::Foggyness > 0.05f)
		CPointLights::AddLight(CPointLights::LIGHT_POINT,
			pos1, CVector(0.0f, 0.0f, 0.0f), 8.0f,
			r/255.0f, g/255.0f, 0/255.0f, CPointLights::FOG_NORMAL, true);

	CShadows::StoreStaticShadow((uintptr)ent,
		SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos1,
		8.0f, 0.0f, 0.0f, -8.0f, 128,
		r*CTimeCycle::GetLightOnGroundBrightness()/8.0f,
		g*CTimeCycle::GetLightOnGroundBrightness()/8.0f,
		0*CTimeCycle::GetLightOnGroundBrightness()/8.0f,
		12.0f, 1.0f, 40.0f, false, 0.0f);

	if(DotProduct(TheCamera.GetForward(), ent->GetForward()) < 0.0f)
		CCoronas::RegisterCorona((uintptr)ent + id,
			r*CTimeCycle::GetSpriteBrightness()*0.7f,
			g*CTimeCycle::GetSpriteBrightness()*0.7f,
			0*CTimeCycle::GetSpriteBrightness()*0.7f,
			255,
			pos1, 1.75f*CTimeCycle::GetSpriteSize(), 50.0f,
			CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
			CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	else
		CCoronas::RegisterCorona((uintptr)ent + id + 3,
			r*CTimeCycle::GetSpriteBrightness()*0.7f,
			g*CTimeCycle::GetSpriteBrightness()*0.7f,
			0*CTimeCycle::GetSpriteBrightness()*0.7f,
			255,
			pos2, 1.75f*CTimeCycle::GetSpriteSize(), 50.0f,
			CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
			CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);

	CBrightLights::RegisterOne(pos1, ent->GetUp(), ent->GetRight(), CVector(0.0f, 0.0f, 0.0f), id + BRIGHTLIGHT_TRAFFIC_GREEN);
	CBrightLights::RegisterOne(pos2, ent->GetUp(), -ent->GetRight(), CVector(0.0f, 0.0f, 0.0f), id + BRIGHTLIGHT_TRAFFIC_GREEN);

	static const float top = -0.127f;
	static const float bot = -0.539f;
	static const float mid = bot + (top-bot)/3.0f;
	static const float left = 1.256f;
	static const float right = 0.706f;
	phase = CTrafficLights::LightForPeds();
	if(phase == PED_LIGHTS_DONT_WALK){
		CVector p0(2.7f, right, top);
		CVector p1(2.7f, left, top);
		CVector p2(2.7f, right, mid);
		CVector p3(2.7f, left, mid);
		CShinyTexts::RegisterOne(ent->GetMatrix()*p0, ent->GetMatrix()*p1, ent->GetMatrix()*p2, ent->GetMatrix()*p3,
			1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,
			SHINYTEXT_WALK, 255, 0, 0, 60.0f);
	}else if(phase == PED_LIGHTS_WALK || CTimer::GetTimeInMilliseconds() & 0x100){
		CVector p0(2.7f, right, mid);
		CVector p1(2.7f, left, mid);
		CVector p2(2.7f, right, bot);
		CVector p3(2.7f, left, bot);
		CShinyTexts::RegisterOne(ent->GetMatrix()*p0, ent->GetMatrix()*p1, ent->GetMatrix()*p2, ent->GetMatrix()*p3,
			1.0f, 0.5f,  0.0f, 0.5f,  1.0f, 1.0f,  0.0f, 1.0f,
			SHINYTEXT_WALK, 255, 255, 255, 60.0f);
	}
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

			// Check cars
			for(i = 0; i < ThePaths.m_numCarPathLinks; i++){
				CVector2D dist = ThePaths.m_carPathLinks[i].GetPosition() - light->GetPosition();
				float dotY = Abs(DotProduct2D(dist, light->GetForward()));	// forward is direction of car light
				float dotX = DotProduct2D(dist, light->GetRight());	// towards base of light
				// it has to be on the correct side of the node and also not very far away
				if(dotX < 0.0f && dotX > -15.0f && dotY < 3.0f){
					float dz = ThePaths.m_pathNodes[ThePaths.m_carPathLinks[i].pathNodeIndex].GetZ() -
						light->GetPosition().z;
					if(dz < 15.0f){
						ThePaths.m_carPathLinks[i].trafficLightType = FindTrafficLightType(light);
						// Find two neighbour nodes of this one
						int n1 = -1;
						int n2 = -1;
						for(j = 0; j < ThePaths.m_numPathNodes; j++)
							for(l = 0; l < ThePaths.m_pathNodes[j].numLinks; l++)
								if(ThePaths.m_carPathConnections[ThePaths.m_pathNodes[j].firstLink + l] == i){
									if(n1 == -1)
										n1 = j;
									else
										n2 = j;
								}
						// What's going on here?
						if(ThePaths.m_pathNodes[n1].numLinks <= ThePaths.m_pathNodes[n2].numLinks)
							n1 = n2;
						if(ThePaths.m_carPathLinks[i].pathNodeIndex != n1)
							ThePaths.m_carPathLinks[i].trafficLightType |= SOME_FLAG;
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

	node = vehicle->AutoPilot.m_nNextPathNodeInfo;
	type = ThePaths.m_carPathLinks[node].trafficLightType;
	if(type){
		if((type & SOME_FLAG || ThePaths.m_carPathLinks[node].pathNodeIndex == vehicle->AutoPilot.m_nNextRouteNode) &&
		   (!(type & SOME_FLAG) || ThePaths.m_carPathLinks[node].pathNodeIndex != vehicle->AutoPilot.m_nNextRouteNode))
			if(alwaysStop ||
			   (type&~SOME_FLAG) == 1 && LightForCars1() != CAR_LIGHTS_GREEN ||
			   (type&~SOME_FLAG) == 2 && LightForCars2() != CAR_LIGHTS_GREEN){
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
	if(type){
		if((type & SOME_FLAG || ThePaths.m_carPathLinks[node].pathNodeIndex == vehicle->AutoPilot.m_nCurrentRouteNode) &&
		   (!(type & SOME_FLAG) || ThePaths.m_carPathLinks[node].pathNodeIndex != vehicle->AutoPilot.m_nCurrentRouteNode))
			if(alwaysStop ||
			   (type&~SOME_FLAG) == 1 && LightForCars1() != CAR_LIGHTS_GREEN ||
			   (type&~SOME_FLAG) == 2 && LightForCars2() != CAR_LIGHTS_GREEN){
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
		if(type){
			if((type & SOME_FLAG || ThePaths.m_carPathLinks[node].pathNodeIndex == vehicle->AutoPilot.m_nPrevRouteNode) &&
			   (!(type & SOME_FLAG) || ThePaths.m_carPathLinks[node].pathNodeIndex != vehicle->AutoPilot.m_nPrevRouteNode))
				if(alwaysStop ||
				   (type&~SOME_FLAG) == 1 && LightForCars1() != CAR_LIGHTS_GREEN ||
				   (type&~SOME_FLAG) == 2 && LightForCars2() != CAR_LIGHTS_GREEN){
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
