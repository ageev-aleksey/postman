#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <memory.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <unistd.h>

typedef struct thread_context {
    pthread_t *pthreads;
    int threads_size;
} thread_context;

typedef struct context {
    fd_set master;
    fd_set read_fds;
    fd_set write_fds;
    int fdmax;
    thread_context thr_context;
} context;

static context app_context;

int init_context();
int start_pselect(struct timespec tv);
int add_socket_to_context(int socket);
int remove_socket_from_context(int socket);
bool is_ready_for_read(int socket);
bool is_ready_for_write(int socket);