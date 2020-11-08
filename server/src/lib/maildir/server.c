//
// Created by nrx on 08.11.2020.
//

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include "maildir/server.h"
#include "maildir/maildir.h"
#include "maildir/user.h"

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
extern char MAILDIR_ERROR_OPENING_SERVERS_PATH[];

extern char* pr_maildir_char_concatenate(size_t n, const char *str, ...);
extern const char MAILDIR_ERROR_CONCATENATE_PATHS[];

extern bool pr_maildir_next_dirent_entry(DIR *dir, struct dirent *entry, struct dirent **result, error_t *error);

bool maildir_server_is_self(maildir_server *server, bool *res, error_t *error) {
    CHECK_PTR(server, error, MAILDIR_SERVER_ERROR_SERVER_PTR_IS_NULL);
    CHECK_PTR(res, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);

    *res = server->pr_server_domain[0] == '\0';
    return true;
}

bool maildir_server_users(maildir_server *server, maildir_users_list *users, error_t *error) {
    CHECK_PTR(server, error, MAILDIR_SERVER_ERROR_SERVER_PTR_IS_NULL);
    CHECK_PTR(users, error, MAILDIR_SERVER_ERROR_ARGUMENT_PTR_IS_NULL);

    char *full_path = pr_maildir_char_concatenate(3, server->pr_md->pr_path, SERVERS_ROOT_NAME_PART, server);
    if (full_path == NULL) {
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