#include <stdio.h>
#include <stdlib.h>

#define ALI_IMPLEMENTATION
#define ALI_REMOVE_PREFIX
#include "ali.h"

#include "foo.h"

int main(void) {
    for (usize i = 0; i < 10; ++i) {
        foo(i);
        log_info("%d", bar());
    }

	return 0;
}

