//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_REGISTERED_EVENTS_QUEUE_H
#define SERVER_REGISTERED_EVENTS_QUEUE_H

#include "events_queue.h"

#include <sys/queue.h>
#include <stdbool.h>

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


/////////// registered_events_queue - req ///////////

/**
 * создание и инициализация структуры для хранения обработчиков событий
 * @return Инициализированная структура
 */
registered_events_queue* req_init(error_t* error);
/**
 * Добавление события для сокета
 * @param queue куда добавлять событе
 * @param socket для кагого сокета
 * @param event  описать события (описатль будет скопирован)
 * @return Успешность выполнения. Возможно для одного секта добавлять только один тип событий
 */
bool req_push_accept(registered_events_queue* queue, int socket, event_sock_accept *event, error_t *error);
bool req_push_read(registered_events_queue* queue, int socket, event_sock_read *event, error_t *error);
bool req_push_write(registered_events_queue* queue, int socket, event_sock_write *event, error_t *error);
//
/**
 * удаление события определенного типа для заданного сокета
 * @param queue Очередь из которой удалять событие
 * @param socket сокет для которого удалять событе
 * @param type тип события, который необходимо удалить
 */
//void req_delete(registered_events_queue* queue, int socket, event_type type);
/**
 * Получение описателя зарегистрированного события для заданного сокета и определнного типа
 * @param queue - Очередь, в которой искать событие
 * @param socket  - для каого сокета искать событие
 * @param type - тип события, которое необходимо найти
 * @return Указатель на событие из очереди или NULL если описатель отсутсвует
 */
event_sock_accept* req_pop_accept(registered_events_queue* queue, int socket);
event_sock_read* req_pop_read(registered_events_queue* queue, int socket);
event_sock_write* req_pop_write(registered_events_queue* queue, int socket);
/**
 * Возвращает битовую маску зарегистрированных событий для сокета
 * @param queue - очередь зарегистрированных событий
 * @param socket - сокет для которого выполнять поиск
 * @return Битовая маска
 */
int req_reg(registered_events_queue* queue, int socket);

/**
 * Освобождение память от всех зарегистрированных событий и очереди событий
 * @param queue очередь событий, из под которой необходимо совободить память
 */
void req_free(registered_events_queue* queue);


#endif //SERVER_REGISTERED_EVENTS_QUEUE_H
