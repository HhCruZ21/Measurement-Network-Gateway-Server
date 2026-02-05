#include "../include/sensorThread.h"

void *sensorTask(void *arg)
{

    printf("Sensor thread initialized correctly...\n");
    int fd = open("/dev/meascdd", O_RDWR);

    if (fd < 0)
        printf("/dev/meascdd failed to open...\n");
    else
        printf("File opened...\n");

    initTimer();
    ringBufferInit();
    tempSnsrInit(fd);

    while (1)
    {
    }

    printf("Sensor thread finished...\n");
    return NULL;
}