//
// Created by nrx on 27.10.2020.
//

#include "log/context.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#define MS_TO_NS 1000000;

#define COEFFICIENT_DOWNTIME 10

#define LOG_ERROR_MSG "FATAL ERROR IN LOGGER: "

#define STYLE_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[36m"
#define COLOR_GRAY "\x1b[90m"


static const char *debug_level_color[] = {
    /*ERROR_LEVEL*/COLOR_RED,
    /*WARNING_LEVEL*/COLOR_YELLOW,
    /*INFO_LEVEL*/COLOR_GREEN,
    /*DEBUG_LEVEL*/COLOR_BLUE,
};

static const char *debug_level_str[] = {
        /*ERROR_LEVEL*/"ERROR",
        /*WARNING_LEVEL*/"WARNING",
        /*INFO_LEVEL*/"INFO",
        /*DEBUG_LEVEL*/"DEBUG",
};


VECTOR_DECLARE(vector_messages, log_message);



void pr_printer_in_console(log_message msg) {
    char buf[16];
    struct tm *time = localtime (&msg.time);
    if (time == NULL) {
        fprintf(stderr, "FATAL ERROR IN LOGGER: error converting time_t in struct tm");
        return;
    }
    size_t len = strftime(buf, sizeof(buf), "%H:%M:%S", time);
    buf[len] = '\0';
            /*TIME DEBUG_LEVEL [file:function:line] : MESSAGE*/
    fprintf(stderr, STYLE_RESET "%s %s%s" COLOR_GRAY " [%s:%s:%zu] " STYLE_RESET ": %s \n",
           buf, debug_level_color[msg.level], debug_level_str[msg.level],
           msg.file, msg.function, msg.line,
           msg.message);
}

void pr_printer_in_file(log_message msg, FILE *file) {
    char buf[16];
    struct tm *time = localtime (&msg.time);
    if (time == NULL) {
        fprintf(stderr, "FATAL ERROR IN LOGGER: error converting time_t in struct tm");
        return;
    }
    size_t len = strftime(buf, sizeof(buf), "%H:%M:%S", time);
    buf[len] = '\0';

    fprintf(file, "%s %s [%s:%s:%zu]: %s\n", buf, debug_level_str[msg.level],
            msg.file, msg.function, msg.line,msg.message);
    fflush(file);
}

int pr_log_message_comparator(const void *first, const void *second) {
    return ((log_message*)first)->time > ((log_message*)second)->time ? 1 : -1;
}

bool pr_log_is_run(log_context *context) {
    bool is;
    pthread_mutex_lock(&context->mutex_property);
    is = context->is_run;
    pthread_mutex_unlock(&context->mutex_property);
    return is;
}

bool pr_log_wait_and_extract_messages(log_context *context, log_message_queue **msg, size_t *size) {
    log_message_queue *new_queue = s_malloc(sizeof(log_message_queue), NULL);
    if(new_queue == NULL) {
        fprintf(stderr, LOG_ERROR_MSG "error malloc for new log_queue");
        return NULL;
    }
    TAILQ_INIT(new_queue);

    pthread_mutex_lock(&context->mutex_messages);
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = context->timeout * MS_TO_NS;
    while (TAILQ_EMPTY(context->messages)) {
        pthread_cond_timedwait(&context->cv, &context->mutex_messages, &timeout);
        if (!pr_log_is_run(context)) {
            pthread_mutex_unlock(&context->mutex_messages);
            free(new_queue);
            return false;
        }
    }
    *msg = context->messages;
    *size = context->num_messages;
    context->messages = new_queue;
    context->num_messages = 0;
    pthread_mutex_unlock(&context->mutex_messages);

    return true;
}

vector_messages* pr_log_convert_msgq_to_msgv(log_message_queue *queue, int size) {
    vector_messages *msgv = s_malloc(sizeof(vector_messages), NULL);
    if (msgv == NULL) {
        return NULL;
    }
    err_t  error;
    VECTOR_INIT_WITH_RESERVE(log_message, msgv, size, error);
    if(error.error) {
        return NULL;
    }

    for(log_message *ptr = TAILQ_FIRST(queue); ptr != NULL; ptr = TAILQ_NEXT(ptr, entries)) {
        VECTOR_PUSH_BACK(log_message, msgv, *ptr, error);
        if (error.error) {
            fprintf(stderr, LOG_ERROR_MSG "error pushing message in vector messages for sorted print");
        }
    }
    return msgv;
}

void pr_log_free_msgq(log_message_queue *messages) {
    if (messages != NULL) {
        while (!TAILQ_EMPTY(messages)) {
            log_message *entry = TAILQ_FIRST(messages);
            TAILQ_REMOVE(messages, entry, entries);
            free(entry->message);
            free(entry);
        }
        free(messages);
    }
}

void pr_messages_print(log_context *context, log_message_queue *messages, size_t msg_size) {
    if (msg_size > 0) {
        vector_messages *msgv = pr_log_convert_msgq_to_msgv(messages, msg_size);
        qsort(VECTOR(msgv), VECTOR_SIZE(msgv), sizeof(log_message), pr_log_message_comparator);

        pthread_mutex_lock(&context->mutex_property);
        for (size_t i = 0; i < VECTOR_SIZE(msgv); ++i) {
            for (size_t j = 0; j < VECTOR_SIZE(context->printers); ++j) {
                if (VECTOR(context->printers)[j].level <= context->enabled_level) {
                    if (VECTOR(context->printers)[j].level >= VECTOR(msgv)[i].level) {
                        printer_t  *ptr = &VECTOR(context->printers)[j];
                        if  (ptr->type == LOG_PRINTER_CONSOLE) {
                            ((log_console_printer*)ptr)->handler(VECTOR(msgv)[i]);
                        } else if (ptr->type == LOG_PRINTER_FILE){
                            log_file_printer* fprinter = (log_file_printer *)ptr;
                            fprinter->handler(VECTOR(msgv)[i], fprinter->file);
                        }
                    }

                }
            }
        }
        pthread_mutex_unlock(&context->mutex_property);

        VECTOR_FREE(msgv);
        free(msgv);
    }

}


void* pr_log_thread(void *ptr) {
    log_context *context= (log_context*) ptr;
    if (context == NULL) {
        fprintf(stderr, LOG_ERROR_MSG "log_context is NULL");
        return NULL;
    }
    while (pr_log_is_run(context)) {
        log_message_queue *messages = NULL;
        size_t msg_size = 0;
        if (!pr_log_wait_and_extract_messages(context, &messages, &msg_size)) {
            fprintf(stderr, "LOG EXIT OK\n");
            pr_log_free_msgq(messages);
            return NULL;
        }
        if (messages == NULL) {
            fprintf(stderr, "LOG ERROR\n");
            return NULL;
        }
        pr_messages_print(context, messages, msg_size);
        pr_log_free_msgq(messages);
    }
    return NULL;
}


bool log_init(log_context *context) {
    if (context == NULL) {
        return false;
    }

    context->enabled_level = DEBUG_LEVEL;
    if (pthread_mutex_init(&(context->mutex_messages), NULL)) {
        goto error;
    }
    if (pthread_cond_init(&(context->cv), NULL)) {
        goto error;
    }
    if (pthread_mutex_init(&context->mutex_property, NULL)) {
        goto error;
    }
    context->messages = s_malloc(sizeof(log_message_queue), NULL);
    if (context->messages == NULL) {
        goto error;
    }
    TAILQ_INIT(context->messages);
    context->printers = s_malloc(sizeof(log_vector_printers), NULL);
    if (context->printers == NULL) {
        goto error;
    }
    err_t  err;
    VECTOR_INIT(printer_t, context->printers, err);
    if (err.error) {
        goto error;
    }
    context->timeout = 500;
    log_console_printer stderr_printer;
    stderr_printer.level = DEBUG_LEVEL;
    stderr_printer.handler = pr_printer_in_console;
    printer_t *ptr = (printer_t*)&stderr_printer;
    VECTOR_PUSH_BACK(printer_t, context->printers, *ptr, err);
    if (err.error) {
        goto error;
    }
    context->is_run = true;
    pthread_create(&context->thread, NULL, pr_log_thread, context);
    return true;

error:
//    pthread_mutex_destroy(&((context)->mutex_messages));
//    pthread_cond_destroy(&((context)->cv));
//    pthread_mutex_destroy(&((context)->mutex_property));
//    if (con != NULL) {
//        if (con->messages != NULL) {
//            free(con->messages);
//        }
//        if (con->printers != NULL) {
//            VECTOR_FREE(con->printers);
//            free(con->printers);
//        }
//        free(con);
//    }
//    *context = NULL;
    log_free(context);
    return false;
}


bool log_file_writer(log_context *context, log_level level, const char *path) {
    log_file_printer file_printer = {0};
    file_printer.level = level;
    file_printer.handler = pr_printer_in_file;
    file_printer.file = fopen(path, "w");
    file_printer.type = LOG_PRINTER_FILE;
    if (file_printer.file == NULL) {
        return false;
    }
    printer_t *ptr = (printer_t*)  &file_printer;
    pthread_mutex_lock(&context->mutex_property);
    err_t err;
    VECTOR_PUSH_BACK(printer_t, context->printers, *ptr ,err);
    pthread_mutex_unlock(&context->mutex_property);
    if (err.error) {
        return false;
    }
    return true;
}

bool log_console_set_level(log_context *context, log_level level) {
    pthread_mutex_lock(&context->mutex_property);
    VECTOR(context->printers)[0].level = level;
    pthread_mutex_unlock(&context->mutex_property);
    return true;
}


bool log_make_message(log_message *message, log_level level, const char *pattern, const char *ptr, ...) {
    va_list vl;

    va_start(vl, ptr);
    sprintf(message->message, pattern, vl);
    return true;
}

log_level log_get_level(log_context *context) {
    return context->enabled_level;
}

bool log_write(log_context *context, log_message *message) {
    if (context == NULL || message == NULL) {
        return false;
    }

    pthread_mutex_lock(&context->mutex_messages);
    TAILQ_INSERT_TAIL(context->messages, message, entries);
    context->num_messages++;
    pthread_mutex_unlock(&context->mutex_messages);
    return true;
}

void pr_log_messages_queue_free(log_context *context) {
    int size = 0;
    for (log_message *ptr = TAILQ_FIRST(context->messages); ptr != NULL; ptr = TAILQ_NEXT(ptr, entries)) {
        size++;
    }
    pr_messages_print(context, context->messages, size);
    pr_log_free_msgq(context->messages);
}


void log_free(log_context *context) {
    context->is_run = false;
    pthread_join(context->thread, NULL);
    pr_log_messages_queue_free(context);
    pthread_mutex_destroy(&context->mutex_messages);
    pthread_cond_destroy(&context->cv);
    pthread_mutex_destroy(&context->mutex_property);
    VECTOR_FREE(context->printers);
    free(context->printers);
}

log_level log_char_to_level(const char *level) {
    if (strcmp(level, "ERROR") == 0) {
        return ERROR_LEVEL;
    } else if (strcmp(level, "WARNING") == 0) {
        return WARNING_LEVEL;
    } else if (strcmp(level, "INFO") == 0) {
        return INFO_LEVEL;
    } else if (strcmp(level, "DEBUG") == 0) {
        return DEBUG_LEVEL;
    } else if (strcmp(level, "OFF") == 0) {
        return OFF;
    }
    return INVALID_LEVEL;
}