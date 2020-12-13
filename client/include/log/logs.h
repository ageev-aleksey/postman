#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/queue.h>
#include <time.h>

#define COLOR_RESET "\x1b[0m"
#define COLOR_BRIGHT "\x1b[1m"
#define COLOR_DIM "\x1b[2m"
#define COLOR_UNDERSCORE "\x1b[4m"
#define COLOR_BLINK "\x1b[5m"
#define COLOR_REVERSE "\x1b[7m"
#define COLOR_HIDDEN "\x1b[8m"

#define COLOR_BLACK "\x1b[30m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_WHITE "\x1b[37m"

#define BACKGROUND_BLACK "\x1b[40m"
#define BACKGROUND_RED "\x1b[41m"
#define BACKGROUND_GREEN "\x1b[42m"
#define BACKGROUND_YELLOW "\x1b[43m"
#define BACKGROUND_BLUE "\x1b[44m"
#define BACKGROUND_MAGENTA "\x1b[45m"
#define BACKGROUND_CYAN "\x1b[46m"
#define BACKGROUND_WHITE "\x1b[47m"

#define LOG_DEBUG(message) log_debug(message, __FILE__, __LINE__)
#define LOG_INFO(message) log_info(message, __FILE__, __LINE__)
#define LOG_ERROR(message) log_error(message, __FILE__, __LINE__)
#define LOG_WARN(message) log_warn(message, __FILE__, __LINE__)

typedef enum log_type {
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG,
    LOG_WARN
} log_type;

typedef struct log {
    char *message;
    char *filename;
    char *thread;
    int line;
    log_type type;
    struct tm time;
} log;

typedef struct node {
    log data;
    TAILQ_ENTRY(node) nodes;
} node;

void start_logger();
void log_debug(char *message, char *filename, int line);
void log_info(char *message, char *filename, int line);
void log_error(char *message, char *filename, int line);
void log_warn(char *message, char *filename, int line);

