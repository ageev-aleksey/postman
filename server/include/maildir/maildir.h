//
// Created by nrx on 02.11.2020.
//

#ifndef SERVER_MAILDIR_H
#define SERVER_MAILDIR_H
#include "error_t.h"

#include <stdbool.h>
#include <sys/queue.h>


typedef struct d_maildir {

} maildir;


typedef struct d_maildir_user_list {

} maildir_user_list;

bool maildir_open(maildir *md);
// USERS
bool maildir_get_users(maildir *md, struct maildir_users_list *users_list);
bool maildir_get_user(maildir *md, maildir_user **user);
bool maildir_add_user(maildir *md, maildir_user *user);
bool maildir_delete_user(maildir *md, maildir_user *user);
// SERVERS
bool maildir_server_list(maildir *md, maildir_server_list *servers_list);
bool maildir_add_server(maildir *md, maildir_server *server);
bool maildir_delete_server(maildir *md, maildir_server *server);


#endif //SERVER_MAILDIR_H
