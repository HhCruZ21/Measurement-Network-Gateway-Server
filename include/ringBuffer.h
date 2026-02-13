/******************************************************************************
 * @file    ringBuffer.h
 * @brief   Thread-safe circular buffer implementation for sensor samples.
 *
 * Provides:
 *  - Ring buffer structure definition
 *  - Initialization function
 *  - Single sample insertion
 *  - Single sample removal
 *  - Batch removal for network transmission
 *
 * Used for decoupling sensor acquisition and network streaming threads.
 *
 * @author  Haizon Helet Cruz
 * @date    2026-02-13
 ******************************************************************************/

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "mainH.h"
#include <string.h>

#define RING_BUF_SIZE 2048
#define NO_OF_SENSORS 5

typedef struct ring_buffer
{
    pthread_mutex_t rbMutex;
    sensor_data_t sampleArray[RING_BUF_SIZE];
    size_t write_index;
    size_t read_index;
    uint16_t count;
} ring_buffer_t;

ring_buffer_t *ringBufferInit();
void ringBufferAddSample(ring_buffer_t *rb, const sensor_data_t *sample);
void ringBufferRemoveSample(ring_buffer_t *rb, sensor_data_t *sample);
size_t ringBufferRemoveBatch(ring_buffer_t *rb,
                             sensor_data_t *dest,
                             size_t max_samples);

#endif /**  RING_BUFFER_H    **/