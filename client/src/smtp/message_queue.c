#include <sys/queue.h>
#include <pthread.h>
#include <signal.h>
#include <config.h>
#include "util.h"
#include "maildir.h"
#include "logs.h"
#include "message_queue.h"
#include "smtp-client.h"

pthread_t *thread_message_queue = NULL;
pthread_mutex_t mutex_queue;

_Noreturn void *message_queue_func() {
    if (interrupt_thread_local) {
        pthread_exit((void *) 0);
    }

    sigset_t set, orig;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGKILL);
    sigaddset(&set, SIGTERM);
    sigemptyset(&orig);
    pthread_sigmask(SIG_BLOCK, &set, &orig);

    if (interrupt_thread_local) {
        pthread_sigmask(SIG_SETMASK, &orig, 0);
        pthread_exit((void *) 0);
    }

    struct timespec ts;
    ts.tv_sec = 5;
    ts.tv_nsec = 0;

    maildir_main *maildir = init_maildir(config_context.maildir.path);

    while (true) {
        nanosleep(&ts, &ts);

        // TODO: добавление в очередь писем, которые потом будут обрабатываться в контексте отправки в SMTP
        if (maildir != NULL) {
            for (int i = 0; i < maildir->servers.messages_size; i++) {
                message *mes = read_message(maildir->servers.message_full_file_names[i]);

//                        smtp_context *context = smtp_open(tokens_name_domain.tokens[1].chars, "25");
//
//                        if (context != NULL && context->state_code == OK) {
//                            smtp_mail(context, mes->from, "");
//                            smtp_rcpt(context, tokens_mail.tokens[j].chars, "");
//                            smtp_data(context);
//                            char *from;
//                            asprintf(&from, "FROM:%s\r\n", mes->from);
//                            smtp_message(context, from);
//                            free(from);
//
//                            char *to;
//                            asprintf(&to, "TO:%s\r\n", mes->to);
//                            smtp_message(context, to);
//                            free(to);
//
//                            for (int k = 0; k < mes->strings_size; k++) {
//                                smtp_message(context, mes->strings[k]);
//                            }
//                            smtp_send_dot(context);
            }
        }
        update_maildir(maildir);
        output_maildir(maildir);


        if (interrupt_thread_local) {
            printf("Попытка приостановить поток");
            pthread_sigmask(SIG_SETMASK, &orig, 0);
            break;
        }
    }

    printf("Поток остановлен");
    pthread_exit((void *) 0);
}

void start_message_queue() {
    if (thread_message_queue == NULL) {
        thread_message_queue = allocate_memory(sizeof(*thread_message_queue));
        pthread_create(thread_message_queue, NULL, message_queue_func, NULL);
    } else {
        LOG_WARN("Очередь сообщений уже инициализирована", NULL);
    }
}

void message_queue_finalize() {
    pthread_kill(*thread_message_queue, SIGINT);
    free(thread_message_queue);
}