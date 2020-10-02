#include "event_loop.h"
#include "foo.h"
#include <stdio.h>

int main() {
    event_loop loop;
    el_open(&loop, true);

    return 0;
}
