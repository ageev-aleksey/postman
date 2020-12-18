//
// Created by nrx on 18.12.2020.
//

#include "server/args.h"
#include <getopt.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>

bool args_parse(int argc, char **argv, struct server_configuration* config) {
    struct option opts[] = {
            {"config", required_argument, NULL, 'c'},
            {"help", no_argument, NULL, 'h'},
            {0, 0, 0, 0}
    };
    int parameter = 0;
    int optindex = 0;
    strcpy(config->conf_path, "./config.cfg");
    int num_processed = 1;
    while ((parameter = getopt_long(argc, argv, "c:", opts, &optindex)) != -1) {
        switch (parameter) {
            case ('c'): {
                num_processed++;
                strcpy(config->conf_path, optarg);
                break;
            }
            case ('h'): {
                num_processed++;
                args_usage(argc, argv);
                return false;
            }
        }
    }

    return num_processed == argc;
}

bool args_usage(int argc, char **argv) {
    printf("Usage: %s [--config <path to config file>]\n", argv[0]);
    return true;
}