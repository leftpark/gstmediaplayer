#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>   /* PATH_MAX */
#include <dirent.h> /* DIR */
#include <sys/stat.h>   /* stat */
#include <errno.h>  /* errno */

#define DBUG 1
//#define TRUE 1
//#define FALSE 0

#define FILE_EXTENSION_MP3 "mp3"
#define FILE_EXTENSION_WAV "wav"

const char* getPath(int argc, char **argv);

int processFile(const char *path);

int processDirectory(const char *path);

void scan(const char *path);
