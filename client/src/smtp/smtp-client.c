#include "smtp-client.h"
#include "util.h"
#include "logs.h"

char* receive_line(int socket_d);
unsigned int send_message(int socket_d, char *message);
void free_resource_smtp(smtp_context *smtp_cont);
void init_smtp_headers(smtp_context *smtp_cont);

smtp_context* smtp_open(char *server, char *port, smtp_context **smtp_contexts) {
    LOG_INFO("Попытка соединения с сервером", NULL);

    smtp_context *smtp_cont_new;

    if ((smtp_cont_new = calloc(1, sizeof(**smtp_contexts) + 100)) == NULL) {
        LOG_ERROR("Ошибка выделения памяти для контекста SMTP", NULL);
        return NULL;
    }

    *smtp_contexts = smtp_cont_new;
    smtp_cont_new->socket_desc = -1;
    asprintf(&smtp_cont_new->ip_or_domain, "%s", server);
    asprintf(&smtp_cont_new->port, "%s", port);
    smtp_cont_new->hostname = allocate_memory(256);
    gethostname(smtp_cont_new->hostname, 256);

    if (smtp_connect(server, port, smtp_cont_new) != 0) {
        LOG_WARN("Не удалось подключиться к серверу", NULL);
        return NULL;
    }

    if (smtp_handshake(smtp_cont_new) != OK) {
        LOG_WARN("Не удалось установить обмен сообщениями с сервером", NULL);
        return NULL;
    }

    init_smtp_headers(smtp_cont_new);
    smtp_cont_new->to_size = 0;
    smtp_cont_new->to = NULL;

    return smtp_cont_new;
}

state_code smtp_handshake(smtp_context *smtp_cont) {
    smtp_cont->state_code = HANDSHAKE;

    smtp_response smtp_response = get_smtp_response(smtp_cont);

    LOG_INFO("Response <CONNECT>: %d, %s", smtp_response.status_code, smtp_response.message);
    if (!is_smtp_success(smtp_response.status_code)) {
        return smtp_cont->state_code;
    }

    if (smtp_helo(smtp_cont) != OK) {
        return smtp_cont->state_code;
    }

    return smtp_cont->state_code;
}

state_code smtp_helo(smtp_context *smtp_cont) {
    char *buffer_send;
    asprintf(&buffer_send, "HELO %s\r\n", smtp_cont->hostname);

    if (send_smtp_request(smtp_cont, buffer_send) == OK) {
        smtp_response response = get_smtp_response(smtp_cont);

        buffer_send[strlen(buffer_send) - 2] = 0;

        LOG_INFO("Response <%s>: %d, %s", buffer_send, response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            free(buffer_send);
            return smtp_cont->state_code;
        }
    }
    free(buffer_send);
    return smtp_cont->state_code;
}

state_code smtp_ehlo(smtp_context *smtp_cont) {
    char *buffer_send;
    asprintf(&buffer_send,"EHLO %s\r\n", smtp_cont->ip_or_domain);

    if (send_smtp_request(smtp_cont, buffer_send) == OK) {
        smtp_response response = get_smtp_response(smtp_cont);

        buffer_send[strlen(buffer_send) - 2] = 0;

        LOG_INFO("Response <%s>: %d, %s", buffer_send, response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            free(buffer_send);
            return smtp_cont->state_code;
        }
    }
    free(buffer_send);
    return smtp_cont->state_code;
}

state_code smtp_mail(smtp_context *smtp_cont, char *from_email, char *from_name) {
    asprintf(&smtp_cont->from.email, "%s", from_email);
    asprintf(&smtp_cont->from.name, "%s", from_name);
    asprintf(&smtp_cont->header_list->value, "%s", from_email);

    char *buffer_send;
    asprintf(&buffer_send, "MAIL from:<%s>\r\n", smtp_cont->from.email);

    if (send_smtp_request(smtp_cont, buffer_send) == OK) {
        smtp_response response = get_smtp_response(smtp_cont);

        buffer_send[strlen(buffer_send) - 2] = 0;
        LOG_INFO("Response <%s>: %d, %s", buffer_send, response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            free(buffer_send);
            return smtp_cont->state_code;
        }
    }

    free(buffer_send);
    return smtp_cont->state_code;
}

state_code smtp_rcpt(smtp_context *smtp_cont, char *to_email, char *name) {
    smtp_addr to_addr;
    asprintf(&to_addr.email, "%s", to_email);
    asprintf(&to_addr.name, "%s", name);
    smtp_cont->to_size++;
    smtp_cont->to = reallocate_memory(smtp_cont->to, smtp_cont->to_size);
    smtp_cont->to[smtp_cont->to_size - 1] = to_addr;

    char *buffer_send;
    asprintf(&buffer_send, "RCPT to:<%s>\r\n", smtp_cont->to->email);

    if (send_smtp_request(smtp_cont, buffer_send) == OK) {
        smtp_response response = get_smtp_response(smtp_cont);

        buffer_send[strlen(buffer_send) - 2] = 0;
        LOG_INFO("Response <%s>: %d, %s", buffer_send, response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            free(buffer_send);
            return smtp_cont->state_code;
        }
    }
    return smtp_cont->state_code;
}

state_code init_to_header(smtp_context *smtp_cont) {
    char *to_addrs = NULL;
    for (int i = 0; i < smtp_cont->to_size; i++) {
        if (to_addrs == NULL) {
            to_addrs = allocate_memory(strlen(smtp_cont->to->email) + 1);
            strcpy(to_addrs, smtp_cont->to->email);
        } else {
            to_addrs = reallocate_memory(to_addrs, strlen(to_addrs) + strlen(smtp_cont->to->email) + 2);
            strcat(to_addrs, ",");
            strcat(to_addrs, smtp_cont->to->email);
        }
    }

    smtp_cont->header_list[SMTP_TO].value = to_addrs;

    return smtp_cont->state_code;
}

state_code smtp_data(smtp_context *smtp_cont, char *message) {
    init_to_header(smtp_cont);

    if (send_smtp_request(smtp_cont, "DATA\r\n") == OK) {
        smtp_response response = get_smtp_response(smtp_cont);

        LOG_INFO("Response <%s>: %d, %s", "DATA", response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            return smtp_cont->state_code;
        }
    }

    char *result_message = NULL;
    for (int i = 0; i < smtp_cont->num_headers; i++) {
        if (smtp_cont->header_list && smtp_cont->header_list[i].value != NULL) {
            if (result_message == NULL) {
                result_message = allocate_memory(strlen(smtp_cont->header_list[i].key)
                        + strlen(smtp_cont->header_list[i].value) + 10);
                strcpy(result_message, smtp_cont->header_list[i].key);
                strcat(result_message, ": ");
                strcat(result_message, smtp_cont->header_list[i].value);
                strcat(result_message, "\r\n");
            } else {
                result_message = reallocate_memory(result_message, strlen(result_message) + strlen(smtp_cont->header_list[i].key)
                                        + strlen(smtp_cont->header_list[i].value) + 10);
                strcat(result_message, smtp_cont->header_list[i].key);
                strcat(result_message, ": ");
                strcat(result_message, smtp_cont->header_list[i].value);
                strcat(result_message, "\r\n");
            }
        }
    }

    if (result_message == NULL) {
        result_message = allocate_memory(strlen(result_message) + 1);
        strcpy(result_message, "\r\n");
        strcat(result_message, message);
    } else {
        result_message = reallocate_memory(result_message,strlen(result_message) + strlen(message) + 3);
        strcat(result_message, "\r\n");
        strcat(result_message, message);
    }

    if (send_smtp_request(smtp_cont, result_message) == OK) {
        smtp_response response = get_smtp_response(smtp_cont);

        LOG_INFO("Response <send message from %s to %s>: %d, %s", smtp_cont->from.email, smtp_cont->to->email,
                 response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            free(result_message);
            return smtp_cont->state_code;
        }
    }

    free(result_message);
    return smtp_cont->state_code;
}

state_code smtp_rset(smtp_context *smtp_cont) {

    if (send_smtp_request(smtp_cont, "RSET\r\n") == OK) {
        smtp_response response = get_smtp_response(smtp_cont);

        LOG_INFO("Response <RSET>: %d, %s", response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            return smtp_cont->state_code;
        }
    }

    return smtp_cont->state_code;
}

state_code smtp_quit(smtp_context *smtp_cont) {

    if (send_smtp_request(smtp_cont, "QUIT\r\n") == OK) {
        smtp_response response = get_smtp_response(smtp_cont);

        LOG_INFO("Response <QUIT>: %d, %s", response.status_code, response.message);
        if (!is_smtp_success(response.status_code)) {
            free_resource_smtp(smtp_cont);
            return smtp_cont->state_code;
        }
    }

    free_resource_smtp(smtp_cont);
    return smtp_cont->state_code;
}

state_code send_smtp_request(smtp_context *smtp_cont, char *str) {
    if (send_message(smtp_cont->socket_desc, str)) {
        set_smtp_state_code(smtp_cont, OK);
    }

    return smtp_cont->state_code;
}

smtp_response get_smtp_response(smtp_context *smtp_cont) {
    smtp_response smtp_response;
    char *buffer = NULL;

    if ((buffer = receive_line(smtp_cont->socket_desc)) == NULL) {
        LOG_WARN("Не удалось прочитать данные из входного буфера", NULL);
        smtp_response.message = "error";
        smtp_response.status_code = UNDEFINED_ERROR;
        return smtp_response;
    }

    char code[3];
    size_t i = 0;
    size_t size_str = strlen(buffer);
    char *message = buffer;
    for (; i < size_str; i++) {
        if (buffer[i] == ' ') {
            message++;
            break;
        }
        code[i] = buffer[i];
        message++;
    }

    smtp_response.status_code = convert_string_to_long_int(code);
    smtp_response.message = message;

    return smtp_response;
}

void init_smtp_headers(smtp_context *smtp_cont) {
    smtp_header *smtp_headers = allocate_memory(sizeof(*smtp_headers) * 23);
    smtp_headers[SMTP_DATE].key = "Date";
    smtp_headers[SMTP_FROM].key = "From";
    smtp_headers[SMTP_SENDER].key = "Sender";
    smtp_headers[SMTP_REPLY_TO].key = "Reply-to";
    smtp_headers[SMTP_TO].key = "To";
    smtp_headers[SMTP_CC].key = "Cc";
    smtp_headers[SMTP_BCC].key = "Bcc";
    smtp_headers[SMTP_MESSAGE_ID].key = "Message-id";
    smtp_headers[SMTP_IN_REPLY_TO].key = "In-Reply-To";
    smtp_headers[SMTP_REFERENCES].key = "References";
    smtp_headers[SMTP_SUBJECT].key = "Subject";
    smtp_headers[SMTP_COMMENTS].key = "Comments";
    smtp_headers[SMTP_KEYWORDS].key = "Keywords";
    smtp_headers[SMTP_RESENT_DATE].key = "Resent-Date";
    smtp_headers[SMTP_RESENT_FROM].key = "Resent-From";
    smtp_headers[SMTP_RESENT_SENDER].key = "Resent-Sender";
    smtp_headers[SMTP_RESENT_TO].key = "Resent-To";
    smtp_headers[SMTP_RESENT_CC].key = "Resent-Cc";
    smtp_headers[SMTP_RESENT_BCC].key = "Resent-Bcc";
    smtp_headers[SMTP_RESENT_MESSAGE_ID].key = "Resent-Message-Id";
    smtp_headers[SMTP_RETURN_PATH].key = "Return-Path";
    smtp_headers[SMTP_RECEIVED].key = "Received";
    smtp_headers[SMTP_CONTENT_TYPE].key = "Content-Type";

    smtp_cont->header_list = smtp_headers;
    smtp_cont->num_headers = 23;

    for (int i = 0; i < smtp_cont->num_headers; i++) {
        smtp_cont->header_list[i].value = NULL;
    }
}

int smtp_connect(char *server, char *port, smtp_context *smtp_cont) {
    struct sockaddr_in server_addr;
    smtp_ip *smtp_ip;
    char *serv_domain;

    int server_socket = socket(PF_INET, SOCK_STREAM, 0);

    set_smtp_state_code(smtp_cont, CONNECT);
    if (server_socket == -1) {
        LOG_ERROR("Критическая ошибка в выделении памяти под сокет", NULL);
        return -1;
    }

    if ((serv_domain = allocate_memory(strlen(server) + 1)) == NULL) {
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 30;
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
            LOG_ERROR("Ошибка в открытие сокетного соединения с SMTP-сервером", NULL);
            continue;
        } else {
            smtp_cont->socket_desc = server_socket;
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
//enum smtp_status_code smtp_initiate_handshake(smtp_context *smtp_cont) {
//
//
//}

state_code get_smtp_state_code(smtp_context *smtp_cont) {
    return smtp_cont->state_code;
}

void set_smtp_state_code(smtp_context *smtp_cont, state_code state_code) {
    smtp_cont->state_code = state_code;
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
//    char *buffer_receive_message = allocate_memory(1000);
//    char *servers_data = allocate_memory(100);
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
//    char **server_configs = allocate_memory(10);
//    for (int i = 0; i < 3; i++) {
//        char *server_config = allocate_memory(100);
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

char* receive_line(int socket_d) {
    int end_chars_count = 0;

    size_t size = 100;

    char *dist_buffer = allocate_memory(100);
    char *ptr = dist_buffer;
    int count_size = 0;

    while ((recv(socket_d, ptr, 1, 0)) > 0) {
        count_size++;

        if (*ptr == '\0') {
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
            size += (size / 2);
            dist_buffer = reallocate_memory(dist_buffer, size);
            ptr = dist_buffer + count_size;
        }
    }
    return dist_buffer;
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

void free_resource_smtp(smtp_context *smtp_context) {
    shutdown(smtp_context->socket_desc, SHUT_RDWR);
    close(smtp_context->socket_desc);
    free(smtp_context->hostname);
    free(smtp_context->ip_or_domain);
    free(smtp_context->port);
    free(smtp_context->to->email);
    free(smtp_context->to->name);
    free(smtp_context->to);
    for (int i = 0; i < smtp_context->num_headers; i++) {
        free(smtp_context->header_list[i].value);
    }
    free(smtp_context->header_list);
    free(smtp_context);
}