#define ALI_REMOVE_PREFIX
#define ALI_IMPLEMENTATION
#include "ali.h"

static AliArena string_arena = {0};

int main(int argc, char** argv) {
    char** cmd = NULL;
    rebuild_yourself(&cmd, argc, argv);

    char* program = shift_args(&argc, &argv);

    char* subcommand = "build";
    if (argc > 0) {
        subcommand = shift_args(&argc, &argv);
    }

    if (sv_eq(sv_from_cstr(subcommand), sv_from_cstr("build"))) {
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
    } else if (sv_eq(sv_from_cstr(subcommand), sv_from_cstr("clean"))) {
        cmd_append_args(&cmd, "rm", "-rf", "main", "server", "client");

        if (!cmd_run_sync_and_reset(cmd)) return 1;
    } else {
        log_error("Unknown subcommand: %s", subcommand);
        return 1;
    }

    da_free(cmd);
    arena_free(&string_arena);

    return 0;
}
