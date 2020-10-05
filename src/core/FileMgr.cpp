#define _CRT_SECURE_NO_WARNINGS
#include <fcntl.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "common.h"
#include "crossplatform.h"

#include "FileMgr.h"

const char *_psGetUserFilesFolder();

/*
 * Windows FILE is BROKEN for GTA.
 *
 * We need to support mapping between LF and CRLF for text files
 * but we do NOT want to end the file at the first sight of a SUB character.
 * So here is a simple implementation of a FILE interface that works like GTA expects.
 */

struct myFILE
{
	bool isText;
	FILE *file;
};

#define NUMFILES 20
static myFILE myfiles[NUMFILES];


#if !defined(_WIN32)
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#define _getcwd getcwd

// Case-insensitivity on linux (from https://github.com/OneSadCookie/fcaseopen)
void mychdir(char const *path)
{
	char* r = casepath(path, false);
    if (r) {
        chdir(r);
		free(r);
    } else {
        errno = ENOENT;
    }
}
#else
#define mychdir chdir
#endif

/* Force file to open as binary but remember if it was text mode */
static int
myfopen(const char *filename, const char *mode)
{
	int fd;
	char realmode[10], *p;

	for(fd = 1; fd < NUMFILES; fd++)
		if(myfiles[fd].file == nil)
			goto found;
	return 0;	// no free fd
found:
	myfiles[fd].isText = strchr(mode, 'b') == nil;
	p = realmode;
	while(*mode)
		if(*mode != 't' && *mode != 'b')
			*p++ = *mode++;
		else
			mode++;
	*p++ = 'b';
	*p = '\0';
	
	myfiles[fd].file = fcaseopen(filename, realmode);
	if(myfiles[fd].file == nil)
		return 0;
	return fd;
}

static int
myfclose(int fd)
{
	int ret;
	assert(fd < NUMFILES);
	if(myfiles[fd].file){
		ret = fclose(myfiles[fd].file);
		myfiles[fd].file = nil;
		return ret;
	}
	return EOF;
}

static int
myfgetc(int fd)
{
	int c;
	c = fgetc(myfiles[fd].file);
	if(myfiles[fd].isText && c == 015){
		/* translate CRLF to LF */
		c = fgetc(myfiles[fd].file);
		if(c == 012)
			return c;
		ungetc(c, myfiles[fd].file);
		return 015;
	}
	return c;
}

static int
myfputc(int c, int fd)
{
	/* translate LF to CRLF */
	if(myfiles[fd].isText && c == 012)
		fputc(015, myfiles[fd].file);
	return fputc(c, myfiles[fd].file);
}

static char*
myfgets(char *buf, int len, int fd)
{
	int c;
	char *p;

	p = buf;
	len--;	// NUL byte
	while(len--){
		c = myfgetc(fd);
		if(c == EOF){
			if(p == buf)
				return nil;
			break;
		}
		*p++ = c;
		if(c == '\n')
			break;
	}
	*p = '\0';
	return buf;
}

static size_t
myfread(void *buf, size_t elt, size_t n, int fd)
{
	if(myfiles[fd].isText){
		char *p;
		size_t i;
		int c;

		n *= elt;
		p = (char*)buf;
		for(i = 0; i < n; i++){
			c = myfgetc(fd);
			if(c == EOF)
				break;
			*p++ = c;
		}
		return i / elt;
	}
	return fread(buf, elt, n, myfiles[fd].file);
}

static size_t
myfwrite(void *buf, size_t elt, size_t n, int fd)
{
	if(myfiles[fd].isText){
		char *p;
		size_t i;
		int c;

		n *= elt;
		p = (char*)buf;
		for(i = 0; i < n; i++){
			c = *p++;
			myfputc(c, fd);
			if(feof(myfiles[fd].file))	// is this right?
				break;
		}
		return i / elt;
	}
	return fwrite(buf, elt, n, myfiles[fd].file);
}

static int
myfseek(int fd, long offset, int whence)
{
	return fseek(myfiles[fd].file, offset, whence);
}

static int
myfeof(int fd)
{
	return feof(myfiles[fd].file);
//	return ferror(myfiles[fd].file);
}


char CFileMgr::ms_rootDirName[128] = {'\0'};
char CFileMgr::ms_dirName[128];

#ifdef XDG_ROOT
#define getenvvar(varName)                            \
  char **p = environ;                                 \
  size_t varNameLength = ARRAY_SIZE(varName) - 1;     \
  for (; *p; *p++)                                    \
  {                                                   \
    if (!strncmp(varName "=", *p, varNameLength + 1)) \
    {                                                 \
      strcpy(homeDir, *p + varNameLength + 1);        \
      break;                                          \
    }                                                 \
  }

void CFileMgr::GetHomeDirectory(char *homeDir) {
  getenvvar("HOME");
}

void CFileMgr::GetXDGDataHome(char *homeDir) {
  getenvvar("XDG_DATA_HOME");
}
#endif

void
CFileMgr::Initialise(void)
{
#ifdef XDG_ROOT
  char homeDir[255];
  struct stat buf;
  bzero(homeDir, 255);
  GetXDGDataHome(homeDir);
  if (strlen(homeDir) > 0) {
    strcat(homeDir, "/re3");
    if (stat(homeDir, &buf) < 0) {
      assert(mkdir(homeDir, 0755) == 0);
    }
  }
  else {
    bzero(homeDir, 255);
    GetHomeDirectory(homeDir);
    // Build up ${HOME}/.local/share/re3
    strcat(homeDir, "/.local");
    if (stat(homeDir, &buf) < 0) {
      assert(mkdir(homeDir, 0755) == 0);
    }
    strcat(homeDir, "/share");
    if (stat(homeDir, &buf) < 0) {
      assert(mkdir(homeDir, 0755) == 0);
    }
    strcat(homeDir, "/re3");
    if (stat(homeDir, &buf) < 0) {
      assert(mkdir(homeDir, 0755) == 0);
    }
  }
  strcpy(ms_rootDirName, homeDir);
#else
	_getcwd(ms_rootDirName, 128);
#endif
  strcat(ms_rootDirName, "\\");
}

void
CFileMgr::ChangeDir(const char *dir)
{
	if(*dir == '\\'){
		strcpy(ms_dirName, ms_rootDirName);
		dir++;
	}
	if(*dir != '\0'){
		strcat(ms_dirName, dir);
		// BUG in the game it seems, it's off by one
		if(dir[strlen(dir)-1] != '\\')
			strcat(ms_dirName, "\\");
	}
	mychdir(ms_dirName);
}

void
CFileMgr::SetDir(const char *dir)
{
	strcpy(ms_dirName, ms_rootDirName);
	if(*dir != '\0'){
		strcat(ms_dirName, dir);
		// BUG in the game it seems, it's off by one
		if(dir[strlen(dir)-1] != '\\')
			strcat(ms_dirName, "\\");
	}
	mychdir(ms_dirName);
}

void
CFileMgr::SetDirMyDocuments(void)
{
	SetDir("");	// better start at the root if user directory is relative
	mychdir(_psGetUserFilesFolder());
}

size_t
CFileMgr::LoadFile(const char *file, uint8 *buf, int unused, const char *mode)
{
	int fd;
	size_t n, len;

	fd = myfopen(file, mode);
	if(fd == 0)
		return 0;
	len = 0;
	do{
		n = myfread(buf + len, 1, 0x4000, fd);
		if(n < 0)
			return -1;
		len += n;
	}while(n == 0x4000);
	buf[len] = 0;
	myfclose(fd);
	return len;
}

int
CFileMgr::OpenFile(const char *file, const char *mode)
{
#ifdef XDG_ROOT
  char fixedPath[128] = {'\0'};
  bzero(fixedPath, 128);
  if (ms_dirName[0] == '\0') {
    strcpy(ms_dirName, ms_rootDirName);
  }
  strncpy(fixedPath, ms_dirName, strlen(ms_dirName));
  strcat(fixedPath, file);
  return myfopen(fixedPath, mode);
#else
  return myfopen(file, mode);
#endif
}

int
CFileMgr::OpenFileForWriting(const char *file)
{
	return OpenFile(file, "wb");
}

size_t
CFileMgr::Read(int fd, const char *buf, int len)
{
	return myfread((void*)buf, 1, len, fd);
}

size_t
CFileMgr::Write(int fd, const char *buf, int len)
{
	return myfwrite((void*)buf, 1, len, fd);
}

bool
CFileMgr::Seek(int fd, int offset, int whence)
{
	return !!myfseek(fd, offset, whence);
}

bool
CFileMgr::ReadLine(int fd, char *buf, int len)
{
	return myfgets(buf, len, fd) != nil;
}

int
CFileMgr::CloseFile(int fd)
{
	return myfclose(fd);
}

int
CFileMgr::GetErrorReadWrite(int fd)
{
	return myfeof(fd);
}
