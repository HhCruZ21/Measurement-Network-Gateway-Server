#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#define PORT 50012

typedef struct 
{
    uint8_t sensor_id;
    uint32_t sensor_value;
    uint64_t timestamp;
} sensor_data_t;

#endif // MAIN_H