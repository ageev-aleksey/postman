#include "smtp-states-fsm.h"
#include "error_t.h"
#include "vector.h"
#include "vector_structures.h"

typedef struct d_recipient {
    char *user_name;
    char *server_name;
} recipient;

VECTOR_DECLARE(vector_recipient, recipient);

typedef struct d_smtp_state {
    te_autofsm_state pr_fsm_state; /// Состояние протокола
    vector_recipient pr_rcpt_list; /// Список получателей
    char *pr_mail_from;            /// Отправитель письма
    char *hello_addr;              /// Адрес сервера предоставленный в HELO/EHLO
    vector_char pr_mail_data;      /// Тело письма
    char *pr_buffer;               /// Буффер для хранения текущей обрабатываемой команды
    size_t pr_bsize;               /// Размер буффера
} smtp_state;

typedef enum d_smtp_status {
    SMTP_ERROR,                   /// Произошла ошибка во время обработки сообщения
    SMTP_OK,                      /// Сообщение полностью обработано
    SMTP_CONTINUE,                /// Сообщение состоит из множества строк, необходимо продолжить обрабатывать строки
} smtp_status;

enum smtp_command_type {
    SMTP_HELLO,
    SMTP_MAILFROM,
    SMTP_RCPTTO,
    SMTP_DATA,
    SMTP_RSET,
    SMTP_VRFY,
    SMTP_EXPN,
    SMTP_HELP,
    SMTP_NOOP,
    SMTP_QUIT,
    SMTP_INVALID_COMMAND;
};


typedef struct smtp_command {
    enum smtp_command_type type;
    char *arg;
} smtp_command;



/**
 * Инициализация библиотеки для обработки ыьез протокола
 */
void smtp_lib_init();
/**
 * Освобождение всех ресурсов из под библиотеки
 */
void smtp_lib_free();
/**
 * Инициализация состояния для протокола smtp
 * @param smtp - описатель состояния
 * @param error
 * @return статус выполнения операции
 */
bool smtp_init(smtp_state *smtp, error_t *error);
void smtp_free(smtp_state *smtp);

/**
 * Обработка протокольных сообщений SMTP. Функция выдает статус обрабокти и протокольный отклик на сообщение
 * @param smtp - описатьель smtp контекста
 * @param message - протокольное сообщение для обработки
 * @param buffer - протокольный отклик. Указатель на указатель буфера - если размер буфера для отклика будет не
 *                  достаточен, то буде выделена новый участок памяти, а старый будет освобожден
 * @param error - описатель статуса выполнения операции
 * @return - статус обработки SMTP сообщения.
 *              - SMTP_ERROR - ошибка обработки сообщения, необходимо проверить error
 *              - SMTP_OK - сообщение полностью обработано
 *              - SMTP_CONTINUE - сообщение яявляется многострочным. Текущая часть сообщения успешно обработано,
 *                  необходимо переадть оставшиеся части (отклик не формируется!)
 *           На каждыое действие в SMTP формируется протокольный отклик, если не указано иного
 */
smtp_status smtp_parse(smtp_state *smtp, const char *message, char **buffer_reply, error_t *error);
/**
 * Перенос буфера теал сообщения. После вызова этой функции буфером владеет пользователь. Он должен освободить ресурсы
 * @param smtp
 * @param buffer
 * @param error
 * @return
 */
bool smtp_move_buffer(smtp_state *smtp, char **buffer, error_t *error);


