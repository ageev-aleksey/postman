#include <libconfig.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#define APPLICATION_CONFIG "resources/application.cfg"

typedef struct maildir_config {
    char *path;
} maildir_config;

typedef struct config {
    int threads;
    maildir_config maildir;
    int debug;
    char *hostname;
} config;

config config_context;
bool loading_config();
int init_signals_handler();