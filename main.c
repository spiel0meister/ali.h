#include <stdio.h>
#include <stdlib.h>

#define ALI_IMPLEMENTATION
#define ALI_REMOVE_PREFIX
#include "ali.h"

#define ALI_MATH_IMPLEMENTATION
#define ALI_MATH_REMOVE_PREFIX
#include "ali_math.h"

#define ALI_NET_IMPLEMENTATION
#define ALI_NET_REMOVE_PREFIX
#include "ali_net.h"

int main(int argc, char** argv) {
    char* path = "/home/overlord/dev/ali/main.c";

    char* slash = ali_strchr(path, '/');
    while (*slash != 0) {
        logn_info("%.*s", (int)(slash - path), path);
        path = slash + 1;
        slash = ali_strchr(path, '/');
    }
    logn_info("%s", path);

    return 0;
}

