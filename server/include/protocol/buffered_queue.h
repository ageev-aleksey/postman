#ifndef SERVER_BUFFERED_QUEUE_H
#define SERVER_BUFFERED_QUEUE_H
#include "error_t.h"
#include <sys/queue.h>

#ifndef BFQ_BUFFER_SIZE
    #define BFQ_BUFFER_SIZE 100
#endif

typedef struct _bf_queue_entry {
    char buffer[BFQ_BUFFER_SIZE];
    TAILQ_ENTRY(_bf_queue_entry) entries;
} bf_queue_entry;

TAILQ_HEAD(_bf_queue, _bf_queue_entry);
typedef struct _bf_queue bf_queue;

/**
 * Инициализация очереди буферов. Функция сама выделит память.
 * Нужен только указатель
 * @param queue - указатель на очередь.
 * @param error - статус выполнения
 * @return true - операция вполнена успешно; false - произошла ошибка при выполнении операции
 */
bool bfq_init(bf_queue **queue, error_t *error);
/**
 * Создание новго буффера, который будет возвращен через параметр
 * @param queue - список буфферов
 * @param buffer - указатель, на указатель в который будет записан адрес нового буффера
 * @param error - статаус выполнения операции
 * @return true - операция вполнена успешно; false - произошла ошибка при выполнении операции
 */
bool bfq_add(bf_queue *queue,  char **buffer, error_t *error);
/**
 * Сумарный размер всех буфферов.
 * Каждый буффер имеет размер BFQ_BUFFER_SIZE. Данная функйия вернет через параметр size
 * значение (n-1)*BFQ_BUFFER_SIZE+strlen(end_buffer), где n - количество буферов в списке.
 * end_buffer - последний буффер в списке. Таким образом функция возвращает размер, равный размер массива,
 * елси все имеющиеся данные объелинить. Т.е. учттывается незаполненность последнего буффера.
 * (Незаполненность промежуточных буфферов не учитывается)
 * @param queue - список буферов
 * @param size - указатель на переменную, в которую бедут записан общий размер
 * @param error - статус выполнения
 * @return true - операция вполнена успешно; false - произошла ошибка при выполнении операции
 */
bool bfq_size(bf_queue *queue, int *size, error_t *error);
/**
 * Выделение массива размером, который возвращает функцция bfq_size. И заполнение его данными из буферов списка.
 * Выполняется копирование данных из буферов в массив, который автоматически выделяется.
 * @param queue - список буферов
 * @param ptr - указатель, на указатель в который будет записан адрес выделенного массива
 * @param error - статус выполения
 * @return true - операция вполнена успешно; false - произошла ошибка при выполнении операции
 */
bool bfq_glue(bf_queue *queue, char **ptr, error_t *error);
/**
 * Поиск последовательности символов в списке буфферов, начиная с конца
 * @param queue
 * @param symbols
 * @param size
 * @param index
 * @param error
 * @return
 */
bool bfq_search(bf_queue *queue, char *symbols, size_t size, size_t *index_start, size_t index_stop, error_t *error);
/**
 * Освобождение свсех ресурсов из под списка буферов
 * @param queue - список буферов
 */
void bfq_free(bf_queue *queue);


#endif //SERVER_BUFFERED_QUEUE_H
