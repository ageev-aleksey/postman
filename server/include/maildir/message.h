//
// Created by nrx on 02.11.2020.
//

#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H
#include "vector_structures.h"

typedef enum d_message_type {
    NEW, CUR
} message_type;

typedef struct d_maildir_message {
    maildir_user *pr_user;
    vector_char *pr_file_name;
    message_type pr_type;
} maildir_message;

bool pr_maildir_message_init(maildir_message *msg);
bool maildir_message_free(maildir_message *msg);
bool maildir_message_release(maildir_message *msg);
bool maildir_message_finalize(maildir_message *msg);

bool maildir_message_get_user(maildir_message *msg, maildir_user *user);
bool maildir_message_read(maildir_message *msg, vector_char *buffer);
bool maildir_message_write(maildir_message *msg, vector_char *buffer);

#endif //SERVER_MESSAGE_H
