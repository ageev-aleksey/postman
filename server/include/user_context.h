//
// Created by nrx on 05.12.2020.
//

#ifndef SERVER_USER_CONTEXT_H
#define SERVER_USER_CONTEXT_H
#include "sys/queue.h"
#include "smtp/state.h"
#include "vector_structures.h"

struct user_context {
    smtp_state smtp;
    vector_char  buffer;
    int socket;
};

#endif //SERVER_USER_CONTEXT_H
