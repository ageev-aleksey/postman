#include "maildir/message.h"
#include "maildir/maildir.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

const char MAILDIR_MESSAGE_ERROR_ALREADY_COMPLETED_WORK[] = "already completed working with message";
const char MAILDIR_MESSAGE_ERROR_REMOVE[] = "error remove message";
const char MAILDIR_MESSAGE_ERROR_RENAME_FILE[] = "error move file from tmp to new dir";

extern bool pr_maildir_char_make_buf_concat(char **buffer, size_t *bsize, size_t nargs,  const char *str, ...);
extern bool pr_maildir_make_full_path(maildir_user *user, char **result_path, error_t *error);

bool pr_maildir_message_make_full_path(maildir_message *msg, message_type path_type, char **result_path, error_t *error) {
    size_t size = 0;
    char *user_path = NULL;
    if (!pr_maildir_make_full_path(msg->pr_user, &user_path, error)) {
        return false;
    }
    if (path_type == NEW) {
        if (!pr_maildir_char_make_buf_concat(result_path, &size, 5, user_path, "/", USER_PATH_NEW, "/", msg->pr_filename)) {
            return false;
        }
    } else if (path_type == TMP) {
        if (!pr_maildir_char_make_buf_concat(result_path, &size, 5, user_path, "/", USER_PATH_TMP, "/", msg->pr_filename)) {
            return false;
        }
    }
    return true;
}

bool maildir_message_free(maildir_message *msg) {
    if (msg->pr_is_open) {
        fclose(msg->pr_fd);
    }
    free(msg->pr_filename);
}

bool maildir_message_release(maildir_message *msg, error_t *error) {
    char *msg_path = NULL;
    if (!pr_maildir_message_make_full_path(msg, msg->pr_type, &msg_path, error)) {
        return false;
    }

    if (unlink(msg_path) != 0) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_MESSAGE_ERROR_REMOVE;
        }
        return false;
    }
    return true;
}

bool maildir_message_finalize(maildir_message *msg, error_t *error) {
    bool status = false;
    if (msg->pr_type == NEW) {
        return true;
    }
    if (!msg->pr_is_open) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_MESSAGE_ERROR_ALREADY_COMPLETED_WORK;
        }
        return false;
    }
    fclose(msg->pr_fd);
    msg->pr_is_open = false;

    char *msg_path = NULL;
    if (!pr_maildir_message_make_full_path(msg, TMP, &msg_path, error)) {
        goto exit;
    }
    char *msg_new_path = NULL;
    if (!pr_maildir_message_make_full_path(msg, NEW, &msg_new_path, error)) {
        goto  exit;
    }

    if (rename(msg_path, msg_new_path) != 0) {
        if (error != NULL) {
            error->error = ERRNO;
            error->errno_value = errno;
            error->message = MAILDIR_MESSAGE_ERROR_RENAME_FILE;
        }
        goto exit;
    }
    status = true;

    exit:
    free(msg_path);
    free(msg_new_path);
    return status;
}

bool maildir_message_get_user(maildir_message *msg, maildir_user **user, error_t *error) {
    *user = msg->pr_user;
    return true;
}

bool maildir_message_write(maildir_message *msg, char *buffer, error_t *error) {

}