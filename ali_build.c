#define ALI_REMOVE_PREFIX
#define ALI_IMPLEMENTATION
#include "ali.h"

int main(int argc, char** argv) {
    char** cmd = NULL;
    rebuild_yourself(&cmd, argc, argv);

    CexeBuilder exe = c_exe("main", NULL);
    c_exe_add_flag(&exe, "-Wall");
    c_exe_add_flag(&exe, "-Wextra");
    c_exe_add_flag(&exe, "-Werror");
    c_exe_add_flag(&exe, "-ggdb");
    if (!c_exe_execute(&cmd, &exe)) return 1;

    return 0;
}
