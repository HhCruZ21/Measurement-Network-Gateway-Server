#include <stdio.h>
#include <unistd.h>
#include <error.h>

#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct {
    int server_port;
} server_thread_arg;


void *sensorTask(void *arg);