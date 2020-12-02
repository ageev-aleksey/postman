#include "smtp/state.h"
#include "smtp/regex.h"
#include <regex.h>
#include <string.h>

#define SMTP_START_BUFFER_SIZE 1024
#define SMTP_COMMAND1_LEN 5

#define VECTOR_ERROR(first_error_, second_error_) \
do {                                              \
       if ((first_error_).error) {\
if ((second_error_) != NULL) { \
*(second_error_) = (first_error_); \
} \
return false;\
}               \
} while(0);

const char SMTP_DESCRIPTOR_IS_NULL[] = "pointer of smtp_state is null";
const char SMTP_MESSAGE_IS_NULL[] = "pointer of smtp message is null";


regex_t pr_smtp_reg_hello;
regex_t pr_smtp_reg_mailform;
regex_t pr_smtp_reg_rcptto;
regex_t pr_smtp_reg_data;
regex_t pr_smtp_reg_rset;
regex_t pr_smtp_reg_vrfy;
regex_t pr_smtp_reg_expn;
regex_t pr_smtp_reg_help;
regex_t pr_smtp_reg_noop;
regex_t pr_smtp_reg_quit;

regex_t pr_smtp_reg_domain;
regex_t pr_smtp_reg_addr_literal;

void smtp_lib_init() {
    regcomp(&pr_smtp_reg_hello, RE_HELLO,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_mailform, RE_MAIL_FROM,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_rcptto, RE_RCPT_TO,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_data, RE_DATA,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_rset, RE_RSET,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_vrfy, RE_VRFY,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_expn, RE_EXPN,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_help, RE_HELP,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_noop, RE_NOOP,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_quit, RE_QUIT,  REG_EXTENDED | REG_ICASE);

    regcomp(&pr_smtp_reg_domain, RE_DOMAIN,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_addr_literal, RE_ADDRESS_LITERAL,  REG_EXTENDED | REG_ICASE);
}

void smtp_lib_free() {
    regfree(&pr_smtp_reg_hello);
    regfree(&pr_smtp_reg_mailform);
    regfree(&pr_smtp_reg_rcptto);
    regfree(&pr_smtp_reg_data);
    regfree(&pr_smtp_reg_rset);
    regfree(&pr_smtp_reg_vrfy);
    regfree(&pr_smtp_reg_expn);
    regfree(&pr_smtp_reg_help);
    regfree(&pr_smtp_reg_noop);
    regfree(&pr_smtp_reg_quit);

    regfree(&pr_smtp_reg_domain);
    regfree(&pr_smtp_reg_addr_literal);
}

bool smtp_init(smtp_state *smtp, error_t *error) {
    CHECK_PTR(smtp, error, SMTP_DESCRIPTOR_IS_NULL);
    ERROR_SUCCESS(error);
    smtp->pr_fsm_state = AUTOFSM_ST_INIT;
    error_t  err;
    VECTOR_INIT(recipient, &smtp->pr_rcpt_list, err);
    VECTOR_ERROR(err, error);
    smtp->hello_addr = NULL;
    smtp->pr_mail_from = NULL;
    smtp->hello_addr = NULL;
    VECTOR_INIT(char, &smtp->pr_mail_data, err);
    VECTOR_ERROR(err, error);
    smtp->pr_buffer = malloc(sizeof(char)*SMTP_START_BUFFER_SIZE);
    smtp->pr_bsize = SMTP_START_BUFFER_SIZE;
    return true;
}
void smtp_free(smtp_state *smtp) {
    if (smtp != NULL) {
        VECTOR_FREE(&smtp->pr_rcpt_list);
        free(smtp->hello_addr);
        free(smtp->pr_mail_from);
        free(smtp->hello_addr);
        VECTOR_FREE(&smtp->pr_mail_data);
    }
}

struct smtp_command pr_smtp_hello_proc(smtp_state *smtp, char *buff) {
    struct smtp_command command;
    command.type = SMTP_HELLO;
    regmatch_t matcher[1];
    // Проверка адреса сервера в параметрах команды "(" RE_DOMAIN ")|(" RE_ADDRESS_LITERAL ")"
    if (regexec(&pr_smtp_reg_domain, buff, 1, matcher, 0) == 0) {
        size_t len = matcher->rm_eo - matcher->rm_so + 1;
        command.arg = malloc(sizeof(char) * len);
        sub_str(buff, command.arg, matcher[0].rm_so, matcher[0].rm_eo);
    } else if (regexec(&pr_smtp_reg_addr_literal, buff, 1, matcher, 0) == 0) {
        size_t len = matcher->rm_eo - matcher->rm_so + 1;
        command.arg = malloc(sizeof(char) * len);
        sub_str(buff, command.arg, matcher[0].rm_so, matcher[0].rm_eo);
    } else {
        command.arg = NULL;
    }
    return command;
}


smtp_command pr_smtp_command_parse(smtp_state *smtp, const char *message, error_t *error) {
    size_t message_size = strlen(message);
    trim_str(message, &smtp->pr_buffer, message_size, &smtp->pr_bsize);
    char *buff = smtp->pr_buffer;
    regmatch_t matcher[1];
    struct smtp_command command;
    if (regexec(&pr_smtp_reg_hello, buff, 1, matcher, 0) == 0) {
        buff += SMTP_COMMAND1_LEN;
        command = pr_smtp_hello_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_mailform, buff, 1, matcher, 0) == 0) {

    }  else if (regexec(&pr_smtp_reg_rcptto, buff, 1, matcher, 0) == 0) {

    } else if (regexec(&pr_smtp_reg_data, buff, 1, matcher, 0) == 0) {

    } else if (regexec(&pr_smtp_reg_rset, buff, 1, matcher, 0) == 0) {

    } else if (regexec(&pr_smtp_reg_vrfy, buff, 1, matcher, 0) == 0) {

    } else if (regexec(&pr_smtp_reg_expn, buff, 1, matcher, 0) == 0) {

    } else if (regexec(&pr_smtp_reg_help, buff, 1, matcher, 0) == 0) {

    } else if (regexec(&pr_smtp_reg_noop, buff, 1, matcher, 0) == 0) {

    } else if (regexec(&pr_smtp_reg_quit, buff, 1, matcher, 0) == 0) {

    } else {
        // TODO error;
    }

    return command;
}

smtp_status smtp_parse(smtp_state *smtp, const char *message, char **buffer_reply, error_t *error) {
    CHECK_PTR(smtp, error, SMTP_DESCRIPTOR_IS_NULL);
    CHECK_PTR(smtp, error, SMTP_MESSAGE_IS_NULL);
    ERROR_SUCCESS(error);

}