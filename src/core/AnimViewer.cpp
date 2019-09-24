#include "common.h"
#include "patcher.h"
#include "Font.h"
#include "Pad.h"
#include "Text.h"
#include "main.h"
#include "Timer.h"
#include "DMAudio.h"
#include "FileMgr.h"
#include "Streaming.h"
#include "TxdStore.h"
#include "General.h"
#include "Camera.h"
#include "Vehicle.h"
#include "PlayerSkin.h"
#include "PlayerInfo.h"
#include "World.h"
#include "Renderer.h"
#include "AnimManager.h"
#include "AnimViewer.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "References.h"
#include "PathFind.h"
#include "HandlingMgr.h"
#include "TempColModels.h"
#include "Particle.h"
#include "CdStream.h"
#include "Messages.h"
#include "CarCtrl.h"
#include "FileLoader.h"
#include "ModelIndices.h"
#include "Clock.h"
#include "Timecycle.h"
#include "RpAnimBlend.h"
#include "Shadows.h"
#include "Radar.h"
#include "Hud.h"

int CAnimViewer::animTxdSlot = 0;
CEntity *CAnimViewer::pTarget = nil;

void
CAnimViewer::Render(void) {
	if (pTarget) {
//		pTarget->GetPosition() = CVector(0.0f, 0.0f, 0.0f);
		if (pTarget) {
			pTarget->Render();
			CRenderer::RenderOneNonRoad(pTarget);
		}
	}
}

void
CAnimViewer::Initialise(void) {
	LoadingScreen("Loading the ModelViewer", "", GetRandomSplashScreen());
	animTxdSlot = CTxdStore::AddTxdSlot("generic");
	CTxdStore::Create(animTxdSlot);
	int hudSlot = CTxdStore::AddTxdSlot("hud");
	CTxdStore::LoadTxd(hudSlot, "MODELS/HUD.TXD");
	int particleSlot = CTxdStore::AddTxdSlot("particle");
	CTxdStore::LoadTxd(particleSlot, "MODELS/PARTICLE.TXD");
	CTxdStore::SetCurrentTxd(animTxdSlot);
	CPools::Initialise();
	CReferences::Init();
	TheCamera.Init();
	TheCamera.SetRwCamera(Scene.camera);

	// I didn't get which camera og code selects.
	for (int i = 0; i < 3; i++) {
		TheCamera.Cams[i].Distance = 5.0f;
	}

	gbModelViewer = true;
	CHud::m_Wants_To_Draw_Hud = false;

	ThePaths.Init();
	ThePaths.AllocatePathFindInfoMem(4500);
	CCollision::Init();
	CWorld::Initialise();
	mod_HandlingManager.Initialise();
	CTempColModels::Initialise();
	CAnimManager::Initialise();
	CModelInfo::Initialise();
	CParticle::Initialise();
	CCarCtrl::Init();
	CPedStats::Initialise();
	CMessages::Init();
	CdStreamAddImage("MODELS\\GTA3.IMG");
	CFileLoader::LoadLevel("DATA\\ANIMVIEWER.DAT");
	CStreaming::Init();
	CStreaming::LoadInitialPeds();
	CStreaming::RequestSpecialModel(MI_PLAYER, "player", STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);
	CRenderer::Init();
	CRadar::Initialise();
	CRadar::LoadTextures();
	CVehicleModelInfo::LoadVehicleColours();
	CAnimManager::LoadAnimFiles();
	CWorld::PlayerInFocus = 0;
	CWeapon::InitialiseWeapons();
	CShadows::Init();
	CPed::Initialise();
	CTimer::Initialise();
	CClock::Initialise(60000);
	CTimeCycle::Initialise();
	CCarCtrl::Init();
	CPlayerPed *player = new CPlayerPed();
	player->GetPosition() = CVector(0.0f, 0.0f, 0.0f);
	CWorld::Players[0].m_pPed = player;
	CDraw::SetFOV(120.0f);
	CDraw::ms_fLODDistance = 500.0f;

	int fd = CFileMgr::OpenFile("DATA\\SPECIAL.TXT", "r");
	char animGroup[32], modelName[32];
	if (fd) {
		for (int lineId = 0; lineId < NUM_OF_SPECIAL_CHARS; lineId++) {
			if (!CFileMgr::ReadLine(fd, gString, 255))
				break;

			sscanf(gString, "%s %s", &modelName, &animGroup);
			int groupId;
			for (groupId = 0; groupId < NUM_ANIM_ASSOC_GROUPS; groupId++) {
				if (!strcmp(animGroup, CAnimManager::GetAnimGroupName((AssocGroupId)groupId)))
					break;
			}

			if (groupId != NUM_ANIM_ASSOC_GROUPS)
				((CPedModelInfo*)CModelInfo::GetModelInfo(MI_SPECIAL01 + lineId))->m_animGroup = groupId;

			CStreaming::RequestSpecialChar(lineId, modelName, STREAMFLAGS_DONT_REMOVE);
		}
		CFileMgr::CloseFile(fd);
	} else {
		// From xbox
		CStreaming::RequestSpecialChar(0, "luigi", STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestSpecialChar(1, "joey", STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestSpecialChar(2, "tony", STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestSpecialChar(3, "curly", STREAMFLAGS_DONT_REMOVE);
	}
}

int
LastPedModelId(int modelId)
{
	CBaseModelInfo *model;
	for (int i = modelId; i >= 0; i--) {
		model = CModelInfo::GetModelInfo(i);
		if (model->m_type == MITYPE_PED)
			return i;
	}
	return modelId;
}

int
LastVehicleModelId(int modelId)
{
	CBaseModelInfo* model;
	for (int i = modelId; i >= 0; i--) {
		model = CModelInfo::GetModelInfo(i);
		if (model->m_type == MITYPE_VEHICLE)
			return i;
	}
	return modelId;
}


// It's me that named this.
int
FindMeAModelID(int modelId, int wantedChange)
{
	// Max. 2 trials wasn't here, it's me that added it.

	int tryCount = 2;
	int ogModelId = modelId;

	while(tryCount != 0) {
		modelId += wantedChange;
		if (modelId < 0 || modelId >= MODELINFOSIZE) {
			tryCount--;
			wantedChange = -wantedChange;
		} else if (modelId != 5 && modelId != 6 && modelId != 405) {
			CBaseModelInfo *model = CModelInfo::GetModelInfo(modelId);
			if (model)
			{
				//int type = model->m_type;
				return modelId;
			}
		}
	}
	return ogModelId;
}

void
PlayAnimation(RpClump *clump, AssocGroupId animGroup, AnimationId anim)
{
	CAnimBlendAssociation *currentAssoc = RpAnimBlendClumpGetAssociation(clump, anim);

	if (currentAssoc && currentAssoc->IsPartial())
		delete currentAssoc;

	RpAnimBlendClumpSetBlendDeltas(clump, ASSOC_PARTIAL, -8.0f);

	CAnimBlendAssociation *animAssoc = CAnimManager::BlendAnimation(clump, animGroup, anim, 8.0f);
	animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	animAssoc->SetCurrentTime(0.0f);
	animAssoc->SetRun();
}

void
CAnimViewer::Update(void)
{
	static int modelId = 0;
	static int animId = 0;
	// Please don't make this bool, static bool's are problematic on my side.
	static int reloadIFP = 0;

	AssocGroupId animGroup = ASSOCGRP_STD;
	int nextModelId = modelId;
	CBaseModelInfo *modelInfo = CModelInfo::GetModelInfo(modelId);
	CEntity *newEntity = nil;

	if (modelInfo->m_type == MITYPE_PED) {
		int animGroup = ((CPedModelInfo*)modelInfo)->m_animGroup;

		if (animId > ANIM_IDLE_STANCE)
			animGroup = ASSOCGRP_STD;

		if (reloadIFP) {
			if (pTarget) {
				CWorld::Remove(pTarget);
				if (pTarget)
					delete pTarget;
			}
			pTarget = nil;
			
			// These calls were inside of LoadIFP function.
			CAnimManager::Shutdown();
			CAnimManager::Initialise();
			CAnimManager::LoadAnimFiles();

			reloadIFP = 0;
		}
	} else {
		animGroup = ASSOCGRP_STD;
	}
	CPad::UpdatePads();
	CPad* pad = CPad::GetPad(0);
	CStreaming::UpdateForAnimViewer();
	CStreaming::RequestModel(modelId, 0);
	if (CStreaming::HasModelLoaded(modelId)) {

		if (!pTarget) {

			if (modelInfo->m_type == MITYPE_VEHICLE) {

				CVehicleModelInfo* veh = (CVehicleModelInfo*)modelInfo;
				if (veh->m_vehicleType != VEHICLE_TYPE_AUTOMOBILE) {
					// Not ready yet
/*					if (veh->m_vehicleType == VEHICLE_TYPE_BOAT)
					{
						v33 = (CBoat*)CVehicle::operator new((CVehicle*)0x488, v6);
						CBoat::CBoat(v33, modelId, 1u);
						newEntity = (int)v33;
						pTarget = (int)v33;
					}
					else
					{
*/						newEntity = pTarget = new CObject(modelId, true);
						if (!modelInfo->GetColModel()) {
							modelInfo->SetColModel(&CTempColModels::ms_colModelWheel1);
						}
//					}
				} else {
					newEntity = pTarget = new CAutomobile(modelId, RANDOM_VEHICLE);
					newEntity->m_status = STATUS_ABANDONED;
				}
				newEntity->bIsStuck = true;
			} else if (modelInfo->m_type == MITYPE_PED) {
				pTarget = newEntity = new CPed(PEDTYPE_CIVMALE);
				newEntity->SetModelIndex(modelId);
			} else {
				newEntity = pTarget = new CObject(modelId, true);
				if (!modelInfo->GetColModel())
				{
					modelInfo->SetColModel(&CTempColModels::ms_colModelWheel1);
				}
				newEntity->bIsStuck = true;
			}
			newEntity->GetPosition() = CVector(0.0f, 0.0f, 0.0f);
			CWorld::Add(newEntity);
			TheCamera.TakeControl(pTarget, 9, 2, 1);
		}
		if (pTarget->m_type == ENTITY_TYPE_VEHICLE || pTarget->m_type == ENTITY_TYPE_PED || pTarget->m_type == ENTITY_TYPE_OBJECT) {
			((CPhysical*)pTarget)->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		}
		pTarget->GetPosition().z = 0.0f;

		if (modelInfo->m_type != MITYPE_PED) {

			if (modelInfo->m_type == MITYPE_VEHICLE) {

				if (pad->NewState.LeftShoulder1 && !pad->OldState.LeftShoulder1) {
					nextModelId = LastPedModelId(modelId);
					AsciiToUnicode("Switched to peds", gUString);
					CMessages::AddMessage(gUString, 1000, 0);
				} else {
					// Start in mobile
					if (pad->NewState.Square && !pad->OldState.Square) {
						CVehicleModelInfo::LoadVehicleColours();
						AsciiToUnicode("Carcols.dat reloaded", gUString);
						CMessages::AddMessage(gUString, 1000, 0);
					}
				}
			}
		} else {
			((CPed*)pTarget)->bKindaStayInSamePlace = true;

			// Triangle in mobile
			if (pad->NewState.Square && !pad->OldState.Square) {
				reloadIFP = 1;
				AsciiToUnicode("IFP reloaded", gUString);
				CMessages::AddMessage(gUString, 1000, 0);

			} else if (pad->NewState.Cross && !pad->OldState.Cross) {
				PlayAnimation(pTarget->GetClump(), animGroup, (AnimationId)animId);
				AsciiToUnicode("Animation restarted", gUString);
				CMessages::AddMessage(gUString, 1000, 0);

			} else if (pad->NewState.Circle && !pad->OldState.Circle) {
				PlayAnimation(pTarget->GetClump(), animGroup, ANIM_IDLE_STANCE);
				AsciiToUnicode("Idle animation playing", gUString);
				CMessages::AddMessage(gUString, 1000, 0);

			} else if (pad->NewState.DPadUp && pad->OldState.DPadUp == 0) {
				animId--;
				if (animId < 0) {
					animId = NUM_ANIMS - 1;
				}
				PlayAnimation(pTarget->GetClump(), animGroup, (AnimationId)animId);

				sprintf(gString, "Current anim: %d", animId);
				AsciiToUnicode(gString, gUString);
				CMessages::AddMessage(gUString, 1000, 0);

			} else if (pad->NewState.DPadDown && !pad->OldState.DPadDown) {
				animId = (animId == (NUM_ANIMS - 1) ? 0 : animId + 1);
				PlayAnimation(pTarget->GetClump(), animGroup, (AnimationId)animId);

				sprintf(gString, "Current anim: %d", animId);
				AsciiToUnicode(gString, gUString);
				CMessages::AddMessage(gUString, 1000, 0);

			} else {
				if (pad->NewState.Start && !pad->OldState.Start) {

				} else {
					if (pad->NewState.LeftShoulder1 && !pad->OldState.LeftShoulder1) {
						nextModelId = LastVehicleModelId(modelId);
						AsciiToUnicode("Switched to vehicles", gUString);
						CMessages::AddMessage(gUString, 1000, 0);
					} else {
						// Originally it was GetPad(1)->LeftShoulder2
						if (pad->NewState.Triangle) {
							CPedModelInfo::AnimatePedColModel(((CPedModelInfo*)CModelInfo::GetModelInfo(pTarget->m_modelIndex))->GetHitColModel(), RpClumpGetFrame(pTarget->GetClump()));
							AsciiToUnicode("Ped Col model will be animated as long as you hold the button", gUString);
							CMessages::AddMessage(gUString, 100, 0);
						}
					}
				}
			}
		}
	}

	if (pad->NewState.DPadLeft && pad->OldState.DPadLeft == 0) {
		nextModelId = FindMeAModelID(modelId, -1);

		sprintf(gString, "Current model ID: %d", nextModelId);
		AsciiToUnicode(gString, gUString);
		CMessages::AddMessage(gUString, 1000, 0);

	} else if (pad->NewState.DPadRight && pad->OldState.DPadRight == 0) {
		nextModelId = FindMeAModelID(modelId, 1);

		sprintf(gString, "Current model ID: %d", nextModelId);
		AsciiToUnicode(gString, gUString);
		CMessages::AddMessage(gUString, 1000, 0);
	}
	// There were extra codes here to let us change model id by 50, but xbox CPad struct is different, so I couldn't port.

	if (nextModelId != modelId) {
		modelId = nextModelId;
		if (pTarget) {
			CWorld::Remove(pTarget);
			if (pTarget)
				delete pTarget;
		}
		pTarget = nil;
		return;
	}

	CTimeCycle::Update();
	CWorld::Process();
	if (pTarget)
		TheCamera.Process();
}

void
CAnimViewer::Shutdown(void)
{
	if (CWorld::Players[0].m_pPed)
		delete CWorld::Players[0].m_pPed;

	CWorld::ShutDown();
	CModelInfo::ShutDown();
	CAnimManager::Shutdown();
	CTimer::Shutdown();
	CStreaming::Shutdown();
	CTxdStore::RemoveTxdSlot(animTxdSlot);
}