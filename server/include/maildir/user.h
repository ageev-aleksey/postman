//
// Created by nrx on 02.11.2020.
//

#ifndef SERVER_USER_H
#define SERVER_USER_H

#include "vector_structures.h"

#include <stdbool.h>
#include <linux/limits.h>
#include <sys/queue.h>


typedef struct d_maildir_user {
    struct d_maildir_server *pr_server;
    char  pr_login[NAME_MAX];
} maildir_user;

typedef struct d_maildir_users_entry {
    maildir_user  user;
    LIST_ENTRY(d_maildir_users_entry) entries;
} maildir_users_entry;

LIST_HEAD(d_maildir_users_list, d_maildir_users_entry);
typedef struct d_maildir_users_list maildir_users_list;

struct d_maildir_message;
typedef struct d_maildir_message maildir_message;
typedef struct d_maildir_messages_list maildir_messages_list;

typedef struct d_maildir_server maildir_server;



// Инициализация структуры
//bool pr_maildir_user_init(maildir_user *user, maildir_server *server, char *user_name,  err_t *error);
void maildir_user_default_init(maildir_user *user);
void maildir_user_free(maildir_user *user);
//bool maildir_user_release(maildir_user *user);
// Свойства пользователя
//  - Логин
bool maildir_user_login(maildir_user *user, char **login);
//  - Сервер
bool maildir_user_server(maildir_user *user, maildir_server **server);
//  - Сообщения
bool maildir_user_create_message(maildir_user *user, maildir_message *message, char *sender_name, err_t *error);
bool maildir_user_message_list(maildir_user *user, maildir_messages_list *msg_list, err_t *error);

#endif //SERVER_USER_H
