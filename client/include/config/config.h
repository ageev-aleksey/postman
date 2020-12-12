#include <stdio.h>
#include <stdbool.h>

#define APPLICATION_CONFIG "resources/application.cfg"

typedef struct s_maildir_config {
    char *path;
} maildir_config;

typedef struct s_config {
    maildir_config maildir;
} config;

config config_context;
bool loading_config();