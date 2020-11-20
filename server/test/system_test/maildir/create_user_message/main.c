//
// Created by nrx on 05.11.2020.
//

#include "maildir/maildir.h"
#include "maildir/server.h"
#include "maildir/user.h"
#include "maildir/message.h"
#include <stdio.h>
#include <string.h>
#include <asm/errno.h>

#define MD_PATH "md_path_test"
#define MD_SERVER_NAME "test_server.ru"
#define MD_USER_NAME  "test_user"
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
            printf("Error maildir init: %s\n", error.message);
        }

        return ERROR;
    }
    maildir_server server;
    bool status = maildir_create_server(md, &server, MD_SERVER_NAME, &error);
    if (error.error == ERRNO && error.errno_value == EEXIST) {
        status = maildir_get_server_by_name(md, &server, MD_SERVER_NAME, &error);
    }

    if (!status) {
        if (error.error == ERRNO) {
            printf("Error maildir create server: %s\n", strerror(error.errno_value));
        } else {
            printf("Error maildir create server: %s\n", error.message);
        }
        return ERROR;
    }

    maildir_user user;
    status = maildir_server_create_user(&server, &user, MD_USER_NAME , &error);
    if (error.error == ERRNO && error.errno_value == EEXIST) {
        status = maildir_server_user(&server, &user, MD_USER_NAME, &error);
    }
    if(!status) {
        if (error.error == ERRNO) {
            printf("Error maildir create user: %s\n", strerror(error.errno_value));
        } else {
            printf("Error maildir create user: %s\n", error.message);
        }
        return ERROR;
    }

    maildir_message msg;
    if (!maildir_user_create_message(&user, &msg, "test_sender.ru", &error)) {
        if (error.error == ERRNO) {
            printf("Error maildir create message: %s\n", strerror(error.errno_value));
        } else {
            printf("Error maildir create message: %s\n", error.message);
        }
        return ERROR;
    }
    char message1[] = "First_part message\n";
    char message2[] = "Second_part message\n";

    if (!maildir_message_write(&msg, message1, sizeof(message1)-1, &error)) {
        if (error.error == ERRNO) {
            printf("Error maildir write message: %s\n", strerror(error.errno_value));
        } else {
            printf("Error maildir write message: %s\n", error.message);
        }
        return ERROR;
    }

    if (!maildir_message_write(&msg, message2, sizeof(message2)-1, &error)) {
        if (error.error == ERRNO) {
            printf("Error maildir write message: %s\n", strerror(error.errno_value));
        } else {
            printf("Error maildir write message: %s\n", error.message);
        }
        return ERROR;
    }

    if (!maildir_message_finalize(&msg, &error)) {
        if (error.error == ERRNO) {
            printf("Error maildir write message: %s\n", strerror(error.errno_value));
        } else {
            printf("Error maildir write message: %s\n", error.message);
        }
        return ERROR;
    }

    maildir_message_free(&msg);
    maildir_user_free(&user);
    maildir_server_free(&server);
    maildir_free(md);
    return OK;
}