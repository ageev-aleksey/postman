#include "config/config.h"
#include "util/util.h"
#include "util/network.h"
#include "log/logs.h"
#include "context.h"

sigset_t orig_mask;

pthread_mutex_t maildir_mutex;
pthread_mutex_t server_thread_mutex;

void *start_thread();

void free_multiplex_context(multiplex_context *multiplex_cont) {
    if (multiplex_cont != NULL) {
        for (int i = 0; i < multiplex_cont->server.messages_size; i++) {
            free(multiplex_cont->server.message_full_file_names[i]);
        }
        free(multiplex_cont->server.message_full_file_names);
        free(multiplex_cont->server.directory);
        free(multiplex_cont->server.server);
    }
}

int init_context() {
    struct timespec tv = {0};
    tv.tv_sec = 3;
    tv.tv_nsec = 0;

    app_context.threads_size = 0;
    app_context.threads = allocate_memory(sizeof(thread) * config_context.threads);
    for (int i = 0; i < config_context.threads; i++) {
        thread thr = {0};
        thr.id_thread = i;
        thr.multiplex_context_size = 0;
        thr.multiplex_context = NULL;
        thr.is_stopped = false;
        thr.tv = tv;
        app_context.threads_size++;
        app_context.threads[i] = thr;
    }

    for (int i = 0; i < app_context.threads_size; i++) {
        pthread_create(&app_context.threads[i].pthread, NULL, start_thread, &app_context.threads[i]);
    }

    return 0;
}

status_code get_response_to_context(multiplex_context *context, thread *thr) {
    if (is_ready_for_read(context->smtp_context.socket_desc, thr)) {
        smtp_response response = get_smtp_response(&context->smtp_context);
        free(response.message);

        return response.status_code;
    }
    return context->response.status_code;
}

bool is_smtp_sender_ready(smtp_context *smtp, thread *thr) {
    return is_ready_for_write(smtp->socket_desc, thr);
}

message *get_message(maildir_other_server *server) {
    pthread_mutex_lock(&maildir_mutex);
    message *message = NULL;
    if (server->messages_size > 0) {
        message = read_message(server->message_full_file_names[0]);
    }
    pthread_mutex_unlock(&maildir_mutex);
    return message;
}

void remove_multiplex_context(multiplex_context *context, thread *thr) {
    remove_socket_from_context(context->smtp_context.socket_desc, thr);
    pthread_mutex_lock(&maildir_mutex);
    pthread_mutex_unlock(&maildir_mutex);
    for (int i = 0; i < thr->multiplex_context_size; i++) {
        if (&thr->multiplex_context[i] == context) {
            for (int j = i; j < thr->multiplex_context_size - 1; j++) {
                thr->multiplex_context[j] = thr->multiplex_context[j + 1];
            }
        }
    }
    free_multiplex_context(context);
    thr->multiplex_context_size--;
}

void connect_handler(multiplex_context *context, thread *thr) {
    status_code status = get_response_to_context(context, thr);
    if (status == NOT_ANSWER || status == context->response.status_code) {
        return;
    }
    context->response.status_code = status;

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            smtp_send_helo(&context->smtp_context);
        }
    } else if (is_smtp_4xx_error(status) || status == UNDEFINED_ERROR) {
        smtp_send_quit(&context->smtp_context);
        remove_multiplex_context(context, thr);
    } else if (is_smtp_5xx_error(status)) {
        smtp_send_quit(&context->smtp_context);
        remove_message_server(&context->server, context->select_message);
        remove_multiplex_context(context, thr);
    }
}

void helo_handler(multiplex_context *context, thread *thr) {
    status_code status = get_response_to_context(context, thr);
    if (status == NOT_ANSWER) {
        return;
    }
    context->response.status_code = status;

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            smtp_send_mail(&context->smtp_context, context->select_message->from[0]);
            context->iteration++;
        }
    } else if (is_smtp_4xx_error(status) || status == UNDEFINED_ERROR) {
        smtp_send_quit(&context->smtp_context);
        remove_multiplex_context(context, thr);
    } else if (is_smtp_5xx_error(status)) {
        smtp_send_quit(&context->smtp_context);
        remove_message_server(&context->server, context->select_message);
        remove_multiplex_context(context, thr);
    }
}

void mail_handler(multiplex_context *context, thread *thr) {
    status_code status = get_response_to_context(context, thr);
    if (status == NOT_ANSWER) {
        return;
    }
    context->response.status_code = status;

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
    } else if (is_smtp_4xx_error(status) || status == UNDEFINED_ERROR) {
        smtp_send_quit(&context->smtp_context);
        remove_multiplex_context(context, thr);
    } else if (is_smtp_5xx_error(status)) {
        smtp_send_quit(&context->smtp_context);
        remove_message_server(&context->server, context->select_message);
        remove_multiplex_context(context, thr);
    }
}

void rcpt_handler(multiplex_context *context, thread *thr) {
    status_code status = get_response_to_context(context, thr);
    if (status == NOT_ANSWER) {
        return;
    }
    context->response.status_code = status;

    if (is_smtp_success(status) || status == SMTP_USER_MAILBOX_WAS_UNAVAILABLE) {
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
    } else if (is_smtp_4xx_error(status) || status == UNDEFINED_ERROR) {
        smtp_send_quit(&context->smtp_context);
        remove_multiplex_context(context, thr);
    } else if (is_smtp_5xx_error(status)) {
        smtp_send_quit(&context->smtp_context);
        remove_message_server(&context->server, context->select_message);
        remove_multiplex_context(context, thr);
    }
}

void data_handler(multiplex_context *context, thread *thr) {
    status_code status = get_response_to_context(context, thr);
    if (status == NOT_ANSWER) {
        return;
    }
    context->response.status_code = status;

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            smtp_send_message(&context->smtp_context,
                              context->select_message->strings[context->iteration]);
            context->iteration++;
        }
    } else if (is_smtp_4xx_error(status) || status == UNDEFINED_ERROR) {
        smtp_send_quit(&context->smtp_context);
        remove_multiplex_context(context, thr);
    } else if (is_smtp_5xx_error(status)) {
        smtp_send_quit(&context->smtp_context);
        remove_message_server(&context->server, context->select_message);
        remove_multiplex_context(context, thr);
    }
}

void message_handler(multiplex_context *context, thread *thr) {
    status_code status = get_response_to_context(context, thr);
    if (status == NOT_ANSWER) {
        return;
    }
    context->response.status_code = status;

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
    } else if (is_smtp_4xx_error(status) || status == UNDEFINED_ERROR) {
        smtp_send_quit(&context->smtp_context);
        remove_multiplex_context(context, thr);
    } else if (is_smtp_5xx_error(status)) {
        smtp_send_quit(&context->smtp_context);
        remove_message_server(&context->server, context->select_message);
        remove_multiplex_context(context, thr);
    }
}

void end_message_handler(multiplex_context *context, thread *thr) {
    status_code status = get_response_to_context(context, thr);
    if (status == NOT_ANSWER) {
        return;
    }
    context->response.status_code = status;

    if (is_smtp_success(status)) {
        if (is_smtp_sender_ready(&context->smtp_context, thr)) {
            context->response.status_code = NOT_ANSWER;
            context->response.message = NULL;
            if (context->server.messages_size > 1) {
                smtp_send_rset(&context->smtp_context);
                context->smtp_context.state_code = SMTP_HELO;
            } else {
                smtp_send_quit(&context->smtp_context);
                remove_message_server(&context->server, context->select_message);
                remove_multiplex_context(context, thr);
                return;
            }
            context->select_message = NULL;
            pthread_mutex_lock(&maildir_mutex);
            pthread_mutex_unlock(&maildir_mutex);
        }
    } else if (is_smtp_4xx_error(status) || status == UNDEFINED_ERROR) {
        smtp_send_quit(&context->smtp_context);
        remove_multiplex_context(context, thr);
    } else if (is_smtp_5xx_error(status)) {
        smtp_send_quit(&context->smtp_context);
        remove_message_server(&context->server, context->select_message);
        remove_multiplex_context(context, thr);
    }
}

bool is_contains(maildir_other_server server) {
    pthread_mutex_lock(&server_thread_mutex);
    for (int i = 0; i < app_context.threads_size; i++) {
        for (int j = 0; j < app_context.threads[i].multiplex_context_size; j++) {
            if (strcmp(server.server, app_context.threads[i].multiplex_context[j].server.server) == 0) {
                return true;
            }
        }
    }
    pthread_mutex_unlock(&server_thread_mutex);
    return false;
}

void free_thread(thread *thr) {
    if (thr != NULL) {
        for (int i = 0; i < thr->multiplex_context_size; i++) {
            free_multiplex_context(&thr->multiplex_context[i]);
        }
        free(thr->multiplex_context);
    }
}

void *start_thread(thread *thr) {
    if (is_interrupt()) {
        thr->is_stopped = true;
        pthread_exit((void *) 0);
    }

    LOG_INFO("Старт потока %d (%lu)", thr->id_thread, thr->pthread);

    thr->multiplex_context = allocate_memory(sizeof(multiplex_context));

    sigset_t mask = {0};
    struct sigaction act = {0};

    memset(&act, 0, sizeof(act));
    act.sa_handler = exit_handler;
    sigaction(SIGINT, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, &orig_mask);

    maildir_main *maildir = NULL;

    while (true) {
        if (is_interrupt()) {
            pthread_sigmask(SIG_SETMASK, &orig_mask, 0);
            thr->is_stopped = true;
            break;
        }

        finalize_maildir(maildir);
        maildir = NULL;

        reset_socket_to_context(thr);

        int ret = pselect(thr->fdmax, &thr->read_fds,
                          &thr->write_fds, NULL, &thr->tv, &orig_mask);

        if (is_interrupt()) {
            pthread_sigmask(SIG_SETMASK, &orig_mask, 0);
            thr->is_stopped = true;
            break;
        }

        if (ret == -1) {
            LOG_ERROR("Ошибка в мультиплексировании", NULL);
            break;
        } else if (ret == 0) {
            maildir = init_maildir(config_context.maildir.path);
            read_maildir_servers(maildir);

            for (int j = 0, i = 0; j < maildir->servers_size; j++) {
                if (maildir->servers[j].messages_size > 0) {
                    if ((i + j) % app_context.threads_size == thr->id_thread && !is_contains(maildir->servers[j])) {
                        smtp_context *cont = smtp_connect(maildir->servers[j].server, "25", NULL);
                        if (cont->state_code == SMTP_INVALID) {
                            remove_all_message_server(&maildir->servers[j]);
                            free(cont);
                            break;
                        }

                        thr->multiplex_context = reallocate_memory(thr->multiplex_context,
                                                                   sizeof(multiplex_context) * (i + 1));
                        thr->multiplex_context[i].smtp_context.socket_desc = cont->socket_desc;
                        thr->multiplex_context[i].smtp_context.state_code = cont->state_code;

                        thr->multiplex_context[i].server.messages_size = maildir->servers[j].messages_size;
                        asprintf(&thr->multiplex_context[i].server.directory, "%s", maildir->servers[j].directory);
                        asprintf(&thr->multiplex_context[i].server.server, "%s", maildir->servers[j].server);
                        thr->multiplex_context[i].server.message_full_file_names = allocate_memory(sizeof(char *)
                                                                                                   *
                                                                                                   maildir->servers[j].messages_size);

                        for (int k = 0; k < maildir->servers[j].messages_size; k++) {
                            asprintf(&thr->multiplex_context[i].server.message_full_file_names[k], "%s",
                                     maildir->servers[j].message_full_file_names[k]);
                        }

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
                    message *mess;
                    if ((mess = get_message(&thr->multiplex_context[i].server)) == NULL) {
                        smtp_send_quit(&thr->multiplex_context[i].smtp_context);
                        remove_message_server(&thr->multiplex_context[i].server, mess);
                        remove_multiplex_context(&thr->multiplex_context[i], thr);
                        continue;
                    } else {
                        thr->multiplex_context[i].select_message = mess;
                    }
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
                        LOG_WARN("invalid SMTP state", NULL);
                        remove_multiplex_context(&thr->multiplex_context[i], thr);
                        break;
                }
            }
        }
    }

    finalize_maildir(maildir);
    free_thread(thr);
    pthread_exit((void *) 0);
}

int add_socket_to_context(int socket, thread *thr) {
    FD_ZERO(&thr->master);
    FD_SET(socket, &thr->master);
    thr->read_fds = thr->master;
    thr->write_fds = thr->master;

    if (socket > 0) {
        thr->fdmax = socket + 1;
    }
    thr->fd_size++;
    return 0;
}

int reset_socket_to_context(thread *thr) {
    FD_ZERO(&thr->master);
    FD_ZERO(&thr->read_fds);
    FD_ZERO(&thr->write_fds);
    for (int i = 0; i < thr->multiplex_context_size; i++) {
        FD_SET(thr->multiplex_context[i].smtp_context.socket_desc, &thr->master);
    }
    thr->read_fds = thr->master;
    thr->write_fds = thr->master;
    return 0;
}

int remove_socket_from_context(int socket, thread *thr) {
    FD_CLR(socket, &thr->master);
    FD_ZERO(&thr->master);
    thr->read_fds = thr->master;
    thr->write_fds = thr->master;
    thr->fd_size--;
    thr->fdmax--;
    shutdown(socket, SHUT_RDWR);
    close(socket);
    return 0;
}

bool is_ready_for_read(int socket, thread *thr) {
    return FD_ISSET(socket, &thr->read_fds);
}

bool is_ready_for_write(int socket, thread *thr) {
    return FD_ISSET(socket, &thr->write_fds);
}

void destroy_context() {
    int threads_size = app_context.threads_size;
    LOG_INFO("Общее количество потоков: %d", threads_size);
    for (int i = 0; i < threads_size; i++) {
        LOG_INFO("Освобождение потока %d", app_context.threads[i].id_thread);
        pthread_join(app_context.threads[i].pthread, NULL);
    }

    free(app_context.threads);
}