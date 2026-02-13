/******************************************************************************
 * @file    main.c
 * @brief   Application entry point.
 *
 * Initializes:
 *  - Ring buffer
 *  - Sensor acquisition thread
 *  - TCP network thread
 *
 * Manages system lifecycle and graceful shutdown.
 *
 * @author  Haizon Helet Cruz
 * @date    2026-02-13
 ******************************************************************************/

#include "../include/sensorThread.h"
#include "../include/networkThread.h"

volatile int system_running = 1;
system_mode_t system_mode = MODE_REAL;

int main()
{
    printf("mng server started...\n");

    pthread_t server_thread, network_thread;
    network_thread_arg_t srv_arg;

    srv_arg.server_port = PORT;

    rb = ringBufferInit();
    if (!rb)
    {
        perror("ringBufferInit failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&server_thread, NULL, sensorTask, NULL) != 0)
    {
        perror("Sensor thread not created...");
        return EXIT_FAILURE;
    }

    if (pthread_create(&network_thread, NULL, networkTask, &srv_arg) != 0)
    {
        perror("Network thread not created...");
        return EXIT_FAILURE;
    }

    if (pthread_join(server_thread, NULL) != 0)
    {
        perror("Sensor thread join failed...");
        return EXIT_FAILURE;
    }

    if (pthread_join(network_thread, NULL) != 0)
    {
        perror("Network thread join failed...");
        return EXIT_FAILURE;
    }

    free(rb);
    printf("mng server finished...\n");

    return EXIT_SUCCESS;
}