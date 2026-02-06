#include "../include/sensorThread.h"

sensor_config_t sensor_cfg[snsr_cnt];

void *sensorTask(void *arg)
{
    printf("Sensor thread initialized correctly...\n");
    (void)arg;
    int fd = open("/dev/meascdd", O_RDWR);
    int i;
    unsigned int tempsnsr_val = 0;

    static uint64_t last_adc_time = 0;
    static unsigned int adc_zero_cache = 0, adc_one_cache = 0;

    if (fd < 0)
    {
        printf("/dev/meascdd failed to open...\n");
        return NULL;
    }
    else
    {
        printf("File opened...\n");
    }

    initTimer();
    tempSnsrInit(fd);

    uint64_t now = getElapsedTime();

    for (i = 0; i < snsr_cnt; i++)
    {
        sensor_cfg[i].sid = i;
        pthread_mutex_init(&sensor_cfg[i].lock, NULL);
        sensor_cfg[i].rate_hz = 300; // default
        sensor_cfg[i].period_us = 1000000ULL / 300;
        sensor_cfg[i].next_deadline = now;
    }

    while (system_running)
    {
        uint64_t now = getElapsedTime();

        for (i = 0; i < snsr_cnt; i++)
        {
            pthread_mutex_lock(&sensor_cfg[i].lock);

            if (now >= sensor_cfg[i].next_deadline)
            {
                sensor_data_t sample = {
                    .sensor_id = i,
                    .timestamp = now};

                switch (i)
                {
                case temp_sid:
                    readTempSnsrVal(fd, &tempsnsr_val);
                    sample.sensor_value = tempsnsr_val;
                    break;

                case adc_zero_sid:
                case adc_one_sid:
                    if (last_adc_time != now)
                    {
                        getADC(fd, &adc_zero_cache, &adc_one_cache);
                        last_adc_time = now;
                    }
                    sample.sensor_value =
                        (i == adc_zero_sid) ? adc_zero_cache : adc_one_cache;
                    break;

                case sw_sid:
                    sample.sensor_value = readZedSwitches(fd);
                    break;

                case pb_sid:
                    sample.sensor_value = readZedPushButtons(fd);
                    break;
                }

                ringBufferAddSample(rb, &sample);

                sensor_cfg[i].next_deadline += sensor_cfg[i].period_us;
            }

            pthread_mutex_unlock(&sensor_cfg[i].lock);
        }

        usleep(50); // prevent CPU burn
    }

    tempSnsrPwrDwn(fd);
    close(fd);
    printf("Sensor thread finished...\n");
    return NULL;
}