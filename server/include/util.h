//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H
#include <stdlib.h>

#include "error_t.h"

void* s_malloc(size_t size, error_t *error);

int make_server_socket(const char *ip, int port, error_t *error);

#endif //SERVER_UTIL_H
