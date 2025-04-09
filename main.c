#include <stdio.h>

#define ALI2_IMPLEMENTATION
#define ALI2_REMOVE_PREFIX
#include "ali2.h"

int main(void) {
    log_info(&ali_libc_logger, "Hello, World!\n");
    return 0;
}
