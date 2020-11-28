//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_ERROR_T_H
#define SERVER_ERROR_T_H

#include <stdbool.h>
#include <stddef.h>

extern const char ERROR_SUCCESS[];

enum ErrorType {
    OK = 0,
    NOT_FOUND,
    EXISTS,
    EMPTY,
    FATAL,
    ERRNO,
    OUT_OF_RANGE,
};

typedef struct _error_t {
    const char *message;
    enum ErrorType error;
    int errno_value;
} error_t;

const char *errtostr(enum ErrorType type);

#define ERROR_SUCCESS(_e_)                \
do {                                        \
    if ((_e_) != NULL) {                  \
        (_e_)->error = OK;                \
        (_e_)->message = ERROR_SUCCESS;   \
    }                                       \
}while(0)


#define CHECK(_q_, _err_, _msg_) \
do {             \
    if ((_q_) == NULL) { \
        (_err_)->error = FATAL; \
        (_err_)->message = (_msg_);                  \
    }\
} while(0)

#define CHECK_AND_RETURN(_q_, _err_, _msg_)     \
do {                            \
    error_t err;                            \
    err.error = OK;                         \
    err.message = ERROR_SUCCESS; \
    CHECK((_q_), &err, (_msg_));          \
    if (err.error) {         \
        if ((_err_) != NULL) {    \
            *(_err_) = err;       \
        }                       \
        return NULL;            \
    }                           \
} while(0)

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

#endif //SERVER_ERROR_T_H
