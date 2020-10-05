#pragma once

class CFileMgr
{
	static char ms_rootDirName[128];
	static char ms_dirName[128];
public:
	static void Initialise(void);
	static void ChangeDir(const char *dir);
	static void SetDir(const char *dir);
	static void SetDirMyDocuments(void);
	static size_t LoadFile(const char *file, uint8 *buf, int unused, const char *mode);
	static int OpenFile(const char *file, const char *mode);
	static int OpenFile(const char *file) { return OpenFile(file, "rb"); }
	static int OpenFileForWriting(const char *file);
	static size_t Read(int fd, const char *buf, int len);
	static size_t Write(int fd, const char *buf, int len);
	static bool Seek(int fd, int offset, int whence);
	static bool ReadLine(int fd, char *buf, int len);
	static int CloseFile(int fd);
	static int GetErrorReadWrite(int fd);
	static char *GetRootDirName() { return ms_rootDirName; }
#ifdef XDG_ROOT
  static void GetHomeDirectory(char *homeDir);
  static void GetXDGDataHome(char *homeDir);
#endif
};
