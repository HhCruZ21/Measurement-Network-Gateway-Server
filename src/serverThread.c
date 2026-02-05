#include "../include/serverThread.h"

void *serverTask(void *arg)
{

    server_thread_arg *srv_arg = (server_thread_arg *)arg;

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed...");
        return NULL;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Set socket for reuse failed...");
        close(server_fd);
        return NULL;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(srv_arg->server_port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Socket bind failed...");
        close(server_fd);
        return NULL;
    }

    if (listen(server_fd, SOMAXCONN) < 0)
    {
        perror("Socket listen failed...");
        close(server_fd);
        return NULL;
    }

    printf("Server listening to port %d...\n", srv_arg->server_port);

    while (1)
    {
        printf("Waiting for client...\n");
        addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

        if (client_fd < 0)
        {
            printf("Client connection failed to server port %d...\n", srv_arg->server_port);
            continue;
        }

        printf("Client connected...\n");

        // Handle client here

        close(client_fd);
    }
    close(server_fd);
    return NULL;
}