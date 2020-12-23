#include "config/config.h"
#include "maildir/maildir.h"
#include "util/util.h"
#include "util/network.h"
#include "smtp/smtp.h"
#include "log/logs.h"
#include "context.h"

typedef struct multiplex_context {
    smtp_context *smtp_context;
    maildir_other_server *server;
    int iteration;
} multiplex_context;

int pselect_exit_request = 0;
sigset_t orig_mask;

void *start_thread();

void pselect_exit_handle(int sig) {
    pselect_exit_request = 1;
}

int init_context() {
    FD_ZERO(&app_context.master);
    FD_ZERO(&app_context.read_fds);
    FD_ZERO(&app_context.write_fds);

    thread_context thr_context = {0};
    thr_context.threads_size = 0;

    sigset_t mask = {0};
    struct sigaction act = {0};

    memset(&act, 0, sizeof(act));
    act.sa_handler = pselect_exit_handle;
    sigaction(SIGTERM, &act, 0);
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, &orig_mask);

    struct timespec tv = {0};
    tv.tv_sec = 1;
    tv.tv_nsec = 0;

    thr_context.pthreads = allocate_memory(sizeof(*thr_context.pthreads));
    pthread_create(thr_context.pthreads, NULL, start_thread, &tv);

    // TODO: сделать множество потоков
//    if (thread_message_queue == NULL) {
//        thread_message_queue = allocate_memory(sizeof(*thread_message_queue));
//        pthread_create(thread_message_queue, NULL, message_queue_func, NULL);
//    } else {
//        LOG_WARN("Очередь сообщений уже инициализирована", NULL);
//    }

    return 0;
}

bool is_success_response(smtp_context *context) {
    if (FD_ISSET(context->socket_desc, &app_context.read_fds)) {
        smtp_response response = get_smtp_response(context);
        if (is_smtp_success(response.status_code)) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool is_smtp_sender_ready(smtp_context *context) {
    if (FD_ISSET(context->socket_desc, &app_context.write_fds)) {
        return true;
    }
    return false;
}

message **get_messages(maildir_other_server *server) {
    message **messages = callocate_memory(server->messages_size, sizeof(**messages));
    for (int i = 0; i < server->messages_size; i++) {
        messages[i] = read_message(server->message_full_file_names[i]);
    }

    return messages;
}

// TODO: сделать множественный рефакторинг, объединить в единый контекст
void *start_thread(struct timespec *tv) {

    multiplex_context *contexts[10];

    maildir_main *maildir = init_maildir(config_context.maildir.path);

    int size = 0;
    while (true) {

        int ret = pselect(app_context.fdmax, &app_context.read_fds,
                          &app_context.write_fds, NULL, tv, &orig_mask);

        if (pselect_exit_request) {
            break;
        }

        if (ret == -1) {
            LOG_ERROR("Ошибка в мультиплексировании", NULL);
        } else if (ret == 0) {
            LOG_INFO("Таймаут подключения", NULL);

            update_maildir(maildir);
            output_maildir(maildir);
            // TODO: update maildir

            for (int j = 0; j < maildir->servers_size; j++) {
                if (maildir->servers[j].messages_size > 0) {
                    smtp_context *cont = smtp_connect(maildir->servers[j].server, "25", NULL);
                    contexts[j] = allocate_memory(sizeof(**contexts));
                    contexts[j]->smtp_context = cont;
                    contexts[j]->server = &maildir->servers[j];
                    contexts[j]->iteration = 0;
                    add_socket_to_context(contexts[j]->smtp_context->socket_desc);
                }
            }
        } else {
            for (int i = 0; i < maildir->servers_size; i++) {
                if (contexts[i] == NULL) {
                    continue;
                }
                message *mess = get_messages(contexts[i]->server)[0];
                switch (contexts[i]->smtp_context->state_code) {
                    case SMTP_CONNECT:
                        if (is_success_response(contexts[i]->smtp_context) && is_smtp_sender_ready(contexts[i]->smtp_context)) {
                            smtp_send_helo(contexts[i]->smtp_context);
                        }
                        add_socket_to_context(contexts[i]->smtp_context->socket_desc);
                        break;
                    case SMTP_HELO:
                        if (is_success_response(contexts[i]->smtp_context) && is_smtp_sender_ready(contexts[i]->smtp_context)) {
                            smtp_send_mail(contexts[i]->smtp_context, mess->from[0]);
                            contexts[i]->iteration++;
                        }
                        add_socket_to_context(contexts[i]->smtp_context->socket_desc);
                        break;
                    case SMTP_MAIL:
                        if (is_success_response(contexts[i]->smtp_context) && is_smtp_sender_ready(contexts[i]->smtp_context)) {
                            if (contexts[i]->iteration < mess->from_size) {
                                smtp_send_mail(contexts[i]->smtp_context, mess->from[contexts[i]->iteration]);
                                contexts[i]->iteration++;
                            } else {
                                contexts[i]->iteration = 0;
                                smtp_send_rcpt(contexts[i]->smtp_context, mess->to[0]);
                                contexts[i]->iteration++;
                            }
                        }
                        add_socket_to_context(contexts[i]->smtp_context->socket_desc);
                        break;
                    case SMTP_RCPT:
                        if (is_success_response(contexts[i]->smtp_context) && is_smtp_sender_ready(contexts[i]->smtp_context)) {
                            if (contexts[i]->iteration < mess->to_size) {
                                smtp_send_mail(contexts[i]->smtp_context, mess->to[contexts[i]->iteration]);
                                contexts[i]->iteration++;
                            } else {
                                contexts[i]->iteration = 0;
                                smtp_send_data(contexts[i]->smtp_context);
                            }
                        }
                        add_socket_to_context(contexts[i]->smtp_context->socket_desc);
                        break;
                    case SMTP_DATA:
                        if (is_success_response(contexts[i]->smtp_context) && is_smtp_sender_ready(contexts[i]->smtp_context)) {
                            smtp_send_message(contexts[i]->smtp_context, mess->strings[contexts[i]->iteration]);
                            contexts[i]->iteration++;
                        }
                        add_socket_to_context(contexts[i]->smtp_context->socket_desc);
                        break;
                    case SMTP_MESSAGE:
                        if (is_smtp_sender_ready(contexts[i]->smtp_context)) {
                            if (contexts[i]->iteration < mess->strings_size) {
                                smtp_send_message(contexts[i]->smtp_context, mess->strings[contexts[i]->iteration]);
                                contexts[i]->iteration++;
                            } else {
                                contexts[i]->iteration = 0;
                                smtp_send_end_message(contexts[i]->smtp_context);
                            }
                        }
                        add_socket_to_context(contexts[i]->smtp_context->socket_desc);
                        break;
                    case SMTP_END_MESSAGE:
                        if (is_success_response(contexts[i]->smtp_context) && is_smtp_sender_ready(contexts[i]->smtp_context)) {
                            if (contexts[i]->server->messages_size != 0) {
                                smtp_send_rset(contexts[i]->smtp_context);
                            } else {
                                smtp_send_quit(contexts[i]->smtp_context);
                            }
                            remove_message_server(contexts[i]->server, mess);
                        }
                        add_socket_to_context(contexts[i]->smtp_context->socket_desc);
                        break;
                    case SMTP_RSET:
                        contexts[i]->smtp_context->state_code = SMTP_CONNECT;
                        add_socket_to_context(contexts[i]->smtp_context->socket_desc);
                        break;
                    case SMTP_QUIT:
                        LOG_INFO("Соединение с %s успешно закрыто.", get_addr_by_socket(contexts[i]->smtp_context->socket_desc));
                        remove_socket_from_context(contexts[i]->smtp_context->socket_desc);
                        size--;
                        free(contexts[i]);
                        contexts[i] = NULL;
                        for (int k = i; k < size - 1; k++) {
                            contexts[k] = contexts[k + 1];
                        }
                        break;
                    case SMTP_EHLO:
                    case SMTP_INVALID:
                        LOG_WARN("undefined SMTP state", NULL);
                        break;
                }
            }
        }
    }
}

int add_socket_to_context(int socket) {
    FD_ZERO(&app_context.master);
    FD_SET(socket, &app_context.master);
    app_context.read_fds = app_context.master;
    app_context.write_fds = app_context.master;

    if (socket > 0) {
        app_context.fdmax = socket + 1;
    }

    return 0;
}

int remove_socket_from_context(int socket) {
    shutdown(socket, SHUT_RDWR);
    close(socket);
    FD_CLR(socket, &app_context.master);
    FD_ZERO(&app_context.master);
    FD_ZERO(&app_context.read_fds);
    FD_ZERO(&app_context.write_fds);
    return 0;
}

bool is_ready_for_read(int socket) {
    return FD_ISSET(socket, &app_context.read_fds);
}

bool is_ready_for_write(int socket) {
    return FD_ISSET(socket, &app_context.write_fds);
}
