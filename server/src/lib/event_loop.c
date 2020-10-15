#include "event_loop/event_loop.h"
#include "protocol/buffered_queue.h"
#include "error_t.h"
#include "util.h"

#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include<fcntl.h>

#include <assert.h>

#define TIMEOUT_ZERO 0
#define POLL_TIMEOUT 0
#define TIMEOUT_INF (-1)

static const char ERROR_MESSAGE_NO_ERROR[] = "Successful";
static const char ERROR_MESSAGE_WRITE_ERROR[] = "Error write data in socket";
const char EL_ERROR_MUTEX_INIT[] = "error mutex init";
const char EL_ERROR_MUTEX[] = "error mutex";
const char EL_EVENT_LOOP_PTR_IS_NULL[] = "event loop ptr is null";
const char EL_INVALID_EVENT_TYPE[] = "invalid event type";
const char EL_POLLFD_ERROR[] = "error execute poll";
const char EL_NO_EVENTS[] = "not occurred events";



#define PTHREAD_CHECK(_pthread_res_, _error_)  \
do {                                           \
    if ((_pthread_res_) != 0) {                \
        if ((_error_) != NULL) {               \
        (_error_)->error = FATAL;              \
        (_error_)->message = EL_ERROR_MUTEX;   \
        }                                      \
        return false;                                \
    }                                          \
} while(0)


bool loop_is_started(event_loop* loop, error_t *error) {
    int res = 0;
    ERROR_SUCCESS(error);
    PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_is_run), error);
    res = loop->_is_run;
    PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_is_run), error);
    return res;
}



int req_length(registered_events_queue *queue) {
    int i = 0;
    registered_events_entry *ptr = NULL;
    TAILQ_FOREACH(ptr, queue, entries) {
        i = i + 1;
    }
    return i;
}

int sq_length(sockets_queue *queue) {
    int i = 0;
    socket_entry *ptr = NULL;
    TAILQ_FOREACH(ptr, queue, entries) {
        i = i + 1;
    }
    return i;
}

int queue_length_bf(bf_queue *queue) {
    int i = 0;
    for(bf_queue_entry *ptr = queue->tqh_first;
        ptr != NULL;
        ptr = ptr->entries.tqe_next) {
        i = i + 1;
    }
    return i;
}

void pr_manager_step(event_loop *loop, int fd_timeout, error_t *error) {
    // TODO (ageev) использовать флаг для определния, что зарегистрированне события не изменились
    //  и следовательно перестроение pollfd[] не требуется.
    /////////Псотроение массива pollfd//////////////////////
    struct pollfd *fd_array = NULL;
    int size = 0;
    pr_create_pollfd(loop, &fd_array, &size, error);
    /////////////////////////////////////////////////////////
//
    int poll_res = poll(fd_array, size, fd_timeout); // TODO (ageev) установить таймаут. Тайматут должен браться из конфигурации
    if (poll_res == POLL_TIMEOUT) {
        goto exit;
    }

    if (poll_res < 0) {
        // TODO (ageev) сделать обработку ошибки через callback
        error->error = FATAL;
        error->message = EL_POLLFD_ERROR;
       goto exit;
    }
    for(int index = 0; index < size; index++) {
        if (fd_array[index].revents == POLLIN) {
            pr_create_pollin_event(loop, fd_array, index, error);
        } else if (fd_array[index].revents == POLLOUT) {
            pr_create_pollout_event(loop, fd_array, index, error);
        }
    }
exit:
    free(fd_array);
}

void *loop_thread(void *args) {
    error_t error;
    ERROR_SUCCESS(&error);
    event_loop *loop = (event_loop*) args;
//    int timeout = 500; // 0.5 sec
    loop->_is_run = true;
    while (loop_is_started(loop, &error)) {
        pr_manager_step(loop, TIMEOUT_INF, &error);
        if (error.error) {
            break;
        }
    }
    return NULL;
}

event_loop* el_init(error_t *error) {
    ERROR_SUCCESS(error);
    event_loop *loop = s_malloc(sizeof(event_loop), error);
    if (loop == NULL) {
        return NULL;
    }

    loop->_registered_events = NULL;
    loop->_acceptors_queue = NULL;
    loop->_occurred_events = NULL;
    error_t err;
    ERROR_SUCCESS(&err);
    loop->_registered_events = req_init(&err);
    if (err.error) {
        goto error_exit;
    }

    loop->_acceptors_queue= sq_init(&err);
    if (err.error) {
        goto error_exit;
    }

    loop->_occurred_events = oeq_init(&err);
    if (err.error) {
        goto error_exit;
    }

    if (pthread_mutex_init(&loop->_mutex_occurred_events, NULL) != 0) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = EL_ERROR_MUTEX_INIT;
        }
        goto error_exit;
    }

    if (pthread_mutex_init(&loop->_mutex_registered_events, NULL) != 0) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = EL_ERROR_MUTEX_INIT;
        }
        goto error_exit;
    }

    if (pthread_mutex_init(&loop->_mutex_is_run, NULL) != 0) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = EL_ERROR_MUTEX_INIT;
        }
        goto error_exit;
    }

    if (pthread_mutex_init(&loop->_mutex_acceptors_queue, NULL) != 0) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = EL_ERROR_MUTEX_INIT;
        }
        goto error_exit;
    }



    return loop;

    error_exit:
    el_close(loop);
    return NULL;
}


void pr_el_close(event_loop *loop) {
    pthread_mutex_destroy(&loop->_mutex_occurred_events);
    pthread_mutex_destroy(&loop->_mutex_registered_events);
    pthread_mutex_destroy(&loop->_mutex_is_run);
    pthread_mutex_destroy(&loop->_mutex_acceptors_queue);
    req_free(loop->_registered_events);
    sq_free(loop->_acceptors_queue);
    oeq_free(loop->_occurred_events);
    free(loop);
}

void el_close(event_loop *loop) {
    // TODO (ageev) Необходио закрыть все сокеты
    if (loop != NULL) {
        el_stop(loop, NULL);
        pr_el_close(loop);
    }
}

bool el_stop(event_loop* loop, error_t *error) {
    ERROR_SUCCESS(error);
    if (loop == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = EL_EVENT_LOOP_PTR_IS_NULL;
        }
        return false;
    }

    PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_is_run), error);

    loop->_is_run = false;

    PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_is_run), error);

    return true;
}

bool el_open(event_loop* loop, work_mode mode, error_t *error_) {
    ERROR_SUCCESS(error_);
    if (loop == NULL) {
        if (error_ != NULL) {
            error_->error = FATAL;
            error_->message = EL_EVENT_LOOP_PTR_IS_NULL;
        }
        return false;
    }
    PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_is_run), error_);
    loop->_is_run = true;
    PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_is_run), error_);
    if (mode == ONE_THREAD) {
        error_t  error;
        ERROR_SUCCESS(&error);

        while (loop_is_started(loop, &error)) {
            pr_manager_step(loop, TIMEOUT_ZERO, &error);
            if (error.error) {
                if (error_ != NULL) {
                    *error_ = error;
                }
                return false;
            }
            el_run(loop, &error);
            if (error.error && error.error != NOT_FOUND) {
                if (error_ != NULL) {
                    *error_ = error;
                }
                return false;
            }
        }
    } else if (mode == OWN_THREAD) {
        return pthread_create(&loop->_thread, NULL, loop_thread, loop);
    }
    return true;
}

void pr_sock_accept_execute(event_loop *loop, event_sock_accept *event) {
    event->handler(loop, event->event.socket, event->client_socket, event->client_addr, event->event.error);
}

void pr_sock_read_execute(event_loop *loop, event_sock_read *event) {
    event->handler(loop, event->event.socket, event->buffer, event->offset, event->event.error);
}

void pr_sock_write_execute(event_loop *loop, event_sock_write *event) {
        event->handler(loop, event->event.socket, event->buffer, event->size, event->offset, event->event.error);
}


bool el_run(event_loop* loop, error_t *error) {
    error_t err;
    ERROR_SUCCESS(&err);
    event_t *event = NULL;
    error_t err_queue;
    ERROR_SUCCESS(&err_queue);

    PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_occurred_events), error);
    if (TAILQ_EMPTY(loop->_occurred_events)) {
        if (error != NULL) {
            error->error = NOT_FOUND;
            error->message = EL_NO_EVENTS;
        }
        pthread_mutex_unlock(&loop->_mutex_occurred_events);
        return false;
    }
    oeq_pop_begin(loop->_occurred_events, &event, &err_queue);
    PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_occurred_events), error);

    if (err_queue.error) {
        // TODO (ageev) ERROR
    }

    switch (event->event.type) {
        case SOCK_ACCEPT:
            pr_sock_accept_execute(loop, (event_sock_accept*)event);
            break;
        case SOCK_READ:
            pr_sock_read_execute(loop, (event_sock_read*) event);
            break;
        case SOCK_WRITE:
            pr_sock_write_execute(loop, (event_sock_write*) event);
            break;
    }
    free(event);
    return true;
}


bool el_async_accept(event_loop* loop, int sock, sock_accept_handler handler, error_t *error) {
   ERROR_SUCCESS(error);
   pthread_mutex_lock(&loop->_mutex_acceptors_queue);
   sq_add(loop->_acceptors_queue, sock, handler, error);
   pthread_mutex_unlock(&loop->_mutex_acceptors_queue);
    // TODO (ageev) ERROR
   return true;
}

bool el_async_read(event_loop* loop, int sock, char *buffer, int size, sock_read_handler handler, error_t *error) {
    error_t err;
    ERROR_SUCCESS(&err);
    event_sock_read *read = s_malloc(sizeof(event_sock_read), &err);
    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    ERROR_SUCCESS(&read->event.error);
    read->event.socket = sock;
    read->event.type = SOCK_READ;
    read->handler = handler;
    read->size = size;
    read->buffer = buffer;

    pthread_mutex_lock(&loop->_mutex_registered_events);
    req_push_read(loop->_registered_events, sock, read, error);
    pthread_mutex_unlock(&loop->_mutex_registered_events);
    // TODO (ageev) ERROR
    return true;
}

bool el_async_write(event_loop* loop, int sock, void *output_buffer, int bsize,
                    sock_write_handler handler,  error_t *error) {
    ERROR_SUCCESS(error);
    event_sock_write *write = s_malloc(sizeof(event_sock_write), error);
    if (write == NULL) {
        return false;
    }
    ERROR_SUCCESS(&write->event.error);
    write->event.type = SOCK_WRITE;
    write->event.socket = sock;
    write->buffer = output_buffer;
    write->size = bsize;
    write->handler = handler;

    pthread_mutex_lock(&loop->_mutex_registered_events);
    bool is_res = req_push_write(loop->_registered_events, sock, write, error);
    pthread_mutex_unlock(&loop->_mutex_registered_events);

    return is_res;
}

bool pr_create_pollfd(event_loop* loop, struct pollfd **fd_array, int *size, error_t *error) {
    error_t  err;
    ERROR_SUCCESS(&err);

    PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_registered_events), &err);

    int socket_events_length = req_length(loop->_registered_events);
    int accept_sockets_count = sq_length(loop->_acceptors_queue);
    int fd_size = socket_events_length + accept_sockets_count;
    struct pollfd *fd_tmp = s_malloc(sizeof(struct pollfd)*fd_size, error);
    if (error->error) {
        goto exit;
    }

    int i = 0;
    registered_events_entry *entry = NULL;
    TAILQ_FOREACH(entry, loop->_registered_events, entries) {
        events_entry *eptr = NULL;
        fd_tmp[i].fd = entry->sock_events.sock;
        TAILQ_FOREACH(eptr, entry->sock_events.events, entries) {
            switch (eptr->event->event.type) {
                case SOCK_WRITE:
                    fd_tmp[i].events |= POLLOUT;
                    break;
                case SOCK_READ:
                    fd_tmp[i].events |= POLLIN;
                    break;
                default:
                    err.error = FATAL;
                    error->message = EL_INVALID_EVENT_TYPE;
                    goto exit;
            }
        }
        i += 1;
    }
    //i += 1;

    loop->_index_acepptors_start = i;
    socket_entry *sentry = NULL;
    TAILQ_FOREACH(sentry, loop->_acceptors_queue, entries) {
        fd_tmp[i].fd = sentry->socket;
        fd_tmp[i].events = POLLIN;
    }

    exit:
    if(error != NULL) {
        *error = err;
    }
    if (err.error) {
        if (fd_tmp != NULL) {
            free(fd_tmp);
            fd_tmp = NULL;
        }
    }
    *fd_array = fd_tmp;
    *size = fd_size;

    PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_registered_events), error);

    return true;
}


// TODO (ageev) разобраться с блокировками
bool pr_create_pollin_event(event_loop *loop, struct pollfd *fd, int index, error_t *error) {
    fd->revents &= ~POLLIN;
    error_t err;
    ERROR_SUCCESS(&err);
    // Проверка: Является ли сокет ацептором
    if (index >= loop->_index_acepptors_start) {
        // Сокет является ацептором
        // TODO (ageev) Нужна ли блокировка для loop->_acceptors_queue и для   loop->_registered_events

        PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_acceptors_queue), error);
        sock_accept_handler handler = sq_get(loop->_acceptors_queue, fd->fd, &err);
        PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_acceptors_queue), error);

        if (err.error) {
            // TODO (ageev) error
        }
        event_sock_accept *occurred = s_malloc(sizeof(event_sock_accept), &err);
        if (error->error) {
            // TODO (ageev) error
        }
        occurred->event.type = SOCK_ACCEPT;
        occurred->event.socket = fd->fd;
        occurred->handler = handler;
        socklen_t addr_len = sizeof(struct sockaddr_in);
        occurred->client_socket = accept(occurred->event.socket,
                                         (struct sockaddr*)&occurred->client_addr,
                                         &addr_len);
        if (occurred->client_socket == -1) {
            // TODO error
            return false;
        } // TODO (ageev) как этот код тестировать!!!
        fcntl(occurred->client_socket, F_SETFL, O_NONBLOCK);

        PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_occurred_events), error);
        oeq_push_back(loop->_occurred_events, (event_t*)occurred, &err);
        PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_occurred_events), error);

    } else {
        // Из этого секета необходимо выплнить чтение
        PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_registered_events), error);
        event_sock_read *ptr = req_pop_read(loop->_registered_events, fd->fd, &err);
        PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_registered_events), error);
        // Вычитывание из сокета по не потребуется блокировка или не будет прочитан до конца буфера
        int res = 0;
        int offset = 0;
        while((res = recv(ptr->event.socket, ptr->buffer + offset, ptr->size - offset, 0)) != -1) {
            offset += res;
            if (res == 0 ) {
                // client disconnect
                break;
            }
        }
        if (errno == EAGAIN) {
            // OK
            PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_occurred_events), error);
            oeq_push_back(loop->_occurred_events, (event_t*) ptr, &err);
            PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_occurred_events), error);
        } else {
            // TODO (ageev) error
        }
    }

    if (error != NULL) {
        *error = err;
    }
    return true;
}

bool pr_create_pollout_event(event_loop *loop, struct pollfd *fd_array, int index, error_t *error) {
    fd_array[index].revents &= ~POLLOUT;
    error_t  err;
    ERROR_SUCCESS(&err);

    PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_registered_events), error);
    event_sock_write *ptr = req_pop_write(loop->_registered_events, fd_array[index].fd, &err);
    PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_registered_events), error);

    int res = 0;
    int old_offset = ptr->offset;
    while ((res = send(ptr->event.socket, ptr->buffer + ptr->offset, ptr->size - ptr->offset, MSG_NOSIGNAL)) != -1) {
        if (res == 0) {
            break;
        }
        ptr->offset += res;
    }
    if ((errno == EAGAIN) && (ptr->offset == 0)) {

        PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_registered_events), error);
        req_push_write(loop->_registered_events, ptr->event.socket, ptr, &err);
        PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_registered_events), error);

        if (err.error) {
            // TODO (ageev) error
        }
    } else if (ptr->offset != old_offset) {

        PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_occurred_events), error);
        oeq_push_back(loop->_occurred_events, (event_t*)ptr, &err);
        PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_occurred_events), error);

        if (err.error) {
            // TODO (ageev) error;
        }
    } else {
        // TODO (ageev) error;
    }
    return true;
}