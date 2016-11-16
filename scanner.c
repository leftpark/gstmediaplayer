#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>   /* PATH_MAX */
#include <dirent.h> /* DIR */
#include <sys/stat.h>   /* stat */
#include <errno.h>  /* errno */

#include "player.h"

#define TRUE 1
#define FALSE 0
#define DBUG 1

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
processFile(const char *path)
{
    if (strstr(path, ".mp3")) {
        // TODO
        play(path);
    } else if (strstr(path, ".wav")) {
        // TODO
    }
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
        if (type == DT_DIR) {   // Directory type
            if (!processDirectory(filename)) {
                closedir(dir);
                return -1;
            }
        } else if (type == DT_REG) {    // File type
            if (!processFile(filename)) {
                printf("Failed to process file = %s\n", filename);
                break;
            }
        }
    }
    closedir(dir);

    return 0;
}

void scan(const char *path){
    processDirectory(path);
}
