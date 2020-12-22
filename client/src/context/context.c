#include "util.h"
#include "context.h"
#include "smtp.h"
#include "logs.h"

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

    struct timespec tv = { 0 };
    tv.tv_sec = 5;
    tv.tv_nsec = 0;

    thr_context.pthreads = allocate_memory(sizeof(*thr_context.pthreads));
    pthread_create(thr_context.pthreads, NULL, start_thread, &tv);
//
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

void *start_thread(struct timespec *tv) {

    TAILQ_HEAD(message_queue, node) head;

    char *server[1];
    server[0] = "yandex.ru";

    char *mail[2];
    mail[0] = "ovchinnikovasv73@gmail.com";
    mail[1] = "vladovchinnikov950@gmail.com";

    char *rcpt[1];
    rcpt[0] = "wedf97@yandex.ru";

    smtp_context **contexts = callocate_memory(1, sizeof(**contexts));

    int j = 0;
    int count_mail = 0;
    while (true) {
        if (contexts[j] != NULL) {
            add_socket_to_context(contexts[j]->socket_desc);
        }

        // TODO: читать письма из maildir тут

        int ret = pselect(app_context.fdmax, &app_context.read_fds,
                          &app_context.write_fds, NULL, tv, &orig_mask);

        if (pselect_exit_request) {
            break;
        }

        if (ret == -1) {
            LOG_ERROR("Ошибка в мультиплексировании", NULL);
        } else if (ret == 0) {
            LOG_INFO("Таймаут подключения", NULL);
            smtp_context *cont = smtp_connect(server[j], "25", NULL);
            contexts[j] = cont;
        } else {
            for (int i = 0; i < 1; i++) {
                switch (contexts[i]->state_code) {
                    case SMTP_CONNECT:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            smtp_send_helo(contexts[i]);
                        }
                        break;
                    case SMTP_HELO:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            smtp_send_mail(contexts[i], mail[0]);
                        }
                        break;
                    case SMTP_MAIL:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            if (count_mail < 1) {
                                smtp_send_mail(contexts[i], mail[1]);
                                count_mail++;
                            } else {
                                smtp_send_rcpt(contexts[i], rcpt[0]);
                            }
                        }
                        break;
                    case SMTP_RCPT:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            smtp_send_data(contexts[i]);
                        }
                        break;
                    case SMTP_DATA:
                        if (is_success_response(contexts[i]) && is_smtp_sender_ready(contexts[i])) {
                            smtp_send_message(contexts[i], "\r\ntodo: message");
                        }
                        break;
                    case SMTP_MESSAGE:
                        if (is_smtp_sender_ready(contexts[i])) {
                            smtp_send_end_message(contexts[i]);
                        }
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
                        }
                        break;
                    case SMTP_RSET:
                        contexts[i]->state_code = SMTP_CONNECT;
                        break;
                    case SMTP_QUIT:
                        LOG_INFO("Соединение с %s успешно закрыто.", get_addr_by_socket(contexts[i]->socket_desc));
                        free(contexts[i]);
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
    return 0;
}

bool is_ready_for_read(int socket) {
    return FD_ISSET(socket, &app_context.read_fds);
}

bool is_ready_for_write(int socket) {
    return FD_ISSET(socket, &app_context.write_fds);
}
