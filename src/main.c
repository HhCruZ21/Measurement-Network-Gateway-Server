#include "../include/mainH.h"
#include "../include/measdev.h"
#include "../include/sensorThread.h"
#include "../include/networkThread.h"

int main()
{
    printf("mng server started");

    pthread_t sensor_thread, network_thread;

    if (pthread_create(&sensor_thread, NULL, sensorTask, NULL) != 0)
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