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

int loop_is_started(event_loop* loop) {
    int res = 0;
    pthread_mutex_lock(&loop->_mutex_is_run);
    res = loop->_is_run;
    pthread_mutex_unlock(&loop->_mutex_is_run);
    return res;
}

int queue_length_req(registered_events_queue *queue) {
    int i = 0;
    for(registered_events_entry *ptr = queue->tqh_first;
        ptr != NULL;
        ptr = ptr->entries.tqe_next) {
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
    int timeout = 500; // 0.5 sec
    loop->_is_run = true;
    while (loop_is_started(loop)) {

        /////////Псотроение массива pollfd//////////////////////
        pthread_mutex_lock(&loop->_mutex_sock_events);

        int length = queue_length_req(loop->_sock_events);
        struct pollfd *fd_array = malloc(sizeof(struct pollfd)*length);
        int i = 0;
        for (registered_events_entry *events_ptr = loop->_sock_events->tqh_first;
                events_ptr != NULL;
                events_ptr = events_ptr->entries.tqe_next)
        {
            memset(&fd_array[i], 0, sizeof(length));
            fd_array[i].fd = events_ptr->sock_events.sock;
            for (events_entry *sock_event = events_ptr->sock_events.events->tqh_first;
                sock_event != NULL;
                sock_event = sock_event->entries.tqe_next)
            {
                event_t *event = sock_event->event;

                if (event->type == SOCK_ACCEPT) {
                    // TODO(ageev) socket bind!
                    fd_array[i].events |= POLLIN;
                } else if (event->type == SOCK_READ) {
                    fd_array[i].events |= POLLIN;
                } else if (event->type == SOCK_WRITE) {
                    // TODO(ageev) создать событие на запись. И положить в очередь событий.
                    //  Данное событие должно быть извлечено из очереди событий очередным вызовом функции run
                    //  События на отправку данных регистрировать не надо. Необходимо сразу создавать событие
                    //  И класть его в очередь.
                }
            }
            i = i + 1;
        }

        pthread_mutex_unlock(&loop->_mutex_sock_events);
        /////////////////////////////////////////////////////////

        int poll_res = poll(fd_array, length, timeout);
        if (poll_res == POLL_TIMEOUT) {
            continue;
        }
        if (poll_res < 0) {
            // TODO (ageev) сделать обработку ошибки через callback
            exit(-1);
        }
        for(int indx = 0; indx < length; indx++) {
            if (fd_array[i].revents == POLLIN) {
                // TODO (ageev) чтение сокета и вызов соответсвующего callback
                //  Добавить проверку, что сокет является для подключения
                //  Сделать отдельную обработку мастер сокета
            }
        }

    }

    return NULL;
}

void el_init(event_loop* el) {
    TAILQ_INIT(el->_event_queue);
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

    if (etype.element.event.type == SOCK_WRITE) {
        event_sock_write *e = (event_sock_write*) &etype.element.event;
        int res = send(e->event.socket, &e->buffer[e->offset], e->size - e->offset, 0);
        if (res == -1) {
            // Ошибка отправки буфера
            async_error error;
            error.is = ERROR;
            error.message = ERROR_MESSAGE_WRITE_ERROR;
            e->handler(el, e->event.socket, e->offset, error);
            e->deleter(e->buffer, e->size);
        } else if (res < (e->size - e->offset)) {
            // TODO(ageev) отправка произведена не полностью
            // _el_async_write(el, e);
        } else {
            // все отправилось вызываем обработчик
            async_error error;
            error.is = NO_ERROR;
            error.message = ERROR_MESSAGE_NO_ERROR;
            e->handler(el, e->event.socket, e->size, error);
            e->deleter(e->buffer, e->size);
        }
    } else if (etype.element.event.type == SOCK_READ) {
        event_sock_read *e = (event_sock_read*)&etype.element.event;
        bf_queue *bf = malloc(sizeof(bf_queue));
        TAILQ_INIT(bf);
        bf_queue_entry  *element = malloc(sizeof(bf_queue_entry));
        int res = 0;
        int offset = 0;
        while((res = recv(e->event.socket, element->buffer, BUFFERED_QUEUE_BUFFER_SIZE, 0)) != -1) {
            TAILQ_INSERT_TAIL(bf, element, entries);
            if (res == 0) {
                // мы все счиатли из сокета
                // перенос buff_queue в array
                unsigned int len = queue_length_bf(bf);
                char *buffer = malloc(len*BUFFERED_QUEUE_BUFFER_SIZE);
                int index_buffer = 0;
                int index_bf = 0;
                for (bf_queue_entry *ptr = bf->tqh_first; ptr != NULL; ptr = ptr->entries.tqe_next) {
                    buffer[index_buffer] = ptr->buffer[index_bf];
                    ++index_bf;
                    ++index_buffer;
                    if (index_bf == BUFFERED_QUEUE_BUFFER_SIZE) {
                        index_bf = 0;
                    }
                }
                async_error error;
                error.is = NO_ERROR;
                error.message = ERROR_MESSAGE_NO_ERROR;
                e->handler(el, e->event.socket, buffer, error);
                // TODO (ageev) как удалить из памяти очередь?
                free(buffer);
            }
            element = malloc(sizeof(bf_queue_entry));
            offset += res;
        }
        if (errno == EAGAIN) {
            e->buffer = bf;
            e->offset = offset;
            // TODO (ageev) _el_async_read(el, e)
        }

    } else if (etype.element.event.type == SOCK_ACCEPT) {
        // подключение клиента
        event_sock_accept *e = (event_sock_accept*)&etype.element.event;
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        socklen_t addr_len = sizeof(struct sockaddr_in);
        // TODO (ageev) Обработка ошибко принятия подключений
        int slave = accept(etype.element.event.socket, (struct sockaddr*)&client_addr, &addr_len);
        async_error error;
        error.is = NO_ERROR;
        error.message = ERROR_MESSAGE_NO_ERROR;
        e->handler(el, slave, error);
    } else if (etype.element.event.type == SOCK_TIMER) {
        // TODO (ageev) вызов обработчика для конкретного сокета по таймеру.(?!)
    }
    return 1;
}


void el_async_accept(event_loop* loop, int sock, sock_accept_handler handler) {
    pthread_mutex_lock(&loop->_mutex_sock_events);
    _el_async_accept(loop, sock, handler);
    pthread_mutex_unlock(&loop->_mutex_sock_events);
}


void _el_async_accept(event_loop* loop, int sock, sock_accept_handler handler) {
    // Зарегистрирован ли данный сокет в цилке событий
    registered_events *event = NULL;
    for (registered_events_entry *ptr = loop->_sock_events->tqh_first;
        ptr != NULL;
        ptr = ptr->entries.tqe_next)
    {
        if (ptr->sock_events.sock == sock) {
            event = &ptr->sock_events;
            break;
        }
    }
    // если нет то регистрируем сокет
    if (!event) {
        registered_events_entry  *registered_event = malloc(sizeof(registered_events_entry));
        registered_event->sock_events.sock = sock;
        TAILQ_INIT(registered_event->sock_events.events);
        event = &registered_event->sock_events;
    }

    //регистрируем событие для сокета
    event_sock_accept *accept_event = malloc(sizeof(event_sock_accept));
    accept_event->event.type = SOCK_ACCEPT;
    accept_event->event.socket = sock;
    accept_event->handler = handler;
    events_entry *e = malloc(sizeof(events_entry));
    e->event = (event_t*)accept_event;
    TAILQ_INSERT_TAIL(event->events, e, entries);
}