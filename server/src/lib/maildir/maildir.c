//
// Created by nrx on 02.11.2020.
//

#include "maildir/maildir.h"
#include "maildir/user.h"
#include "maildir/server.h"
#include "maildir/message.h"

#include <sys/types.h>
#include <regex.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>


#define WITH_CUR_FOLDER true
#define WITHOUT_CUR_FOLDER false

#define RWX_MODE 0777

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


const char MAILDIR_MD_PTR_NULL[] = "pointer of maildir object is null";
const char MAILDIR_ROOT_PATH_ERROR[] = "error opening maildir root";
const char MAILDIR_ERROR_OPENING_SERVERS_PATH[] = "error opening maildir servers path";
const char MAILDIR_PATH_DONT_CONTAINING_MAILDIR_STRUCTURE[] = "Directory of path already exists and "
                                                              "path don't containing maildir directory structure. "
                                                              "Not found " SERVERS_ROOT_NAME " directory";
const char MAILDIR_ERROR_CONVERT_RELATIVE_TO_ABSOLUTE[] = "error converting path from relative to absolute form";
const char MAILDIR_ERROR_PTR_OF_SERVER_IS_NULL[] = "pointer of server object is null";
const char MAILDIR_ERROR_PTR_OF_SERVER_NAME_IS_NULL[] = "pointer of server name is null";
const char MAILDIR_ERROR_CREATE_DIRECTORY[] = "error create directory";
const char MAILDIR_ERROR_CONCATENATE_PATHS[] = "error concatenate string for get path";
const char MAILDIR_ERROR_PTR_OF_SERVER_LIST_IS_NULL[] = "double pointer of servers list is null";
const char MAILDIR_ERROR_MAKE_DIR[] = "error make dir for maildir struct";
const char MAILDIR_ERROR_STAT_FILE_READ[] = "error get statInfo for file";
const char MAILDIR_ERROR_GET_NEXT_DIRENT[] = "error read directory for extract file names";
const char MAILDIR_ERROR_CONTAINING_FOREIGN_OBJECT[] = "user directory containing foreign object";
const char MAILDIR_SERVER_NOT_FOUND[] = "not found server with name";
const char MAILDIR_ERROR_FIND_SERVER[] = "error when performed find dir by path of the server by name";
const char MAILDIR_ERROR_STRUCTURE_OF_USER_PATH_OF_THE_SERVER[] =
        "invalid inner structure user directory of the external server";


char* pr_maildir_char_concatenate(size_t n, const char *str, ...) {

    size_t result_size = 0;
    size_t size_array[50] = {0};

    va_list va;
    va_start(va, str);
    size_array[0] = strlen(str);
    result_size += size_array[0];
    for (size_t i = 1; i < n; ++i) {
        const char *ptr = va_arg(va, const char*);
        size_array[i] = strlen(ptr);
        result_size += size_array[i];
    }
    result_size++; // учет нулевого символа в конце строки
    va_end(va);
    char *res = s_malloc(sizeof(char)*result_size, NULL);
    if (res == NULL) {
        return NULL;
    }
    strcpy(res, str);
    va_start(va, str);
    size_t offset = 0;
    for (size_t i = 1; i < n; ++i) {
        offset += size_array[i-1];
        const char *ptr = va_arg(va, const char*);
        strcpy(res + offset, ptr);
    }
    va_end(va);
    return res;
}

bool pr_maildir_path_syntax_check(char *path) {
//    regex_t regex;
//    if (regcomp(&regex, "", 0) != 0) {
//        // error;
//    }
    return true;
}

bool pr_maildir_make_dir(char *path, int mode, error_t *error) {
    if (mkdir(path, mode) != 0) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_MAKE_DIR;
        }
        return false;
    }
    return true;
}

bool pr_maildir_make_root_structure(maildir *md, char* path, error_t *error) {

    if (pr_maildir_make_dir(path, RWX_MODE, error)) {
        char *servers_path = pr_maildir_char_concatenate(2, path, SERVERS_ROOT_NAME_PART);
        bool res =  pr_maildir_make_dir(servers_path, RWX_MODE, error);
        free(servers_path);
        return res;
    }


    return false;
}

bool pr_maildir_next_dirent_entry(DIR *dir, struct dirent *entry, struct dirent **result, error_t *error) {
    int res = readdir_r(dir, entry, result);
    if (res != 0) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_GET_NEXT_DIRENT;
        }
        return false;
    }
    return true;
}

bool pr_maildir_check_is_dir(char *full_path, bool *result, error_t *error) {
    *result = false;
    struct stat entry_info;
    if (stat(full_path, &entry_info) != 0) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_STAT_FILE_READ;
        }
        return false;
    }
    if (S_ISDIR(entry_info.st_mode)) {
        *result = true;
        return true;
    }
    return true;
}

bool pr_maildir_check_user_structure(char *full_path, bool cur_must_exist, error_t *error) {
    bool is_new_dir_found = false;
    bool is_cur_dir_found = false;
    bool is_tmp_dir_found = false;

    DIR *dir = opendir(full_path);
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
    if (!pr_maildir_next_dirent_entry(dir, &entry, &result, error)) {
        closedir(dir);
        return false;
    }
    while (result != NULL) {
        char *path_name = entry.d_name;
        if (!((strcmp(path_name, ".") == 0) || (strcmp(path_name, "..") == 0))) {
            char *dir_path = pr_maildir_char_concatenate(3, full_path, "/", path_name);
            if (strcmp(path_name, USER_PATH_CUR) == 0) {
                bool is_path;
                if(!pr_maildir_check_is_dir(dir_path, &is_path, error)) {
                    free(dir_path);
                    closedir(dir);
                    return false;
                }
                if (is_path) {
                    is_cur_dir_found = true;
                }
            } else if (strcmp(path_name, USER_PATH_TMP) == 0) {
                bool is_path;
                if(!pr_maildir_check_is_dir(dir_path, &is_path, error)) {
                    free(dir_path);
                    closedir(dir);
                    return false;
                }
                if (is_path) {
                    is_tmp_dir_found = true;
                }
            } else if (strcmp(path_name, USER_PATH_NEW) == 0) {
                bool is_path;
                if(!pr_maildir_check_is_dir(dir_path, &is_path, error)) {
                    free(dir_path);
                    closedir(dir);
                    return false;
                }
                if (is_path) {
                    is_new_dir_found = true;
                }
            } else {
                if (error != NULL) {
                    error->error = FATAL;
                    error->message = MAILDIR_ERROR_CONTAINING_FOREIGN_OBJECT;

                }
                free(dir_path);
                closedir(dir);
                return false;
            }
            free(dir_path);
        }


        if (!pr_maildir_next_dirent_entry(dir, &entry, &result, error)) {
            closedir(dir);
            return false;
        }
    }
    closedir(dir);
    if (cur_must_exist) {
        return is_cur_dir_found && is_new_dir_found && is_tmp_dir_found;
    } else {
        return is_new_dir_found && is_tmp_dir_found;
    }
}


bool pr_maildir_check_server_structure(char *server_full_path, error_t *error) {
    DIR *dir = opendir(server_full_path);
    if (dir == NULL) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_OPENING_SERVERS_PATH;
        }
        return false;
    }

    struct dirent entry;
    struct dirent *result = NULL;

    if (!pr_maildir_next_dirent_entry(dir, &entry, &result, error)) {
        closedir(dir);
        return false;
    }
    while (result != NULL) {
        if (!((strcmp(entry.d_name, ".") == 0) || (strcmp(entry.d_name, "..") == 0))) {
            char *user_of_server_path = pr_maildir_char_concatenate(3, server_full_path, "/", entry.d_name);
            if (user_of_server_path == NULL) {
                if (error != NULL) {
                    error->error = FATAL;
                    error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
                }
                closedir(dir);
                return false;
            }

            bool res = pr_maildir_check_user_structure(user_of_server_path, WITHOUT_CUR_FOLDER, error);
            free(user_of_server_path);
            if (res == false) {
                if (error != NULL) {
                    error->error = FATAL;
                    error->message = MAILDIR_ERROR_STRUCTURE_OF_USER_PATH_OF_THE_SERVER;
                }
                closedir(dir);
                return false;
            }
        }


        if (!pr_maildir_next_dirent_entry(dir, &entry, &result, error)) {
            closedir(dir);
            return false;
        }
    }
    return true;
}

bool pr_maildir_check_other_servers_structure(char *server_full_path, error_t *error) {
    DIR *dir = opendir(server_full_path);
    if (dir == NULL) {
        if(error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_OPENING_SERVERS_PATH;
        }
        return false;
    }

    struct dirent entry;
    struct dirent *result;
    if (!pr_maildir_next_dirent_entry(dir, &entry, &result, error)) {
        closedir(dir);
        return false;
    }
    while (result != NULL) {
        if (!((strcmp(entry.d_name, ".") == 0)||(strcmp(entry.d_name, "..") == 0))) {
            char *full_path = pr_maildir_char_concatenate(3, server_full_path, "/", entry.d_name);
            if (full_path == NULL) {
                if (error != NULL) {
                    error->error = FATAL;
                    error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
                }
                closedir(dir);
                return false;
            }

            if(!pr_maildir_check_server_structure(full_path, error)) {
                free((full_path));
                closedir(dir);
                return false;
            }
            free(full_path);
        }

        if (!pr_maildir_next_dirent_entry(dir, &entry, &result, error)) {
            closedir(dir);
            return false;
        }
    }
    return true;
}

bool pr_maildir_check_root_structure(maildir *md, DIR *dir, error_t *error) {
    struct dirent entry;
    struct dirent *result;
    bool servers_dir_is_founded = false;
    if (!pr_maildir_next_dirent_entry(dir, &entry, &result, error)) {
        return false;
    }
    struct stat entryInfo;
    while (result != NULL) {
        char *path_name = entry.d_name;
        if (!((strcmp(path_name, ".") == 0)||(strcmp(path_name, "..") == 0))) {
            char *full_path = pr_maildir_char_concatenate(3, md->pr_path, "/", path_name);
            if (full_path == 0) {
                if (error != NULL) {
                    error->error = FATAL;
                    error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
                }
                return false;
            }

            if (strcmp(path_name, SERVERS_ROOT_NAME) == 0) {
                bool is_dir;
                if (!pr_maildir_check_is_dir(full_path, &is_dir, error)) {
                    free(full_path);
                    return false;
                }
                if (is_dir) {
                    servers_dir_is_founded =  true;
                    if (!pr_maildir_check_other_servers_structure(full_path, error)) {
                        free(full_path);
                        return false;
                    }
                }

            } else {
                bool is_dir;
                if (!pr_maildir_check_is_dir(full_path, &is_dir, error)) {
                    free(full_path);
                    return false;
                }
                if (!pr_maildir_check_user_structure(full_path, WITH_CUR_FOLDER, error)) {
                    free(full_path);
                    return false;
                }
            }
            free(full_path);
        }


        if (!pr_maildir_next_dirent_entry(dir, &entry, &result, error)) {
            return false;
        }
    }

    return true;
}

bool pr_maildir_relative_to_absolute_path(maildir *md, char *path, error_t *error) {
    char *buffer = s_malloc(PATH_MAX+1, error);
    if (buffer == NULL) {
        return false;
    }
    md->pr_path = realpath(path, buffer);
    if (md->pr_path == NULL) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_CONVERT_RELATIVE_TO_ABSOLUTE;
        }
        return false;
    }
    return true;
}

bool maildir_init(maildir *md, char* path, error_t *error) {
    CHECK_PTR(md, error, MAILDIR_MD_PTR_NULL);
    CHECK_PTR(path, error, MAILDIR_MD_PTR_NULL);
    ERROR_SUCCESS(error);

    DIR *dir = opendir(path);
    if (dir == NULL) {

        if (errno == ENOENT) {
            if (pr_maildir_make_root_structure(md, path, error)) {
                bool res = pr_maildir_relative_to_absolute_path(md, path, error);
                return res;
            }
            return false;
        }

        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ROOT_PATH_ERROR;
        }
        return false;
    }
    if (pr_maildir_relative_to_absolute_path(md, path, error)) {
        bool res = pr_maildir_check_root_structure(md, dir, error);
        closedir(dir);
        return res;
    }
    closedir(dir);
    return false;

}

void maildir_free(maildir *md) {
    if (md == NULL) {
        return;
    }
    free(md->pr_path);
}
bool maildir_release(maildir *md, error_t *error) {
    CHECK_PTR(md, error, MAILDIR_MD_PTR_NULL);
    // TODO (ageev) реализовать очистку папки maildir
    return true;
}


bool maildir_server_list(maildir *md, maildir_servers_list *servers_list, error_t *error) {
    CHECK_PTR(md, error, MAILDIR_MD_PTR_NULL);
    CHECK_PTR(servers_list, error, MAILDIR_ERROR_PTR_OF_SERVER_LIST_IS_NULL);
    char *path = pr_maildir_char_concatenate(2 , md->pr_path, SERVERS_ROOT_NAME_PART);
    if (path == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
        }
        return false;
    }

    DIR *dir = opendir(path);
    if (dir == NULL) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_OPENING_SERVERS_PATH;
        }
        free(path);
        return false;
    }
    for(struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
            char *server_path = pr_maildir_char_concatenate(3, path, "/", entry->d_name);
            struct stat entry_info;
            stat(server_path, &entry_info);
            if (S_ISDIR(entry_info.st_mode)) {
                maildir_server_entry *server_entry = s_malloc(sizeof(maildir_server_entry), error);
                if (server_entry == NULL) {
                    free(path);
                    free(server_path);
                    closedir(dir);
                    return false;
                }
                strcpy(server_entry->server.pr_server_domain, entry->d_name);
                server_entry->server.pr_md = md;
                LIST_INSERT_HEAD(servers_list, server_entry, entries);
                free(server_path);
            }
        }
    }
    free(path);
    closedir(dir);
    return true;
}

bool maildir_get_self_server(maildir *md, maildir_server *server, error_t *error) {
    CHECK_PTR(md, error, MAILDIR_MD_PTR_NULL);
    CHECK_PTR(server, error, MAILDIR_ERROR_PTR_OF_SERVER_IS_NULL);
    server->pr_server_domain[0] = '\0';
    server->pr_md = md;
    return true;
}

bool maildir_get_server_by_name(maildir *md, maildir_server *server,const char *name,  error_t *error) {
    CHECK_PTR(md, error, MAILDIR_MD_PTR_NULL);
    CHECK_PTR(server, error, MAILDIR_ERROR_PTR_OF_SERVER_IS_NULL);
    CHECK_PTR(name, error, MAILDIR_ERROR_PTR_OF_SERVER_NAME_IS_NULL);
    char *server_full_path = pr_maildir_char_concatenate(3, md->pr_path, SERVERS_ROOT_NAME_PART, name);
    if (server_full_path == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
        }
        return NULL;
    }
    DIR *dir = opendir(server_full_path);
    if (dir == NULL) {
        if (errno == ENOENT) {
            if (error != NULL) {
                error->error = NOT_FOUND;
                error->message = MAILDIR_SERVER_NOT_FOUND;
            }
        } else {
            if (error != NULL) {
                error->error = ERRNO;
                error->errno_value = errno;
                error->message = MAILDIR_ERROR_FIND_SERVER;
            }
        }
        return false;
    }
    closedir(dir);

    strcpy(server->pr_server_domain, name);
    server->pr_md = md;
    return true;

}



bool maildir_create_server(maildir *md, maildir_server *server, char *server_domain, error_t *error) {
    CHECK_PTR(md, error, MAILDIR_MD_PTR_NULL);
    CHECK_PTR(server, error, MAILDIR_ERROR_PTR_OF_SERVER_IS_NULL);
    CHECK_PTR(server_domain, error, MAILDIR_ERROR_PTR_OF_SERVER_NAME_IS_NULL);
    char *path = pr_maildir_char_concatenate(3, md->pr_path, SERVERS_ROOT_NAME_PART, server_domain);
    if (path == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_ERROR_CONCATENATE_PATHS;
        }
        return false;
    }
    if (mkdir(path, RWX_MODE) == -1) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_ERROR_CREATE_DIRECTORY;
        }
        free(path);
        return false;
    }
    strcpy(server->pr_server_domain, server_domain);
    server->pr_md = md;
    free(path);
    return  true;
}

bool maildir_delete_server(maildir *md, maildir_server *server, error_t *error) {
    // TODO (ageev) Написать функцию удаляющую все содержимое папки с сервером
    return false;
}