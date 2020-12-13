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

typedef enum type_header {
    SMTP_DATE,
    SMTP_FROM,
    SMTP_SENDER,
    SMTP_REPLY_TO,
    SMTP_TO,
    SMTP_CC,
    SMTP_BCC,
    SMTP_MESSAGE_ID,
    SMTP_IN_REPLY_TO,
    SMTP_REFERENCES,
    SMTP_SUBJECT,
    SMTP_COMMENTS,
    SMTP_KEYWORDS,
    SMTP_RESENT_DATE,
    SMTP_RESENT_FROM,
    SMTP_RESENT_SENDER,
    SMTP_RESENT_TO,
    SMTP_RESENT_CC,
    SMTP_RESENT_BCC,
    SMTP_RESENT_MESSAGE_ID,
    SMTP_RETURN_PATH,
    SMTP_RECEIVED,
    SMTP_ENCRYPTED,
    SMTP_CONTENT_TYPE,
    SMTP_CONTENT_TRANSFER_ENCODING
} type_header;

typedef enum smtp_status_code {
    SMTP_SERVICE_READY = 220,
    SMTP_SERVICE_CLOSING = 221,
    SMTP_REQUESTED_ACTION_TAKEN_AND_COMPLETED = 250,
    smtp_context_INPUT = 354,
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
    smtp_context_COULD_NOT_BE_DELIVERED_FOR_POLICY_REASONS = 541,
    SMTP_USER_MAILBOX_WAS_UNAVAILABLE = 550,
    SMTP_RECIPIENT_IS_NOT_LOCAL_TO_THE_SERVER = 551,
    SMTP_ACTION_WAS_ABORTED_DUE_TO_EXCEEDED_STORAGE_ALLOCATION = 552,
    SMTP_MAILBOX_NAME_IS_INVALID = 553,
    SMTP_MAILBOX_DISABLED = 554,
    UNDEFINED_ERROR
} status_code;


typedef struct smtp_address {
    char *email;
    char *name;
} smtp_addr;

typedef struct smtp_header {
    char *key;
    char *value;
} smtp_header;

typedef struct smtp_context {
    int socket_desc;
    char *address;
    char *port;
    smtp_header *header_list;
    size_t num_headers;
    smtp_addr from;
    smtp_addr *to;
    size_t num_to;
    char *message;
    char *from_domain;
    state_code state_code;
} smtp_context;

typedef struct smtp_response {
    status_code status_code;
    char *message;
} smtp_response;

typedef struct smtp_ip {
    char ip[15];
    size_t num_ips;
} smtp_ip;

smtp_context* smtp_open(char *server, char *port, smtp_context **smtp_cont);
int smtp_connect(char *server, char *port, smtp_context *smtp_cont);
state_code smtp_handshake(smtp_context *smtp_cont);
state_code smtp_helo(smtp_context *smtp_cont);
state_code smtp_ehlo(smtp_context *smtp_cont);
state_code smtp_mail(smtp_context *smtp_cont, char *from_email, char *from_name);
state_code smtp_rcpt(smtp_context *smtp_cont, smtp_addr *to, size_t size_to);
state_code smtp_data(smtp_context *smtp_cont, char *message);
state_code smtp_rset(smtp_context *smtp_cont);

state_code send_smtp_request(smtp_context *smtp_cont, char *str);
smtp_response get_smtp_response(smtp_context *smtp_cont);
smtp_ip* get_ip_by_hostname(char *hostname);
state_code get_smtp_state_code(smtp_context *smtp_cont);
void set_smtp_state_code(smtp_context *smtp_cont, state_code state_code);
bool is_smtp_success(status_code);
