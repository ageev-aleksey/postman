//
// Created by nrx on 18.12.2020.
//

#ifndef SERVER_OPTIONS_H
#define SERVER_OPTIONS_H
#include <stdbool.h>
#include "server/global_context.h"

bool args_parse(int argc, char **argv, struct server_configuration*);

bool args_usage(int argc, char **argv);

#endif //SERVER_OPTIONS_H
