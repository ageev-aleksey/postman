#include "server/server.h"
#include "server/users_list.h"
#include "server/timers.h"
#include "server/table.h"
#include "log/context.h"
#include "event_loop/event_loop.h"
#include "smtp/state.h"
#include "smtp/response.h"
#include <regex.h>
#include <string.h>
#include <errno.h>
#include <maildir/user.h>
#include <maildir/message.h>
#include <unistd.h>


#define RE_CONFIG_DOMAIN "([[:alnum:]_-]+\.)*[[:alnum:]_-]+"

bool user_init(user_context *context, struct sockaddr_in *addr, int sock) {
    if (context != NULL) {
        err_t err;
        smtp_init(&context->smtp, &err);
        if (err.error) {
            LOG_ERROR("smtp_init: %s", err.message);
            return false;
        }
        VECTOR_INIT(char, &context->buffer, err);
        if (err.error) {
            LOG_ERROR("vector_init: %s", err.message);
            return false;
        }
        VECTOR_INIT(char, &context->write_buffer, err);
        if (err.error) {
            LOG_ERROR("vector_init: %s", err.message);
            return false;
        }
        context->socket = sock;
        context->addr = get_addr(addr, &err);
        if (err.error) {
            LOG_ERROR("get_addr: %s", err.message);
            return false;
        }
        return true;
    } else {
        return false;
    }
}


void handler_hello_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error);

void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error) {
    if (error.error) {
        LOG_WARNING("handler_accept start: [errno -> %d] %s", error.errno_value, error.message);
    }

    user_context *context = malloc(sizeof(user_context));
    if (context == NULL) {
        LOG_ERROR("%s", "Error alloc memory");
        exit(-1);
    }
    if (!user_init(context, &client_addr, client_socket)) {
        return;
    }
    LOG_INFO("user connect [%s:%d]", context->addr.ip, context->addr.port);
    users_list__add(&server_config.users, &context);

    err_t err;
    el_async_write(el, client_socket,
                   server_config.hello_msg, server_config.hello_msg_size,
                   handler_hello_write, &err);
    if (err.error) {
        LOG_ERROR("el_async_write: %s", err.message);
    }
    timers_make_for_socket(&server_config.timers, client_socket);
    struct timer_event_entry *td;
    el_timer(el, client_socket, POSTMAN_TIMEOUT_OF_TIMER, handler_timer, &td, &err);
    if (err.error) {
        LOG_ERROR("el_timer: %s", err.message);
    }
}
void handler_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error) {
    if (error.error) {
        LOG_WARNING("handler_write start: [errno -> %d] %s", error.errno_value, error.message);
    }

    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }

    timers_update_by_socket(&server_config.timers, client_socket);

    user_accessor acc;
    if(users_list__user_find_by_sock(&server_config.users, &acc, client_socket)) {

     //   err_t verr;
//        if(size >= VECTOR_SIZE(&acc.user->buffer)) {
//            VECTOR_CLEAR(&acc.user->buffer);
//        } else {
//            vector_char new_buffer;
//            VECTOR_SUB(char, &acc.user->buffer, &new_buffer, size, VECTOR_SIZE(&acc.user->buffer)-1, verr);
//            if (verr.error) {
//                LOG_ERROR("vector_sub: %s", error.message);
//            }
//            VECTOR_FREE(&acc.user->buffer);
//            acc.user->buffer = new_buffer;
//        }

        smtp_status sstat = smtp_get_status(&acc.user->smtp);

        if (sstat == SMTP_STATUS_EXIT) {
            // Закрытие соединения, так как пришла команда QUIT
            close(client_socket);
            LOG_INFO("server close connection with user [smtp command QUIT] [%s:%d]", acc.user->addr.ip, acc.user->addr.port);
            user_free(acc.user);
            users_list__delete_user(&acc);
            free(acc.user);
            timers_remove_for_socket(&server_config.timers, client_socket);
        } else {
            err_t err;
            char *buffer_ptr = acc.user->read_buffer;
            user_accessor_release(&acc);
            el_async_read(el, client_socket,
                          buffer_ptr, TEMPORARY_BUFFER_SIZE,
                          handler_read, &err);
            if (err.error) {
                LOG_ERROR("el_async_read: %s", err.message);
            }
        }
    } else {
        LOG_ERROR("Not found user by socket [%d]", client_socket);
    }

    timers_update_by_socket(&server_config.timers, client_socket);
}

void user_disconnected(int sock) {
    user_accessor acc;
    if (users_list__user_find_by_sock(&server_config.users, &acc, sock)) {
        LOG_INFO("user close connection [%s:%d]", acc.user->addr.ip, acc.user->addr.port);
        users_list__delete_user(&acc);
        user_free(acc.user);
        free(acc.user);
        timers_remove_for_socket(&server_config.timers, sock);
    } else {
        LOG_ERROR("user bys socket [%d] not found", sock);
    }
}

void handler_hello_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error) {
    if (error.error) {
        LOG_WARNING("handler_hello_write start: [errno -> %d] %s", error.errno_value, error.message);
    }

    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }

    timers_update_by_socket(&server_config.timers, client_socket);

    user_accessor acc;
    if (users_list__user_find_by_sock(&server_config.users, &acc, client_socket)) {
        err_t err;
        el_async_read(el, client_socket,
                      acc.user->read_buffer, TEMPORARY_BUFFER_SIZE,
                      handler_read, &err);
        user_accessor_release(&acc);
        if (err.error) {
            LOG_ERROR("el_async_read: %s", err.message);
        }
    } else {
        LOG_ERROR("User not found by socket [%d]", client_socket);
    }

    timers_update_by_socket(&server_config.timers, client_socket);
}

bool is_line_and(const char *str) {
    return str[0] == '\r' && str[1] == '\n';
}

void handler_read(event_loop *loop, int client_socket, char *buffer, int size, client_status status, err_t error) {
    if (error.error) {
        LOG_WARNING("handler_read start: [errno -> %d] %s", error.errno_value, error.message);
    }

    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }

    timers_update_by_socket(&server_config.timers, client_socket);

    user_accessor acc;
    if (users_list__user_find_by_sock(&server_config.users, &acc, client_socket)) {

        //записываем данные в буфер
        err_t err;
        for(int j = 0; j < size; j++) {
            VECTOR_PUSH_BACK(char, &acc.user->buffer, buffer[j], err);
            if (err.error) {
                LOG_ERROR("vector push back: %s", err.message);
            }
        }
//        int entry_index = find_first_entry_str(VECTOR(&acc.user->buffer), SMTP_COMMAND_END,
//                                               VECTOR_SIZE(&acc.user->buffer), SMTP_COMMAND_END_LEN);

        struct sub_str_iterator itr;
        itr.str = VECTOR(&acc.user->buffer);
        itr.str_len = VECTOR_SIZE(&acc.user->buffer);
        itr.sep = SMTP_COMMAND_END;
        itr.sep_len = SMTP_COMMAND_END_LEN;
        itr.begin = 0;
        itr.end = 0;
        while(true) {
            sub_str_iterate(&itr);
            //char tmp =  VECTOR(&acc.user->buffer)[itr.end];
            //       VECTOR(&acc.user->buffer)[itr.end] = '\0';
            // if (strcmp(&VECTOR(&acc.user->buffer)[itr.end - SMTP_COMMAND_END_LEN], SMTP_COMMAND_END) == 0) {
            if (is_line_and(&VECTOR(&acc.user->buffer)[itr.end - SMTP_COMMAND_END_LEN])) {
                // Обрабатываем команду
                char tmp =  VECTOR(&acc.user->buffer)[itr.end];
                VECTOR(&acc.user->buffer)[itr.end] = '\0';

                struct pair reply = handler_smtp(acc.user, VECTOR(&acc.user->buffer) + itr.begin);
                VECTOR(&acc.user->buffer)[itr.end] = tmp;
                if (reply.status == SMTP_STATUS_CONTINUE && itr.end != itr.str_len) {
                    continue;
                }


                if (reply.buffer != NULL) {
                    if (VECTOR_SIZE(&acc.user->buffer) != itr.end) {
                        // ПРислали больше чем одна строка
                        LOG_WARNING("the client [%s:%d] sent more than one line", acc.user->addr.ip, acc.user->addr.port);
                    }
                    VECTOR_CLEAR(&acc.user->buffer); // сервер при чтении заголовков воспринимает, только одну строчку
                    user_accessor_release(&acc);
                    el_async_write(loop, client_socket, reply.buffer, strlen(reply.buffer),
                                   handler_write, &err);
                    if (err.error) {
                        LOG_ERROR("el_async_write: %s", err.message);
                    }

                    timers_update_by_socket(&server_config.timers, client_socket);
                    return;
                } else {
                    // нечего отвечать, продолжаем вычитывать остальные команды
                    VECTOR_CLEAR(&acc.user->buffer);
                    char *ptr = acc.user->read_buffer;
                    user_accessor_release(&acc);
                    el_async_read(loop, client_socket,
                                  ptr, TEMPORARY_BUFFER_SIZE,
                                  handler_read, &err);

                    timers_update_by_socket(&server_config.timers, client_socket);
                    return;
                }

            } else {
                if (itr.begin != 0) {
                    // в конце буфера содержится неполная команда перетаскиваем ее в начало буфера
                    char *ptr = VECTOR(&acc.user->buffer);
                    VECTOR_CLEAR(&acc.user->buffer);
                    for (int j = itr.begin; j < itr.end; j++) {
                        err_t ver;
                        VECTOR_PUSH_BACK(char, &acc.user->buffer, ptr[j], ver);
                    }
                }
                // продолжаем вычитывать команду
                char *ptr = acc.user->read_buffer;
                user_accessor_release(&acc);
                el_async_read(loop, client_socket,
                              ptr, TEMPORARY_BUFFER_SIZE,
                              handler_read, &err);
                if (err.error) {
                    LOG_ERROR("el_async_read: %s", err.message);
                }

                timers_update_by_socket(&server_config.timers, client_socket);
                return;
            }

        }

    } else {
        LOG_ERROR("not found user by socket [%d]", client_socket);
    }
}

void handler_write_msg_of_close_connection_by_timeout(
        event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error)
{
    free(buffer);
    err_t err;
    el_socket_close(el, client_socket, handler_close_socket, &err);
    if (err.error) {
        LOG_ERROR("el_socket_close: %s", err.message);
    }
}

void handler_timer(event_loop* el, int sock, struct timer_event_entry *descriptor) {
    LOG_INFO("timer [%d]", sock);
    if (!timers_is_exist_for_socket(&server_config.timers, sock)) {
        el_timer_free(el, descriptor);
    } else if (timers_is_elapsed_for_socket(&server_config.timers, sock, server_config.timer_period)) {
        // Проверяем наличие пользователя в списке, если его нет, это означает, что он обрабатывается,
        // соответственно таймер не должен срабатывать
        if (!users_list__is_exist(&server_config.users, sock)) {
            return;
        }

        LOG_INFO("%s", "таймер истек, закрываем сокет");
        el_timer_free(el, descriptor);
        err_t err;
        char *response = NULL;
        int len = asprintf(&response, "%d %s",
                           SMTP_CODE_CLOSE_CONNECTION_BY_TIMEOUT,
                           SMTP_CODE_CLOSE_CONNECTION_BY_TIMEOUT_MSG);

        el_async_write(el, sock,response, len, handler_write_msg_of_close_connection_by_timeout, &err);
        if (err.error) {
            LOG_ERROR("el_async_write: %s", err.message);
        }
    }
}

void handler_close_socket(event_loop* el, int sock, err_t *err) {
    user_disconnected(sock);
}


char* smtp_status_to_str(smtp_status status) {
    switch (status) {
        case SMTP_STATUS_ERROR:
            return "SMTP_STATUS_ERROR";
        case SMTP_STATUS_OK:
            return "SMTP_STATUS_OK";
        case SMTP_STATUS_WARNING:
            return "SMTP_STATUS_WARNING";
        case SMTP_STATUS_CONTINUE:
            return "SMTP_STATUS_CONTINUE";
        case SMTP_STATUS_DATA_END:
            return "SMTP_STATUS_DATA_END";
        case SMTP_STATUS_EXIT:
            return "SMTP_STATUS_EXIT";
            break;
    }
    return "NONE";
}

vector_char recipients_to_string(vector_smtp_mailbox *recipients){
    vector_char msg;
    err_t verr;
    VECTOR_INIT(char, &msg, verr);
    for (int i = 0; i < VECTOR_SIZE(recipients); ++i) {
        char *tmp = NULL;
        size_t len = asprintf(&tmp, " -- %s@%s\n", VECTOR(recipients)[i].user_name, VECTOR(recipients)[i].server_name);
        for (int j = 0; j < len; ++j) {
            VECTOR_PUSH_BACK(char, &msg, tmp[j], verr);
        }
        free(tmp);
    }
    return msg;
}

#define ERROR_LOG_AND_CLEANUP(error_, _mark_)                                                       \
do {                                                                                    \
    if ((error_).error) {                                                               \
        if ((error_).error == ERRNO) {                                                  \
            LOG_ERROR("%s : %s", (error_).message, strerror((error_).errno_value));     \
        } else {                                                                        \
            LOG_ERROR("%s", (error_).message);                                          \
        }                                                                               \
        status = false;                                                                 \
        goto _mark_;                                                              \
    }                                                                                   \
} while(0)

char* make_x_headers(vector_smtp_mailbox *rcpts, smtp_mailbox *from) {
    // Создание дополнительных заголовков
    char *x_headers = NULL;
    size_t x_headers_len = 0;
    char *header = NULL;
    size_t header_len = 0;
    // Записываем отрпавителя
    char_make_buf_concat(&x_headers, &x_headers_len, 5, "X-Postman-From: ", from->user_name, "@", from->server_name, "\r\n");
    // Записываем текущее время
    char *timestamp = NULL;
    asprintf(&timestamp, "%ld", time(NULL));
    char_make_buf_concat(&header, &header_len, 4, x_headers, "X-Postman-Date: ", timestamp, "\r\n");
    free(x_headers);
    x_headers = header;
    header = NULL;
    x_headers_len = header_len;
    header_len = 0;
    free(timestamp);
    // Записываем получателей

    char_make_buf_concat(&header, &header_len, 2, x_headers, "X-Postman-To: ");
    free(x_headers);
    x_headers = header;
    header = NULL;
    x_headers_len = header_len;
    header_len = 0;

    int j = 0;
    for (; j < VECTOR_SIZE(rcpts)-1; j++) {
        char_make_buf_concat(&header, &header_len, 5, x_headers, VECTOR(rcpts)[j].user_name, "@", VECTOR(rcpts)[j].server_name, ",");
        free(x_headers);
        x_headers = header;
        x_headers_len = header_len;
        header = NULL;
        header_len = 0;
    }
    char_make_buf_concat(&header, &header_len, 5, x_headers, VECTOR(rcpts)[j].user_name, "@", VECTOR(rcpts)[j].server_name, "\r\n\r\n");
    free(x_headers);
    x_headers = header;
    x_headers_len = header_len;
    header = NULL;
    header_len = 0;
    return x_headers;
}

bool send_mail(smtp_state *smtp) {
    bool status = true;
    smtp_mailbox *sender = smtp_get_sender(smtp);
    char *sender_mailbox_str = NULL;
    asprintf(&sender_mailbox_str, "%s@%s", sender->user_name, sender->server_name);
    char *message = NULL;
    size_t message_len = 0;
    err_t e;
    smtp_move_buffer(smtp, &message, &message_len, &e);

    if (e.error) {
        LOG_ERROR("smtp_move_buffer: %s", e.message);
        status = false;
        goto exit;
    };

    vector_smtp_mailbox *mailboxes = smtp_get_rcpt(smtp);
    if (VECTOR_SIZE(mailboxes) == 0) {
        LOG_WARNING("%s", "vector of recipients is empty");
        free(sender_mailbox_str);
        return false;
    }
    // Запоминаем список получателей чужого и своего сервера
    err_t verr;
    vector_smtp_mailbox self_recipients;
    vector_smtp_mailbox foreign_recipients;
    VECTOR_INIT(struct smtp_mailbox, &self_recipients, verr);
    VECTOR_INIT(struct smtp_mailbox, &foreign_recipients, verr);

    for (int j = 0; j < VECTOR_SIZE(mailboxes); j++) {
        if (strcmp(VECTOR(mailboxes)[j].server_name, server_config.self_server_name) == 0) {
            VECTOR_PUSH_BACK(smtp_mailbox, &self_recipients, VECTOR(mailboxes)[j], verr);
        } else {
            VECTOR_PUSH_BACK(smtp_mailbox, &foreign_recipients, VECTOR(mailboxes)[j], verr);
        }
    }

    vector_smtp_mailbox current_mailbox;
    VECTOR_INIT(struct smtp_mailbox, &current_mailbox, verr);
    for (int j = 0; j < VECTOR_SIZE(&self_recipients); j++) {
        smtp_mailbox *mb = &VECTOR(&self_recipients)[j];

        maildir_server server;
        maildir_user user;
        maildir_message mail;
        char *x_headers = NULL;
        maildir_server_default_init(&server);
        maildir_user_default_init(&user);
        maildir_message_default_init(&mail);


        LOG_INFO("Start sending mail to [%s@%s]", mb->user_name, mb->server_name);

        err_t md_error;
        maildir_get_self_server(&server_config.md, &server, &md_error);
        ERROR_LOG_AND_CLEANUP(md_error, loop_cleanup);

        maildir_server_user(&server, &user, mb->user_name, &md_error);
        if (md_error.error == ERRNO && md_error.errno_value == ENOENT) {
            maildir_server_create_user(&server, &user, mb->user_name, &md_error);
            ERROR_LOG_AND_CLEANUP(md_error, loop_cleanup);
        } else {
            ERROR_LOG_AND_CLEANUP(md_error, loop_cleanup);
        }

        maildir_user_create_message(&user, &mail, sender_mailbox_str, &md_error);
        ERROR_LOG_AND_CLEANUP(md_error, loop_cleanup);

        VECTOR_PUSH_BACK(struct smtp_mailbox, &current_mailbox, *mb, verr);
        x_headers = make_x_headers(&current_mailbox, sender);
        VECTOR_CLEAR(&current_mailbox);

        maildir_message_write(&mail, x_headers, strlen(x_headers), &md_error);
        ERROR_LOG_AND_CLEANUP(md_error, loop_cleanup);
        if (message_len != 0) {
            maildir_message_write(&mail, message, message_len, &md_error);
            ERROR_LOG_AND_CLEANUP(md_error, loop_cleanup);
        }
        maildir_message_finalize(&mail, &md_error);

        LOG_INFO("Mail send from [%s@%s] to [%s@%s]", sender->user_name, sender->server_name,
                 mb->user_name, mb->server_name);

    loop_cleanup:
        maildir_message_free(&mail);
        maildir_user_free(&user);
        maildir_server_free(&server);
        free(x_headers);
        if(status == false) {
            goto exit;
        }
    }
    VECTOR_FREE(&current_mailbox);

    if (VECTOR_SIZE(&foreign_recipients) != 0) {
        // Отправка пиьсма внешним сревреам
        // Распределяем получателей по серверам
        table_t table;
        table_init(&table);
        for (int j = 0; j < VECTOR_SIZE(&foreign_recipients); j++) {
            table_push(&table, VECTOR(&foreign_recipients)[j].server_name, &VECTOR(&foreign_recipients)[j]);
        }

        // Теперь отправлем письма группам
        table_vector_keys keys;
        VECTOR_INIT(table_key_t, &keys, e);
        table_keys(&table, &keys);

        for (int j = 0; j < VECTOR_SIZE(&keys); j++) {
            maildir_server server;
            maildir_user user;
            maildir_message mail;
            maildir_server_default_init(&server);
            maildir_user_default_init(&user);
            maildir_message_default_init(&mail);
            char *x_headers = NULL;

            err_t md_error;
            maildir_get_server(&server_config.md, &server, VECTOR(&keys)[j].value, &md_error);
            if (md_error.error) {
                if (md_error.error == NOT_FOUND) {
                    maildir_create_server(&server_config.md, &server, VECTOR(&keys)[j].value, &md_error);
                } else {
                    ERROR_LOG_AND_CLEANUP(md_error, cleanup2);
                }
            }

            maildir_server_user(&server, &user, "", &md_error);
            ERROR_LOG_AND_CLEANUP(md_error, cleanup2);
            maildir_user_create_message(&user, &mail, sender_mailbox_str, &md_error);
            ERROR_LOG_AND_CLEANUP(md_error, cleanup2);

            struct vector_smtp_mailbox *server_recipients = table_get(&table, VECTOR(&keys)[j].value);
            x_headers = make_x_headers(server_recipients, sender);

            maildir_message_write(&mail, x_headers, strlen(x_headers), &md_error);
            ERROR_LOG_AND_CLEANUP(md_error, cleanup2);
            if (message_len != 0) {
                maildir_message_write(&mail, message, message_len, &md_error);
                ERROR_LOG_AND_CLEANUP(md_error, cleanup2);
            }

            maildir_message_finalize(&mail, &md_error);
            ERROR_LOG_AND_CLEANUP(md_error, cleanup2);

            cleanup2:
            maildir_message_free(&mail);
            maildir_user_free(&user);
            maildir_server_free(&server);
            free(x_headers);

        }

        table_free(&table);
        VECTOR_FREE(&keys);

    }




 exit:
    VECTOR_FREE(&self_recipients);
    VECTOR_FREE(&foreign_recipients);
    free(sender_mailbox_str);
    free(message);
    return status;
}


void check_server_name(user_context* user) {
    smtp_address  addr = smtp_get_hello_addr(&user->smtp);
    if (addr.type == SMTP_ADDRESS_TYPE_IPv4) {
        if (strcmp(user->addr.ip, addr.address) == 0) {
            LOG_INFO("client ip [%s]; ip in hello message [%s] is equal", user->addr.ip, addr.address);
        } else {
            LOG_WARNING("client ip [%s]; ip in hello message [%s] is not equal", user->addr.ip, addr.address);
        }
    } else if (addr.type == SMTP_ADDRESS_TYPE_DOMAIN) {
        char *servername = dns_get_ptr(user->addr.ip);
        if (servername != NULL) {
            LOG_INFO("resolved server name [%s]", servername);
            free(servername);
        } else {

            LOG_WARNING("error resolve server name [%s]", strerror(errno));
        }
    } else {
        LOG_WARNING("resolving this type [%s] not implemented", "-");
    }


}

struct pair handler_smtp(user_context *user, char *message) {
    LOG_INFO("\n======\n%s\n======", message);
    struct pair ret;
    ret.buffer = NULL;
    err_t  error;
    ret.status  = smtp_parse(&user->smtp, message, &ret.buffer, &error);
    if (error.error) {
        LOG_ERROR("smtp_parse: %s", error.message);
    }

    if ( ret.status == SMTP_STATUS_OK ||  ret.status == SMTP_STATUS_WARNING) {
        LOG_INFO("user [%s:%d] smtp [%s] reply : %s", user->addr.ip, user->addr.port,
                 smtp_status_to_str( ret.status), ret.buffer);
        return ret;
    }

    if (ret.status == SMTP_STATUS_CONTINUE) {
        LOG_INFO("user [%s:%d] smtp [%s] reply", user->addr.ip, user->addr.port,
                 smtp_status_to_str(ret.status));
        ret.buffer = NULL; // TODO fix
        return ret;
    }

    if (ret.status == SMTP_STATUS_DATA_END) {
        // Проверяем PTR запись
        check_server_name(user);
        // Отправка письма
        vector_smtp_mailbox *recipients = smtp_get_rcpt(&user->smtp);
        vector_char rcpt = recipients_to_string(recipients);
        smtp_mailbox *mb = smtp_get_sender(&user->smtp);
        LOG_INFO("Send mail from [%s@%s] to: %s\n", mb->user_name, mb->user_name, VECTOR(&rcpt));
        VECTOR_FREE(&rcpt);
        if (send_mail(&user->smtp)) {
            ret.buffer = smtp_make_response(&user->smtp, SMTP_CODE_OK, SMTP_CODE_OK_MSG);
        } else {
            ret.buffer = smtp_make_response(&user->smtp, SMTP_CODE_ERROR_IN_PROCESSING, SMTP_CODE_ERROR_IN_PROCESSING_MSG);
        }
        return ret;
    }

    if (ret.status == SMTP_STATUS_EXIT) {
        char *buff = NULL;
        size_t buff_len = 0;
        char_make_buf_concat(&buff, &buff_len, 3, server_config.self_server_name, " ", SMTP_CODE_CLOSE_CONNECTION_MSG);
        ret.buffer = smtp_make_response(&user->smtp, SMTP_CODE_CLOSE_CONNECTION, buff);
        free(buff);
        return ret;
    }
    LOG_ERROR("%s", "undefined error");
    ret.buffer = smtp_make_response(&user->smtp, SMTP_CODE_ERROR_IN_PROCESSING, SMTP_CODE_ERROR_IN_PROCESSING_MSG);
    return ret;
}

