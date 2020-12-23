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
    SMTP_CONNECT = 0,
    SMTP_HELO = 1,
    SMTP_EHLO = 2,
    SMTP_MAIL = 3,
    SMTP_RCPT = 4,
    SMTP_DATA = 5,
    SMTP_MESSAGE = 6,
    SMTP_END_MESSAGE = 7,
    SMTP_RSET = 8,
    SMTP_QUIT = 9,
    SMTP_INVALID
} state_code;

typedef enum smtp_status_code {
    SMTP_SERVICE_READY = 220,
    SMTP_SERVICE_CLOSING = 221,
    SMTP_REQUESTED_ACTION_TAKEN_AND_COMPLETED = 250,
    SMTP_CONTEXT_INPUT = 354,
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
    state_code state_code;
} smtp_context;

typedef struct smtp_response {
    status_code status_code;
    char *message;
} smtp_response;

smtp_context* smtp_connect(char *server, char *port, smtp_context *context);
state_code smtp_send_helo(smtp_context *context);
state_code smtp_send_ehlo(smtp_context *context);
state_code smtp_send_mail(smtp_context *context, char *from_email);
state_code smtp_send_rcpt(smtp_context *context, char *to_email);
state_code smtp_send_data(smtp_context *context);
state_code smtp_send_message(smtp_context *context, char *message);
state_code smtp_send_end_message(smtp_context *context);
state_code smtp_send_rset(smtp_context *context);
state_code smtp_send_quit(smtp_context *context);

state_code send_smtp_request(smtp_context *context, char *str);
smtp_response get_smtp_response(smtp_context *context);

bool is_smtp_success(status_code);
