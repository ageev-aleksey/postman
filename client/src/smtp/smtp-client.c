#include "smtp-client.h"
#include "util.h"

unsigned int receive_line(int socket_d, char *dist_buffer);
unsigned int send_message(int socket_d, char *message);

state_code smtp_open(char *server, char *port, smtp_message **smtp_messages) {
    smtp_message *smtp_mes_new;

    if ((smtp_mes_new = calloc(1, sizeof(**smtp_messages))) == NULL) {
        return get_smtp_state_code(smtp_mes_new);
    }

    *smtp_messages = smtp_mes_new;
    smtp_mes_new->socket_desc = -1;

    if (smtp_connect(server, port, smtp_mes_new) != 0) {
        return get_smtp_state_code(smtp_mes_new);
    }

    if (smtp_handshake(smtp_mes_new) != OK) {

    }

    return smtp_mes_new->state_code;
}

state_code smtp_handshake(smtp_message *smtp_mes) {
    smtp_mes->state_code = HANDSHAKE;

    smtp_response smtp_response = get_smtp_response(smtp_mes);

    printf("smtp-code: %i, smtp-message: %s\n", smtp_response.status_code, smtp_response.message);
    if (!is_smtp_success(smtp_response.status_code)) {
        return smtp_mes->state_code;
    }

    if (smtp_helo(smtp_mes) != OK) {
        return smtp_mes->state_code;
    }

    return smtp_mes->state_code;
}

state_code smtp_helo(smtp_message *smtp_mes) {
    if (send_smtp_request(smtp_mes, "HELO 192.168.1.1\r\n") == OK) {
        smtp_response response = get_smtp_response(smtp_mes);

        printf("smtp-code: %i, smtp-message: %s\n", response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            return smtp_mes->state_code;
        }
    }
    return smtp_mes->state_code;
}

state_code send_smtp_request(smtp_message *smtp_mes, char *str) {
    if (send_message(smtp_mes->socket_desc, str)) {
        set_smtp_state_code(smtp_mes, OK);
    }

    return smtp_mes->state_code;
}

smtp_response get_smtp_response(smtp_message *smtp_mes) {
    smtp_response smtp_response;
    char *buffer = malloc(5);

    if (receive_line(smtp_mes->socket_desc, buffer) != -128128) {
        smtp_response.message = "error";
        smtp_response.status_code = UNDEFINED_ERROR;
        return smtp_response;
    }

    char code[3];
    size_t i = 0;
    size_t size_str = strlen(buffer);
    for (; i < size_str; i++) {
        if (buffer[i] == ' ') {
            i++;
            break;
        }
        code[i] = buffer[i];
    }
    char *message = malloc(strlen(buffer) - i + 1);
    size_t j = 0;
    for (; i < size_str; j++, i++) {
        message[j] = buffer[i];
    }
    message[j] = '\0';

    smtp_response.status_code = convert_string_to_long_int(code);
    smtp_response.message = message;

    free(buffer);
    return smtp_response;
}

int smtp_connect(char *server, char *port, smtp_message *smtp_mes) {
    struct sockaddr_in server_addr;
    smtp_ip *smtp_ip;
    char *serv_domain;

    int server_socket = socket(PF_INET, SOCK_STREAM, 0);

    set_smtp_state_code(smtp_mes, CONNECT);
    if (server_socket == -1) {
        perror("smtp_connect");
        return -1;
    }

    if ((serv_domain = malloc(sizeof(server))) == NULL) {
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;

    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout)) == -1) {
        perror("ERROR: timeout recv");
        exit(-1);
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

    free(smtp_ip->ip);
    free(smtp_ip);
    free(serv_domain);
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
//
//enum smtp_status_code smtp_initiate_handshake(smtp_message *smtp_mes) {
//
//
//}

state_code get_smtp_state_code(smtp_message *smtp_mes) {
    return smtp_mes->state_code;
}

void set_smtp_state_code(smtp_message *smtp_mes, state_code state_code) {
    smtp_mes->state_code = state_code;
}

bool is_smtp_success(status_code status_code) {
    if ((status_code - 200) / 100 == 0 || (status_code - 300) / 100 == 0) {
        return true;
    }
    return false;
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

unsigned int receive_line(int socket_d, char *dist_buffer) {
    int end_chars_count = 0;
    char *ptr = dist_buffer;
    unsigned int start_size = sizeof(dist_buffer);
    int count_size = 0;
    int bytes;
    while ((bytes = recv(socket_d, ptr, 1, 0)) > 0) {
        count_size++;

        if (*ptr == '\0') {
            return bytes;
        }

        if (*ptr == '\n' || *ptr == '\r') {
            end_chars_count++;
        }
        ptr++;

        if (end_chars_count == 2) {
            *(ptr - 2) = '\0';
            return -128128;
        }

        if (count_size == start_size - 2) {
            start_size += (start_size / 2);
            dist_buffer = realloc(dist_buffer, start_size);
        }
    }
    return bytes;
}

unsigned int send_message(int socket_d, char *message) {
    char *ptr = message;
    size_t all_size = strlen(message);
    while (all_size > 0) {
        int send_size = send(socket_d, message, all_size, 0);
        if (send_size == -1) {
            return 0;
        }
        all_size -= send_size;
        ptr += send_size;
    }
    return 1;
}

