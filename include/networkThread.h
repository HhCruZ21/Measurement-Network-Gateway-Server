/******************************************************************************
 * @file    networkThread.h
 * @brief   TCP server thread interface for sensor data streaming.
 *
 * Defines:
 *  - Network thread entry function
 *  - Command handling states
 *  - Sensor rate configuration structure
 *  - Stream state machine
 *
 * Responsible for client communication, command parsing, and
 * real-time batch transmission of sensor data.
 *
 * @author  Haizon Helet Cruz
 * @date    2026-02-13
 ******************************************************************************/

#ifndef NETWORK_THREAD_H
#define NETWORK_THREAD_H

#include "mainH.h"
#include <error.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> // For TCP_NODELAY
#include <netinet/in.h>
#include <inttypes.h>

#define CMD_BUF_SIZE 1024

extern volatile int system_running;

typedef struct
{
    int server_port;
} network_thread_arg_t;

typedef struct
{
    uint32_t sensor_id;
    uint32_t rate_hz;
} sensor_rate_t;

typedef enum
{
    STREAM_IDLE,
    STREAM_RUNNING,
    STREAM_DISCONNECT
} stream_t;

void *networkTask(void *arg);

#endif /**  NETWORK_THREAD_H    **/