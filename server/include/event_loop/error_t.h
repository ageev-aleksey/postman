//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_ERROR_T_H
#define SERVER_ERROR_T_H

#include <stdbool.h>

const char ERROR_SUCCESS[] = "Success";

typedef struct _error_t {
    bool is_error;
    const char *message;
} error_t;


#define ERROR_SUCCESS(error) \
do {                         \
    if ((error) != NULL) {   \
        (error)->is_error = false; \
        (error)->message = ERROR_SUCCESS;\
    }\
}while(0)                             \

#endif //SERVER_ERROR_T_H
