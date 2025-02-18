#define ALI_REMOVE_PREFIX
#define ALI_IMPLEMENTATION
#include "ali.h"

static AliArena string_arena = {0};

#define BUILD_DIR "./build/"

int main(int argc, char** argv) {
    AliAllocator arena_allocator = ali_arena_allocator(&string_arena);

    char** cmd = NULL;
    rebuild_yourself(&cmd, argc, argv);

    char* program = shift_args(&argc, &argv);

    char* subcommand = "build";
    if (argc > 0 && !sv_starts_with(sv_from_cstr(argv[0]), sv_from_cstr("-"))) {
        subcommand = shift_args(&argc, &argv);
    }

    if (sv_eq(sv_from_cstr(subcommand), sv_from_cstr("build"))) {
        bool* force = flag_option("-f", "Should force building?", false, 0, 0);
        if (!flag_parse(&argc, &argv, program)) return 1;

        AliCBuilder builder = {0};

        if (!create_dir_if_not_exists(BUILD_DIR)) return 1;

        c_builder_reset(&builder, C_EXE, arena_allocator, BUILD_DIR"main", "main.c");
        c_builder_add_flags(&builder, "-Wall", "-Wextra", "-ggdb");
        c_builder_add_libs(&builder, "-lm");
        if (!c_builder_execute(&builder, &cmd, *force)) return 1;

        c_builder_reset(&builder, C_EXE, arena_allocator, BUILD_DIR"server", "server.c");
        c_builder_add_flags(&builder, "-Wall", "-Wextra", "-ggdb");
        if (!c_builder_execute(&builder, &cmd, *force)) return 1;

        c_builder_reset(&builder, C_EXE, arena_allocator, BUILD_DIR"client", "client.c");
        c_builder_add_flags(&builder, "-Wall", "-Wextra", "-ggdb");
        if (!c_builder_execute(&builder, &cmd, *force)) return 1;
    } else if (sv_eq(sv_from_cstr(subcommand), sv_from_cstr("clean"))) {
        cmd_append_args(&cmd, "rm", "-rf", BUILD_DIR);

        if (!cmd_run_sync_and_reset(cmd)) return 1;
    } else {
        logn_error("Unknown subcommand: %s", subcommand);
        return 1;
    }

    da_free(ali_libc_allocator, cmd);
    arena_free(&string_arena);

    return 0;
}
