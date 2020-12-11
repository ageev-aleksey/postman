#include "server/global_context.h"
#include "log/context.h"
#include "server/users_list.h"
#include "event_loop/event_loop.h"
#include "smtp/state.h"
#include "smtp/response.h"
#include <regex.h>
#include <string.h>
#include <errno.h>
#include <maildir/user.h>
#include <maildir/message.h>

#define RE_CONFIG_DOMAIN "([[:alnum:]_-]+\.)*[[:alnum:]_-]+"


bool server_config_init(const char *path) {
    config_t cfg;
    config_init(&cfg);

    if(!config_read_file(&cfg, path))
    {
        LOG_ERROR("%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return false;
    }
    int port = 0;
    if (!config_lookup_int(&cfg, "server.port", &port)) {
        LOG_ERROR("%s [%s]", "not found server.port field in file", path);
        return false;
    }
    if (port > 65535) {
        LOG_ERROR("%s [%d]", "invalid port value", port);
        return false;
    }
    const char *host = NULL;
    if (!config_lookup_string(&cfg, "server.host", &host)) {
        LOG_ERROR("%s [%s]", "not found server.host field in file", path);
        return false;
    }
    // TODO (ageev) добавить проверку на корректный ip
//    regex_t reg_domain;
//    regcomp(&reg_domain, )
    const char *domain = NULL;
    if (!config_lookup_string(&cfg, "server.domain", &domain)) {
        LOG_ERROR("%s [%s]", "not found server.domain field in file", path);
        return false;
    }
    const char *maildir_path = NULL;
    if (!config_lookup_string(&cfg, "server.maildir_path", &maildir_path)) {
        LOG_ERROR("%s [%s]", "not found server.maildir_path field in file", path);
        return false;
    }

    server_config.ip = malloc(sizeof(char) * strlen(host));
    server_config.self_server_name = malloc(sizeof(char) * strlen(domain));
    strcpy(server_config.ip, host);
    strcpy(server_config.self_server_name, domain);
    server_config.log_file_path = NULL;
    server_config.port = port;
    server_config.hello_msg_size =
            asprintf(&server_config.hello_msg, "220 %s The Postman Server v%d.%d\r\n",
                     domain, POSTMAN_VERSION_MAJOR, POSTMAN_VERSION_MINOR);
    users_list__init(&server_config.users);
    err_t  error;
    if (!maildir_init(&server_config.md, maildir_path, &error)) {
        LOG_ERROR("maildir: %s", error.message);
        return false;
    }

    LOG_INFO("\nConfig loaded: \n -- host: %s\n -- port: %d\n -- domain: %s\n -- maildir: %s",
             server_config.ip,
             server_config.port,
             server_config.self_server_name,
             maildir_path);
    config_destroy(&cfg);
    return true;
}

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

void server_config_free() {
    free(server_config.self_server_name);
    free(server_config.ip);
    free(server_config.log_file_path);
    free(server_config.hello_msg);
    users_list__free(&server_config.users);
}


void handler_hello_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error);

void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error) {
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
}
void handler_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error) {
    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }
    
    user_accessor acc;
    if(users_list__user_find_by_sock(&server_config.users, &acc, client_socket)) {

        err_t verr;
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

        err_t err;
        el_async_read(el, client_socket,
                      acc.user->read_buffer, TEMPORARY_BUFFER_SIZE,
                      handler_read, &err);
        if (err.error) {
            LOG_ERROR("el_async_read: %s", err.message);
        }

        user_accessor_release(&acc);

    } else {
        LOG_ERROR("Not found user by socket [%d]", client_socket);
    }
}

void user_disconnected(int sock) {
    user_accessor acc;
    if (users_list__user_find_by_sock(&server_config.users, &acc, sock)) {
        LOG_INFO("user close connection [%s:%d]", acc.user->addr.ip, acc.user->addr.port);
        users_list__delete_user(&acc);
        user_free(acc.user);
    } else {
        LOG_ERROR("user bys socket [%d] not found", sock);
    }
}

void handler_hello_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error) {
    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }
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

}


void handler_read(event_loop *loop, int client_socket, char *buffer, int size, client_status status, err_t error) {
    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }
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
        int entry_index = find_first_entry_str(VECTOR(&acc.user->buffer), SMTP_COMMAND_END,
                                               VECTOR_SIZE(&acc.user->buffer), SMTP_COMMAND_END_LEN);

        if (entry_index != -1) {
            // Обрабатываем команду
            VECTOR(&acc.user->buffer)[entry_index+SMTP_COMMAND_END_LEN] = '\0';
            err_t smtp_err;
            char *reply = handler_smtp(acc.user, VECTOR(&acc.user->buffer));
            VECTOR_CLEAR(&acc.user->buffer);
             if (reply != NULL) {
                 el_async_write(loop, acc.user->socket, reply, strlen(reply),
                                handler_write, &err);
                 if (err.error) {
                     LOG_ERROR("el_async_write: %s", err.message);
                 }
             } else {
                // нечего отвечать, продолжаем вычитывать остальные команды
                 el_async_read(loop, client_socket,
                               acc.user->read_buffer, TEMPORARY_BUFFER_SIZE,
                               handler_read, &err);
             }

        } else {
            // продолжаем вычитывать команду
            el_async_read(loop, client_socket,
                          acc.user->read_buffer, TEMPORARY_BUFFER_SIZE,
                          handler_read, &err);
            if (err.error) {
                LOG_ERROR("el_async_read: %s", err.message);
            }
        }

        user_accessor_release(&acc);

    } else {
        LOG_ERROR("not found user by socket [%d]", client_socket);
    }
}

void handler_timer(event_loop* el, int socket, struct timer_event_entry *descriptor) {
    client_addr client = {0};
    LOG_INFO("timer for client: %s:%d", client.ip, client.port);
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

#define ERROR_LOG_AND_CLEANUP(error_)                                                       \
do {                                                                                    \
    if ((error_).error) {                                                               \
        if ((error_).error == ERRNO) {                                                  \
            LOG_ERROR("%s : %s", (error_).message, strerror((error_).errno_value));     \
        } else {                                                                        \
            LOG_ERROR("%s", (error_).message);                                          \
        }                                                                               \
        status = false;                                                                 \
        goto loop_cleanup;                                                              \
    }                                                                                   \
} while(0)

bool send_mail(smtp_state *smtp) {
    // 1. ПРоходимся по всем получателям
    // 2. Для каждого получателя инициализируем структуры сервера
    //      -- Если домен сервера соответсвтует домену из конфиграционного файла
    //          то это означает, что письма адресовано нашему серверу
    //      -- Иначе, письмо адресовано внешнему серверу
    // 3. Инициализируем структуру пользователя
    // 4. Инициализируем структуру сообщения
    // 5. Записываем сообщение
    bool status = true;
    smtp_mailbox *sender = smtp_get_sender(smtp);
    char *sender_mailbox_str = NULL;
    asprintf(&sender_mailbox_str, "%s@%s", sender->user_name, sender->server_name);

    vector_smtp_mailbox *mailboxes = smtp_get_rcpt(smtp);
    if (VECTOR_SIZE(mailboxes) == 0) {
        LOG_WARNING("%s", "vector of recipients is empty");
        free(sender_mailbox_str);
        return false;
    }

    char *message = NULL;
    size_t message_len = 0;
    err_t e;
    smtp_move_buffer(smtp, &message, &message_len, &e);
    if (e.error) {
        LOG_ERROR("smtp_move_buffer: %s", e.message);
        status = false;
        goto exit;
    }
    for (int i = 0; i < VECTOR_SIZE(mailboxes); i++) {
        smtp_mailbox *mb = &VECTOR(mailboxes)[i];
        maildir_server server;
        maildir_user user;
        maildir_message mail;
        maildir_server_default_init(&server);
        maildir_user_default_init(&user);
        maildir_message_default_init(&mail);


        LOG_INFO("Start sending mail to [%s@%s]", mb->user_name, mb->server_name);

        err_t md_error;
        if ( strcmp(mb->server_name, server_config.self_server_name) == 0) {
            // Письмо адресовано пользователю нашего сервера
            maildir_get_self_server(&server_config.md, &server, &md_error);
            ERROR_LOG_AND_CLEANUP(md_error);
        } else {
            // Письмо адресовано внешнему серверу
            maildir_create_server(&server_config.md, &server, mb->server_name, &md_error);
            if (md_error.error == ERRNO && md_error.errno_value == EEXIST) {
                maildir_get_server_by_name(&server_config.md, &server, mb->server_name, &md_error);
                ERROR_LOG_AND_CLEANUP(md_error);
            } else {
                ERROR_LOG_AND_CLEANUP(md_error);
            }
        }


        maildir_server_user(&server, &user, mb->user_name, &md_error);
        if (md_error.error == ERRNO && md_error.errno_value == ENOENT) {
            maildir_server_create_user(&server, &user, mb->user_name, &md_error);
            ERROR_LOG_AND_CLEANUP(md_error);
        } else {
            ERROR_LOG_AND_CLEANUP(md_error);
        }


        maildir_user_create_message(&user, &mail, sender_mailbox_str, &md_error);
        ERROR_LOG_AND_CLEANUP(md_error);


        maildir_message_write(&mail, message, message_len, &md_error);
        ERROR_LOG_AND_CLEANUP(md_error);
        maildir_message_finalize(&mail, &md_error);


        LOG_INFO("Mail send from [%s@%s] to [%s@%s]", sender->user_name, sender->server_name,
                 mb->user_name, mb->user_name);


    loop_cleanup:
        maildir_message_free(&mail);
        maildir_user_free(&user);
        maildir_server_free(&server);
        if (!status) {
            goto exit;
        }
    }

exit:
    free(sender_mailbox_str);
    free(message);
    return status;
}

char *handler_smtp(user_context *user, char *message) {
    char *reply = NULL;
    err_t  error;
    smtp_status status  = smtp_parse(&user->smtp, message, &reply, &error);
    if (error.error) {
        LOG_ERROR("smtp_parse: %s", error.message);
    }

    if (status == SMTP_STATUS_OK || status == SMTP_STATUS_WARNING) {
        LOG_INFO("user [%s:%d] smtp [%s] reply : %s", user->addr.ip, user->addr.port,
                 smtp_status_to_str(status), reply);
        return reply;
    }

    if (status == SMTP_STATUS_CONTINUE) {
        LOG_INFO("user [%s:%d] smtp [%s] reply", user->addr.ip, user->addr.port,
                 smtp_status_to_str(status));
        return NULL;
    }

    if (status == SMTP_STATUS_DATA_END) {
        // Отправка письма
        vector_smtp_mailbox *recipients = smtp_get_rcpt(&user->smtp);
        vector_char rcpt = recipients_to_string(recipients);
        smtp_mailbox *mb = smtp_get_sender(&user->smtp);
        LOG_INFO("Send mail from [%s@%s] to: %s\n", mb->user_name, mb->user_name, VECTOR(&rcpt));
        VECTOR_FREE(&rcpt);
        if (send_mail(&user->smtp)) {
            reply = smtp_make_response(&user->smtp, SMTP_CODE_OK, SMTP_CODE_OK_MSG);
        } else {
            reply = smtp_make_response(&user->smtp, SMTP_CODE_ERROR_IN_PROCESSING, SMTP_CODE_ERROR_IN_PROCESSING_MSG);
        }
        return reply;
    }

    if (status == SMTP_STATUS_EXIT) {
        reply = smtp_make_response(&user->smtp, SMTP_CODE_ERROR_IN_PROCESSING, SMTP_CODE_ERROR_IN_PROCESSING_MSG);
        return reply;
    }
    LOG_ERROR("%s", "undefined error");
    reply = smtp_make_response(&user->smtp, SMTP_CODE_ERROR_IN_PROCESSING, SMTP_CODE_ERROR_IN_PROCESSING_MSG);
    return reply;
}

