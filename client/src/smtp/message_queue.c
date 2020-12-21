#include <sys/queue.h>
#include <pthread.h>
#include <signal.h>
#include <config.h>
#include "util.h"
#include "maildir.h"
#include "logs.h"
#include "message_queue.h"
#include "smtp-client.h"

typedef struct node {
    message *data;
    TAILQ_ENTRY(node) nodes;
} node;

TAILQ_HEAD(message_queue, node) head;

pthread_t *thread_message_queue = NULL;
pthread_mutex_t mutex_queue;

void init_message_queue() {
    TAILQ_INIT(&head);
}

void push_message(message *value) {
    pthread_mutex_lock(&mutex_queue);
    node *new = allocate_memory(sizeof(node));
    new->data = value;
    TAILQ_INSERT_TAIL(&head, new, nodes);
    pthread_mutex_unlock(&mutex_queue);
}

message* pop_message() {
    pthread_mutex_lock(&mutex_queue);
    node *old = TAILQ_FIRST(&head);
    TAILQ_REMOVE(&head, old, nodes);
    message* data = old->data;
    free(old);
    pthread_mutex_unlock(&mutex_queue);
    return data;
}

bool is_message_queue_empty() {
    return TAILQ_EMPTY(&head);
}

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

        // TODO: здесь должно быть мультиплексирование и полная реализация последовательности общения с SMTP-серверам
        // TODO: Конечный автомат наглядно реализуется здесь
        if (maildir != NULL) {
            for (int i = 0; i < maildir->servers.messages_size; i++) {
                message *mes = read_message(maildir->servers.message_full_file_names[i]);
                string_tokens tokens_mail = split(mes->to, ",");
                for (int j = 0; j < tokens_mail.count_tokens; j++) {
                    string_tokens tokens_name_domain = split(tokens_mail.tokens[j].chars, "@");
                    if (tokens_name_domain.count_tokens == 2) {
                        smtp_context **contexts = malloc(sizeof **contexts);
                        smtp_context *context = smtp_open(tokens_name_domain.tokens[1].chars, "25", contexts);

                        if (context != NULL && context->state_code == OK) {
                            smtp_mail(context, mes->from, "");
                            smtp_rcpt(context, tokens_mail.tokens[j].chars, "");
                            smtp_data(context);
                            char *from;
                            asprintf(&from, "FROM:%s\r\n", mes->from);
                            smtp_message(context, from);
                            free(from);

                            char *to;
                            asprintf(&to, "TO:%s\r\n", mes->to);
                            smtp_message(context, to);
                            free(to);

                            for (int k = 0; k < mes->strings_size; k++) {
                                smtp_message(context, mes->strings[k]);
                            }
                            smtp_send_dot(context);
                        }
                    }
                }
                remove_message(maildir, mes);
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
        init_message_queue();
        pthread_create(thread_message_queue, NULL, message_queue_func, NULL);
    } else {
        LOG_WARN("Очередь сообщений уже инициализирована", NULL);
    }
}

void message_queue_finalize() {
    pthread_kill(*thread_message_queue, SIGINT);
    free(thread_message_queue);
}