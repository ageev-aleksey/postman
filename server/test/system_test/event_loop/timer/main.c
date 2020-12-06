#include "event_loop/event_loop.h"
#include <stdio.h>

void timer_handler(event_loop *el, int fd, timer_event_entry *timer_descriptor) {
    static int i = 0;
    i++;
    printf("timer handler\n");
    if (i == 5) {
        el_stop(el, NULL);
    }
}

int main() {
    int fd = 0;
    event_loop *el = el_init(NULL);
    timer_event_entry *desc = NULL;
    el_timer(el, fd, 5, timer_handler, &desc, NULL);
    el_open(el, ONE_THREAD, NULL);
}
