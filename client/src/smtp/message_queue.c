#include <sys/queue.h>
#include <pthread.h>
#include <signal.h>
#include <config.h>
#include "util.h"
#include "maildir.h"
#include "logs.h"
#include "message_queue.h"

typedef struct node {
    message *data;
    TAILQ_ENTRY(node) nodes;
} node;

pthread_t *thread_message_queue = NULL;
pthread_mutex_t mutex_queue;

TAILQ_HEAD(message_queue, node) head;

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

        if (maildir != NULL) {
            for (int i = 0; i < maildir->servers.messages_size; i++) {
                message *mes = read_message(maildir->servers.message_full_file_names[i]);
                push_message(mes);
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
    init_message_queue();
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