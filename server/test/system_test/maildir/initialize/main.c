//
// Created by nrx on 05.11.2020.
//

#include "maildir/maildir.h"
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

    return OK;
}