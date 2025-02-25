#define ALI_REMOVE_PREFIX
#define ALI_IMPLEMENTATION
#include "ali.h"

#define BUILD_DIR "./build/"

int main(int argc, char** argv) {
    char** cmd = ali_da_create(ali_libc_allocator, 16, sizeof(*cmd));
    rebuild_yourself(&cmd, argc, argv);

    char* program = shift_args(&argc, &argv);

    char* subcommand = "build";
    if (argc > 0 && !sv_starts_with(sv_from_cstr(argv[0]), sv_from_cstr("-"))) {
        subcommand = shift_args(&argc, &argv);
    }

    if (sv_eq(sv_from_cstr(subcommand), sv_from_cstr("build"))) {
        bool* force = flag_option("-f", "Should force building?", false);
        if (!flag_parse(&argc, &argv, program)) return 1;

        if (force || ali_needs_rebuild1("main", "main.c")) {
            cmd_append_args(&cmd, "gcc", "-Wall", "-Wextra", "-Werror", "-o", "main", "main.c");
            if (!cmd_run_sync_and_reset(cmd)) return 1;
        }
    } else if (sv_eq(sv_from_cstr(subcommand), sv_from_cstr("clean"))) {
        cmd_append_args(&cmd, "rm", "-rf", BUILD_DIR);

        if (!cmd_run_sync_and_reset(cmd)) return 1;
    } else {
        logn_error("Unknown subcommand: %s", subcommand);
        return 1;
    }

    da_free(cmd);
    return 0;
}
