#include <string.h>
#include <netdb.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "logs.h"
#include "util.h"

long int convert_string_to_long_int(const char *str) {
    char *end;
    return strtol(str, &end, 10);
}

int check_equation_delim(const char *const str, const char *const delim, int offset) {
    int i = 0;
    while (str[offset] == delim[i]) {
        offset++;
        i++;
    }

    if (i == strlen(delim)) {
        return offset;
    }

    return -1;
}

string_tokens split(const char *const str, const char *const delim) {

    int count_tokens = 0;
    string *tokens = allocate_memory((sizeof(*tokens)));

    string *token = allocate_memory(sizeof(*token));
    token->chars = NULL;
    token->length = 0;

    for (int i = 0; i < strlen(str) + 1; i++) {
        int offset;
        if ((offset = check_equation_delim(str, delim, i)) != -1 || str[i] == '\0') {
            if (offset != -1) {
                i = offset - 1;
            }
            if (token->length != 0) {
                tokens[count_tokens] = *token;
                count_tokens++;
                token = allocate_memory(sizeof(*token));
                token->chars = 0;
                token->length = 0;
                tokens = reallocate_memory(tokens, sizeof(*tokens) * (count_tokens + 1));
                continue;
            }
        }
        add_character(token, str[i]);
    }

    string_tokens string_tokens;
    string_tokens.tokens = tokens;
    string_tokens.count_tokens = count_tokens;

    return string_tokens;
}

void free_string_tokens(string_tokens *tokens) {
    for (int i = 0; i < tokens->count_tokens; i++) {
        free(tokens->tokens[i].chars);
    }
}

string *get_string_from_characters(string *str, char *characters) {
    size_t capacity = str->length + 1;
    char *s;
    if (str->chars == NULL) {
        str->length = 0;
        s = (char *) allocate_memory(sizeof(char));
    }

    for (int i = 0; i < strlen(characters); i++) {
        s[(str->length)++] = characters[i];

        if (str->length >= capacity) {
            capacity *= 2; // увеличиваем ёмкость строки в два раза
            s = (char *) reallocate_memory(s, capacity); // создаём новую строку с увеличенной ёмкостью
        }
    }

    s[str->length] = '\0';

    str->chars = s;

    return str;
}

string *add_character(string *str, char character) {
    size_t capacity = str->length + 1;
    if (str->chars == NULL) {
        str->length = 0;
        str->chars = allocate_memory(sizeof(char));
    }

    str->chars[(str->length)++] = character;

    if (str->length >= capacity) {
        capacity *= 2; // увеличиваем ёмкость строки в два раза
        str->chars = (char *) reallocate_memory(str->chars, capacity); // создаём новую строку с увеличенной ёмкостью
    }

    str->chars[str->length] = '\0';

    return str;
}

void free_string(string *str) {
    free(str->chars);
    free(str);
}

void *allocate_memory(size_t bytes) {
    void *buffer = malloc(bytes);

    if (buffer != NULL) {
        return buffer;
    }

    LOG_ERROR("Ошибка выделения памяти", NULL);
    return NULL;
}

void *reallocate_memory(void *buffer, size_t bytes) {
    buffer = realloc(buffer, bytes);

    if (buffer != NULL) {
        return buffer;
    }

    LOG_ERROR("Ошибка перераспределения памяти", NULL);
    return NULL;
}

void *callocate_memory(size_t size, size_t bytes) {
    void *buffer = calloc(size, bytes);

    if (buffer != NULL) {
        return buffer;
    }

    LOG_ERROR("Ошибка выделения памяти", NULL);
    return NULL;
}

void trim(char *str) {
    {
        // удаляем пробелы и табы с начала строки:
        size_t i = 0, j;
        while ((str[i] == ' ') || (str[i] == '\t')) {
            i++;
        }
        if (i > 0) {
            for (j = 0; j < strlen(str); j++) {
                str[j] = str[j + i];
            }
            str[j] = '\0';
        }

        // удаляем пробелы и табы с конца строки:
        i = strlen(str) - 1;
        while ((str[i] == ' ') || (str[i] == '\t')) {
            i--;
        }
        if (i < (strlen(str) - 1)) {
            str[i + 1] = '\0';
        }
    }
}

char *get_addr_by_socket(int socket) {
    struct sockaddr_in client_addr;
    socklen_t s_len = sizeof(struct sockaddr);
    if (getpeername(socket, (struct sockaddr *) &client_addr, &s_len) == -1) {
        LOG_ERROR("Невозможно получить имя по сокету", NULL);
        return NULL;
    }
    char *addr;
    asprintf(&addr, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    client_addr.sin_addr;

    return addr;
}

ips get_ips_by_hostname(char *hostname) {
    struct hostent *hostent;
    struct in_addr **addr_list;
    ips ips = {0};

    if ((hostent = gethostbyname(hostname)) == NULL) {
        LOG_ERROR("Ошибка в получении адреса по доменному имени", NULL);
        return ips;
    }

    addr_list = (struct in_addr **) hostent->h_addr_list;

    for (int i = 0; addr_list[i] != NULL; i++) {
        ips.ip[i] = strdup(inet_ntoa(*addr_list[i]));
        ips.ips_size++;
    }

    return ips;
}

int resolvmx(const char *name, char **mxs, int limit) {
    unsigned char response[NS_PACKETSZ];  /* big enough, right? */
    ns_msg handle;
    ns_rr rr;
    int mx_index, ns_index, len;
    char dispbuf[4096];

    if ((len = res_search(name, C_IN, T_MX, response, sizeof(response))) < 0) {
        /* WARN: res_search failed */
        return -1;
    }

    if (ns_initparse(response, len, &handle) < 0) {
        /* WARN: ns_initparse failed */
        return 0;
    }

    len = ns_msg_count(handle, ns_s_an);
    if (len < 0)
        return 0;

    for (mx_index = 0, ns_index = 0;
         mx_index < limit && ns_index < len;
         ns_index++) {
        if (ns_parserr(&handle, ns_s_an, ns_index, &rr)) {
            /* WARN: ns_parserr failed */
            continue;
        }
        ns_sprintrr(&handle, &rr, NULL, NULL, dispbuf, sizeof(dispbuf));
        if (ns_rr_class(rr) == ns_c_in && ns_rr_type(rr) == ns_t_mx) {
            char mxname[MAXDNAME];
            dn_expand(ns_msg_base(handle), ns_msg_base(handle) + ns_msg_size(handle), ns_rr_rdata(rr) + NS_INT16SZ,
                      mxname, sizeof(mxname));
            mxs[mx_index++] = strdup(mxname);
        }
    }

    return mx_index;
}

bool set_socket_blocking_enabled(int socket, bool blocking) {
    if (socket < 0) return false;

    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(socket, F_SETFL, flags) == 0) ? true : false;
}