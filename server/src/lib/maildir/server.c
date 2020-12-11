//
// Created by nrx on 08.11.2020.
//

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "maildir/server.h"
#include "maildir/maildir.h"
#include "maildir/user.h"
#include "util.h"

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


const char MAILDIR_SERVER_ERROR_SERVER_PTR_IS_NULL[] = "ptr of server object is null";
const char MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL[] = "argument of function is null";
const char MAILDIR_SERVER_ERROR_CREATE_USER_ROOT_DIR[] = "error create root user dir";
const char MAILDIR_SERVER_ERROR_CREATE_USER_TMP_DIR[] = "error create tmp user dir";
const char MAILDIR_SERVER_ERROR_CREATE_USER_CUR_DIR[] = "error create cur user dir";
const char MAILDIR_SERVER_ERROR_CREATE_USER_NEW_DIR[] = "error create new user dir";
const char MAILDIR_SERVER_ERROR_USER_NOT_FOUND[] = "not found user in server";
extern char MAILDIR_ERROR_OPENING_SERVERS_PATH[];

extern char* pr_maildir_char_concatenate(size_t n, const char *str, ...);

extern const char MAILDIR_ERROR_CONCATENATE_PATHS[];

extern bool pr_maildir_next_dirent_entry(DIR *dir, struct dirent *entry, struct dirent **result, err_t *error);

bool maildir_server_is_self(maildir_server *server, bool *res, err_t *error) {
    CHECK_PTR(server, error, MAILDIR_SERVER_ERROR_SERVER_PTR_IS_NULL);
    CHECK_PTR(res, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);

    *res = server->pr_server_domain[0] == '\0';
    return true;
}

bool pr_maildir_server_path(maildir_server *server, char **path) {
    size_t length = 0;
    bool is_self = false;
    if (!maildir_server_is_self(server, &is_self, NULL)) {
        return false;
    }
    if (is_self) {
        return char_make_buf_concat(path, &length, 3, server->pr_md->pr_path, "/", server);
    } else {
        return char_make_buf_concat(path, &length, 3, server->pr_md->pr_path, SERVERS_ROOT_NAME_PART, server);
    }

}


bool maildir_server_users(maildir_server *server, maildir_users_list *users, err_t *error) {
    CHECK_PTR(server, error, MAILDIR_SERVER_ERROR_SERVER_PTR_IS_NULL);
    CHECK_PTR(users, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);

    char *full_path = NULL;
    if (!pr_maildir_server_path(server, &full_path)) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
        }
    }
    DIR *dir = opendir(full_path);
    free(full_path);
    if (dir == NULL) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_OPENING_SERVERS_PATH;
        }
        return false;
    }
    struct dirent entry;
    struct dirent *result;
    if (!pr_maildir_next_dirent_entry(dir,&entry, &result, error)) {
        return false;
    }
    while (result != NULL) {
        if (!((strcmp(entry.d_name, ".") == 0)||(strcmp(entry.d_name, "..") == 0))) {
            maildir_users_entry *list_entry = s_malloc(sizeof(maildir_users_entry), error);
            if (list_entry == NULL) {
                closedir(dir);
                return false;
            }
            strcpy(list_entry->user.pr_login, entry.d_name);
            list_entry->user.pr_server = server;
            LIST_INSERT_HEAD(users, list_entry, entries);
        }

        if (!pr_maildir_next_dirent_entry(dir,&entry, &result, error)) {
            return false;
        }
    }

    closedir(dir);
    return true;
}

bool maildir_server_domain(maildir_server *server, char **domain, err_t *error) {
    CHECK_PTR(server, error, MAILDIR_SERVER_ERROR_SERVER_PTR_IS_NULL);
    CHECK_PTR(domain, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);
    ERROR_SUCCESS(error);
    *domain = server->pr_server_domain;
    return true;
}

bool pr_maildir_server_mkdir(char *path, err_t *error, const char *error_message) {
    if (mkdir(path, S_IRWXU) != 0) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = error_message;
        }
        return false;
    }
    return true;
}

bool maildir_server_create_user(maildir_server *server, maildir_user *user, const char *username, err_t *error) {
    CHECK_PTR(server, error, MAILDIR_SERVER_ERROR_SERVER_PTR_IS_NULL);
    CHECK_PTR(user, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);
    CHECK_PTR(username, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);
    ERROR_SUCCESS(error);

    char *server_path = NULL;
    char *user_path = NULL;
    char *path = NULL;
    size_t path_length = 0;

    if (!pr_maildir_server_path(server, &server_path)) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
        }
        return false;
    }

    bool status = false;

    size_t length = 0;

    if (!char_make_buf_concat(&user_path, &length, 3, server_path, "/", username)) {
        if (error != NULL) {
                error->error = FATAL;
                error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
        }
        status = false;
        goto exit;
    }




    if (pr_maildir_server_mkdir(user_path, error, MAILDIR_SERVER_ERROR_CREATE_USER_ROOT_DIR)) {
        if (!char_make_buf_concat(&path, &path_length, 3, user_path, "/", USER_PATH_TMP)) {
            if (error != NULL) {
                error->error = FATAL;
                error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
            }
            status = false;
            goto exit;
        }
        bool is_make_dir = pr_maildir_server_mkdir(path, error, MAILDIR_SERVER_ERROR_CREATE_USER_TMP_DIR);
        if (is_make_dir) {
            bool is = false;
            maildir_server_is_self(server, &is, error);
            if (is) {
                if (!char_make_buf_concat(&path, &path_length, 3, user_path, "/", USER_PATH_CUR)) {
                    if (error != NULL) {
                        error->error = FATAL;
                        error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
                    }
                    status = false;
                    goto exit;
                }

                is_make_dir = pr_maildir_server_mkdir(path, error, MAILDIR_SERVER_ERROR_CREATE_USER_CUR_DIR);
            }

            if (is_make_dir) {
                if (!char_make_buf_concat(&path, &path_length, 3, user_path, "/", USER_PATH_NEW)) {
                    if (error != NULL) {
                        error->error = FATAL;
                        error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
                    }
                    status = false;
                    goto exit;
                }

                is_make_dir = pr_maildir_server_mkdir(path, error, MAILDIR_SERVER_ERROR_CREATE_USER_NEW_DIR);
                status = is_make_dir;
            }

        }
    }

exit:
    free(server_path);
    free(user_path);
    free(path);
    user->pr_server = server;
    strcpy(user->pr_login, username);
    return status;
}

void maildir_server_default_init(maildir_server *server) {
    server->pr_server_domain[0] = '\0';
    server->pr_md = NULL;
}

void maildir_server_free(maildir_server *server) {
    if (server != NULL) {
        server->pr_md = NULL;
    }
}

bool maildir_server_user(maildir_server *server, maildir_user *user, const char *username, err_t *error) {
    CHECK_PTR(server, error, MAILDIR_SERVER_ERROR_SERVER_PTR_IS_NULL);
    CHECK_PTR(user, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);
    CHECK_PTR(username, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);
    ERROR_SUCCESS(error);

    char *path = NULL;
    if (!pr_maildir_server_path(server, &path)) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
        }
        return false;
    }
    char *user_path = NULL;
    size_t up_length = 0;
    if (!char_make_buf_concat(&user_path, &up_length, 3, path,"/", username)) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
        }
        return false;
    }

    DIR *dir = opendir(user_path);
    free(path);
    free(user_path);
    if (dir == NULL) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_SERVER_ERROR_USER_NOT_FOUND;
        }

        return false;
    }

    strcpy(user->pr_login, username);
    user->pr_server = server;

    return true;
}