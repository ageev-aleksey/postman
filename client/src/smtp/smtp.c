#include "config.h"
#include "smtp.h"
#include "util.h"
#include "logs.h"

#define MAX_MX_ADDRS 10

char *receive_line(int socket_d);

int send_line(int socket_d, char *message);

smtp_context* smtp_connect(char *server, char *port, smtp_context *context) {
    LOG_INFO("Попытка соединения с сервером", NULL);

    if (context == NULL) {
        LOG_INFO("Выделяем память под SMTP-контекст", NULL);
        context = allocate_memory(sizeof(*context));
    }

    context->socket_desc = -1;
    context->state_code = SMTP_INVALID;

    struct sockaddr_in server_addr;
    ips ips;

    int server_socket = socket(PF_INET, SOCK_STREAM, 0);


    if (server_socket == -1) {
        LOG_ERROR("Критическая ошибка в выделении памяти под сокет", NULL);
        return NULL;
    }

    set_socket_blocking_enabled(server_socket, false);

    char *mxs[MAX_MX_ADDRS];
    int len = resolvmx(server, mxs, MAX_MX_ADDRS);

    for (int i = 0; i < len; i++) {
        ips = get_ips_by_hostname(mxs[i]);

        server_addr.sin_family = PF_INET;
        memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
        server_addr.sin_port = htons(convert_string_to_long_int(port));

        for (int j = 0; j < ips.ips_size; i++) {

            inet_aton(ips.ip[j], &(server_addr.sin_addr));

            socklen_t socklen = sizeof(struct sockaddr);

            if (connect(server_socket, (struct sockaddr *) &server_addr, socklen) == -1 && errno != EINPROGRESS) {
                LOG_ERROR("Ошибка в открытие сокетного соединения с SMTP-сервером", NULL);
                continue;
            } else {
                context->socket_desc = server_socket;
                context->state_code = SMTP_CONNECT;
                LOG_INFO("Успешное подключение к %s по адресу: %s", server, ips.ip[j]);

                goto exit;
            }
        }
    }

    exit:
    for (int l = 0; l < len; l++) {
        free(mxs[l]);
    }

    for (int l = 0; l < ips.ips_size; l++) {
        free(ips.ip[l]);
    }

    return context;
}

state_code smtp_send_helo(smtp_context *context) {
    char *buffer_send;
    asprintf(&buffer_send, "HELO %s\r\n", config_context.hostname);

    if (send_smtp_request(context, buffer_send) != SMTP_INVALID) {
        context->state_code = SMTP_HELO;

        buffer_send[strlen(buffer_send) - 1] = 0;
        buffer_send[strlen(buffer_send) - 1] = 0;
        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), buffer_send);
    }
    free(buffer_send);
    return context->state_code;
}

state_code smtp_send_ehlo(smtp_context *context) {
    char *buffer_send;
    asprintf(&buffer_send,"EHLO %s\r\n", config_context.hostname);

    if (send_smtp_request(context, buffer_send) != SMTP_INVALID) {
        context->state_code = SMTP_EHLO;

        buffer_send[strlen(buffer_send) - 1] = 0;
        buffer_send[strlen(buffer_send) - 1] = 0;
        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), buffer_send);
    }
    free(buffer_send);
    return context->state_code;
}

state_code smtp_send_mail(smtp_context *context, char *from_email) {
    char *buffer_send;
    asprintf(&buffer_send, "MAIL from:<%s>\r\n", from_email);

    if (send_smtp_request(context, buffer_send) != SMTP_INVALID) {
        context->state_code = SMTP_MAIL;

        buffer_send[strlen(buffer_send) - 1] = 0;
        buffer_send[strlen(buffer_send) - 1] = 0;
        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), buffer_send);
    }

    free(buffer_send);
    return context->state_code;
}

state_code smtp_send_rcpt(smtp_context *context, char *to_email) {
    char *buffer_send;
    asprintf(&buffer_send, "RCPT to:<%s>\r\n", to_email);

    if (send_smtp_request(context, buffer_send) != SMTP_INVALID) {
        context->state_code = SMTP_RCPT;

        buffer_send[strlen(buffer_send) - 1] = 0;
        buffer_send[strlen(buffer_send) - 1] = 0;
        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), buffer_send);
    }

    free(buffer_send);
    return context->state_code;
}

state_code smtp_send_data(smtp_context *context) {
    if (send_smtp_request(context, "DATA\r\n") != SMTP_INVALID) {
        context->state_code = SMTP_DATA;
        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), "DATA");
    }

    return context->state_code;
}

state_code smtp_send_message(smtp_context *context, char *message) {
    if (send_smtp_request(context, message) != SMTP_INVALID) {
        context->state_code = SMTP_MESSAGE;

        char *body_request;
        asprintf(&body_request, "%s", message);

        if (body_request[strlen(body_request) - 1] == '\r' || body_request[strlen(body_request) - 1] == '\n') {
            body_request[strlen(body_request) - 1] = 0;
        }

        if (body_request[strlen(body_request) - 1] == '\r' || body_request[strlen(body_request) - 1] == '\n') {
            body_request[strlen(body_request) - 1] = 0;
        }

        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), body_request);
        free(body_request);
    }

    return context->state_code;
}

state_code smtp_send_end_message(smtp_context *context) {
    if (send_smtp_request(context, "\r\n.\r\n") != SMTP_INVALID) {
        context->state_code = SMTP_END_MESSAGE;
        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), ".");
    }

    return context->state_code;
}

state_code smtp_send_rset(smtp_context *context) {
    if (send_smtp_request(context, "RSET\r\n") != SMTP_INVALID) {
        context->state_code = SMTP_RSET;
        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), "RSET");
    }

    return context->state_code;
}

state_code smtp_send_quit(smtp_context *context) {
    if (send_smtp_request(context, "QUIT\r\n") != SMTP_INVALID) {
        context->state_code = SMTP_QUIT;
        LOG_INFO("Request <%s>: %s", get_addr_by_socket(context->socket_desc), "QUIT");
    }

    return context->state_code;
}

state_code send_smtp_request(smtp_context *smtp_cont, char *str) {
    if (!send_line(smtp_cont->socket_desc, str)) {
        smtp_cont->state_code = SMTP_INVALID;
    }

    return smtp_cont->state_code;
}

smtp_response get_smtp_response(smtp_context *context) {
    smtp_response smtp_response;
    char *buffer = NULL;

    if ((buffer = receive_line(context->socket_desc)) == NULL) {
        LOG_WARN("Не удалось прочитать данные из входного буфера", NULL);
        smtp_response.message = "error";
        smtp_response.status_code = UNDEFINED_ERROR;
        return smtp_response;
    }

    int i = 0;
    char code[3];
    size_t size_str = strlen(buffer);
    for (; i < 3; i++) {
        if (buffer[i] == ' ') {
            break;
        }
        code[i] = buffer[i];
    }

    char *message = allocate_memory(strlen(buffer) - i + 1);
    if (i < size_str - 1) {
        strncpy(message, buffer + i + 1, strlen(buffer) - i);
        message[strlen(buffer) - i] = 0;
        smtp_response.message = message;
    } else {
        free(message);
    }

    smtp_response.status_code = convert_string_to_long_int(code);

    LOG_INFO("Response <%s>: %d %s", get_addr_by_socket(context->socket_desc),
             smtp_response.status_code, smtp_response.message);

    free(buffer);
    return smtp_response;
}

bool is_smtp_success(status_code status_code) {
    if ((status_code - 200) / 100 == 0 || (status_code - 300) / 100 == 0) {
        return true;
    }
    return false;
}

// TODO: отрефакторить (убрать end_chars_count при первой возможности)
char *receive_line(int socket_d) {
    int end_chars_count = 0;

    size_t size = 100;

    char *dist_buffer = allocate_memory(size);
    char *ptr = dist_buffer;
    int count_size = 0;

    int bytes;
    while ((bytes = recv(socket_d, ptr, 1, 0)) > 0) {
        if (bytes <= 0) {
            return NULL;
        }

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

int send_line(int socket_d, char *message) {
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
