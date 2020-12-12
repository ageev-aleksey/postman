#include <stdlib.h>
#include <libconfig.h>
#include <string.h>
#include "config.h"

bool loading_config() {
    config_t cfg;
    config_init(&cfg);

    if (!(config_read_file(&cfg, APPLICATION_CONFIG))) {
        config_destroy(&cfg);
        return false;
    }

    const char *maildir_path = NULL;
    if (!config_lookup_string(&cfg, "application.maildir.path", &maildir_path)) {
        return false;
    }

    config_context.maildir.path = malloc(sizeof(maildir_path));
    strcpy(config_context.maildir.path, maildir_path);

    config_destroy(&cfg);
    return true;
}