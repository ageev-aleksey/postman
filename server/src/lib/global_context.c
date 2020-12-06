#include "global_context.h"
#include "log/context.h"
#include "event_loop/event_loop.h"
#include <regex.h>
#include <string.h>
#define RE_CONFIG_DOMAIN "([[:alnum:]_-]+\.)*[[:alnum:]_-]+"


bool server_config_init(const char *path) {
    config_t cfg;
    config_init(&cfg);

    if(!config_read_file(&cfg, path))
    {
        LOG_ERROR("%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return false;
    }
    int port = 0;
    if (!config_lookup_int(&cfg, "server.port", &port)) {
        LOG_ERROR("%s [%s]", "not found server.port field in file", path);
        return false;
    }
    if (port > 65535) {
        LOG_ERROR("%s [%d]", "invalid port value", port);
        return false;
    }
    const char *host = NULL;
    if (!config_lookup_string(&cfg, "server.host", &host)) {
        LOG_ERROR("%s [%s]", "not found server.host field in file", path);
        return false;
    }
    // TODO (ageev) добавить проверку на корректный ip
//    regex_t reg_domain;
//    regcomp(&reg_domain, )
    const char *domain = NULL;
    if (!config_lookup_string(&cfg, "server.domain", &domain)) {
        LOG_ERROR("%s [%s]", "not found server.domain field in file", path);
        return false;
    }
    const char *maildir_path = NULL;
    if (!config_lookup_string(&cfg, "server.maildir_path", &maildir_path)) {
        LOG_ERROR("%s [%s]", "not found server.maildir_path field in file", path);
        return false;
    }

    global_config_server.ip = malloc(sizeof(char)*strlen(host));
    global_config_server.self_server_name = malloc(sizeof(char)*strlen(domain));
    strcpy(global_config_server.ip, host);
    strcpy(global_config_server.self_server_name, domain);
    global_config_server.log_file_path = NULL;
    global_config_server.port = port;
    global_config_server.hello_msg_size =
            asprintf(&global_config_server.hello_msg, "220 %s The Postman Server v%d.%d",
                     domain, POSTMAN_VERSION_MAJOR, POSTMAN_VERSION_MINOR);
    TAILQ_INIT(&global_config_server.users);
    err_t  error;
    if (!maildir_init(&global_config_server.md, maildir_path, &error)) {
        LOG_ERROR("maildir: %s", error.message);
        return false;
    }

    LOG_INFO("\nConfig loaded: \n -- host: %s\n -- port: %d\n -- domain: %s\n -- maildir: %s",
             global_config_server.ip,
             global_config_server.port,
             global_config_server.self_server_name,
             maildir_path);
    config_destroy(&cfg);
    return true;
}

void server_config_free() {
    free(global_config_server.self_server_name);
    free(global_config_server.ip);
    free(global_config_server.log_file_path);
    free(global_config_server.hello_msg);

    while (!TAILQ_EMPTY(&global_config_server.users)) {
        user_context_entry  *ptr = TAILQ_FIRST(&global_config_server.users);
        TAILQ_REMOVE(&global_config_server.users, ptr, entries);
        free(ptr);
    }
}

bool user_init(user_context *context, struct sockaddr_in *addr, int sock) {
    if (context != NULL) {
        err_t err;
        smtp_init(&context->smtp, &err);
        if (err.error) {
            LOG_ERROR("smtp_init: %s", err.message);
            return false;
        }
        VECTOR_INIT(char, &context->buffer, err);
        if (err.error) {
            LOG_ERROR("vector_init: %s", err.message);
            return false;
        }
        context->socket = sock;
        context->addr = get_addr(addr, &err);
        if (err.error) {
            LOG_ERROR("get_addr: %s", err.message);
            return false;
        }
        return true;
    } else {
        return false;
    }
}

void user_free(user_context *context)  {
    if (context != NULL) {
        VECTOR_FREE(&context->buffer);
        smtp_free(&context->smtp);
    }
}

struct user_context_entry* users_find(users_list *users, int sock) {
    user_context_entry *ptr = NULL;
    TAILQ_FOREACH(ptr, users, entries) {
        if (ptr->context.socket == sock) {
            return ptr;
        }
    }
    return NULL;
}

void users_delete(users_list *users, int sock) {
    user_context_entry *ptr = users_find(users, sock);
    TAILQ_REMOVE(users, ptr, entries);
    user_free(&ptr->context);
    free(ptr);
}

void users_delete_by_ptr(users_list *users, user_context_entry *ptr) {
    TAILQ_REMOVE(users, ptr, entries);
    user_free(&ptr->context);
    free(ptr);
}

void handler_hello_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error);

void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error) {
    user_context_entry *user = malloc(sizeof(user_context_entry));
    if (user == NULL) {
        LOG_ERROR("%s", "Error alloc memory");
        exit(-1);
    }
    if (!user_init(&user->context, &client_addr, client_socket)) {
        return;
    }
    TAILQ_INSERT_TAIL(&global_config_server.users, user, entries);
    LOG_INFO("user connect [%s:%d]", user->context.addr.ip, user->context.addr.port);
    err_t err;
    el_async_write(el, client_socket,
                   global_config_server.hello_msg, global_config_server.hello_msg_size,
                   handler_hello_write, &err);
    if (err.error) {
        LOG_ERROR("el_async_write: %s", err.message);
    }
}
void handler_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error) {
    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }
    

    err_t err;
    free(buffer);
    el_async_read(el, client_socket,
                   global_config_server.hello_msg, global_config_server.hello_msg_size,
                   handler_read, &err);
    if (err.error) {
        LOG_ERROR("el_async_read: %s", err.message);
    }
}

void user_disconnected(int sock) {
    user_context_entry *ptr = users_find(&global_config_server.users, sock);
    LOG_INFO("user close connection [%s:%d]", ptr->context.addr.ip, ptr->context.addr.port);
    users_delete_by_ptr(&global_config_server.users, ptr);
}

void handler_hello_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error) {
    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }


    err_t err;
    el_async_read(el, client_socket,
                  global_config_server.hello_msg, global_config_server.hello_msg_size,
                  handler_read, &err);
    if (err.error) {
        LOG_ERROR("el_async_read: %s", err.message);
    }
}


void handler_read(event_loop *el, int client_socket, char *buffer, int size, client_status status, err_t error) {
    if (status == DISCONNECTED) {
        user_disconnected(client_socket);
        return;
    }

    err_t err;
    el_async_write(el, client_socket,
                  global_config_server.hello_msg, global_config_server.hello_msg_size,
                  handler_write, &err);
    if (err.error) {
        LOG_ERROR("el_async_read: %s", err.message);
    }
}

void handler_timer(event_loop* el, int socket, struct timer_event_entry *descriptor) {
    client_addr client = {0};
    LOG_INFO("timer for client: %s:%d", client.ip, client.port);
}

