#include <string.h>
#include "log/logs.h"
#include "network.h"
#include "util.h"

char *receive_line(int socket_d) {
    int end_chars_count = 0;

    size_t size = 4;

    char *dist_buffer = allocate_memory(sizeof(char) * size);
    memset(dist_buffer, 0, size);
    char *ptr = dist_buffer;
    int count_size = 0;

    int bytes;
    while ((bytes = recv(socket_d, ptr, 1, 0)) > 0) {
        count_size++;

        if (*ptr == '\0') {
            free(dist_buffer);
            return NULL;
        }

        if (*ptr == '\n' || *ptr == '\r') {
            end_chars_count++;
        }
        ptr++;

        if (end_chars_count == 2) {
            *(ptr - 2) = '\0';
            return dist_buffer;
        }

        if (count_size == size - 2) {
            size_t prev_size = size;
            size += (size / 2);
            dist_buffer = reallocate_memory(dist_buffer, sizeof(char) * prev_size, sizeof(char) * size);
            ptr = dist_buffer + count_size;
        }
    }

    if (bytes <= 0) {
        free(dist_buffer);
        return NULL;
    }

    return dist_buffer;
}

int send_line(int socket_d, char *message) {
    char *ptr = message;
    size_t all_size = strlen(message);
    while (all_size > 0) {
        int send_size = send(socket_d, message, all_size, MSG_NOSIGNAL);
        if (send_size == -1) {
            return 0;
        }
        all_size -= send_size;
        ptr += send_size;
    }
    return 1;
}

char *get_addr_by_socket(int socket) {
    struct sockaddr_in client_addr = { 0 };
    socklen_t s_len = sizeof(struct sockaddr);
    if (getpeername(socket, (struct sockaddr *) &client_addr, &s_len) == -1) {
        LOG_ERROR("Невозможно получить имя по сокету", NULL);
        return NULL;
    }
    char *addr;
    asprintf(&addr, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

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

    ips.ip_array = allocate_memory(sizeof(char*));
    for (int i = 0; addr_list[i] != NULL; i++) {
        ips.ip_array = reallocate_memory(ips.ip_array, sizeof(char*) * ips.ips_size,
                                         sizeof(char*) * (ips.ips_size + 1));
        ips.ip_array[i] = strdup(inet_ntoa(*addr_list[i]));
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

    if ((len = res_search(name, C_IN, T_MX, response, NS_PACKETSZ + 1)) < 0) {
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

