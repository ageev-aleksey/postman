//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_EVENTS_QUEUE_H
#define SERVER_EVENTS_QUEUE_H

#include <sys/queue.h>
#include "event_t.h"

typedef struct _events_entry {
    event_t *event;
    TAILQ_ENTRY(_events_entry) entries;
} events_entry;

TAILQ_HEAD(_events_queue,  _events_entry);
typedef struct _events_queue events_queue;

/**
 * Создание и инициализация списка событий
 * @param error - статус выполнения
 * @return указатель на список событий если операция удачна; NULL - если операци не удачна
 */
events_queue* eq_init(err_t *error);
/**
 * Освобождение памяти из под списка и всех его элементов
 * @param queue  Список из под которого необходимо освободить память
 */
void eq_free(events_queue *queue);

/**
 * Добавление в список события "Подключение клиента"
 * @param queue - список событий
 * @param event  - описатьль события
 * @param error - статус выполнения
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool eq_push_accept(events_queue *queue, event_sock_accept *event, err_t *error);
/**
 * Добавление в список события "Чтение из сокета"
 * @param queue - список событий
 * @param event  - описатьль события
 * @param error - статус выполнения
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool eq_push_read(events_queue *queue, event_sock_read *event, err_t *error);
/**
 * Добавление в список события "Запись в сокет"
 * @param queue - список событий
 * @param event  - описатьль события
 * @param error - статус выполнения
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool eq_push_write(events_queue *queue, event_sock_write *event, err_t *error);

// bool eq_push_disconnect(events_queue *queue, event_sock_disconnect *event, err_t *error);

/**
 * Извлечение (удаление из списка и возрат удаленного объекта) события "Подключение клиента" из списка
 * @param queue список из которого извлеч
 * @param event указатель на указатель, в которой будет присовено значение адреса извлекаемого элемента
 * @param error статус выполнения операции
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool eq_pop_accept(events_queue *queue, event_sock_accept **event, err_t *error);
/**
 * Извлечение (удаление из списка и возрат удаленного объекта) события "Чтение из сокета" из списка
 * @param queue список из которого извлеч
 * @param event указатель на указатель, в которой будет присовено значение адреса извлекаемого элемента
 * @param error статус выполнения операции
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool eq_pop_read(events_queue *queue, event_sock_read **event, err_t *error);
/**
 * Извлечение (удаление из списка и возрат удаленного объекта) события "Запись в сокет" из списка
 * @param queue список из которого извлеч
 * @param event указатель на указатель, в которой будет присовено значение адреса извлекаемого элемента
 * @param error статус выполнения операции
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool eq_pop_write(events_queue *queue, event_sock_write **event, err_t *error);

// bool eq_pop_disconnect(events_queue *queue, event_sock_disconnect **event, err_t *error);

#endif //SERVER_EVENTS_QUEUE_H
