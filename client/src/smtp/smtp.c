#include "config/config.h"
#include "util/util.h"
#include "util/network.h"
#include "log/logs.h"
#include "smtp.h"

#define MAX_MX_ADDRS 30

int server_connect(ips ips, char *port) {
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        LOG_ERROR("Критическая ошибка в выделении памяти под сокет", NULL);
        return -1;
    }

    set_socket_blocking_enabled(server_socket, false);

    struct sockaddr_in server_addr;

    server_addr.sin_family = PF_INET;
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    server_addr.sin_port = htons(convert_string_to_long_int(port));

    for (int j = 0; j < ips.ips_size; j++) {

        inet_aton(ips.ip_array[j], &(server_addr.sin_addr));

        socklen_t socklen = sizeof(struct sockaddr);

        if (connect(server_socket, (struct sockaddr *) &server_addr, socklen) == -1 && errno != EINPROGRESS) {
            LOG_ERROR("Ошибка в открытие сокетного соединения с SMTP-сервером", NULL);
            continue;
        } else {
            return server_socket;
        }
    }

    return -1;
}

smtp_context* smtp_connect(char *server, char *port, smtp_context *context) {
    LOG_INFO("Попытка соединения с сервером %s", server);

    if (context == NULL) {
        LOG_INFO("Выделяем память под SMTP-контекст <%s>", server);
        context = allocate_memory(sizeof(smtp_context));
    }

    context->socket_desc = -1;
    context->state_code = SMTP_INVALID;

    ips ips = { 0 };
    ips.ip_array = NULL;

    // Хак, который позволяет не проверять доменное имя хоста
    if (strcmp(server, config_context.hostname) == 0) {
        ips.ip_array = allocate_memory(sizeof(char*));
        ips.ip_array[0] = "127.0.0.1";
        ips.ips_size++;

        int server_socket;
        if ((server_socket = server_connect(ips, config_context.server_port)) != -1) {
            context->socket_desc = server_socket;
            context->state_code = SMTP_CONNECT;

            LOG_INFO("Успешное подключение к %s по адресу: %s:%s", server, ips.ip_array[0], port);
        }
    } else {
        char *mxs[MAX_MX_ADDRS];
        int len = resolvmx(server, mxs, MAX_MX_ADDRS);

        if (len == -1) {
            LOG_WARN("MX-запись для данного домена %s не найдена", server);
            return context;
        }

        for (int i = 0; i < len; i++) {
            ips = get_ips_by_hostname(mxs[i]);

            int server_socket;
            if ((server_socket = server_connect(ips, port)) != -1) {
                context->socket_desc = server_socket;
                context->state_code = SMTP_CONNECT;

                struct timespec tv = { 0 };
                tv.tv_sec = 1;
                nanosleep(&tv, &tv);

                char *addr = get_addr_by_socket(server_socket);
                LOG_INFO("Успешное подключение к %s по адресу: %s", server, addr);
                free(addr);

                break;
            }

        }

        for (int l = 0; l < len; l++) {
            free(mxs[l]);
        }

        for (int l = 0; l < ips.ips_size; l++) {
            free(ips.ip_array[l]);
        }
    }

    free(ips.ip_array);
    return context;
}

state_code smtp_send_helo(smtp_context *context) {
    char *buffer_send;
    asprintf(&buffer_send, "HELO %s\r\n", config_context.hostname);

    if (send_smtp_request(context, buffer_send) != SMTP_INVALID) {
        context->state_code = SMTP_HELO;

        buffer_send[strlen(buffer_send) - 1] = 0;
        buffer_send[strlen(buffer_send) - 1] = 0;
        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_INFO("Request <%s>: %s", addr, buffer_send);
        free(addr);
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
        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_INFO("Request <%s>: %s", addr, buffer_send);
        free(addr);
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
        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_INFO("Request <%s>: %s", addr, buffer_send);
        free(addr);
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
        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_INFO("Request <%s>: %s", addr, buffer_send);
        free(addr);
    }

    free(buffer_send);
    return context->state_code;
}

state_code smtp_send_data(smtp_context *context) {
    if (send_smtp_request(context, "DATA\r\n") != SMTP_INVALID) {
        context->state_code = SMTP_DATA;
        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_INFO("Request <%s>: %s", addr, "DATA");
        free(addr);
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

        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_DEBUG("Request <%s>: %s", addr, body_request);
        free(addr);
        free(body_request);
    }

    return context->state_code;
}

state_code smtp_send_end_message(smtp_context *context) {
    if (send_smtp_request(context, "\r\n.\r\n") != SMTP_INVALID) {
        context->state_code = SMTP_END_MESSAGE;
        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_INFO("Request <%s>: %s", addr, ".");
        free(addr);
    }

    return context->state_code;
}

state_code smtp_send_rset(smtp_context *context) {
    if (send_smtp_request(context, "RSET\r\n") != SMTP_INVALID) {
        context->state_code = SMTP_CONNECT;
        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_INFO("Request <%s>: %s", addr, "RSET");
        free(addr);
    }

    return context->state_code;
}

state_code smtp_send_quit(smtp_context *context) {
    if (send_smtp_request(context, "QUIT\r\n") != SMTP_INVALID) {
        context->state_code = SMTP_QUIT;
        char *addr = get_addr_by_socket(context->socket_desc);
        LOG_INFO("Request <%s>: %s", addr, "QUIT");
        free(addr);
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
    size_t size_str = 0;

    smtp_response.message = NULL;
    char *addr = get_addr_by_socket(context->socket_desc);
    if ((buffer = receive_line(context->socket_desc)) == NULL) {
        LOG_WARN("Не удалось прочитать данные из входного буфера (%s)", addr);
        smtp_response.status_code = UNDEFINED_ERROR;
        free(addr);
        return smtp_response;
    }

    int i = 0;
    char code[3];
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

    LOG_INFO("Response <%s>: %d %s", addr,
             smtp_response.status_code, smtp_response.message);

    free(addr);
    free(buffer);
    return smtp_response;
}

bool is_smtp_success(status_code status_code) {
    if ((status_code - 200) / 100 == 0 || (status_code - 300) / 100 == 0) {
        return true;
    }
    return false;
}

bool is_smtp_4xx_error(status_code status_code) {
    if ((status_code - 400) / 100 == 0) {
        return true;
    }
    return false;
}

bool is_smtp_5xx_error(status_code status_code) {
    if ((status_code - 500) / 100 == 0) {
        return true;
    }
    return false;
}