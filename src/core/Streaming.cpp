#include "common.h"

#include "General.h"
#include "Pad.h"
#include "Hud.h"
#include "Text.h"
#include "Clock.h"
#include "Renderer.h"
#include "ModelInfo.h"
#include "TxdStore.h"
#include "ModelIndices.h"
#include "Pools.h"
#include "Wanted.h"
#include "Directory.h"
#include "RwHelper.h"
#include "World.h"
#include "Entity.h"
#include "FileMgr.h"
#include "FileLoader.h"
#include "Zones.h"
#include "ZoneCull.h"
#include "Radar.h"
#include "Camera.h"
#include "Record.h"
#include "CarCtrl.h"
#include "Population.h"
#include "Gangs.h"
#include "CutsceneMgr.h"
#include "CdStream.h"
#include "Streaming.h"
#ifdef FIX_BUGS
#include "Replay.h"
#endif
#include "main.h"
#include "Frontend.h"
#include "Font.h"
#include "MemoryMgr.h"
#include "MemoryHeap.h"

bool CStreaming::ms_disableStreaming;
bool CStreaming::ms_bLoadingBigModel;
int32 CStreaming::ms_numModelsRequested;
CStreamingInfo CStreaming::ms_aInfoForModel[NUMSTREAMINFO];
CStreamingInfo CStreaming::ms_startLoadedList;
CStreamingInfo CStreaming::ms_endLoadedList;
CStreamingInfo CStreaming::ms_startRequestedList;
CStreamingInfo CStreaming::ms_endRequestedList;
int32 CStreaming::ms_oldSectorX;
int32 CStreaming::ms_oldSectorY;
int32 CStreaming::ms_streamingBufferSize;
#ifndef ONE_THREAD_PER_CHANNEL
int8 *CStreaming::ms_pStreamingBuffer[2];
#else
int8 *CStreaming::ms_pStreamingBuffer[4];
#endif
size_t CStreaming::ms_memoryUsed;
CStreamingChannel CStreaming::ms_channel[2];
int32 CStreaming::ms_channelError;
int32 CStreaming::ms_numVehiclesLoaded;
int32 CStreaming::ms_vehiclesLoaded[MAXVEHICLESLOADED];
int32 CStreaming::ms_lastVehicleDeleted;
CDirectory *CStreaming::ms_pExtraObjectsDir;
int32 CStreaming::ms_numPriorityRequests;
bool CStreaming::ms_hasLoadedLODs;
int32 CStreaming::ms_currentPedGrp;
int32 CStreaming::ms_currentPedLoading;
int32 CStreaming::ms_lastCullZone;
uint16 CStreaming::ms_loadedGangs;
uint16 CStreaming::ms_loadedGangCars;
int32 CStreaming::ms_imageOffsets[NUMCDIMAGES];
int32 CStreaming::ms_lastImageRead;
int32 CStreaming::ms_imageSize;
size_t CStreaming::ms_memoryAvailable;

int32 desiredNumVehiclesLoaded = 12;

CEntity *pIslandLODindustEntity;
CEntity *pIslandLODcomIndEntity;
CEntity *pIslandLODcomSubEntity;
CEntity *pIslandLODsubIndEntity;
CEntity *pIslandLODsubComEntity;
int32 islandLODindust;
int32 islandLODcomInd;
int32 islandLODcomSub;
int32 islandLODsubInd;
int32 islandLODsubCom;

bool
CStreamingInfo::GetCdPosnAndSize(uint32 &posn, uint32 &size)
{
	if(m_size == 0)
		return false;
	posn = m_position;
	size = m_size;
	return true;
}

void
CStreamingInfo::SetCdPosnAndSize(uint32 posn, uint32 size)
{
	m_position = posn;
	m_size = size;
}

void
CStreamingInfo::AddToList(CStreamingInfo *link)
{
	// Insert this after link
	m_next = link->m_next;
	m_prev = link;
	link->m_next = this;
	m_next->m_prev = this;
}

void
CStreamingInfo::RemoveFromList(void)
{
	m_next->m_prev = m_prev;
	m_prev->m_next = m_next;
	m_next = nil;
	m_prev = nil;
}

void
CStreaming::Init2(void)
{
	int i;

	for(i = 0; i < NUMSTREAMINFO; i++){
		ms_aInfoForModel[i].m_loadState = STREAMSTATE_NOTLOADED;
		ms_aInfoForModel[i].m_next = nil;
		ms_aInfoForModel[i].m_prev = nil;
		ms_aInfoForModel[i].m_nextID = -1;
		ms_aInfoForModel[i].m_size = 0;
		ms_aInfoForModel[i].m_position = 0;
	}

	ms_channelError = -1;

	// init lists

	ms_startLoadedList.m_next = &ms_endLoadedList;
	ms_startLoadedList.m_prev = nil;
	ms_endLoadedList.m_prev = &ms_startLoadedList;
	ms_endLoadedList.m_next = nil;

	ms_startRequestedList.m_next = &ms_endRequestedList;
	ms_startRequestedList.m_prev = nil;
	ms_endRequestedList.m_prev = &ms_startRequestedList;
	ms_endRequestedList.m_next = nil;

	// init misc

	ms_oldSectorX = 0;
	ms_oldSectorY = 0;
	ms_streamingBufferSize = 0;
	ms_disableStreaming = false;
	ms_memoryUsed = 0;
	ms_bLoadingBigModel = false;

	// init channels

	ms_channel[0].state = CHANNELSTATE_IDLE;
	ms_channel[1].state = CHANNELSTATE_IDLE;
	for(i = 0; i < 4; i++){
		ms_channel[0].streamIds[i] = -1;
		ms_channel[0].offsets[i] = -1;
		ms_channel[1].streamIds[i] = -1;
		ms_channel[1].offsets[i] = -1;
	}

	// init stream info, mark things that are already loaded

	for(i = 0; i < MODELINFOSIZE; i++){
		CBaseModelInfo *mi = CModelInfo::GetModelInfo(i);
		if(mi && mi->GetRwObject()){
			ms_aInfoForModel[i].m_loadState = STREAMSTATE_LOADED;
			ms_aInfoForModel[i].m_flags = STREAMFLAGS_DONT_REMOVE;
			if(mi->IsSimple())
				((CSimpleModelInfo*)mi)->m_alpha = 255;
		}
	}

	for(i = 0; i < TXDSTORESIZE; i++)
		if(CTxdStore::GetSlot(i) && CTxdStore::GetSlot(i)->texDict)
			ms_aInfoForModel[i + STREAM_OFFSET_TXD].m_loadState = STREAMSTATE_LOADED;


	for(i = 0; i < MAXVEHICLESLOADED; i++)
		ms_vehiclesLoaded[i] = -1;
	ms_numVehiclesLoaded = 0;

	ms_pExtraObjectsDir = new CDirectory(EXTRADIRSIZE);
	ms_numPriorityRequests = 0;
	ms_hasLoadedLODs = true;
	ms_currentPedGrp = -1;
	ms_lastCullZone = -1;		// unused because RemoveModelsNotVisibleFromCullzone is gone
	ms_loadedGangs = 0;
	ms_currentPedLoading = 8;	// unused, whatever it is

	LoadCdDirectory();

	// allocate streaming buffers
	if(ms_streamingBufferSize & 1) ms_streamingBufferSize++;
#ifndef ONE_THREAD_PER_CHANNEL
	ms_pStreamingBuffer[0] = (int8*)RwMallocAlign(ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE, CDSTREAM_SECTOR_SIZE);
	ms_streamingBufferSize /= 2;
	ms_pStreamingBuffer[1] = ms_pStreamingBuffer[0] + ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
#else
	ms_pStreamingBuffer[0] = (int8*)RwMallocAlign(ms_streamingBufferSize*2*CDSTREAM_SECTOR_SIZE, CDSTREAM_SECTOR_SIZE);
	ms_streamingBufferSize /= 2;
	ms_pStreamingBuffer[1] = ms_pStreamingBuffer[0] + ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
	ms_pStreamingBuffer[2] = ms_pStreamingBuffer[1] + ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
	ms_pStreamingBuffer[3] = ms_pStreamingBuffer[2] + ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
#endif
	debug("Streaming buffer size is %d sectors", ms_streamingBufferSize);

	// PC only, figure out how much memory we got
#ifdef GTA_PC
#define MB (1024*1024)

	extern size_t _dwMemAvailPhys;
	ms_memoryAvailable = (_dwMemAvailPhys - 10*MB)/2;
	if(ms_memoryAvailable < 50*MB)
		ms_memoryAvailable = 50*MB;
	desiredNumVehiclesLoaded = (int32)((ms_memoryAvailable / MB - 50) / 3 + 12);
	if(desiredNumVehiclesLoaded > MAXVEHICLESLOADED)
		desiredNumVehiclesLoaded = MAXVEHICLESLOADED;
	debug("Memory allocated to Streaming is %zuMB", ms_memoryAvailable/MB); // original modifier was %d
#undef MB
#endif

	// find island LODs

	pIslandLODindustEntity = nil;
	pIslandLODcomIndEntity = nil;
	pIslandLODcomSubEntity = nil;
	pIslandLODsubIndEntity = nil;
	pIslandLODsubComEntity = nil;
	islandLODindust = -1;
	islandLODcomInd = -1;
	islandLODcomSub = -1;
	islandLODsubInd = -1;
	islandLODsubCom = -1;
        CModelInfo::GetModelInfo("IslandLODInd", &islandLODindust);
        CModelInfo::GetModelInfo("IslandLODcomIND", &islandLODcomInd);
        CModelInfo::GetModelInfo("IslandLODcomSUB", &islandLODcomSub);
        CModelInfo::GetModelInfo("IslandLODsubIND", &islandLODsubInd);
        CModelInfo::GetModelInfo("IslandLODsubCOM", &islandLODsubCom);

	for(i = CPools::GetBuildingPool()->GetSize()-1; i >= 0; i--){
		CBuilding *building = CPools::GetBuildingPool()->GetSlot(i);
		if(building == nil)
			continue;
		if(building->GetModelIndex() == islandLODindust) pIslandLODindustEntity = building;
		if(building->GetModelIndex() == islandLODcomInd) pIslandLODcomIndEntity = building;
		if(building->GetModelIndex() == islandLODcomSub) pIslandLODcomSubEntity = building;
		if(building->GetModelIndex() == islandLODsubInd) pIslandLODsubIndEntity = building;
		if(building->GetModelIndex() == islandLODsubCom) pIslandLODsubComEntity = building;
	}
}

void
CStreaming::Init(void)
{
#ifdef USE_TXD_CDIMAGE
	int txdHandle = CFileMgr::OpenFile("MODELS\\TXD.IMG", "r");
	if (txdHandle)
		CFileMgr::CloseFile(txdHandle);
	if (!CheckVideoCardCaps() && txdHandle) {
		CdStreamAddImage("MODELS\\TXD.IMG");
		CStreaming::Init2();
	} else {
		CStreaming::Init2();
		if (CreateTxdImageForVideoCard()) {
			CStreaming::Shutdown();
			CdStreamAddImage("MODELS\\TXD.IMG");
			CStreaming::Init2();
		}
	}
#else
	CStreaming::Init2();
#endif
}

void
CStreaming::Shutdown(void)
{
	RwFreeAlign(ms_pStreamingBuffer[0]);
	ms_streamingBufferSize = 0;
	if(ms_pExtraObjectsDir){
		delete ms_pExtraObjectsDir;
#ifdef FIX_BUGS
		ms_pExtraObjectsDir = nil;
#endif
	}
}

#ifndef MASTER
uint64 timeProcessingTXD;
uint64 timeProcessingDFF;
#endif

void
CStreaming::Update(void)
{
	CEntity *train;
	CStreamingInfo *si, *prev;
	bool requestedSubway = false;

#ifndef MASTER
	timeProcessingTXD = 0;
	timeProcessingDFF = 0;
#endif

	UpdateMemoryUsed();

	if(ms_channelError != -1){
		RetryLoadFile(ms_channelError);
		return;
	}

	if(CTimer::GetIsPaused())
		return;

	train = FindPlayerTrain();
	if(train && train->GetPosition().z < 0.0f){
		RequestSubway();
		requestedSubway = true;
	}else if(!ms_disableStreaming)
		AddModelsToRequestList(TheCamera.GetPosition());

	DeleteFarAwayRwObjects(TheCamera.GetPosition());

	if(!ms_disableStreaming &&
	   !CCutsceneMgr::IsRunning() &&
	   !requestedSubway &&
	   !CGame::playingIntro &&
	   ms_numModelsRequested < 5 &&
	   !CRenderer::m_loadingPriority
#ifdef FIX_BUGS
		&& !CReplay::IsPlayingBack()
#endif
		){
		StreamVehiclesAndPeds();
		StreamZoneModels(FindPlayerCoors());
	}

	LoadRequestedModels();

#ifndef MASTER
	if (CPad::GetPad(1)->GetLeftShoulder1JustDown() && CPad::GetPad(1)->GetRightShoulder1() && CPad::GetPad(1)->GetRightShoulder2())
		PrintStreamingBufferState();

	// TODO: PrintRequestList
	//if (CPad::GetPad(1)->GetLeftShoulder2JustDown() && CPad::GetPad(1)->GetRightShoulder1() && CPad::GetPad(1)->GetRightShoulder2())
	//	PrintRequestList();
#endif

	for(si = ms_endRequestedList.m_prev; si != &ms_startRequestedList; si = prev){
		prev = si->m_prev;
		if((si->m_flags & (STREAMFLAGS_KEEP_IN_MEMORY|STREAMFLAGS_PRIORITY)) == 0)
			RemoveModel(si - ms_aInfoForModel);
	}
}

void
CStreaming::LoadCdDirectory(void)
{
	char dirname[132];
	int i;

#ifdef GTA_PC
	ms_imageOffsets[0] = 0;
	ms_imageOffsets[1] = -1;
	ms_imageOffsets[2] = -1;
	ms_imageOffsets[3] = -1;
	ms_imageOffsets[4] = -1;
	ms_imageOffsets[5] = -1;
	ms_imageOffsets[6] = -1;
	ms_imageOffsets[7] = -1;
	ms_imageOffsets[8] = -1;
	ms_imageOffsets[9] = -1;
	ms_imageOffsets[10] = -1;
	ms_imageOffsets[11] = -1;
	ms_imageSize = GetGTA3ImgSize();
	// PS2 uses CFileMgr::GetCdFile on all IMG files to fill the array
#endif

	i = CdStreamGetNumImages();
	while(i-- >= 1){
		strcpy(dirname, CdStreamGetImageName(i));
		strncpy(strrchr(dirname, '.') + 1, "DIR", 3);
		LoadCdDirectory(dirname, i);
	}

	ms_lastImageRead = 0;
	ms_imageSize /= CDSTREAM_SECTOR_SIZE;
}

void
CStreaming::LoadCdDirectory(const char *dirname, int n)
{
	int fd, lastID, imgSelector;
	int modelId, txdId;
	uint32 posn, size;
	CDirectory::DirectoryInfo direntry;
	char *dot;

	lastID = -1;
	fd = CFileMgr::OpenFile(dirname, "rb");
	assert(fd > 0);

	imgSelector = n<<24;
	assert(sizeof(direntry) == 32);
	while(CFileMgr::Read(fd, (char*)&direntry, sizeof(direntry))){
		dot = strchr(direntry.name, '.');
		assert(dot);
		if(dot) *dot = '\0';
		if(direntry.size > (uint32)ms_streamingBufferSize)
			ms_streamingBufferSize = direntry.size;

		if(!CGeneral::faststrcmp(dot+1, "DFF") || !CGeneral::faststrcmp(dot+1, "dff")){
			if(CModelInfo::GetModelInfo(direntry.name, &modelId)){
				if(ms_aInfoForModel[modelId].GetCdPosnAndSize(posn, size)){
					debug("%s appears more than once in %s\n", direntry.name, dirname);
					lastID = -1;
				}else{
					direntry.offset |= imgSelector;
					ms_aInfoForModel[modelId].SetCdPosnAndSize(direntry.offset, direntry.size);
					if(lastID != -1)
						ms_aInfoForModel[lastID].m_nextID = modelId;
					lastID = modelId;
				}
			}else{
#ifdef FIX_BUGS
				// remember which cdimage this came from
				ms_pExtraObjectsDir->AddItem(direntry, n);
#else
				ms_pExtraObjectsDir->AddItem(direntry);
#endif
				lastID = -1;
			}
		}else if(!CGeneral::faststrcmp(dot+1, "TXD") || !CGeneral::faststrcmp(dot+1, "txd")){
			txdId = CTxdStore::FindTxdSlot(direntry.name);
			if(txdId == -1)
				txdId = CTxdStore::AddTxdSlot(direntry.name);
			if(ms_aInfoForModel[txdId + STREAM_OFFSET_TXD].GetCdPosnAndSize(posn, size)){
				debug("%s appears more than once in %s\n", direntry.name, dirname);
				lastID = -1;
			}else{
				direntry.offset |= imgSelector;
				ms_aInfoForModel[txdId + STREAM_OFFSET_TXD].SetCdPosnAndSize(direntry.offset, direntry.size);
				if(lastID != -1)
					ms_aInfoForModel[lastID].m_nextID = txdId + STREAM_OFFSET_TXD;
				lastID = txdId + STREAM_OFFSET_TXD;
			}
		}else
			lastID = -1;
	}

	CFileMgr::CloseFile(fd);
}

#ifdef USE_CUSTOM_ALLOCATOR
RpAtomic*
RegisterAtomicMemPtrsCB(RpAtomic *atomic, void *data)
{
#if THIS_IS_COMPATIBLE_WITH_GTA3_RW31
	// not quite sure what's going on here:
	// gta3's RW 3.1 allocates separate memory for geometry data of RpGeometry.
	// Is that a R* change? rpDefaultGeometryInstance also depends on it
	RpGeometry *geo = RpAtomicGetGeometry(atomic);
	if(geo->triangles)
		REGISTER_MEMPTR(&geo->triangles);
	if(geo->matList.materials)
		REGISTER_MEMPTR(&geo->matList.materials);
	if(geo->preLitLum)
		REGISTER_MEMPTR(&geo->preLitLum);
	if(geo->texCoords[0])
		REGISTER_MEMPTR(&geo->texCoords[0]);
	if(geo->texCoords[1])
		REGISTER_MEMPTR(&geo->texCoords[1]);
#else
	// normally RpGeometry is allocated in one block (excluding morph targets)
	// so we don't really have allocated pointers in the struct.
	// NB: in librw we actually do it in two allocations (geometry itself and data)
	// so we could conceivably come up with something here
#endif
	return atomic;
}
#endif

bool
CStreaming::ConvertBufferToObject(int8 *buf, int32 streamId)
{
	RwMemory mem;
	RwStream *stream;
	int cdsize;
	uint32 startTime, endTime, timeDiff;
	CBaseModelInfo *mi;
	bool success;

	startTime = CTimer::GetCurrentTimeInCycles() / CTimer::GetCyclesPerMillisecond();

	cdsize = ms_aInfoForModel[streamId].GetCdSize();
	mem.start = (uint8*)buf;
	mem.length = cdsize * CDSTREAM_SECTOR_SIZE;
	stream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &mem);

	if(streamId < STREAM_OFFSET_TXD){
		// Model
		mi = CModelInfo::GetModelInfo(streamId);

		// Txd has to be loaded
#ifdef FIX_BUGS
		if(!HasTxdLoaded(mi->GetTxdSlot())){
#else
		// texDict will exist even if only first part has loaded
		if(CTxdStore::GetSlot(mi->GetTxdSlot())->texDict == nil){
#endif
			debug("failed to load %s because TXD %s is not in memory\n", mi->GetModelName(), CTxdStore::GetTxdName(mi->GetTxdSlot()));
			RemoveModel(streamId);
#ifndef FIX_BUGS
			// if we're just waiting for it to load, don't remove this
			RemoveTxd(mi->GetTxdSlot());
#endif
			ReRequestModel(streamId);
			RwStreamClose(stream, &mem);
			return false;
		}

		// Set Txd to use
		CTxdStore::AddRef(mi->GetTxdSlot());

		PUSH_MEMID(MEMID_STREAM_MODELS);
		CTxdStore::SetCurrentTxd(mi->GetTxdSlot());
		if(mi->IsSimple()){
			success = CFileLoader::LoadAtomicFile(stream, streamId);
#ifdef USE_CUSTOM_ALLOCATOR
			RegisterAtomicMemPtrsCB(((CSimpleModelInfo*)mi)->m_atomics[0], nil);
#endif
		} else if (mi->GetModelType() == MITYPE_VEHICLE) {
			// load vehicles in two parts
			CModelInfo::GetModelInfo(streamId)->AddRef();
			success = CFileLoader::StartLoadClumpFile(stream, streamId);
			if(success)
				ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_STARTED;
		}else{
			success = CFileLoader::LoadClumpFile(stream, streamId);
#ifdef USE_CUSTOM_ALLOCATOR
			if(success)
				RpClumpForAllAtomics((RpClump*)mi->GetRwObject(), RegisterAtomicMemPtrsCB, nil);
#endif
		}
		POP_MEMID();
		UpdateMemoryUsed();

		// Txd no longer needed unless we only read part of the file
		if(ms_aInfoForModel[streamId].m_loadState != STREAMSTATE_STARTED)
			CTxdStore::RemoveRefWithoutDelete(mi->GetTxdSlot());

		if(!success){
			debug("Failed to load %s\n", CModelInfo::GetModelInfo(streamId)->GetModelName());
			RemoveModel(streamId);
			ReRequestModel(streamId);
			RwStreamClose(stream, &mem);
			return false;
		}
	}else{
		// Txd
		assert(streamId < NUMSTREAMINFO);
		if((ms_aInfoForModel[streamId].m_flags & STREAMFLAGS_KEEP_IN_MEMORY) == 0 &&
		   !IsTxdUsedByRequestedModels(streamId - STREAM_OFFSET_TXD)){
			RemoveModel(streamId);
			RwStreamClose(stream, &mem);
			return false;
		}

		PUSH_MEMID(MEMID_STREAM_TEXUTRES);
		if(ms_bLoadingBigModel || cdsize > 200){
			success = CTxdStore::StartLoadTxd(streamId - STREAM_OFFSET_TXD, stream);
			if(success)
				ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_STARTED;
		}else
		        success = CTxdStore::LoadTxd(streamId - STREAM_OFFSET_TXD, stream);
		POP_MEMID();
		UpdateMemoryUsed();

		if(!success){
			debug("Failed to load %s.txd\n", CTxdStore::GetTxdName(streamId - STREAM_OFFSET_TXD));
			RemoveModel(streamId);
			ReRequestModel(streamId);
			RwStreamClose(stream, &mem);
			return false;
		}
	}

	RwStreamClose(stream, &mem);

	// We shouldn't even end up here unless load was successful
	if(!success){
		ReRequestModel(streamId);
		if(streamId < STREAM_OFFSET_TXD)
			debug("Failed to load %s.dff\n", mi->GetModelName());
		else
			debug("Failed to load %s.txd\n", CTxdStore::GetTxdName(streamId - STREAM_OFFSET_TXD));
		return false;
	}

	if(streamId < STREAM_OFFSET_TXD){
		// Model
		// Vehicles and Peds not in loaded list
		if (mi->GetModelType() != MITYPE_VEHICLE && mi->GetModelType() != MITYPE_PED) {
			CSimpleModelInfo *smi = (CSimpleModelInfo*)mi;

			// Set fading for some objects
			if(mi->IsSimple() && !smi->m_isBigBuilding){
				if(ms_aInfoForModel[streamId].m_flags & STREAMFLAGS_NOFADE)
					smi->m_alpha = 255;
				else
					smi->m_alpha = 0;
			}

			if((ms_aInfoForModel[streamId].m_flags & STREAMFLAGS_CANT_REMOVE) == 0)
				ms_aInfoForModel[streamId].AddToList(&ms_startLoadedList);
		}
	}else{
		// Txd
		if((ms_aInfoForModel[streamId].m_flags & STREAMFLAGS_CANT_REMOVE) == 0)
			ms_aInfoForModel[streamId].AddToList(&ms_startLoadedList);
	}

	// Mark objects as loaded
	if(ms_aInfoForModel[streamId].m_loadState != STREAMSTATE_STARTED){
		ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_LOADED;
#ifndef USE_CUSTOM_ALLOCATOR
		ms_memoryUsed += ms_aInfoForModel[streamId].GetCdSize() * CDSTREAM_SECTOR_SIZE;
#endif
	}

	endTime = CTimer::GetCurrentTimeInCycles() / CTimer::GetCyclesPerMillisecond();
	timeDiff = endTime - startTime;
	if(timeDiff > 5){
		if(streamId < STREAM_OFFSET_TXD)
			debug("model %s took %d ms\n", CModelInfo::GetModelInfo(streamId)->GetModelName(), timeDiff);
		else
			debug("txd %s took %d ms\n", CTxdStore::GetTxdName(streamId - STREAM_OFFSET_TXD), timeDiff);
	}

	return true;
}


bool
CStreaming::FinishLoadingLargeFile(int8 *buf, int32 streamId)
{
	RwMemory mem;
	RwStream *stream;
	uint32 startTime, endTime, timeDiff;
	CBaseModelInfo *mi;
	bool success;

	startTime = CTimer::GetCurrentTimeInCycles() / CTimer::GetCyclesPerMillisecond();

	if(ms_aInfoForModel[streamId].m_loadState != STREAMSTATE_STARTED){
		if(streamId < STREAM_OFFSET_TXD)
			CModelInfo::GetModelInfo(streamId)->RemoveRef();
		return false;
	}

	mem.start = (uint8*)buf;
	mem.length = ms_aInfoForModel[streamId].GetCdSize() * CDSTREAM_SECTOR_SIZE;
	stream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &mem);

	if(streamId < STREAM_OFFSET_TXD){
		// Model
		mi = CModelInfo::GetModelInfo(streamId);
		PUSH_MEMID(MEMID_STREAM_MODELS);
		CTxdStore::SetCurrentTxd(mi->GetTxdSlot());
		success = CFileLoader::FinishLoadClumpFile(stream, streamId);
		if(success){
#ifdef USE_CUSTOM_ALLOCATOR
			RpClumpForAllAtomics((RpClump*)mi->GetRwObject(), RegisterAtomicMemPtrsCB, nil);
#endif
			success = AddToLoadedVehiclesList(streamId);
		}
		POP_MEMID();
		mi->RemoveRef();
		CTxdStore::RemoveRefWithoutDelete(mi->GetTxdSlot());
	}else{
		// Txd
		CTxdStore::AddRef(streamId - STREAM_OFFSET_TXD);
		PUSH_MEMID(MEMID_STREAM_TEXUTRES);
		success = CTxdStore::FinishLoadTxd(streamId - STREAM_OFFSET_TXD, stream);
		POP_MEMID();
		CTxdStore::RemoveRefWithoutDelete(streamId - STREAM_OFFSET_TXD);
	}

	RwStreamClose(stream, &mem);

	ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_LOADED;	// only done if success on PS2
#ifndef USE_CUSTOM_ALLOCATOR
	ms_memoryUsed += ms_aInfoForModel[streamId].GetCdSize() * CDSTREAM_SECTOR_SIZE;
#endif

	if(!success){
		RemoveModel(streamId);
		ReRequestModel(streamId);
		UpdateMemoryUsed();	// directly after pop on PS2
		return false;
	}

	UpdateMemoryUsed();	// directly after pop on PS2

	endTime = CTimer::GetCurrentTimeInCycles() / CTimer::GetCyclesPerMillisecond();
	timeDiff = endTime - startTime;
	if(timeDiff > 5){
		if(streamId < STREAM_OFFSET_TXD)
			debug("finish model %s took %d ms\n", CModelInfo::GetModelInfo(streamId)->GetModelName(), timeDiff);
		else
			debug("finish txd %s took %d ms\n", CTxdStore::GetTxdName(streamId - STREAM_OFFSET_TXD), timeDiff);
	}

	return true;
}

void
CStreaming::RequestModel(int32 id, int32 flags)
{
	CSimpleModelInfo *mi;

	if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_INQUEUE){
		// updgrade to priority
		if(flags & STREAMFLAGS_PRIORITY && !ms_aInfoForModel[id].IsPriority()){
			ms_numPriorityRequests++;
			ms_aInfoForModel[id].m_flags |= STREAMFLAGS_PRIORITY;
		}
	}else if(ms_aInfoForModel[id].m_loadState != STREAMSTATE_NOTLOADED){
		flags &= ~STREAMFLAGS_PRIORITY;
	}
	ms_aInfoForModel[id].m_flags |= flags;

	if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_LOADED){
		// Already loaded, only check changed flags

		if(ms_aInfoForModel[id].m_flags & STREAMFLAGS_NOFADE && id < STREAM_OFFSET_TXD){
			mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
			if(mi->IsSimple())
				mi->m_alpha = 255;
		}

		// reinsert into list
		if(ms_aInfoForModel[id].m_next){
			ms_aInfoForModel[id].RemoveFromList();
			if((ms_aInfoForModel[id].m_flags & STREAMFLAGS_CANT_REMOVE) == 0)
				ms_aInfoForModel[id].AddToList(&ms_startLoadedList);
		}
	}else if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_NOTLOADED ||
	         ms_aInfoForModel[id].m_loadState == STREAMSTATE_LOADED){	// how can this be true again?

		if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_NOTLOADED){
			if(id < STREAM_OFFSET_TXD)
				RequestTxd(CModelInfo::GetModelInfo(id)->GetTxdSlot(), flags);
			ms_aInfoForModel[id].AddToList(&ms_startRequestedList);
			ms_numModelsRequested++;
			if(flags & STREAMFLAGS_PRIORITY)
				ms_numPriorityRequests++;
		}

		ms_aInfoForModel[id].m_loadState = STREAMSTATE_INQUEUE;
		ms_aInfoForModel[id].m_flags = flags;
	}
}

void
CStreaming::RequestSubway(void)
{
	RequestModel(MI_SUBWAY1, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY2, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY3, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY4, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY5, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY6, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY7, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY8, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY9, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY10, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY11, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY12, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY13, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY14, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY15, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY16, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY17, STREAMFLAGS_NOFADE);
	RequestModel(MI_SUBWAY18, STREAMFLAGS_NOFADE);

	switch(CGame::currLevel){
	case LEVEL_INDUSTRIAL:
		RequestModel(MI_SUBPLATFORM_IND, STREAMFLAGS_NOFADE);
		break;
	case LEVEL_COMMERCIAL:
		if(FindPlayerTrain()->GetPosition().y < -700.0f){
			RequestModel(MI_SUBPLATFORM_COMS, STREAMFLAGS_NOFADE);
			RequestModel(MI_SUBPLATFORM_COMS2, STREAMFLAGS_NOFADE);
		}else{
			RequestModel(MI_SUBPLATFORM_COMN, STREAMFLAGS_NOFADE);
		}
		break;
	case LEVEL_SUBURBAN:
		RequestModel(MI_SUBPLATFORM_SUB, STREAMFLAGS_NOFADE);
		RequestModel(MI_SUBPLATFORM_SUB2, STREAMFLAGS_NOFADE);
		break;
	default: break;
	}
}

#define BIGBUILDINGFLAGS STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_PRIORITY

void
CStreaming::RequestBigBuildings(eLevelName level)
{
	int i, n;
	CBuilding *b;

	n = CPools::GetBuildingPool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		b = CPools::GetBuildingPool()->GetSlot(i);
		if(b && b->bIsBIGBuilding
#ifdef NO_ISLAND_LOADING
		    && (((CMenuManager::m_PrefsIslandLoading != CMenuManager::ISLAND_LOADING_LOW) && (b != pIslandLODindustEntity) && (b != pIslandLODcomIndEntity) &&
		         (b != pIslandLODcomSubEntity) && (b != pIslandLODsubIndEntity) && (b != pIslandLODsubComEntity)
				) || (b->m_level == level))
#else
		    && b->m_level == level
#endif
		)
			RequestModel(b->GetModelIndex(), BIGBUILDINGFLAGS);
	}
	RequestIslands(level);
	ms_hasLoadedLODs = false;
}

void
CStreaming::RequestIslands(eLevelName level)
{
	ISLAND_LOADING_ISNT(HIGH)
	switch(level){
	case LEVEL_INDUSTRIAL:
		RequestModel(islandLODcomInd, BIGBUILDINGFLAGS);
		RequestModel(islandLODsubInd, BIGBUILDINGFLAGS);
		break;
	case LEVEL_COMMERCIAL:
		RequestModel(islandLODindust, BIGBUILDINGFLAGS);
		RequestModel(islandLODsubCom, BIGBUILDINGFLAGS);
		break;
	case LEVEL_SUBURBAN:
		RequestModel(islandLODindust, BIGBUILDINGFLAGS);
		RequestModel(islandLODcomSub, BIGBUILDINGFLAGS);
		break;
	default: break;
	}
}

void
CStreaming::RequestSpecialModel(int32 modelId, const char *modelName, int32 flags)
{
	CBaseModelInfo *mi;
	int txdId;
	char oldName[48];
	uint32 pos, size;

	mi = CModelInfo::GetModelInfo(modelId);
	if(!CGeneral::faststrcmp(mi->GetModelName(), modelName)){
		// Already have the correct name, just request it
		RequestModel(modelId, flags);
		return;
	}

	strcpy(oldName, mi->GetModelName());
	mi->SetModelName(modelName);

	// What exactly is going on here?
	if(CModelInfo::GetModelInfo(oldName, nil)){
		txdId = CTxdStore::FindTxdSlot(oldName);
		if(txdId != -1 && CTxdStore::GetSlot(txdId)->texDict){
			CTxdStore::AddRef(txdId);
			RemoveModel(modelId);
			CTxdStore::RemoveRefWithoutDelete(txdId);
		}else
			RemoveModel(modelId);
	}else
		RemoveModel(modelId);

	bool found = ms_pExtraObjectsDir->FindItem(modelName, pos, size);
	assert(found);
	mi->ClearTexDictionary();
	if(CTxdStore::FindTxdSlot(modelName) == -1)
		mi->SetTexDictionary("generic");
	else
		mi->SetTexDictionary(modelName);
	ms_aInfoForModel[modelId].SetCdPosnAndSize(pos, size);
	RequestModel(modelId, flags);
}

void
CStreaming::RequestSpecialChar(int32 charId, const char *modelName, int32 flags)
{
	RequestSpecialModel(charId + MI_SPECIAL01, modelName, flags);
}

bool
CStreaming::HasSpecialCharLoaded(int32 id)
{
	return HasModelLoaded(id + MI_SPECIAL01);
}

void
CStreaming::SetMissionDoesntRequireSpecialChar(int32 id)
{
	return SetMissionDoesntRequireModel(id + MI_SPECIAL01);
}

void
CStreaming::DecrementRef(int32 id)
{
	ms_numModelsRequested--;
	if(ms_aInfoForModel[id].IsPriority()){
		ms_aInfoForModel[id].m_flags &= ~STREAMFLAGS_PRIORITY;
		ms_numPriorityRequests--;
	}
}

void
CStreaming::RemoveModel(int32 id)
{
	int i;

	if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_NOTLOADED)
		return;

	if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_LOADED){
		if(id < STREAM_OFFSET_TXD)
			CModelInfo::GetModelInfo(id)->DeleteRwObject();
		else
			CTxdStore::RemoveTxd(id - STREAM_OFFSET_TXD);
#ifdef USE_CUSTOM_ALLOCATOR
		UpdateMemoryUsed();
#else
		ms_memoryUsed -= ms_aInfoForModel[id].GetCdSize()*CDSTREAM_SECTOR_SIZE;
#endif
	}

	if(ms_aInfoForModel[id].m_next){
		// Remove from list, model is neither loaded nor requested
		if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_INQUEUE)
			DecrementRef(id);
		ms_aInfoForModel[id].RemoveFromList();
	}else if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_READING){
		for(i = 0; i < 4; i++){
			if(ms_channel[0].streamIds[i] == id)
				ms_channel[0].streamIds[i] = -1;
			if(ms_channel[1].streamIds[i] == id)
				ms_channel[1].streamIds[i] = -1;
		}
	}

	if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_STARTED){
		if(id < STREAM_OFFSET_TXD)
			RpClumpGtaCancelStream();
		else
			CTxdStore::RemoveTxd(id - STREAM_OFFSET_TXD);
#ifdef USE_CUSTOM_ALLOCATOR
		UpdateMemoryUsed();
#endif
	}

	ms_aInfoForModel[id].m_loadState = STREAMSTATE_NOTLOADED;
}

void
CStreaming::RemoveUnusedBuildings(eLevelName level)
{
	if(level != LEVEL_INDUSTRIAL)
		RemoveBuildings(LEVEL_INDUSTRIAL);
	if(level != LEVEL_COMMERCIAL)
		RemoveBuildings(LEVEL_COMMERCIAL);
	if(level != LEVEL_SUBURBAN)
		RemoveBuildings(LEVEL_SUBURBAN);
}

void
CStreaming::RemoveBuildings(eLevelName level)
{
	int i, n;
	CEntity *e;
	CBaseModelInfo *mi;

	n = CPools::GetBuildingPool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		e = CPools::GetBuildingPool()->GetSlot(i);
		if(e && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}

	n = CPools::GetTreadablePool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		e = CPools::GetTreadablePool()->GetSlot(i);
		if(e && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}

	n = CPools::GetObjectPool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		e = CPools::GetObjectPool()->GetSlot(i);
		if(e && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered && ((CObject*)e)->ObjectCreatedBy == GAME_OBJECT){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}

	n = CPools::GetDummyPool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		e = CPools::GetDummyPool()->GetSlot(i);
		if(e && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}
}

void
CStreaming::RemoveUnusedBigBuildings(eLevelName level)
{
	ISLAND_LOADING_IS(LOW)
	{
		if (level != LEVEL_INDUSTRIAL)
			RemoveBigBuildings(LEVEL_INDUSTRIAL);
		if (level != LEVEL_COMMERCIAL)
			RemoveBigBuildings(LEVEL_COMMERCIAL);
		if (level != LEVEL_SUBURBAN)
			RemoveBigBuildings(LEVEL_SUBURBAN);
	}
	RemoveIslandsNotUsed(level);
}

void
DeleteIsland(CEntity *island)
{
	if(island == nil)
		return;
	if(island->bImBeingRendered)
		debug("Didn't delete island because it was being rendered\n");
	else{
		island->DeleteRwObject();
		CStreaming::RemoveModel(island->GetModelIndex());
	}
}

void
CStreaming::RemoveIslandsNotUsed(eLevelName level)
{
#ifdef NO_ISLAND_LOADING
	if (CMenuManager::m_PrefsIslandLoading == CMenuManager::ISLAND_LOADING_HIGH) {
		DeleteIsland(pIslandLODindustEntity);
		DeleteIsland(pIslandLODcomIndEntity);
		DeleteIsland(pIslandLODcomSubEntity);
		DeleteIsland(pIslandLODsubIndEntity);
		DeleteIsland(pIslandLODsubComEntity);
	} else
#endif
	switch(level){
	case LEVEL_INDUSTRIAL:
		DeleteIsland(pIslandLODindustEntity);
		DeleteIsland(pIslandLODcomSubEntity);
		DeleteIsland(pIslandLODsubComEntity);
		break;
	case LEVEL_COMMERCIAL:
		DeleteIsland(pIslandLODcomIndEntity);
		DeleteIsland(pIslandLODcomSubEntity);
		DeleteIsland(pIslandLODsubIndEntity);
		break;
	case LEVEL_SUBURBAN:
		DeleteIsland(pIslandLODsubIndEntity);
		DeleteIsland(pIslandLODsubComEntity);
		DeleteIsland(pIslandLODcomIndEntity);
		break;
	default:
		DeleteIsland(pIslandLODindustEntity);
		DeleteIsland(pIslandLODcomIndEntity);
		DeleteIsland(pIslandLODcomSubEntity);
		DeleteIsland(pIslandLODsubIndEntity);
		DeleteIsland(pIslandLODsubComEntity);
		break;
	}
}

void
CStreaming::RemoveBigBuildings(eLevelName level)
{
	int i, n;
	CEntity *e;
	CBaseModelInfo *mi;

	n = CPools::GetBuildingPool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		e = CPools::GetBuildingPool()->GetSlot(i);
		if(e && e->bIsBIGBuilding && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}
}

bool
CStreaming::RemoveLoadedVehicle(void)
{
	int i, id;

	for(i = 0; i < MAXVEHICLESLOADED; i++){
		ms_lastVehicleDeleted++;
		if(ms_lastVehicleDeleted == MAXVEHICLESLOADED)
			ms_lastVehicleDeleted = 0;
		id = ms_vehiclesLoaded[ms_lastVehicleDeleted];
		if(id != -1 &&
		   (ms_aInfoForModel[id].m_flags & STREAMFLAGS_CANT_REMOVE) == 0 && CModelInfo::GetModelInfo(id)->GetNumRefs() == 0 &&
		   ms_aInfoForModel[id].m_loadState == STREAMSTATE_LOADED)
			goto found;
	}
	return false;
found:
	RemoveModel(ms_vehiclesLoaded[ms_lastVehicleDeleted]);
	ms_numVehiclesLoaded--;
	ms_vehiclesLoaded[ms_lastVehicleDeleted] = -1;
	return true;
}

bool
CStreaming::RemoveLeastUsedModel(void)
{
	CStreamingInfo *si;
	int streamId;

	for(si = ms_endLoadedList.m_prev; si != &ms_startLoadedList; si = si->m_prev){
		streamId = si - ms_aInfoForModel;
		if(streamId < STREAM_OFFSET_TXD){
			if (CModelInfo::GetModelInfo(streamId)->GetNumRefs() == 0) {
				RemoveModel(streamId);
				return true;
			}
		}else{
			if(CTxdStore::GetNumRefs(streamId - STREAM_OFFSET_TXD) == 0 &&
			   !IsTxdUsedByRequestedModels(streamId - STREAM_OFFSET_TXD)){
				RemoveModel(streamId);
				return true;
			}
		}
	}
	return ms_numVehiclesLoaded > 7 && RemoveLoadedVehicle();
}

void
CStreaming::RemoveAllUnusedModels(void)
{
	int i;

	for(i = 0; i < MAXVEHICLESLOADED; i++)
		RemoveLoadedVehicle();

	for(i = NUM_DEFAULT_MODELS; i < MODELINFOSIZE; i++){
		if(ms_aInfoForModel[i].m_loadState == STREAMSTATE_LOADED &&
		   ms_aInfoForModel[i].m_flags & STREAMFLAGS_DONT_REMOVE &&
		    CModelInfo::GetModelInfo(i)->GetNumRefs() == 0) {
			RemoveModel(i);
			ms_aInfoForModel[i].m_loadState = STREAMSTATE_NOTLOADED;
		}
	}
}

bool
CStreaming::RemoveReferencedTxds(size_t mem)
{
	CStreamingInfo *si;
	int streamId;

	for(si = ms_endLoadedList.m_prev; si != &ms_startLoadedList; si = si->m_prev){
		streamId = si - ms_aInfoForModel;
		if(streamId >= STREAM_OFFSET_TXD &&
		   CTxdStore::GetNumRefs(streamId-STREAM_OFFSET_TXD) == 0){
			RemoveModel(streamId);
			if(ms_memoryUsed < mem)
				return true;
		}
	}
	return false;
}

void
CStreaming::RemoveUnusedModelsInLoadedList(void)
{
	// empty
}

bool
CStreaming::IsTxdUsedByRequestedModels(int32 txdId)
{
	CStreamingInfo *si;
	int streamId;
	int i;

	for(si = ms_startRequestedList.m_next; si != &ms_endRequestedList; si = si->m_next){
		streamId = si - ms_aInfoForModel;
		if(streamId < STREAM_OFFSET_TXD &&
		   CModelInfo::GetModelInfo(streamId)->GetTxdSlot() == txdId)
			return true;
	}

	for(i = 0; i < 4; i++){
		streamId = ms_channel[0].streamIds[i];
		if(streamId != -1 && streamId < STREAM_OFFSET_TXD &&
		   CModelInfo::GetModelInfo(streamId)->GetTxdSlot() == txdId)
			return true;
		streamId = ms_channel[1].streamIds[i];
		if(streamId != -1 && streamId < STREAM_OFFSET_TXD &&
		   CModelInfo::GetModelInfo(streamId)->GetTxdSlot() == txdId)
			return true;
	}

	return false;
}

int32
CStreaming::GetAvailableVehicleSlot(void)
{
	int i;
	for(i = 0; i < MAXVEHICLESLOADED; i++)
		if(ms_vehiclesLoaded[i] == -1)
			return i;
	return -1;
}

bool
CStreaming::AddToLoadedVehiclesList(int32 modelId)
{
	int i;
	int id;

	if(ms_numVehiclesLoaded < desiredNumVehiclesLoaded){
		// still room for vehicles
		for(i = 0; i < MAXVEHICLESLOADED; i++){
			if(ms_vehiclesLoaded[ms_lastVehicleDeleted] == -1)
				break;
			ms_lastVehicleDeleted++;
			if(ms_lastVehicleDeleted == MAXVEHICLESLOADED)
				ms_lastVehicleDeleted = 0;
		}
		assert(ms_vehiclesLoaded[ms_lastVehicleDeleted] == -1);
		ms_numVehiclesLoaded++;
	}else{
		// find vehicle we can remove
		for(i = 0; i < MAXVEHICLESLOADED; i++){
			id = ms_vehiclesLoaded[ms_lastVehicleDeleted];
			if(id != -1 &&
			   (ms_aInfoForModel[id].m_flags & STREAMFLAGS_CANT_REMOVE) == 0 && CModelInfo::GetModelInfo(id)->GetNumRefs() == 0)
				goto found;
			ms_lastVehicleDeleted++;
			if(ms_lastVehicleDeleted == MAXVEHICLESLOADED)
				ms_lastVehicleDeleted = 0;
		}
		id = -1;
found:
		if(id == -1){
			// didn't find anything, try a free slot
			id = GetAvailableVehicleSlot();
			if(id == -1)
				return false;	// still no luck
			ms_lastVehicleDeleted = id;
			// this is more than we wanted actually
			ms_numVehiclesLoaded++;
		}else
			RemoveModel(id);
	}

	ms_vehiclesLoaded[ms_lastVehicleDeleted++] = modelId;
	if(ms_lastVehicleDeleted == MAXVEHICLESLOADED)
		ms_lastVehicleDeleted = 0;
	return true;
}

bool
CStreaming::IsObjectInCdImage(int32 id)
{
	uint32 posn, size;
	return ms_aInfoForModel[id].GetCdPosnAndSize(posn, size);
}

void
CStreaming::HaveAllBigBuildingsLoaded(eLevelName level)
{
	int i, n;
	CEntity *e;

	if(ms_hasLoadedLODs)
		return;

	if(level == LEVEL_INDUSTRIAL){
		if(ms_aInfoForModel[islandLODcomInd].m_loadState != STREAMSTATE_LOADED ||
		   ms_aInfoForModel[islandLODsubInd].m_loadState != STREAMSTATE_LOADED)
			return;
	}else if(level == LEVEL_COMMERCIAL){
		if(ms_aInfoForModel[islandLODindust].m_loadState != STREAMSTATE_LOADED ||
		   ms_aInfoForModel[islandLODsubCom].m_loadState != STREAMSTATE_LOADED)
			return;
	}else if(level == LEVEL_SUBURBAN){
		if(ms_aInfoForModel[islandLODindust].m_loadState != STREAMSTATE_LOADED ||
		   ms_aInfoForModel[islandLODcomSub].m_loadState != STREAMSTATE_LOADED)
			return;
	}

	n = CPools::GetBuildingPool()->GetSize()-1;
	for(i = n; i >= 0; i--){
		e = CPools::GetBuildingPool()->GetSlot(i);
		if(e && e->bIsBIGBuilding && e->m_level == level &&
		   ms_aInfoForModel[e->GetModelIndex()].m_loadState != STREAMSTATE_LOADED)
			return;
	}

	RemoveUnusedBigBuildings(level);
	ms_hasLoadedLODs = true;
}

void
CStreaming::SetModelIsDeletable(int32 id)
{
	ms_aInfoForModel[id].m_flags &= ~STREAMFLAGS_DONT_REMOVE;
	if ((id >= STREAM_OFFSET_TXD || CModelInfo::GetModelInfo(id)->GetModelType() != MITYPE_VEHICLE) &&
	   (ms_aInfoForModel[id].m_flags & STREAMFLAGS_SCRIPTOWNED) == 0){
		if(ms_aInfoForModel[id].m_loadState != STREAMSTATE_LOADED)
			RemoveModel(id);
		else if(ms_aInfoForModel[id].m_next == nil)
			ms_aInfoForModel[id].AddToList(&ms_startLoadedList);
	}
}

void
CStreaming::SetModelTxdIsDeletable(int32 id)
{
	SetModelIsDeletable(CModelInfo::GetModelInfo(id)->GetTxdSlot() + STREAM_OFFSET_TXD);
}

void
CStreaming::SetMissionDoesntRequireModel(int32 id)
{
	ms_aInfoForModel[id].m_flags &= ~STREAMFLAGS_SCRIPTOWNED;
	if ((id >= STREAM_OFFSET_TXD || CModelInfo::GetModelInfo(id)->GetModelType() != MITYPE_VEHICLE) &&
	   (ms_aInfoForModel[id].m_flags & STREAMFLAGS_DONT_REMOVE) == 0){
		if(ms_aInfoForModel[id].m_loadState != STREAMSTATE_LOADED)
			RemoveModel(id);
		else if(ms_aInfoForModel[id].m_next == nil)
			ms_aInfoForModel[id].AddToList(&ms_startLoadedList);
	}
}

void
CStreaming::LoadInitialPeds(void)
{
	RequestModel(MI_COP, STREAMFLAGS_DONT_REMOVE);
	RequestModel(MI_MALE01, STREAMFLAGS_DONT_REMOVE);
	RequestModel(MI_TAXI_D, STREAMFLAGS_DONT_REMOVE);
}

void
CStreaming::LoadInitialVehicles(void)
{
	int id;

	ms_numVehiclesLoaded = 0;
	ms_lastVehicleDeleted = 0;

	if(CModelInfo::GetModelInfo("taxi", &id))
		RequestModel(id, STREAMFLAGS_DONT_REMOVE);
	if(CModelInfo::GetModelInfo("police", &id))
		RequestModel(id, STREAMFLAGS_DONT_REMOVE);
}

void
CStreaming::StreamVehiclesAndPeds(void)
{
	int i, model;
	static int timeBeforeNextLoad = 0;
	static int modelQualityClass = 0;

	if(CRecordDataForGame::IsRecording() ||
	   CRecordDataForGame::IsPlayingBack()
#ifdef FIX_BUGS
	   || CReplay::IsPlayingBack()
#endif
		)
		return;

	if(FindPlayerPed()->m_pWanted->AreSwatRequired()){
		RequestModel(MI_ENFORCER, STREAMFLAGS_DONT_REMOVE);
		RequestModel(MI_SWAT, STREAMFLAGS_DONT_REMOVE);
	}else{
		SetModelIsDeletable(MI_ENFORCER);
		if(!HasModelLoaded(MI_ENFORCER))
			SetModelIsDeletable(MI_SWAT);
	}

	if(FindPlayerPed()->m_pWanted->AreFbiRequired()){
		RequestModel(MI_FBICAR, STREAMFLAGS_DONT_REMOVE);
		RequestModel(MI_FBI, STREAMFLAGS_DONT_REMOVE);
	}else{
		SetModelIsDeletable(MI_FBICAR);
		if(!HasModelLoaded(MI_FBICAR))
			SetModelIsDeletable(MI_FBI);
	}

	if(FindPlayerPed()->m_pWanted->AreArmyRequired()){
		RequestModel(MI_RHINO, STREAMFLAGS_DONT_REMOVE);
		RequestModel(MI_BARRACKS, STREAMFLAGS_DONT_REMOVE);
		RequestModel(MI_ARMY, STREAMFLAGS_DONT_REMOVE);
	}else{
		SetModelIsDeletable(MI_RHINO);
		SetModelIsDeletable(MI_BARRACKS);
		if(!HasModelLoaded(MI_RHINO) && !HasModelLoaded(MI_BARRACKS))
			SetModelIsDeletable(MI_ARMY);
	}

	if(FindPlayerPed()->m_pWanted->NumOfHelisRequired() > 0)
		RequestModel(MI_CHOPPER, STREAMFLAGS_DONT_REMOVE);
	else
		SetModelIsDeletable(MI_CHOPPER);

	if(timeBeforeNextLoad >= 0)
		timeBeforeNextLoad--;
	else if(ms_numVehiclesLoaded <= desiredNumVehiclesLoaded){
		for(i = 1; i <= 10; i++){
			model =  CCarCtrl::ChooseCarModel(modelQualityClass);
			modelQualityClass++;
			if(modelQualityClass >= CCarCtrl::TOTAL_CUSTOM_CLASSES)
				modelQualityClass = 0;

			// check if we want to load this model
			if(ms_aInfoForModel[model].m_loadState == STREAMSTATE_NOTLOADED &&
			   ((CVehicleModelInfo*)CModelInfo::GetModelInfo(model))->m_level & (1 << (CGame::currLevel-1)))
				break;
		}

		if(i <= 10){
			RequestModel(model, STREAMFLAGS_DEPENDENCY);
			timeBeforeNextLoad = 500;
		}
	}
}

void
CStreaming::StreamZoneModels(const CVector &pos)
{
	int i;
	uint16 gangsToLoad, gangCarsToLoad, bit;
	CZoneInfo info;

	CTheZones::GetZoneInfoForTimeOfDay(&pos, &info);

	if(info.pedGroup != ms_currentPedGrp){

		// unload pevious group
		if(ms_currentPedGrp != -1)
			for(i = 0; i < NUMMODELSPERPEDGROUP; i++){
				if(CPopulation::ms_pPedGroups[ms_currentPedGrp].models[i] != -1){
					SetModelIsDeletable(CPopulation::ms_pPedGroups[ms_currentPedGrp].models[i]);
					SetModelTxdIsDeletable(CPopulation::ms_pPedGroups[ms_currentPedGrp].models[i]);
				}
			}

		ms_currentPedGrp = info.pedGroup;

		for(i = 0; i < NUMMODELSPERPEDGROUP; i++){
			if(CPopulation::ms_pPedGroups[ms_currentPedGrp].models[i] != -1)
				RequestModel(CPopulation::ms_pPedGroups[ms_currentPedGrp].models[i], STREAMFLAGS_DONT_REMOVE);
		}
	}
	RequestModel(MI_MALE01, STREAMFLAGS_DONT_REMOVE);

	gangsToLoad = 0;
	gangCarsToLoad = 0;
	if(info.gangDensity[0] != 0) gangsToLoad |= 1<<0;
	if(info.gangDensity[1] != 0) gangsToLoad |= 1<<1;
	if(info.gangDensity[2] != 0) gangsToLoad |= 1<<2;
	if(info.gangDensity[3] != 0) gangsToLoad |= 1<<3;
	if(info.gangDensity[4] != 0) gangsToLoad |= 1<<4;
	if(info.gangDensity[5] != 0) gangsToLoad |= 1<<5;
	if(info.gangDensity[6] != 0) gangsToLoad |= 1<<6;
	if(info.gangDensity[7] != 0) gangsToLoad |= 1<<7;
	if(info.gangDensity[8] != 0) gangsToLoad |= 1<<8;
	if(info.gangThreshold[0] != info.copDensity) gangCarsToLoad |= 1<<0;
	if(info.gangThreshold[1] != info.gangThreshold[0]) gangCarsToLoad |= 1<<1;
	if(info.gangThreshold[2] != info.gangThreshold[1]) gangCarsToLoad |= 1<<2;
	if(info.gangThreshold[3] != info.gangThreshold[2]) gangCarsToLoad |= 1<<3;
	if(info.gangThreshold[4] != info.gangThreshold[3]) gangCarsToLoad |= 1<<4;
	if(info.gangThreshold[5] != info.gangThreshold[4]) gangCarsToLoad |= 1<<5;
	if(info.gangThreshold[6] != info.gangThreshold[5]) gangCarsToLoad |= 1<<6;
	if(info.gangThreshold[7] != info.gangThreshold[6]) gangCarsToLoad |= 1<<7;
	if(info.gangThreshold[8] != info.gangThreshold[7]) gangCarsToLoad |= 1<<8;

	if(gangsToLoad == ms_loadedGangs && gangCarsToLoad == ms_loadedGangCars)
		return;

	// This makes things simpler than the game does it
	gangsToLoad |= gangCarsToLoad;

	for(i = 0; i < NUM_GANGS; i++){
		bit = 1<<i;

		if(gangsToLoad & bit && (ms_loadedGangs & bit) == 0){
			RequestModel(MI_GANG01 + i*2, STREAMFLAGS_DONT_REMOVE);
			RequestModel(MI_GANG01 + i*2 + 1, STREAMFLAGS_DONT_REMOVE);
			ms_loadedGangs |= bit;
		}else if((gangsToLoad & bit) == 0 && ms_loadedGangs & bit){
			SetModelIsDeletable(MI_GANG01 + i*2);
			SetModelIsDeletable(MI_GANG01 + i*2 + 1);
			SetModelTxdIsDeletable(MI_GANG01 + i*2);
			SetModelTxdIsDeletable(MI_GANG01 + i*2 + 1);
			ms_loadedGangs &= ~bit;
		}

		if(gangCarsToLoad & bit && (ms_loadedGangCars & bit) == 0){
			RequestModel(CGangs::GetGangInfo(i)->m_nVehicleMI, STREAMFLAGS_DONT_REMOVE);
		}else if((gangCarsToLoad & bit) == 0 && ms_loadedGangCars & bit){
			SetModelIsDeletable(CGangs::GetGangInfo(i)->m_nVehicleMI);
			SetModelTxdIsDeletable(CGangs::GetGangInfo(i)->m_nVehicleMI);
		}
	}
	ms_loadedGangCars = gangCarsToLoad;
}

void
CStreaming::RemoveCurrentZonesModels(void)
{
	int i;

	if(ms_currentPedGrp != -1)
		for(i = 0; i < 8; i++){
			if(CPopulation::ms_pPedGroups[ms_currentPedGrp].models[i] == -1)
				break;
			if(CPopulation::ms_pPedGroups[ms_currentPedGrp].models[i] != MI_MALE01)
				SetModelIsDeletable(CPopulation::ms_pPedGroups[ms_currentPedGrp].models[i]);
		}

	for(i = 0; i < NUM_GANGS; i++){
		SetModelIsDeletable(MI_GANG01 + i*2);
		SetModelIsDeletable(MI_GANG01 + i*2 + 1);
		if(CGangs::GetGangInfo(i)->m_nVehicleMI != -1)
			SetModelIsDeletable(CGangs::GetGangInfo(i)->m_nVehicleMI);
	}

	ms_currentPedGrp = -1;
	ms_loadedGangs = 0;
	ms_loadedGangCars = 0;
}


// Find starting offset of the cdimage we next want to read
// Not useful at all on PC...
int32
CStreaming::GetCdImageOffset(int32 lastPosn)
{
	int offset, off;
	int i, img;
	int dist, mindist;

	img = -1;
	mindist = INT32_MAX;
	offset = ms_imageOffsets[ms_lastImageRead];
	if(lastPosn <= offset || lastPosn > offset + ms_imageSize){
		// last read position is not in last image
		for(i = 0; i < NUMCDIMAGES; i++){
			off = ms_imageOffsets[i];
			if(off == -1) continue;
			if((uint32)lastPosn > (uint32)off)
				// after start of image, get distance from end
				// negative if before end!
				dist = lastPosn - (off + ms_imageSize);
			else
				// before image, get offset to start
				// this will never be negative
				dist = off - lastPosn;
			if(dist < mindist){
				img = i;
				mindist = dist;
			}
		}
		assert(img >= 0);
		offset = ms_imageOffsets[img];
		ms_lastImageRead = img;
	}
	return offset;
}

inline bool
ModelNotLoaded(int32 modelId)
{
	CStreamingInfo *si = &CStreaming::ms_aInfoForModel[modelId];
	return si->m_loadState != STREAMSTATE_LOADED && si->m_loadState != STREAMSTATE_READING;
}

inline bool TxdNotLoaded(int32 txdId) { return ModelNotLoaded(txdId + STREAM_OFFSET_TXD); }

// Find stream id of next requested file in cdimage
int32
CStreaming::GetNextFileOnCd(int32 lastPosn, bool priority)
{
	CStreamingInfo *si, *next;
	int streamId;
	uint32 posn, size;
	int streamIdFirst, streamIdNext;
	uint32 posnFirst, posnNext;

	streamIdFirst = -1;
	streamIdNext = -1;
	posnFirst = UINT32_MAX;
	posnNext = UINT32_MAX;

	for(si = ms_startRequestedList.m_next; si != &ms_endRequestedList; si = next){
		next = si->m_next;
		streamId = si - ms_aInfoForModel;

		// only priority requests if there are any
		if(priority && ms_numPriorityRequests != 0 && !si->IsPriority())
			continue;

		// request Txd if necessary
		if(streamId < STREAM_OFFSET_TXD){
			int txdId = CModelInfo::GetModelInfo(streamId)->GetTxdSlot();
			if(TxdNotLoaded(txdId)){
				ReRequestTxd(txdId);
				continue;
			}
		}

		if(ms_aInfoForModel[streamId].GetCdPosnAndSize(posn, size)){
			if(posn < posnFirst){
				// find first requested file in image
				streamIdFirst = streamId;
				posnFirst = posn;
			}
			if(posn < posnNext && posn >= (uint32)lastPosn){
				// find first requested file after last read position
				streamIdNext = streamId;
				posnNext = posn;
			}
		}else{
			// empty file
			DecrementRef(streamId);
			si->RemoveFromList();
			si->m_loadState = STREAMSTATE_LOADED;
		}
	}

	// wrap around
	if(streamIdNext == -1)
		streamIdNext = streamIdFirst;

	if(streamIdNext == -1 && ms_numPriorityRequests != 0){
		// try non-priority files
		ms_numPriorityRequests = 0;
		streamIdNext = GetNextFileOnCd(lastPosn, false);
	}

	return streamIdNext;
}

/*
 * Streaming buffer size is half of the largest file.
 * Files larger than the buffer size can only be loaded by channel 0,
 * which then uses both buffers, while channel 1 is idle.
 * ms_bLoadingBigModel is set to true to indicate this state.
 */

// Make channel read from disc
void
CStreaming::RequestModelStream(int32 ch)
{
	int lastPosn, imgOffset, streamId;
	int totalSize;
	uint32 posn, size, unused;
	int i;
	int haveBigFile, havePed;

	lastPosn = CdStreamGetLastPosn();
	imgOffset = GetCdImageOffset(lastPosn);
	streamId = GetNextFileOnCd(lastPosn - imgOffset, true);

	if(streamId == -1)
		return;

	// remove Txds that aren't requested anymore
	while(streamId >= STREAM_OFFSET_TXD){
		if(ms_aInfoForModel[streamId].m_flags & STREAMFLAGS_KEEP_IN_MEMORY ||
		   IsTxdUsedByRequestedModels(streamId - STREAM_OFFSET_TXD))
			break;
		RemoveModel(streamId);
		// so try next file
		ms_aInfoForModel[streamId].GetCdPosnAndSize(posn, size);
		streamId = GetNextFileOnCd(posn + size, true);
	}

	if(streamId == -1)
		return;

	ms_aInfoForModel[streamId].GetCdPosnAndSize(posn, size);
	if(size > (uint32)ms_streamingBufferSize){
		// Can only load big models on channel 0, and 1 has to be idle
		if(ch == 1 || ms_channel[1].state != CHANNELSTATE_IDLE)
			return;
		ms_bLoadingBigModel = true;
	}

	// Load up to 4 adjacent files
	haveBigFile = 0;
	havePed = 0;
	totalSize = 0;
	for(i = 0; i < 4; i++){
		// no more files we can read
		if(streamId == -1 || ms_aInfoForModel[streamId].m_loadState != STREAMSTATE_INQUEUE)
			break;

		// also stop at non-priority files
		ms_aInfoForModel[streamId].GetCdPosnAndSize(unused, size);
		if(ms_numPriorityRequests != 0 && !ms_aInfoForModel[streamId].IsPriority())
			break;

		// Can't load certain combinations of files together
		if(streamId < STREAM_OFFSET_TXD){
			if (havePed && CModelInfo::GetModelInfo(streamId)->GetModelType() == MITYPE_PED ||
			    haveBigFile && CModelInfo::GetModelInfo(streamId)->GetModelType() == MITYPE_VEHICLE ||
			    TxdNotLoaded(CModelInfo::GetModelInfo(streamId)->GetTxdSlot()))
				break;
		}else{
			if(haveBigFile && size > 200)
				break;
		}

		// Now add the file
		ms_channel[ch].streamIds[i] = streamId;
		ms_channel[ch].offsets[i] = totalSize;
		totalSize += size;

		// To big for buffer, remove again
		if(totalSize > ms_streamingBufferSize && i > 0){
			totalSize -= size;
			break;
		}
		if(streamId < STREAM_OFFSET_TXD){
			if (CModelInfo::GetModelInfo(streamId)->GetModelType() == MITYPE_PED)
				havePed = 1;
			if (CModelInfo::GetModelInfo(streamId)->GetModelType() == MITYPE_VEHICLE)
				haveBigFile = 1;
		}else{
			if(size > 200)
				haveBigFile = 1;
		}
		ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_READING;
		ms_aInfoForModel[streamId].RemoveFromList();
		DecrementRef(streamId);

		streamId = ms_aInfoForModel[streamId].m_nextID;
	}

	// clear remaining slots
	for(; i < 4; i++)
		ms_channel[ch].streamIds[i] = -1;
	// Now read the data
	assert(!(ms_bLoadingBigModel && ch == 1));	// this would clobber the buffer
	if(CdStreamRead(ch, ms_pStreamingBuffer[ch], imgOffset+posn, totalSize) == STREAM_NONE)
		debug("FUCKFUCKFUCK\n");
	ms_channel[ch].state = CHANNELSTATE_READING;
	ms_channel[ch].field24 = 0;
	ms_channel[ch].size = totalSize;
	ms_channel[ch].position = imgOffset+posn;
	ms_channel[ch].numTries = 0;
}

// Load data previously read from disc
bool
CStreaming::ProcessLoadingChannel(int32 ch)
{
	int status;
	int i, id, cdsize;

	status = CdStreamGetStatus(ch);
	if(status != STREAM_NONE){
		// busy
		if(status != STREAM_READING && status != STREAM_WAITING){
			ms_channelError = ch;
			ms_channel[ch].state = CHANNELSTATE_ERROR;
			ms_channel[ch].status = status;
		}
		return false;
	}

	if(ms_channel[ch].state == CHANNELSTATE_STARTED){
		ms_channel[ch].state = CHANNELSTATE_IDLE;
		FinishLoadingLargeFile(&ms_pStreamingBuffer[ch][ms_channel[ch].offsets[0]*CDSTREAM_SECTOR_SIZE],
			ms_channel[ch].streamIds[0]);
		ms_channel[ch].streamIds[0] = -1;
	}else{
		ms_channel[ch].state = CHANNELSTATE_IDLE;
		for(i = 0; i < 4; i++){
			id = ms_channel[ch].streamIds[i];
			if(id == -1)
				continue;

			cdsize = ms_aInfoForModel[id].GetCdSize();
			if(id < STREAM_OFFSET_TXD && CModelInfo::GetModelInfo(id)->GetModelType() == MITYPE_VEHICLE &&
			   ms_numVehiclesLoaded >= desiredNumVehiclesLoaded &&
			   !RemoveLoadedVehicle() &&
			   ((ms_aInfoForModel[id].m_flags & STREAMFLAGS_CANT_REMOVE) == 0 || GetAvailableVehicleSlot() == -1)){
				// can't load vehicle
				RemoveModel(id);
				if(ms_aInfoForModel[id].m_flags & STREAMFLAGS_CANT_REMOVE)
					ReRequestModel(id);
				else if(CTxdStore::GetNumRefs(CModelInfo::GetModelInfo(id)->GetTxdSlot()) == 0)
					RemoveTxd(CModelInfo::GetModelInfo(id)->GetTxdSlot());
			}else{
				MakeSpaceFor(cdsize * CDSTREAM_SECTOR_SIZE);
				ConvertBufferToObject(&ms_pStreamingBuffer[ch][ms_channel[ch].offsets[i]*CDSTREAM_SECTOR_SIZE],
					id);
				if(ms_aInfoForModel[id].m_loadState == STREAMSTATE_STARTED){
					// queue for second part
					ms_channel[ch].state = CHANNELSTATE_STARTED;
					ms_channel[ch].offsets[0] = ms_channel[ch].offsets[i];
					ms_channel[ch].streamIds[0] = id;
					if(i != 0)
						ms_channel[ch].streamIds[i] = -1;
				}else
					ms_channel[ch].streamIds[i] = -1;
			}
		}
	}

	if(ms_bLoadingBigModel && ms_channel[ch].state != CHANNELSTATE_STARTED){
		ms_bLoadingBigModel = false;
		// reset channel 1 after loading a big model
		for(i = 0; i < 4; i++)
			ms_channel[1].streamIds[i] = -1;
		ms_channel[1].state = CHANNELSTATE_IDLE;
	}

	return true;
}

void
CStreaming::RetryLoadFile(int32 ch)
{
	Const char *key;

	CPad::StopPadsShaking();

	if(ms_channel[ch].numTries >= 3){
		switch(ms_channel[ch].status){
		case STREAM_ERROR_NOCD: key = "NOCD"; break;
		case STREAM_ERROR_OPENCD: key = "OPENCD"; break;
		case STREAM_ERROR_WRONGCD: key = "WRONGCD"; break;
		default: key = "CDERROR"; break;
		}
		CHud::SetMessage(TheText.Get(key));
		CTimer::SetCodePause(true);
	}

	switch(ms_channel[ch].state){
	case CHANNELSTATE_ERROR:
		ms_channel[ch].numTries++;
		if (CdStreamGetStatus(ch) == STREAM_READING || CdStreamGetStatus(ch) == STREAM_WAITING) break;
	case CHANNELSTATE_IDLE:
		CdStreamRead(ch, ms_pStreamingBuffer[ch], ms_channel[ch].position, ms_channel[ch].size);
		ms_channel[ch].state = CHANNELSTATE_READING;
		ms_channel[ch].field24 = -600;
		break;
	case CHANNELSTATE_READING:
		if(ProcessLoadingChannel(ch)){
			ms_channelError = -1;
			CTimer::SetCodePause(false);
		}
		break;
	}
}

void
CStreaming::LoadRequestedModels(void)
{
	static int currentChannel = 0;

	// We can't read with channel 1 while channel 0 is using its buffer
	if(ms_bLoadingBigModel)
		currentChannel = 0;

	// We have data, load
	if(ms_channel[currentChannel].state == CHANNELSTATE_READING ||
	   ms_channel[currentChannel].state == CHANNELSTATE_STARTED)
		ProcessLoadingChannel(currentChannel);

	if(ms_channelError == -1){
		// Channel is idle, read more data
		if(ms_channel[currentChannel].state == CHANNELSTATE_IDLE)
			RequestModelStream(currentChannel);
		// Switch channel
		if(ms_channel[currentChannel].state != CHANNELSTATE_STARTED)
			currentChannel = 1 - currentChannel;
	}
}


// Let's load models in 4 threads; when one of them becomes idle, process the file, and fill thread with another file. Unfortunately processing models are still single-threaded.
// Currently only supported on POSIX streamer.
// WIP - some files are loaded swapped (CdStreamPosix problem?)
#if 0 //def ONE_THREAD_PER_CHANNEL
void
CStreaming::LoadAllRequestedModels(bool priority)
{
	static bool bInsideLoadAll = false;
	int imgOffset, streamId, status;
	int i;
	uint32 posn, size;

	if(bInsideLoadAll)
		return;

	FlushChannels();
	imgOffset = GetCdImageOffset(CdStreamGetLastPosn());

	int streamIds[ARRAY_SIZE(ms_pStreamingBuffer)];
	int streamSizes[ARRAY_SIZE(ms_pStreamingBuffer)];
	int streamPoses[ARRAY_SIZE(ms_pStreamingBuffer)];
	int readOrder[4] = {-1}; // Channel IDs ordered by read time
	int readI = 0;
	int processI = 0;
	bool first = true;

	// All those "first" checks are because of variables aren't initialized in first pass.

	while (true) {
		for (int i=0; i<ARRAY_SIZE(ms_pStreamingBuffer); i++) {

			// Channel has file to load
			if (!first && streamIds[i] != -1) {
				continue;
			}

			if(ms_endRequestedList.m_prev != &ms_startRequestedList){
				streamId = GetNextFileOnCd(0, priority);
				if(streamId == -1){
					streamIds[i] = -1;
					break;
				}

				if (ms_aInfoForModel[streamId].GetCdPosnAndSize(posn, size)) {
					streamIds[i] = -1;

					// Big file, needs 2 buffer
					if (size > (uint32)ms_streamingBufferSize) {
						if (i + 1 == ARRAY_SIZE(ms_pStreamingBuffer))
							break;
						else if (!first && streamIds[i+1] != -1)
							continue;

					} else {
						// Buffer of current channel is part of a "big file", pass
						if (i != 0 && streamIds[i-1] != -1 && streamSizes[i-1] > (uint32)ms_streamingBufferSize)
							continue;
					}
					ms_aInfoForModel[streamId].RemoveFromList();
					DecrementRef(streamId);

					streamIds[i] = streamId;
					streamSizes[i] = size;
					streamPoses[i] = posn;

					if (!first)
						assert(readOrder[readI] == -1);

					//printf("read: order %d, ch %d, id %d, size %d\n", readI, i, streamId, size);

					CdStreamRead(i, ms_pStreamingBuffer[i], imgOffset+posn, size);
					readOrder[readI] = i;
					if (first && readI+1 != ARRAY_SIZE(readOrder))
						readOrder[readI+1] = -1;

					readI = (readI + 1) % ARRAY_SIZE(readOrder);
				} else {
					ms_aInfoForModel[streamId].RemoveFromList();
					DecrementRef(streamId);

					ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_LOADED;
					streamIds[i] = -1;
				}
			} else {
				streamIds[i] = -1;
				break;
			}
		}

		first = false;
		int nextChannel = readOrder[processI];

		// Now start processing
		if (nextChannel == -1 || streamIds[nextChannel] == -1)
			break;

		//printf("process: order %d, ch %d, id %d\n", processI, nextChannel, streamIds[nextChannel]);

		// Try again on error
		while (CdStreamSync(nextChannel) != STREAM_NONE) {
			CdStreamRead(nextChannel, ms_pStreamingBuffer[nextChannel], imgOffset+streamPoses[nextChannel], streamSizes[nextChannel]);
		}
		ms_aInfoForModel[streamIds[nextChannel]].m_loadState = STREAMSTATE_READING;

		MakeSpaceFor(streamSizes[nextChannel] * CDSTREAM_SECTOR_SIZE);
		ConvertBufferToObject(ms_pStreamingBuffer[nextChannel], streamIds[nextChannel]);
		if(ms_aInfoForModel[streamIds[nextChannel]].m_loadState == STREAMSTATE_STARTED)
			FinishLoadingLargeFile(ms_pStreamingBuffer[nextChannel], streamIds[nextChannel]);

		if(streamIds[nextChannel] < STREAM_OFFSET_TXD){
			CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(streamIds[nextChannel]);
			if(mi->IsSimple())
				mi->m_alpha = 255;
		}
		streamIds[nextChannel] = -1;
		readOrder[processI] = -1;
		processI = (processI + 1) % ARRAY_SIZE(readOrder);
	}

	ms_bLoadingBigModel = false;
	for(i = 0; i < 4; i++){
		ms_channel[1].streamIds[i] = -1;
		ms_channel[1].offsets[i] = -1;
	}
	ms_channel[1].state = CHANNELSTATE_IDLE;
	bInsideLoadAll = false;
}
#else
void
CStreaming::LoadAllRequestedModels(bool priority)
{
	static bool bInsideLoadAll = false;
	int imgOffset, streamId, status;
	int i;
	uint32 posn, size;

	if(bInsideLoadAll)
		return;

	FlushChannels();
	imgOffset = GetCdImageOffset(CdStreamGetLastPosn());

	while(ms_endRequestedList.m_prev != &ms_startRequestedList){
		streamId = GetNextFileOnCd(0, priority);
		if(streamId == -1)
			break;

		ms_aInfoForModel[streamId].RemoveFromList();
		DecrementRef(streamId);

		if(ms_aInfoForModel[streamId].GetCdPosnAndSize(posn, size)){
			do
				status = CdStreamRead(0, ms_pStreamingBuffer[0], imgOffset+posn, size);
			while(CdStreamSync(0) || status == STREAM_NONE);
			ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_READING;

			MakeSpaceFor(size * CDSTREAM_SECTOR_SIZE);
			ConvertBufferToObject(ms_pStreamingBuffer[0], streamId);
			if(ms_aInfoForModel[streamId].m_loadState == STREAMSTATE_STARTED)
				FinishLoadingLargeFile(ms_pStreamingBuffer[0], streamId);

			if(streamId < STREAM_OFFSET_TXD){
				CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(streamId);
				if(mi->IsSimple())
					mi->m_alpha = 255;
			}
		}else{
			// empty
			ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_LOADED;
		}
	}

	ms_bLoadingBigModel = false;
	for(i = 0; i < 4; i++){
		ms_channel[1].streamIds[i] = -1;
		ms_channel[1].offsets[i] = -1;
	}
	ms_channel[1].state = CHANNELSTATE_IDLE;
	bInsideLoadAll = false;
}
#endif

void
CStreaming::FlushChannels(void)
{
	if(ms_channel[1].state == CHANNELSTATE_STARTED)
		ProcessLoadingChannel(1);

	if(ms_channel[0].state == CHANNELSTATE_READING){
		CdStreamSync(0);
		ProcessLoadingChannel(0);
	}
	if(ms_channel[0].state == CHANNELSTATE_STARTED)
		ProcessLoadingChannel(0);

	if(ms_channel[1].state == CHANNELSTATE_READING){
		CdStreamSync(1);
		ProcessLoadingChannel(1);
	}
	if(ms_channel[1].state == CHANNELSTATE_STARTED)
		ProcessLoadingChannel(1);
}

void
CStreaming::FlushRequestList(void)
{
	CStreamingInfo *si, *next;

	for(si = ms_startRequestedList.m_next; si != &ms_endRequestedList; si = next){
		next = si->m_next;
		RemoveModel(si - ms_aInfoForModel);
	}
#ifdef FLUSHABLE_STREAMING
	if(ms_channel[0].state == CHANNELSTATE_READING) {
		flushStream[0] = 1;
	}
	if(ms_channel[1].state == CHANNELSTATE_READING) {
		flushStream[1] = 1;
	}
#endif
	FlushChannels();
}


void
CStreaming::ImGonnaUseStreamingMemory(void)
{
	PUSH_MEMID(MEMID_STREAM);
}

void
CStreaming::IHaveUsedStreamingMemory(void)
{
	POP_MEMID();
	UpdateMemoryUsed();
}

void
CStreaming::UpdateMemoryUsed(void)
{
#ifdef USE_CUSTOM_ALLOCATOR
	ms_memoryUsed =
		gMainHeap.GetMemoryUsed(MEMID_STREAM) +
		gMainHeap.GetMemoryUsed(MEMID_STREAM_MODELS) +
		gMainHeap.GetMemoryUsed(MEMID_STREAM_TEXUTRES);
#endif
}

#define STREAM_DIST 80.0f

void
CStreaming::AddModelsToRequestList(const CVector &pos)
{
	float xmin, xmax, ymin, ymax;
	int ixmin, ixmax, iymin, iymax;
	int ix, iy;
	int dx, dy, d;
	CSector *sect;

	xmin = pos.x - STREAM_DIST;
	ymin = pos.y - STREAM_DIST;
	xmax = pos.x + STREAM_DIST;
	ymax = pos.y + STREAM_DIST;

	ixmin = CWorld::GetSectorIndexX(xmin);
	if(ixmin < 0) ixmin = 0;
	ixmax = CWorld::GetSectorIndexX(xmax);
	if(ixmax >= NUMSECTORS_X) ixmax = NUMSECTORS_X-1;
	iymin = CWorld::GetSectorIndexY(ymin);
	if(iymin < 0) iymin = 0;
	iymax = CWorld::GetSectorIndexY(ymax);
	if(iymax >= NUMSECTORS_Y) iymax = NUMSECTORS_Y-1;

	CWorld::AdvanceCurrentScanCode();

	for(iy = iymin; iy <= iymax; iy++){
		dy = iy - CWorld::GetSectorIndexY(pos.y);
		for(ix = ixmin; ix <= ixmax; ix++){

			if(CRenderer::m_loadingPriority && ms_numModelsRequested > 5)
				return;

			dx = ix - CWorld::GetSectorIndexX(pos.x);
			d = dx*dx + dy*dy;
			sect = CWorld::GetSector(ix, iy);
			if(d <= 1){
				ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
				ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
				ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
				ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
			}else if(d <= 4*4){
				ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], pos.x, pos.y, xmin, ymin, xmax, ymax);
				ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], pos.x, pos.y, xmin, ymin, xmax, ymax);
				ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], pos.x, pos.y, xmin, ymin, xmax, ymax);
				ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], pos.x, pos.y, xmin, ymin, xmax, ymax);
			}
		}
	}
}

void
CStreaming::ProcessEntitiesInSectorList(CPtrList &list, float x, float y, float xmin, float ymin, float xmax, float ymax)
{
	CPtrNode *node;
	CEntity *e;
	float lodDistSq;
	CVector2D pos;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;

		if(e->m_scanCode == CWorld::GetCurrentScanCode())
			continue;

		e->m_scanCode = CWorld::GetCurrentScanCode();
		if(!e->bStreamingDontDelete && !e->bIsSubway &&
		   (!e->IsObject() || ((CObject*)e)->ObjectCreatedBy != TEMP_OBJECT)){
			CTimeModelInfo *mi = (CTimeModelInfo*)CModelInfo::GetModelInfo(e->GetModelIndex());
			if (mi->GetModelType() != MITYPE_TIME || CClock::GetIsTimeInRange(mi->GetTimeOn(), mi->GetTimeOff())) {
				lodDistSq = sq(mi->GetLargestLodDistance());
				lodDistSq = Min(lodDistSq, sq(STREAM_DIST));
				pos = CVector2D(e->GetPosition());
				if(xmin < pos.x && pos.x < xmax &&
				   ymin < pos.y && pos.y < ymax &&
				   (CVector2D(x, y) - pos).MagnitudeSqr() < lodDistSq)
					if(CRenderer::IsEntityCullZoneVisible(e))
						RequestModel(e->GetModelIndex(), 0);
			}
		}
	}
}

void
CStreaming::ProcessEntitiesInSectorList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;

		if(e->m_scanCode == CWorld::GetCurrentScanCode())
			continue;

		e->m_scanCode = CWorld::GetCurrentScanCode();
		if(!e->bStreamingDontDelete && !e->bIsSubway &&
		   (!e->IsObject() || ((CObject*)e)->ObjectCreatedBy != TEMP_OBJECT)){
			CTimeModelInfo *mi = (CTimeModelInfo*)CModelInfo::GetModelInfo(e->GetModelIndex());
			if (mi->GetModelType() != MITYPE_TIME || CClock::GetIsTimeInRange(mi->GetTimeOn(), mi->GetTimeOff()))
				if(CRenderer::IsEntityCullZoneVisible(e))
					RequestModel(e->GetModelIndex(), 0);
		}
	}
}

void
CStreaming::DeleteFarAwayRwObjects(const CVector &pos)
{
	int posx, posy;
	int x, y;
	int r, i;
	CSector *sect;

	posx = CWorld::GetSectorIndexX(pos.x);
	posy = CWorld::GetSectorIndexY(pos.y);

	// Move oldSectorX/Y to new sector and delete RW objects in its "wake" for every step:
	// O is the old sector, <- is the direction in which we move it,
	// X are the sectors we delete RW objects from (except we go up to 10)
	//            X
	//          X X
	//        X X X
	//        X X X
	// <- O   X X X
	//        X X X
	//        X X X
	//          X X
	//            X

	while(posx != ms_oldSectorX){
		if(posx < ms_oldSectorX){
			for(r = 2; r <= 10; r++){
				x = ms_oldSectorX + r;
				if(x < 0)
					continue;
				if(x >= NUMSECTORS_X)
					break;

				for(i = -r; i <= r; i++){
					y = ms_oldSectorY + i;
					if(y < 0)
						continue;
					if(y >= NUMSECTORS_Y)
						break;

					sect = CWorld::GetSector(x, y);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
					DeleteRwObjectsInOverlapSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], ms_oldSectorX, ms_oldSectorY);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				}
			}
			ms_oldSectorX--;
		}else{
			for(r = 2; r <= 10; r++){
				x = ms_oldSectorX - r;
				if(x < 0)
					break;
				if(x >= NUMSECTORS_X)
					continue;

				for(i = -r; i <= r; i++){
					y = ms_oldSectorY + i;
					if(y < 0)
						continue;
					if(y >= NUMSECTORS_Y)
						break;

					sect = CWorld::GetSector(x, y);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
					DeleteRwObjectsInOverlapSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], ms_oldSectorX, ms_oldSectorY);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				}
			}
			ms_oldSectorX++;
		}
	}

	while(posy != ms_oldSectorY){
		if(posy < ms_oldSectorY){
			for(r = 2; r <= 10; r++){
				y = ms_oldSectorY + r;
				if(y < 0)
					continue;
				if(y >= NUMSECTORS_Y)
					break;

				for(i = -r; i <= r; i++){
					x = ms_oldSectorX + i;
					if(x < 0)
						continue;
					if(x >= NUMSECTORS_X)
						break;

					sect = CWorld::GetSector(x, y);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
					DeleteRwObjectsInOverlapSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], ms_oldSectorX, ms_oldSectorY);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				}
			}
			ms_oldSectorY--;
		}else{
			for(r = 2; r <= 10; r++){
				y = ms_oldSectorY - r;
				if(y < 0)
					break;
				if(y >= NUMSECTORS_Y)
					continue;

				for(i = -r; i <= r; i++){
					x = ms_oldSectorX + i;
					if(x < 0)
						continue;
					if(x >= NUMSECTORS_X)
						break;

					sect = CWorld::GetSector(x, y);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
					DeleteRwObjectsInOverlapSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], ms_oldSectorX, ms_oldSectorY);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
					DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				}
			}
			ms_oldSectorY++;
		}
	}
}

void
CStreaming::DeleteAllRwObjects(void)
{
	int x, y;
	CSector *sect;

	for(x = 0; x < NUMSECTORS_X; x++)
		for(y = 0; y < NUMSECTORS_Y; y++){
			sect = CWorld::GetSector(x, y);
			DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
			DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
			DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
			DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS_OVERLAP]);
			DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
			DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
		}
}

void
CStreaming::DeleteRwObjectsAfterDeath(const CVector &pos)
{
	int ix, iy;
	int x, y;
	CSector *sect;

	ix = CWorld::GetSectorIndexX(pos.x);
	iy = CWorld::GetSectorIndexY(pos.y);

	for(x = 0; x < NUMSECTORS_X; x++)
		for(y = 0; y < NUMSECTORS_Y; y++)
			if(Abs(ix - x) > 3.0f &&
			   Abs(iy - y) > 3.0f){
				sect = CWorld::GetSector(x, y);
				DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
				DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
				DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
				DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS_OVERLAP]);
				DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
			}
}

void
CStreaming::DeleteRwObjectsBehindCamera(size_t mem)
{
	int ix, iy;
	int x, y;
	int xmin, xmax, ymin, ymax;
	int inc;
	CSector *sect;

	if(ms_memoryUsed < mem)
		return;

	ix = CWorld::GetSectorIndexX(TheCamera.GetPosition().x);
	iy = CWorld::GetSectorIndexY(TheCamera.GetPosition().y);

	if(Abs(TheCamera.GetForward().x) > Abs(TheCamera.GetForward().y)){
		// looking west/east

		ymin = Max(iy - 10, 0);
		ymax = Min(iy + 10, NUMSECTORS_Y - 1);
		assert(ymin <= ymax);

		// Delete a block of sectors that we know is behind the camera
		if(TheCamera.GetForward().x > 0.0f){
			// looking east
			xmax = Max(ix - 2, 0);
			xmin = Max(ix - 10, 0);
			inc = 1;
		}else{
			// looking west
			xmax = Min(ix + 2, NUMSECTORS_X - 1);
			xmin = Min(ix + 10, NUMSECTORS_X - 1);
			inc = -1;
		}
		for(y = ymin; y <= ymax; y++){
			for(x = xmin; x != xmax; x += inc){
				sect = CWorld::GetSector(x, y);
				if(DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}

		// Now a block that intersects with the camera's frustum
		if(TheCamera.GetForward().x > 0.0f){
			// looking east
			xmax = Max(ix + 10, 0);
			xmin = Max(ix - 2, 0);
			inc = 1;
		}else{
			// looking west
			xmax = Min(ix - 10, NUMSECTORS_X - 1);
			xmin = Min(ix + 2, NUMSECTORS_X - 1);
			inc = -1;
		}
		for(y = ymin; y <= ymax; y++){
			for(x = xmin; x != xmax; x += inc){
				sect = CWorld::GetSector(x, y);
				if(DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}

		if(RemoveReferencedTxds(mem))
			return;

		// As last resort, delete objects from the last step more aggressively
		for(y = ymin; y <= ymax; y++){
			for(x = xmax; x != xmin; x -= inc){
				sect = CWorld::GetSector(x, y);
				if(DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}
	}else{
		// looking north/south

		xmin = Max(ix - 10, 0);
		xmax = Min(ix + 10, NUMSECTORS_X - 1);
		assert(xmin <= xmax);

		// Delete a block of sectors that we know is behind the camera
		if(TheCamera.GetForward().y > 0.0f){
			// looking north
			ymax = Max(iy - 2, 0);
			ymin = Max(iy - 10, 0);
			inc = 1;
		}else{
			// looking south
			ymax = Min(iy + 2, NUMSECTORS_Y - 1);
			ymin = Min(iy + 10, NUMSECTORS_Y - 1);
			inc = -1;
		}
		for(x = xmin; x <= xmax; x++){
			for(y = ymin; y != ymax; y += inc){
				sect = CWorld::GetSector(x, y);
				if(DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}

		// Now a block that intersects with the camera's frustum
		if(TheCamera.GetForward().y > 0.0f){
			// looking north
			ymax = Max(iy + 10, 0);
			ymin = Max(iy - 2, 0);
			inc = 1;
		}else{
			// looking south
			ymax = Min(iy - 10, NUMSECTORS_Y - 1);
			ymin = Min(iy + 2, NUMSECTORS_Y - 1);
			inc = -1;
		}
		for(x = xmin; x <= xmax; x++){
			for(y = ymin; y != ymax; y += inc){
				sect = CWorld::GetSector(x, y);
				if(DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}

		if(RemoveReferencedTxds(mem))
			return;

		// As last resort, delete objects from the last step more aggressively
		for(x = xmin; x <= xmax; x++){
			for(y = ymax; y != ymin; y -= inc){
				sect = CWorld::GetSector(x, y);
				if(DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}
	}
}

void
CStreaming::DeleteRwObjectsInSectorList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(!e->bStreamingDontDelete && !e->bImBeingRendered)
			e->DeleteRwObject();
	}
}

void
CStreaming::DeleteRwObjectsInOverlapSectorList(CPtrList &list, int32 x, int32 y)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(e->m_rwObject && !e->bStreamingDontDelete && !e->bImBeingRendered){
			// Now this is pretty weird...
			if(Abs(CWorld::GetSectorIndexX(e->GetPosition().x) - x) >= 2.0f)
//			{
				e->DeleteRwObject();
//				return;		// BUG?
//			}
			else	// FIX?
			if(Abs(CWorld::GetSectorIndexY(e->GetPosition().y) - y) >= 2.0f)
				e->DeleteRwObject();
		}
	}
}

bool
CStreaming::DeleteRwObjectsBehindCameraInSectorList(CPtrList &list, size_t mem)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(!e->bStreamingDontDelete && !e->bImBeingRendered &&
		   e->m_rwObject && ms_aInfoForModel[e->GetModelIndex()].m_next){
			e->DeleteRwObject();
			if (CModelInfo::GetModelInfo(e->GetModelIndex())->GetNumRefs() == 0) {
				RemoveModel(e->GetModelIndex());
				if(ms_memoryUsed < mem)
					return true;
			}
		}
	}
	return false;
}

bool
CStreaming::DeleteRwObjectsNotInFrustumInSectorList(CPtrList &list, size_t mem)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(!e->bStreamingDontDelete && !e->bImBeingRendered &&
		   e->m_rwObject && !e->IsVisible() && ms_aInfoForModel[e->GetModelIndex()].m_next){
			e->DeleteRwObject();
			if (CModelInfo::GetModelInfo(e->GetModelIndex())->GetNumRefs() == 0) {
				RemoveModel(e->GetModelIndex());
				if(ms_memoryUsed < mem)
					return true;
			}
		}
	}
	return false;
}

void
CStreaming::MakeSpaceFor(int32 size)
{
#ifdef FIX_BUGS
#define MB (1024 * 1024)
	if(ms_memoryAvailable == 0) {
		extern size_t _dwMemAvailPhys;
		ms_memoryAvailable = (_dwMemAvailPhys - 10 * MB) / 2;
		if(ms_memoryAvailable < 50 * MB) ms_memoryAvailable = 50 * MB;
	}
#undef MB
#endif
	while(ms_memoryUsed >= ms_memoryAvailable - size)
		if(!RemoveLeastUsedModel()) {
			DeleteRwObjectsBehindCamera(ms_memoryAvailable - size);
			return;
		}
}

void
CStreaming::LoadScene(const CVector &pos)
{
	CStreamingInfo *si, *prev;
	eLevelName level;

	level = CTheZones::GetLevelFromPosition(&pos);
	debug("Start load scene\n");
	for(si = ms_endRequestedList.m_prev; si != &ms_startRequestedList; si = prev){
		prev = si->m_prev;
		if((si->m_flags & (STREAMFLAGS_KEEP_IN_MEMORY|STREAMFLAGS_PRIORITY)) == 0)
			RemoveModel(si - ms_aInfoForModel);
	}
	CRenderer::m_loadingPriority = false;
	CCullZones::ForceCullZoneCoors(pos);
	DeleteAllRwObjects();
	AddModelsToRequestList(pos);
	CRadar::StreamRadarSections(pos);
	RemoveUnusedBigBuildings(level);
	RequestBigBuildings(level);
	LoadAllRequestedModels(false);
	debug("End load scene\n");
}

void
CStreaming::MemoryCardSave(uint8 *buf, uint32 *size)
{
	int i;

	*size = NUM_DEFAULT_MODELS;
	for(i = 0; i < NUM_DEFAULT_MODELS; i++)
		if(ms_aInfoForModel[i].m_loadState == STREAMSTATE_LOADED)
			buf[i] = ms_aInfoForModel[i].m_flags;
		else
			buf[i] = 0xFF;
}

void 
CStreaming::MemoryCardLoad(uint8 *buf, uint32 size)
{
	uint32 i;

	assert(size == NUM_DEFAULT_MODELS);
	for(i = 0; i < size; i++)
		if(ms_aInfoForModel[i].m_loadState == STREAMSTATE_LOADED)
			if(buf[i] != 0xFF)
				ms_aInfoForModel[i].m_flags = buf[i];
}

void
CStreaming::UpdateForAnimViewer(void)
{
	if (CStreaming::ms_channelError == -1) {
		CStreaming::AddModelsToRequestList(CVector(0.0f, 0.0f, 0.0f));
		CStreaming::LoadRequestedModels();
		sprintf(gString, "Requested %d, memory size %zuK\n", CStreaming::ms_numModelsRequested, 2 * CStreaming::ms_memoryUsed); // original modifier was %d
	}
	else {
		CStreaming::RetryLoadFile(CStreaming::ms_channelError);
	}
}


void
CStreaming::PrintStreamingBufferState()
{
	char str[128];
	wchar wstr[128];
	uint32 offset, size;

	CTimer::Stop();
	int i = 0;
	while (i < NUMSTREAMINFO) {
		while (true) {
			int j = 0;
			DoRWStuffStartOfFrame(50, 50, 50, 0, 0, 0, 255);
			CPad::UpdatePads();
			CSprite2d::InitPerFrame();
			CFont::InitPerFrame();
			DefinedState();

			CRect unusedRect(0, 0, RsGlobal.maximumWidth, RsGlobal.maximumHeight);
			CRGBA unusedColor(255, 255, 255, 255);
			CFont::SetFontStyle(FONT_BANK);
			CFont::SetBackgroundOff();
			CFont::SetWrapx(DEFAULT_SCREEN_WIDTH);
			CFont::SetScale(0.5f, 0.75f);
			CFont::SetCentreOff();
			CFont::SetCentreSize(DEFAULT_SCREEN_WIDTH);
			CFont::SetJustifyOff();
			CFont::SetColor(CRGBA(200, 200, 200, 200));
			CFont::SetBackGroundOnlyTextOff();
			int modelIndex = i;
			if (modelIndex < NUMSTREAMINFO) {
				int y = 24;
				for ( ; j < 34 && modelIndex < NUMSTREAMINFO; modelIndex++) {
					CStreamingInfo *streamingInfo = &ms_aInfoForModel[modelIndex];
					CBaseModelInfo *modelInfo = CModelInfo::GetModelInfo(modelIndex);
					if (streamingInfo->m_loadState != STREAMSTATE_LOADED || !streamingInfo->GetCdPosnAndSize(offset, size))
						continue;

					if (modelIndex >= STREAM_OFFSET_TXD)
						sprintf(str, "txd %s, refs %d, size %dK, flags 0x%x", CTxdStore::GetTxdName(modelIndex - STREAM_OFFSET_TXD),
						        CTxdStore::GetNumRefs(modelIndex - STREAM_OFFSET_TXD), 2 * size, streamingInfo->m_flags);
					else
						sprintf(str, "model %d,%s, refs%d, size%dK, flags%x", modelIndex, modelInfo->GetModelName(), modelInfo->GetNumRefs(), 2 * size,
						        streamingInfo->m_flags);
					AsciiToUnicode(str, wstr);
					CFont::PrintString(24.0f, y, wstr);
					y += 12;
					j++;
				}
			}

			if (CPad::GetPad(1)->GetCrossJustDown())
				i = modelIndex;

			if (!CPad::GetPad(1)->GetTriangleJustDown())
				break;

			i = 0;
			CFont::DrawFonts();
			DoRWStuffEndOfFrame();
		}
		CFont::DrawFonts();
		DoRWStuffEndOfFrame();
	}
	CTimer::Update();
}
