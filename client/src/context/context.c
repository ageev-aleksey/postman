#include "config/config.h"
#include "util/util.h"
#include "util/network.h"
#include "log/logs.h"
#include "context.h"

sigset_t orig_mask;

void *start_thread();

int pselect_close = 0;

void pselect_close_connection() {
    pselect_close = 1;
}

void free_multiplex_context(multiplex_context context);

int init_context() {
    sigset_t mask = {0};
    struct sigaction act = {0};

    memset(&act, 0, sizeof(act));
    act.sa_handler = exit_handler;
    sigaction(SIGTERM, &act, 0);
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, &orig_mask);

    struct timespec tv = {0};
    tv.tv_sec = 1;
    tv.tv_nsec = 0;

    app_context.threads = allocate_memory(sizeof(*app_context.threads));
    for (int i = 0; i < config_context.threads; i++) {
        thread thr = {0};
        thr.id_thread = app_context.threads_size;
        thr.multiplex_context = allocate_memory(sizeof(*thr.multiplex_context));
        thr.multiplex_context_size = 0;
        thr.is_stopped = false;
        thr.tv = tv;
        app_context.threads_size++;
        app_context.threads = reallocate_memory(app_context.threads,
                                                sizeof(*app_context.threads) * app_context.threads_size);
        app_context.threads[i] = thr;
    }

    for (int i = 0; i < app_context.threads_size; i++) {
        pthread_create(&app_context.threads[i].pthread, NULL, start_thread, &app_context.threads[i]);
    }

    return 0;
}

void set_response_to_context(multiplex_context *context, thread *thr) {
    if (is_ready_for_read(context->smtp_context.socket_desc, thr)) {
        context->response = get_smtp_response(&context->smtp_context);
    }
}

bool is_smtp_sender_ready(smtp_context *smtp, thread *thr) {
    return is_ready_for_write(smtp->socket_desc, thr);
}

message *get_message(maildir_other_server *server) {
    message *message = NULL;
    if (server->messages_size > 0) {
        message = read_message(server->message_full_file_names[0]);
    }
    return message;
}

void connect_handler(multiplex_context *context, thread *thr) {
    status_code status = context->response.status_code;
    if (status == NOT_ANSWER) {
        set_response_to_context(context, thr);
        return;
    }

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            smtp_send_helo(&context->smtp_context);
        }
    }
}

void helo_handler(multiplex_context *context, thread *thr) {
    status_code status = context->response.status_code;
    if (status == NOT_ANSWER) {
        set_response_to_context(context, thr);
        return;
    }

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            smtp_send_mail(&context->smtp_context, context->select_message->from[0]);
            context->iteration++;
        }
    }
}

void mail_handler(multiplex_context *context, thread *thr) {
    status_code status = context->response.status_code;
    if (status == NOT_ANSWER) {
        set_response_to_context(context, thr);
        return;
    }

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            if (context->iteration < context->select_message->from_size) {
                smtp_send_mail(&context->smtp_context, context->select_message->from[context->iteration]);
                context->iteration++;
            } else {
                context->iteration = 0;
                smtp_send_rcpt(&context->smtp_context, context->select_message->to[0]);
                context->iteration++;
            }
        }
    }
}

void rcpt_handler(multiplex_context *context, thread *thr) {
    status_code status = context->response.status_code;
    if (status == NOT_ANSWER) {
        set_response_to_context(context, thr);
        return;
    }

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            if (context->iteration < context->select_message->to_size) {
                smtp_send_rcpt(&context->smtp_context,
                               context->select_message->to[context->iteration]);
                context->iteration++;
            } else {
                context->iteration = 0;
                smtp_send_data(&context->smtp_context);
            }
        }
    }
}

void data_handler(multiplex_context *context, thread *thr) {
    status_code status = context->response.status_code;
    if (status == NOT_ANSWER) {
        set_response_to_context(context, thr);
        return;
    }

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            smtp_send_message(&context->smtp_context,
                              context->select_message->strings[context->iteration]);
            context->iteration++;
        }
    }
}

void message_handler(multiplex_context *context, thread *thr) {
    status_code status = context->response.status_code;
    if (status == NOT_ANSWER) {
        set_response_to_context(context, thr);
        return;
    }

    if (is_smtp_success(status)) {
        if (context->iteration < context->select_message->strings_size) {
            smtp_send_message(&context->smtp_context,
                              context->select_message->strings[context->iteration]);
            context->iteration++;
        } else {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            context->iteration = 0;
            smtp_send_end_message(&context->smtp_context);
        }
    }
}

void end_message_handler(multiplex_context *context, thread *thr) {
    status_code status = context->response.status_code;
    if (status == NOT_ANSWER) {
        set_response_to_context(context, thr);
        return;
    }

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            if (context->server->messages_size > 1) {
                smtp_send_rset(&context->smtp_context);
                context->smtp_context.state_code = SMTP_HELO;
            } else {
                smtp_send_quit(&context->smtp_context);
                remove_socket_from_context(context->smtp_context.socket_desc, thr);
                remove_message_server(context->server, context->select_message);
                free_multiplex_context(*context);
                for (int i = 0; i < thr->multiplex_context_size; i++) {
                    if (&thr->multiplex_context[i] == context) {
                        for (int j = i; j < thr->multiplex_context_size - 1; j++) {
                            thr->multiplex_context[j] = thr->multiplex_context[j + 1];
                        }
                    }
                }
                thr->multiplex_context_size--;
                return;
            }
            context->select_message = NULL;
            remove_message_server(context->server, context->select_message);
        }
    }
}

bool is_contains(maildir_other_server server, thread *thr) {
    for (int i = 0; i < thr->multiplex_context_size; i++) {
        if (strcmp(server.server, thr->multiplex_context[i].server->server) == 0) {
            return true;
        }
    }
    return false;
}

void *start_thread(thread *thr) {

    maildir_main *maildir = init_maildir(config_context.maildir.path);

    while (true) {

        reset_socket_to_context(thr);

        int ret = pselect(thr->fdmax, &thr->read_fds,
                          &thr->write_fds, NULL, &thr->tv, &orig_mask);

        if (is_interrupt()) {
            free(thr->multiplex_context);
            thr->is_stopped = true;
            break;
        }

        if (pselect_close) {
            LOG_INFO("Закрыто соединение", NULL);
            continue;
        }

        if (ret == -1) {
            LOG_ERROR("Ошибка в мультиплексировании", NULL);
            break;
        } else if (ret == 0) {
            update_maildir(maildir);

            for (int j = 0, i = 0; j < maildir->servers_size; j++) {
                if (maildir->servers[j].messages_size > 0) {
                    if ((i + j) % app_context.threads_size == thr->id_thread) {
                        smtp_context *cont = smtp_connect(maildir->servers[j].server, "25", NULL);
                        thr->multiplex_context = reallocate_memory(thr->multiplex_context,
                                                                   sizeof(*thr->multiplex_context) * (i + 1));
                        thr->multiplex_context[i].smtp_context.socket_desc = cont->socket_desc;
                        thr->multiplex_context[i].smtp_context.state_code = cont->state_code;
                        thr->multiplex_context[i].server = &maildir->servers[j];
                        thr->multiplex_context[i].iteration = 0;
                        thr->multiplex_context[i].select_message = NULL;
                        thr->multiplex_context[i].response.status_code = NOT_ANSWER;
                        thr->multiplex_context_size++;
                        i++;
                        add_socket_to_context(cont->socket_desc, thr);
                        free(cont);
                    }
                }
            }
        } else {
            for (int i = 0; i < thr->multiplex_context_size; i++) {
                if (thr->multiplex_context[i].select_message == NULL) {
                    thr->multiplex_context[i].select_message = get_message(thr->multiplex_context[i].server);
                }

                switch (thr->multiplex_context[i].smtp_context.state_code) {
                    case SMTP_CONNECT:
                        connect_handler(&thr->multiplex_context[i], thr);
                        break;
                    case SMTP_HELO:
                        helo_handler(&thr->multiplex_context[i], thr);
                        break;
                    case SMTP_MAIL:
                        mail_handler(&thr->multiplex_context[i], thr);
                        break;
                    case SMTP_RCPT:
                        rcpt_handler(&thr->multiplex_context[i], thr);
                        break;
                    case SMTP_DATA:
                        data_handler(&thr->multiplex_context[i], thr);
                        break;
                    case SMTP_MESSAGE:
                        message_handler(&thr->multiplex_context[i], thr);
                        break;
                    case SMTP_END_MESSAGE:
                        end_message_handler(&thr->multiplex_context[i], thr);
                        break;
                    case SMTP_RSET:
                    case SMTP_QUIT:
                    case SMTP_EHLO:
                    case SMTP_INVALID:
                        LOG_WARN("undefined SMTP state", NULL);
                        break;
                }
            }
        }
    }
}

void free_multiplex_context(multiplex_context multiplex_cont) {
    for (int i = 0; i < multiplex_cont.server->messages_size + 1; i++) {
        free(multiplex_cont.server->message_full_file_names[i]);
    }
    free(multiplex_cont.server->message_full_file_names);
    free(multiplex_cont.server->directory);
    free(multiplex_cont.server->server);
}

int add_socket_to_context(int socket, thread *thr) {
    pthread_mutex_lock(&thr->mutex);
    FD_ZERO(&thr->master);
    FD_SET(socket, &thr->master);
    thr->read_fds = thr->master;
    thr->write_fds = thr->master;

    if (socket > 0) {
        thr->fdmax = socket + 1;
    }
    thr->fd_size++;
    pthread_mutex_unlock(&thr->mutex);
    return 0;
}

int reset_socket_to_context(thread *thr) {
    pthread_mutex_lock(&thr->mutex);
    FD_ZERO(&thr->master);
    FD_ZERO(&thr->read_fds);
    FD_ZERO(&thr->write_fds);
    for (int i = 0; i < thr->multiplex_context_size; i++) {
        FD_SET(thr->multiplex_context[i].smtp_context.socket_desc, &thr->master);
    }
    thr->read_fds = thr->master;
    thr->write_fds = thr->master;
    pthread_mutex_unlock(&thr->mutex);
    return 0;
}

int remove_socket_from_context(int socket, thread *thr) {
    pthread_mutex_lock(&thr->mutex);
    FD_CLR(socket, &thr->master);
    FD_ZERO(&thr->master);
    thr->read_fds = thr->master;
    thr->write_fds = thr->master;
    thr->fd_size--;
    thr->fdmax--;
    shutdown(socket, SHUT_RDWR);
    close(socket);
    pthread_mutex_unlock(&thr->mutex);
    return 0;
}

bool is_ready_for_read(int socket, thread *thr) {
    return FD_ISSET(socket, &thr->read_fds);
}

bool is_ready_for_write(int socket, thread *thr) {
    return FD_ISSET(socket, &thr->write_fds);
}

int destroy_context() {
    int count_threads_stopped = 0;
    while (true) {
        if (count_threads_stopped == app_context.threads_size) {
            free(app_context.threads);
            break;
        }
        for (int i = 0; i < app_context.threads_size; i++) {
            if (app_context.threads[i].is_stopped) {
                count_threads_stopped++;
            }
        }
    }
}