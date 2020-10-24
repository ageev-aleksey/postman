//
// Created by nrx on 08.10.2020.
//


#include "util.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define __USE_MISC
#include <arpa/inet.h>
#define ERROR (-1)

const char ERROR_SUCCESS[] = "Success";
const char UTIL_ERROR_ALLOCATED_MEMORY[] = "Error allocated memory";
const char UTIL_ERROR_CONVERT_IP[] = "error converting ip from string";
const char UTIL_ERROR_BIND[] = "error binding socket to address";
const char UTIL_ERROR_SET_LISTEN_SOCKET[] = "error setting listening mode for socket";
const char UTIL_ERROR_CREATE_SOCKET[] = "error creating socket";

const char UTIL_PARAMETER_IS_NULL[] = "parameter is null";
const char UTIL_ERROR_IP_BUFFER_SIZE[] = "invalid buffer size for writing string representation of ip address";


void* s_malloc(size_t size, error_t *error) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = UTIL_ERROR_ALLOCATED_MEMORY;
        }
        return NULL;
    }

    if (error != NULL) {
        error->error = OK;
        error->message = ERROR_SUCCESS;
    }
    memset(ptr, 0, size);
    return ptr;
}

const char ERROR_TYPE_OK[] = "OK";
const char ERROR_TYPE_NOT_FOUND[] = "NOT_FOUND";
const char ERROR_TYPE_EXISTS[] = "EXISTS";
const char ERROR_TYPE_FATAL[] = "FATAL";
const char ERROR_TYPE_ERROR[] = "ERROR";

const char *errtostr(enum ErrorType type) {
    const char* str = NULL;
    switch(type) {
        case OK:
            str = ERROR_TYPE_OK;
            break;
        case NOT_FOUND:
            str = ERROR_TYPE_NOT_FOUND;
            break;
        case EXISTS:
            str = ERROR_TYPE_EXISTS;
            break;
        case FATAL:
            str = ERROR_TYPE_FATAL;
            break;
        default:
            str = ERROR_TYPE_ERROR;
    }
    return str;
}

int make_server_socket(const char *ip, int port, error_t *error) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    const char *message = ERROR_SUCCESS;
    if (sock == -1) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = UTIL_ERROR_CREATE_SOCKET;
        }
        return ERROR;
    }
    fcntl(sock, O_NONBLOCK);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    int res = inet_aton("127.0.0.1", &addr.sin_addr);
    if (res == -1) {
        message = UTIL_ERROR_CONVERT_IP;
        goto exit_error;
    }
    res = bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (res == -1) {
        message = UTIL_ERROR_BIND;
        goto exit_error;
    }
    res = listen(sock, 1);
    if (res == -1) {
        message = UTIL_ERROR_SET_LISTEN_SOCKET;
        goto exit_error;
    }
    return sock;

exit_error:
    close(sock);
    if (error != NULL) {
        error->error = FATAL;
        error->message = message;
    }
    return ERROR;
}

bool get_addr(struct sockaddr_in* addr, char *ip, size_t buffer_size, uint16_t *port, error_t *error) {
    ERROR_SUCCESS(error);
    if (addr == NULL || ip == NULL || port == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = UTIL_PARAMETER_IS_NULL;
        }
        return false;
    }
    if (buffer_size < IP_BUFFER_LEN) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = UTIL_ERROR_IP_BUFFER_SIZE;
        }
        return false;
    }
    inet_ntop(AF_INET, &addr->sin_addr, ip, buffer_size);
    *port = ntohs(addr->sin_port);
    return true;
}