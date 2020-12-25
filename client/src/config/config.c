#include "util/util.h"
#include "log/logs.h"
#include "config.h"

bool loading_config() {
    config_t cfg;
    config_init(&cfg);

    config_context.maildir.path = NULL;
    config_context.hostname = NULL;
    config_context.server_port = NULL;

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

    const char *hostname = NULL;
    if (!config_lookup_string(&cfg, "application.hostname", &hostname)) {
        LOG_ERROR("Ошибка получения конфигурации: application.hostname", NULL);
        return false;
    }

    // Порт задается для локального сервера
    const char *server_port = NULL;
    if (!config_lookup_string(&cfg, "application.server_port", &server_port)) {
        LOG_ERROR("Ошибка получения конфигурации: application.server_port", NULL);
        return false;
    }

    asprintf(&config_context.maildir.path, "%s", maildir_path);
    config_context.threads = threads;
    config_context.debug = debug;
    asprintf(&config_context.hostname, "%s", hostname);
    asprintf(&config_context.server_port, "%s", server_port);

    LOG_INFO("Конфигурация - application.maildir.path = '%s'", config_context.maildir.path);
    LOG_INFO("Конфигурация - application.threads = %d", config_context.threads);
    LOG_INFO("Конфигурация - application.debug = %d", config_context.debug);
    LOG_INFO("Конфигурация - application.hostname = '%s'", config_context.hostname);
    LOG_INFO("Конфигурация - application.server_port = '%s'", config_context.server_port);

    config_destroy(&cfg);
    return true;
}

bool destroy_configuration() {
    free(config_context.hostname);
    free(config_context.server_port);
    free(config_context.maildir.path);
}