#include "../include/networkThread.h"
#include "../include/sensorThread.h"

#define MAX_BATCH 96

extern sensor_config_t sensor_cfg[snsr_cnt];

static int sensor_from_string(const char *s)
{
    if (!strcmp(s, "TEMP"))
        return temp_sid;
    if (!strcmp(s, "ADC0"))
        return adc_zero_sid;
    if (!strcmp(s, "ADC1"))
        return adc_one_sid;
    if (!strcmp(s, "SW"))
        return sw_sid;
    if (!strcmp(s, "PB"))
        return pb_sid;
    return -1;
}

void *networkTask(void *arg)
{
    printf("Network thread initialized...\n");

    network_thread_arg_t *srv_arg = (network_thread_arg_t *)arg;

    char cmd_buffer[CMD_BUF_SIZE];
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    stream_t stream_state = STREAM_IDLE;
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

    while (system_running)
    {
        printf("Waiting for client...\n");
        addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50000; // 50 ms timeout

        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (client_fd < 0)
        {
            if (!system_running)
                break;
            printf("Client connection failed to server port %d...\n", srv_arg->server_port);
            continue;
        }

        printf("Client connected...\n");
        stream_state = STREAM_IDLE;

        while (1)
        {
            memset(cmd_buffer, 0, CMD_BUF_SIZE);
            ssize_t bytes_received = recv(client_fd, cmd_buffer, CMD_BUF_SIZE - 1, 0);

            if (bytes_received > 0)
            {
                cmd_buffer[bytes_received] = '\0';

                // strip newline
                for (char *p = cmd_buffer; *p; ++p)
                    if (*p == '\n' || *p == '\r')
                        *p = '\0';

                printf("Received command: %s\n", cmd_buffer);

                if (!strcmp(cmd_buffer, "START"))
                {
                    stream_state = STREAM_RUNNING;
                }
                else if (!strncmp(cmd_buffer, "CONFIGURE", 9))
                {
                    char sensor[16];
                    uint32_t rate;

                    if (sscanf(cmd_buffer, "CONFIGURE %15s %u", sensor, &rate) == 2)
                    {
                        int sid = sensor_from_string(sensor);
                        if (sid >= 0 && rate > 0)
                        {
                            pthread_mutex_lock(&sensor_cfg[sid].lock);
                            sensor_cfg[sid].rate_hz = rate;
                            sensor_cfg[sid].period_us = 1000000ULL / rate;
                            sensor_cfg[sid].next_deadline = getElapsedTime() + sensor_cfg[sid].period_us;
                            pthread_mutex_unlock(&sensor_cfg[sid].lock);

                            send(client_fd, "OK\n", 3, 0);
                        }
                        else
                            send(client_fd, "ERR\n", 4, 0);
                    }
                    else
                    {
                        send(client_fd, "ERR\n", 4, 0);
                    }
                }
                else if (!strcmp(cmd_buffer, "STOP"))
                {
                    stream_state = STREAM_IDLE;
                }
                else if (!strcmp(cmd_buffer, "DISCONNECT"))
                {
                    stream_state = STREAM_DISCONNECT;
                }
                else if (!strcmp(cmd_buffer, "SHUTDOWN"))
                {
                    system_running = 0;
                    stream_state = STREAM_DISCONNECT;
                }
                else
                {
                    printf("Unknown command: %s\n", cmd_buffer);
                }
            }
            else if (bytes_received == 0)
            {
                stream_state = STREAM_DISCONNECT;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // no command, continue normal operation
                }
                else
                {
                    perror("recv failed");
                    stream_state = STREAM_DISCONNECT;
                }
            }

            if (stream_state == STREAM_RUNNING)
            {
                sensor_data_t batch[MAX_BATCH];
                int count = 0;

                while (count < MAX_BATCH)
                {

                    pthread_mutex_lock(&rb->rbMutex);
                    int empty = (rb->count == 0);
                    pthread_mutex_unlock(&rb->rbMutex);
                    if (empty)
                        break;

                    ringBufferRemoveSample(rb, &batch[count]);
                    count++;
                }

                char logbuf[128];

                for (int i = 0; i < count; i++)
                {
                    int len = snprintf(logbuf, sizeof(logbuf),
                                       "[NET TX] id=%d val=%u ts=%lu\n",
                                       batch[i].sensor_id,
                                       batch[i].sensor_value,
                                       (unsigned long)batch[i].timestamp);

                    if (len > 0)
                        write(STDERR_FILENO, logbuf, len);
                }

                size_t total_bytes = count * sizeof(sensor_data_t);
                size_t sent_bytes = 0;
                uint8_t *ptr = (uint8_t *)batch;

                while (sent_bytes < total_bytes)
                {
                    ssize_t n = send(client_fd,
                                     ptr + sent_bytes,
                                     total_bytes - sent_bytes,
                                     0);
                    if (n <= 0)
                    {
                        perror("send failed");
                        stream_state = STREAM_DISCONNECT;
                        break;
                    }
                    sent_bytes += n;
                }
                if (count == 0)
                    usleep(100); // .1 ms backoff when no data
            }

            // Handle client here1
            if (stream_state == STREAM_DISCONNECT)
            {
                printf("Closing client connection...\n");
                shutdown(client_fd, SHUT_RDWR);
                close(client_fd);
                break;
            }
        }
        if (!system_running)
            break;
    }
    close(server_fd);
    return NULL;
}