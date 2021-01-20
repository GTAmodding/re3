#pragma once

#if defined _WIN32
#ifdef WITHWINDOWS
#include <windows.h>
#endif
#else
#include "crossplatform.h"
#endif

#ifdef RELOCATABLE

void reloc_initialize(void);

FILE *reloc_fopen(const char *path, const char *mode);
const char *reloc_realpath(const char *filename);

#ifdef _WIN32
#define reloc_casepath(PATH, CHECKFIRST) reloc_realpath(PATH)
#define reloc_fcaseopen reloc_fopen
#else
char *reloc_casepath(char const *path, bool checkPathFirst = true);
FILE *reloc_fcaseopen(char const *filename, char const *mode);
#endif

#if defined WITHWINDOWS
HANDLE reloc_FindFirstFile(const char *pathName, WIN32_FIND_DATA *findFileData);
bool reloc_FindNextFile(HANDLE hFind, WIN32_FIND_DATA *findFileData);
bool reloc_FindClose(HANDLE hFind);
#endif

#else
#define reloc_initialize()

#define reloc_fopen(path, mode) fopen(path, mode)
#define reloc_realpath(filename) (filename)

#define reloc_casepath casepath
#define reloc_fcaseopen fcaseopen

#define reloc_FindFirstFile FindFirstFile
#define reloc_FindNextFile FindNextFile
#define reloc_FindClose FindClose

#endif
