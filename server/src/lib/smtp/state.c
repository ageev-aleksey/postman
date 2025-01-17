#include "smtp/state.h"
#include "smtp/regex.h"
#include "smtp/response.h"
#include "vector.h"
#include <assert.h>
#include <regex.h>
#include <string.h>
#include <stdio.h>

#define SMTP_START_BUFFER_SIZE 1024
#define SMTP_DATA_END ".\r\n"
#define SMTP_MAILBOX_SEP '@'

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
regex_t pr_smtp_reg_mailbox;
regex_t pr_smtp_reg_path;
regex_t pr_smtp_reg_empty_path;
regex_t pr_smtp_reg_postmaster;

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
    regcomp(&pr_smtp_reg_mailbox, RE_MAILBOX,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_path, RE_PATH,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_empty_path, RE_EMPTY_PATH,  REG_EXTENDED | REG_ICASE);
    regcomp(&pr_smtp_reg_postmaster, RE_PATH,  REG_EXTENDED | REG_ICASE);
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
    regfree(&pr_smtp_reg_mailbox);
    regfree(&pr_smtp_reg_path);
    regfree(&pr_smtp_reg_empty_path);
    regfree(&pr_smtp_reg_postmaster);
}

bool smtp_init(smtp_state *smtp, err_t *error) {
    CHECK_PTR(smtp, error, SMTP_DESCRIPTOR_IS_NULL);
    ERROR_SUCCESS(error);
    smtp->pr_fsm_state = AUTOFSM_ST_INIT;
    err_t  err;
    VECTOR_INIT(smtp_mailbox, &smtp->pr_rcpt_list, err);
    VECTOR_ERROR(err, error);
    smtp->pr_hello_addr = NULL;
    smtp->pr_mail_from = NULL;
    VECTOR_INIT(char, &smtp->pr_mail_data, err);
    VECTOR_ERROR(err, error);
    smtp->pr_buffer = malloc(sizeof(char)*SMTP_START_BUFFER_SIZE);
    smtp->pr_bsize = SMTP_START_BUFFER_SIZE;
    return true;
}

// Освобождение памяти из под внутренносетй вектора
// но из под вектора память не совобождается
void pr_smtp_rcpt_list_free(vector_smtp_mailbox *rcpts_list) {
    for (size_t j = 0; j < VECTOR_SIZE(rcpts_list); j++) {
        free(VECTOR(rcpts_list)[j].server_name);
        free(VECTOR(rcpts_list)[j].user_name);
        VECTOR(rcpts_list)[j].server_name = NULL;
        VECTOR(rcpts_list)[j].user_name = NULL;
    }
}

// Осовобождение памяти из под внутренностей
// необходимо освободить вектора после вызва функции
void pr_smtp_buffers_free(smtp_state *smtp) {
        pr_smtp_rcpt_list_free(&smtp->pr_rcpt_list);
        if (smtp->pr_hello_addr != NULL) {
            free(smtp->pr_hello_addr->address);
            free(smtp->pr_hello_addr);
            smtp->pr_hello_addr = NULL;
        }
        if (smtp->pr_mail_from != NULL) {
            free(smtp->pr_mail_from->user_name);
            free(smtp->pr_mail_from->server_name);
            free(smtp->pr_mail_from);
            smtp->pr_mail_from = NULL;
        }
        free(smtp->pr_buffer);
        smtp->pr_buffer = NULL;
        smtp->pr_bsize = 0;
}

void smtp_free(smtp_state *smtp) {
    if (smtp != NULL) {
        pr_smtp_buffers_free(smtp);
        VECTOR_FREE(&smtp->pr_rcpt_list);
        VECTOR_FREE(&smtp->pr_mail_data);
    }
}

/// ehlo = "EHLO" SP Domain CRLF
/// helo = "HELO" SP Domain CRLF
struct smtp_command pr_smtp_hello_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    struct smtp_command command;
    command.type = SMTP_HELLO;
    command.event = AUTOFSM_EV_HELO;
    command.status = true;
    command.arg = NULL;
    regmatch_t matcher[1];

    // Проверка адреса сервера в параметрах команды "(" RE_DOMAIN ")|(" RE_ADDRESS_LITERAL ")"
    if (regexec(&pr_smtp_reg_addr_literal, buff, 1, matcher, 0) == 0) {
        size_t len = (matcher->rm_eo - matcher->rm_so + 1)-2; // не учитываем знаки [ ]
        smtp_address *addr = malloc(sizeof(smtp_address));
        addr->type = SMTP_ADDRESS_TYPE_IPv4;
        addr->address = malloc(sizeof(char)*len);
        sub_str(buff, addr->address, matcher[0].rm_so+1, matcher[0].rm_eo-1); // не учитываем знаки [ ]
        command.arg = addr;
    } else if (regexec(&pr_smtp_reg_domain, buff, 1, matcher, 0) == 0) {
        size_t len = matcher->rm_eo - matcher->rm_so + 1;
        smtp_address *addr = malloc(sizeof(smtp_address));
        addr->type = SMTP_ADDRESS_TYPE_DOMAIN;
        addr->address = malloc(sizeof(char)*len);
        sub_str(buff, addr->address, matcher[0].rm_so, matcher[0].rm_eo);
        command.arg = addr;
    } else {
        command.arg = NULL;
        command.status = false;
    }
    return command;
}

/// "MAIL FROM:" ("<>" / Reverse-Path) [SP Mail-parameters] CRLF
struct smtp_command pr_smtp_mailfrom_proc(smtp_state *smtp, char *buff) {
    (void)smtp;
    struct smtp_command command;
    command.type = SMTP_MAILFROM;
    command.event = AUTOFSM_EV_MAIL;
    command.status = true;
    command.arg = NULL;
    regmatch_t matcher[1];
    // проверям соответствие формату
    if (regexec(&pr_smtp_reg_path,  buff, 1, matcher, 0) == 0) {
        // Проверяем наличие потового ящика
        if (regexec(&pr_smtp_reg_mailbox, buff, 1, matcher, 0) == 0) {
            char *split[2];
            split_sub_str(buff, matcher->rm_so, matcher->rm_eo, split, 2, SMTP_MAILBOX_SEP);
            smtp_mailbox *mailbox = malloc(sizeof(smtp_mailbox));
            mailbox->user_name = split[0];
            mailbox->server_name = split[1];
            command.arg = mailbox;
        } else {
            // Ошибка, которая не должна происходить
            assert(false);
        }
    } else {
        if (regexec(&pr_smtp_reg_empty_path, buff, 1, matcher, 0) == 0) {
            // Значит отправитель не указан
            command.status = true;
        } else {
            // некорректный формат
            command.status = false;
        }
    }
    return command;
}

/// "RCPT TO:" ("<Postmaster@" domain ">" / "<Postmaster>" / Forward-Path)
struct smtp_command pr_smtp_rcptto_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    struct smtp_command command;
    command.type = SMTP_RCPTTO;
    command.event = AUTOFSM_EV_RCPT;
    command.status = true;
    command.arg = NULL;
    regmatch_t matcher[1];
    // проверям соответствие формату
    if (regexec(&pr_smtp_reg_path,  buff, 1, matcher, 0) == 0) {
        // проверяем наличие почтового ящика
        if (regexec(&pr_smtp_reg_mailbox, buff, 1, matcher, 0) == 0) {
            char *split[2];
            split_sub_str(buff, matcher->rm_so, matcher->rm_eo, split, 2, SMTP_MAILBOX_SEP);
            smtp_mailbox *mailbox = malloc(sizeof(smtp_mailbox));
            mailbox->user_name = split[0];
            mailbox->server_name = split[1];
            command.arg = mailbox;
        } else if (regexec(&pr_smtp_reg_postmaster, buff, 1, matcher, 0) == 0) {
            // Проверка на postmaster
            smtp_mailbox *postmaster = malloc(sizeof(smtp_mailbox));
            postmaster->server_name = NULL;
            postmaster->user_name = "postmaster";
            command.arg = postmaster;
            sub_str(buff, command.arg, matcher->rm_so, matcher->rm_eo);
        } else {
            // некорректный офрмат
            command.status = false;
        }
    } else {
        // некорректный формат
        command.status = false;
    }
    return command;
}

struct smtp_command pr_smtp_data_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    (void) buff;
    struct smtp_command command;
    command.type = SMTP_DATA;
    command.event = AUTOFSM_EV_DATA;
    command.status = true;
    command.arg = NULL;
    return command;
}
/// "RSET"
struct smtp_command pr_smtp_rset_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    (void) buff;
    struct smtp_command command;
    command.type = SMTP_RSET;
    command.event = AUTOFSM_EV_RSET;
    command.status = true;
    command.arg = NULL;
    return command;
}

/// "VRFY"
struct smtp_command pr_smtp_vrfy_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    (void) buff;
    struct smtp_command command;
    command.type = SMTP_VRFY;
    command.event = AUTOFSM_EV_TEST;
    command.status = true;
    command.arg = NULL;
    return command;
}

/// "EXPN"
struct smtp_command pr_smtp_expn_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    (void) buff;
    struct smtp_command command;
    command.type = SMTP_EXPN;
    command.event = AUTOFSM_EV_TEST;
    command.status = true;
    command.arg = NULL;
    return command;
}

/// "HELP"
struct smtp_command pr_smtp_help_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    (void) buff;
    struct smtp_command command;
    command.type = SMTP_HELP;
    command.event = AUTOFSM_EV_TEST;
    command.status = true;
    command.arg = NULL;
    return command;
}

/// "NOOP"
struct smtp_command pr_smtp_noop_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    (void) buff;
    struct smtp_command command;
    command.type = SMTP_NOOP;
    command.event = AUTOFSM_EV_TEST;
    command.status = true;
    command.arg = NULL;
    return command;
}

/// "QUIT"
struct smtp_command pr_smtp_quit_proc(smtp_state *smtp, char *buff) {
    (void) smtp;
    (void) buff;
    struct smtp_command command;
    command.type = SMTP_QUIT;
    command.event = AUTOFSM_EV_QUIT;
    command.status = true;
    command.arg = NULL;
    return command;
}

///
smtp_command pr_smtp_command_parse(smtp_state *smtp, const char *message) {
    size_t message_size = strlen(message);
    trim_str(message, &smtp->pr_buffer, message_size, &smtp->pr_bsize);
    char *buff = smtp->pr_buffer;
    regmatch_t matcher[1];
    struct smtp_command command;
    if (regexec(&pr_smtp_reg_hello, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_hello_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_mailform, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_mailfrom_proc(smtp, buff);
    }  else if (regexec(&pr_smtp_reg_rcptto, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_rcptto_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_data, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_data_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_rset, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_rset_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_vrfy, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_vrfy_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_expn, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_expn_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_help, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_help_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_noop, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_noop_proc(smtp, buff);
    } else if (regexec(&pr_smtp_reg_quit, buff, 1, matcher, 0) == 0) {
        buff += matcher[0].rm_eo;
        command = pr_smtp_quit_proc(smtp, buff);
    } else {
        command.arg = NULL;
        command.status = false;
        command.type = SMTP_INVALID_COMMAND;
        command.event = AUTOFSM_EV_INVALID;
    }

    return command;
}

void pr_smtp_command_hello_free(smtp_state *smtp, smtp_command *command) {
    smtp_address *addr = command->arg;
    if (addr != NULL) {
        free(addr->address);
        addr->address = NULL;
        free(addr);
        addr = NULL;
    }
}

void pr_smtp_command_mailfrom_free(smtp_state *smtp, smtp_command *command) {
    struct smtp_mailbox *mailbox = command->arg;
    if (mailbox != NULL) {
        free(mailbox->server_name);
        mailbox->server_name = NULL;
        free(mailbox->user_name);
        mailbox->user_name = NULL;
        free(mailbox);
        mailbox = NULL;
    }
}

void pr_smtp_command_rcpto_free(smtp_state *smtp, smtp_command *command) {
    smtp_mailbox *rcpt = command->arg;
    if (rcpt != NULL) {
        free(rcpt->user_name);
        rcpt->user_name = NULL;
        free(rcpt->server_name);
        rcpt->server_name = NULL;
        free(rcpt);
        rcpt = NULL;
    }
}



smtp_status pr_smtp_command_hello(smtp_state *smtp, smtp_command *command) {
    if (command->status == true) {
        smtp_make_response(smtp, SMTP_CODE_OK, SMTP_CODE_OK_MSG);
        // очистка от предыдущей команды hello
        if (smtp->pr_hello_addr != NULL) {
            free(smtp->pr_hello_addr->address);
            smtp->pr_hello_addr->address = NULL;
            free(smtp->pr_hello_addr);
            smtp->pr_hello_addr = NULL;
        }
        smtp->pr_hello_addr = (smtp_address*) command->arg;
        command->arg = NULL;
        return SMTP_STATUS_OK;
    } else {
//        smtp_address *addr = (smtp_address*) command->arg;
//        free(addr->address);
        char message[] = SMTP_CODE_INVALID_ARGUMENT_MSG " Expected format <domain> or '['<ip>']'";
        smtp_make_response(smtp, SMTP_CODE_INVALID_ARGUMENT, message);
        return SMTP_STATUS_WARNING;
    }
}

smtp_status pr_smtp_command_mailfrom(smtp_state *smtp, smtp_command *command) {
    if (command->status == true) {
        // выполняем сброс данных из предудущей транзакции
        for (int j = 0; j < VECTOR_SIZE(&smtp->pr_rcpt_list); j++) {
            free(VECTOR(&smtp->pr_rcpt_list)[j].server_name);
            VECTOR(&smtp->pr_rcpt_list)[j].server_name = NULL;

            free(VECTOR(&smtp->pr_rcpt_list)[j].user_name);
            VECTOR(&smtp->pr_rcpt_list)[j].user_name = NULL;
        }
        VECTOR_CLEAR(&smtp->pr_rcpt_list);
        VECTOR_CLEAR(&smtp->pr_mail_data);
        if (smtp->pr_mail_from != NULL) {
            free(smtp->pr_mail_from->user_name);
            free(smtp->pr_mail_from->server_name);
            smtp->pr_mail_from->user_name = NULL;
            smtp->pr_mail_from->server_name = NULL;
            free(smtp->pr_mail_from);
            smtp->pr_mail_from = NULL;
        }

        smtp->pr_mail_from = (smtp_mailbox*) command->arg;
        command->arg = NULL;
        smtp_make_response(smtp, SMTP_CODE_OK, SMTP_CODE_OK_MSG);
        return SMTP_STATUS_OK;
    } else {
//        smtp_mailbox *mailbox = (smtp_mailbox*) command->arg;
//        free(mailbox->user_name);
//        free(mailbox->server_name);
        smtp_make_response(smtp, SMTP_CODE_INVALID_ARGUMENT, SMTP_CODE_INVALID_ARGUMENT_MSG);
        return SMTP_STATUS_WARNING;
    }
}

smtp_status pr_smtp_command_rcpto(smtp_state *smtp, smtp_command *command) {
    smtp_status ret;
    if (command->status == true) {
        smtp_make_response(smtp, SMTP_CODE_OK, SMTP_CODE_OK_MSG);
        smtp_mailbox *rcpt = (smtp_mailbox*) command->arg;
        err_t err;
        VECTOR_PUSH_BACK(smtp_mailbox, &smtp->pr_rcpt_list, *rcpt, err); // rcpt копируется
        free(rcpt);
        command->arg = NULL;
        ret = SMTP_STATUS_OK;
    } else {
        smtp_make_response(smtp, SMTP_CODE_INVALID_ARGUMENT, SMTP_CODE_INVALID_ARGUMENT_MSG);
        ret = SMTP_STATUS_WARNING;
    }
    return ret;
}

smtp_status pr_smtp_command_data(smtp_state *smtp, smtp_command *command) {
    smtp_make_response(smtp, SMTP_CODE_MAIL_INPUT, SMTP_CODE_MAIL_INPUT_MSG);
    return SMTP_STATUS_OK;
}

smtp_status pr_smtp_command_rset(smtp_state *smtp, smtp_command *command) {
    pr_smtp_buffers_free(smtp);
    VECTOR_CLEAR(&smtp->pr_rcpt_list);
    VECTOR_CLEAR(&smtp->pr_mail_data);
    smtp_make_response(smtp, SMTP_CODE_OK, SMTP_CODE_OK_MSG);
    return SMTP_STATUS_OK;
}

char *smtp_make_response(smtp_state *smtp, size_t code, const char* msg) {
    char code_str[5];
    sprintf(code_str, "%zu", code);
    char_make_buf_concat(&smtp->pr_buffer, &smtp->pr_bsize, 4, code_str, " ", msg, SMTP_COMMAND_END);
    return smtp->pr_buffer;
}



smtp_status smtp_parse(smtp_state *smtp, const char *message, char **buffer_reply, err_t *error) {
    CHECK_PTR(smtp, error, SMTP_DESCRIPTOR_IS_NULL);
    CHECK_PTR(smtp, error, SMTP_MESSAGE_IS_NULL);
    ERROR_SUCCESS(error);
    smtp_status  ret = SMTP_STATUS_ERROR;
    if (smtp->pr_fsm_state != AUTOFSM_ST_READ_DATA) {
        smtp_command command = pr_smtp_command_parse(smtp, message);
        if (command.type == SMTP_INVALID_COMMAND) {
            // Команда не распознона
            smtp_make_response(smtp, SMTP_CODE_SYNTAX_ERROR, SMTP_CODE_SYNTAX_ERROR_MSG);
            ret = SMTP_STATUS_WARNING;
        } else {
            te_autofsm_state new_state = autofsm_step(smtp->pr_fsm_state, command.event, NULL);
            if (new_state == AUTOFSM_ST_INVALID) {
                // Неверная последовательность команд
                smtp_make_response(smtp, SMTP_CODE_INVALID_SEQUENCE, SMTP_CODE_INVALID_SEQUENCE_MSG);
                ret = SMTP_STATUS_WARNING;

                // Осовобождение ресурсов из под команды
                if (command.type == SMTP_HELLO) {
                    pr_smtp_command_hello_free(smtp, &command);
                } else if (command.type == SMTP_MAILFROM) {
                    pr_smtp_command_mailfrom_free(smtp, &command);
                } else if (command.type == SMTP_RCPTTO) {
                    pr_smtp_command_rcpto_free(smtp, &command);
                }

            } else {
                // Обработка команд
                if (command.type == SMTP_HELLO) {
                   ret = pr_smtp_command_hello(smtp, &command);
                } else if (command.type == SMTP_MAILFROM) {
                    ret = pr_smtp_command_mailfrom(smtp, &command);
                } else if (command.type == SMTP_RCPTTO) {
                    ret = pr_smtp_command_rcpto(smtp, &command);
                } else if (command.type == SMTP_DATA) {
                    ret = pr_smtp_command_data(smtp, &command);
                } else if(command.type == SMTP_RSET) {
                    ret = pr_smtp_command_rset(smtp, &command);
                } else if(command.type == SMTP_QUIT) {
                    ret = SMTP_STATUS_EXIT;
                }   else if (command.type == SMTP_NOOP) {
                    smtp_make_response(smtp, SMTP_CODE_OK,
                                       SMTP_CODE_OK_MSG);
                    ret = SMTP_STATUS_OK;
                } else if (command.event == AUTOFSM_EV_TEST) {
                    smtp_make_response(smtp, SMTP_CODE_COMMAND_NOT_IMPLEMENTED,
                                       SMTP_CODE_COMMAND_NOT_IMPLEMENTED_MSG);
                    ret = SMTP_STATUS_WARNING;
                }
            }
            if (new_state != AUTOFSM_ST_INVALID) {
                smtp->pr_fsm_state = new_state;
            }
        }
      //  free(command.arg);
    } else {
        // записываем данные в буфер до тех пор пока не встретится ".\r\n" (строка содержащая только точку)
        if (strcmp(SMTP_DATA_END, message) == 0) {
            smtp->pr_fsm_state = autofsm_step(smtp->pr_fsm_state, AUTOFSM_EV_END, NULL);
            smtp_make_response(smtp, SMTP_CODE_OK, SMTP_CODE_OK_MSG);
            ret = SMTP_STATUS_DATA_END;
        } else {
            err_t  err;
            for (int j = 0; message[j] != '\0'; j++) {
                VECTOR_PUSH_BACK(char, &smtp->pr_mail_data, message[j], err);
            }
            ret = SMTP_STATUS_CONTINUE;
        }
    }
    *buffer_reply = smtp->pr_buffer;
    smtp->pr_status = ret;
    return ret;
}


bool smtp_move_buffer(smtp_state *smtp, char **buffer, size_t *blen, err_t *error) {
    // TODO (ageev) Плохо лазить во внутренности другой структуры
    ERROR_SUCCESS(error);
    *buffer = smtp->pr_mail_data.array;
    *blen = VECTOR_SIZE(&smtp->pr_mail_data);
    err_t err;
    VECTOR_INIT(char, &smtp->pr_mail_data, err);
    return true;
}

vector_smtp_mailbox* smtp_get_rcpt(smtp_state *smtp) {
    return &smtp->pr_rcpt_list;
}

smtp_mailbox* smtp_get_sender(smtp_state *smtp) {
    return smtp->pr_mail_from;
}

smtp_status smtp_get_status(smtp_state *smtp) {
    return smtp->pr_status;
}

smtp_address smtp_get_hello_addr(smtp_state *smtp) {
    if (smtp != NULL && smtp->pr_hello_addr != NULL) {
        return *smtp->pr_hello_addr;
    }
    smtp_address ret = {0};
    return ret;
}