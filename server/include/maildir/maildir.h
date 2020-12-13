//
// Created by nrx on 02.11.2020.
//

#ifndef SERVER_MAILDIR_H
#define SERVER_MAILDIR_H
#include "error_t.h"
#include "vector_structures.h"
#include "server.h"

#include <stdbool.h>
#include <sys/queue.h>
#include <dirent.h>

#define SERVERS_ROOT_NAME ".OTHER_SERVERS"
#define SERVERS_ROOT_NAME_PART "/.OTHER_SERVERS/"
#define USER_PATH_CUR "cur"
#define USER_PATH_TMP "tmp"
#define USER_PATH_NEW "new"

typedef struct d_maildir_user maildir_user;
typedef struct d_maildir_server maildir_server;
//typedef struct d_maildir_servers_list maildir_servers_list;
typedef struct d_maildir_users_list maildir_users_list;

typedef struct d_maildir_server_entry {
    maildir_server server;
    LIST_ENTRY(d_maildir_server_entry) entries;
} maildir_server_entry;
LIST_HEAD(d_maildir_servers_list, d_maildir_server_entry);
typedef struct d_maildir_servers_list maildir_servers_list;

typedef void (*maildir_log_handler)(char*message);

struct maildir_log_handlers {
    maildir_log_handler debug_handler;
    maildir_log_handler error_handler;
};

typedef struct d_maildir {
   char *pr_path;
   struct maildir_log_handlers pr_log;
} maildir;


bool maildir_init(maildir *md, const char* path, err_t *error);
void maildir_free(maildir *md);
bool maildir_release(maildir *md, err_t *error);
// USERS
//bool maildir_get_users_list(maildir *md, maildir_users_list *users_list);
//// ??? bool maildir_get_user(maildir *md, maildir_user **user);
//bool maildir_create_user(maildir *md, maildir_user *user);
//bool maildir_delete_user(maildir *md, maildir_user *user);
// SERVERS
bool maildir_server_list(maildir *md, maildir_servers_list *servers_list, err_t *error);
bool maildir_get_self_server(maildir *md, maildir_server *server, err_t *error);
bool maildir_get_server(maildir *md, maildir_server *server, const char *name, err_t *error);
//bool maildir_create_server(maildir *md, maildir_server *server, char *server_name, err_t *error);
bool maildir_delete_server(maildir *md, maildir_server *server, err_t *error);
// LOGGING
bool maildir_set_logger_handlers(maildir *md, struct maildir_log_handlers *handlers);


#endif //SERVER_MAILDIR_H
