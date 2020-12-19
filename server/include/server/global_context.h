//
// Created by nrx on 05.12.2020.
//

#ifndef SERVER_USER_CONTEXT_H
#define SERVER_USER_CONTEXT_H
#include "sys/queue.h"
#include "smtp/state.h"
#include "vector_structures.h"
#include "maildir/maildir.h"
#include "event_loop/event_loop.h"
#include "server/users_list.h"
#include "server/timers.h"
#include "util.h"
#include <pthread.h>
#include <libconfig.h>

#define POSTMAN_VERSION_MAJOR 0
#define POSTMAN_VERSION_MINOR 1
#define POSTMAN_TIMEOUT_OF_TIMER 15


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
} server_config;

struct pair {
    char *buffer;
    smtp_status status;
};

bool server_config_init(const char *path);
void server_config_free();


void user_disconnected(int sock);
void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error);
void handler_write(event_loop *el, int socket, char* buffer, int size, int writing, client_status status, err_t error);
void handler_read(event_loop *el, int socket, char *buffer, int size, client_status status, err_t error);
void handler_timer(event_loop*, int socket, struct timer_event_entry *descriptor);
void handler_close_socket(event_loop*, int sock, err_t *err);
struct pair handler_smtp(user_context *user, char *message);
#endif //SERVER_USER_CONTEXT_H
