//
// Created by nrx on 27.10.2020.
//

#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H

#include "vector.h"

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/queue.h>
#include <time.h>



typedef enum d_log_level {
    ERROR_LEVEL = 0,
    WARNING_LEVEL = 1,
    INFO_LEVEL = 2,
    DEBUG_LEVEL = 3,
} log_level;

typedef struct d_log_message {
    log_level  level;
    time_t time;
    const char* file;
    const char* function;
    size_t line;
    char *message;
    TAILQ_ENTRY(d_log_message) entries;
} log_message;

typedef void (*log_handler)(log_message);

typedef struct d_printer_t {
    log_level  level;
    log_handler handler;
} printer_t;

VECTOR_DECLARE(log_vector_printers, printer_t);


TAILQ_HEAD(d_log_message_queue, d_log_message);
typedef struct d_log_message_queue log_message_queue;

typedef struct d_log_context {
    log_level  enabled_level;
    log_message_queue *messages;
    log_vector_printers *printers;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
    size_t timeout;
    bool is_run;
} log_context;





log_context *GLOBAL_LOG_CONTEXT;

bool log_init(log_context **context);
bool log_file_writer(log_context *context, log_level level, const char *path);
bool log_console_writer(log_context *context, log_level level);
bool log_timeout(log_context *context, size_t ms);
bool log_make_message(log_message *message, log_level level, const char *pattern, const char *ptr, ...);
bool log_write(log_context *context, log_message *message);
log_level log_get_level(log_context *context);
void log_free(log_context *context);


#define LOG_MAKE_MESSAGE(log_level_, format_, ...) \
({                                     \
    log_message *message = s_malloc(sizeof(log_message), NULL); \
    if(message != NULL) {              \
        asprintf(&message->message, format_, __VA_ARGS__);                             \
        message->file = __FILE__;      \
        message->function = __FUNCTION__;                       \
        message->line = __LINE__;                  \
        message->level = (log_level_); \
    }              \
    message;            \
})


#define LOG_ERROR(format_, ...) \
do {                   \
  if ((log_get_level(GLOBAL_LOG_CONTEXT) >= ERROR_LEVEL)) { \
    log_message *message = LOG_MAKE_MESSAGE(ERROR_LEVEL, (format_),  __VA_ARGS__); \
    if (message == NULL) {                                  \
        fprintf(stderr, "LOG FATAL ERROR: error allocating memory for message struct\n");                   \
    }                          \
    log_write(GLOBAL_LOG_CONTEXT, message);          \
  }  \
} while(0)                       \



#endif //SERVER_CONTEXT_H
