#include "../include/ringBuffer.h"

ring_buffer_t *rb;

ring_buffer_t *ringBufferInit()
{
    ring_buffer_t *rb = malloc(sizeof(ring_buffer_t));
    if(!rb)
        return NULL;

    pthread_mutex_init(&rb->rbMutex, NULL);
    rb->write_index = 0;
    rb->read_index = 0;
    rb->count = 0;

    memset(rb->sampleArray, 0, sizeof(rb->sampleArray));
    return rb;
}

void ringBufferAddSample(ring_buffer_t *rb, const sensor_data_t *sample)
{
    if(!rb || !sample)
        return;

    pthread_mutex_lock(&rb->rbMutex);

    rb->sampleArray[rb->write_index] = *sample;
    rb->write_index = (rb->write_index + 1) % RING_BUF_SIZE;

    if(rb->count < RING_BUF_SIZE)
        rb->count++;
    else
        rb->read_index = (rb->read_index + 1) % RING_BUF_SIZE;

    pthread_mutex_unlock(&rb->rbMutex);
}

void ringBufferRemoveSample(ring_buffer_t *rb, sensor_data_t *sample)
{
    if(!rb || !sample)
        return;

    pthread_mutex_lock(&rb->rbMutex);
    if(rb->count == 0)
    {
        pthread_mutex_unlock(&rb->rbMutex);
        return;
    }
    *sample = rb->sampleArray[rb->read_index];
    rb->read_index = (rb->read_index + 1) % RING_BUF_SIZE;
    rb->count--;
    
    pthread_mutex_unlock(&rb->rbMutex);
}