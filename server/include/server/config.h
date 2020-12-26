//
// Created by nrx on 20.12.2020.
//

#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include "smtp/state.h"
#include "vector_structures.h"
#include "maildir/maildir.h"
#include "event_loop/event_loop.h"
#include "server/users_list.h"
#include "server/timers.h"
#include <libconfig.h>

#define POSTMAN_VERSION_MAJOR 0
#define POSTMAN_VERSION_MINOR 1

struct server_configuration {
    maildir md;                 /// Описатель maildir
    char *ip;                   /// ip адрес сервера
    int16_t  port;              /// порт сервера
    char *log_file_path;        /// Путь до конфигурационного файла
    char *self_server_name;     /// домен данноо сервера
    struct users_list users;    /// список контекство подключенных в данный момент пользователей
    char *hello_msg;            /// 220 ответ сервера при подключении клиента
    size_t hello_msg_size;      /// длина 220 ответа
    event_loop *loop;           /// цикл событий сервера
    size_t num_worker_threads;  /// число рабочих потоков
    timers_t timers;            /// таймеры подключенных пользователей
    int timer_period;           /// время истечение таймера
    char conf_path[NAME_MAX];   /// путь до файла конфигурации
    int nice_value;             /// уровень вежливости процесса
} server_config;

struct pair {
    char *buffer;
    smtp_status status;
};

bool server_config_init(const char *path);
void server_config_free();

#endif //SERVER_CONFIG_H
