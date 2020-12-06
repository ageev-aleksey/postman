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
#include "util.h"
#include <pthread.h>
#include <libconfig.h>

#define POSTMAN_VERSION_MAJOR 0
#define POSTMAN_VERSION_MINOR 1

typedef struct user_context {
    smtp_state smtp;
    vector_char  buffer;
    int socket;
    client_addr addr;
} user_context;

typedef struct user_context_entry {
    user_context *pr_context;
    TAILQ_ENTRY(user_context_entry) pr_entries;
} user_context_entry;

TAILQ_HEAD(users_tailq_list, user_context_entry);
typedef struct users_list {
    struct users_tailq_list pr_list;
    pthread_mutex_t pr_mutex;
} users_list;

typedef struct user_accessor {
    // PUBLIC
    user_context *user;
    // PRIVATE
    user_context_entry *pr_list_entry;
    users_list *pr_users_list;
} user_accessor;


struct {
    maildir md;
    char *ip;
    int16_t  port;
    char *log_file_path;
    char *self_server_name;
    struct users_list users;
    char *hello_msg;
    size_t hello_msg_size;
} global_config_server;

bool server_config_init(const char *path);
void server_config_free();

/**
 * Инициализация контекста пользователя
 * @param context - контекст
 * @param addr - системная стркутура описывющая адрес подключившегося пользователя
 * @param socket - сокет, по которому подключился пользователь
 * @return статус выполнения операции
 */
bool user_init(user_context *context, struct sockaddr_in *addr, int socket);

/**
 * Освобожление ресурсов из под контекста пользователя
 * @param context
 */
void user_free(user_context *context);



/**
 * Инициализация списка содержащий контексты пользователей
 * @param users - список
 * @return Успешность операции
 */
bool users_list__init(users_list *users);

/**
 * Освобождение всех ресуросв из под списка контекстов.
 * Оставшиеся контексты пользователей, так же будут удалены
 * @param users
 * @return
 */
void users_list__free(users_list *users);
/**
 * Добавление контекста пользователя в список
 * @param users список
 * @param user контекст
 * @return успешность операции
 * @remark thread safe
 */
bool users_list__add(users_list *users, user_context **user);
/**
 * Поск пользователя по сокету
 * @param users список
 * @param sock сокет
 * @return true - если контекст был найден
 * @remark thread safe
 */
bool users_list__user_find_by_sock(users_list *users, user_accessor *accessor, int sock);
/**
 * Особождение контекста из под уникального доступа.
 * После вызова accessor становится не валидный. Для доступа к данным
 * необходимо воспользоваться соответсвующей функцией получения доступа к
 * элементу списка
 * @see users_list__user_find_by_sock
 * @param accessor
 */
void user_accessor_release(user_accessor *accessor);
/**
 * Удаление пользователя из списка
 * (ни какие ресурсы не освобождаются)
 * @param accessor
 */
void users_list__delete_user(user_accessor *accessor);



void user_disconnected(int sock);
void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error);
void handler_write(event_loop *el, int socket, char* buffer, int size, int writing, client_status status, err_t error);
void handler_read(event_loop *el, int socket, char *buffer, int size, client_status status, err_t error);
void handler_timer(event_loop*, int socket, struct timer_event_entry *descriptor);
#endif //SERVER_USER_CONTEXT_H
