//
// Created by nrx on 08.10.2020.
//
#include "util.h"

const char ERROR_SUCCESS[] = "Success";
const char UTIL_ERROR_ALLOCATED_MEMORY[] = "Error allocated memory";

void* s_malloc(size_t size, error_t *error) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = UTIL_ERROR_ALLOCATED_MEMORY;
        }
        return NULL;
    }

    if (error != NULL) {
        error->error = OK;
        error->message = ERROR_SUCCESS;
    }
    return ptr;
}

const char ERROR_TYPE_OK[] = "OK";
const char ERROR_TYPE_NOT_FOUND[] = "NOT_FOUND";
const char ERROR_TYPE_EXISTS[] = "EXISTS";
const char ERROR_TYPE_FATAL[] = "FATAL";
const char ERROR_TYPE_ERROR[] = "ERROR";

const char *errtostr(enum ErrorType type) {
    const char* str = NULL;
    switch(type) {
        case OK:
            str = ERROR_TYPE_OK;
            break;
        case NOT_FOUND:
            str = ERROR_TYPE_NOT_FOUND;
            break;
        case EXISTS:
            str = ERROR_TYPE_EXISTS;
            break;
        case FATAL:
            str = ERROR_TYPE_FATAL;
            break;
        default:
            str = ERROR_TYPE_ERROR;
    }
    return str;
}