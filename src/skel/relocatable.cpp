#ifdef RELOCATABLE
#include "common.h"
#include "FileMgr.h"

#include <string>
#ifdef _MSC_VER
#include <io.h>
#else
#include <libgen.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <sys/param.h>
#include <mach-o/dyld.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <direct.h>
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#endif

#define WITHWINDOWS
#include "crossplatform.h"
#include "relocatable.h"


#ifndef R_OK
#define R_OK 4
#endif

#ifdef _WIN32
#define PATHJOIN '\\'
#define PATHJOIN_S "\\"
#define PATHSEP ';'
#else
#define PATHJOIN '/'
#define PATHJOIN_S "/"
#define PATHSEP ':'
#endif

bool
FileExists(const char *path)
{
#if _WIN32
	bool exists = PathFileExistsA(path);
	struct stat status;
	stat(path, &status);
	return (status.st_mode & _S_IFREG) != 0;
#else
	return access(path, R_OK) == 0;
#endif
}


bool
containsGta3Data(const std::string &dirPath)
{
	std::string path(dirPath);
	path.append("models" PATHJOIN_S "gta3.img");
	return FileExists(path.c_str());
}

#ifdef _WIN32
std::string
winReadRegistry(HKEY hParentKey, const char *keyName, const char *valueName)
{
	HKEY hKey;
	if (RegOpenKeyExA(hParentKey, keyName, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		std::string res;
		DWORD valueSize = 1024;
		res.resize(valueSize);
		DWORD keyType;
		if (RegQueryValueEx(hKey, valueName, NULL, &keyType, (LPBYTE)res.data(), &valueSize) == ERROR_SUCCESS) {
			RegCloseKey(hKey);
			if(keyType == REG_SZ) {
				res.resize(strlen(res.data()));
				return res;
			}
		}
		RegCloseKey(hKey);
	}
	return "";
}

std::string
psExePath()
{
	std::string buf;
	buf.resize(MAX_PATH);
	if (GetModuleFileName(NULL, (LPSTR)buf.c_str(), MAX_PATH) == 0) {
		debug("GetModuleFileNameA failed: cannot get path of executable.\n");
		return "";
	}
	buf.resize(strlen(buf.c_str()));
	return buf;
}
#endif

#ifdef __APPLE__
std::string
psExePath()
{
	std::string buf;
	uint32_t bufsize = 0;
	_NSGetExecutablePath(NULL, &bufsize);
	buf.resize(bufsize + 1);
	if(_NSGetExecutablePath((char*)buf.c_str(), &bufsize) != 0) {
		debug("_NSGetExecutablePath failed: cannot get path of executable.\n");
		return "";
	}
	buf.resize(bufsize);
	return buf;
}
#endif

#ifdef __linux__
std::string
psExePath()
{
	std::string buf;
	buf.resize(MAX_PATH);
	ssize_t nb = readlink("/proc/self/exe", (char*)buf.c_str(), MAX_PATH);
	if (nb == -1) {
		return "";
	}
	buf.resize(strlen(buf.c_str()));
	return buf;
}
#endif

std::string
psExeDir()
{
	std::string exePath = psExePath();
	if (!exePath.size()) {
		return "";
	}
#ifdef _MSC_VER
	std::string::size_type lastSepPos = exePath.rfind(PATHJOIN);
	if (lastSepPos >= 0) {
		exePath.resize(lastSepPos);
		return exePath;
	}
	return "";
#else
	return dirname((char*)exePath.c_str());
#endif
}

#ifdef _WIN32
std::string
winGetGtaDataFolder()
{
	if (containsGta3Data("C:\\Program Files\\Rockstar Games\\GTAIII\\")) { 
		return "C:\\Program Files\\Rockstar Games\\GTAIII\\";
	}
	std::string location;
	location.resize(MAX_PATH);
	BOOL success = SHGetSpecialFolderPathA(NULL, (LPSTR)location.c_str(), CSIDL_PROGRAM_FILESX86, FALSE);
	if (success) {
		location.resize(strlen(location.c_str()));
		location.append("\\Rockstar Games\\GTAIII\\");
		if(containsGta3Data(location)) { 
			return location;
		}
	}
#if defined(STEAMID)
	location = winReadRegistry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App " STR(STEAMID), "InstallLocation");
	if (location.size()) {
		location.push_back('\\');
		if (containsGta3Data(location)) { 
			return location;
		}
	}
#endif
	return "";
}
#endif

void
normalizePathCorrectPathJoiner(std::string &s)
{
#ifdef _WIN32
#define PATHJOIN_BAD '/'
#else
#define PATHJOIN_BAD '\\'
#endif
	std::string::size_type pos(0);
	while((pos = s.find(PATHJOIN_BAD, pos)) != std::string::npos) {
		s[pos] = PATHJOIN;
	}
}

void
ensureTrailingPathJoiner(std::string &s)
{
	if(s.size() && *s.rbegin() != PATHJOIN) {
		s.push_back(PATHJOIN); 
	}
}


#include <vector>
std::vector<std::string> searchPaths;

void
reloc_initialize()
{
	// FIXME: is called twice
	searchPaths.clear();
	std::string path;
	// 1. add search paths from command line
	// FIXME
	// 2. `GTA_III_RE_DIR` environment variable
	const char *pEnv = ::getenv("GTA3DIR");
	if (pEnv) {
		std::string env = pEnv;
		std::string::size_type start(0);
		std::string::size_type end;
		while ((end = env.find(PATHSEP, start)) != std::string::npos) {
			searchPaths.push_back(env.substr(start, end - start));
			start = end + 1;
		}
		searchPaths.push_back(env.substr(start));
	}
	// 3. add search paths from INI
	// FIXME
	// 4. add current working directory to search path
	path.resize(MAX_PATH);
	char *cwd = getcwd((char *)path.c_str(), path.size());
	if (cwd) {
		path.resize(strlen(path.c_str()));
		searchPaths.push_back(path);
	}
	// 5. add exe path to search path
	path = psExeDir();
	if(path.size()) {
		searchPaths.push_back(path);
	}
#ifdef _WIN32
	// 6. add default installation paths
	path = winGetGtaDataFolder();
	if(path.size()) {
		searchPaths.push_back(path);
	}
#endif
	// 6. built-in path
#ifdef GTA_III_RE_DIR
	searchPaths.push_back(GTA_III_RE_DIR);
#endif

	// Normalize paths
	for(std::string &path : searchPaths) {
		normalizePathCorrectPathJoiner(path);
		ensureTrailingPathJoiner(path);
	}

	debug("Full search path is:\n");
	for (const std::string &s: searchPaths) {
		debug("- %s\n", s.c_str());
	}
}

std::string
searchPathsFindData(const char *path)
{
	const char *cwd = CFileMgr::GetWorkingDir(); 
	std::string suffix;
	if (cwd && *cwd != '\0') {
		suffix.append(cwd);
		std::string::size_type pos = 0;
		while(suffix[pos] == PATHJOIN) {
			++pos;
		}
		suffix = suffix.substr(pos, suffix.size() - pos);
		if (suffix[suffix.size() - 1] != PATHJOIN) {
			suffix.push_back(PATHJOIN);
		}
	}
	suffix.append(path);
	normalizePathCorrectPathJoiner(suffix);
	for (const auto &searchpath : searchPaths) {
		std::string curPath = searchpath + suffix;
		if (FileExists(curPath.c_str())) {
			return curPath;
		}
		FILE *f = fopen(curPath.c_str(), "rb");
		if(f) { 
			fclose(f);
			debug("FileExists failed, but fopen succeeded. huh?\n");
		}
	}
	TRACE("Could not find \"%s\" in search paths (cwd=\"%s\").\n", path, cwd);
	std::string res;
	if (cwd && *cwd != '\0') {
		res.append(cwd);
		res.push_back(PATHJOIN);
	}
	res.append(path);
	normalizePathCorrectPathJoiner(res);
	return res;
}

#include "FileMgr.h"
#include "common.h"

FILE *
reloc_fopen(const char *path, const char *mode)
{
	std::string fullPath = searchPathsFindData(path);
	TRACE("reloc_fopen(%s, %s) -> fopen(%s, %s)", path, mode, fullPath.c_str(), mode);
	return fopen(fullPath.c_str(), mode);
}

const char*
reloc_realpath(const char *filename)
{
	static std::string fullPath;
	fullPath = searchPathsFindData(filename);
	TRACE("reloc_realpath(%s) -> %s", filename, fullPath.c_str());
	return fullPath.c_str();
}

struct reloc_HANDLE {
	HANDLE hFind;
	std::string fullPathName;
	int searchPathIndex;
};

HANDLE
reloc_FindFirstFile(const char *pathName, WIN32_FIND_DATA *findFileData)
{
	std::string suffix = CFileMgr::GetWorkingDir();
	if (suffix.size()) {
		std::string::size_type pos = 0;
		while(suffix[pos] == PATHJOIN) { ++pos; }
		suffix = suffix.substr(pos, suffix.size() - pos);
		if (suffix[suffix.size() - 1] != PATHJOIN) {
			suffix.push_back(PATHJOIN);
		}
	}
	suffix.append(pathName);

	for (int i = 0; i < (int)searchPaths.size(); ++i) {
		HANDLE hFind = FindFirstFile((searchPaths[i] + suffix).c_str(), findFileData);
		if (hFind != INVALID_HANDLE_VALUE) {
			return (HANDLE) new reloc_HANDLE {hFind, suffix, i};
		}
	}
	return INVALID_HANDLE_VALUE;
}

bool
reloc_FindNextFile(HANDLE hFind, WIN32_FIND_DATA *findFileData)
{
	reloc_HANDLE *relocHandle = (reloc_HANDLE *) hFind;
	if (FindNextFile(relocHandle->hFind, findFileData)) {
		return true;
	}
	FindClose(relocHandle->hFind);
	relocHandle->hFind = NULL;

	for (int i = relocHandle->searchPathIndex + 1; i < (int)searchPaths.size(); ++i) {
		HANDLE newHFind = FindFirstFile((searchPaths[i] + relocHandle->fullPathName).c_str(), findFileData);
		if (newHFind != INVALID_HANDLE_VALUE) {
			relocHandle->hFind = newHFind;
			relocHandle->searchPathIndex = i;
			return true;
		}
	}
	return false;
}

bool
reloc_FindClose(HANDLE hFind)
{
	reloc_HANDLE *relocHandle = (reloc_HANDLE *)hFind;
	bool res = true; 
	if (relocHandle->hFind != NULL) {
		res = FindClose(relocHandle->hFind);
	}
	delete relocHandle;
	return res;
}

#ifndef _WIN32
static bool
casepath_subpath(const std::string &basePath, const char *path, std::string &result) {
	const char *curLevelStart;
	size_t curLevelSize;
	bool currentIsFile;

	if (path == NULL) {
		return false;
	}

	// Fail early when the base path does not exist.
	DIR *d = opendir(basePath.c_str());
	if (d == NULL) {
		return false;
	}

	// Split path in parts, ignoring empty levels (e.g. '/a//b' --> {'a', 'b'})
	const char *nextCurLevelStart = path;
	while (true) {
		curLevelStart = nextCurLevelStart;
		const char *delimPos = strpbrk(curLevelStart, "/\\");
		if (delimPos == NULL) {
			curLevelSize = strlen(curLevelStart);
			currentIsFile = true;
			break;
		}
		curLevelSize = delimPos - curLevelStart;
		nextCurLevelStart = delimPos + 1;
		currentIsFile = false;

		if (curLevelSize > 0) {
			break;
		}
	}
	char *curLevel = (char*) alloca(curLevelSize + 1);
	strncpy(curLevel, curLevelStart, curLevelSize);
	curLevel[curLevelSize] = '\0';

	struct dirent *e;
	while ((e = readdir(d)) != NULL) {
		if (strcasecmp(curLevel, e->d_name) == 0) {
			if (currentIsFile) {
				result = basePath + e->d_name;
				closedir(d);
				return true;
			}
			bool subres = casepath_subpath(basePath + e->d_name + PATHJOIN_S, nextCurLevelStart, result);
			if (subres) {
				return subres;
			}
		}
	}
	closedir(d);
	return false;
}


char*
reloc_casepath(const char *path, bool checkPathFirst)
{
	std::string realPath;
	realPath.reserve(PATH_MAX);
	if (checkPathFirst) {
		for (const std::string &searchPath : searchPaths) {
			realPath = searchPath + CFileMgr::GetWorkingDir() + path;
			if (access(realPath.c_str(), F_OK) != -1) {
				// File path is correct
				debug("reloc_casepath(%s, %d) -> %s\n", path, checkPathFirst, realPath.c_str());
				return strdup(realPath.c_str());
			}
		}
	}

	std::string suffixPath = std::string{CFileMgr::GetWorkingDir()} + path;
	for (const std::string &searchPath : searchPaths) {
		bool res = casepath_subpath(searchPath, suffixPath.c_str(), realPath);
		if (res) {
			debug("reloc_casepath(%s, %d) -> %s\n", path, checkPathFirst, realPath.c_str());
			return strdup(realPath.c_str());
		}
	}
	debug("reloc_casepath(%s, %d) failed (wd=%s)\n", path, checkPathFirst, CFileMgr::GetWorkingDir());
	return NULL;
}

FILE *
reloc_fcaseopen(char const *filename, char const *mode)
{
	FILE *result;
	char *real = reloc_casepath(filename);
	if (real == NULL) {
		result = fopen(filename, mode);
	} else {
		result = fopen(real, mode);
		free(real);
	}
	return result;
}

#endif
#endif

