#ifndef SMTP_EVENT_LOOP_H
#define SMTP_EVENT_LOOP_H

#include "buffered_queue.h"

#include <stdbool.h>
#include <sys/queue.h>
#include <pthread.h>

typedef enum _event_type {
    SOCK_ACCEPT,
    SOCK_READ,
    SOCK_WRITE,
    SOCK_TIMER,
    NONE,
} event_type;

typedef  enum _error_type  {
    NO_ERROR, ERROR
} error_type;

typedef struct _async_error {
    error_type is;
    const char *message;
} async_error;

typedef struct _event_loop event_loop;
typedef  void (*sock_accept_handler)(event_loop*, int socket, async_error);
typedef  void (*sock_read_handler)(event_loop*, int socket, char *buffer, async_error);
typedef  void (*sock_write_handler)(event_loop*, int socket, int size, async_error);
typedef void (*sock_timer_handler)(event_loop*, int socket, unsigned int time, async_error);
typedef void (*buff_deleter)(void *buffer, int bsize);

typedef struct _event_t {
    event_type type;
    int socket;
} event_t;

typedef struct _event_sock_accept {
    event_t event;
    sock_accept_handler handler;
} event_sock_accept;

typedef struct _event_sock_read {
    event_t event;
    sock_read_handler handler;
    bf_queue  *buffer;
    int size;
    int offset;
} event_sock_read;

typedef struct _event_sock_write {
    event_t event;
    sock_write_handler handler;
    int offset; // возможна не полная отправка данных за раз. Это смещенеи в буфере
    int size;
    char *buffer;
    buff_deleter deleter;

} event_sock_write;

typedef struct _events_entry {
    event_t *event;
    TAILQ_ENTRY(_events_entry) entries;
} events_entry;

TAILQ_HEAD(_events_queue,  _events_entry);
typedef struct _events_queue events_queue;

typedef struct _registered_events {
    int sock;
    events_queue *events;
} registered_events;


typedef struct _registered_events_entry {
    registered_events sock_events;
    TAILQ_ENTRY(_registered_events_entry) entries;
} registered_events_entry;

TAILQ_HEAD(_registered_events_queue, _registered_events_entry);
typedef struct _registered_events_queue registered_events_queue;


typedef struct _occurred_event {
    event_t event;

} occurred_event;

typedef struct _occurred_event_entry {
    occurred_event element;
    TAILQ_ENTRY(_occurred_event_entry) entries;
} occurred_event_entry;

TAILQ_HEAD(_event_queue_t, _occurred_event_entry);
typedef struct _event_queue_t event_queue_t;


typedef struct _event_loop {
    registered_events_queue *_sock_events; // Зарегистрированные обработчики событий
    event_queue_t *_event_queue; // Произошедшие события
    pthread_t _thread; // поток менеджера событий
    pthread_mutex_t _mutex_event_queue; // Защита очереди произошедших событий
    pthread_mutex_t _mutex_sock_events; // Защита списка зарегистрированных обработчиков событий
    pthread_mutex_t _mutex_is_run; // Защита флага is_run
    int _is_run; // флаг работы менеджера. Если 1 то менеджер работает.
} event_loop;


void el_init(event_loop*);
/**
 * Инициализация цикла событий.
 * @param is_in_thread true - Запуск менеджера цикла событий в текущем потоке.
 *  В этом случае предполагается однопоточная обработка. Регистрация событий из других потоков
 *  будет потоко не безопасным. Менеджер сам будет вызывать обработчики
 *  false - Запуск цикла событий в отдельном потоке. Предполагается многопоточная работа.
 *  Менеджер не будет вызывать обработчики событий. Для обработки возникшех событий в отдельных потоках
 *  необходимо вызывать функцию run на данном цикле событией. В этом случае работа потокобезопасная.
 */
int el_open(event_loop*, bool is_in_thread);
void el_close(event_loop*);
bool el_run(event_loop*);
void el_async_accept(event_loop* loop, int sock, sock_accept_handler);
void el_async_read(event_loop* loop, int sock, sock_read_handler);
void el_async_write(event_loop* loop, int sock, void *output_buffer, unsigned int bsize, buff_deleter, sock_read_handler);
void el_timer(event_loop* loop, int sock, unsigned int ms, sock_timer_handler);
#endif //SMTP_EVENT_LOOP_H
