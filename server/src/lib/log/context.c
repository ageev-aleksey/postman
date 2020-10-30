//
// Created by nrx on 27.10.2020.
//

#include "log/context.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#define MS_TO_NS 1000000;

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
    fprintf(stderr, STYLE_RESET "%s %s%s" COLOR_GRAY "[%s:%s:%zu]" STYLE_RESET ": %s \n",
           buf, debug_level_color[msg.level], debug_level_str[msg.level],
           msg.file, msg.function, msg.line,
           msg.message);
}

int pr_log_message_comparator(const void *first, const void *second) {
    return ((log_message*)first)->time > ((log_message*)second)->time ? 1 : -1;
}

void* pr_log_thread(void *ptr) {
    log_context *log = (log_context*) ptr;
    log_vector_printers *printers = s_malloc(sizeof(log_vector_printers), NULL);
    if (printers == NULL) {
        fprintf(stderr, "FATAL ERROR IN LOGGER: error malloc for vector_printers");
        return NULL;
    }
    error_t err;
    VECTOR_INIT(printer_t, printers, err);
    if (err.error) {
        fprintf(stderr, "FATAL ERROR IN LOGGER: error init vector_printers");
        return NULL;
    }
    log_level  level;
    while (log->is_run) { // TODO (aageev) - необходимо добавить отдельный мьютекс для флага
        pthread_mutex_lock(&log->mutex);
        level = log->enabled_level;
        if (VECTOR_SIZE(log->printers) != 0) {
            for(size_t i = 0; i < VECTOR_SIZE(log->printers); ++i) {
                VECTOR_PUSH_BACK(printer_t, printers, VECTOR(log->printers)[i], err);
                if (err.error) {
                    fprintf(stderr, "FATAL ERROR IN LOGGER: error push back to log_vector_printers");
                    pthread_mutex_unlock(&log->mutex);
                    return NULL;
                }
            }
            VECTOR_FREE(log->printers);
            free(log->printers);
            log->printers = s_malloc(sizeof(log_vector_printers), NULL);
            if (log->printers == NULL) {
                fprintf(stderr, "FATAL ERROR IN LOGGER: error allocate memory for new push printers");
                pthread_mutex_unlock(&log->mutex);
                return NULL;
            }
            VECTOR_INIT(printer_t, log->printers, err);
            if (err.error) {
                fprintf(stderr, "FATAL ERROR IN LOGGER: error init vector printers");
                pthread_mutex_unlock(&log->mutex);
                return NULL;
            }
        }

        struct timespec time;
        time.tv_sec = 0;
        time.tv_nsec = log->timeout * MS_TO_NS;
        while (TAILQ_EMPTY(log->messages)) {
            pthread_cond_timedwait(&log->cv, &log->mutex, &time);
        }
        log_message_queue *msg = log->messages;
        log->messages = s_malloc(sizeof(log_message_queue), NULL);
        if (log->messages == NULL) {
            fprintf(stderr, "FATAL ERROR IN LOGGER: error malloc for new queue of messages");
            log->is_run = false;
            pthread_mutex_unlock(&log->mutex);
            return NULL;
        }
        TAILQ_INIT(log->messages);
        pthread_mutex_unlock(&log->mutex);

        vector_messages *v_msg = s_malloc(sizeof(vector_messages), NULL);
        if (v_msg == NULL) {
            fprintf(stderr, "FATAL ERROR IN LOGGER: error malloc for new vector of messages");
            log->is_run = false;
            return NULL;
        }

        VECTOR_INIT(vector_messages, v_msg, err);
        if (err.error) {
            fprintf(stderr, "FATAL ERROR IN LOGGER: error init vector of messages");
            log->is_run = false;
            return NULL;
        }

        while(!TAILQ_EMPTY(msg)) {
            log_message *entry = TAILQ_FIRST(msg);
            TAILQ_REMOVE(msg, entry, entries);
            VECTOR_PUSH_BACK(log_message, v_msg, *entry, err);
            if (err.error) {
                fprintf(stderr, "FATAL ERROR IN LOGGER: error copy list to vector");
                log->is_run = false;
                return NULL;
            }
            free(entry);
        }
        free(msg);

        qsort(VECTOR(v_msg), VECTOR_SIZE(v_msg), sizeof(log_message), pr_log_message_comparator);

        for (size_t i = 0; i < VECTOR_SIZE(v_msg); ++i) {
            for (size_t j = 0; j < VECTOR_SIZE(printers); ++j) {
                if (VECTOR(printers)[j].level <= level) {
                    VECTOR(printers)[j].handler(VECTOR(v_msg)[i]);
                }
            }
        }

    }

    return NULL;
}


bool log_init(log_context **context) {
    if (context == NULL) {
        return false;
    }
    log_context *con = s_malloc(sizeof(log_context), NULL);
    if (con == NULL) {
        return false;
    }
    con->enabled_level = DEBUG_LEVEL;
    if (pthread_mutex_init(&(con->mutex), NULL)) {
        goto error;
    }
    if (pthread_cond_init(&(con->cv), NULL)) {
        goto error;
    }
    con->messages = s_malloc(sizeof(log_message_queue), NULL);
    if (con->messages == NULL) {
        goto error;
    }
    TAILQ_INIT(con->messages);
    con->printers = s_malloc(sizeof(log_vector_printers), NULL);
    if (con->printers == NULL) {
        goto error;
    }
    error_t  err;
    VECTOR_INIT(printer_t, con->printers, err);
    if (err.error) {
        goto error;
    }
    con->timeout = 500;
    printer_t stderr_printer;
    stderr_printer.level = DEBUG_LEVEL;
    stderr_printer.handler = pr_printer_in_console;
    VECTOR_PUSH_BACK(printer_t, con->printers, stderr_printer, err);
    if (err.error) {
        goto error;
    }
    *context = con;
    con->is_run = true;
    pthread_create(&con->thread, NULL, pr_log_thread, con);
    return true;

error:
    pthread_mutex_destroy(&((*context)->mutex));
    pthread_cond_destroy(&((*context)->cv));
    if (con != NULL) {
        if (con->messages != NULL) {
            free(con->messages);
        }
        if (con->printers != NULL) {
            free(con->printers);
        }
        free(con);
    }
    *context = NULL;
    return false;
}


bool log_make_message(log_message *message, log_level level, const char *pattern, const char *ptr, ...) {
    va_list vl;

    va_start(vl, ptr);
    int buffer_size = sprintf(message->message, pattern, vl);
    return true;
}

log_level log_get_level(log_context *context) {
    return context->enabled_level;
}

bool log_write(log_context *context, log_message *message) {
    if (context == NULL || message == NULL) {
        return false;
    }

    pthread_mutex_lock(&context->mutex);
    TAILQ_INSERT_TAIL(context->messages, message, entries);
    pthread_mutex_unlock(&context->mutex);
}