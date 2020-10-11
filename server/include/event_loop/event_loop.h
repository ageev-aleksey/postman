#ifndef SMTP_EVENT_LOOP_H
#define SMTP_EVENT_LOOP_H

#include "event_t.h"
#include "sockets_queue.h"
#include "registered_events_queue.h"
#include "occurred_event_queue.h"

#include "buffered_queue.h"

#include <stdbool.h>
#include <sys/queue.h>
#include <pthread.h>
#include <poll.h>
#include <netinet/in.h>



typedef  enum _error_type  {
    NO_ERROR, ERROR
} error_type;

typedef struct _async_error {
    error_type is;
    const char *message;
} async_error;




typedef struct _event_loop {
    sockets_queue *_sockets_accepts; // Список сокетов ожидающих принятия подключения
    int _index_acepptors_start; // Индекс в pollfd[] с которого начинаются сокеты-приемщики
    registered_events_queue *_sock_events; // Зарегистрированные обработчики событий
    occurred_event_queue *_event_queue; // Произошедшие события
    pthread_t _thread; // поток менеджера событий
    pthread_mutex_t _mutex_event_queue; // Защита очереди произошедших событий
    pthread_mutex_t _mutex_sock_events; // Защита списка зарегистрированных обработчиков событий
    pthread_mutex_t _mutex_is_run; // Защита флага is_run
    int _is_run; // флаг работы менеджера. Если 1 то менеджер работает.
} event_loop;

//PUBLIC

event_loop* el_init(error_t *error);
/**
 * Инициализация цикла событий.
 * @param is_in_thread true - Запуск менеджера цикла событий в текущем потоке.
 *  В этом случае предполагается однопоточная обработка. Регистрация событий из других потоков
 *  будет потоко не безопасным. Менеджер сам будет вызывать обработчики
 *  false - Запуск цикла событий в отдельном потоке. Предполагается многопоточная работа.
 *  Менеджер не будет вызывать обработчики событий. Для обработки возникшех событий в отдельных потоках
 *  необходимо вызывать функцию run на данном цикле событией. В этом случае работа потокобезопасная.
 */
bool el_open(event_loop*, bool is_in_thread,  error_t *error);
void el_close(event_loop*);
//bool el_run(event_loop*,  error_t *error);
//bool el_async_accept(event_loop* loop, int sock, sock_accept_handler,  error_t *error);
//bool el_async_read(event_loop* loop, int sock, char *buffer, int size, sock_read_handler,  error_t *error);
//bool el_async_write(event_loop* loop, int sock, void *output_buffer, unsigned int bsize,
//                    buff_deleter, sock_read_handler,  error_t *error);
// bool el_timer(event_loop* loop, int sock, unsigned int ms, sock_timer_handler);
bool el_stop(event_loop* loop,  error_t *error);

// PRIVATE
void pr_el_async_accept(event_loop* loop, int sock, sock_accept_handler handler);
void pr_socket_read(event_loop *loop, occurred_event_entry *occurred);
void pr_el_async_read(event_loop *loop, occurred_event_entry *occurred);
void pr_socket_accept(event_loop *loop, occurred_event_entry *occurred);
bool pr_create_pollfd(event_loop* loop, struct pollfd **fd_array, int *size, error_t *error);
bool pr_create_pollin_event(event_loop *loop, struct pollfd *fd, int index, error_t *error);
//void pr_create_pollout_event(event_loop *loop, struct pollfd *fd_array, int index, int size);


#define QUEUE_SIZE(entry_type, queue, field, res) \
     do {                                    \
        int i = 0;                           \
        entry_type *__ptr__ = NULL;              \
         TAILQ_FOREACH(__ptr__, queue, field) {  \
            i = i + 1;                                    \
         }                                        \
        *res = i;                                             \
     } while(0)\

#endif //SMTP_EVENT_LOOP_H
