#include "common.h"
#include "main.h"

#include "General.h"
#include "Quaternion.h"
#include "ModelInfo.h"
#include "ModelIndices.h"
#include "TempColModels.h"
#include "VisibilityPlugins.h"
#include "FileMgr.h"
#include "HandlingMgr.h"
#include "CarCtrl.h"
#include "PedType.h"
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
#include "MemoryHeap.h"

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
#if GTA_VERSION <= GTA3_PS2_160
	CFileMgr::ChangeDir("\\DATA\\");
	fd = CFileMgr::OpenFile(filename, "r");
	CFileMgr::ChangeDir("\\");
#else
	fd = CFileMgr::OpenFile(filename, "r");
#endif
	assert(fd > 0);

	for(line = LoadLine(fd); line; line = LoadLine(fd)){
		if(*line == '#')
			continue;

#ifdef FIX_BUGS
		if(strncmp(line, "EXIT", 4) == 0)
#else
		if(strncmp(line, "EXIT", 9) == 0)
#endif
			break;

		if(strncmp(line, "IMAGEPATH", 9) == 0){
			RwImageSetPath(line + 10);
		}else if(strncmp(line, "TEXDICTION", 10) == 0){
			PUSH_MEMID(MEMID_TEXTURES);
			strcpy(txdname, line+11);
			LoadingScreenLoadingFile(txdname);
			RwTexDictionary *txd = LoadTexDictionary(txdname);
			AddTexDictionaries(savedTxd, txd);
			RwTexDictionaryDestroy(txd);
			POP_MEMID();
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
				PUSH_MEMID(MEMID_DEF_MODELS);
				CModelInfo::ConstructMloClumps();
				POP_MEMID();
				CObjectData::Initialise("DATA\\OBJECT.DAT");
				objectsLoaded = true;
			}
			PUSH_MEMID(MEMID_WORLD);
			LoadingScreenLoadingFile(line + 4);
			LoadScene(line + 4);
			POP_MEMID();
		}else if(strncmp(line, "MAPZONE", 7) == 0){
			LoadingScreenLoadingFile(line + 8);
			LoadMapZones(line + 8);
		}else if(strncmp(line, "SPLASH", 6) == 0){
#ifndef DISABLE_LOADING_SCREEN
			LoadSplash(GetRandomSplashScreen());
#endif
#ifndef GTA_PS2
		}else if(strncmp(line, "CDIMAGE", 7) == 0){
			CdStreamAddImage(line + 8);
#endif
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

struct ColHeader
{
	uint32 ident;
	uint32 size;
};

void
CFileLoader::LoadCollisionFile(const char *filename)
{
	int fd;
	char modelname[24];
	CBaseModelInfo *mi;
	ColHeader header;

	PUSH_MEMID(MEMID_COLLISION);

	debug("Loading collision file %s\n", filename);
	fd = CFileMgr::OpenFile(filename, "rb");

	while(CFileMgr::Read(fd, (char*)&header, sizeof(header))){
		assert(header.ident == 'LLOC');
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

	POP_MEMID();
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
		REGISTER_MEMPTR(&model.spheres);
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
		REGISTER_MEMPTR(&model.lines);
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
		REGISTER_MEMPTR(&model.boxes);
		for(i = 0; i < model.numBoxes; i++){
			model.boxes[i].Set(*(CVector*)buf, *(CVector*)(buf+12), buf[24], buf[25]);
			buf += 28;
		}
	}else
		model.boxes = nil;

	int32 numVertices = *(int16*)buf;
	buf += 4;
	if(numVertices > 0){
		model.vertices = (CompressedVector*)RwMalloc(numVertices*sizeof(CompressedVector));
		REGISTER_MEMPTR(&model.vertices);
		for(i = 0; i < numVertices; i++){
			model.vertices[i].Set(*(float*)buf, *(float*)(buf+4), *(float*)(buf+8));
			if(Abs(*(float*)buf) >= 256.0f ||
			   Abs(*(float*)(buf+4)) >= 256.0f ||
			   Abs(*(float*)(buf+8)) >= 256.0f)
				printf("%s:Collision volume too big\n", modelname);
			buf += 12;
		}
	}else
		model.vertices = nil;

	model.numTriangles = *(int16*)buf;
	buf += 4;
	if(model.numTriangles > 0){
		model.triangles = (CColTriangle*)RwMalloc(model.numTriangles*sizeof(CColTriangle));
		REGISTER_MEMPTR(&model.triangles);
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

	nodename = GetFrameNodeName(RpAtomicGetFrame(atomic));
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

#ifdef LIBRW
void
InitClump(RpClump *clump)
{
	RpClumpForAllAtomics(clump, ConvertPlatformAtomic, nil);
}
#else
#define InitClump(clump)
#endif

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
			InitClump(clump);
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
				InitClump(clump);
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
	if (mi->GetModelType() == MITYPE_PED && id != 0 && RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)) {
		// Read LOD ped
		clump = RpClumpStreamRead(stream);
		InitClump(clump);
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
		printf("Start loading %s\n", CModelInfo::GetModelInfo(id)->GetModelName());
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

	printf("Finish loading %s\n", CModelInfo::GetModelInfo(id)->GetModelName());
	clump = RpClumpGtaStreamRead2(stream);

	if(clump){
		InitClump(clump);
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
		InitClump(clump);
		gpRelatedModelInfo = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
		RpClumpForAllAtomics(clump, SetRelatedModelInfoCB, clump);
		RpClumpDestroy(clump);
	}
	return true;
}

#ifdef HARDCODED_MODEL_FLAGS
char *DoubleSidedNames[] = {
	"chnabankdoor",
	"Security_Hut",
	"Hospital_Sub",
	"phonebooth1",
	"trafficlight1",
	"sub_roadbarrier",
	"redlightbuild09",
	"doublestreetlght1",
	"doc_shedbig31",
	"com_land_128",
	"garage7",
	"proj_garage01",
	"buildingground2",
	"buildingground3",
	"ch_roof_kb",
	"overpassind",
	"casino",
	"ind_land100",
	"fuckedup_skewlbus",
	"Police_Station_ind",
	"flagsitaly",
	"sidebarrier_gaz1",
	"bar_barrier12",
	"bar_barrier10b",
	"sidebarrier_gaz2",
	"doc_shedbig3",
	"doc_shedbig4",
	"verticalift_bridge",
	"verticalift_bridg2",
	"usdcrdlrbuild01",
	"apairporthanger",
	"apairporthangerA",
	"porthangerclosed",
	"redlightbuild13",
	"doc_rave",
	"const_woodfence",
	"const_woodfence2",
	"const_woodfence3",
	"subfraightback01",
	"subfraightback02",
	"subfraightback03",
	"subfraightback04",
	"subind_build03",
	"chinabanner1",
	"chinabanner2",
	"chinabanner3",
	"chinabanner4",
	"Pumpfirescape",
	"Pumphouse",
	"amcounder",
	"barrel1",
	"barrel2",
	"barrel3",
	"barrel4",
	"com_1way50",
	"com_1way20",
	"overpasscom01",
	"overpasscom02",
	"overpasscom03",
	"overpasscom04",
	"overpass_comse",
	"newdockbuilding",
	"newdockbuilding2",
	"policeballhall",
	"fuzballdoor",
	"ind_land106",
	"PoliceBallSigns",
	"amcoudet",
	"rustship_structure",
	"impexpgrgesub",
	"ind_land128",
	"fshfctry_dstryd",
	"railtrax_bentl",
	"railtrax_lo4b",
	"railtrax_straight",
	"railtrax_bentrb",
	"railtrax_skew",
	"newtrackaaa",
	"railtrax_skew5",
	// these they forgot:
	"railtrax_skewp",
	"railtrax_ske2b",
	"railtrax_strtshort",
	"railtrax_2b",
	"railtrax_straightss",
	"railtrax_bentr",
	"ind_land125",
	"salvstrans",
	"bridge_liftsec",
	"subsign1",
	"carparkfence",
	"newairportwall4",
	"apair_terminal",
	"Helipad",
	"bar_barrier10",
	"damissionfence",
	"sub_floodlite",
	"suburbbridge1",
	"damfencing",
	"demfence08",
	"damfence07",
	"damfence06",
	"damfence05",
	"damfence04",
	"damfence03",
	"damfence02",
	"damfence01",
	"Dam_pod2",
	"Dam_pod1",
	"columansion_wall",
	"wrckdhse020",
	"wrckdhse01",
	"arc_bridge",
	"gRD_overpass19kbc",
	"gRD_overpass19bkb",
	"gRD_overpass19kb",
	"gRD_overpass18kb",
	"road_under",
	"com_roadkb23",
	"com_roadkb22",
	"nbbridgerda",
	"nbbridgerdb",
	"policetenkb1",
	"block3_scraper2",
	"Clnm_cthdrlfcde",
	"broadwaybuild",
	"combillboard03",
	"com_park3b",
	"com_docksaa",
	"newdockbuilding2",
	"com_roadkb22",
	"sidebarrier_gaz2",
	"tunnelsupport1",
	"skyscrpunbuilt2",
	"cons_buid02",
	"rail_platformw",
	"railtrax_bent1",
	"nrailstepswest",
	"building_fucked",
	"franksclb02",
	"salvsdetail",
	"crgoshp01",
	"shp_wlkway",
	"bar_barriergate1",
	"plnt_pylon01",
	"fishfctory",
	"doc_crane_cab",
	"nrailsteps",
	"iten_club01",
	"mak_Watertank",
	"basketballcourt"
	"carlift01",
	"carlift02",
	"iten_chinatown4",
	"iten_details7",
	"ind_customroad002"
	"ind_brgrd1way",
	"ind_customroad060",
	"ind_customroad002",
	"ind_land108",
	"ind_customroad004",
	"ind_customroad003",
	"nbbridgcabls01",
	"sbwy_tunl_bit",
	"sbwy_tunl_bend",
	"sbwy_tunl_cstm11",
	"sbwy_tunl_cstm10",
	"sbwy_tunl_cstm9",
	"sbwy_tunl_cstm8",
	"sbwy_tunl_cstm7",
	"sbwy_tunl_cstm6",
	"sbwy_tunl_cstm5",
	"sbwy_tunl_cstm4",
	"sbwy_tunl_cstm3",
	"sbwy_tunl_cstm2",
	"sbwy_tunl_cstm1",
	""

};
char *TreeNames[] = {
	"coast_treepatch",
	"comparknewtrees",
	"comtreepatchprk",
	"condotree01",
	"condotree1",
	"indatree03",
	"indtreepatch5",
	"indtreepatch06f",
	"new_carprktrees",
	"new_carprktrees4",
	"newcoasttrees1",
	"newcoasttrees2",
	"newcoasttrees3",
	"newtreepatch_sub",
	"newtrees1_sub",
	"newunitrepatch",
	"pinetree_narrow",
	"pinetree_wide",
	"treencom2",
	"treepatch",
	"treepatch01_sub",
	"treepatch02_sub",
	"treepatch2",
	"treepatch2b",
	"treepatch03",
	"treepatch03_sub",
	"treepatch04_sub",
	"treepatch05_sub",
	"treepatch06_sub",
	"treepatch07_sub",
	"treepatch08_sub",
	"treepatch09_sub",
	"treepatch10_sub",
	"treepatch11_sub",
	"treepatch12_sub",
	"treepatch13_sub",
	"treepatch14_sub",
	"treepatch15_sub",
	"treepatch16_sub",
	"treepatch17_sub",
	"treepatch18_sub",
	"treepatch19_sub",
	"treepatch20_sub",
	"treepatch21_sub",
	"treepatch22_sub",
	"treepatch23_sub",
	"treepatch24_sub",
	"treepatch25_sub",
	"treepatch26_sub",
	"treepatch27_sub",
	"treepatch28_sub",
	"treepatch29_sub",
	"treepatch30_sub",
	"treepatch31_sub",
	"treepatch32_sub",
	"treepatch33_sub",
	"treepatch34_sub",
	"treepatch35_sub",
	"treepatch69",
	"treepatch152_sub",
	"treepatch153_sub",
	"treepatch171_sub",
	"treepatch172_sub",
	"treepatch173_sub",
	"treepatch212_sub",
	"treepatch213_sub",
	"treepatch214_sub",
	"treepatcha",
	"treepatchb",
	"treepatchcomtop1",
	"treepatchd",
	"treepatche",
	"treepatchh",
	"treepatchindaa2",
	"treepatchindnew",
	"treepatchindnew2",
	"treepatchk",
	"treepatchkb4",
	"treepatchkb5",
	"treepatchkb6",
	"treepatchkb7",
	"treepatchkb9",
	"treepatchl",
	"treepatchm",
	"treepatchnew_sub",
	"treepatchttwrs",
	"treesuni1",
	"trepatchindaa1",
	"veg_bush2",
	"veg_bush14",
	"veg_tree1",
	"veg_tree3",
	"veg_treea1",
	"veg_treea3",
	"veg_treeb1",
	"veg_treenew01",
	"veg_treenew03",
	"veg_treenew05",
	"veg_treenew06",
	"veg_treenew08",
	"veg_treenew09",
	"veg_treenew10",
	"veg_treenew16",
	"veg_treenew17",
	"vegclubtree01",
	"vegclubtree02",
	"vegclubtree03",
	"vegpathtree",
	""
};
char *OptimizedNames[] = {
	"coast_treepatch",
	"comparknewtrees",
	"comtreepatchprk",
	"indtreepatch5",
	"indtreepatch06f",
	"new_carprktrees",
	"new_carprktrees4",
	"newcoasttrees1",
	"newcoasttrees2",
	"newcoasttrees3",
	"newtreepatch_sub",
	"newtrees1_sub",
	"newunitrepatch",
	"treepatch",
	"treepatch01_sub",
	"treepatch02_sub",
	"treepatch2",
	"treepatch2b",
	"treepatch03",
	"treepatch03_sub",
	"treepatch04_sub",
	"treepatch05_sub",
	"treepatch06_sub",
	"treepatch07_sub",
	"treepatch08_sub",
	"treepatch09_sub",
	"treepatch10_sub",
	"treepatch11_sub",
	"treepatch12_sub",
	"treepatch13_sub",
	"treepatch14_sub",
	"treepatch15_sub",
	"treepatch16_sub",
	"treepatch17_sub",
	"treepatch18_sub",
	"treepatch19_sub",
	"treepatch20_sub",
	"treepatch21_sub",
	"treepatch22_sub",
	"treepatch23_sub",
	"treepatch24_sub",
	"treepatch25_sub",
	"treepatch26_sub",
	"treepatch27_sub",
	"treepatch28_sub",
	"treepatch29_sub",
	"treepatch30_sub",
	"treepatch31_sub",
	"treepatch32_sub",
	"treepatch33_sub",
	"treepatch34_sub",
	"treepatch35_sub",
	"treepatch69",
	"treepatch152_sub",
	"treepatch153_sub",
	"treepatch171_sub",
	"treepatch172_sub",
	"treepatch173_sub",
	"treepatch212_sub",
	"treepatch213_sub",
	"treepatch214_sub",
	"treepatcha",
	"treepatchb",
	"treepatchcomtop1",
	"treepatchd",
	"treepatche",
	"treepatchh",
	"treepatchindaa2",
	"treepatchindnew",
	"treepatchindnew2",
	"treepatchk",
	"treepatchkb4",
	"treepatchkb5",
	"treepatchkb6",
	"treepatchkb7",
	"treepatchkb9",
	"treepatchl",
	"treepatchm",
	"treepatchnew_sub",
	"treepatchttwrs",
	"treesuni1",
	"trepatchindaa1",
	"combtm_treeshad01",
	"combtm_treeshad02",
	"combtm_treeshad03",
	"combtm_treeshad04",
	"combtm_treeshad05",
	"combtm_treeshad06",
	"comtop_tshad",
	"comtop_tshad2",
	"comtop_tshad3",
	"comtop_tshad4",
	"comtop_tshad5",
	"comtop_tshad6",
	"se_treeshad01",
	"se_treeshad02",
	"se_treeshad03",
	"se_treeshad04",
	"se_treeshad05",
	"se_treeshad06",
	"treeshads01",
	"treeshads02",
	"treeshads03",
	"treeshads04",
	"treeshads05",
	""
};
// not from mobile
static bool
MatchModelName(char *name, char **list)
{
	int i;
	char *s;
	for(i = 0; *list[i] != '\0'; i++)
		if(strncmp(name, "LOD", 3) == 0){
			if(!CGeneral::faststricmp(name+3, list[i]+3))
				return true;
		}else{
			if(!CGeneral::faststricmp(name, list[i]))
				return true;
		}
	return false;
}
#endif

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
	if(clump)
		InitClump(clump);
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

#define isLine3(l, a, b, c) ((l[0] == a) && (l[1] == b) && (l[2] == c))
#define isLine4(l, a, b, c, d) ((l[0] == a) && (l[1] == b) && (l[2] == c) && (l[3] == d))

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
	int mlo;

	section = NONE;
	pathIndex = -1;
	mlo = 0;
	debug("Loading object types from %s...\n", filename);

	fd = CFileMgr::OpenFile(filename, "rb");
	for(line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)){
		if(*line == '\0' || *line == '#')
			continue;

		if(section == NONE){
			if(isLine4(line, 'o','b','j','s')) section = OBJS;
			else if(isLine4(line, 't','o','b','j')) section = TOBJ;
			else if(isLine4(line, 'h','i','e','r')) section = HIER;
			else if(isLine4(line, 'c','a','r','s')) section = CARS;
			else if(isLine4(line, 'p','e','d','s')) section = PEDS;
			else if(isLine4(line, 'p','a','t','h')) section = PATH;
			else if(isLine4(line, '2','d','f','x')) section = TWODFX;
		}else if(isLine3(line, 'e','n','d')){
			section = section == MLO ? OBJS : NONE;
		}else switch(section){
		case OBJS:
			if(isLine3(line, 's','t','a'))
				mlo = LoadMLO(line);
			else
				LoadObject(line);
			break;
		case MLO:
			LoadMLOInstance(mlo, line);
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
				if(strcmp(pathTypeStr, "ped") == 0)
					pathType = 1;
				else if(strcmp(pathTypeStr, "car") == 0)
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
#ifdef EXTRA_MODEL_FLAGS
	// same flag values as SA
	mi->m_bIsTree =        !!(flags & 0x2000);
	mi->m_bIsDoubleSided = !!(flags & 0x200000);
	// new value otherwise unused
	mi->m_bCanBeIgnored =  !!(flags & 0x10000);

#ifdef HARDCODED_MODEL_FLAGS
	// mobile sets these flags in CFileLoader::SetRelatedModelInfoCB, but that's stupid
	if(MatchModelName(mi->GetModelName(), DoubleSidedNames)) mi->m_bIsDoubleSided = true;
	if(MatchModelName(mi->GetModelName(), TreeNames)) mi->m_bIsTree = true;
	if(MatchModelName(mi->GetModelName(), OptimizedNames)) mi->m_bCanBeIgnored = true;
#endif

#endif
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
	mi->SetModelName(model);
	mi->SetNumAtomics(numObjs);
	mi->SetLodDistances(dist);
	SetModelInfoFlags(mi, flags);
	mi->m_firstDamaged = damaged;
	mi->SetTexDictionary(txd);
	MatchModelString(model, id);
}

int
CFileLoader::LoadMLO(const char *line)
{
	char smth[8];
	char name[24];
	int modelIndex;
	float someFloat;

	sscanf(line, "%s %s %d %f", smth, name, &modelIndex, &someFloat);
	CMloModelInfo *minfo = CModelInfo::AddMloModel(modelIndex);
	minfo->SetModelName(name);
	minfo->field_34 = someFloat;
	int instId = CModelInfo::GetMloInstanceStore().allocPtr;
	minfo->firstInstance = instId;
	minfo->lastInstance = instId;
	minfo->SetTexDictionary("generic");
	return modelIndex;
}

void
CFileLoader::LoadMLOInstance(int id, const char *line)
{
	char name[24];
	RwV3d pos, scale, rot;
	float angle;
	int modelIndex;

	CMloModelInfo *minfo = (CMloModelInfo*)CModelInfo::GetModelInfo(id);
	sscanf(line, "%d %s %f %f %f %f %f %f %f %f %f %f",
		&modelIndex,
		name,
		&pos.x, &pos.y, &pos.z,
		&scale.x, &scale.y, &scale.z,
		&rot.x, &rot.y, &rot.z,
		&angle);
	float rad = Acos(angle) * 2.0f;
	CInstance *inst = CModelInfo::GetMloInstanceStore().Alloc();
	minfo->lastInstance++;

	RwMatrix *matrix = RwMatrixCreate();
	RwMatrixScale(matrix, &scale, rwCOMBINEREPLACE);
	RwMatrixRotate(matrix, &rot, -RADTODEG(rad), rwCOMBINEPOSTCONCAT);
	RwMatrixTranslate(matrix, &pos, rwCOMBINEPOSTCONCAT);

	inst->GetMatrix() = CMatrix(matrix);
	inst->GetMatrix().UpdateRW();

	inst->m_modelIndex = modelIndex;
	RwMatrixDestroy(matrix);
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
	mi->SetModelName(model);
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

	if(sscanf(line, "%d %s %s", &id, model, txd) == 3){
		mi = CModelInfo::AddClumpModel(id);
		mi->SetModelName(model);
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
	mi->SetModelName(model);
	mi->SetTexDictionary(txd);
	for(p = gamename; *p; p++)
		if(*p == '_') *p = ' ';
	strcpy(mi->m_gameName, gamename);
	mi->m_level = level;
	mi->m_compRules = comprules;

	if(strcmp(type, "car") == 0){
		mi->m_wheelId = misc;
		mi->m_wheelScale = wheelScale;
		mi->m_vehicleType = VEHICLE_TYPE_CAR;
	}else if(strcmp(type, "boat") == 0){
		mi->m_vehicleType = VEHICLE_TYPE_BOAT;
	}else if(strcmp(type, "train") == 0){
		mi->m_vehicleType = VEHICLE_TYPE_TRAIN;
	}else if(strcmp(type, "heli") == 0){
		mi->m_vehicleType = VEHICLE_TYPE_HELI;
	}else if(strcmp(type, "plane") == 0){
		mi->m_planeLodId = misc;
		mi->m_wheelScale = 1.0f;
		mi->m_vehicleType = VEHICLE_TYPE_PLANE;
	}else if(strcmp(type, "bike") == 0){
		mi->m_bikeSteerAngle = misc;
		mi->m_wheelScale = wheelScale;
		mi->m_vehicleType = VEHICLE_TYPE_BIKE;
	}else
		assert(0);

	mi->m_handlingId = mod_HandlingManager.GetHandlingId(handlingId);

	// Well this is kinda dumb....
	if(strcmp(vehclass, "poorfamily") == 0){
		mi->m_vehicleClass = CCarCtrl::POOR;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, CCarCtrl::POOR);
	}else if(strcmp(vehclass, "richfamily") == 0){
		mi->m_vehicleClass = CCarCtrl::RICH;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, CCarCtrl::RICH);
	}else if(strcmp(vehclass, "executive") == 0){
		mi->m_vehicleClass = CCarCtrl::EXEC;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, CCarCtrl::EXEC);
	}else if(strcmp(vehclass, "worker") == 0){
		mi->m_vehicleClass = CCarCtrl::WORKER;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, CCarCtrl::WORKER);
	}else if(strcmp(vehclass, "special") == 0){
		mi->m_vehicleClass = CCarCtrl::SPECIAL;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, CCarCtrl::SPECIAL);
	}else if(strcmp(vehclass, "big") == 0){
		mi->m_vehicleClass = CCarCtrl::BIG;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, CCarCtrl::BIG);
	}else if(strcmp(vehclass, "taxi") == 0){
		mi->m_vehicleClass = CCarCtrl::TAXI;
		while(frequency-- > 0)
			CCarCtrl::AddToCarArray(id, CCarCtrl::TAXI);
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
	mi->SetModelName(model);
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
	effect = CModelInfo::Get2dEffectStore().Alloc();
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
			&effect->light.shadowSize,
			&shadowIntens, &lightType, &roadReflection, &flare, &flags);
		effect->light.corona = RwTextureRead(corona, nil);
		effect->light.shadow = RwTextureRead(shadow, nil);
		effect->light.shadowIntensity = shadowIntens;
		effect->light.lightType = lightType;
		effect->light.roadReflection = roadReflection;
		effect->light.flareType = flare;

		if(flags & LIGHTFLAG_FOG_ALWAYS)
			flags &= ~LIGHTFLAG_FOG_NORMAL;
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
		effect->attractor.type = flags;
#ifdef FIX_BUGS
		effect->attractor.probability = clamp(probability, 0, 255);
#else
		effect->attractor.probability = probability;
#endif
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
			if(isLine4(line, 'i','n','s','t')) section = INST;
			else if(isLine4(line, 'z','o','n','e')) section = ZONE;
			else if(isLine4(line, 'c','u','l','l')) section = CULL;
			else if(isLine4(line, 'p','i','c','k')) section = PICK;
			else if(isLine4(line, 'p','a','t','h')) section = PATH;
		}else if(isLine3(line, 'e','n','d')){
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
				strcmp(pathTypeStr, "ped");
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
		entity->m_level = CTheZones::GetLevelFromPosition(&entity->GetPosition());
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
		entity->m_level = CTheZones::GetLevelFromPosition(&entity->GetPosition());
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
			if(isLine4(line, 'z','o','n','e')) section = ZONE;
		}else if(isLine3(line, 'e','n','d')){
			section = NONE;
		}else switch(section){
		case ZONE: {
			char name[24];
			int type, level;
			float minx, miny, minz;
			float maxx, maxy, maxz;
			if(sscanf(line, "%s %d %f %f %f %f %f %f %d",
			          name, &type,
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

void
CFileLoader::ReloadPaths(const char *filename)
{
	enum {
		NONE,
		PATH,
	};
	char *line;
	int section = NONE;
	int id, pathType, pathIndex = -1;
	char pathTypeStr[20];
	debug("Reloading paths from %s...\n", filename);

	int fd = CFileMgr::OpenFile(filename, "r");
	for (line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)) {
		if (*line == '\0' || *line == '#')
			continue;

		if (section == NONE) {
			if (isLine4(line, 'p','a','t','h')) {
				section = PATH;
				ThePaths.AllocatePathFindInfoMem(4500);
			}
		} else if (isLine3(line, 'e','n','d')) {
			section = NONE;
		} else {
			switch (section) {
				case PATH:
					if (pathIndex == -1) {
						id = LoadPathHeader(line, pathTypeStr);
						if (strcmp(pathTypeStr, "ped") == 0)
							pathType = 1;
						else if (strcmp(pathTypeStr, "car") == 0)
							pathType = 0;
						pathIndex = 0;
					} else {
						if (pathType == 1)
							LoadPedPathNode(line, id, pathIndex);
						else if (pathType == 0)
							LoadCarPathNode(line, id, pathIndex);
						pathIndex++;
						if (pathIndex == 12)
							pathIndex = -1;
					}
					break;
				default:
					break;
			}
		}
	}
	CFileMgr::CloseFile(fd);
}

void
CFileLoader::ReloadObjectTypes(const char *filename)
{
	enum {
		NONE,
		OBJS,
		TOBJ,
		TWODFX
	};
	char *line;
	int section = NONE;
	CModelInfo::ReInit2dEffects();
	debug("Reloading object types from %s...\n", filename);

	CFileMgr::ChangeDir("\\DATA\\MAPS\\");
	int fd = CFileMgr::OpenFile(filename, "r");
	CFileMgr::ChangeDir("\\");
	for (line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)) {
		if (*line == '\0' || *line == '#')
			continue;

		if (section == NONE) {
			if (isLine4(line, 'o','b','j','s')) section = OBJS;
			else if (isLine4(line, 't','o','b','j')) section = TOBJ;
			else if (isLine4(line, '2','d','f','x')) section = TWODFX;
		} else if (isLine3(line, 'e','n','d')) {
			section = NONE;
		} else {
			switch (section) {
				case OBJS:
				case TOBJ:
					ReloadObject(line);
					break;
				case TWODFX:
					Load2dEffect(line);
					break;
				default:
					break;
			}
		}
	}
	CFileMgr::CloseFile(fd);
}

void
CFileLoader::ReloadObject(const char *line)
{
	int id, numObjs;
	char model[24], txd[24];
	float dist[3];
	uint32 flags;
	CSimpleModelInfo *mi;

	if(sscanf(line, "%d %s %s %d", &id, model, txd, &numObjs) != 4)
		return;

	switch(numObjs){
	case 1:
		sscanf(line, "%d %s %s %d %f %d",
			&id, model, txd, &numObjs, &dist[0], &flags);
		break;
	case 2:
		sscanf(line, "%d %s %s %d %f %f %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &flags);
		break;
	case 3:
		sscanf(line, "%d %s %s %d %f %f %f %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &dist[2], &flags);
		break;
	}

	mi = (CSimpleModelInfo*) CModelInfo::GetModelInfo(id);
	if (
#ifdef FIX_BUGS
		mi &&
#endif
	    mi->GetModelType() == MITYPE_SIMPLE && !strcmp(mi->GetModelName(), model) && mi->m_numAtomics == numObjs) {
		mi->SetLodDistances(dist);
		SetModelInfoFlags(mi, flags);
	} else {
		printf("Can't reload %s\n", model);
	}
}

// unused mobile function - crashes
void
CFileLoader::ReLoadScene(const char *filename)
{
	char *line;
	CFileMgr::ChangeDir("\\DATA\\");
	int fd = CFileMgr::OpenFile(filename, "r");
	CFileMgr::ChangeDir("\\");

	for (line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)) {
		if (*line == '#')
			continue;

#ifdef FIX_BUGS
		if (strncmp(line, "EXIT", 4) == 0)
#else
		if (strncmp(line, "EXIT", 9) == 0)
#endif
			break;

		if (strncmp(line, "IDE", 3) == 0) {
			LoadObjectTypes(line + 4);
		}
	}
	CFileMgr::CloseFile(fd);
}
