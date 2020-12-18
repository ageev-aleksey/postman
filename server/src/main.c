//
// Created by nrx on 23.10.2020.
//

#include "server/global_context.h"
#include "log/context.h"
#include "event_loop/event_loop.h"
#include <stdio.h>
#include <unistd.h>
#include "server/args.h"

#define ERROR (-1)
#define OK_FLAG 0
#define BUFFER_READ 5

#include "server/thread_pool.h"

void *worker_thread (void *args) {
    LOG_INFO("%s", "thread start");
    err_t error;
    while (el_is_run(server_config.loop)) {
        el_run(server_config.loop, &error);
        if (error.error != OK && error.error != NOT_FOUND) {
            LOG_ERROR("el_run: %s", error.message);
            return NULL;
        }
        sleep(1);
    }

    LOG_INFO("%s", "thread stop");
    return NULL;
}

int main(int argc, char **argv) {
    if (!args_parse(argc, argv, &server_config)) {
        return OK_FLAG;
    }

    int status = OK_FLAG;
    int master_socket = ERROR;
    LOG_INIT();
    smtp_lib_init();
    thread_pool_t threads;
    thread_pool_init(&threads);

    LOG_INFO("%s", "server start init...");
    if (!server_config_init(server_config.conf_path)) {
        status = ERROR;
        return ERROR;
    }

    err_t error;
    master_socket = make_server_socket(server_config.ip,
                                       server_config.port,
                                       &error);
    if (master_socket == ERROR) {
        status = ERROR;
        LOG_ERROR("create server socket: %s", error.message);
        goto exit;
    }
    el_async_accept(server_config.loop, master_socket, handler_accept, &error);
    if (error.error) {
        status = ERROR;
        LOG_ERROR("el_async_accept: %s", error.message);
        goto exit;
    }

    LOG_INFO("%s", "Server init");

    /////////////////////////
//    thread_pool_t threads;
//    thread_pool_init(&threads);
//    thread_pool_push(&threads, thread_test, NULL);
//    thread_pool_free(&threads);
    ////////////////////////
    el_open(server_config.loop, OWN_THREAD, &error);

    for (int i = 1; i < server_config.num_worker_threads; i++) {
        thread_pool_push(&threads, worker_thread, NULL);
    }

    worker_thread(NULL);


exit:
    thread_pool_free(&threads);
    if (master_socket != ERROR) {
        close(master_socket);
    }
    server_config_free();
    smtp_lib_free();

    LOG_INFO("%s", "Server stop!");
    LOG_FREE();
    return status;
}