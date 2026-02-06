#ifndef NETWORK_THREAD_H
#define NETWORK_THREAD_H

#include "mainH.h"
#include <error.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct {
    int server_port;
} network_thread_arg_t;

typedef enum {
    STREAM_IDLE,
    STREAM_RUNNING,
    STREAM_DISCONNECT
} stream_t;

void *networkTask(void *arg);

#endif /**  NETWORK_THREAD_H    **/