//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_ERROR_T_H
#define SERVER_ERROR_T_H

#include <stdbool.h>

const char SUCCESS[] = "Success";


typedef struct _error_t {
    bool is_error;
    char *message;
} error_t;


#endif //SERVER_ERROR_T_H
