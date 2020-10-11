#include "event_loop/event_loop.h"
#include "event_loop/buffered_queue.h"
#include "event_loop/error_t.h"
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

#define POLL_TIMEOUT 0


static const char ERROR_MESSAGE_NO_ERROR[] = "Successful";
static const char ERROR_MESSAGE_WRITE_ERROR[] = "Error write data in socket";
const char EL_ERROR_MUTEX_INIT[] = "error mutex init";
const char EL_ERROR_MUTEX[] = "error mutex";
const char EL_EVENT_LOOP_PTR_IS_NULL[] = "event loop ptr is null";
const char EL_INVALID_EVENT_TYPE[] = "invalid event type";
const char EL_POLLFD_ERROR[] = "error execute poll";



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

void *loop_thread(void *args) {
    error_t error;
    ERROR_SUCCESS(&error);
    event_loop *loop = (event_loop*) args;
//    int timeout = 500; // 0.5 sec
    loop->_is_run = true;
    while (loop_is_started(loop, &error)) {
        // TODO (ageev) использовать флаг для определния, что зарегистрированне события не изменились
        //  и следовательно перестроение pollfd[] не требуется.
        /////////Псотроение массива pollfd//////////////////////
        struct pollfd *fd_array = NULL;
        int size = 0;
        pr_create_pollfd(loop, &fd_array, &size, &error);
        /////////////////////////////////////////////////////////
//
        int poll_res = poll(fd_array, size, -1); // TODO (ageev) установить таймаут. Тайматут должен браться из конфигурации
        if (poll_res == POLL_TIMEOUT) {
            continue;
        }

        if (poll_res < 0) {
            // TODO (ageev) сделать обработку ошибки через callback
            error.error = FATAL;
            error.message = EL_POLLFD_ERROR;
            if(fd_array != NULL) {
                free(fd_array);
            }
            break;
        }

        for(int index = 0; index < size; index++) {
            if (fd_array[index].revents == POLLIN) {
                pr_create_pollin_event(loop, fd_array, index, size);
            } else if (fd_array[index].revents == POLLOUT) {
                pr_create_pollout_event(loop, fd_array, index, size);
            }
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

    loop->_sock_events = NULL;
    loop->_sockets_accepts = NULL;
    loop->_event_queue = NULL;
    error_t err;
    ERROR_SUCCESS(&err);
    loop->_sock_events = req_init(&err);
    if (err.error) {
        goto error_exit;
    }

    loop->_sockets_accepts= sq_init(&err);
    if (err.error) {
        goto error_exit;
    }

    loop->_event_queue = oeq_init(&err);
    if (err.error) {
        goto error_exit;
    }

    if (pthread_mutex_init(&loop->_mutex_event_queue, NULL) != 0) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = EL_ERROR_MUTEX_INIT;
        }
        goto error_exit;
    }

    if (pthread_mutex_init(&loop->_mutex_sock_events, NULL) != 0) {
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

    return loop;

error_exit:
    el_close(loop);
    return NULL;
}


void pr_el_close(event_loop *loop) {
    pthread_mutex_destroy(&loop->_mutex_event_queue);
    pthread_mutex_destroy(&loop->_mutex_sock_events);
    pthread_mutex_destroy(&loop->_mutex_is_run);
    req_free(loop->_sock_events);
    sq_free(loop->_sockets_accepts);
    oeq_free(loop->_event_queue);
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

bool el_open(event_loop* el, bool is_in_thread, error_t *error) {
    ERROR_SUCCESS(error);
    if (el == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = EL_EVENT_LOOP_PTR_IS_NULL;
        }
        return false;
    }


    //   if (is_in_thread) {
//       loop_thread(el);
//       return 1;
//   } else {
//      return pthread_create(&el->_thread, NULL, loop_thread, el);
//   }

}



bool el_run(event_loop* el) {
//    pthread_mutex_lock(&el->_mutex_event_queue);
//
//    occurred_event_entry  *etype = *el->_event_queue->tqh_last;
//    TAILQ_REMOVE(el->_event_queue, etype, entries);
//
//    pthread_mutex_unlock(&el->_mutex_event_queue);
//
//    if (etype->element.event->event.type == SOCK_WRITE) {
//        event_sock_write *e = (event_sock_write*) etype->element.event;
//        int res = send(e->event.event.socket, &e->buffer[e->offset], e->size - e->offset, 0);
//        if (res == -1) {
//            // Ошибка отправки буфера
//            async_error error;
//            error.is = ERROR;
//            error.message = ERROR_MESSAGE_WRITE_ERROR;
//            e->handler(el, e->event.event.socket, e->offset, error);
//            e->deleter(e->buffer, e->size);
//        } else if (res < (e->size - e->offset)) {
//            // TODO(ageev) отправка произведена не полностью
//            // _el_async_write(el, e);
//        } else {
//            // все отправилось вызываем обработчик
//            async_error error;
//            error.is = NO_ERROR;
//            error.message = ERROR_MESSAGE_NO_ERROR;
//            e->handler(el, e->event.event.socket, e->size, error);
//            e->deleter(e->buffer, e->size);
//        }
//    } else if (etype->element.event->event.type == SOCK_READ) {
//        _socket_read(el, etype);
//    } else if (etype->element.event->event.type == SOCK_ACCEPT) {
//        _socket_accept(el, etype);
//    } else if (etype->element.event->event.type == SOCK_TIMER) {
//        // TODO (ageev) вызов обработчика для конкретного сокета по таймеру.(?!)
//    }
//    free(etype);
//    return 1;
}


void el_async_accept(event_loop* loop, int sock, sock_accept_handler handler) {
//    pthread_mutex_lock(&loop->_mutex_sock_events);
//    _el_async_accept(loop, sock, handler);
//    pthread_mutex_unlock(&loop->_mutex_sock_events);
}

// TODO (ageev) перепроверить реализацию функции
void pr_el_async_accept(event_loop* loop, int sock, sock_accept_handler handler) {
    // Зарегистрирован ли данный сокет в цилке событий
    // Соект, который выполняет прием соединений находится отдельно от остальных собыйтий и не удаляется
    // пока явно не будет запрос на отключение приемника
//    registered_events *event = NULL;
//    for (registered_events_entry *ptr = loop->_sock_events->tqh_first;
//        ptr != NULL;
//        ptr = ptr->entries.tqe_next)
//    {
//        if (ptr->sock_events.sock == sock) {
//            event = &ptr->sock_events;
//            break;
//        }
//    }
//    // если нет то регистрируем сокет
//    if (!event) {
//        registered_events_entry  *registered_event = malloc(sizeof(registered_events_entry));
//        registered_event->sock_events.sock = sock;
//        registered_event->sock_events.events = malloc(sizeof(events_queue));
//        TAILQ_INIT(registered_event->sock_events.events);
//        event = &registered_event->sock_events;
//        TAILQ_INSERT_TAIL(loop->_sock_events, registered_event, entries);
//    }
//
//    //регистрируем событие для сокета
//    event_sock_accept *accept_event = malloc(sizeof(event_sock_accept));
//    accept_event->event.type = SOCK_ACCEPT;
//    accept_event->event.socket = sock;
//    accept_event->handler = handler;
//    events_entry *e = malloc(sizeof(events_entry));
//    e->event = (event_t*)accept_event;
//    TAILQ_INSERT_TAIL(event->events, e, entries);

    // добавление сокета в список сокетов ожидающиз подключения
//    socket_entry *se = malloc(sizeof(socket_entry));
//    TAILQ_INSERT_TAIL(loop->_sockets_accepts, se, entries);
}

void el_async_read(event_loop* loop, int sock, char *buffer, int size, sock_read_handler handler) {
//    event_sock_read  *event = (event_sock_read*)malloc(sizeof(event_sock_read));
//    event->event.event.socket = sock;
//    event->event.event.type = SOCK_READ;
//    event->size = size;
//    event->buffer = buffer;
//    event->handler = handler;
//    event->offset = 0;
//
//    occurred_event_entry *occurred = malloc(sizeof(occurred_event_entry));
//    occurred->element.event = (event_t*) event;
//    _el_async_read(loop, occurred);

}

void pr_el_async_read(event_loop *loop, occurred_event_entry *occurred) {
//    pthread_mutex_lock(&loop->_mutex_event_queue);
//
//    TAILQ_INSERT_TAIL(loop->_event_queue, occurred, entries);
//
//    pthread_mutex_unlock(&loop->_mutex_event_queue);
}


void pr_socket_read(event_loop *loop, occurred_event_entry *occurred) {
//    event_sock_read *e = (event_sock_read*)occurred->element.event;
//    int res = 0;
//    while((res = recv(e->event.event.socket, e->buffer + e->offset, e->size - e->offset, MSG_NOSIGNAL)) != -1) {
//        if (res == 0) {
//            async_error error;
//            error.is = NO_ERROR;
//            error.message = ERROR_MESSAGE_NO_ERROR;
//            e->handler(loop, e->event.event.socket, e->buffer, e->offset, error);
//        }
//        e->offset += res;
//    }
//    if (errno == EAGAIN) {
//        _el_async_read(loop, occurred);
//    }

}

void pr_socket_accept(event_loop *loop, occurred_event_entry *occurred) {
//    // подключение клиента
//    event_sock_accept *e = (event_sock_accept*)occurred->element.event;
//    struct sockaddr_in client_addr;
//    memset(&client_addr, 0, sizeof(struct sockaddr_in));
//    socklen_t addr_len = sizeof(struct sockaddr_in);
//    // TODO (ageev) Обработка ошибко принятия подключений
//    int slave = accept(e->event.event.socket, (struct sockaddr*)&client_addr, &addr_len);
//    async_error error;
//    error.is = NO_ERROR;
//    error.message = ERROR_MESSAGE_NO_ERROR;
//    e->handler(loop, e->event.event.socket, slave, client_addr, error);
//    socket_entry *s_entry = malloc(sizeof(socket_entry));
//    s_entry->socket = e->event.event.socket;
//    s_entry->handler = e->handler;
//    TAILQ_INSERT_TAIL(loop->_sockets_accepts, s_entry, entries);
}

bool pr_create_pollfd(event_loop* loop, struct pollfd **fd_array, int *size, error_t *error) {
    error_t  err;
    ERROR_SUCCESS(&err);

    PTHREAD_CHECK(pthread_mutex_lock(&loop->_mutex_sock_events), &err);

    int socket_events_length = req_length(loop->_sock_events);
    int accept_sockets_count = sq_length(loop->_sockets_accepts);
    int fd_size = socket_events_length + accept_sockets_count;
    struct pollfd *fd_tmp = s_malloc(sizeof(struct pollfd)*fd_size, error);
    if (error->error) {
        goto exit;
    }

    int i = 0;
    registered_events_entry *entry = NULL;
    TAILQ_FOREACH(entry, loop->_sock_events, entries) {
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
    i += 1;

    loop->_index_acepptors_start = i;
    socket_entry *sentry = NULL;
    TAILQ_FOREACH(sentry, loop->_sockets_accepts, entries) {
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

    PTHREAD_CHECK(pthread_mutex_unlock(&loop->_mutex_sock_events), error);

    return true;
}


// TODO (ageev) разобраться с блокировками
bool pr_create_pollin_event(event_loop *loop, struct pollfd *fd, int index, error_t *error) {
    fd->revents &= !POLLIN;
    error_t err;
    ERROR_SUCCESS(&err);
    // Проверка: Является ли сокет ацептором
    if (index >= loop->_index_acepptors_start) {
        // Сокет является ацептором
        sock_accept_handler handler = sq_get(loop->_sockets_accepts, fd->fd, &err);
        if (err.error) {
            // TODO error
        }
        event_sock_accept *occurred = s_malloc(sizeof(event_sock_accept), &err);
        if (error->error) {
            // TODO error
        }
        occurred->event.type = SOCK_ACCEPT;
        occurred->event.socket = fd->fd;
        occurred->handler = handler;
        occurred->client_socket = accept(occurred->event.socket,
                                             (struct sockaddr*)&occurred->client_addr,
                                                     NULL);
        fcntl(fd->fd, F_SETFL, O_NONBLOCK);
        // <БЛОКИРОВКА>
        oeq_push_back(loop->_event_queue, (event_t*)occurred, &err);
        // </БЛОКИРОВКА>
    } else {
        // Из этого секета необходимо выплнить чтение
        event_sock_read *ptr = req_pop_read(loop->_sock_events, fd->fd, &err);
        // Вычитывание из сокета до тех пор, пока это возможно
        //TODO Проинициализировать bufferd_queue, вычитать до EAGAIN туда, и затем положить в очередь произошедших событий
        recv(ptr->event.socket, ptr->buffer, ptr->size, 0);
        
    }

//    fd_array[index].revents &= !POLLIN;
//    occurred_event_entry *occurred = malloc(sizeof(occurred_event_entry));
//    // ПРоверка: Ожидает ли сокет подключения
//    if (index >= loop->_index_acepptors_start) {
//        // Создаем событие - подключение клиента
//        occurred->element.event = (event_t*)malloc(sizeof(event_sock_accept));
//        occurred->element.event->event.type = SOCK_ACCEPT;
//        occurred->element.event->event.socket = fd_array[index].fd;
//        // выполняем поиск описания зарегистрированного события
//        socket_entry *se_ptr = NULL;
//        TAILQ_FOREACH(se_ptr, loop->_sockets_accepts, entries) {
//            if (se_ptr->socket == fd_array[index].fd) {
//                break;
//            }
//        }
//
//        assert(se_ptr != NULL);
//
//        // извлекаем обработчик для данного события
//        ((event_sock_accept*)occurred->element.event)->handler = se_ptr->handler;
//        TAILQ_INSERT_TAIL(loop->_event_queue, occurred, entries);
//        // убираем событие из списка, чтобы poll не реагировал на то, что мы еще не обработали его
//        // после обработки подключения, необходимо обратно его добавить!
//        TAILQ_REMOVE(loop->_sockets_accepts, se_ptr, entries);
//        free(se_ptr);
//    } else {
//        // Создаем событие - чтение из сокета
//        event_sock_read *e= (event_sock_read*)malloc(sizeof(event_sock_read));
//        e->event.event.type = SOCK_READ;
//        e->event.event.socket = fd_array[index].fd;
//        // TODO (ageev) заменить список на динамический массив!
//        registered_events_entry *re_ptr = NULL; // поиск группы событий для сокета
//        TAILQ_FOREACH(re_ptr, loop->_sock_events, entries) {
//            if (re_ptr->sock_events.sock == fd_array[index].fd) {
//                break;
//            }
//        }
//
//        assert(re_ptr != NULL);
//
//        // поиск зарегистрированного события для сокета
//        // Событие должно существовать. Так на основе него строился fd_array
//        events_entry *ee_ptr = NULL;
//        TAILQ_FOREACH(ee_ptr, re_ptr->sock_events.events, entries) {
//            if (ee_ptr->event->event.type == SOCK_READ) {
//                break;
//            }
//        }
//
//        assert(ee_ptr != NULL);
//
//        e->handler = ((event_sock_read*)(ee_ptr->event))->handler;
//        e->size = ((event_sock_read*)(ee_ptr->event))->size;
//        occurred->element.event = (event_t*)e;
//        TAILQ_INSERT_TAIL(loop->_event_queue, occurred, entries);
//
//        TAILQ_REMOVE(re_ptr->sock_events.events, ee_ptr, entries);
//        free(ee_ptr);
//        if (TAILQ_EMPTY(re_ptr->sock_events.events)) {
//            TAILQ_REMOVE(loop->_sock_events, re_ptr, entries);
//            free(re_ptr);
//        }
//    }
}

void pr_create_pollout_event(event_loop *loop, struct pollfd *fd_array, int index, int size) {
    // TODO (ageev) написать создание события записи в сокет.
}