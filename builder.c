#define ALI2_IMPLEMENTATION
#define ALI2_REMOVE_PREFIX
#include "ali2.h"

int main(int argc, char** argv) {
    Ali_Cmd cmd = {0};
    ALI_REBUILD_YOURSELF(&cmd, argc, argv);

    Ali_Build b = {0};

    {
        Ali_Step exe = ali_step_executable("main");
        ali_step_add_src(&exe, ali_step_file("main.c"));
        ali_build_install(&b, exe);
    }

    if (!ali_build_build(&b)) return 1;
    ali_build_free(&b);

    da_free(&cmd);
    return 0;
}
