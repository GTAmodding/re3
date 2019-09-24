#include "common.h"
#include "main.h"
#include "patcher.h"
#include "Quaternion.h"
#include "ModelInfo.h"
#include "ModelIndices.h"
#include "TempColModels.h"
#include "VisibilityPlugins.h"
#include "FileMgr.h"
#include "HandlingMgr.h"
#include "CarCtrl.h"
#include "PedType.h"
#include "PedStats.h"
#include "AnimManager.h"
#include "Game.h"
#include "RwHelper.h"
#include "NodeName.h"
#include "TxdStore.h"
#include "PathFind.h"
#include "ObjectData.h"
#include "DummyObject.h"
#include "World.h"
#include "Zones.h"
#include "ZoneCull.h"
#include "CdStream.h"
#include "FileLoader.h"

WRAPPER void CFileLoader::ReloadPaths(const char *filename) { EAXJMP(0x476DB0); }

char CFileLoader::ms_line[256];

const char*
GetFilename(const char *filename)
{
	char *s = strrchr((char*)filename, '\\');
	return s ? s+1 : filename;
}

void
LoadingScreenLoadingFile(const char *filename)
{
	sprintf(gString, "Loading %s", GetFilename(filename));
	LoadingScreen("Loading the Game", gString, nil);
}

void
CFileLoader::LoadLevel(const char *filename)
{
	int fd;
	RwTexDictionary *savedTxd;
	eLevelName savedLevel;
	bool objectsLoaded;
	char *line;
	char txdname[64];

	savedTxd = RwTexDictionaryGetCurrent();
	objectsLoaded = false;
	savedLevel = CGame::currLevel;
	if(savedTxd == nil){
		savedTxd = RwTexDictionaryCreate();
		RwTexDictionarySetCurrent(savedTxd);
	}
	fd = CFileMgr::OpenFile(filename, "r");
	assert(fd > 0);

	for(line = LoadLine(fd); line; line = LoadLine(fd)){
		if(*line == '#')
			continue;

		if(strncmp(line, "EXIT", 9) == 0)	// BUG: 9?
			break;

		if(strncmp(line, "IMAGEPATH", 9) == 0){
			RwImageSetPath(line + 10);
		}else if(strncmp(line, "TEXDICTION", 10) == 0){
			strcpy(txdname, line+11);
			LoadingScreenLoadingFile(txdname);
			RwTexDictionary *txd = LoadTexDictionary(txdname);
			AddTexDictionaries(savedTxd, txd);
			RwTexDictionaryDestroy(txd);
		}else if(strncmp(line, "COLFILE", 7) == 0){
			int level;
			sscanf(line+8, "%d", &level);
			CGame::currLevel = (eLevelName)level;
			LoadingScreenLoadingFile(line+10);
			LoadCollisionFile(line+10);
			CGame::currLevel = savedLevel;
		}else if(strncmp(line, "MODELFILE", 9) == 0){
			LoadingScreenLoadingFile(line + 10);
			LoadModelFile(line + 10);
		}else if(strncmp(line, "HIERFILE", 8) == 0){
			LoadingScreenLoadingFile(line + 9);
			LoadClumpFile(line + 9);
		}else if(strncmp(line, "IDE", 3) == 0){
			LoadingScreenLoadingFile(line + 4);
			LoadObjectTypes(line + 4);
		}else if(strncmp(line, "IPL", 3) == 0){
			if(!objectsLoaded){
				// CModelInfo::ConstructMloClumps();
				CObjectData::Initialise("DATA\\OBJECT.DAT");
				objectsLoaded = true;
			}
			LoadingScreenLoadingFile(line + 4);
			LoadScene(line + 4);
		}else if(strncmp(line, "MAPZONE", 7) == 0){
			LoadingScreenLoadingFile(line + 8);
			LoadMapZones(line + 8);
		}else if(strncmp(line, "SPLASH", 6) == 0){
			LoadSplash(GetRandomSplashScreen());
		}else if(strncmp(line, "CDIMAGE", 7) == 0){
			CdStreamAddImage(line + 8);
		}
	}

	CFileMgr::CloseFile(fd);
	RwTexDictionarySetCurrent(savedTxd);
}

void
CFileLoader::LoadCollisionFromDatFile(int currlevel)
{
	int fd;
	char *line;

	fd = CFileMgr::OpenFile(CGame::aDatFile, "r");
	assert(fd > 0);

	for(line = LoadLine(fd); line; line = LoadLine(fd)){
		if(*line == '#')
			continue;

		if(strncmp(line, "COLFILE", 7) == 0){
			int level;
			sscanf(line+8, "%d", &level);
			if(currlevel == level)
				LoadCollisionFile(line+10);
		}
	}

	CFileMgr::CloseFile(fd);
}

char*
CFileLoader::LoadLine(int fd)
{
	int i;
	char *line;

	if(CFileMgr::ReadLine(fd, ms_line, 256) == false)
		return nil;
	for(i = 0; ms_line[i] != '\0'; i++)
		if(ms_line[i] < ' ' || ms_line[i] == ',')
			ms_line[i] = ' ';
	for(line = ms_line; *line <= ' ' && *line != '\0'; line++);
	return line;
}

RwTexDictionary*
CFileLoader::LoadTexDictionary(const char *filename)
{
	RwTexDictionary *txd;
	RwStream *stream;

	txd = nil;
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	debug("Loading texture dictionary file %s\n", filename);
	if(stream){
		if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil))
			txd = RwTexDictionaryGtaStreamRead(stream);
		RwStreamClose(stream, nil);
	}
	if(txd == nil)
		txd = RwTexDictionaryCreate();
	return txd;
}

void
CFileLoader::LoadCollisionFile(const char *filename)
{
	int fd;
	char modelname[24];
	CBaseModelInfo *mi;
	struct {
		char ident[4];
		uint32 size;
	} header;

	debug("Loading collision file %s\n", filename);
	fd = CFileMgr::OpenFile(filename, "rb");

	while(CFileMgr::Read(fd, (char*)&header, sizeof(header))){
		assert(strncmp(header.ident, "COLL", 4) == 0);
		CFileMgr::Read(fd, (char*)work_buff, header.size);
		memcpy(modelname, work_buff, 24);

		mi = CModelInfo::GetModelInfo(modelname, nil);
		if(mi){
			if(mi->GetColModel()){
				LoadCollisionModel(work_buff+24, *mi->GetColModel(), modelname);
			}else{
				CColModel *model = new CColModel;
				LoadCollisionModel(work_buff+24, *model, modelname);
				mi->SetColModel(model, true);
			}
		}else{
			debug("colmodel %s can't find a modelinfo\n", modelname);
		}
	}

	CFileMgr::CloseFile(fd);
}

void
CFileLoader::LoadCollisionModel(uint8 *buf, CColModel &model, char *modelname)
{
	int i;

	model.boundingSphere.radius = *(float*)(buf);
	model.boundingSphere.center.x = *(float*)(buf+4);
	model.boundingSphere.center.y = *(float*)(buf+8);
	model.boundingSphere.center.z = *(float*)(buf+12);
	model.boundingBox.min.x = *(float*)(buf+16);
	model.boundingBox.min.y = *(float*)(buf+20);
	model.boundingBox.min.z = *(float*)(buf+24);
	model.boundingBox.max.x = *(float*)(buf+28);
	model.boundingBox.max.y = *(float*)(buf+32);
	model.boundingBox.max.z = *(float*)(buf+36);
	model.numSpheres = *(int16*)(buf+40);
	buf += 44;
	if(model.numSpheres > 0){
		model.spheres = (CColSphere*)RwMalloc(model.numSpheres*sizeof(CColSphere));
		for(i = 0; i < model.numSpheres; i++){
			model.spheres[i].Set(*(float*)buf, *(CVector*)(buf+4), buf[16], buf[17]);
			buf += 20;
		}
	}else
		model.spheres = nil;

	model.numLines = *(int16*)buf;
	buf += 4;
	if(model.numLines > 0){
		model.lines = (CColLine*)RwMalloc(model.numLines*sizeof(CColLine));
		for(i = 0; i < model.numLines; i++){
			model.lines[i].Set(*(CVector*)buf, *(CVector*)(buf+12));
			buf += 24;
		}
	}else
		model.lines = nil;

	model.numBoxes = *(int16*)buf;
	buf += 4;
	if(model.numBoxes > 0){
		model.boxes = (CColBox*)RwMalloc(model.numBoxes*sizeof(CColBox));
		for(i = 0; i < model.numBoxes; i++){
			model.boxes[i].Set(*(CVector*)buf, *(CVector*)(buf+12), buf[24], buf[25]);
			buf += 28;
		}
	}else
		model.boxes = nil;

	int32 numVertices = *(int16*)buf;
	buf += 4;
	if(numVertices > 0){
		model.vertices = (CVector*)RwMalloc(numVertices*sizeof(CVector));
		for(i = 0; i < numVertices; i++){
			model.vertices[i] = *(CVector*)buf;
			if(Abs(model.vertices[i].x) >= 256.0f ||
			   Abs(model.vertices[i].y) >= 256.0f ||
			   Abs(model.vertices[i].z) >= 256.0f)
				printf("%s:Collision volume too big\n", modelname);
			buf += 12;
		}
	}else
		model.vertices = nil;

	model.numTriangles = *(int16*)buf;
	buf += 4;
	if(model.numTriangles > 0){
		model.triangles = (CColTriangle*)RwMalloc(model.numTriangles*sizeof(CColTriangle));
		for(i = 0; i < model.numTriangles; i++){
			model.triangles[i].Set(model.vertices, *(int32*)buf, *(int32*)(buf+4), *(int32*)(buf+8), buf[12], buf[13]);
			buf += 16;
		}
	}else
		model.triangles = nil;
}

static void
GetNameAndLOD(char *nodename, char *name, int *n)
{
	char *underscore = nil;
	for(char *s = nodename; *s != '\0'; s++){
		if(s[0] == '_' && (s[1] == 'l' || s[1] == 'L'))
			underscore = s;
	}
	if(underscore){
		strncpy(name, nodename, underscore - nodename);
		name[underscore - nodename] = '\0';
		*n = atoi(underscore + 2);
	}else{
		strncpy(name, nodename, 24);
		*n = 0;
	}
}

RpAtomic*
CFileLoader::FindRelatedModelInfoCB(RpAtomic *atomic, void *data)
{
	CSimpleModelInfo *mi;
	char *nodename, name[24];
	int n;
	RpClump *clump = (RpClump*)data;

	nodename = GetFrameNodeName(RpClumpGetFrame(atomic));
	GetNameAndLOD(nodename, name, &n);
	mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(name, nil);
	if(mi){
		assert(mi->IsSimple());
		mi->SetAtomic(n, atomic);
		RpClumpRemoveAtomic(clump, atomic);
		RpAtomicSetFrame(atomic, RwFrameCreate());
		CVisibilityPlugins::SetAtomicModelInfo(atomic, mi);
		CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
	}else{
		debug("Can't find Atomic %s\n", name);
	}

	return atomic;
}

void
CFileLoader::LoadModelFile(const char *filename)
{
	RwStream *stream;
	RpClump *clump;

	debug("Loading model file %s\n", filename);
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	if(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		clump = RpClumpStreamRead(stream);
		if(clump){
			RpClumpForAllAtomics(clump, FindRelatedModelInfoCB, clump);
			RpClumpDestroy(clump);
		}
	}
	RwStreamClose(stream, nil);
}

void
CFileLoader::LoadClumpFile(const char *filename)
{
	RwStream *stream;
	RpClump *clump;
	char *nodename, name[24];
	int n;
	CClumpModelInfo *mi;

	debug("Loading model file %s\n", filename);
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	while(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		clump = RpClumpStreamRead(stream);
		if(clump){
			nodename = GetFrameNodeName(RpClumpGetFrame(clump));
			GetNameAndLOD(nodename, name, &n);
			mi = (CClumpModelInfo*)CModelInfo::GetModelInfo(name, nil);
			if(mi){
				assert(mi->IsClump());
				mi->SetClump(clump);
			}else
				RpClumpDestroy(clump);
		}
	}
	RwStreamClose(stream, nil);
}

bool
CFileLoader::LoadClumpFile(RwStream *stream, uint32 id)
{
	RpClump *clump;
	CClumpModelInfo *mi;

	if(!RwStreamFindChunk(stream, rwID_CLUMP, nil, nil))
		return false;
	clump = RpClumpStreamRead(stream);
	if(clump == nil)
		return false;
	mi = (CClumpModelInfo*)CModelInfo::GetModelInfo(id);
	mi->SetClump(clump);
	if(mi->m_type == MITYPE_PED && id != 0 && RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		// Read LOD ped
		clump = RpClumpStreamRead(stream);
		if(clump){
			((CPedModelInfo*)mi)->SetLowDetailClump(clump);
			RpClumpDestroy(clump);
		}
	}
	return true;
}

bool
CFileLoader::StartLoadClumpFile(RwStream *stream, uint32 id)
{
	if(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		printf("Start loading %s\n", CModelInfo::GetModelInfo(id)->GetName());
		return RpClumpGtaStreamRead1(stream);
	}else{
		printf("FAILED\n");
		return false;
	}
}

bool
CFileLoader::FinishLoadClumpFile(RwStream *stream, uint32 id)
{
	RpClump *clump;
	CClumpModelInfo *mi;

	printf("Finish loading %s\n", CModelInfo::GetModelInfo(id)->GetName());
	clump = RpClumpGtaStreamRead2(stream);

	if(clump){
		mi = (CClumpModelInfo*)CModelInfo::GetModelInfo(id);
		mi->SetClump(clump);
		return true;
	}else{
		printf("FAILED\n");
		return false;
	}
}

CSimpleModelInfo *gpRelatedModelInfo;

bool
CFileLoader::LoadAtomicFile(RwStream *stream, uint32 id)
{
	RpClump *clump;

	if(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		clump = RpClumpStreamRead(stream);
		if(clump == nil)
			return false;
		gpRelatedModelInfo = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
		RpClumpForAllAtomics(clump, SetRelatedModelInfoCB, clump);
		RpClumpDestroy(clump);
	}
	return true;
}

RpAtomic*
CFileLoader::SetRelatedModelInfoCB(RpAtomic *atomic, void *data)
{
	char *nodename, name[24];
	int n;
	RpClump *clump = (RpClump*)data;

	nodename = GetFrameNodeName(RpAtomicGetFrame(atomic));
	GetNameAndLOD(nodename, name, &n);
	gpRelatedModelInfo->SetAtomic(n, atomic);
	RpClumpRemoveAtomic(clump, atomic);
	RpAtomicSetFrame(atomic, RwFrameCreate());
	CVisibilityPlugins::SetAtomicModelInfo(atomic, gpRelatedModelInfo);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
	return atomic;
}

RpClump*
CFileLoader::LoadAtomicFile2Return(const char *filename)
{
	RwStream *stream;
	RpClump *clump;

	clump = nil;
	debug("Loading model file %s\n", filename);
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	if(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil))
		clump = RpClumpStreamRead(stream);
	RwStreamClose(stream, nil);
	return clump;
}

static RwTexture*
MoveTexturesCB(RwTexture *texture, void *pData)
{
	RwTexDictionaryAddTexture((RwTexDictionary*)pData, texture);
	return texture;
}

void
CFileLoader::AddTexDictionaries(RwTexDictionary *dst, RwTexDictionary *src)
{
	RwTexDictionaryForAllTextures(src, MoveTexturesCB, dst);
}

void
CFileLoader::LoadObjectTypes(const char *filename)
{
	enum {
		NONE,
		OBJS,
		MLO,
		TOBJ,
		HIER,
		CARS,
		PEDS,
		PATH,
		TWODFX
	};
	char *line;
	int fd;
	int section;
	int pathIndex;
	char pathTypeStr[20];
	int id, pathType;
//	int mlo;

	section = NONE;
	pathIndex = -1;
//	mlo = 0;
	debug("Loading object types from %s...\n", filename);

	fd = CFileMgr::OpenFile(filename, "rb");
	for(line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)){
		if(*line == '\0' || *line == '#')
			continue;

		if(section == NONE){
			if(strncmp(line, "objs", 4) == 0) section = OBJS;
			else if(strncmp(line, "tobj", 4) == 0) section = TOBJ;
			else if(strncmp(line, "hier", 4) == 0) section = HIER;
			else if(strncmp(line, "cars", 4) == 0) section = CARS;
			else if(strncmp(line, "peds", 4) == 0) section = PEDS;
			else if(strncmp(line, "path", 4) == 0) section = PATH;
			else if(strncmp(line, "2dfx", 4) == 0) section = TWODFX;
		}else if(strncmp(line, "end", 3) == 0){
			section = section == MLO ? OBJS : NONE;
		}else switch(section){
		case OBJS:
			if(strncmp(line, "sta", 3) == 0)
				assert(0);	// LoadMLO
			else
				LoadObject(line);
			break;
		case MLO:
			assert(0);	// LoadMLOInstance
			break;
		case TOBJ:
			LoadTimeObject(line);
			break;
		case HIER:
			LoadClumpObject(line);
			break;
		case CARS:
			LoadVehicleObject(line);
			break;
		case PEDS:
			LoadPedObject(line);
			break;
		case PATH:
			if(pathIndex == -1){
				id = LoadPathHeader(line, pathTypeStr);
				if(strncmp(pathTypeStr, "ped", 4) == 0)
					pathType = 1;
				else if(strncmp(pathTypeStr, "car", 4) == 0)
					pathType = 0;
				pathIndex = 0;
			}else{
				if(pathType == 1)
					LoadPedPathNode(line, id, pathIndex);
				else if(pathType == 0)
					LoadCarPathNode(line, id, pathIndex);
				pathIndex++;
				if(pathIndex == 12)
					pathIndex = -1;
			}
			break;
		case TWODFX:
			Load2dEffect(line);
			break;
		}
	}
	CFileMgr::CloseFile(fd);

	for(id = 0; id < MODELINFOSIZE; id++){
		CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
		if(mi && mi->IsSimple())
			mi->SetupBigBuilding();
	}
}

void
SetModelInfoFlags(CSimpleModelInfo *mi, uint32 flags)
{
	mi->m_normalCull =	!!(flags & 1);
	mi->m_noFade =	!!(flags & 2);
	mi->m_drawLast =	!!(flags & (4|8));
	mi->m_additive =	!!(flags & 8);
	mi->m_isSubway =	!!(flags & 0x10);
	mi->m_ignoreLight =	!!(flags & 0x20);
	mi->m_noZwrite =	!!(flags & 0x40);
}

void
CFileLoader::LoadObject(const char *line)
{
	int id, numObjs;
	char model[24], txd[24];
	float dist[3];
	uint32 flags;
	int damaged;
	CSimpleModelInfo *mi;

	if(sscanf(line, "%d %s %s %d", &id, model, txd, &numObjs) != 4)
		return;

	switch(numObjs){
	case 1:
		sscanf(line, "%d %s %s %d %f %d",
			&id, model, txd, &numObjs, &dist[0], &flags);
		damaged = 0;
		break;
	case 2:
		sscanf(line, "%d %s %s %d %f %f %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &flags);
		damaged = dist[0] < dist[1] ?	// Are distances increasing?
			0 :	// Yes, no damage model
			1;	// No, 1 is damaged
		break;
	case 3:
		sscanf(line, "%d %s %s %d %f %f %f %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &dist[2], &flags);
		damaged = dist[0] < dist[1] ?	// Are distances increasing?
				(dist[1] < dist[2] ? 0 : 2) :	// Yes, only 2 can still be a damage model
			1;	// No, 1 and 2 are damaged
		break;
	}

	mi = CModelInfo::AddSimpleModel(id);
	mi->SetName(model);
	mi->SetNumAtomics(numObjs);
	mi->SetLodDistances(dist);
	SetModelInfoFlags(mi, flags);
	mi->m_firstDamaged = damaged;
	mi->SetTexDictionary(txd);
	MatchModelString(model, id);
}

void
CFileLoader::LoadTimeObject(const char *line)
{
	int id, numObjs;
	char model[24], txd[24];
	float dist[3];
	uint32 flags;
	int timeOn, timeOff;
	int damaged;
	CTimeModelInfo *mi, *other;

	if(sscanf(line, "%d %s %s %d", &id, model, txd, &numObjs) != 4)
		return;

	switch(numObjs){
	case 1:
		sscanf(line, "%d %s %s %d %f %d %d %d",
			&id, model, txd, &numObjs, &dist[0], &flags, &timeOn, &timeOff);
		damaged = 0;
		break;
	case 2:
		sscanf(line, "%d %s %s %d %f %f %d %d %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &flags, &timeOn, &timeOff);
		damaged = dist[0] < dist[1] ?	// Are distances increasing?
			0 :	// Yes, no damage model
			1;	// No, 1 is damaged
		break;
	case 3:
		sscanf(line, "%d %s %s %d %f %f %f %d %d %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &dist[2], &flags, &timeOn, &timeOff);
		damaged = dist[0] < dist[1] ?	// Are distances increasing?
				(dist[1] < dist[2] ? 0 : 2) :	// Yes, only 2 can still be a damage model
			1;	// No, 1 and 2 are damaged
		break;
	}

	mi = CModelInfo::AddTimeModel(id);
	mi->SetName(model);
	mi->SetNumAtomics(numObjs);
	mi->SetLodDistances(dist);
	SetModelInfoFlags(mi, flags);
	mi->m_firstDamaged = damaged;
	mi->SetTimes(timeOn, timeOff);
	mi->SetTexDictionary(txd);
	other = mi->FindOtherTimeModel();
	if(other)
		other->SetOtherTimeModel(id);
	MatchModelString(model, id);
}

void
CFileLoader::LoadClumpObject(const char *line)
{
	int id;
	char model[24], txd[24];
	CClumpModelInfo *mi;

	if(sscanf(line, "%d %s %s", &id, &model, &txd) == 3){
		mi = CModelInfo::AddClumpModel(id);
		mi->SetName(model);
		mi->SetTexDictionary(txd);
		mi->SetColModel(&CTempColModels::ms_colModelBBox);
	}
}

void
CFileLoader::LoadVehicleObject(const char *line)
{
	int id;
	char model[24], txd[24];
	char type[8], handlingId[16], gamename[32], vehclass[12];
	uint32 frequency, comprules;
	int32 level, misc;
	float wheelScale;
	CVehicleModelInfo *mi;
	char *p;

	sscanf(line, "%d %s %s %s %s %s %s %d %d %x %d %f",
		&id, model, txd,
		type, handlingId, gamename, vehclass,
		&frequency, &level, &comprules, &misc, &wheelScale);

	mi = CModelInfo::AddVehicleModel(id);
	mi->SetName(model);
	mi->SetTexDictionary(txd);
	for(p = gamename; *p; p++)
		if(*p == '_') *p = ' ';
	strncpy(mi->m_gameName, gamename, 32);
	mi->m_level = level;
	mi->m_compRules = comprules;

	if(strncmp(type, "car", 4) == 0){
		mi->m_wheelId = misc;
		mi->m_wheelScale = wheelScale;
		mi->m_vehicleType = VEHICLE_TYPE_AUTOMOBILE;
	}else if(strncmp(type, "boat", 5) == 0){
		mi->m_vehicleType = VEHICLE_TYPE_BOAT;
	}else if(strncmp(type, "train", 6) == 0){
		mi->m_vehicleType = VEHICLE_TYPE_TRAIN;
	}else if(strncmp(type, "heli", 5) == 0){
		mi->m_vehicleType = VEHICLE_TYPE_HELI;
	}else if(strncmp(type, "plane", 6) == 0){
		mi->m_planeLodId = misc;
		mi->m_wheelScale = 1.0f;
		mi->m_vehicleType = VEHICLE_TYPE_PLANE;
	}else if(strncmp(type, "bike", 5) == 0){
		mi->m_bikeSteerAngle = misc;
		mi->m_wheelScale = wheelScale;
		mi->m_vehicleType = VEHICLE_TYPE_BIKE;
	}else
		assert(0);

	mi->m_handlingId = mod_HandlingManager.GetHandlingId(handlingId);

	// Well this is kinda dumb....
	if(strncmp(vehclass, "poorfamily", 11) == 0){
		mi->m_vehicleClass = VEHICLE_CLASS_POOR;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, VEHICLE_CLASS_POOR);
	}else if(strncmp(vehclass, "richfamily", 11) == 0){
		mi->m_vehicleClass = VEHICLE_CLASS_RICH;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, VEHICLE_CLASS_RICH);
	}else if(strncmp(vehclass, "executive", 10) == 0){
		mi->m_vehicleClass = VEHICLE_CLASS_EXECUTIVE;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, VEHICLE_CLASS_EXECUTIVE);
	}else if(strncmp(vehclass, "worker", 7) == 0){
		mi->m_vehicleClass = VEHICLE_CLASS_WORKER;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, VEHICLE_CLASS_WORKER);
	}else if(strncmp(vehclass, "special", 8) == 0){
		mi->m_vehicleClass = VEHICLE_CLASS_SPECIAL;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, VEHICLE_CLASS_SPECIAL);
	}else if(strncmp(vehclass, "big", 4) == 0){
		mi->m_vehicleClass = VEHICLE_CLASS_BIG;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, VEHICLE_CLASS_BIG);
	}else if(strncmp(vehclass, "taxi", 5) == 0){
		mi->m_vehicleClass = VEHICLE_CLASS_TAXI;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, VEHICLE_CLASS_TAXI);
	}
}

void
CFileLoader::LoadPedObject(const char *line)
{
	int id;
	char model[24], txd[24];
	char pedType[24], pedStats[24], animGroup[24];
	int carsCanDrive;
	CPedModelInfo *mi;
	int animGroupId;

	if(sscanf(line, "%d %s %s %s %s %s %x",
	          &id, model, txd,
	          pedType, pedStats, animGroup, &carsCanDrive) != 7)
		return;

	mi = CModelInfo::AddPedModel(id);
	mi->SetName(model);
	mi->SetTexDictionary(txd);
	mi->SetColModel(&CTempColModels::ms_colModelPed1);
	mi->m_pedType = CPedType::FindPedType(pedType);
	mi->m_pedStatType = CPedStats::GetPedStatType(pedStats);
	for(animGroupId = 0; animGroupId < NUM_ANIM_ASSOC_GROUPS; animGroupId++)
		if(strcmp(animGroup, CAnimManager::GetAnimGroupName((AssocGroupId)animGroupId)) == 0)
			break;
	mi->m_animGroup = animGroupId;
	mi->m_carsCanDrive = carsCanDrive;

	// ???
	CModelInfo::GetModelInfo(MI_LOPOLYGUY)->SetColModel(&CTempColModels::ms_colModelPed1);
}

int
CFileLoader::LoadPathHeader(const char *line, char *type)
{
	int id;
	char modelname[32];

	sscanf(line, "%s %d %s", type, &id, modelname);
	return id;
}

void
CFileLoader::LoadPedPathNode(const char *line, int id, int node)
{
	int type, next, cross;
	float x, y, z, width;

	sscanf(line, "%d %d %d %f %f %f %f", &type, &next, &cross, &x, &y, &z, &width);
	ThePaths.StoreNodeInfoPed(id, node, type, next, x, y, z, 0, !!cross);
}

void
CFileLoader::LoadCarPathNode(const char *line, int id, int node)
{
	int type, next, cross, numLeft, numRight;
	float x, y, z, width;

	sscanf(line, "%d %d %d %f %f %f %f %d %d", &type, &next, &cross, &x, &y, &z, &width, &numLeft, &numRight);
	ThePaths.StoreNodeInfoCar(id, node, type, next, x, y, z, 0, numLeft, numRight);
}


void
CFileLoader::Load2dEffect(const char *line)
{
	int id, r, g, b, a, type;
	float x, y, z;
	char corona[32], shadow[32];
	int shadowIntens, lightType, roadReflection, flare, flags, probability;
	CBaseModelInfo *mi;
	C2dEffect *effect;
	char *p;

	sscanf(line, "%d %f %f %f %d %d %d %d %d", &id, &x, &y, &z, &r, &g, &b, &a, &type);

	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));

	mi = CModelInfo::GetModelInfo(id);
	effect = CModelInfo::Get2dEffectStore().alloc();
	mi->Add2dEffect(effect);
	effect->pos = CVector(x, y, z);
	effect->col = CRGBA(r, g, b, a);
	effect->type = type;

	switch(effect->type){
	case EFFECT_LIGHT:
		while(*line++ != '"');
		p = corona;
		while(*line != '"') *p++ = *line++;
		*p = '\0';
		line++;

		while(*line++ != '"');
		p = shadow;
		while(*line != '"') *p++ = *line++;
		*p = '\0';
		line++;

		sscanf(line, "%f %f %f %f %d %d %d %d %d",
			&effect->light.dist,
			&effect->light.range,
			&effect->light.size,
			&effect->light.shadowRange,
			&shadowIntens, &lightType, &roadReflection, &flare, &flags);
		effect->light.corona = RwTextureRead(corona, nil);
		effect->light.shadow = RwTextureRead(shadow, nil);
		effect->light.shadowIntensity = shadowIntens;
		effect->light.lightType = lightType;
		effect->light.roadReflection = roadReflection;
		effect->light.flareType = flare;
		// TODO: check out the flags
		if(flags & 4)
			flags &= ~2;
		effect->light.flags = flags;
		break;

	case EFFECT_PARTICLE:
		sscanf(line, "%d %f %f %f %d %d %d %d %d %d %f %f %f %f",
			&id, &x, &y, &z, &r, &g, &b, &a, &type,
			&effect->particle.particleType,
			&effect->particle.dir.x,
			&effect->particle.dir.y,
			&effect->particle.dir.z,
			&effect->particle.scale);
		break;

	case EFFECT_ATTRACTOR:
		sscanf(line, "%d %f %f %f %d %d %d %d %d %d %f %f %f %d",
			&id, &x, &y, &z, &r, &g, &b, &a, &type,
			&flags,
			&effect->attractor.dir.x,
			&effect->attractor.dir.y,
			&effect->attractor.dir.z,
			&probability);
		effect->attractor.flags = flags;
		effect->attractor.probability = probability;
		break;
	}

	CTxdStore::PopCurrentTxd();
}

void
CFileLoader::LoadScene(const char *filename)
{
	enum {
		NONE,
		INST,
		ZONE,
		CULL,
		PICK,
		PATH,
	};
	char *line;
	int fd;
	int section;
	int pathIndex;
	char pathTypeStr[20];

	section = NONE;
	pathIndex = -1;
	debug("Creating objects from %s...\n", filename);

	fd = CFileMgr::OpenFile(filename, "rb");
	for(line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)){
		if(*line == '\0' || *line == '#')
			continue;

		if(section == NONE){
			if(strncmp(line, "inst", 4) == 0) section = INST;
			else if(strncmp(line, "zone", 4) == 0) section = ZONE;
			else if(strncmp(line, "cull", 4) == 0) section = CULL;
			else if(strncmp(line, "pick", 4) == 0) section = PICK;
			else if(strncmp(line, "path", 4) == 0) section = PATH;
		}else if(strncmp(line, "end", 3) == 0){
			section = NONE;
		}else switch(section){
		case INST:
			LoadObjectInstance(line);
			break;
		case ZONE:
			LoadZone(line);
			break;
		case CULL:
			LoadCullZone(line);
			break;
		case PICK:
			// unused
			LoadPickup(line);
			break;
		case PATH:
			// unfinished in the game
			if(pathIndex == -1){
				LoadPathHeader(line, pathTypeStr);
				// type not set
				pathIndex = 0;
			}else{
				// nodes not loaded
				pathIndex++;
				if(pathIndex == 12)
					pathIndex = -1;
			}
			break;
		}
	}
	CFileMgr::CloseFile(fd);

	debug("Finished loading IPL\n");
}

void
CFileLoader::LoadObjectInstance(const char *line)
{
	int id;
	char name[24];
	RwV3d trans, scale, axis;
	float angle;
	CSimpleModelInfo *mi;
	RwMatrix *xform;
	CEntity *entity;

	if(sscanf(line, "%d %s %f %f %f %f %f %f %f %f %f %f",
	          &id, name,
	          &trans.x, &trans.y, &trans.z,
	          &scale.x, &scale.y, &scale.z,
	          &axis.x, &axis.y, &axis.z, &angle) != 12)
		return;

	mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
	if(mi == nil)
		return;
	assert(mi->IsSimple());

	angle = -RADTODEG(2.0f * acosf(angle));
	xform = RwMatrixCreate();
	RwMatrixRotate(xform, &axis, angle, rwCOMBINEREPLACE);
	RwMatrixTranslate(xform, &trans, rwCOMBINEPOSTCONCAT);

	if(mi->GetObjectID() == -1){
		if(ThePaths.IsPathObject(id)){
			entity = new CTreadable;
			ThePaths.RegisterMapObject((CTreadable*)entity);
		}else
			entity = new CBuilding;
		entity->SetModelIndexNoCreate(id);
		entity->GetMatrix() = CMatrix(xform);
		entity->m_level = CTheZones::GetLevelFromPosition(entity->GetPosition());
		if(mi->IsSimple()){
			if(mi->m_isBigBuilding)
				entity->SetupBigBuilding();
			if(mi->m_isSubway)
				entity->bIsSubway = true;
		}
		if(mi->GetLargestLodDistance() < 2.0f)
			entity->bIsVisible = false;
		CWorld::Add(entity);
	}else{
		entity = new CDummyObject;
		entity->SetModelIndexNoCreate(id);
		entity->GetMatrix() = CMatrix(xform);
		CWorld::Add(entity);
		if(IsGlass(entity->GetModelIndex()))
			entity->bIsVisible = false;
		entity->m_level = CTheZones::GetLevelFromPosition(entity->GetPosition());
	}

	RwMatrixDestroy(xform);
}

void
CFileLoader::LoadZone(const char *line)
{
	char name[24];
	int type, level;
	float minx, miny, minz;
	float maxx, maxy, maxz;

	if(sscanf(line, "%s %d %f %f %f %f %f %f %d", name, &type, &minx, &miny, &minz, &maxx, &maxy, &maxz, &level) == 9)
		CTheZones::CreateZone(name, (eZoneType)type, minx, miny, minz, maxx, maxy, maxz, (eLevelName)level);
}

void
CFileLoader::LoadCullZone(const char *line)
{
	CVector pos;
	float minx, miny, minz;
	float maxx, maxy, maxz;
	int flags;
	int wantedLevelDrop = 0;

	sscanf(line, "%f %f %f %f %f %f %f %f %f %d %d",
		&pos.x, &pos.y, &pos.z,
		&minx, &miny, &minz,
		&maxx, &maxy, &maxz,
		&flags, &wantedLevelDrop);
	CCullZones::AddCullZone(pos, minx, maxx, miny, maxy, minz, maxz, flags, wantedLevelDrop);
}

// unused
void
CFileLoader::LoadPickup(const char *line)
{
	int id;
	float x, y, z;

	sscanf(line, "%d %f %f %f", &id, &x, &y, &z);
}

void
CFileLoader::LoadMapZones(const char *filename)
{
	enum {
		NONE,
		INST,
		ZONE,
		CULL,
		PICK,
		PATH,
	};
	char *line;
	int fd;
	int section;

	section = NONE;
	debug("Creating zones from %s...\n", filename);

	fd = CFileMgr::OpenFile(filename, "rb");
	for(line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)){
		if(*line == '\0' || *line == '#')
			continue;

		if(section == NONE){
			if(strncmp(line, "zone", 4) == 0) section = ZONE;
		}else if(strncmp(line, "end", 3) == 0){
			section = NONE;
		}else switch(section){
		case ZONE: {
			char name[24];
			int type, level;
			float minx, miny, minz;
			float maxx, maxy, maxz;
			if(sscanf(line, "%s %d %f %f %f %f %f %f %d",
			          &name, &type,
			          &minx, &miny, &minz,
			          &maxx, &maxy, &maxz,
			          &level) == 9)
				CTheZones::CreateMapZone(name, (eZoneType)type, minx, miny, minz, maxx, maxy, maxz, (eLevelName)level);
			}
			break;
		}
	}
	CFileMgr::CloseFile(fd);

	debug("Finished loading IPL\n");
}


STARTPATCHES
	InjectHook(0x476290, CFileLoader::LoadLevel, PATCH_JUMP);

	InjectHook(0x476520, CFileLoader::LoadCollisionFromDatFile, PATCH_JUMP);
	InjectHook(0x4761D0, CFileLoader::LoadLine, PATCH_JUMP);
	InjectHook(0x4765B0, CFileLoader::LoadTexDictionary, PATCH_JUMP);
	InjectHook(0x478B20, CFileLoader::LoadCollisionFile, PATCH_JUMP);
	InjectHook(0x478C20, CFileLoader::LoadCollisionModel, PATCH_JUMP);
	InjectHook(0x476750, CFileLoader::LoadModelFile, PATCH_JUMP);
	InjectHook(0x476810, (void (*)(const char*))CFileLoader::LoadClumpFile, PATCH_JUMP);
	InjectHook(0x476990, (bool (*)(RwStream*,uint32))CFileLoader::LoadClumpFile, PATCH_JUMP);
	InjectHook(0x476A20, CFileLoader::StartLoadClumpFile, PATCH_JUMP);
	InjectHook(0x476A70, CFileLoader::FinishLoadClumpFile, PATCH_JUMP);
	InjectHook(0x476930, CFileLoader::LoadAtomicFile, PATCH_JUMP);
	InjectHook(0x4767C0, CFileLoader::LoadAtomicFile2Return, PATCH_JUMP);
	InjectHook(0x476630, CFileLoader::AddTexDictionaries, PATCH_JUMP);

	InjectHook(0x476AC0, CFileLoader::LoadObjectTypes, PATCH_JUMP);
	InjectHook(0x477040, CFileLoader::LoadObject, PATCH_JUMP);
	InjectHook(0x4774B0, CFileLoader::LoadTimeObject, PATCH_JUMP);
	InjectHook(0x477920, CFileLoader::LoadClumpObject, PATCH_JUMP);
	InjectHook(0x477990, CFileLoader::LoadVehicleObject, PATCH_JUMP);
	InjectHook(0x477DE0, CFileLoader::LoadPedObject, PATCH_JUMP);
	InjectHook(0x477ED0, CFileLoader::LoadPathHeader, PATCH_JUMP);
	InjectHook(0x477FF0, CFileLoader::LoadCarPathNode, PATCH_JUMP);
	InjectHook(0x477F00, CFileLoader::LoadPedPathNode, PATCH_JUMP);
	InjectHook(0x4780E0, CFileLoader::Load2dEffect, PATCH_JUMP);

	InjectHook(0x478370, CFileLoader::LoadScene, PATCH_JUMP);
	InjectHook(0x4786B0, CFileLoader::LoadObjectInstance, PATCH_JUMP);
	InjectHook(0x478A00, CFileLoader::LoadZone, PATCH_JUMP);
	InjectHook(0x478A90, CFileLoader::LoadCullZone, PATCH_JUMP);

	InjectHook(0x478550, CFileLoader::LoadMapZones, PATCH_JUMP);
ENDPATCHES
