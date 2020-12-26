#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <memory.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <unistd.h>
#include "smtp/smtp.h"
#include "maildir/maildir.h"

typedef struct multiplex_context {
    smtp_context smtp_context;
    smtp_response response;
    maildir_other_server server;
    message *select_message;
    int iteration;
} multiplex_context;

typedef struct thread {
    fd_set master;
    fd_set read_fds;
    fd_set write_fds;
    int fdmax;
    int fd_size;

    int id_thread;
    pthread_t pthread;
    multiplex_context *multiplex_context;
    int multiplex_context_size;
    struct timespec tv;
    bool is_stopped;
} thread;

typedef struct context {
    thread *threads;
    int threads_size;
} context;

extern context app_context;

int init_context();
int add_socket_to_context(int socket, thread *thr);
int remove_socket_from_context(int socket, thread *thr);
int reset_socket_to_context(thread *thr);
void exit_handler(int sig);
bool is_ready_for_read(int socket, thread *thr);
bool is_ready_for_write(int socket, thread *thr);

void destroy_context();