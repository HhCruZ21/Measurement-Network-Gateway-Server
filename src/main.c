#include "../include/mainH.h"
#include "../include/measdev.h"
#include "../include/serverThread.h"
#include "../include/networkThread.h"


int main()
{
    printf("mng server started");

    pthread_t sensor_thread, network_thread;
    server_thread_arg srv_arg;

    srv_arg.server_port = PORT;

    if (pthread_create(&sensor_thread, NULL, sensorTask, &srv_arg) != 0)
    {
        printf("Sensor thread not created...");
        return EXIT_FAILURE;
    }

    if (pthread_create(&network_thread, NULL, networkTask, NULL) != 0)
    {
        printf("Network thread not created...");
        return EXIT_FAILURE;
    }

    if (pthread_join(sensor_thread, NULL) != 0)
    {
        printf("Sensor thread join failed...");
        return EXIT_FAILURE;
    }

    if (pthread_join(network_thread, NULL) != 0)
    {
        printf("Network thread join failed...");
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}