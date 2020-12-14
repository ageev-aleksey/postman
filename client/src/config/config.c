#include "util.h"
#include "config.h"
#include "logs.h"

bool loading_config() {
    config_t cfg;
    config_init(&cfg);

    LOG_INFO("Загрузка конфигурации SMTP-клиента", NULL);
    if (!(config_read_file(&cfg, APPLICATION_CONFIG))) {
        config_destroy(&cfg);
        return false;
    }

    int threads;
    if (!config_lookup_int(&cfg, "application.threads", &threads)) {
        LOG_ERROR("Ошибка получения конфигурации: application.threads", NULL);
        return false;
    }

    if (threads < 1) {
        LOG_ERROR("Должен быть выделен хотя бы один дополнительный поток для журналирования.", NULL);
        return false;
    }

    const char *maildir_path = NULL;
    if (!config_lookup_string(&cfg, "application.maildir.path", &maildir_path)) {
        LOG_ERROR("Ошибка получения конфигурации: application.maildir.path", NULL);
        return false;
    }

    int debug;
    if (!config_lookup_int(&cfg, "application.debug", &debug)) {
        LOG_ERROR("Ошибка получения конфигурации: application.debug", NULL);
        return false;
    }

    if (debug != 0) {
        LOG_INFO("Режим DEBUG активен", NULL);
    }

    config_context.maildir.path = allocate_memory(strlen(maildir_path) + 1);
    strcpy(config_context.maildir.path, maildir_path);
    config_context.threads = threads;
    config_context.debug = debug;

    LOG_DEBUG("Конфигурация - application.maildir.path = '%s'", maildir_path);
    LOG_INFO("Конфигурация - application.threads = %d", threads);

    config_destroy(&cfg);
    return true;
}