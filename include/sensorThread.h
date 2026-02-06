#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

#include "utils.h"
#include "ringBuffer.h"
#include <fcntl.h>

extern volatile int system_running;

typedef struct {
    sensor_id_t sid;
    uint32_t rate_hz;
    uint64_t period_us;
    uint64_t next_deadline;
    pthread_mutex_t lock;
} sensor_config_t;

extern sensor_config_t sensor_cfg[snsr_cnt];

void *sensorTask(void *arg);

#endif /**  SERVER_THREAD_H    **/