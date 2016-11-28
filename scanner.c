#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>   /* PATH_MAX */
#include <dirent.h> /* DIR */
#include <sys/stat.h>   /* stat */
#include <errno.h>  /* errno */

#include "player.h"

struct MediaNode* mHead;

const char*
getPath(int argc, char **argv) {
    int opt;
    char *path = NULL;

    while ((opt = getopt(argc, argv, "s:")) != -1) 
    {   
        switch (opt)
        {
            case 's':
                path = strdup(optarg);
                break;
            default:
                printf("useage: %s - <scan_path>\n", argv[0]);
                return NULL;
        }
    }   

    char fullPath[PATH_MAX];
    if (!realpath(path, fullPath))
    {
        printf("Failed to resolve path=[%s]\n", path);
    }   
    
    return fullPath; 
}

int
processFile(char *path)
{
    //play(path);
    printf("processFile(%s)\n", path);
    insertAtTail(path, NULL, NULL, 0);
    return TRUE;
}

int
processDirectory(const char *path)
{
    if (DBUG) {
        printf("[D]path = [%s]\n", path);
    }

    DIR* dir = opendir(path);
    if (!dir) {
        printf("Error opening directory '%s', skipping: %s.\n", path, strerror(errno));
    }

    struct dirent *entry;
    struct stat statBuf;

    while ((entry = readdir(dir)))
    {
        const char* name = entry->d_name;
        if (DBUG) {
            printf("[D]name = %s\n", name);
        }

        /* ignore "." and ".." */
        if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0)))
        {
            continue;
        }

        int type = entry->d_type;
        if (type == DT_UNKNOWN)
        {
            /**
             * If the type is UNKNOWN, stat() the file instead.
             * This is sometimes necessary when accessing NFS mounted
             * filesystems, but could be needed in other cases well.
             */
            if (stat(path, &statBuf) == 0)
            {
                if (S_ISREG(statBuf.st_mode))
                {
                    type = DT_REG;
                }
                else if (S_ISDIR(statBuf.st_mode))
                {
                    type = DT_DIR;
                }
            }
            else
            {
                printf("stat() failed for %s: %s\n", path, strerror(errno));
            }
        }
        if (name[0] == '.') {
            continue;
        }

        char filename[PATH_MAX];
        snprintf(filename, PATH_MAX, "%s/%s", path, name);
        if (DBUG) {
            printf("path = %s, filename = %s\n", path, filename);
        }
        if (type == DT_DIR) {   // Directory type
            if (!processDirectory(filename)) {
                closedir(dir);
                return FALSE;
            }
        } else if (type == DT_REG) {    // File type
                printf("filePath(%s)\n", filename);
            if (!processFile((char *)filename)) {
                printf("Failed to process file = %s\n", filename);
                break;
            }
            printf("processDirectory(): mHead->path = %s\n\n", mHead->path);
        }
    }

    closedir(dir);
    return TRUE;
}

struct MediaNode* getMediaNodeHead()
{
    return mHead;
}

struct MediaNode* setMediaNodeHead(MediaNode* node)
{
    return mHead = node;
}

//void scan(const char *path){
struct MediaNode* scan(const char *path){
    mHead = NULL;

    if (!processDirectory(path)) {
        return NULL;
    }

    return getMediaNodeHead();
}

struct MediaNode *getMediaNode(char *path, char *title, char *artist, int duration)
{
    printf("getMediaNode(S)\n");
    struct MediaNode *node;

    node = malloc(sizeof(struct MediaNode));

    printf("getMediaNode(): node(%d)\n", node);

    node->path = NULL;
    node->title = NULL;
    node->artist = NULL;
    node->duration = 0;

    if (path != NULL) {
        node->path = path;
        //printf("    ) path = %s\n", node->path);
    }
    if (title != NULL) {
        node->title = title;
        //printf("    ) title = %s\n", node->title);
    }
    if (artist != NULL) {
        node->artist = artist;
        //printf("    ) artist = %s\n", node->artist);
    }
    if (duration != 0) {
        node->duration = duration;
        //printf("    ) dura = %d\n", node->duration);
    }

    node->prev = NULL;
    node->next = NULL;

#if 1
    struct MediaNode *aaa= getMediaNodeHead();
    int cnt = 1;
    if (aaa != NULL) {
        if (aaa->next == NULL) {
            printf("getMediaNode(): %d. HEAD(%d)->path = %s\n", aaa, cnt, aaa->path);
        }
        while (aaa->next != NULL) {
            printf("getMediaNode(): %d. HEAD->path = %s\n", cnt, aaa->path);
            aaa = aaa->next;
            cnt++;
        }
    } else {
            printf("getMediaNode(): MediaNodeHead is NULL\n");
    }
#endif

    printf("getMediaNode(X)\n");
    return node;
}

void insertAtTail(char *path, char *title, char *artist, int duration)
{
    printf("insertAtTail(S)\n");

    struct MediaNode *temp = mHead;
    struct MediaNode *newNode = getMediaNode(path, title, artist, duration);

    if (temp == NULL) {
        setMediaNodeHead(newNode);
        temp = NULL;
        printf("insertAtTail(X): Head is NULL.\n");
        return;
    }

    while (temp->next != NULL) {
        temp = temp->next;
        printf("insertAtTail(): temp(%d) path = %s\n", temp, temp->path);
    }
    temp->next = newNode;
    newNode->prev = temp;
    temp = NULL;

    printf("insertAtTail(X)\n");
    return;
}
