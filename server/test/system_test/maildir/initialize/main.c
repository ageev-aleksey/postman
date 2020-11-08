//
// Created by nrx on 05.11.2020.
//

#include "maildir/maildir.h"
#include "maildir/server.h"
#include "maildir/user.h"
#include <stdio.h>
#include <string.h>

#define MD_PATH "md_path_test"
#define OK 0
#define ERROR (-1)

int main() {
    maildir *md = s_malloc(sizeof(maildir), NULL);
    if (md == NULL) {
        printf("Error allocated memory\n");
        return ERROR;
    }
    error_t  error;
    if (!maildir_init(md, MD_PATH, &error)) {
        if (error.error == ERRNO) {
            printf("Error maildir init: %s\n", strerror(error.errno_value));
        } else {
            printf("Error maildir init\n");
        }

        return ERROR;
    }

    maildir_servers_list *servers = malloc(sizeof(maildir_servers_list));
    if (servers == NULL) {
        perror("malloc");
        return ERROR;
    }
    LIST_INIT(servers);

    maildir_server_list(md, servers,&error);
    if (error.error) {
        if (error.error == ERRNO) {
            printf("get servers list error: %s -> %s\n", error.message, strerror(error.errno_value));
        } else {
            printf("get servers list error: %s\n", error.message);
        }
        return ERROR;
    }

    maildir_server_entry *ptr = NULL;
    LIST_FOREACH(ptr, servers, entries) {
        printf("server name: %s\n", ptr->server.pr_server_domain);
        maildir_users_list *users = malloc(sizeof(maildir_users_list));
        if (users == NULL) {
            perror("malloc");
            return ERROR;
        }
        LIST_INIT(users);
        maildir_server_users(&ptr->server, users, &error);

        maildir_users_entry *uptr = NULL;
        LIST_FOREACH(uptr, users, entries) {
            printf(" -- user name: %s\n", uptr->user.pr_login);
        }
    }

    return OK;
}