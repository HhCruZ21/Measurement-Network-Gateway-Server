#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#define PORT 50012

extern ring_buffer_t *rb;

typedef enum {
    temp_sid = 0,
    adc_zero_sid,
    adc_one_sid,
    sw_sid,
    pb_sid
} sensor_id_t;

typedef struct 
{
    sensor_id_t sensor_id;
    unsigned int sensor_value;
    uint64_t timestamp;
} sensor_data_t;

#endif // MAIN_H