#pragma once

class CFileLoader
{
	static char ms_line[256];
public:
	static void LoadLevel(const char *filename);
	static void LoadCollisionFromDatFile(int currlevel);
	static char *LoadLine(int fd);
	static RwTexDictionary *LoadTexDictionary(const char *filename);
	static void LoadCollisionFile(const char *filename);
	static void LoadCollisionModel(uint8 *buf, CColModel &model, char *name);
	static void LoadModelFile(const char *filename);
	static RpAtomic *FindRelatedModelInfoCB(RpAtomic *atomic, void *data);
	static void LoadClumpFile(const char *filename);
	static bool LoadClumpFile(RwStream *stream, uint32 id);
	static bool StartLoadClumpFile(RwStream *stream, uint32 id);
	static bool FinishLoadClumpFile(RwStream *stream, uint32 id);
	static bool LoadAtomicFile(RwStream *stream, uint32 id);
	static RpAtomic *SetRelatedModelInfoCB(RpAtomic *atomic, void *data);
	static RpClump *LoadAtomicFile2Return(const char *filename);
	static void AddTexDictionaries(RwTexDictionary *dst, RwTexDictionary *src);

	static void LoadObjectTypes(const char *filename);
	static void LoadObject(const char *line);
	static void LoadTimeObject(const char *line);
	static void LoadClumpObject(const char *line);
	static void LoadVehicleObject(const char *line);
	static void LoadPedObject(const char *line);
	static int LoadPathHeader(const char *line, char *type);
	static void LoadPedPathNode(const char *line, int id, int node);
	static void LoadCarPathNode(const char *line, int id, int node);
	static void Load2dEffect(const char *line);

	static void LoadScene(const char *filename);
	static void LoadObjectInstance(const char *line);
	static void LoadZone(const char *line);
	static void LoadCullZone(const char *line);
	static void LoadPickup(const char *line);

	static void LoadMapZones(const char *filename);

	static void ReloadPaths(const char *filename);
};
