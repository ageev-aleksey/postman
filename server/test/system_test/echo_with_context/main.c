//
// Created by nrx on 23.10.2020.
//

#include "util.h"
#include "event_loop/event_loop.h"
#include "vector.h"
#include <sys/queue.h>
#include <search.h>

#include <stdio.h>
#include <string.h>

#define ERROR (-1)
#define OK 0
#define BUFFER_READ 5

VECTOR_DECLARE(vector_char, char);
typedef struct d_client_context {
    int socket;
    vector_char *data;
} client_context;

VECTOR_DECLARE(vector_context, client_context);

vector_context *contexts;
size_t clients_connected;


char *make_buffer_from_vector_char(vector_char *vec) {
    size_t size = VECTOR_SIZE(vec);
    char *buffer = malloc(size + 1);
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = VECTOR(vec)[i];
    }
    buffer[size] = '\0';
    return buffer;
}

int find_client(vector_context *con, int socket) {
    size_t size = VECTOR_SIZE(con);
    for (int i = 0; i < size; ++i) {
        if (VECTOR(con)[i].socket == socket) {
            return i;
        }
    }
    return -1;
}
void read_handler(event_loop *loop, int socket, char *buffer, int size, client_status status, error_t error);
void write_handler(event_loop *loop, int socket, char* buffer, int size, int writing, client_status status,  error_t error) {
    if (error.error) {
        //printf("%s\n", error.message);
        perror(error.message);
        return;
    }
    free(buffer);

    if (status == DISCONNECTED) {
        printf("Client Disconnected - write handler\n");
        clients_connected--;
        if (clients_connected == 0) {
            el_stop(loop, NULL);
        }
        // TODO
        // TODO (ageev) remove context
        return;
    } else {
        buffer = s_malloc(BUFFER_READ, NULL);
        error_t async_error;
        if (!el_async_read(loop, socket, buffer, BUFFER_READ, read_handler, &error)) {
            printf("write_handler - error async operation: %s\n", error.message);
        }
    }


}

// todo (ageev) добавить в интерферй уоличество прочитанных байт и размер буффер
void read_handler(event_loop *loop, int socket, char *buffer, int size, client_status status, error_t error) {
    // TODO (ageev) сделать удаление элемента по индексу из вектора
    if (error.error) {
        perror(error.message);
        return;
    }
    if (status == DISCONNECTED) {
        printf("Client Disconnected - read handler\n");
        clients_connected--;
        if (clients_connected == 0) {
            el_stop(loop, NULL);
        }
        // TODO (ageev) remove context
        return;
    }

    if (size > 0) {
        vector_char *client_data = NULL;
        int client_index = find_client(contexts, socket);
        if (client_index == -1) {
            printf("error: Not found context for client");
            return;
        }
        client_data = VECTOR(contexts)[client_index].data;
        // Записываем принятый буфер в буффер контекста клиента
        error_t er;
        for (int i = 0; i < size; ++i) {
            VECTOR_PUSH_BACK(char, client_data, buffer[i], er);
            if (error.error) {
                printf("%s\n", error.message);
                return;
            }
        }
        memset(buffer, 0, size);
        // ищем конец сообщения
        int buff_len = VECTOR_SIZE(client_data);
        int  end_message = -2;
        for (int j = 0; j < buff_len; ++j) {
            if (VECTOR(client_data)[j] == '\r' && (j+4) < buff_len &&
                VECTOR(client_data)[j+1] == '\n' && VECTOR(client_data)[j+2] == '.' &&
                    VECTOR(client_data)[j+3] == '\r' && VECTOR(client_data)[j+4] == '\n')
            {
                end_message = j;
            }
        }
        if (end_message != -2) {
            if (end_message == -1) {
                // message empty
            }
            vector_char *message = s_malloc(sizeof(vector_char), NULL);
            if (message == NULL) {
                // ERROR
            }
            VECTOR_SUB(char, client_data, message, 0, end_message, error);
            vector_char *new_context_buffer = s_malloc(sizeof(vector_char), NULL);
            if ((end_message + 5) < buff_len) {
                VECTOR_SUB(char, client_data, new_context_buffer, end_message + 5, buff_len, error);
                VECTOR_FREE(client_data);
                free(client_data);
                VECTOR(contexts)[client_index].data = new_context_buffer;
            } else {
                VECTOR_INIT(char, new_context_buffer, error);
                VECTOR_FREE(client_data);
                free(client_data);
                VECTOR(contexts)[client_index].data = new_context_buffer;
            }
            size_t message_size = VECTOR_SIZE(message);
            if (message_size == 0) {
                printf("Client send empty message\n");
                error_t async_error;
                if (!el_async_read(loop, socket, buffer, BUFFER_READ, read_handler, &async_error)) {
                    printf("read handler - error async operation: %s\n", async_error.message);
                }
                return;
            }
            char *write_buffer = NULL;
            char *recv_message = make_buffer_from_vector_char(message);
            printf("recv message: %s\n--------\n", recv_message);
            free(recv_message);
            VECTOR_RESET(message, write_buffer);
            error_t  async_error;
            if (!el_async_write(loop, socket, write_buffer,message_size, write_handler, &async_error)) {
                printf("read handler - error async operation: %s\n", async_error.message);
            }
        } else {
            error_t async_error;
            if (!el_async_read(loop, socket, buffer, BUFFER_READ, read_handler, &async_error)) {
                printf("read handler - error async operation: %s\n", async_error.message);
            }
        }


    }

}


void accept_handler(event_loop *loop, int acceptor, int client_socket, struct sockaddr_in client_addr, error_t error) {
    if (error.error) {
        printf("%s\n", error.message);
    } else {
        clients_connected++;
        char ip[IP_BUFFER_LEN] = {0};
        uint16_t port = 0;
        get_addr(&client_addr, ip, IP_BUFFER_LEN, &port, NULL);
        printf("- connect client from: %s:%d\n", ip, port);
        client_context con;
        con.socket = client_socket;
        con.data = s_malloc(sizeof(client_context), &error);
        if (error.error) {
            printf("%s\n", error.message);
            return;
        }

        VECTOR_INIT(char, con.data, error);
        if (error.error) {
            printf("%s\n", error.message);
            return;
        }

        VECTOR_PUSH_BACK(client_context, contexts, con, error);
        char *read_buffer = s_malloc(BUFFER_READ, &error);
        if (error.error) {
            printf("%s\n", error.message);
            return;
        }
        el_async_read(loop, client_socket, read_buffer, BUFFER_READ, read_handler, &error);
        if (error.error) {
            printf("accept handler - error async operation: %s\n", error.message);
        }
    }



}


int main() {
    error_t error;
    contexts = s_malloc(sizeof(vector_context), &error);
    if (error.error) {
        printf("%s\n", error.message);
        return ERROR;
    }
    VECTOR_INIT(client_context, contexts, error);
    if (error.error) {
        printf("%s\n", error.message);
        return ERROR;
    }
    event_loop *loop = el_init(&error);
    if (error.error) {
        printf("%s\n", error.message);
        return ERROR;
    }
    int master_socket = make_server_socket("127.0.0.1", 8080, &error);
    if (error.error) {
        printf("%s\n", error.message);
        return ERROR;
    }
    el_async_accept(loop, master_socket, accept_handler, NULL);

    el_open(loop, ONE_THREAD, NULL);

    return 0;
}