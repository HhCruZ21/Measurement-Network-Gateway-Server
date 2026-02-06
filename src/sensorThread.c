#include "../include/sensorThread.h"

config_struct_t main_config = {
    .temp_config = 300,
    .adc_zero_config = 300,
    .adc_one_config = 300,
    .switch_config = 300,
    .pushb_config = 300};

void *sensorTask(void *arg)
{
    printf("Sensor thread initialized correctly...\n");
    int fd = open("/dev/meascdd", O_RDWR);
    int i;
    unsigned int tempsnsr_val = 0, adc_zero = 0, adc_one = 0;

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

    config_struct_t cfg = main_config;

    sensor_config_t config_tasks[] = {
        {temp_sid, 1000000ULL / cfg.temp_config, 0},
        {adc_zero_sid, 1000000ULL / cfg.adc_zero_config, 0},
        {adc_one_sid, 1000000ULL / cfg.adc_one_config, 0},
        {sw_sid, 1000000ULL / cfg.switch_config, 0},
        {pb_sid, 1000000ULL / cfg.pushb_config, 0},
    };

    const int NUM_TASKS = sizeof(config_tasks) / sizeof(config_tasks[0]);

    uint64_t now = getElapsedTime();
    for (i = 0; i < NUM_TASKS; i++)
        config_tasks[i].next_time = now;

    while (1)   // Thread exits cleanly to be implemented
    {
        uint64_t current_time = getElapsedTime();

        /* -------- Find earliest deadline (EDF) -------- */
        uint64_t sleep_target = config_tasks[0].next_time;
        for (int i = 1; i < NUM_TASKS; i++)
        {
            if (config_tasks[i].next_time < sleep_target)
                sleep_target = config_tasks[i].next_time;
        }

        /* -------- High-precision sleep -------- */
        if (current_time < sleep_target)
        {
            uint64_t sleep_us = sleep_target - current_time;
            while (sleep_us > 0)
            {
                uint64_t chunk = (sleep_us > 100) ? 100 : sleep_us;
                usleep(chunk);
                sleep_us -= chunk;
                current_time = getElapsedTime();
                if (current_time >= sleep_target)
                    break;
            }
        }

        for (i = 0; i < NUM_TASKS; i++)
        {
            if (current_time >= config_tasks[i].next_time)
            {
                sensor_data_t sample;
                sample.sensor_id = config_tasks[i].sid;
                sample.timestamp = current_time;
                switch (config_tasks[i].sid)
                {
                case temp_sid:
                    readTempSnsrVal(fd, &tempsnsr_val);
                    sample.sensor_value = tempsnsr_val;
                    break;

                case adc_zero_sid:
                case adc_one_sid:
                    getADC(fd, &adc_zero, &adc_one);
                    sample.sensor_value = (config_tasks[i].sid == adc_zero_sid) ? adc_zero : adc_one;
                    break;

                case sw_sid:
                    sample.sensor_value = readZedSwitches(fd);
                    break;

                case pb_sid:
                    sample.sensor_value = readZedPushButtons(fd);
                    break;
                }
                ringBufferAddSample(rb, &sample);
                config_tasks[i].next_time += config_tasks[i].period_us;
            }
        }
    }

    if (tempSnsrPwrDwn(fd) != 0)
    {
        close(fd);
        return NULL;
    }

    printf("Sensor thread finished...\n");
    return NULL;
}