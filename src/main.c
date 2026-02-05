#include "../include/mainH.h"
#include "../include/measdev.h"
#include "../include/serverThread.h"
#include "../include/networkThread.h"


int main()
{
    printf("mng server started");

    pthread_t server_thread, network_thread;
    server_thread_arg_t srv_arg;

    srv_arg.server_port = PORT;

    if (pthread_create(&server_thread, NULL, serverTask, &srv_arg) != 0)
    {
        perror("Sensor thread not created...");
        return EXIT_FAILURE;
    }

    if (pthread_create(&network_thread, NULL, networkTask, NULL) != 0)
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


    return EXIT_SUCCESS;
}