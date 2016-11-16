#include <stdio.h>
#include "scanner.h"

#define LOG(format, args...)\
        printf(format " %s(%d) : ", __FILE__, __LINE__, ##args)

void play(const char *path);
