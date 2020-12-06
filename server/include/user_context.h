//
// Created by nrx on 05.12.2020.
//

#ifndef SERVER_USER_CONTEXT_H
#define SERVER_USER_CONTEXT_H
#include "sys/queue.h"
#include "smtp/state.h"
#include "vector_structures.h"
#include "maildir/maildir.h"
struct {
    maildir md;
    int16_t  port;
    char *log_file_path;
    
} global_config_server;

struct user_context {
    smtp_state smtp;
    vector_char  buffer;
    int socket;

};

#endif //SERVER_USER_CONTEXT_H
