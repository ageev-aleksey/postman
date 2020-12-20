#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include "maildir.h"
#include "logs.h"
#include "util.h"

#define DIRECTORY_NEW_MESSAGES "new"
#define DIRECTORY_CUR_MESSAGES "cur"
#define DIRECTORY_OTHER_SERVERS ".OTHER_SERVERS"

void update_maildir(maildir_main *maildir);

void output_maildir(maildir_main *maildir);

void read_maildir_servers_new(maildir_main *maildir);

void read_maildir_user_new(maildir_user *maildir_user);

void read_new_messages_list(maildir_user *maildir_user);

message *read_message(char *filepath);

maildir_main *init_maildir(char *directory) {
    LOG_INFO("Инициализация maildir", NULL);

    maildir_main *maildir = allocate_memory(sizeof(*maildir));
    memset(maildir, 0, sizeof(*maildir));

    struct stat stat_info;
    if (!stat(directory, &stat_info)) {
        if (S_ISDIR(stat_info.st_mode)) {
            maildir->directory = directory;
            update_maildir(maildir);
        }
    }
    LOG_INFO("Инициализация maildir завершена", NULL);
    output_maildir(maildir);

    return maildir;
}

void update_maildir(maildir_main *maildir) {
    LOG_DEBUG("Чтение структуры maildir", NULL);
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
                read_maildir_servers_new(maildir);
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

    LOG_DEBUG("Чтение сообщений пользователя: %s (%s)", user->username, user->directory);

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
    LOG_DEBUG("Чтение сообщение пользователя: %s", user->username);

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

void read_maildir_servers_new(maildir_main *maildir) {
    if (maildir == NULL) {
        LOG_ERROR("Ошибка чтения сообщений пользователей других серверов: main = NULL", NULL);
        return;
    }
    if (maildir->directory == NULL) {
        LOG_ERROR("Ошибка чтения сообщений пользователей других серверов: main.directory == NULL", NULL);
        return;
    }

    char *path_new;
    asprintf(&path_new, "%s/%s/%s", maildir->directory, DIRECTORY_OTHER_SERVERS, DIRECTORY_NEW_MESSAGES);

    LOG_DEBUG("Чтение сообщений пользователей: %s (%s)", maildir->servers, path_new);

    struct stat stat_info;
    if (!stat(path_new, &stat_info)) {
        if (!S_ISDIR(stat_info.st_mode)) {
            LOG_ERROR("Ошибка чтения структуры maildir: %s - не директория", path_new);
            return;
        }
    }

    maildir->servers.message_full_file_names = NULL;
    maildir->servers.messages_size = 0;

    DIR *dir = opendir(path_new);
    struct dirent *entry = readdir(dir);
    int messages_count = 0;
    maildir->servers.message_full_file_names = allocate_memory(sizeof(maildir->servers.message_full_file_names));
    while (entry != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            messages_count++;
            maildir->servers.message_full_file_names = reallocate_memory( maildir->servers.message_full_file_names,
                                                                          sizeof( maildir->servers.message_full_file_names)
                                                                          * messages_count);
            asprintf(&maildir->servers.message_full_file_names[messages_count - 1], "%s/%s", path_new, entry->d_name);
        }
        entry = readdir(dir);
    }
    closedir(dir);

    maildir->servers.messages_size = messages_count;
}

message *read_message(char *filepath) {
    FILE *fp;
    if ((fp = fopen(filepath, "r")) != NULL) {
        char *message = allocate_memory(256);
        while ((fgets(message, 256, fp)) != NULL) {
            LOG_INFO("%s", message);
        }
        fclose(fp);
        free(message);
    }
    free(filepath);
}

void output_maildir(maildir_main *maildir) {
    if (maildir->directory == NULL) {
        LOG_ERROR("Ошибка чтения структуры maildir: директория не найдена", NULL);
    }

    LOG_INFO("Вывод структуры каталогов maildir: ", NULL);
    maildir_other_server servers = maildir->servers;
    LOG_ADDINFO("\t.other_servers", NULL);
    if (servers.message_full_file_names != NULL) {
        for (int i = 0; i < servers.messages_size; i++) {
            LOG_ADDINFO("\t\t%s", servers.message_full_file_names[i]);
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