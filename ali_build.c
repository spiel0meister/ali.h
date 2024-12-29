#define ALI_REMOVE_PREFIX
#define ALI_IMPLEMENTATION
#include "ali.h"

static AliArena string_arena = {0};

int main(int argc, char** argv) {
    char** cmd = NULL;
    rebuild_yourself(&cmd, argc, argv);

    bool* force = flag_option("-f", "Should force building?", false);

    argc = flag_parse(argc, argv);

    AliCBuilder builder = {0};

    c_builder_reset(&builder, C_EXE, &string_arena, "main", "main.c");
    c_builder_add_flags(&builder, "-Wall", "-Wextra", "-ggdb");
    c_builder_add_libs(&builder, "-lm");
    if (!c_builder_execute(&builder, &cmd, *force)) return 1;

    c_builder_reset(&builder, C_EXE, &string_arena, "server", "server.c");
    c_builder_add_flags(&builder, "-Wall", "-Wextra", "-ggdb");
    if (!c_builder_execute(&builder, &cmd, *force)) return 1;

    c_builder_reset(&builder, C_EXE, &string_arena, "client", "client.c");
    c_builder_add_flags(&builder, "-Wall", "-Wextra", "-ggdb");
    if (!c_builder_execute(&builder, &cmd, *force)) return 1;

    da_free(cmd);
    arena_free(&string_arena);

    return 0;
}
