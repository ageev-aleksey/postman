#include "config.h"

bool loading_config() {
    config_t cfg;
    config_init(&cfg);

    LOG_INFO("Загрузка конфигурации SMTP-клиента");
    if (!(config_read_file(&cfg, APPLICATION_CONFIG))) {
        config_destroy(&cfg);
        return false;
    }

    int threads;
    if (!config_lookup_int(&cfg, "application.threads", &threads)) {
        LOG_ERROR("Ошибка получения конфигурации: application.threads");
        return false;
    }

    if (threads < 1) {
        LOG_ERROR("Должен быть выделен хотя бы один дополнительный поток для журналирования.");
        return false;
    }

    const char *maildir_path = NULL;
    if (!config_lookup_string(&cfg, "application.maildir.path", &maildir_path)) {
        LOG_ERROR("Ошибка получения конфигурации: application.maildir.path");
        return false;
    }

    config_context.maildir.path = malloc(sizeof(maildir_path));
    strcpy(config_context.maildir.path, maildir_path);
    config_context.threads = threads;

    char *message_application_maildir_path = malloc(100);
    sprintf(message_application_maildir_path, "Конфигурация - application.maildir.path = '%s'", maildir_path);
    LOG_INFO(message_application_maildir_path);

    char *message_application_threads = malloc(100);
    sprintf(message_application_threads, "Конфигурация - application.threads = %d", threads);
    LOG_INFO(message_application_threads);

    config_destroy(&cfg);
    return true;
}