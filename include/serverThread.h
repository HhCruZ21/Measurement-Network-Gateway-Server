#include "mainH.h"
#include <error.h>

#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct {
    int server_port;
} server_thread_arg_t;


void *serverTask(void *arg);