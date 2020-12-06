//
// Created by nrx on 02.11.2020.
//

#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H

#include <bits/types/FILE.h>
#include "vector_structures.h"
#include <sys/queue.h>
#include <linux/limits.h>

typedef struct d_maildir_user maildir_user;

typedef enum d_message_type {
    NEW, TMP
} message_type;

typedef struct d_maildir_message {
    maildir_user *pr_user;
    char pr_filename[NAME_MAX];
    FILE *pr_fd;
    message_type pr_type;
    bool pr_is_open;
} maildir_message;


struct d_maildir_message_entry {
    maildir_message  msg;
   LIST_ENTRY(d_maildir_message_entry) entries;
};

typedef struct d_maildir_message_entry maildir_message_entry;
LIST_HEAD(d_maildir_messages_list, d_maildir_message_entry);
typedef struct d_maildir_messages_list d_maildir_messages_list;

//bool pr_maildir_message_init(maildir_message *msg);
///Осовбождение памяти из под структуры
void maildir_message_free(maildir_message *msg);
/// Удаление файла
bool maildir_message_release(maildir_message *msg, err_t *error);
/// Заверешение работы с файлом, после того как вся нужная информация будет записана
/// Выполняет перенос файла из папки tmp в new
bool maildir_message_finalize(maildir_message *msg, err_t *error);

bool maildir_message_get_user(maildir_message *msg, maildir_user **user, err_t *error);
/// Чтение из файла в буфер
bool maildir_message_read(maildir_message *msg, char **buffer);
/// Запись буфера в файл
bool maildir_message_write(maildir_message *msg, const char *buffer, size_t b_len, err_t *error);

#endif //SERVER_MESSAGE_H
