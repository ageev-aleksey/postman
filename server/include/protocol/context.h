#include "protocol/buffered_queue.h"

#include "sys/queue.h"

typedef struct __context {
    int socket;
    bf_queue bf;
} context_t;


typedef struct __context_entry {
    context_t *context;
    TAILQ_ENTRY(__context_entry) entries;
} context_entry;

TAILQ_HEAD(__context_queue, __context_entry);
typedef struct __context_queue context_queue;

/**
 * Инициализация двухсвязного списка очереди
 * @param error - статус
 * @return указатель на двухсвязанный список; NULL - в случае ошибки
 */
context_queue* cq_init(error_t *error);
/**
 * Добавление контекста в очередь. Контекст должен быть создан в куче. Копирование не выполнятеся.
 * Так же предполагается, что для каждого сокета будет создан один контекст.
 * Проверка на существование не выоплняется
 * @param queue - очередь
 * @param context - указател на контекст
 * @param error
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool cq_add(context_queue *queue, context_t *context, error_t *error);
/**
 * Поиск контекста по сокету.
 * @param queue - список контекстов
 * @param context - сюда будет записан указатель на найденный контекст
 * @param socket - сокет, для которого ищется контекст
 * @param error - статус выполнения операции
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool cq_find(context_queue *queue, context_t **context, int socket, error_t *error);

/**
 * Удаление контекста сокета
 * @param queue
 * @param socket
 * @param error
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool cq_del(context_queue *queue, int socket, error_t *error);

/**
 * Полное освобождение ресурсов. Усли содержит элементы, то из подних так же будет освобождена память,
 * таким образом указатели полученные из очереди (@see cq_find) будут недействительными
 * @param queue - список из подкоторого необходимо освободить память
 */
void cq_free(context_queue *queue);
