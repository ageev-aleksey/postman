#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

typedef struct pselect_thread_args {
    int *fdmax;
    fd_set *read_fds;
    fd_set *write_fds;
    char *data;
} pselect_args_t;

char **read_ips();
unsigned int receive_line(int socket_d, char *dist_buffer);
unsigned int send_message(int socket_d, char *message);
void *pselect_thread(void *args);

int main(int argc, char **argv) {

    fd_set master;
    fd_set read_fds;
    fd_set write_fds;
    int fdmax = 0;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    printf("CLIENT START\n");

    char *buffer_receive_message = malloc(1000);
    char *servers_data = malloc(100);
    memset(servers_data, 0, sizeof *servers_data);

    char **server_configs = read_ips();

    for (int i = 0; i < 3; i++) {
        char *ip = strtok(server_configs[i], ":");
        char *port = strtok(NULL, ":");
        char *data = strtok(NULL, ":");

        int server_socket = socket(PF_INET, SOCK_STREAM, 0);
        fcntl(server_socket, F_SETFL, O_NONBLOCK);

        if (server_socket == -1) {
            perror("ERROR: server socket");
            break;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = PF_INET;
        inet_aton(ip, &(server_addr.sin_addr));
        server_addr.sin_port = htons(atoi(port));
        memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

        socklen_t socklen = sizeof(struct sockaddr);

        if (connect(server_socket, (struct sockaddr *) &server_addr, socklen) == -1 && errno != EINPROGRESS) {
            perror("ERROR: connect");
            break;
        } else {
            printf("server socket for %s:%s - %i\n", ip, port, server_socket);
        }

        FD_SET(server_socket, &master);

        servers_data[server_socket] = data;

        if (server_socket > fdmax) {
            fdmax = server_socket;
        }
    }


    read_fds = master;
    write_fds = master;

    while (TRUE) {

        for (int fd = 0; fd <= fdmax; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                memset(buffer_receive_message, 0, sizeof *buffer_receive_message);
                receive_line(fd, buffer_receive_message);

                printf("Output socket #%i: %s\n", fd, buffer_receive_message);
            }
            if (FD_ISSET(fd, &write_fds)) {
                send_message(fd, &servers_data[fd]);
            }
        }
    }
}

char **read_ips() {
    char *datafile = "/home/ubuntu/pvs/pvs-postman/client/resource/servers_ip.config";
    int fd = open(datafile, O_RDONLY, S_IRUSR|S_IWUSR);
    if (fd == -1) {
        perror("ERROR: open file");
        return NULL;
    }

    char **server_configs = malloc(10);
    for (int i = 0; i < 3; i++) {
        char *server_config = malloc(100);
        server_configs[i] = server_config;

        char byte;
        int count = 0;
        while (read(fd, &byte, 1) > 0) {
            if (byte == '\n') {
                break;
            }
            *(server_config + count) = byte;
            count++;
        }
    }

    return server_configs;
}

unsigned int receive_line(int socket_d, char *dist_buffer) {
    char *ptr = dist_buffer;
    unsigned int start_size = sizeof(dist_buffer);
    memset(dist_buffer, 0, start_size);
    unsigned int count_bytes = 0;

    while (recv(socket_d, ptr, 1, 0) != -1) {
        count_bytes++;
        ptr++;
    }

    return strlen(dist_buffer);
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

