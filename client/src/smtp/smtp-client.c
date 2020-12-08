#include "smtp-client.h"
#include "util.h"

enum smtp_status_code smtp_open(const char *server, const char *port, smtp_message **smtp_messages) {
    smtp_message *smtp_mes_new;

    if ((smtp_mes_new = calloc(1, sizeof(**smtp_messages))) == NULL) {
        *smtp_messages = &smtp_error_memory;
        return get_smtp_status_code(smtp_mes_new);
    }

    *smtp_messages = smtp_mes_new;
    smtp_mes_new->socket_desc = -1;

    if (smtp_connect(server, port, smtp_mes_new) < 0) {
        set_smtp_status_code(smtp_mes_new, SMTP_STATUS_CONNECT);
        return get_smtp_status_code(smtp_mes_new);
    }

//    if (smtp_initiate_handshake(server, smtp_mes_new) != SMTP_STATUS_OK) {
//        set_smtp_status_code(smtp_mes_new, SMTP_STATUS_HANDSHAKE);
//    }

    return smtp_mes_new->status_code;
}

int smtp_connect(const char *server, const char *port, smtp_message *smtp_mes) {
    struct sockaddr_in server_addr;
    smtp_ip *smtp_ip;
    char *serv_domain;

    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    if (server_socket == -1) {
        perror("smtp_connect");
        return -1;
    }

    if ((serv_domain = malloc(sizeof(server))) == NULL) {
        return -1;
    }
    strcpy(serv_domain, server);
    smtp_ip = get_ip_by_hostname(serv_domain);

    server_addr.sin_family = PF_INET;
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    server_addr.sin_port = htons(convert_string_to_long_int(port));

    for (int i = 0; i < smtp_ip->num_ips; i++) {

        inet_aton(smtp_ip[i].ip, &(server_addr.sin_addr));

        socklen_t socklen = sizeof(struct sockaddr);

        if (connect(server_socket, (struct sockaddr*) &server_addr, socklen) == -1 && errno != EINPROGRESS) {
            perror("smtp_connect");
            continue;
        } else {
            smtp_mes->socket_desc = server_socket;
            return 0;
        }
    }

    return -1;
}

smtp_ip* get_ip_by_hostname(char *hostname)
{
    struct hostent *hostent;
    struct in_addr **addr_list;
    smtp_ip *smtp_ip;
    int count_ip;

    if ((hostent = gethostbyname(hostname)) == NULL)
    {
        herror("ip_by_hostname");
        return NULL;
    }

    addr_list = (struct in_addr **) hostent->h_addr_list;
    count_ip = sizeof(addr_list);

    smtp_ip = calloc(count_ip, sizeof(*smtp_ip));
    smtp_ip->num_ips = count_ip;

    for (int i = 0; addr_list[i] != NULL; i++) {
        strcpy(smtp_ip[i].ip, inet_ntoa(*addr_list[i]));
    }

    return smtp_ip;
}

//enum smtp_status_code smtp_initiate_handshake(smtp_message *smtp_mes) {
//
//    set_smtp_read_timeout(smtp_mes, 60 * 5);
//    if (smtp_getline(smtp_mes) == STRING_GETDELIMFD_ERROR) {
//        return smtp_mes->status_code;
//    }
//
//    if (smtp_ehlo(smtp_mes) != SMTP_STATUS_OK) {
//        return smtp_mes->status_code;
//    }
//}

enum smtp_status_code get_smtp_status_code(const smtp_message *smtp_mes) {
    return smtp_mes->status_code;
}

void set_smtp_status_code(smtp_message *smtp_mes, enum smtp_status_code status_code) {
    smtp_mes->status_code = status_code;
}

void set_smtp_read_timeout(smtp_message *smtp_mes, long seconds) {
    smtp_mes->timeout_sec = seconds;
}

//void smpt_connect() {
//    fd_set master;
//    fd_set read_fds;
//    fd_set write_fds;
//    int fdmax = 0;
//
//    FD_ZERO(&master);
//    FD_ZERO(&read_fds);
//    FD_ZERO(&write_fds);
//
//    printf("CLIENT START\n");
//
//    char *buffer_receive_message = malloc(1000);
//    char *servers_data = malloc(100);
//    memset(servers_data, 0, sizeof *servers_data);
//
//    char **server_configs = read_ips();
//
//    for (int i = 0; i < 3; i++) {
//        char *ip = strtok(server_configs[i], ":");
//        char *port = strtok(NULL, ":");
//        char *data = strtok(NULL, ":");
//
//        int server_socket = socket(PF_INET, SOCK_STREAM, 0);
//        fcntl(server_socket, F_SETFL, O_NONBLOCK);
//
//        if (server_socket == -1) {
//            perror("ERROR: server socket");
//            break;
//        }
//
//        struct sockaddr_in server_addr;
//        server_addr.sin_family = PF_INET;
//        inet_aton(ip, &(server_addr.sin_addr));
//        server_addr.sin_port = htons(atoi(port));
//        memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
//
//        socklen_t socklen = sizeof(struct sockaddr);
//
//        if (connect(server_socket, (struct sockaddr *) &server_addr, socklen) == -1 && errno != EINPROGRESS) {
//            perror("ERROR: connect");
//            break;
//        } else {
//            printf("server socket for %s:%s - %i\n", ip, port, server_socket);
//        }
//
//        FD_SET(server_socket, &master);
//
//        servers_data[server_socket] = data;
//
//        if (server_socket > fdmax) {
//            fdmax = server_socket;
//        }
//    }
//
//
//    read_fds = master;
//    write_fds = master;
//
//    while (TRUE) {
//
//        for (int fd = 0; fd <= fdmax; fd++) {
//            if (FD_ISSET(fd, &read_fds)) {
//                memset(buffer_receive_message, 0, sizeof *buffer_receive_message);
//                receive_line(fd, buffer_receive_message);
//
//                printf("Output socket #%i: %s\n", fd, buffer_receive_message);
//            }
//            if (FD_ISSET(fd, &write_fds)) {
//                send_message(fd, &servers_data[fd]);
//            }
//        }
//    }
//}
//
//char **read_ips() {
//    char *datafile = "/home/ubuntu/pvs/pvs-postman/client/resources/servers_ip.config";
//    int fd = open(datafile, O_RDONLY, S_IRUSR|S_IWUSR);
//    if (fd == -1) {
//        perror("ERROR: open file");
//        return NULL;
//    }
//
//    char **server_configs = malloc(10);
//    for (int i = 0; i < 3; i++) {
//        char *server_config = malloc(100);
//        server_configs[i] = server_config;
//
//        char byte;
//        int count = 0;
//        while (read(fd, &byte, 1) > 0) {
//            if (byte == '\n') {
//                break;
//            }
//            *(server_config + count) = byte;
//            count++;
//        }
//    }
//
//    return server_configs;
//}
//
//unsigned int receive_line(int socket_d, char *dist_buffer) {
//    char *ptr = dist_buffer;
//    unsigned int start_size = sizeof(dist_buffer);
//    memset(dist_buffer, 0, start_size);
//    unsigned int count_bytes = 0;
//
//    while (recv(socket_d, ptr, 1, 0) != -1) {
//        count_bytes++;
//        ptr++;
//    }
//
//    return strlen(dist_buffer);
//}
//
//unsigned int send_message(int socket_d, char *message) {
//    char *ptr = message;
//    size_t all_size = strlen(message);
//    while (all_size > 0) {
//        int send_size = send(socket_d, message, all_size, 0);
//        if (send_size == -1) {
//            return 0;
//        }
//        all_size -= send_size;
//        ptr += send_size;
//    }
//    return 1;
//}

