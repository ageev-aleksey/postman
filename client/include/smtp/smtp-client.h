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
#include <netdb.h>

enum smtp_status_code {
    SMTP_STATUS_OK = 0,
    SMTP_STATUS_NO_MEMORY = 1,
    SMTP_STATUS_CONNECT = 2,
    SMTP_STATUS_HANDSHAKE = 3,
    SMTP_STATUS_AUTH = 4,
    SMTP_STATUS_SEND = 5,
    SMTP_STATUS_RECV = 6,
    SMTP_STATUS_CLOSE = 7,
    SMTP_STATUS_SERVER_RESPONSE = 8,
    SMTP_STATUS_PARAM = 9,
    SMTP_STATUS_FILE = 10,
    SMTP_STATUS_DATE = 11,
    SMTP_STATUS_LAST
};

enum smtp_address_type {
    SMTP_ADDRESS_FROM = 0,
    SMTP_ADDRESS_TO = 1,
    SMTP_ADDRESS_CC = 2,
    SMTP_ADDRESS_BCC = 3
};

enum smtp_auth_method {
    SMTP_AUTH_NONE = 0,
    SMTP_AUTH_PLAIN = 1,
    SMTP_AUTH_LOGIN = 2
};

typedef struct smtp_address {
    char *email;
    char *name;
    enum smtp_address_type type;
} smtp_addr;

typedef struct smtp_attachment {
    char *name;
    char *base64_data;
} smtp_attach;

typedef struct smtp_header {
    char *key;
    char *value;
} smtp_header;

typedef struct smtp_message {
    int socket_desc;
    smtp_header *header_list;
    size_t num_headers;
    smtp_addr *address_list;
    size_t num_address;
    smtp_attach *attachment_list;
    size_t num_attachment;
    long timeout_sec;
    enum smtp_status_code status_code;
} smtp_message;

typedef struct smtp_ip {
    char ip[15];
    size_t num_ips;
} smtp_ip;

static smtp_message smtp_error_memory = {
    0,
    NULL,
    0,
    NULL,
    0,
    NULL,
    0,
    0,
    SMTP_STATUS_NO_MEMORY
};

enum smtp_status_code smtp_open(const char *server, const char *port, smtp_message **smtp_mes);
int smtp_connect(const char *server, const char *port, smtp_message *smtp_mes);
enum smtp_status_code smtp_initiate_handshake(smtp_message *smtp_mes);
smtp_ip* get_ip_by_hostname(char *hostname);
enum smtp_status_code get_smtp_status_code(const smtp_message *smtp_mes);
void set_smtp_status_code(smtp_message *smtp_mes, enum smtp_status_code status_code);
void set_smtp_read_timeout(smtp_message *smtp_mes, long seconds);
