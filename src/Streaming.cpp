#include "common.h"
#include "patcher.h"
#include "ModelInfo.h"
#include "TxdStore.h"
#include "ModelIndices.h"
#include "Pools.h"
#include "Directory.h"
#include "RwHelper.h"
#include "World.h"
#include "Entity.h"
#include "FileMgr.h"
#include "FileLoader.h"
#include "CdStream.h"
#include "Streaming.h"

bool &CStreaming::ms_disableStreaming = *(bool*)0x95CD6E;
bool &CStreaming::ms_bLoadingBigModel = *(bool*)0x95CDB0;
int32 &CStreaming::ms_numModelsRequested = *(int32*)0x8E2C10;
CStreamingInfo *CStreaming::ms_aInfoForModel = (CStreamingInfo*)0x6C7088;
CStreamingInfo &CStreaming::ms_startLoadedList = *(CStreamingInfo*)0x942F60;
CStreamingInfo &CStreaming::ms_endLoadedList = *(CStreamingInfo*)0x8F1AC0;
CStreamingInfo &CStreaming::ms_startRequestedList = *(CStreamingInfo*)0x8F1B3C;
CStreamingInfo &CStreaming::ms_endRequestedList = *(CStreamingInfo*)0x940738;
int32 &CStreaming::ms_oldSectorX = *(int32*)0x8F2C84;
int32 &CStreaming::ms_oldSectorY = *(int32*)0x8F2C88;
int32 &CStreaming::ms_streamingBufferSize = *(int32*)0x942FB0;
int8 **CStreaming::ms_pStreamingBuffer = (int8**)0x87F818;
int32 &CStreaming::ms_memoryUsed = *(int32*)0x940568;
CStreamingChannel *CStreaming::ms_channel = (CStreamingChannel*)0x727EE0;
int32 &CStreaming::ms_channelError = *(int32*)0x880DB8;
int32 &CStreaming::ms_numVehiclesLoaded = *(int32*)0x8F2C80;
int32 *CStreaming::ms_vehiclesLoaded = (int32*)0x773560;
int32 &CStreaming::ms_lastVehicleDeleted = *(int32*)0x95CBF8;
CDirectory *&CStreaming::ms_pExtraObjectsDir = *(CDirectory**)0x95CB90;
int32 &CStreaming::ms_numPriorityRequests = *(int32*)0x8F31C4;
bool &CStreaming::ms_hasLoadedLODs = *(bool*)0x95CD47;
int32 &CStreaming::ms_currentPedGrp = *(int32*)0x8F2BBC;
int32 CStreaming::ms_currentPedLoading;
int32 CStreaming::ms_lastCullZone;
uint16 &CStreaming::ms_loadedGangs = *(uint16*)0x95CC60;
int32 *CStreaming::ms_imageOffsets = (int32*)0x6E60A0;
int32 &CStreaming::ms_lastImageRead = *(int32*)0x880E2C;
int32 &CStreaming::ms_imageSize = *(int32*)0x8F1A34;
int32 &CStreaming::ms_memoryAvailable = *(int32*)0x880F8C;

int32 &desiredNumVehiclesLoaded = *(int32*)0x5EC194;

CEntity *&pIslandLODindustEntity = *(CEntity**)0x6212DC;
CEntity *&pIslandLODcomIndEntity = *(CEntity**)0x6212E0;
CEntity *&pIslandLODcomSubEntity = *(CEntity**)0x6212E4;
CEntity *&pIslandLODsubIndEntity = *(CEntity**)0x6212E8;
CEntity *&pIslandLODsubComEntity = *(CEntity**)0x6212EC;
int32 &islandLODindust = *(int32*)0x6212C8;
int32 &islandLODcomInd = *(int32*)0x6212CC;
int32 &islandLODcomSub = *(int32*)0x6212D0;
int32 &islandLODsubInd = *(int32*)0x6212D4;
int32 &islandLODsubCom = *(int32*)0x6212D8;

WRAPPER void CStreaming::MakeSpaceFor(int32 size) { EAXJMP(0x409B70); }
//WRAPPER bool CStreaming::IsTxdUsedByRequestedModels(int32 txdId) { EAXJMP(0x4094C0); }
//WRAPPER bool CStreaming::AddToLoadedVehiclesList(int32 modelId) { EAXJMP(0x40B060); }


void
CStreaming::Init(void)
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
	ms_pStreamingBuffer[0] = (int8*)RwMallocAlign(ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE, CDSTREAM_SECTOR_SIZE);
	ms_streamingBufferSize /= 2;
	ms_pStreamingBuffer[1] = ms_pStreamingBuffer[0] + ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
	debug("Streaming buffer size is %d sectors", ms_streamingBufferSize);

	// PC only, figure out how much memory we got
#define MB (1024*1024)
	extern DWORD &_dwMemAvailPhys;
	ms_memoryAvailable = (_dwMemAvailPhys - 10*MB)/2;
	if(ms_memoryAvailable < 50*MB)
		ms_memoryAvailable = 50*MB;
	desiredNumVehiclesLoaded = (ms_memoryAvailable/MB - 50)/3 + 12;
	if(desiredNumVehiclesLoaded > MAXVEHICLESLOADED)
		desiredNumVehiclesLoaded = MAXVEHICLESLOADED;
	debug("Memory allocated to Streaming is %dMB", ms_memoryAvailable/MB);
#undef MB

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

	for(i = 0; i < CPools::GetBuildingPool()->GetSize(); i++){
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
CStreaming::Shutdown(void)
{
	RwFreeAlign(ms_pStreamingBuffer[0]);
	ms_streamingBufferSize = 0;
	if(ms_pExtraObjectsDir)
		delete ms_pExtraObjectsDir;
}

void
CStreaming::LoadCdDirectory(void)
{
	char dirname[132];
	int i;

	// PC specific stuff
	ms_imageOffsets[0] = 0;
	for(i = 1; i < NUMCDIMAGES; i++)
		ms_imageOffsets[i] = -1;
	ms_imageSize = GetGTA3ImgSize();

	// PS2 uses CFileMgr::GetCdFile on all IMG files to fill the array


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
		if(dot) *dot = '\0';
		if(direntry.size > (uint32)ms_streamingBufferSize)
			ms_streamingBufferSize = direntry.size;

		if(strcmp(dot+1, "DFF") == 0 || strcmp(dot+1, "dff") == 0){
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
				// BUG: doesn't remember which cdimage this was in
				ms_pExtraObjectsDir->AddItem(direntry);
				lastID = -1;
			}
		}else if(strcmp(dot+1, "TXD") == 0 || strcmp(dot+1, "txd") == 0){
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
		if(CTxdStore::GetSlot(mi->GetTxdSlot())->texDict == nil){
			debug("failed to load %s because TXD %s is not in memory\n", mi->GetName(), CTxdStore::GetTxdName(mi->GetTxdSlot()));
			RemoveModel(streamId);
			RemoveTxd(mi->GetTxdSlot());
			ReRequestModel(streamId);
			RwStreamClose(stream, &mem);
			return false;
		}

		// Set Txd to use
		CTxdStore::AddRef(mi->GetTxdSlot());
		CTxdStore::SetCurrentTxd(mi->GetTxdSlot());

		if(mi->IsSimple()){
			success = CFileLoader::LoadAtomicFile(stream, streamId);
		}else if(mi->m_type == MITYPE_VEHICLE){
			// load vehicles in two parts
			CModelInfo::GetModelInfo(streamId)->AddRef();
			success = CFileLoader::StartLoadClumpFile(stream, streamId);
			if(success)
				ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_STARTED;
		}else{
			success = CFileLoader::LoadClumpFile(stream, streamId);
		}
		UpdateMemoryUsed();

		// Txd no longer needed unless we only read part of the file
		if(ms_aInfoForModel[streamId].m_loadState != STREAMSTATE_STARTED)
			CTxdStore::RemoveRefWithoutDelete(mi->GetTxdSlot());

		if(!success){
			debug("Failed to load %s\n", CModelInfo::GetModelInfo(streamId)->GetName());
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

		if(ms_bLoadingBigModel || cdsize > 200){
			success = CTxdStore::StartLoadTxd(streamId - STREAM_OFFSET_TXD, stream);
			if(success)
				ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_STARTED;
		}else
		        success = CTxdStore::LoadTxd(streamId - STREAM_OFFSET_TXD, stream);
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
			debug("Failed to load %s.dff\n", mi->GetName());
		else
			debug("Failed to load %s.txd\n", CTxdStore::GetTxdName(streamId - STREAM_OFFSET_TXD));
		return false;
	}

	if(streamId < STREAM_OFFSET_TXD){
		// Model
		// Vehicles and Peds not in loaded list
		if(mi->m_type != MITYPE_VEHICLE && mi->m_type != MITYPE_PED){
			CSimpleModelInfo *smi = (CSimpleModelInfo*)mi;

			// Set fading for some objects
			if(mi->IsSimple() && !smi->m_isBigBuilding){
				if(ms_aInfoForModel[streamId].m_flags & STREAMFLAGS_NOFADE)
					smi->m_alpha = 255;
				else
					smi->m_alpha = 0;
			}

			if((ms_aInfoForModel[streamId].m_flags & STREAMFLAGS_NOT_IN_LIST) == 0)
				ms_aInfoForModel[streamId].AddToList(&ms_startLoadedList);
		}
	}else{
		// Txd
		if((ms_aInfoForModel[streamId].m_flags & STREAMFLAGS_NOT_IN_LIST) == 0)
			ms_aInfoForModel[streamId].AddToList(&ms_startLoadedList);
	}

	// Mark objects as loaded
	if(ms_aInfoForModel[streamId].m_loadState != STREAMSTATE_STARTED){
		ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_LOADED;
		ms_memoryUsed += ms_aInfoForModel[streamId].GetCdSize() * CDSTREAM_SECTOR_SIZE;
	}

	endTime = CTimer::GetCurrentTimeInCycles() / CTimer::GetCyclesPerMillisecond();
	timeDiff = endTime - startTime;
	if(timeDiff > 5){
		if(streamId < STREAM_OFFSET_TXD)
			debug("model %s took %d ms\n", CModelInfo::GetModelInfo(streamId)->GetName(), timeDiff);
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
		CTxdStore::SetCurrentTxd(mi->GetTxdSlot());
		success = CFileLoader::FinishLoadClumpFile(stream, streamId);
		if(success)
			success = AddToLoadedVehiclesList(streamId);
		mi->RemoveRef();
		CTxdStore::RemoveRefWithoutDelete(mi->GetTxdSlot());
	}else{
		// Txd
		CTxdStore::AddRef(streamId - STREAM_OFFSET_TXD);
		success = CTxdStore::FinishLoadTxd(streamId - STREAM_OFFSET_TXD, stream);
		CTxdStore::RemoveRefWithoutDelete(streamId - STREAM_OFFSET_TXD);
	}

	RwStreamClose(stream, &mem);
	ms_aInfoForModel[streamId].m_loadState = STREAMSTATE_LOADED;
	ms_memoryUsed += ms_aInfoForModel[streamId].GetCdSize() * CDSTREAM_SECTOR_SIZE;

	if(!success){
		RemoveModel(streamId);
		ReRequestModel(streamId);
		UpdateMemoryUsed();
		return false;
	}

	UpdateMemoryUsed();

	endTime = CTimer::GetCurrentTimeInCycles() / CTimer::GetCyclesPerMillisecond();
	timeDiff = endTime - startTime;
	if(timeDiff > 5){
		if(streamId < STREAM_OFFSET_TXD)
			debug("finish model %s took %d ms\n", CModelInfo::GetModelInfo(streamId)->GetName(), timeDiff);
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
			if((ms_aInfoForModel[id].m_flags & STREAMFLAGS_NOT_IN_LIST) == 0)
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
	}
}

void
CStreaming::RequestBigBuildings(eLevelName level)
{
	int i, n;
	CBuilding *b;

	n = CPools::GetBuildingPool()->GetSize();
	for(i = 0; i < n; i++){
		b = CPools::GetBuildingPool()->GetSlot(i);
		if(b && b->bIsBIGBuilding && b->m_level == level)
			RequestModel(b->GetModelIndex(), STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_PRIORITY);
	}
	RequestIslands(level);
	ms_hasLoadedLODs = false;
}

void
CStreaming::RequestIslands(eLevelName level)
{
	switch(level){
	case LEVEL_INDUSTRIAL:
		RequestModel(islandLODcomInd, STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_PRIORITY);
		RequestModel(islandLODsubInd, STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_PRIORITY);
		break;
	case LEVEL_COMMERCIAL:
		RequestModel(islandLODindust, STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_PRIORITY);
		RequestModel(islandLODsubCom, STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_PRIORITY);
		break;
	case LEVEL_SUBURBAN:
		RequestModel(islandLODindust, STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_PRIORITY);
		RequestModel(islandLODcomSub, STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_PRIORITY);
		break;
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
	if(strcmp(mi->GetName(), modelName) == 0){
		// Already have the correct name, just request it
		RequestModel(modelId, flags);
		return;
	}

	strcpy(oldName, mi->GetName());
	mi->SetName(modelName);

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

	ms_pExtraObjectsDir->FindItem(modelName, pos, size);
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
		ms_memoryUsed -= ms_aInfoForModel[id].GetCdSize()*CDSTREAM_SECTOR_SIZE;
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

	n = CPools::GetBuildingPool()->GetSize();
	for(i = 0; i < n; i++){
		e = CPools::GetBuildingPool()->GetSlot(i);
		if(e && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if(mi->m_refCount == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}

	n = CPools::GetTreadablePool()->GetSize();
	for(i = 0; i < n; i++){
		e = CPools::GetTreadablePool()->GetSlot(i);
		if(e && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if(mi->m_refCount == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}

	n = CPools::GetObjectPool()->GetSize();
	for(i = 0; i < n; i++){
		e = CPools::GetObjectPool()->GetSlot(i);
		if(e && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered && ((CObject*)e)->ObjectCreatedBy == GAME_OBJECT){
				e->DeleteRwObject();
				if(mi->m_refCount == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}

	n = CPools::GetDummyPool()->GetSize();
	for(i = 0; i < n; i++){
		e = CPools::GetDummyPool()->GetSlot(i);
		if(e && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if(mi->m_refCount == 0)
					RemoveModel(e->GetModelIndex());
			}
		}
	}
}

void
CStreaming::RemoveUnusedBigBuildings(eLevelName level)
{
	if(level != LEVEL_INDUSTRIAL)
		RemoveBigBuildings(LEVEL_INDUSTRIAL);
	if(level != LEVEL_COMMERCIAL)
		RemoveBigBuildings(LEVEL_COMMERCIAL);
	if(level != LEVEL_SUBURBAN)
		RemoveBigBuildings(LEVEL_SUBURBAN);
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

	n = CPools::GetBuildingPool()->GetSize();
	for(i = 0; i < n; i++){
		e = CPools::GetBuildingPool()->GetSlot(i);
		if(e && e->bIsBIGBuilding && e->m_level == level){
			mi = CModelInfo::GetModelInfo(e->GetModelIndex());
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if(mi->m_refCount == 0)
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
		   (ms_aInfoForModel[id].m_flags & STREAMFLAGS_NOT_IN_LIST) == 0 &&
		   CModelInfo::GetModelInfo(id)->m_refCount == 0 &&
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
		if(streamId >= STREAM_OFFSET_TXD){
			if(CTxdStore::GetNumRefs(streamId - STREAM_OFFSET_TXD) == 0 &&
			   !IsTxdUsedByRequestedModels(streamId - STREAM_OFFSET_TXD)){
				RemoveModel(streamId);
				return true;
			}
		}else{
			if(CModelInfo::GetModelInfo(streamId)->m_refCount == 0){
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
		   CModelInfo::GetModelInfo(i)->m_refCount == 0){
			RemoveModel(i);
			ms_aInfoForModel[i].m_loadState = STREAMSTATE_NOTLOADED;
		}
	}
}

bool
CStreaming::RemoveReferencedTxds(int32 mem)
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

// TODO: RemoveCurrentZonesModels

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
			   (ms_aInfoForModel[id].m_flags & STREAMFLAGS_NOT_IN_LIST) == 0 &&
			    CModelInfo::GetModelInfo(id)->m_refCount == 0)
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
			// this is more that we wanted actually
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

	n = CPools::GetBuildingPool()->GetSize();
	for(i = 0; i < n; i++){
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
	if((id >= STREAM_OFFSET_TXD || CModelInfo::GetModelInfo(id)->m_type != MITYPE_VEHICLE)
		&& (ms_aInfoForModel[id].m_flags & STREAMFLAGS_SCRIPTOWNED) == 0){

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
	if((id >= STREAM_OFFSET_TXD || CModelInfo::GetModelInfo(id)->m_type != MITYPE_VEHICLE)
		&& (ms_aInfoForModel[id].m_flags & STREAMFLAGS_DONT_REMOVE) == 0){

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


// Find starting offset of the cdimage we next want to read
// Not useful at all on PC...
int32
CStreaming::GetCdImageOffset(int32 lastPosn)
{
	int offset, off;
	int i, img;
	int dist, mindist;

	img = -1;
	mindist = INT_MAX;
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
TxdAvailable(int32 txdId)
{
	CStreamingInfo *si = &CStreaming::ms_aInfoForModel[txdId + STREAM_OFFSET_TXD];
	return si->m_loadState == STREAMSTATE_LOADED || si->m_loadState == STREAMSTATE_READING;
}

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
	posnFirst = UINT_MAX;
	posnNext = UINT_MAX;

	for(si = ms_startRequestedList.m_next; si != &ms_endRequestedList; si = next){
		next = si->m_next;
		streamId = si - ms_aInfoForModel;

		// only priority requests if there are any
		if(priority && ms_numPriorityRequests != 0 && !si->IsPriority())
			continue;

		// request Txd if necessary
		if(streamId < STREAM_OFFSET_TXD &&
		   !TxdAvailable(CModelInfo::GetModelInfo(streamId)->GetTxdSlot())){
			ReRequestTxd(CModelInfo::GetModelInfo(streamId)->GetTxdSlot());
		}else if(ms_aInfoForModel[streamId].GetCdPosnAndSize(posn, size)){
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
 *
 * TODO: two-part files
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
			if(havePed && CModelInfo::GetModelInfo(streamId)->m_type == MITYPE_PED ||
			   haveBigFile && CModelInfo::GetModelInfo(streamId)->m_type == MITYPE_VEHICLE ||
			   !TxdAvailable(CModelInfo::GetModelInfo(streamId)->GetTxdSlot()))
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
			if(CModelInfo::GetModelInfo(streamId)->m_type == MITYPE_PED)
				havePed = 1;
			if(CModelInfo::GetModelInfo(streamId)->m_type == MITYPE_VEHICLE)
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
			if(id < STREAM_OFFSET_TXD &&
			   CModelInfo::GetModelInfo(id)->m_type == MITYPE_VEHICLE &&
			   ms_numVehiclesLoaded >= desiredNumVehiclesLoaded &&
			   !RemoveLoadedVehicle() &&
			   ((ms_aInfoForModel[id].m_flags & STREAMFLAGS_NOT_IN_LIST) == 0 || GetAvailableVehicleSlot() == -1)){
				// can't load vehicle
				RemoveModel(id);
				if(ms_aInfoForModel[id].m_flags & STREAMFLAGS_NOT_IN_LIST)
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
		for(i = 0; i < 4; i++){
			ms_channel[1].streamIds[i] = -1;
			ms_channel[1].offsets[i] = -1;
		}
		ms_channel[1].state = CHANNELSTATE_IDLE;
	}

	return true;
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
	FlushChannels();
}


void
CStreaming::ImGonnaUseStreamingMemory(void)
{
	// empty
}

void
CStreaming::IHaveUsedStreamingMemory(void)
{
	UpdateMemoryUsed();
}

void
CStreaming::UpdateMemoryUsed(void)
{
	// empty
}


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

STARTPATCHES
	InjectHook(0x406430, CStreaming::Init, PATCH_JUMP);
	InjectHook(0x406C80, CStreaming::Shutdown, PATCH_JUMP);
	InjectHook(0x406CC0, (void (*)(void))CStreaming::LoadCdDirectory, PATCH_JUMP);
	InjectHook(0x406DA0, (void (*)(const char*, int))CStreaming::LoadCdDirectory, PATCH_JUMP);
	InjectHook(0x409740, CStreaming::ConvertBufferToObject, PATCH_JUMP);
	InjectHook(0x409580, CStreaming::FinishLoadingLargeFile, PATCH_JUMP);
	InjectHook(0x407EA0, CStreaming::RequestModel, PATCH_JUMP);
	InjectHook(0x407FD0, CStreaming::RequestSubway, PATCH_JUMP);
	InjectHook(0x408190, CStreaming::RequestBigBuildings, PATCH_JUMP);
	InjectHook(0x408210, CStreaming::RequestIslands, PATCH_JUMP);
	InjectHook(0x40A890, CStreaming::RequestSpecialModel, PATCH_JUMP);
	InjectHook(0x40ADA0, CStreaming::RequestSpecialChar, PATCH_JUMP);

	InjectHook(0x408830, CStreaming::RemoveModel, PATCH_JUMP);
	InjectHook(0x4083A0, CStreaming::RemoveUnusedBuildings, PATCH_JUMP);
	InjectHook(0x4083D0, CStreaming::RemoveBuildings, PATCH_JUMP);
	InjectHook(0x408640, CStreaming::RemoveUnusedBigBuildings, PATCH_JUMP);
	InjectHook(0x408680, CStreaming::RemoveBigBuildings, PATCH_JUMP);
	InjectHook(0x408780, CStreaming::RemoveIslandsNotUsed, PATCH_JUMP);
	InjectHook(0x40B180, CStreaming::RemoveLoadedVehicle, PATCH_JUMP);
	InjectHook(0x4089B0, CStreaming::RemoveLeastUsedModel, PATCH_JUMP);
	InjectHook(0x408940, CStreaming::RemoveAllUnusedModels, PATCH_JUMP);
	InjectHook(0x409450, CStreaming::RemoveReferencedTxds, PATCH_JUMP);

	InjectHook(0x40B160, CStreaming::GetAvailableVehicleSlot, PATCH_JUMP);
	InjectHook(0x40B060, CStreaming::AddToLoadedVehiclesList, PATCH_JUMP);
	InjectHook(0x4094C0, CStreaming::IsTxdUsedByRequestedModels, PATCH_JUMP);
	InjectHook(0x407E70, CStreaming::IsObjectInCdImage, PATCH_JUMP);
	InjectHook(0x408280, CStreaming::HaveAllBigBuildingsLoaded, PATCH_JUMP);
	InjectHook(0x40A790, CStreaming::SetModelIsDeletable, PATCH_JUMP);
	InjectHook(0x40A800, CStreaming::SetModelTxdIsDeletable, PATCH_JUMP);
	InjectHook(0x40A820, CStreaming::SetMissionDoesntRequireModel, PATCH_JUMP);

	InjectHook(0x40AA00, CStreaming::LoadInitialPeds, PATCH_JUMP);
	InjectHook(0x40ADF0, CStreaming::LoadInitialVehicles, PATCH_JUMP);

	InjectHook(0x409BE0, CStreaming::ProcessLoadingChannel, PATCH_JUMP);
	InjectHook(0x40A610, CStreaming::FlushChannels, PATCH_JUMP);
	InjectHook(0x40A680, CStreaming::FlushRequestList, PATCH_JUMP);
	InjectHook(0x409FF0, CStreaming::GetCdImageOffset, PATCH_JUMP);
	InjectHook(0x409E50, CStreaming::GetNextFileOnCd, PATCH_JUMP);
	InjectHook(0x40A060, CStreaming::RequestModelStream, PATCH_JUMP);
	InjectHook(0x40A390, CStreaming::LoadRequestedModels, PATCH_JUMP);
	InjectHook(0x40A440, CStreaming::LoadAllRequestedModels, PATCH_JUMP);

	InjectHook(0x4063E0, &CStreamingInfo::GetCdPosnAndSize, PATCH_JUMP);
	InjectHook(0x406410, &CStreamingInfo::SetCdPosnAndSize, PATCH_JUMP);
	InjectHook(0x4063D0, &CStreamingInfo::GetCdSize, PATCH_JUMP);
	InjectHook(0x406380, &CStreamingInfo::AddToList, PATCH_JUMP);
	InjectHook(0x4063A0, &CStreamingInfo::RemoveFromList, PATCH_JUMP);
ENDPATCHES
