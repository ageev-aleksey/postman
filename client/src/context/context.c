#include "config/config.h"
#include "util/util.h"
#include "util/network.h"
#include "log/logs.h"
#include "context.h"

sigset_t orig_mask;
pthread_mutex_t mutex_pselect;

void *start_thread();

int pselect_close = 0;

void pselect_close_connection() {
    pselect_close = 1;
}

void free_multiplex_context(multiplex_context context);

int init_context() {
    FD_ZERO(&app_context.master);
    FD_ZERO(&app_context.read_fds);
    FD_ZERO(&app_context.write_fds);

    sigset_t mask = {0};
    struct sigaction act = {0};

    memset(&act, 0, sizeof(act));
    act.sa_handler = exit_handler;
    sigaction(SIGTERM, &act, 0);
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, &orig_mask);

    struct timespec tv = { 0 };
    tv.tv_sec = 5;
    tv.tv_nsec = 0;

    app_context.threads = allocate_memory(sizeof(*app_context.threads));
    for (int i = 0; i < config_context.threads; i++) {
        thread thr = { 0 };
        thr.id_thread = app_context.threads_size;
        thr.multiplex_context = allocate_memory(sizeof(*thr.multiplex_context));
        thr.multiplex_context_size = 0;
        thr.tv = tv;
        app_context.threads_size++;
        app_context.threads = reallocate_memory(app_context.threads, sizeof(*app_context.threads) * app_context.threads_size);
        app_context.threads[i] = thr;
    }

    for (int i = 0; i < app_context.threads_size; i++) {
        pthread_create(&app_context.threads[i].pthread, NULL, start_thread, &app_context.threads[i]);
    }

    return 0;
}

bool is_success_response(smtp_context *smtp) {
    if (is_ready_for_read(smtp->socket_desc)) {
        smtp_response response = get_smtp_response(smtp);
        if (is_smtp_success(response.status_code)) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool is_smtp_sender_ready(smtp_context *smtp) {
    return is_ready_for_write(smtp->socket_desc);
}

message **get_messages(maildir_other_server *server) {
    pthread_mutex_lock(&mutex_pselect);
    message **messages = callocate_memory(server->messages_size, sizeof(**messages));
    for (int i = 0; i < server->messages_size; i++) {
        messages[i] = read_message(server->message_full_file_names[i]);
    }
    pthread_mutex_unlock(&mutex_pselect);
    return messages;
}

void *start_thread(thread *thr) {

    maildir_main *maildir = init_maildir(config_context.maildir.path);

    while (true) {

        int ret = pselect(app_context.fdmax, &app_context.read_fds,
                          &app_context.write_fds, NULL, &thr->tv, &orig_mask);

        if (is_interrupt()) {
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
            LOG_INFO("Таймаут подключения", NULL);

            pthread_mutex_lock(&mutex_pselect);
            update_maildir(maildir);
            output_maildir(maildir);

            for (int j = 0, i = 0; j < maildir->servers_size; j++) {
                if (maildir->servers[j].messages_size > 0) {
                    if ((i + j) % app_context.threads_size == thr->id_thread) {
                        smtp_context *cont = smtp_connect(maildir->servers[j].server, "25", NULL);
                        thr->multiplex_context = reallocate_memory(thr->multiplex_context,
                                                                   sizeof(*thr->multiplex_context) * (i + 1));
                        thr->multiplex_context[i].smtp_context = *cont;
                        thr->multiplex_context[i].server = &maildir->servers[j];
                        thr->multiplex_context[i].iteration = 0;
                        thr->multiplex_context[i].select_message = NULL;
                        add_socket_to_context(thr->multiplex_context[i].smtp_context.socket_desc);
                        thr->multiplex_context_size++;
                        i++;
                        free(cont);
                    }
                }
            }
            pthread_mutex_unlock(&mutex_pselect);
        } else {
            for (int i = 0; i < thr->multiplex_context_size; i++) {
                if (thr->multiplex_context[i].select_message == NULL) {
                    thr->multiplex_context->select_message = get_messages(thr->multiplex_context[i].server)[0];
                }
                message *mess = thr->multiplex_context->select_message;
                switch (thr->multiplex_context[i].smtp_context.state_code) {
                    case SMTP_CONNECT:
                        if (is_success_response(&thr->multiplex_context[i].smtp_context) &&
                            is_smtp_sender_ready(&thr->multiplex_context[i].smtp_context)) {
                            smtp_send_helo(&thr->multiplex_context[i].smtp_context);
                        }
                        break;
                    case SMTP_HELO:
                        if (is_success_response(&thr->multiplex_context[i].smtp_context) &&
                            is_smtp_sender_ready(&thr->multiplex_context[i].smtp_context)) {
                            smtp_send_mail(&thr->multiplex_context[i].smtp_context, mess->from[0]);
                            thr->multiplex_context[i].iteration++;
                        }
                        break;
                    case SMTP_MAIL:
                        if (is_success_response(&thr->multiplex_context[i].smtp_context) &&
                            is_smtp_sender_ready(&thr->multiplex_context[i].smtp_context)) {
                            if (thr->multiplex_context[i].iteration < mess->from_size) {
                                smtp_send_mail(&thr->multiplex_context[i].smtp_context, mess->from[thr->multiplex_context[i].iteration]);
                                thr->multiplex_context[i].iteration++;
                            } else {
                                thr->multiplex_context[i].iteration = 0;
                                smtp_send_rcpt(&thr->multiplex_context[i].smtp_context, mess->to[0]);
                                thr->multiplex_context[i].iteration++;
                            }
                        }
                        break;
                    case SMTP_RCPT:
                        if (is_success_response(&thr->multiplex_context[i].smtp_context) &&
                            is_smtp_sender_ready(&thr->multiplex_context[i].smtp_context)) {
                            if (thr->multiplex_context[i].iteration < mess->to_size) {
                                smtp_send_rcpt(&thr->multiplex_context[i].smtp_context, mess->to[thr->multiplex_context[i].iteration]);
                                thr->multiplex_context[i].iteration++;
                            } else {
                                thr->multiplex_context[i].iteration = 0;
                                smtp_send_data(&thr->multiplex_context[i].smtp_context);
                            }
                        }
                        break;
                    case SMTP_DATA:
                        if (is_success_response(&thr->multiplex_context[i].smtp_context) &&
                            is_smtp_sender_ready(&thr->multiplex_context[i].smtp_context)) {
                            smtp_send_message(&thr->multiplex_context[i].smtp_context, mess->strings[thr->multiplex_context[i].iteration]);
                            thr->multiplex_context[i].iteration++;
                        }
                        break;
                    case SMTP_MESSAGE:
                        if (is_smtp_sender_ready(&thr->multiplex_context[i].smtp_context)) {
                            if (thr->multiplex_context[i].iteration < mess->strings_size) {
                                smtp_send_message(&thr->multiplex_context[i].smtp_context, mess->strings[thr->multiplex_context[i].iteration]);
                                thr->multiplex_context[i].iteration++;
                            } else {
                                thr->multiplex_context[i].iteration = 0;
                                smtp_send_end_message(&thr->multiplex_context[i].smtp_context);
                            }
                        }
                        break;
                    case SMTP_END_MESSAGE:
                        if (is_success_response(&thr->multiplex_context[i].smtp_context) &&
                            is_smtp_sender_ready(&thr->multiplex_context[i].smtp_context)) {
                            if (thr->multiplex_context[i].server->messages_size > 1) {
                                smtp_send_rset(&thr->multiplex_context[i].smtp_context);
                                thr->multiplex_context[i].smtp_context.state_code = SMTP_HELO;
                            } else {
                                smtp_send_quit(&thr->multiplex_context[i].smtp_context);
                                remove_socket_from_context(thr->multiplex_context[i].smtp_context.socket_desc);
                                remove_message_server(thr->multiplex_context[i].server, mess);
                                thr->multiplex_context_size--;
                                free_multiplex_context(thr->multiplex_context[i]);
                                for (int k = i; k < thr->multiplex_context_size; k++) {
                                    thr->multiplex_context[k] = thr->multiplex_context[k + 1];
                                }
                                continue;
                            }
                            thr->multiplex_context[i].select_message = NULL;
                            remove_message_server(thr->multiplex_context[i].server, mess);
                        }
                        break;
                    case SMTP_RSET:
                    case SMTP_QUIT:
                    case SMTP_EHLO:
                    case SMTP_INVALID:
                        LOG_WARN("undefined SMTP state", NULL);
                        break;
                }
                reset_socket_to_context(thr->multiplex_context[i].smtp_context.socket_desc);
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

int add_socket_to_context(int socket) {
    FD_ZERO(&app_context.master);
    FD_SET(socket, &app_context.master);
    app_context.read_fds = app_context.master;
    app_context.write_fds = app_context.master;

    if (socket > 0) {
        app_context.fdmax = socket + 1;
    }
    app_context.fd_size++;
    return 0;
}

int reset_socket_to_context(int socket) {
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
    FD_CLR(socket, &app_context.master);
    FD_ZERO(&app_context.master);
    app_context.read_fds = app_context.master;
    app_context.write_fds = app_context.master;
    app_context.fd_size--;
    app_context.fdmax--;
    shutdown(socket, SHUT_RDWR);
    close(socket);
    return 0;
}

bool is_ready_for_read(int socket) {
    return FD_ISSET(socket, &app_context.read_fds);
}

bool is_ready_for_write(int socket) {
    return FD_ISSET(socket, &app_context.write_fds);
}
