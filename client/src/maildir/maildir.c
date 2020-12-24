#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include "log/logs.h"
#include "util/util.h"
#include "maildir.h"

#define DIRECTORY_NEW_MESSAGES "new"
#define DIRECTORY_CUR_MESSAGES "cur"
#define DIRECTORY_OTHER_SERVERS ".OTHER_SERVERS"

void read_maildir_servers(maildir_main *maildir);

void read_maildir_servers_new(maildir_other_server *maildir);

void read_maildir_user_new(maildir_user *maildir_user);

void read_new_messages_list(maildir_user *maildir_user);

pair *check_message_header(char *line);

maildir_main *init_maildir(char *directory) {
    maildir_main *maildir = allocate_memory(sizeof(*maildir));
    memset(maildir, 0, sizeof(*maildir));

    struct stat stat_info;
    if (!stat(directory, &stat_info)) {
        if (S_ISDIR(stat_info.st_mode)) {
            asprintf(&maildir->directory, "%s", directory);
        }
    }

    return maildir;
}

void update_maildir(maildir_main *maildir) {
    if (maildir->directory == NULL) {
        LOG_ERROR("Ошибка чтения структуры maildir: директория не найдена", NULL);
        return;
    }

    struct stat stat_info;
    if (!stat(maildir->directory, &stat_info)) {
        if (!S_ISDIR(stat_info.st_mode)) {
            LOG_ERROR("Ошибка чтения структуры maildir: %s - не директории", maildir->directory);
            return;
        }
    }

    maildir->users = NULL;
    maildir->users_size = 0;

    int users_count = 0;
    DIR *dir = opendir(maildir->directory);
    struct dirent *entry = readdir(dir);
    maildir_user *users = allocate_memory(sizeof(*users));

    while (entry != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if (strcmp(entry->d_name, DIRECTORY_OTHER_SERVERS) == 0) {
                read_maildir_servers(maildir);
            } else {
                users_count++;
                users = reallocate_memory(users, sizeof(*users) * users_count);
                asprintf(&users[users_count - 1].username, "%s", entry->d_name);
                asprintf(&users[users_count - 1].directory, "%s/%s", maildir->directory,
                         users[users_count - 1].username);
                read_maildir_user_new(&users[users_count - 1]);
            }
        }
        entry = readdir(dir);
    }
    closedir(dir);

    maildir->users = users;
    maildir->users_size = users_count;
}

void read_maildir_user_new(maildir_user *user) {
    if (user == NULL) {
        LOG_ERROR("Ошибка чтения сообщений пользователя: user = NULL", NULL);
        return;
    }
    if (user->directory == NULL) {
        LOG_ERROR("Ошибка чтения сообщений пользователя: user.directory == NULL", NULL);
        return;
    }

    struct stat stat_info;
    if (!stat(user->directory, &stat_info)) {
        if (!S_ISDIR(stat_info.st_mode)) {
            LOG_ERROR("Ошибка чтения структуры maildir: %s - не директория", user->directory);
            return;
        }
    }

    DIR *dir = opendir(user->directory);
    struct dirent *entry = readdir(dir);

    while (entry != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if (strcmp(entry->d_name, DIRECTORY_NEW_MESSAGES) == 0) {
                read_new_messages_list(user);
            }
        }
        entry = readdir(dir);
    }
    closedir(dir);
}

void read_new_messages_list(maildir_user *user) {

    char *user_fulldir_new;
    asprintf(&user_fulldir_new, "%s/%s", user->directory, DIRECTORY_NEW_MESSAGES);

    struct stat stat_info;
    if (!stat(user_fulldir_new, &stat_info)) {
        if (!S_ISDIR(stat_info.st_mode)) {
            LOG_ERROR("Ошибка чтения структуры maildir: %s - не директория", user_fulldir_new);
            return;
        }
    }

    user->message_full_file_names = NULL;
    user->messages_size = 0;

    DIR *dir = opendir(user_fulldir_new);
    int messages_count = 0;
    struct dirent *entry = readdir(dir);
    user->message_full_file_names = allocate_memory(sizeof(user->message_full_file_names));
    while (entry != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            messages_count++;
            user->message_full_file_names = reallocate_memory(user->message_full_file_names,
                                                              sizeof(user->message_full_file_names) * messages_count);
            asprintf(&user->message_full_file_names[messages_count - 1], "%s/%s", user_fulldir_new, entry->d_name);
        }
        entry = readdir(dir);
    }
    closedir(dir);

    user->messages_size = messages_count;

    free(user_fulldir_new);
}

void read_maildir_servers(maildir_main *maildir) {
    if (maildir->directory == NULL) {
        LOG_ERROR("Ошибка чтения структуры maildir: директория не найдена", NULL);
        return;
    }

    struct stat stat_info;
    if (!stat(maildir->directory, &stat_info)) {
        if (!S_ISDIR(stat_info.st_mode)) {
            LOG_ERROR("Ошибка чтения структуры maildir: %s - не директории", maildir->directory);
            return;
        }
    }

    char *path_other_servers;
    asprintf(&path_other_servers, "%s/%s", maildir->directory, DIRECTORY_OTHER_SERVERS);

    maildir->servers = NULL;
    maildir->servers_size = 0;

    int servers_count = 0;
    DIR *dir = opendir(path_other_servers);
    struct dirent *entry = readdir(dir);
    maildir_other_server *servers = allocate_memory(sizeof(*servers));

    while (entry != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            servers_count++;
            servers = reallocate_memory(servers, sizeof(*servers) * servers_count);

            maildir_other_server server = { 0 };

            asprintf(&server.server, "%s", entry->d_name);
            asprintf(&server.directory, "%s/%s", path_other_servers, server.server);
            read_maildir_servers_new(&server);

            servers[servers_count - 1] = server;
        }
        entry = readdir(dir);
    }
    closedir(dir);

    maildir->servers = servers;
    maildir->servers_size = servers_count;

    free(path_other_servers);
}

void read_maildir_servers_new(maildir_other_server *server) {
    if (server == NULL) {
        LOG_ERROR("Ошибка чтения сообщений пользователей других серверов: main = NULL", NULL);
        return;
    }
    if (server->directory == NULL) {
        LOG_ERROR("Ошибка чтения сообщений пользователей других серверов: main.directory == NULL", NULL);
        return;
    }

    struct stat stat_info;
    if (!stat(server->directory, &stat_info)) {
        if (!S_ISDIR(stat_info.st_mode)) {
            LOG_ERROR("Ошибка чтения структуры maildir: %s - не директория", server->directory);
            return;
        }
    }

    server->message_full_file_names = NULL;
    server->messages_size = 0;

    DIR *dir = opendir(server->directory);
    struct dirent *entry = readdir(dir);
    int messages_count = 0;
    server->message_full_file_names = allocate_memory(sizeof(server->message_full_file_names));
    while (entry != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, "tmp") != 0) {
            messages_count++;
            server->message_full_file_names = reallocate_memory(server->message_full_file_names,
                                                                         sizeof(server->message_full_file_names)
                                                                         * messages_count);
            asprintf(&server->message_full_file_names[messages_count - 1], "%s/%s", server->directory, entry->d_name);
        }
        entry = readdir(dir);
    }
    closedir(dir);

    server->messages_size = messages_count;
}

message *read_message(char *filepath) {
    FILE *fp;
    if ((fp = fopen(filepath, "r")) != NULL) {
        message *mes = allocate_memory(sizeof(*mes));
        memset(mes, 0, sizeof(*mes));

        char *string;
        while ((string = file_readline(fp)) != NULL) {
            trim(string);

            if (strcmp(string, "\n") == 0 || strcmp(string, "\r\n") == 0) {
                free(string);
                break;
            }

            pair *p = check_message_header(string);
            if (p != NULL) {
                if (strcmp(p->first, "X-POSTMAN-FROM") == 0) {
                    string_tokens tokens = split(p->second, ",");
                    mes->from_size = tokens.count_tokens;
                    mes->from = callocate_memory(mes->from_size, sizeof(*mes->from));
                    for (size_t i = 0; i < mes->from_size; i++) {
                        asprintf(&mes->from[i], "%s", tokens.tokens[i].chars);
                    }
                    free_string_tokens(&tokens);
                } else if (strcmp(p->first, "X-POSTMAN-TO") == 0) {
                    string_tokens tokens = split(p->second, ",");
                    mes->to_size = tokens.count_tokens;
                    mes->to = callocate_memory(mes->to_size, sizeof(*mes->to));
                    mes->addresses = callocate_memory(1, sizeof(*mes->addresses));
                    for (size_t i = 0; i < mes->to_size; i++) {
                        asprintf(&mes->to[i], "%s", tokens.tokens[i].chars);
                        string_tokens ts = split(p->second, "@");
                        if (ts.count_tokens == 2) {
                            bool flag = true;
                            for (int k = 0; k < mes->addresses_size; k++) {
                                if (strcmp(ts.tokens[1].chars, mes->addresses[k]) == 0) {
                                    flag = false;
                                }
                            }
                            if (flag) {
                                mes->addresses = reallocate_memory(mes->addresses,sizeof(*mes->addresses) * (mes->addresses_size + 1));
                                asprintf(&mes->addresses[mes->addresses_size++], "%s", ts.tokens[1].chars);
                            }
                        }
                        free_string_tokens(&ts);
                    }
                    free_string_tokens(&tokens);
                } else if (strcmp(p->first, "X-POSTMAN-DATE") == 0) {
                    // mes->date = p->second;
                    // TODO: пока игнорирую дату, а нужна ли она вообще?
                }
            }
            if (p != NULL) {
                free(p->first);
                free(p->second);
                free(p);
            }
            free(string);
        }

        int strings_count = 0;
        mes->strings = allocate_memory(sizeof(*mes->strings));
        while ((string = file_readline(fp)) != NULL) {
            if (mes->strings != NULL) {
                mes->strings = reallocate_memory(mes->strings, sizeof(mes->strings) * (strings_count + 1));
            }
            asprintf(&mes->strings[strings_count], "%s", string);
            strings_count++;
            free(string);
        }

        if (feof(fp)) {
            mes->strings_size = strings_count;
            asprintf(&mes->directory, "%s", filepath);
        } else {
            LOG_ERROR("Ошибка чтения файла", NULL);
        }

        fclose(fp);
        return mes;
    }

    return NULL;
}

pair *check_message_header(char *line) {
    if (line == NULL) {
        LOG_ERROR("Ошибка считывания заголовка в сообщении", NULL);
        return NULL;
    }

    pair *p = allocate_memory(sizeof(*p));

    string_tokens tokens = split(line, ":");

    if (tokens.tokens != NULL) {
        char *str = tokens.tokens[0].chars;
        for (int i = 0; i < strlen(str); i++) {
            str[i] = toupper(str[i]);
        }

        if (strstr(str, "X-POSTMAN-FROM") != NULL) {
            asprintf(&p->first, "%s", "X-POSTMAN-FROM");
            if (tokens.count_tokens == 2) {
                str = tokens.tokens[1].chars;
                trim(str);
                if (str[strlen(str) - 1] == '\n' && str[strlen(str) - 2] == '\r') {
                    str[strlen(str) - 1] = 0;
                    str[strlen(str) - 1] = 0;
                }
                if (str[strlen(str) - 1] == '\n') {
                    str[strlen(str) - 1] = 0;
                }
                asprintf(&p->second, "%s", tokens.tokens[1].chars);
            }
            free_string_tokens(&tokens);
            return p;
        } else if (strstr(str, "X-POSTMAN-TO") != NULL) {
            asprintf(&p->first, "%s", "X-POSTMAN-TO");
            if (tokens.count_tokens == 2) {
                str = tokens.tokens[1].chars;
                trim(str);
                if (str[strlen(str) - 1] == '\n' && str[strlen(str) - 2] == '\r') {
                    str[strlen(str) - 1] = 0;
                    str[strlen(str) - 1] = 0;
                }
                if (str[strlen(str) - 1] == '\n') {
                    str[strlen(str) - 1] = 0;
                }
                asprintf(&p->second, "%s", tokens.tokens[1].chars);
            }
            free_string_tokens(&tokens);
            return p;
        } else if (strstr(str, "X-POSTMAN-DATE") != NULL) {
            asprintf(&p->first, "X-POSTMAN-DATE");
            if (tokens.count_tokens == 2) {
                str = tokens.tokens[1].chars;
                trim(str);
                if (str[strlen(str) - 1] == '\n' && str[strlen(str) - 2] == '\r') {
                    str[strlen(str) - 1] = 0;
                    str[strlen(str) - 1] = 0;
                }
                if (str[strlen(str) - 1] == '\n') {
                    str[strlen(str) - 1] = 0;
                }
                asprintf(&p->second, "%s", tokens.tokens[1].chars);
            }
            free_string_tokens(&tokens);
            return p;
        }
    }

    return NULL;
}

void output_maildir(maildir_main *maildir) {
    if (maildir->directory == NULL) {
        LOG_ERROR("Ошибка чтения структуры maildir: директория не найдена", NULL);
    }

    LOG_INFO("Вывод структуры каталогов maildir: ", NULL);
    maildir_other_server *servers = maildir->servers;
    LOG_ADDINFO("\t.other_servers", NULL);
    if (servers != NULL) {
        for (int i = 0; i < maildir->servers_size; i++) {
            LOG_ADDINFO("\t%s", servers[i].server);
            if (servers[i].message_full_file_names != NULL) {
                for (int j = 0; j < servers[i].messages_size; j++) {
                    LOG_ADDINFO("\t\t%s", servers[i].message_full_file_names[j]);
                }
            }
        }
    }

    maildir_user *users = maildir->users;
    if (users != NULL) {
        for (int i = 0; i < maildir->users_size; i++) {
            LOG_ADDINFO("\t%s", users[i].username);
            if (users[i].message_full_file_names != NULL) {
                for (int j = 0; j < users[i].messages_size; j++) {
                    LOG_ADDINFO("\t\t%s", users[i].message_full_file_names[j]);
                }
            }
        }
    }
}

void remove_message_server(maildir_other_server *server, message *mes) {
    if (mes == NULL || mes->directory == NULL) {
        LOG_ERROR("Невозможно удалить сообщение: mes = NULL || mes->directory == NULL", NULL);
        return;
    }

    struct stat stat_info;
    if (!stat(mes->directory, &stat_info)) {
        if (!S_ISREG(stat_info.st_mode)) {
            LOG_ERROR("Ошибка удаления письма: %s - не файл", mes->directory);
            return;
        }
    }

    if (strstr(mes->directory, DIRECTORY_OTHER_SERVERS) != NULL) {
        if (remove(mes->directory) != 0) {
            LOG_ERROR("Не удалось удалить письмо", NULL);
            return;
        }

        if (server->message_full_file_names != NULL) {
            for (int i = 0; i < server->messages_size; i++) {
                if (strcmp(server->message_full_file_names[i], mes->directory) == 0) {
                    for (int j = i; j < server->messages_size - 1; j++) {
                        server->message_full_file_names[j] = server->message_full_file_names[j + 1];
                    }
                    server->messages_size--;
                }
            }
        }
    }

    free(mes->directory);
    for (int i = 0; i < mes->to_size; i++) {
        free(mes->to[i]);
    }
    free(mes->to);
    for (int i = 0; i < mes->from_size; i++) {
        free(mes->from[i]);
    }
    free(mes->from);
    for (int i = 0; i < mes->addresses_size; i++) {
        free(mes->addresses[i]);
    }
    free(mes->addresses);
    for (int i = 0; i < mes->strings_size; i++) {
        free(mes->strings[i]);
    }
    free(mes->strings);
    free(mes);
}

void finalize_maildir(maildir_main *maildir) {
    for (int i = 0; i < maildir->servers_size; i++) {
        free(maildir->servers[i].server);
        free(maildir->servers[i].directory);
        for (int k = 0; k < maildir->servers[i].messages_size; k++) {
            free(maildir->servers[i].message_full_file_names[k]);
        }
        free(maildir->servers[i].message_full_file_names);
    }
    free(maildir->servers);
    for (int i = 0; i < maildir->users_size; i++) {
        free(maildir->users[i].username);
        free(maildir->users[i].directory);
        for (int k = 0; k < maildir->users[i].messages_size; k++) {
            free(maildir->users[i].message_full_file_names[k]);
        }
        free(maildir->users[i].message_full_file_names);
    }
    free(maildir->users);
    free(maildir->directory);
    free(maildir);
}