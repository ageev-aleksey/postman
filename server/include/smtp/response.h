//
// Created by nrx on 04.12.2020.
//

#ifndef SERVER_RESPONSE_H
#define SERVER_RESPONSE_H

#define SMTP_CODE_OK 250
#define SMTP_CODE_OK_MSG "Requested mail action okay, completed"

#define SMTP_CODE_MAIL_INPUT 354
#define SMTP_CODE_MAIL_INPUT_MSG "Start mail input; end with <CRLF>.<CRLF>"

#define SMTP_CODE_SYNTAX_ERROR 500
#define SMTP_CODE_SYNTAX_ERROR_MSG "Syntax error, command unrecognized"

#define SMTP_CODE_INVALID_ARGUMENT 501
#define SMTP_CODE_INVALID_ARGUMENT_MSG "Invalid argument: "

#define SMTP_CODE_COMMAND_NOT_IMPLEMENTED 502
#define SMTP_CODE_COMMAND_NOT_IMPLEMENTED_MSG "Command not implemented"

#define SMTP_CODE_INVALID_SEQUENCE 503
#define SMTP_CODE_INVALID_SEQUENCE_MSG "Bad sequence of commands"

#endif //SERVER_RESPONSE_H
