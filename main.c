#include "scanner.h"
#include "player.h"

int main(int argc, char **argv)
{
    printf("main(S)\n");
    //scan(getPath(argc, argv));

    // test code :Play list
    struct MediaNode *node = scan(getPath(argc, argv));
    if (node == NULL) {
        return 0;
    }

    while (node != NULL) {
        printf("    path = %s\n", node->path);
        node = node->next;
    }

    printf("main(X)\n");
    return 0;
}
