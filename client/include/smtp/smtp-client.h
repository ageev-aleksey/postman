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
#include <stdbool.h>

typedef enum smtp_state_code {
    OK = 0,
    NO_MEMORY = 1,
    CONNECT = 2,
    HANDSHAKE = 3,
    AUTH = 4,
    SEND = 5,
    RECV = 6,
    CLOSE = 7,
    SERVER_RESPONSE = 8,
    PARAM = 9,
    SMTP_FILE = 10,
    DATE = 11,
    INVALID
} state_code;

typedef enum smtp_status_code {
    SMTP_SERVICE_READY = 220,
    SMTP_SERVICE_CLOSING = 221,
    SMTP_REQUESTED_ACTION_TAKEN_AND_COMPLETED = 250,
    SMTP_MESSAGE_INPUT = 354,
    SMTP_SERVICE_IS_NOT_AVAILABLE = 421,
    SMTP_REQUESTED_COMMAND_FAILED = 450,
    SMTP_COMMAND_ABORTED_SERVER_ERROR = 451,
    SMTP_COMMAND_ABORTED_INSUFFICIENT_SYSTEM_STORAGE = 452,
    SMTP_SERVER_CANNOT_DEAL_WITH_THE_COMMAND = 455,
    SMTP_SERVER_COULD_NOT_RECOGNIZE_COMMAND = 500,
    SMTP_SYNTAX_ERROR_COMMAND_ARGS = 501,
    SMTP_COMMAND_IS_NOT_IMPLEMENTED = 502,
    SMTP_SERVER_HAS_ENCOUNTERED_A_BAD_SEQUENCE_OF_COMMANDS = 503,
    SMTP_COMMAND_PARAMETER_IS_NOT_IMPLEMENTED = 504,
    SMTP_HOST_NEVER_ACCEPTS_MAIL = 521,
    SMTP_MESSAGE_COULD_NOT_BE_DELIVERED_FOR_POLICY_REASONS = 541,
    SMTP_USER_MAILBOX_WAS_UNAVAILABLE = 550,
    SMTP_RECIPIENT_IS_NOT_LOCAL_TO_THE_SERVER = 551,
    SMTP_ACTION_WAS_ABORTED_DUE_TO_EXCEEDED_STORAGE_ALLOCATION = 552,
    SMTP_MAILBOX_NAME_IS_INVALID = 553,
    SMTP_MAILBOX_DISABLED = 554
} status_code;

typedef enum smtp_address_type {
    FROM = 0,
    TO = 1
} addr_type;

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
    state_code state_code;
} smtp_message;

typedef struct smtp_response {
    status_code status_code;
    char *message;
} smtp_response;

typedef struct smtp_ip {
    char ip[15];
    size_t num_ips;
} smtp_ip;

state_code smtp_open(char *server, char *port, smtp_message **smtp_mes);
int smtp_connect(char *server, char *port, smtp_message *smtp_mes);
state_code smtp_handshake(smtp_message *smtp_mes);
state_code smtp_helo(smtp_message *smtp_mes);

state_code send_smtp_request(smtp_message *smtp_mes, char *str);
smtp_response get_smtp_response(smtp_message *smtp_mes);
smtp_ip* get_ip_by_hostname(char *hostname);
state_code get_smtp_state_code(smtp_message *smtp_mes);
void set_smtp_state_code(smtp_message *smtp_mes, state_code state_code);
bool is_smtp_success(status_code);
