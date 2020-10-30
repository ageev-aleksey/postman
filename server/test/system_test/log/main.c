//
// Created by nrx on 31.10.2020.
//

#include <stdio.h>
#include "log/log.h"
#include "log/context.h"

int main() {
    if(!log_init(&GLOBAL_LOG_CONTEXT)) {
        printf("Error nit logger context\n");
        return -1;
    }
    const char a[]  = "aaa" "bbb";
    LOG_ERROR("test error: %s [%d]", "message", 154);
    LOG_ERROR("test error: %s [%d]", "hello world", 154);


//    if(log_get_level(GLOBAL_LOG_CONTEXT) >= ERROR_LEVEL) {
//        printf("LEVEL_ERROR\n");
//    }

    pthread_join(GLOBAL_LOG_CONTEXT->thread, NULL);
    return 0;
}