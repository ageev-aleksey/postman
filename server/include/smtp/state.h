#include "smtp-states-fsm.h"
#include "error_t.h"
#include "vector.h"
#include "vector_structures.h"

#ifndef SMTP_STATE_H_
#define SMTP_STATE_H_

#define SMTP_COMMAND_END "\r\n"
#define SMTP_COMMAND_END_LEN 2

typedef struct smtp_mailbox {
    char *user_name;
    char *server_name;
} smtp_mailbox;

VECTOR_DECLARE(vector_smtp_mailbox, smtp_mailbox);

enum smtp_address_type {
    SMTP_ADDRESS_TYPE_IPv4, SMTP_ADDRESS_TYPE_IPv6, SMTP_ADDRESS_TYPE_DOMAIN, SMTP_ADDRESS_TYPE_NONE
};

typedef struct smtp_address {
    char *address;
    enum smtp_address_type type;
} smtp_address;

typedef enum d_smtp_status {
    SMTP_STATUS_ERROR,                   /// Произошла ошибка во время обработки сообщения
    SMTP_STATUS_OK,                      /// Сообщение полностью обработано
    SMTP_STATUS_WARNING,                 /// Сообщение полностью обраотано, но ответ для клиента отрицательный
    SMTP_STATUS_CONTINUE,                /// Сообщение состоит из множества строк, необходимо продолжить обрабатывать строки
    SMTP_STATUS_DATA_END,                /// Тело письма завершено. письмо можно доставлять.
    SMTP_STATUS_EXIT,
} smtp_status;

typedef struct d_smtp_state {
    te_autofsm_state pr_fsm_state;    /// Состояние протокола
    vector_smtp_mailbox pr_rcpt_list; /// Список получателей
    smtp_mailbox *pr_mail_from;       /// Отправитель письма
    smtp_address *pr_hello_addr;      /// Адрес сервера предоставленный в HELO/EHLO
    vector_char pr_mail_data;         /// Тело письма
    char *pr_buffer;                  /// Буффер для хранения текущей обрабатываемой команды
    size_t pr_bsize;                  /// Размер буффера
    smtp_status pr_status;            /// статус обработки последней команды
} smtp_state;



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
    SMTP_INVALID_COMMAND
};


typedef struct smtp_command {
    bool status;
    enum smtp_command_type type;
    te_autofsm_event event;
    void *arg;
} smtp_command;



/**
 * Инициализация библиотеки для обработки smtp протокола
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
bool smtp_init(smtp_state *smtp, err_t *error);
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
smtp_status smtp_parse(smtp_state *smtp, const char *message, char **buffer_reply, err_t *error);

char *smtp_make_response(smtp_state *smtp, size_t code, const char* msg);

/**
 * Перенос буфера получателя сообщения. После вызова этой функции буфером владеет пользователь. Он должен освободить ресурсы
 * @param smtp
 * @param buffer
 * @param error
 * @return
 */
bool smtp_move_buffer(smtp_state *smtp, char **buffer, size_t *blen, err_t *error);

vector_smtp_mailbox* smtp_get_rcpt(smtp_state *smtp);
smtp_mailbox* smtp_get_sender(smtp_state *smtp);

smtp_status smtp_get_status(smtp_state *smtp);

smtp_address smtp_get_hello_addr(smtp_state *smtp);

#endif
