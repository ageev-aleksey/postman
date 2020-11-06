//
// Created by nrx on 02.11.2020.
//

#ifndef SERVER_USER_H
#define SERVER_USER_H

#include "vector_structures.h"

#include <stdbool.h>

extern struct maildir_server;

typedef struct d_maildir_user {
    struct maildir_server *pr_server;
    vector_char *pr_login;
} maildir_user;


// Инициализация структуры
bool pr_maildir_user_init(maildir_user **user);
void maildir_user_free(maildir_user *user);
bool maildir_user_release(maildir_user *user);
// Свойства пользователя
//  - Логин
bool maildir_user_get_login(maildir_user *user, char *login);
//  - Сервер
bool maildir_user_get_server(maildir_user *user, struct maildir_server *server);
//  - Сообщения
bool maildir_user_create_message(maildir_user *user, struct maildir_message *message);
bool maildir_user_message_list(maildir_user *user, struct maildir_message_list *msg_list);

#endif //SERVER_USER_H
