//
// Created by nrx on 13.11.2020.
//

#include <dirent.h>
#include <errno.h>
#include "maildir/user.h"
#include "maildir/server.h"
#include "maildir/maildir.h"
#include "maildir/message.h"
#include "util.h"

#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#define MAILDIR_MAX_RANDOM_VALUE 99999
#define MAX_NUM_ATTEMPTS_TO_CREATE_FILENAME 10
#define FILE_READ "r"
#define FILE_WRITE "w"

#define CHECK_PTR(ptr_, error_, error_message_) \
do {                                    \
    if ((ptr_) == NULL) {               \
        if ((error_) != NULL) { \
            (error_)->error = FATAL; \
            (error_)->message = (error_message_); \
        } \
        return false;   \
    }\
} while(0)

const char MAILDIR_USER_PTR_IS_NULL[] = "ptr of maildir_user is null";
const char MAILDIR_MESSAGE_PTR_IS_NULL[] = "ptr of maildir_message is null";
const char MAILDIR_MESSAGE_ERROR_OPEN_PATH[] = "error open path";
const char MAILDIR_MESSAGE_ERROR_CONCATENATE_STR[] = "error concatenate paths";
const char MAILDIR_MESSAGE_ERROR_GENERATE_FILENAME[] = "error creating unique filename for message";
const char MAILDIR_MESSAGE_ERROR_CREATE_FILE[] = "error crating file of message";

extern char* pr_maildir_char_concatenate(size_t n, const char *str, ...);
extern bool pr_maildir_next_dirent_entry(DIR *dir, struct dirent *entry, struct dirent **result, err_t *error);

bool pr_maildir_make_full_path(maildir_user *user, char **result_path, err_t *error) {
    bool is_self = false;
    size_t size = 0;
    maildir_server_is_self(user->pr_server, &is_self, error);
    if (is_self) {
        char_make_buf_concat(result_path, &size, 5,
                             user->pr_server->pr_md->pr_path,
                             "/",
                             user->pr_server->pr_server_domain,
                             "/",
                             user->pr_login);
        if (*result_path == NULL) {
            return false;
        }
    } else {
        char_make_buf_concat(result_path, &size, 5,
                             user->pr_server->pr_md->pr_path,
                             SERVERS_ROOT_NAME_PART,
                             user->pr_server->pr_server_domain,
                             "/",
                             user->pr_login);
        if (*result_path == NULL) {
            return false;
        }
    }
    return true;

}

bool pr_maildir_user_opendir(char *path, DIR **dir, err_t *error) {
    *dir = opendir(path);
    if (*dir == NULL) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_MESSAGE_ERROR_OPEN_PATH;
        }
        return false;
    }
    return true;
}

void pr_maildir_message_filename_generate(char *file_name, char* sender_name) {
    time_t timestamp = time(NULL);
    pid_t pid = getpid();
    int random_value = rand() / MAILDIR_MAX_RANDOM_VALUE;
    sprintf(file_name, "%ld%s%d%d", timestamp, sender_name, pid, random_value);
}


bool maildir_user_create_message(maildir_user *user, maildir_message *message, char *sender_name, err_t *error) {
    CHECK_PTR(user, error, MAILDIR_USER_PTR_IS_NULL);
    CHECK_PTR(message, error, MAILDIR_MESSAGE_PTR_IS_NULL);
    CHECK_PTR(sender_name, error, MAILDIR_MESSAGE_PTR_IS_NULL);

    char *user_full_path = NULL;
    bool status = false;

    if (!pr_maildir_make_full_path(user, &user_full_path, error)) {
        return false;
    }

//    DIR *tmp_dir = NULL;
//    DIR *new_dir = NULL;
//    if (!pr_maildir_user_opendir(tmp_path, &tmp_dir, error) &&
//        !pr_maildir_user_opendir(new_path, &new_dir, error)) {
//        goto error_exit;
//    }



    // Генерация имени файла до тех пор, пока в обоих папках такого файлна не будет
    char *tmp_file_path = NULL;
    FILE *in_tmp = NULL;
    FILE *in_new = NULL;
    bool is_continue = true;
    int attempts =  0;
    char *tmp_path = NULL;
    size_t tmp_path_size = 0;
    char *new_path = NULL;
    size_t new_path_size = 0;
    // Создание уникального имени для файла
    // Внутри цикла проверяется, что созданное имя не зането в папках пользователя сервера
    //   - tmp
    //   - new
    // Если имя зането, то выполняется новая попытка создать уникальное имя файла.
    // Если бло принято MAX_NUM_ATTEMPTS_TO_CREATE_FILENAME попыток, то выход из функции с ошибкой
    while (is_continue) {
        pr_maildir_message_filename_generate(message->pr_filename, sender_name);
        char_make_buf_concat(&tmp_path, &tmp_path_size, 5, user_full_path, "/",
                             USER_PATH_TMP, "/", message->pr_filename);
        char_make_buf_concat(&new_path, &new_path_size, 5, user_full_path, "/",
                             USER_PATH_NEW, "/", message->pr_filename);
        if (tmp_path == NULL || new_path == NULL) {
            if (error != NULL) {
                error->error = FATAL;
                error->message = MAILDIR_MESSAGE_ERROR_CONCATENATE_STR;
            }
            status = false;
            goto exit;
        }
        in_tmp = fopen(tmp_path, FILE_READ);
        in_new = fopen(new_path, FILE_READ);
        if (in_tmp != NULL || in_new != NULL) {
            if (in_tmp != NULL) {
                fclose(in_tmp);
            }
            if (in_new != NULL) {
                fclose(in_new);
            }
            is_continue = true;
        } else {
            is_continue = false;
        }
        attempts++;
        if (attempts >= MAX_NUM_ATTEMPTS_TO_CREATE_FILENAME) {
            if (error != NULL) {
                error->error = FATAL;
                error->message = MAILDIR_MESSAGE_ERROR_GENERATE_FILENAME;
            }
            status = false;
            goto exit;
        }
       // sleep(DELAY_FOR_FILENAME_GENERATOR);
    }


    in_tmp = fopen(tmp_path, FILE_WRITE);
    if (in_tmp == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_MESSAGE_ERROR_CREATE_FILE;
        }
        status = false;
        goto exit;
    }

    message->pr_user = user;
    message->pr_type = TMP;
    message->pr_fd = in_tmp;
    message->pr_is_open = true;
    status = true;

exit:
    free(tmp_path);
    free(new_path);
    free(user_full_path);
    return status;

}

void maildir_user_free(maildir_user *user) {

}

bool maildir_user_login(maildir_user *user, char **login) {
    *login = user->pr_login;
    return true;
}

bool maildir_user_server(maildir_user *user, maildir_server **server) {
    *server = user->pr_server;
    return true;
}

bool maildir_user_message_list(maildir_user *user, maildir_messages_list *msg_list, err_t *error) {
//    char *path = NULL;
//    if (!pr_maildir_make_full_path(user, &path, error)) {
//        return false;
//    }
//    DIR *dir = opendir(path);
//    if (dir == NULL) {
//        if (error != NULL) {
//            error->error = ERRNO;
//            error->errno_value = errno;
//            error->message = MAILDIR_USER_ERROR_OPEN_MSG_CUR_DIR;
//        }
//        free(path);
//        return false;
//    }
}