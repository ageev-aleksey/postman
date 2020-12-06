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
    users_list__init(&global_config_server.users);
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
    users_list__free(&global_config_server.users);
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

bool users_list__init(users_list *users) {
    TAILQ_INIT(&users->pr_list);
    pthread_mutex_init(&users->pr_mutex, NULL);
    return true;
}

void users_list__free(users_list *users) {
    if (users != NULL) {
        pthread_mutex_destroy(&users->pr_mutex);
        while (!TAILQ_EMPTY(&users->pr_list)) {
            struct user_context_entry *ptr = TAILQ_FIRST(&users->pr_list);
            TAILQ_REMOVE(&users->pr_list, ptr, pr_entries);
            user_free(ptr->pr_context);
            free(ptr->pr_context);
            free(ptr);
        }
    }
}

bool users_list__add(users_list *users, user_context **user) {
    user_context_entry *entry = malloc(sizeof(user_context_entry));
    entry->pr_context = *user;
    pthread_mutex_lock(&users->pr_mutex);
    TAILQ_INSERT_TAIL(&users->pr_list, entry, pr_entries);
    pthread_mutex_unlock(&users->pr_mutex);
    *user = NULL;
}

bool users_list__user_find_by_sock(users_list *users, user_accessor *accessor, int sock) {
    user_context_entry *ptr = NULL;
    bool is_found = false;
    pthread_mutex_lock(&users->pr_mutex);
    TAILQ_FOREACH(ptr, &users->pr_list, pr_entries) {
        if (ptr->pr_context->socket == sock) {
            TAILQ_REMOVE(&users->pr_list, ptr, pr_entries);
            is_found = true;
            break;
        }
    }
    pthread_mutex_unlock(&users->pr_mutex);

    if (is_found) {
        accessor->user = ptr->pr_context;
        accessor->pr_list_entry = ptr;
        accessor->pr_users_list = users;
    }

    return is_found;
}

void user_accessor_release(user_accessor *accessor) {
    if (accessor != NULL && accessor->user != NULL &&
        accessor->pr_users_list != NULL && accessor->pr_list_entry != NULL)
    {
        pthread_mutex_lock(&accessor->pr_users_list->pr_mutex);
        TAILQ_INSERT_TAIL(&accessor->pr_users_list->pr_list, accessor->pr_list_entry, pr_entries);
        pthread_mutex_unlock(&accessor->pr_users_list->pr_mutex);
        accessor->pr_list_entry = NULL;
        accessor->pr_users_list = NULL;
        accessor->user = NULL;
    }

}

void users_list__delete_user(user_accessor *accessor) {
    free(accessor->pr_list_entry);
    accessor->pr_users_list = NULL;
    accessor->pr_list_entry = NULL;
}



void handler_hello_write(event_loop *el, int client_socket, char* buffer, int size, int writing, client_status status, err_t error);

void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error) {
    user_context *context = malloc(sizeof(user_context));
    if (context == NULL) {
        LOG_ERROR("%s", "Error alloc memory");
        exit(-1);
    }
    if (!user_init(context, &client_addr, client_socket)) {
        return;
    }
    LOG_INFO("user connect [%s:%d]", context->addr.ip, context->addr.port);
    users_list__add(&global_config_server.users, &context);

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
    user_accessor acc;
    if (users_list__user_find_by_sock(&global_config_server.users, &acc, sock)) {
        LOG_INFO("user close connection [%s:%d]", acc.user->addr.ip, acc.user->addr.port);
        users_list__delete_user(&acc);
        user_free(acc.user);
    } else {
        LOG_ERROR("user bys socket [%d] not found", sock);
    }
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
    user_accessor acc = users_list__user_find_by_sock(&global_config_server.users, client_socket);
    if (acc.user != NULL) {

        //записываем данные в буфер
        err_t err;
        for(int j = 0; buffer[j] != '\0'; j++) {
            VECTOR_PUSH_BACK(char, &acc.user->buffer, buffer[j], err);
            if (err.error) {
                LOG_ERROR("vector push back: %s", err.message);
            }
        }

        user_accessor_release(&acc);
        el_async_write(el, client_socket,
                       global_config_server.hello_msg, global_config_server.hello_msg_size,
                       handler_write, &err);
        if (err.error) {
            LOG_ERROR("el_async_read: %s", err.message);
        }
    } else {
        LOG_ERROR("not found user by socket [%d]", client_socket);
    }
}

void handler_timer(event_loop* el, int socket, struct timer_event_entry *descriptor) {
    client_addr client = {0};
    LOG_INFO("timer for client: %s:%d", client.ip, client.port);
}

