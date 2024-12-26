#include "foo.h"

int num = 0;

void foo(int num_) {
    num += num_;
}

int bar(void) {
    return num;
}

