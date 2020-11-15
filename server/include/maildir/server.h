//
// Created by nrx on 02.11.2020.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "error_t.h"

#include <stdbool.h>
#include <linux/limits.h>


typedef struct d_maildir_user maildir_user;

typedef struct d_maildir maildir;

typedef struct d_maildir_users_list maildir_users_list;

typedef struct d_maildir_server {
    char pr_server_domain[NAME_MAX];
    maildir *pr_md;
} maildir_server;

bool pr_maildir_server_init(maildir_server *server, error_t *error);
void maildir_server_free(maildir_server *server);
bool maildir_server_is_self(maildir_server *server, bool *res, error_t *error);
bool maildir_server_domain(maildir_server *server, char **domain, error_t *error);
bool maildir_server_users(maildir_server *server, maildir_users_list *users, error_t *error);
bool maildir_server_create_user(maildir_server *server, maildir_user *user, const char *username, error_t *error);
bool maildir_server_user(maildir_server *server, maildir_user *user, const char *username, error_t *error);



#endif //SERVER_SERVER_H
