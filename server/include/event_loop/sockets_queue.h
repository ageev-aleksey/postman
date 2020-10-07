//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_SOCKETS_QUEUE_H
#define SERVER_SOCKETS_QUEUE_H

#include "error_t.h"
#include "event_t.h"

#include <sys/queue.h>

/////////// sockets_queue - sq ///////////
struct _event_loop;
struct sockaddr_in;

typedef struct _sockets_entry {
    int socket;
    sock_accept_handler handler;
    TAILQ_ENTRY(_sockets_entry) entries;
} socket_entry;

TAILQ_HEAD(_sockets_queue, _sockets_entry);
typedef struct _sockets_queue sockets_queue;

/**
 * Инициализации очредеи ацепторов
 * @return указатель на инициализированную структуру
 */
sockets_queue *sq_init(error_t *error);
/**
 * Добавление ацептора и его обработчика
 * @param queue - очередь
 * @param socket - сокет ацептор
 * @param handler - обработчик (не может быть равным NULL)
 * @return успешность операции. Возможно добавить только один обработчик для одного сокета
 *  и обработчик не должен быть равным NULL
 */
bool sq_add(sockets_queue *queue, int socket, sock_accept_handler handler, error_t *error);
/**
 * Получение обработчика для сокета-ацептора
 * @param queue - очередь обработчиков
 * @param socket - сокет-ацептор
 * @return Указатель на обработчик или NULL если обработчик отсутствует
 */
sock_accept_handler sq_get(sockets_queue *queue, int socket, error_t *error);
/**
 * Удаление обработчика для указанного сокета
 * @param queue - очередь содрежащая обработчик
 * @param socket - сокет для которого выполняется удаление
 * @return Успешность операии. Если обработчик отсутсвовал или queue=NULL -> false
 */
bool sq_delete(sockets_queue *queue, int socket);
/**
 * Освобождение памяти из подвсех элементов и очереди ацепторов
 * @param queue - очередь из под которой необходимо освободить память
 */
void sq_free(sockets_queue* queue);



#endif //SERVER_SOCKETS_QUEUE_H
