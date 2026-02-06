#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

#include "utils.h"
#include "ringBuffer.h"
#include <fcntl.h>

typedef struct
{
    uint32_t temp_config;      
    uint32_t adc_zero_config;      
    uint32_t adc_one_config;      
    uint32_t switch_config;   
    uint32_t pushb_config;     
} config_struct_t;

typedef struct {
    sensor_id_t sid;
    uint64_t period_us;
    uint64_t next_time;
} sensor_config_t;

void *sensorTask(void *arg);

#endif /**  SERVER_THREAD_H    **/