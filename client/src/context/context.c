#include <config.h>
#include "maildir.h"
#include "util.h"
#include "context.h"
#include "smtp.h"
#include "logs.h"
#include "message_queue.h"

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
    tv.tv_sec = 10;
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

message **get_messages(maildir_main *maildir) {
    message **messages = callocate_memory(maildir->servers.messages_size, sizeof(**messages));
    if (maildir != NULL) {
        for (int i = 0; i < maildir->servers.messages_size; i++) {
            messages[i] = read_message(maildir->servers.message_full_file_names[i]);
        }
    }

    return messages;
}

// TODO: переделать на одно-подключение равно несколько писем
_Noreturn void *start_thread(struct timespec *tv) {

    smtp_context **contexts = callocate_memory(1, sizeof(**contexts));

    maildir_main *maildir = init_maildir(config_context.maildir.path);

    int size = 0;
    int count = 0;
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
            message **messages = get_messages(maildir);

            for (int j = 0; j < maildir->servers.messages_size; j++) {
                for (int i = 0; i < messages[j]->addresses_size; i++) {
                    smtp_context *cont = smtp_connect(messages[j]->addresses[i], "25", NULL);
                    contexts[i] = cont;
                    cont->message = messages[j];
                    add_socket_to_context(contexts[i]->socket_desc);
                    size++;
                }
            }
        } else {
            for (int i = 0; i < size; i++) {
                message *mess = (message *) contexts[i]->message;
                switch (contexts[i]->state_code) {
                    case SMTP_CONNECT:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            smtp_send_helo(contexts[i]);
                        }
                        add_socket_to_context(contexts[i]->socket_desc);
                        break;
                    case SMTP_HELO:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            smtp_send_mail(contexts[i], mess->from[0]);
                            count++;
                        }
                        add_socket_to_context(contexts[i]->socket_desc);
                        break;
                    case SMTP_MAIL:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            if (count < mess->from_size) {
                                smtp_send_mail(contexts[i], mess->from[count]);
                                count++;
                            } else {
                                count = 0;
                                smtp_send_rcpt(contexts[i], mess->to[0]);
                                count++;
                            }
                        }
                        add_socket_to_context(contexts[i]->socket_desc);
                        break;
                    case SMTP_RCPT:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            if (count < mess->to_size) {
                                smtp_send_mail(contexts[i], mess->to[count]);
                                count++;
                            } else {
                                count = 0;
                                smtp_send_data(contexts[i]);
                            }
                        }
                        add_socket_to_context(contexts[i]->socket_desc);
                        break;
                    case SMTP_DATA:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            smtp_send_message(contexts[i], mess->strings[count]);
                            count++;
                        }
                        add_socket_to_context(contexts[i]->socket_desc);
                        break;
                    case SMTP_MESSAGE:
                        if (is_smtp_sender_ready(contexts[i])) {
                            if (count < mess->strings_size) {
                                smtp_send_message(contexts[i], mess->strings[count]);
                                count++;
                            } else {
                                count = 0;
                                smtp_send_end_message(contexts[i]);
                            }
                        }
                        add_socket_to_context(contexts[i]->socket_desc);
                        break;
                    case SMTP_END_MESSAGE:
                        // TODO: если есть еще письма переходим на reset, иначе разрываем соединение
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            // TODO: условие перехода на reset вместо false здесь
                            if (false) {
                                smtp_send_rset(contexts[i]);
                            } else {
                                smtp_send_quit(contexts[i]);
                            }
                            remove_message(maildir, mess);
                        }
                        add_socket_to_context(contexts[i]->socket_desc);
                        break;
                    case SMTP_RSET:
                        contexts[i]->state_code = SMTP_CONNECT;
                        add_socket_to_context(contexts[i]->socket_desc);
                        break;
                    case SMTP_QUIT:
                        LOG_INFO("Соединение с %s успешно закрыто.", get_addr_by_socket(contexts[i]->socket_desc));
                        remove_socket_from_context(contexts[i]->socket_desc);
                        size--;
                        free(contexts[i]);
                        contexts[i] = NULL;
                        for (int k = i; k < size - 1; k++) {
                            contexts[k] = contexts[k + 1];
                        }
                        // TODO: удалить SMTP-context из списка
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
