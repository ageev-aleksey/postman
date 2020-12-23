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
    char *server_port;
} config;

config config_context;
bool loading_config();
bool destroy_configuration();