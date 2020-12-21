#include "server/config.h"
#include "log/context.h"

#include <signal.h>
#include <string.h>
#include <unistd.h>


void posix_signal_handler(int signal) {
    LOG_INFO("Signal handler triggered [%d]", signal);
    if (signal == SIGTERM || signal == SIGINT) {
        LOG_INFO("%s", "Shutdown signal received");
        err_t  error;
        el_stop(server_config.loop, &error);
        if (error.error) {
            LOG_ERROR("el_stop: %s", error.message);
        }
    }
}

struct log_file_settings {
    char *path;
    log_level level;
};

bool server_config_init(const char *path) {
    err_t error;
    server_config.loop = el_init(&error);
    if (error.error) {
        LOG_ERROR("el_init: %s", error.message);
        return false;
    }
    config_t cfg;
    config_init(&cfg);

    if(!config_read_file(&cfg, path))
    {
        LOG_ERROR("%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return false;
    }

    /// Настройка порта
    int port = 0;
    if (!config_lookup_int(&cfg, "server.port", &port)) {
        LOG_ERROR("%s [%s]", "not found server.port field in file", path);
        return false;
    }
    if (port > 65535) {
        LOG_ERROR("%s [%d]", "invalid port value", port);
        return false;
    }

    /// Настройка ip адреса
    const char *host = NULL;
    if (!config_lookup_string(&cfg, "server.host", &host)) {
        LOG_ERROR("%s [%s]", "not found server.host field in file", path);
        return false;
    }

    /// число рабочих потоков
    int num_threads= 0;
    if (!config_lookup_int(&cfg, "server.worker_threads", &num_threads)) {
        LOG_ERROR("%s [%s]", "not found server.worker_threads field in file", path);
        return false;
    }
    if (num_threads <= 0) {
        LOG_ERROR("invalid number threads value [%d]", num_threads);
        return false;
    }
    server_config.num_worker_threads  = num_threads;

    /// Настройка доменного имени сервера
    const char *domain = NULL;
    if (!config_lookup_string(&cfg, "server.domain", &domain)) {
        LOG_ERROR("%s [%s]", "not found server.domain field in file", path);
        return false;
    }

    /// настрока таймера на ожидание команд от клиента
    if(!config_lookup_int(&cfg, "server.timer", &server_config.timer_period)) {
        LOG_ERROR("%s [%s]", "not found server.timer field in file", path);
        return false;
    }
    if (server_config.timer_period <= 0) {
        LOG_ERROR("value of server.timer [%d] in config file [%s] must be greater than zero",
                  server_config.timer_period, path);
        return false;
    }


    /// Путь до корня maildir
    const char *maildir_path = NULL;
    if (!config_lookup_string(&cfg, "server.maildir_path", &maildir_path)) {
        LOG_ERROR("%s [%s]", "not found server.maildir_path field in file", path);
        return false;
    }

    /// вежливость серверного процесса
    if(!config_lookup_int(&cfg, "server.nice", &server_config.nice_value)) {
        LOG_ERROR("%s [%s]", "not found server.nice field in file", path);
        return false;
    }
    if (server_config.nice_value >= -20 && server_config.nice_value <= 20 ) {
        nice(server_config.nice_value);
    } else {
        LOG_ERROR("invalid server.nice value [%d]. The value must be in the range (-20; 20)",
                    server_config.nice_value);
        return false;
    };

    // Настройка уровня логирования
    config_setting_t *log_settings = config_lookup(&cfg, "server.log");
    log_level  console_level = DEBUG;
    if (log_settings != NULL) {

        const char *console_level_char = NULL;
        if (!config_setting_lookup_string(log_settings, "console_level", &console_level_char)) {
            LOG_ERROR("%s", "error server.log.console_level");
            return false;
        }

        console_level = log_char_to_level(console_level_char);
        if (console_level == INVALID_LEVEL) {
            LOG_ERROR("%s", "error server.log.console_level - invalid value");
            return false;
        }

        config_setting_t *log_files_settings = config_setting_lookup(log_settings, "files");
        struct log_file_settings *file_settings = NULL;
        if (log_files_settings != NULL) {
            int len = config_setting_length(log_files_settings);
            file_settings = malloc(sizeof(struct log_file_settings)*len);
            for (int i = 0; i < len; i++) {
                config_setting_t *el = config_setting_get_elem(log_files_settings, i);

                const char *file_path = NULL;
                if (!config_setting_lookup_string(el, "path", &file_path)) {
                    LOG_ERROR("%s", "error server.log.console_level");
                    return false;
                }

                const char *file_level = NULL;
                if (!config_setting_lookup_string(el, "level", &file_level)) {
                    LOG_ERROR("%s", "error server.log.console_level");
                    return false;
                }
                log_level  fl = log_char_to_level(file_level);
                if (fl == INVALID_LEVEL) {
                    LOG_ERROR("%s", "error server.log.files.level - invalid value");
                    return false;
                }
                file_settings[i].path = malloc(strlen(file_path));
                strcpy(file_settings[i].path, file_path);
                file_settings[i].level = fl;
            }

            for (int i = 0; i < len; i++) {
                if (!log_file_writer(&GLOBAL_LOG_CONTEXT, file_settings[i].level, file_settings[i].path)) {
                    printf("LOG SETTINGS FATAL ERROR!\n");
                    return false;
                }
                free(file_settings[i].path);
            }
            free(file_settings);
        }


    } else {
        LOG_INFO("%s", "server.log not found. Using default settings for logging");
    }
    // log_file_writer(&GLOBAL_LOG_CONTEXT, DEBUG_LEVEL, "./debug_server.log");
     log_console_set_level(&GLOBAL_LOG_CONTEXT, console_level);

    server_config.ip = malloc(sizeof(char) * strlen(host)+1);
    server_config.self_server_name = malloc(sizeof(char) * strlen(domain)+1);
    strcpy(server_config.ip, host);
    strcpy(server_config.self_server_name, domain);
    server_config.log_file_path = NULL;
    server_config.port = port;
    server_config.hello_msg_size =
            asprintf(&server_config.hello_msg, "220 %s The Postman Server v%d.%d\r\n",
                     domain, POSTMAN_VERSION_MAJOR, POSTMAN_VERSION_MINOR);
    users_list__init(&server_config.users);

    if (!maildir_init(&server_config.md, maildir_path, &error)) {
        LOG_ERROR("maildir: %s", error.message);
        return false;
    }



    LOG_INFO("\nConfig loaded: \n -- host: %s\n -- port: %d\n -- domain: %s\n -- maildir: %s"
             "\n -- num_workers: %zu",
             server_config.ip,
             server_config.port,
             server_config.self_server_name,
             maildir_path,
             server_config.num_worker_threads);
    config_destroy(&cfg);

    timers_init(&server_config.timers);

    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = posix_signal_handler;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    sig_act.sa_mask = set;
    sigaction(SIGTERM, &sig_act, NULL);
    sigaction(SIGINT, &sig_act, NULL);


    return true;
}



void server_config_free() {
    el_close(server_config.loop);
    free(server_config.self_server_name);
    free(server_config.ip);
    free(server_config.log_file_path);
    free(server_config.hello_msg);
    maildir_free(&server_config.md);
    users_list__free(&server_config.users);
    timers_free(&server_config.timers);
}

