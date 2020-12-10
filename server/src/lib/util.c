//
// Created by nrx on 08.10.2020.
//


#include "util.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#define ERROR (-1)
#define UTIL_MAX_NUMBER_OF_VARIANT_PARAMETERS 50

const char ERROR_SUCCESS[] = "Success";
const char UTIL_ERROR_ALLOCATED_MEMORY[] = "Error allocated memory";
const char UTIL_ERROR_CONVERT_IP[] = "error converting ip from string";
const char UTIL_ERROR_BIND[] = "error binding socket to address";
const char UTIL_ERROR_SET_LISTEN_SOCKET[] = "error setting listening mode for socket";
const char UTIL_ERROR_CREATE_SOCKET[] = "error creating socket";

const char UTIL_PARAMETER_IS_NULL[] = "parameter is null";
const char UTIL_ERROR_IP_BUFFER_SIZE[] = "invalid buffer size for writing string representation of ip address";


void* s_malloc(size_t size, err_t *error) {
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

int make_server_socket(const char *ip, int port, err_t *error) {
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
    int option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
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

client_addr get_addr(struct sockaddr_in* addr, err_t *error) {
    client_addr ret = {0};
    if (addr == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = UTIL_PARAMETER_IS_NULL;
        }
        return ret;
    }
    ERROR_SUCCESS(error);

    inet_ntop(AF_INET, &addr->sin_addr, ret.ip, IP_BUFFER_LEN);
    ret.port = ntohs(addr->sin_port);
    return ret;
}

#define SPACE_SYMBOLS_CHECK(ptr_, index_) \
((ptr_)[(index_)] == ' ' || (ptr_)[(index_)] == '\n' || (ptr_)[(index_)] == '\r' || (ptr_)[(index_)] == '\t')

// TODO (ageev)  добавить проверки
bool trim_str(const char* src, char **dst, size_t src_size, size_t *dst_size) {
    size_t begin = 0;
    size_t end = src_size-1;
    for(; SPACE_SYMBOLS_CHECK(src, begin) && (begin < src_size); ++begin) {}
    for(; SPACE_SYMBOLS_CHECK(src, end) && (begin >= 0); --end) {}
    end++;
    if (begin < end) {
        size_t new_size = end - begin + 1;
        if (*dst_size < new_size) {
            free(*dst);
            *dst = malloc(sizeof(char)*new_size);
            *dst_size = new_size;
        }
        memcpy(*dst, src + begin, new_size);
        (*dst)[new_size-1] = '\0';
    } else {
        if (*dst_size == 0) {
            free(*dst);
            *dst = malloc(sizeof(char)*1);
            *dst_size = 1;
        }
        *dst[0] = '\0';
    }
    return true;
}

bool sub_str(const char* const src, char * const dst, size_t begin, size_t end) {
    size_t i = begin;
    for (; i < end; ++i) {
        dst[i-begin] = src[i];
    };
    dst[i-begin] = '\0';
    return true;
}


bool char_make_buf_concat(char **buffer, size_t *bsize, size_t nargs, const char *str, ...) {
    size_t result_size = 0;
    size_t size_array[UTIL_MAX_NUMBER_OF_VARIANT_PARAMETERS];
    size_t length_size_array = nargs;

    va_list va;
    va_start(va, str);
    size_array[0] = strlen(str);
    result_size += size_array[0];
    for (size_t i = 1; i < nargs; ++i) {
        const char *ptr = va_arg(va, const char*);
        size_array[i] = strlen(ptr);
        result_size += size_array[i];
    }
    result_size++; // учет последнего нулевого символа в строке
    va_end(va);
    if (*bsize < result_size) {
        free(*buffer);
        *buffer = s_malloc(sizeof(char)*result_size, NULL);
        *bsize = result_size;
    }
    if (*buffer == NULL) {
        *buffer = NULL;
        return false;
    }
    strcpy(*buffer, str);
    va_start(va, str);
    size_t offset = 0;
    for (size_t i = 1; i < nargs; ++i) {
        offset += size_array[i-1];
        const char *ptr = va_arg(va, const char*);
        strcpy(*buffer + offset, ptr);
    }
    va_end(va);
    return true;
}

bool split_str(const char *str, char *array_str[], int num_split, char sep) {
    return split_sub_str(str, 0, UTIL_STR_END, array_str, num_split, sep);
}


bool split_sub_str(const char *str, int begin, int end, char *array_str[], int num_split, char sep) {
    if(str == NULL || array_str == NULL) {
        return false;
    }
    size_t begin_sub = begin;
    size_t end_sub = begin;
    size_t i = 0;
    bool is = false;
    if (end == UTIL_STR_END) {
        is = i < num_split &&  str[end_sub] != '\0';
    } else {
        is = i < num_split &&  str[end_sub] != '\0' && end_sub < end;
    }

    while(is) {
        if(str[end_sub] == sep) {
            array_str[i] = malloc(sizeof(char)*(end_sub-begin_sub+1));
            sub_str(str, array_str[i], begin_sub, end_sub);
            i++;
            begin_sub = end_sub+1;
        }
        end_sub++;

        if (end == UTIL_STR_END) {
            is = i < num_split &&  str[end_sub] != '\0';
        } else {
            is = i < (num_split-1) &&  str[end_sub] != '\0' && end_sub < end;
        }
    }

    if (end == UTIL_STR_END) {
        size_t len = strlen(str + begin_sub)+1;
        array_str[i] = malloc(sizeof(char)*len);
        strcpy(array_str[i], str + begin_sub);
        array_str[i][len-1] = '\0';
    } else {
        size_t len = strlen(str + begin_sub)+1;
        array_str[i] = malloc(sizeof(char)*len);
        sub_str(str,  array_str[i], begin_sub, end);
    }

    return true;
}

int find_first_entry_str(const char *str, const char *sequence, int str_len, int seq_len) {
    if (seq_len > str_len) {
        return -1;
    }
    int seq_i = 0;
    int i = 0;
    for(; i < str_len; i++) {
        if (str[i] == sequence[seq_i]) {
            seq_i++;
            if (seq_i == seq_len) {
                break;
            }
        } else {
            seq_i = 0;
        }
    }
    if (seq_i == seq_len) {
        return i - (seq_len-1);
    }
    return -1;
}

int find_revers_first_entry_str(const char *str, const char *sequence, int str_len, int seq_len) {
    if (seq_len > str_len) {
        return -1;
    }
    int seq_i = seq_len;
    int i = str_len;
    for(; i > 0; i--) {
        if (str[i-1] == sequence[seq_i-1]) {
            seq_i--;
            if (seq_i == 0) {
                break;
            }
        } else {
            seq_i = seq_len;
        }
    }
    if (seq_i == seq_len) {
        return i+seq_len;
    }
    return -1;
}