#include <netdb.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

typedef struct ips {
    char **ip_array;
    size_t ips_size;
} ips;

char *get_addr_by_socket(int socket);
ips get_ips_by_hostname(char *hostname);
int resolvmx(const char *name, char **mxs, int limit);
char *receive_line(int socket_d);
int send_line(int socket_d, char *message);
bool set_socket_blocking_enabled(int socket, bool blocking);
