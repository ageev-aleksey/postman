//
// Created by nrx on 08.10.2020.
//
#include "util.h"

const char UTIL_ERROR_ALLOCATED_MEMORY[] = "Error allocated memory";

void* s_malloc(size_t size, error_t *error) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        if (error != NULL) {
            error->is_error = true;
            error->message = UTIL_ERROR_ALLOCATED_MEMORY;
        }
        return NULL;
    }

    if (error != NULL) {
        error->is_error = false;
        error->message = ERROR_NO_ERROR;
    }
    return ptr;
}