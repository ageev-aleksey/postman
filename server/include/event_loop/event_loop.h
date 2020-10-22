// TODO (ageev) необходимо реализовать свойства цикла событий.
//  - Время на таймаут в poll
//  - Выбор способа мультипликсирвоания (?)
//  - ...

//  TODO (ageev) Использовать глобальный обработчик для Debug логирования

#ifndef SMTP_EVENT_LOOP_H
#define SMTP_EVENT_LOOP_H

#include "event_t.h"
#include "sockets_queue.h"
#include "registered_events_queue.h"
#include "occurred_event_queue.h"

#include "protocol/buffered_queue.h"

#include <stdbool.h>
#include <sys/queue.h>
#include <pthread.h>
#include <poll.h>
#include <netinet/in.h>

/**
 * Описание глобального обработчика
 * socket - файловый дескриптор (сокет) при работе с котроым возникал ошибка
 * error - что произошло
 */
typedef void (*error_global_handler)(int socket, error_t error, int line_execute, const char* function_execute);

typedef enum __work_mode {
    ONE_THREAD, // Менеджер очереди и обработка произошедших событий будет выполнятся в одном потоке
    OWN_THREAD // Менеджер очереди будет выполнятся в отдельном потоке, для обработки событий необходимо вызывать функцию el_run
} work_mode;

typedef  enum _error_type  {
    NO_ERROR, ERROR
} error_type;

typedef struct _async_error {
    error_type is;
    const char *message;
} async_error;




typedef struct _event_loop {
    sockets_queue *_acceptors_queue; // Список сокетов ожидающих принятия подключения
    int _index_acepptors_start; // Индекс в pollfd[] с которого начинаются сокеты-приемщики
    registered_events_queue *_registered_events; // Зарегистрированные обработчики событий
    occurred_event_queue *_occurred_events; // Произошедшие события
    pthread_t _thread; // поток менеджера событий
    pthread_mutex_t _mutex_occurred_events; // Защита очереди произошедших событий
    pthread_mutex_t _mutex_registered_events; // Защита списка зарегистрированных обработчиков событий
    pthread_mutex_t _mutex_acceptors_queue;
    pthread_mutex_t _mutex_is_run; // Защита флага is_run
    int _is_run; // флаг работы менеджера. Если 1 то менеджер работает.
    error_global_handler _global_handler; // Обрабочтик событий не остносящийся к событиям.
} event_loop;

//PUBLIC

/**
 * Создание цикла событий
 * @param error - статус выполнения операции
 * @return указатель на event_loop при успехе; NULL - если возникла ошибка
 */
event_loop* el_init(error_t *error);

 /**
  * Инициализация цикла событий и его запуск.
  * Поведение функции зависит от значение параметра mode:
  *     - ONE_THREAD - в одном потоке будет работать менеджер событий и их обработчик.
  *         функция будет заблокирована до тех пор, пока цикл событий не будет остановлен (@see el_stop)
  *     - OWN_THREAD - для менеджера событий будет создан отдельный поток. Функция сразу вернет управление.
  *         Обработка происходящих событий должна вестить вручную (вызовом функции el_run).
  *         Таким образом менеджер событий и обработчик соыбтий работают в разных потоках.
  *         Обработчиков событий может быть несколько
  * @param mode  режим работы
  * @param error статус выполнения операции
  * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
  * @remark  Не предназначен для запуска из множества потоков.
  * Создание несколько менеджеров событий не поддерживается.
  */
bool el_open(event_loop*, work_mode mode,  error_t *error);
/**
 * Освобождение ресусров
 * @param loop - цикл событий из подкоторого необходимо освободить ресурсы
 * @remark No thread save
 */
void el_close(event_loop* loop);
/**
 * Выполнение обработки одного произошедшего события.
 * Если существует некоторое событие, то для него будет вызван обработчик в том потоке,
 * в котором была вызвана данная функция. Если события отсутсвуют,
 * то тогда функция вернет ошибку NOT_FOUND в параметре error
 * @param loop - уикл событий, для которого необходимо обработать произошедшие события
 * @param error - статус выполнения операции
 * @return  true - если был вызван обработчик события; false - если обработчик события не был вызван
 * @remark thread save
 */
bool el_run(event_loop* loop,  error_t *error);
/**
 * Регистрация обработчика осбытия "Подключение нового клиента" для сокета.
 * Соект должен быть настроен, как неблокирующий и быть слушающим.
 * @param loop цикл событий, в котором зарегистрировать данное событие
 * @param sock - слушающий неблокирующийся сокет
 * @param error - статус выполнения операции
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 * @remark thread save
 */
bool el_async_accept(event_loop* loop, int sock, sock_accept_handler,  error_t *error);
/**
 * Регистрация обработчика события "Чтение данных из сокета" для указанного сокета.
 * Соект должен быть настроен, как неблокирующий. Если сокет был получен врезультате вызова обработчкиа
 * события "подключение нового клиента" (@see el_async_accept) то сокет уже имеет соответсвующие настройки.
 * С ним ни чего делать не нужно.
 * @param loop цикл событий, в котором зарегистрировать данное событие
 * @param sock  неблокирующий сокет, для которого регистрировать событие
 * @param buffer  Указатель на буффер, в который должно произойти запиь данных при чтении
 * @param size  размер буфера
 * @param error  статус выполнения операции
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 * @remark thread save
 */
bool el_async_read(event_loop* loop, int sock, char *buffer, int size, sock_read_handler,  error_t *error);

/**
 * Регистрация обработчика события "Запись данных в сокет" для указанного сокета.
 * Соект должен быть настроен, как неблокирующий. Если сокет был получен врезультате вызова обработчкиа
 * события "подключение нового клиента" (@see el_async_accept) то сокет уже имеет соответсвующие настройки.
 * С ним ни чего делать не нужно.
 * @param loop цикл событий, в котором зарегистрировать данное событие
 * @param sock неблокирующий сокет, для которого регистрировать событие
 * @param output_buffer буфер из которого небходимо выполнить чтение при записи даннх в сокет
 * @param bsize  размер буфера
 * @param error статус выполнения операции
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 * @remark thread save
 */
bool el_async_write(event_loop* loop, int sock, void *output_buffer, int bsize,
                    sock_write_handler,  error_t *error);
// TODO (ageev) выполнить реализцию события "Истек таймер"
// bool el_timer(event_loop* loop, int sock, unsigned int ms, sock_timer_handler);
/**
 * Остановка цикла событий
 * @param loop  цикл событий, который необходимо остановить
 * @param error статуст выполнения операции
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool el_stop(event_loop* loop,  error_t *error);

/**
 * Регистрация глобального обработчика ошибок. Имеются набор ошибок, которые необходимо обрабатывать
 * немедленно. Например, ошибка добавление нового события в очередь зарегистрированных событий.
 * Данный обработчик вызывается (Если это возможно) при фатальных ошибках.
 * @param loop - цикл событий
 * @param hander - обрабочик
 * @param error - возрват ошибок, которые могут произойти при добавлении обработчика
 * @return true - операция заврешилась успешно; false -  операция завершилась с ошибкой.
 */
bool el_reg_global_error_handler(event_loop *loop, error_global_handler hander, error_t *error);

// PRIVATE
bool pr_create_pollfd(event_loop* loop, struct pollfd **fd_array, int *size, error_t *error);
bool pr_create_pollin_event(event_loop *loop, struct pollfd *fd, int index, error_t *error);
bool pr_create_pollout_event(event_loop *loop, struct pollfd *fd_array, int index, error_t *error);


#define QUEUE_SIZE(entry_type, queue, field, res) \
     do {                                    \
        int i = 0;                           \
        entry_type *__ptr__ = NULL;              \
         TAILQ_FOREACH(__ptr__, queue, field) {  \
            i = i + 1;                                    \
         }                                        \
        *res = i;                                             \
     } while(0)\

#endif //SMTP_EVENT_LOOP_H
