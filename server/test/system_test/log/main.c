//
// Created by nrx on 31.10.2020.
//
#define _GNU_SOURCE
#include <stdio.h>
#include "log/context.h"

int main() {
    if(!LOG_INIT()) {
        printf("Error init logger context\n");
        return -1;
    }
    const char a[]  = "aaa" "bbb";
    LOG_ERROR("test error: %s [%d]", "message", 154);
    LOG_WARNING("test error: %s [%d]", "message", 154);
    LOG_INFO("test error: %s [%d]", "message", 154);
    LOG_DEBUG("test error: %s [%d]", "message", 154);


    LOG_FREE();
    return 0;
}