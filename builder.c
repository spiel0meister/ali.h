#define ALI2_IMPLEMENTATION
#define ALI2_REMOVE_PREFIX
#include "ali2.h"

int main(int argc, char** argv) {
    AliCmd cmd = {0};
    ALI_REBUILD_YOURSELF(&cmd, argc, argv);

    cmd_append_many(&cmd, "gcc", "-ggdb", "-o", "main", "main.c");
    if (!cmd_run_sync_and_reset(&cmd)) return 1;

    return 0;
}
