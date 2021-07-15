#include "common.h"
#include "crossplatform.h"

// Codes compatible with Windows and Linux
#ifndef _WIN32

// For internal use
// wMilliseconds is not needed
void tmToSystemTime(const tm *tm, SYSTEMTIME *out) {
    out->wYear = tm->tm_year + 1900;
    out->wMonth = tm->tm_mon + 1;
    out->wDayOfWeek = tm->tm_wday;
    out->wDay = tm->tm_mday;
    out->wHour = tm->tm_hour;
    out->wMinute = tm->tm_min;
    out->wSecond = tm->tm_sec;
}

void GetLocalTime_CP(SYSTEMTIME *out) {
    time_t timestamp = time(nil);
    tm *localTm = localtime(&timestamp);
    tmToSystemTime(localTm, out);
}
#endif

// Compatible with Linux/POSIX and MinGW on Windows
#ifndef _WIN32
HANDLE FindFirstFile(const char* pathname, WIN32_FIND_DATA* firstfile) {
	char pathCopy[MAX_PATH];
	strcpy(pathCopy, pathname);

	char *folder = strtok(pathCopy, "*");
	char *extension = strtok(NULL, "*");

	// because I remember like strtok might not return NULL for last delimiter
	if (extension && extension - folder == strlen(pathname))
		extension = nil;
	
	// Case-sensitivity and backslashes...
	// Will be freed at the bottom
	char *realFolder = casepath(folder);
	if (realFolder) {
		folder = realFolder;
	}

	strncpy(firstfile->folder, folder, sizeof(firstfile->folder));

	if (extension)
		strncpy(firstfile->extension, extension, sizeof(firstfile->extension));
	else
		firstfile->extension[0] = '\0';

	if (realFolder)
		free(realFolder);

	HANDLE d;
	if ((d = (HANDLE)opendir(firstfile->folder)) == NULL || !FindNextFile(d, firstfile))
		return NULL;

	return d;
}

bool FindNextFile(HANDLE d, WIN32_FIND_DATA* finddata) {
	dirent *file;
	static struct stat fileStats;
	static char path[PATH_MAX], relativepath[NAME_MAX + sizeof(finddata->folder) + 1];
	int extensionLen = strlen(finddata->extension);
	while ((file = readdir((DIR*)d)) != NULL) {

		// We only want "DT_REG"ular Files, but reportedly some FS and OSes gives DT_UNKNOWN as type.
		if ((file->d_type == DT_UNKNOWN || file->d_type == DT_REG || file->d_type == DT_LNK) &&
			(extensionLen == 0 || strncasecmp(&file->d_name[strlen(file->d_name) - extensionLen], finddata->extension, extensionLen) == 0)) {

			sprintf(relativepath, "%s/%s", finddata->folder, file->d_name);
			realpath(relativepath, path);
			stat(path, &fileStats);
			strncpy(finddata->cFileName, file->d_name, sizeof(finddata->cFileName));
			finddata->ftLastWriteTime = fileStats.st_mtime;
			return true;
		}
	}
	return false;
}

void GetDateFormat(int unused1, int unused2, SYSTEMTIME* in, int unused3, char* out, int size) {
	tm linuxTime;
	linuxTime.tm_year = in->wYear - 1900;
	linuxTime.tm_mon = in->wMonth - 1;
	linuxTime.tm_wday = in->wDayOfWeek;
	linuxTime.tm_mday = in->wDay;
	linuxTime.tm_hour = in->wHour;
	linuxTime.tm_min = in->wMinute;
	linuxTime.tm_sec = in->wSecond;
	strftime(out, size, nl_langinfo(D_FMT), &linuxTime);
}

void FileTimeToSystemTime(time_t* writeTime, SYSTEMTIME* out) {
	tm *ptm = gmtime(writeTime);
	tmToSystemTime(ptm, out);
}
#endif

// Because wchar length differs between platforms.
wchar*
AllocUnicode(const char* src)
{
	wchar *dst = (wchar*)malloc(strlen(src)*2 + 2);
	wchar *i = dst;
	while((*i++ = (unsigned char)*src++) != '\0');
	return dst;
}

// Funcs/features from Windows that we need on other platforms
#ifndef _WIN32
char *strupr(char *s) {
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = toupper((unsigned char) *tmp);
    }

    return s;
}
char *strlwr(char *s) {
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = tolower((unsigned char) *tmp);
    }

    return s;
}

char *trim(char *s) {
    char *ptr;
    if (!s)
        return NULL;   // handle NULL string
    if (!*s)
        return s;      // handle empty string
    for (ptr = s + strlen(s) - 1; (ptr >= s) && isspace(*ptr); --ptr);
    ptr[1] = '\0';
    return s;
}

FILE* _fcaseopen(char const* filename, char const* mode)
{
    FILE* result;
    char* real = casepath(filename);
    if (!real)
        result = fopen(filename, mode);
    else {
        result = fopen(real, mode);
        free(real);
    }
    return result;
}

int _caserename(const char *old_filename, const char *new_filename)
{
    int result;
    char *real_old = casepath(old_filename);
    char *real_new = casepath(new_filename);

    // hack so we don't even try to rename it to new_filename if it already exists
    if (!real_new) {
        free(real_old);
        return -1;
    }

    if (!real_old)
        result = rename(old_filename, real_new);
    else
        result = rename(real_old, real_new);

    free(real_old);
    free(real_new);

    return result;
}

// Case-insensitivity on linux (from https://github.com/OneSadCookie/fcaseopen)
// Returned string should freed manually (if exists)
char* casepath(char const* path, bool checkPathFirst)
{
    if (checkPathFirst && access(path, F_OK) != -1) {
        // File path is correct
        return nil;
    }

    size_t l = strlen(path);
    char* p = (char*)alloca(l + 1);
    char* out = (char*)malloc(l + 3); // for extra ./
    strcpy(p, path);

    // my addon: linux doesn't handle filenames with spaces at the end nicely
    p = trim(p);

    size_t rl = 0;

    DIR* d;
    char* c;

    #if defined(__SWITCH__) || defined(PSP2)
    if( (c = strstr(p, ":/")) != NULL) // scheme used by some environments, eg. switch, vita
    {
        size_t deviceNameOffset = c - p + 3;
        char* deviceNamePath = (char*)alloca(deviceNameOffset + 1);
        strlcpy(deviceNamePath, p, deviceNameOffset);
        deviceNamePath[deviceNameOffset] = 0;
        d = opendir(deviceNamePath);
        p = c + 1;
    }
    else
    #endif
    if (p[0] == '/' || p[0] == '\\')
    {
        d = opendir("/");
    }
    else
    {
        d = opendir(".");
        out[0] = '.';
        out[1] = 0;
        rl = 1;
    }

    bool cantProceed = false; // just convert slashes in what's left in string, don't correct case of letters(because we can't)
    bool mayBeTrailingSlash = false;

    while (c = strsep(&p, "/\\"))
    {
        // May be trailing slash(allow), slash at the start(avoid), or multiple slashes(avoid)
        if (*c == '\0')
        {
            mayBeTrailingSlash = true;
            continue;
        } else {
            mayBeTrailingSlash = false;
        }

        out[rl] = '/';
        rl += 1;
        out[rl] = 0;

        if (cantProceed)
        {
            strcpy(out + rl, c);
            rl += strlen(c);
            continue;
        }

        struct dirent* e;
        while (e = readdir(d))
        {
            if (strcasecmp(c, e->d_name) == 0)
            {
                strcpy(out + rl, e->d_name);
                int reportedLen = (int)strlen(e->d_name);
                rl += reportedLen;
                assert(reportedLen == strlen(c) && "casepath: This is not good at all");

                closedir(d);
                d = opendir(out);

                // Either it wasn't a folder, or permission error, I/O error etc.
                if (!d) {
                    cantProceed = true;
                }

                break;
            }
        }

        if (!e)
        {
            printf("casepath couldn't find dir/file \"%s\", full path was %s\n", c, path);
            // No match, add original name and continue converting further slashes.
            strcpy(out + rl, c);
            rl += strlen(c);
            cantProceed = true;
        }
    }

    if (d) closedir(d);
    if (mayBeTrailingSlash) {
        out[rl] = '/';  rl += 1;
        out[rl] = '\0';
    }

    if (rl > l + 2) {
        printf("\n\ncasepath: Corrected path length is longer then original+2:\n\tOriginal: %s (%zu chars)\n\tCorrected: %s (%zu chars)\n\n", path, l, out, rl);
    }
    return out;
}
#endif

#ifdef __SWITCH__
/* Taken from glibc */
char *realpath(const char *name, char *resolved)
{
   char *rpath, *dest = NULL;
   const char *start, *end, *rpath_limit;
   long int path_max;

   /* As per Single Unix Specification V2 we must return an error if
      either parameter is a null pointer.  We extend this to allow
      the RESOLVED parameter to be NULL in case the we are expected to
      allocate the room for the return value.  */
   if (!name)
      return NULL;

   /* As per Single Unix Specification V2 we must return an error if
      the name argument points to an empty string.  */
   if (name[0] == '\0')
      return NULL;

#ifdef PATH_MAX
   path_max = PATH_MAX;
#else
   path_max = pathconf(name, _PC_PATH_MAX);
   if (path_max <= 0)
      path_max = 1024;
#endif

   if (!resolved)
   {
      rpath = (char*)malloc(path_max);
      if (!rpath)
         return NULL;
   }
   else
      rpath = resolved;
   rpath_limit = rpath + path_max;

   if (name[0] != '/')
   {
      if (!getcwd(rpath, path_max))
      {
         rpath[0] = '\0';
         goto error;
      }
      dest = (char*)memchr(rpath, '\0', path_max);
   }
   else
   {
      rpath[0] = '/';
      dest = rpath + 1;
   }

   for (start = end = name; *start; start = end)
   {
      /* Skip sequence of multiple path-separators.  */
      while (*start == '/')
         ++start;

      /* Find end of path component.  */
      for (end = start; *end && *end != '/'; ++end)
         /* Nothing.  */;

      if (end - start == 0)
         break;
      else if (end - start == 1 && start[0] == '.')
         /* nothing */;
      else if (end - start == 2 && start[0] == '.' && start[1] == '.')
      {
         /* Back up to previous component, ignore if at root already.  */
         if (dest > rpath + 1)
            while ((--dest)[-1] != '/')
               ;
      }
      else
      {
         size_t new_size;

         if (dest[-1] != '/')
            *dest++ = '/';

         if (dest + (end - start) >= rpath_limit)
         {
            ptrdiff_t dest_offset = dest - rpath;
            char *new_rpath;

            if (resolved)
            {
               if (dest > rpath + 1)
                  dest--;
               *dest = '\0';
               goto error;
            }
            new_size = rpath_limit - rpath;
            if (end - start + 1 > path_max)
               new_size += end - start + 1;
            else
               new_size += path_max;
            new_rpath = (char *)realloc(rpath, new_size);
            if (!new_rpath)
               goto error;
            rpath = new_rpath;
            rpath_limit = rpath + new_size;

            dest = rpath + dest_offset;
         }

         dest = (char*)memcpy(dest, start, end - start);
         *dest = '\0';
      }
   }
   if (dest > rpath + 1 && dest[-1] == '/')
      --dest;
   *dest = '\0';

   return rpath;

error:
   if (!resolved)
      free(rpath);
   return NULL;
}

ssize_t readlink (const char * __path, char * __buf, size_t __buflen)
{
   errno = ENOSYS;
   return -1;
}
#endif
