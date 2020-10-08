#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <libunshield.h>

#ifdef _MSC_VER
#include <direct.h>
#define mkdir _mkdir
#define strncasecmp _strnicmp
#endif

#ifdef __APPLE__
#include <crt_externs.h>
#include <sysdir.h>
#define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif

const char *GROUPS[] = {"App Executables", "Don't Delete", NULL};

char *getenvvar(const char *varName) {
	char **p = environ;
	char *value = malloc(PATH_MAX);
	memset(value, 0, PATH_MAX);
	size_t varNameLength = strlen(varName);
	char *search = malloc(varNameLength + 2);
	memset(search, 0, varNameLength + 2);
	strcpy(search, varName);
	strcat(search, "=");
	for (; *p; p++) {
		if (!strncmp(search, *p, varNameLength + 1)) {
			strcpy(value, *p + varNameLength + 1);
			break;
		}
	}
	free(search);
	return value;
}

/** From
 * https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950 */
int mkdir_p(const char *path) {
	/* Adapted from http://stackoverflow.com/a/2336245/119527 */
	const size_t len = strlen(path);
	char _path[PATH_MAX];
	char *p;

	errno = 0;

	/* Copy string so its mutable */
	if (len > sizeof(_path) - 1) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(_path, path);

	/* Iterate the string */
	for (p = _path + 1; *p; p++) {
		if (*p == '/') {
			/* Temporarily truncate */
			*p = '\0';

			if (mkdir(_path, S_IRWXU) != 0) {
				if (errno != EEXIST)
					return -1;
			}

			*p = '/';
		}
	}

	if (mkdir(_path, S_IRWXU) != 0) {
		if (errno != EEXIST)
			return -1;
	}

	return 0;
}

bool isFile(const char *path) {
	struct stat statbuf;
	int ret = stat(path, &statbuf);
	if (ret < 0) {
		return false;
	}
	return (statbuf.st_mode & S_IFMT) == S_IFREG;
}

bool isDirectory(const char *path) {
	struct stat statbuf;
	int ret = stat(path, &statbuf);
	if (ret < 0) {
		return false;
	}
	return (statbuf.st_mode & S_IFMT) == S_IFDIR;
}

bool isISO(const char *path) {
	if (!strncmp(path, "iso", 3)) {
		return true;
	}
	size_t len = strlen(path);
	if (len < 3) {
		return false;
	}
	return !strncasecmp(path + len - 3, "iso", 3);
}

bool endsWithDLL(const char *path) {
	size_t len = strlen(path);
	if (len < 3) {
		return false;
	}
	return !strncasecmp(path + len - 3, "dll", 3);
}

bool endsWithEXE(const char *path) {
	size_t len = strlen(path);
	if (len < 3) {
		return false;
	}
	return !strncasecmp(path + len - 3, "exe", 3);
}

bool endsWithURL(const char *path) {
	size_t len = strlen(path);
	if (len < 3) {
		return false;
	}
	return !strncasecmp(path + len - 3, "url", 3);
}

bool canIgnore(const char *path) {
	return endsWithEXE(path) || endsWithDLL(path) || endsWithURL(path);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s ISO1|DIR1 ISO2|DIR2\n", argv[0]);
		fprintf(stderr, "\nISO1 must be the first disc as an ISO image.\n");
		fprintf(stderr, "ISO2 must be the second disc as an ISO image.\n");
		fprintf(stderr,
		        "DIR1 must be the directory containing data1.cab, "
		        "data1.hdr, and data2.cab from the first disc.\n");
		fprintf(stderr,
		        "DIR2 must be the directory containing the Audio directory "
		        "from the second disc.\n");
		fprintf(stderr, "\nBoth arguments must be of the same type.\n");
		return 1;
	}

	if ((!isISO(argv[1]) && isISO(argv[2])) ||
	    (!isDirectory(argv[1]) && isDirectory(argv[2]))) {
		fprintf(stderr, "\nBoth arguments must be of the same type.\n");
		return 1;
	}

	struct stat buf;
	if (stat(argv[1], &buf) < 0) {
		fprintf(stderr, "Error occurred when reading %s: ", argv[1]);
		perror(NULL);
		return 1;
	}
	if (stat(argv[2], &buf) < 0) {
		fprintf(stderr, "Error occurred when reading %s: ", argv[2]);
		perror(NULL);
		return 1;
	}

	char *dir1 = strdup(argv[1]);
	char *dir2 = strdup(argv[2]);
	bool isoMode = false;

	if (isISO(argv[1]) && isISO(argv[2])) {
		isoMode = true;
		// Extract disc 1
		char *tmpDirEnv = getenvvar("TMPDIR");
		if (!strlen(tmpDirEnv)) {
			memset(tmpDirEnv, 0, PATH_MAX);
			strcpy(tmpDirEnv, "/tmp/re3.XXXXXX");
		} else {
			strcat(tmpDirEnv, "/re3.XXXXXX");
		}
		mkdtemp(tmpDirEnv);
		assert(tmpDirEnv != NULL);
		char outArg[PATH_MAX];
		memset(outArg, 0, PATH_MAX);
		sprintf(outArg, "-o%s", tmpDirEnv);
		int status;
		pid_t pid = fork();
		if (pid == 0) {
			// Child process
			int ret = execlp("7z",
			                 "7z",
			                 "x",
			                 "-aoa",
			                 "-bb0",
			                 "-bd",
			                 "-y",
			                 outArg,
			                 "--",
			                 argv[1],
			                 "data1.cab",
			                 "data1.hdr",
			                 "data2.cab",
			                 NULL);
			assert(ret == 0);
			_exit(EXIT_SUCCESS);
		} else if (pid < 0) {
			// Fork failed
			status = -1;
		} else {
			// Parent process. Wait for child to exit
			if (waitpid(pid, &status, 0) != pid) {
				status = -1;
			}
			if (status != 0) {
				fprintf(stderr, "7z failed to extract %s\n.", argv[1]);
				free(tmpDirEnv);
				free(dir1);
				free(dir2);
				return 1;
			}
		}
		free(dir1);
		free(dir2);
		dir1 = dir2 = strdup(tmpDirEnv);
		free(tmpDirEnv);
		// Extract disc 2
		pid = fork();
		if (pid == 0) {
			// Child process
			int ret = execlp("7z",
			                 "7z",
			                 "x",
			                 "-aoa",
			                 "-bb0",
			                 "-bd",
			                 "-y",
			                 outArg,
			                 "--",
			                 argv[2],
			                 "Audio",
			                 NULL);
			assert(ret == 0);
			_exit(EXIT_SUCCESS);
		} else if (pid < 0) {
			// Fork failed
			status = -1;
		} else {
			// Parent process. Wait for child to exit
			if (waitpid(pid, &status, 0) != pid) {
				status = -1;
			}
		}
		if (status != 0) {
			fprintf(stderr, "7z failed to extract %s\n.", argv[2]);
			free(dir1);
			return 1;
		}
	}

	char *outputDir = malloc(PATH_MAX);
	char *homeDir = getenvvar("HOME");
#ifdef XDG_ROOT
	char *xdgDataHome = getenvvar("XDG_DATA_HOME");
	if (strlen(xdgDataHome) == 0) {
		assert(strlen(homeDir) > 0);
		sprintf(outputDir, "%s/.local/share/re3", homeDir);
	} else {
		sprintf(outputDir, "%s/re3", xdgDataHome);
	}
	free(xdgDataHome);
#elif defined(__APPLE__)
	char path[PATH_MAX];
	sysdir_search_path_enumeration_state state =
	    sysdir_start_search_path_enumeration(
	        SYSDIR_DIRECTORY_APPLICATION_SUPPORT, SYSDIR_DOMAIN_MASK_USER);
	while ((state = sysdir_get_next_search_path_enumeration(state, path)) !=
	       0) {
		if (state == 0) {
			fprintf(stderr, "Failed to get a valid directory.\n");
			return 1;
		}
		assert(path[0] != '\0');
		sprintf(outputDir, "%s%s/re3", homeDir, path + 1);
		break;
	}
#endif
	free(homeDir);

	// Extract data1.cab
	char *data1CabPath = malloc(PATH_MAX);
	memset(data1CabPath, 0, PATH_MAX);
	sprintf(data1CabPath, "%s/data1.cab", dir1);
	unshield_set_log_level(UNSHIELD_LOG_LEVEL_ERROR);
	Unshield *unshield = unshield_open(data1CabPath);
	if (!unshield) {
		fprintf(stderr, "Failed to open %s\n", data1CabPath);
		free(dir2);
		free(outputDir);
		return 1;
	}
	for (int groupIndex = 0;
	     groupIndex < ((sizeof GROUPS / sizeof GROUPS[0]) - 1);
	     groupIndex++) {
		UnshieldFileGroup *group =
		    unshield_file_group_find(unshield, GROUPS[groupIndex]);
		if (!group) {
			fprintf(stderr, "Could not find %s group\n", GROUPS[groupIndex]);
			free(dir2);
			free(outputDir);
			return 1;
		}
		char *targetDir = malloc(PATH_MAX);
		for (int i = group->first_file; i <= group->last_file; i++) {
			if (unshield_file_is_valid(unshield, i)) {
				const char *name = unshield_file_name(unshield, i);
				if (canIgnore(name)) {
					continue;
				}
				char *dir = (char *)unshield_directory_name(
				    unshield, unshield_file_directory(unshield, i));
				for (int j = 0; j < strlen(dir); j++) {
					if (dir[j] == '\\') {
						dir[j] = '/';
					}
				}
				memset(targetDir, 0, PATH_MAX);
				if (groupIndex == 0 &&
				    !strcmp(dir, "audio")) { // Keep casing consistent
					dir[0] = 'A';
				}
				int ret = sprintf(targetDir,
				                  "%s%s%s",
				                  outputDir,
				                  dir ? "/" : "",
				                  dir ? dir : "");
				assert(ret > 0);
				if (mkdir_p(targetDir) < 0) {
					if (errno != EEXIST) {
						free(dir2);
						fprintf(stderr,
						        "Failed to create directory %s: ",
						        targetDir);
						perror(NULL);
						return 1;
					}
				}
				char *targetPath = malloc(PATH_MAX);
				memset(targetPath, 0, PATH_MAX);
				ret = sprintf(targetPath, "%s/%s", targetDir, name);
				assert(ret > 0);
				bool unshieldRet = unshield_file_save(unshield, i, targetPath);
				assert(unshieldRet);
				free(targetPath);
			}
		}
		free(targetDir);
	}
	unshield_close(unshield);
	// Copy disc 2 Audio
	int status;
	pid_t pid = fork();
	char *audioDir = malloc(PATH_MAX);
	sprintf(audioDir, "%s/Audio/", dir2);
	if (pid == 0) {
		// Child process
		int ret = execlp("cp", "cp", "-R", "--", audioDir, outputDir, NULL);
		assert(ret == 0);
		_exit(EXIT_SUCCESS);
	} else if (pid < 0) {
		// Fork failed
		status = -1;
	} else {
		// Parent process. Wait for child to exit
		if (waitpid(pid, &status, 0) != pid) {
			status = -1;
		}
		if (status != 0) {
			free(outputDir);
			fprintf(stderr, "cp command failed.\n.");
			return 1;
		}
	}
	// Clean up
	if (isoMode) {
		pid = fork();
		if (pid == 0) {
			execlp("rm", "rm", "-fR", dir2, NULL);
			_exit(EXIT_SUCCESS);
		} else if (pid < 0) {
			status = -1;
		} else {
			if (waitpid(pid, &status, 0) != pid) {
				perror(NULL);
			}
		}
	}
	free(dir1);
	if (dir2 != dir1) {
		free(dir2);
	}
	free(audioDir);
	free(outputDir);
	free(data1CabPath);
	return status == 0 ? 0 : 1;
}
