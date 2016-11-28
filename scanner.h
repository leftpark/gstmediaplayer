#ifndef __SCANNER__
#define __SCANNER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>   /* PATH_MAX */
#include <dirent.h> /* DIR */
#include <sys/stat.h>   /* stat */
#include <errno.h>  /* errno */

#define DBUG 0

#define FILE_EXTENSION_MP3 "mp3"
#define FILE_EXTENSION_WAV "wav"

typedef struct MediaNode
{
    const char *path;
    const char *title;
    const char *artist;
    int duration;

    struct MediaNode *prev;
    struct MediaNode *next;
}MediaNode;

//struct MediaNode* mHead;

struct MediaNode* getMediaNode(char *path, char *title, char *artist, int duration);
void insertAtTail(char *path, char *title, char *artist, int duration);

const char* getPath(int argc, char **argv);

int processFile(char *path);

int processDirectory(const char *path);

//void scan(const char *path);
struct MediaNode *scan(const char *path);


#endif //__SCANNER__
