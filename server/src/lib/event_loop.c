#include "event_loop.h"
#include "buffered_queue.h"

#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>

#define POLL_TIMEOUT 0


static const char ERROR_MESSAGE_NO_ERROR[] = "Successful";
static const char ERROR_MESSAGE_WRITE_ERROR[] = "Error write data in socket";

void _el_async_accept(event_loop* loop, int sock, sock_accept_handler handler);
void _socket_read(event_loop *loop, occurred_event_entry *occurred);
void _el_async_read(event_loop *loop, occurred_event_entry *occurred);
void _socket_accept(event_loop *loop, occurred_event_entry *occurred);

int loop_is_started(event_loop* loop) {
    int res = 0;
    pthread_mutex_lock(&loop->_mutex_is_run);
    res = loop->_is_run;
    pthread_mutex_unlock(&loop->_mutex_is_run);
    return res;
}


#define QUEUE_SIZE(entry_type, queue, field, res) \
     do {                                    \
        int i = 0;                           \
        entry_type *__ptr__ = NULL;              \
         TAILQ_FOREACH(__ptr__, queue, field) {  \
            i = i + 1;                                    \
         }                                        \
        *res = i;                                             \
     } while(0)\

int queue_length_req(registered_events_queue *queue) {
    int i = 0;
    registered_events_entry *ptr = NULL;
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
    event_loop *loop = (event_loop*) args;
//    int timeout = 500; // 0.5 sec
    loop->_is_run = true;
    while (loop_is_started(loop)) {
        // TODO (ageev) использовать флаг для определния, что зарегистрированне события не изменились
        //  и следовательно перестроение pollfd[] не требуется.
        /////////Псотроение массива pollfd//////////////////////
        pthread_mutex_lock(&loop->_mutex_sock_events);

        // Определяем количество зарегестрированных событий
        int number_reg_sockets = 0; // число зарегистрированных сокетов
        QUEUE_SIZE(registered_events_entry, loop->_sock_events,
                   entries, &number_reg_sockets);
        int number_acceptors = 0; // число зарегистриованных приемщиков
        QUEUE_SIZE(socket_entry, loop->_sockets_accepts,
                   entries, &number_acceptors);

        int number_events = 0;
        registered_events_entry *ptr = NULL;
        TAILQ_FOREACH(ptr, loop->_sock_events, entries) {
            int tmp = 0;
            QUEUE_SIZE(registered_events_entry, loop->_sock_events, entries, &tmp);
            number_events += tmp;
        }

        int size = number_events+number_acceptors; // Общее число зарегистрированных событий
        struct pollfd *fd_array = malloc(sizeof(struct pollfd)*(size));
        int i = 0;
        registered_events_entry  *events_ptr = NULL;
        TAILQ_FOREACH(events_ptr, loop->_sock_events, entries) {
            memset(&fd_array[i], 0, size);
            fd_array[i].fd = events_ptr->sock_events.sock;

            // Заполнение pollfd событиями чтения и записи
            events_entry *sock_event = NULL;
            TAILQ_FOREACH(sock_event, events_ptr->sock_events.events, entries) {
                event_t *event = sock_event->event;
               if (event->event.type == SOCK_READ) {
                    fd_array[i].events |= POLLIN;
                } else if (event->event.type == SOCK_WRITE) {
                    // TODO(ageev) создать событие на запись. И положить в очередь событий.
                    //  Данное событие должно быть извлечено из очереди событий очередным вызовом функции run
                    //  События на отправку данных регистрировать не надо. Необходимо сразу создавать событие
                    //  И класть его в очередь.
                }
                i = i + 1;
            }
            // Приемщики записываем в опследний момет, и запоминаем с какого индекса они начианются.
            // Таким образом мы може определить, что события  POLLIN необходимо выполнить приняте подключения а не
            // чтение данных
            loop->_index_acepptors_start = i;
            socket_entry *se_ptr = NULL;
            TAILQ_FOREACH(se_ptr, loop->_sockets_accepts, entries) {
                fd_array[i].fd = se_ptr->socket;
                fd_array[i].events |= POLLIN;
                i = i + 1;
            }
        }

        pthread_mutex_unlock(&loop->_mutex_sock_events);
        /////////////////////////////////////////////////////////

        int poll_res = poll(fd_array, size, -1); // TODO (ageev) установить таймаут. Тайматут должен браться из конфигурации
        if (poll_res == POLL_TIMEOUT) {
            continue;
        }
        if (poll_res < 0) {
            // TODO (ageev) сделать обработку ошибки через callback
            exit(-1);
        }
        for(int index = 0; index < size; index++) {
            if (fd_array[index].revents == POLLIN) {
                fd_array[index].revents |= POLLIN;
                occurred_event_entry *occurred = malloc(sizeof(events_entry));
                // ПРоверка: Ожидает ли сокет подключения
                if (index >= loop->_index_acepptors_start) {
                    // TODO дописать создание события подключения клиента
                    occurred->element.event = (event_t*)malloc(sizeof(event_sock_accept));
                    occurred->element.event->event.socket = fd_array[index].fd;
                    occurred->element.event->event.socket = SOCK_ACCEPT;
                    socket_entry *se_ptr = NULL;
                    TAILQ_FOREACH(se_ptr, loop->_sockets_accepts, entries) {
                        if (se_ptr->socket == fd_array[index].fd) {
                            break;
                        }
                    }
                    ((event_sock_accept*)occurred->element.event)->handler = se_ptr->handler;
                    TAILQ_INSERT_TAIL(loop->_event_queue, occurred, entries);
                } else {
                    event_sock_read *e= (event_sock_read*)malloc(sizeof(event_sock_read));
                    e->event.event.type = SOCK_READ;
                    e->event.event.socket = fd_array[index].fd;
                    // TODO (ageev) заменить список на динамический массив!
                    registered_events_entry *re_ptr = NULL; // поиск группы событий для сокета
                    TAILQ_FOREACH(re_ptr, loop->_sock_events, entries) {
                        if (re_ptr->sock_events.sock == fd_array[index].fd) {
                            break;
                        }
                    }
                    // поиск зарегистрированного события для сокета
                    events_entry *ee_ptr = NULL;
                    TAILQ_FOREACH(ee_ptr, re_ptr->sock_events.events, entries) {
                        if (ee_ptr->event->event.type == SOCK_READ) {
                            break;
                        }
                    }
                    e->handler = ((event_sock_read*)ee_ptr)->handler;
                    e->size = ((event_sock_read*)ee_ptr)->size;
                    occurred->element.event = (event_t*)e;

                    TAILQ_INSERT_TAIL(loop->_event_queue, occurred, entries);
                }
            }
        }

    }

    return NULL;
}

void el_init(event_loop* el) {
    el->_sockets_accepts = malloc(sizeof(sockets_queue));
    TAILQ_INIT(el->_sockets_accepts);
    el->_event_queue = malloc(sizeof(event_queue_t));
    TAILQ_INIT(el->_event_queue);
    el->_sock_events = malloc(sizeof(registered_events_queue));
    TAILQ_INIT(el->_sock_events);
    pthread_mutex_init(&el->_mutex_sock_events, NULL);
    pthread_mutex_init(&el->_mutex_event_queue, NULL);
    pthread_mutex_init(&el->_mutex_is_run, NULL);
}

int el_open(event_loop* el, bool is_in_thread) {
   if (is_in_thread) {
       loop_thread(el);
       return 1;
   } else {
      return pthread_create(&el->_thread, NULL, loop_thread, el);
   }

}


void el_close(event_loop *el) {
    pthread_mutex_lock(&el->_mutex_is_run);

    el->_is_run = false;

    pthread_mutex_unlock(&el->_mutex_is_run);

    pthread_join(el->_thread, NULL);

}

bool el_run(event_loop* el) {
    pthread_mutex_lock(&el->_mutex_event_queue);

    occurred_event_entry  etype = **el->_event_queue->tqh_last;
    TAILQ_REMOVE(el->_event_queue,*el->_event_queue->tqh_last, entries);

    pthread_mutex_unlock(&el->_mutex_event_queue);

    if (etype.element.event->event.type == SOCK_WRITE) {
        event_sock_write *e = (event_sock_write*) &etype.element.event;
        int res = send(e->event.event.socket, &e->buffer[e->offset], e->size - e->offset, 0);
        if (res == -1) {
            // Ошибка отправки буфера
            async_error error;
            error.is = ERROR;
            error.message = ERROR_MESSAGE_WRITE_ERROR;
            e->handler(el, e->event.event.socket, e->offset, error);
            e->deleter(e->buffer, e->size);
        } else if (res < (e->size - e->offset)) {
            // TODO(ageev) отправка произведена не полностью
            // _el_async_write(el, e);
        } else {
            // все отправилось вызываем обработчик
            async_error error;
            error.is = NO_ERROR;
            error.message = ERROR_MESSAGE_NO_ERROR;
            e->handler(el, e->event.event.socket, e->size, error);
            e->deleter(e->buffer, e->size);
        }
    } else if (etype.element.event->event.type == SOCK_READ) {
        _socket_read(el, &etype);
    } else if (etype.element.event->event.type == SOCK_ACCEPT) {
        _socket_accept(el, &etype);
    } else if (etype.element.event->event.type == SOCK_TIMER) {
        // TODO (ageev) вызов обработчика для конкретного сокета по таймеру.(?!)
    }
    return 1;
}


void el_async_accept(event_loop* loop, int sock, sock_accept_handler handler) {
    pthread_mutex_lock(&loop->_mutex_sock_events);
    _el_async_accept(loop, sock, handler);
    pthread_mutex_unlock(&loop->_mutex_sock_events);
}

// TODO (ageev) перепроверить реализацию функции
void _el_async_accept(event_loop* loop, int sock, sock_accept_handler handler) {
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
    socket_entry *se = malloc(sizeof(socket_entry));
    TAILQ_INSERT_TAIL(loop->_sockets_accepts, se, entries);
}

void el_async_read(event_loop* loop, int sock, char *buffer, int size, sock_read_handler handler) {
    event_sock_read  *event = (event_sock_read*)malloc(sizeof(event_sock_read));
    event->event.event.socket = sock;
    event->event.event.type = SOCK_READ;
    event->size = size;
    event->buffer = buffer;
    event->handler = handler;
    event->offset = 0;

    occurred_event_entry *occurred = malloc(sizeof(occurred_event_entry));
    occurred->element.event = (event_t*) event;
    _el_async_read(loop, occurred);

}

void _el_async_read(event_loop *loop, occurred_event_entry *occurred) {
    pthread_mutex_lock(&loop->_mutex_event_queue);

    TAILQ_INSERT_TAIL(loop->_event_queue, occurred, entries);

    pthread_mutex_unlock(&loop->_mutex_event_queue);
}


void _socket_read(event_loop *loop, occurred_event_entry *occurred) {
    // TODO (ageev) обновить логику чтения из сокета. Читать из буфера, котороый предоставлен пользователем.
    event_sock_read *e = (event_sock_read*)occurred->element.event;
    int res = 0;
    while((res = recv(e->event.event.socket, e->buffer + e->offset, e->size - e->offset, MSG_NOSIGNAL)) != -1) {
        if (res == 0) {
            async_error error;
            error.is = NO_ERROR;
            error.message = ERROR_MESSAGE_NO_ERROR;
            e->handler(loop, e->event.event.socket, e->buffer, e->offset, error);
        }
        e->offset += res;
    }
    if (errno == EAGAIN) {
        _el_async_read(loop, occurred);
    }

}

void _socket_accept(event_loop *loop, occurred_event_entry *occurred) {
    // подключение клиента
    event_sock_accept *e = (event_sock_accept*)occurred->element.event;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t addr_len = sizeof(struct sockaddr_in);
    // TODO (ageev) Обработка ошибко принятия подключений
    int slave = accept(e->event.event.socket, (struct sockaddr*)&client_addr, &addr_len);
    async_error error;
    error.is = NO_ERROR;
    error.message = ERROR_MESSAGE_NO_ERROR;
    e->handler(loop, e->event.event.socket, slave, client_addr, error);
}