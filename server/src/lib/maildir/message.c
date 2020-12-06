#include "maildir/message.h"
#include "maildir/maildir.h"
#include "util.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>


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

const char MAILDIR_MESSAGE_ERROR_ALREADY_COMPLETED_WORK[] = "already completed working with message";
const char MAILDIR_MESSAGE_ERROR_REMOVE[] = "error remove message";
const char MAILDIR_MESSAGE_ERROR_RENAME_FILE[] = "error move file from tmp to new dir";
const char MAILDIR_MESSAGE_ERROR_MSG_DESCRIPTOR_IS_NULL[] = "ptr of maildir_message is null";
const char MAILDIR_MESSAGE_ERROR_PARAMETER_IS_NULL[] = "ptr of parameter function is null";
const char MAILDIR_MESSAGE_ERROR_WRITE_TO_FILE[] = "error write buffer to file";
const char MAILDIR_MESSAGE_INVALID_SIZE_PARAMETER[] = "error size of buffer equal zero";


extern bool pr_maildir_make_full_path(maildir_user *user, char **result_path, err_t *error);

bool pr_maildir_message_make_full_path(maildir_message *msg, message_type path_type, char **result_path, err_t *error) {
    bool status = false;
    size_t size = 0;
    char *user_path = NULL;
    if (!pr_maildir_make_full_path(msg->pr_user, &user_path, error)) {
        goto exit;
    }
    if (path_type == NEW) {
        if (!char_make_buf_concat(result_path, &size, 5, user_path, "/", USER_PATH_NEW, "/", msg->pr_filename)) {
            goto exit;
        }
    } else if (path_type == TMP) {
        if (!char_make_buf_concat(result_path, &size, 5, user_path, "/", USER_PATH_TMP, "/", msg->pr_filename)) {
            goto exit;
        }
    }
    status = true;
exit:
    free(user_path);
    return status;
}

void maildir_message_free(maildir_message *msg) {
    if (msg->pr_is_open) {
        fclose(msg->pr_fd);
    }
}

bool maildir_message_release(maildir_message *msg, err_t *error) {
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

bool maildir_message_finalize(maildir_message *msg, err_t *error) {
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
    char *msg_new_path = NULL;

    if (!pr_maildir_message_make_full_path(msg, TMP, &msg_path, error)) {
        goto exit;
    }
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

bool maildir_message_get_user(maildir_message *msg, maildir_user **user, err_t *error) {
    *user = msg->pr_user;
    return true;
}

bool maildir_message_write(maildir_message *msg, const char *buffer, size_t b_len, err_t *error) {
    CHECK_PTR(msg, error, MAILDIR_MESSAGE_ERROR_MSG_DESCRIPTOR_IS_NULL);
    CHECK_PTR(buffer, error, MAILDIR_MESSAGE_ERROR_PARAMETER_IS_NULL);
    if (b_len == 0) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_MESSAGE_INVALID_SIZE_PARAMETER;
        }
        return false;
    }
    if (!msg->pr_is_open) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = MAILDIR_MESSAGE_ERROR_ALREADY_COMPLETED_WORK;
        }
        return false;
    }
//    int fd = fileno(msg->pr_fd);
//    write(fd, buffer, b_len);
    size_t res = fwrite(buffer, sizeof(char), b_len, msg->pr_fd);
    if (res == 0) {
        if (error != 0) {
            error->error = FATAL;
            error->message = MAILDIR_MESSAGE_ERROR_WRITE_TO_FILE;
        }
        return false;
    }
    return true;
}